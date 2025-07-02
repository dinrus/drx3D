#include <drx3D/SharedMemory/PhysicsClientUDP.h>
#include <X/enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <drx3D/Common/b3Clock.h>
#include <drx3D/SharedMemory/PhysicsClient.h>
#include <drx3D/SharedMemory/SharedMemoryCommands.h>
#include <string>
#include <drx3D/Common/b3Logging.h>
#include <drx3D/MT/ThreadSupportInterface.h>
void UDPThreadFunc(uk userPtr, uk lsMemory);
uk UDPlsMemoryFunc();
void UDPlsMemoryReleaseFunc(uk ptr);

bool gVerboseNetworkMessagesClient = false;

#ifndef _WIN32
#include <drx3D/MT/PosixThreadSupport.h>

ThreadSupportInterface* createUDPThreadSupport(i32 numThreads)
{
	PosixThreadSupport::ThreadConstructionInfo constructionInfo("UDPThread",
																  UDPThreadFunc,
																  UDPlsMemoryFunc,
																  UDPlsMemoryReleaseFunc,
																  numThreads);
	ThreadSupportInterface* threadSupport = new PosixThreadSupport(constructionInfo);

	return threadSupport;
}

#elif defined(_WIN32)
#include <drx3D/MT/b3Win32ThreadSupport.h>

ThreadSupportInterface* createUDPThreadSupport(i32 numThreads)
{
	b3Win32ThreadSupport::Win32ThreadConstructionInfo threadConstructionInfo("UDPThread", UDPThreadFunc, UDPlsMemoryFunc, UDPlsMemoryReleaseFunc, numThreads);
	b3Win32ThreadSupport* threadSupport = new b3Win32ThreadSupport(threadConstructionInfo);
	return threadSupport;
}
#endif

struct UDPThreadLocalStorage
{
	i32 threadId;
};

u32 b3DeserializeInt(u8k* input)
{
	u32 tmp = (input[3] << 24) + (input[2] << 16) + (input[1] << 8) + input[0];
	return tmp;
}

struct UdpNetworkedInternalData
{
	ENetHost* m_client;
	ENetAddress m_address;
	ENetPeer* m_peer;
	ENetEvent m_event;
	bool m_isConnected;

	ThreadSupportInterface* m_threadSupport;

	b3CriticalSection* m_cs;

	UdpNetworkedInternalData* m_udpInternalData;

	SharedMemoryCommand m_clientCmd;
	bool m_hasCommand;

	bool m_hasStatus;
	SharedMemoryStatus m_lastStatus;
	b3AlignedObjectArray<char> m_stream;

	STxt m_hostName;
	i32 m_port;
	double m_timeOutInSeconds;

	UdpNetworkedInternalData()
		: m_client(0),
		  m_peer(0),
		  m_isConnected(false),
		  m_threadSupport(0),
		  m_hasCommand(false),
		  m_hasStatus(false),
		  m_timeOutInSeconds(60)
	{
	}

	bool connectUDP()
	{
		if (m_isConnected)
			return true;

		if (enet_initialize() != 0)
		{
			fprintf(stderr, "Error initialising enet");

			exit(EXIT_FAILURE);
		}

		m_client = enet_host_create(NULL,       /* create a client host */
									1,          /* number of clients */
									2,          /* number of channels */
									57600 / 8,  /* incoming bandwith */
									14400 / 8); /* outgoing bandwith */

		if (m_client == NULL)
		{
			fprintf(stderr, "Could not create client host");
			return false;
		}

		enet_address_set_host(&m_address, m_hostName.c_str());
		m_address.port = m_port;

		m_peer = enet_host_connect(m_client,
								   &m_address, /* address to connect to */
								   2,          /* number of channels */
								   0);         /* user data supplied to
						 the receiving host */

		if (m_peer == NULL)
		{
			fprintf(stderr,
					"No available peers for initiating an ENet "
					"connection.\n");
			return false;
		}

		/* Try to connect to server within 5 seconds */
		if (enet_host_service(m_client, &m_event, 5000) > 0 &&
			m_event.type == ENET_EVENT_TYPE_CONNECT)
		{
			puts("Connection to server succeeded.");
		}
		else
		{
			/* Either the 5 seconds are up or a disconnect event was */
			/* received. Reset the peer in the event the 5 seconds   */
			/* had run out without any significant event.            */
			enet_peer_reset(m_peer);

			fprintf(stderr, "Connection to server failed.");
			return false;
		}

		i32 serviceResult = enet_host_service(m_client, &m_event, 0);

		if (serviceResult > 0)
		{
			switch (m_event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
					printf("A new client connected from %x:%u.\n",
						   m_event.peer->address.host,
						   m_event.peer->address.port);
					m_event.peer->data = (uk )"New User";
					break;

				case ENET_EVENT_TYPE_RECEIVE:

					if (gVerboseNetworkMessagesClient)
					{
						printf(
							"A packet of length %lu containing '%s' was "
							"received from %s on channel %u.\n",
							m_event.packet->dataLength,
							(tuk)m_event.packet->data,
							(tuk)m_event.peer->data,
							m_event.channelID);
					}
					/* Clean up the packet now that we're done using it.
				> */
					enet_packet_destroy(m_event.packet);

					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					printf("%s disconnected.\n", (tuk)m_event.peer->data);

					break;
				default:
				{
					printf("unknown event type: %d.\n", m_event.type);
				}
			}
		}
		else if (serviceResult > 0)
		{
			puts("Error with servicing the client");
			return false;
		}

		m_isConnected = true;
		return m_isConnected;
	}

	bool checkData()
	{
		bool hasStatus = false;

		i32 serviceResult = enet_host_service(m_client, &m_event, 0);

		if (serviceResult > 0)
		{
			switch (m_event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
					printf("A new client connected from %x:%u.\n",
						   m_event.peer->address.host,
						   m_event.peer->address.port);

					m_event.peer->data = (uk )"New User";
					break;

				case ENET_EVENT_TYPE_RECEIVE:
				{
					if (gVerboseNetworkMessagesClient)
					{
						printf(
							"A packet of length %lu containing '%s' was "
							"received from %s on channel %u.\n",
							m_event.packet->dataLength,
							(tuk)m_event.packet->data,
							(tuk)m_event.peer->data,
							m_event.channelID);
					}

					i32 packetSizeInBytes = b3DeserializeInt(m_event.packet->data);

					if (packetSizeInBytes == m_event.packet->dataLength)
					{
						SharedMemoryStatus* statPtr = (SharedMemoryStatus*)&m_event.packet->data[4];
						if (statPtr->m_type == CMD_STEP_FORWARD_SIMULATION_COMPLETED)
						{
							SharedMemoryStatus dummy;
							dummy.m_type = CMD_STEP_FORWARD_SIMULATION_COMPLETED;
							m_lastStatus = dummy;
							m_stream.resize(0);
						}
						else
						{
							m_lastStatus = *statPtr;
							i32 streamOffsetInBytes = 4 + sizeof(SharedMemoryStatus);
							i32 numStreamBytes = packetSizeInBytes - streamOffsetInBytes;
							m_stream.resize(numStreamBytes);
							for (i32 i = 0; i < numStreamBytes; i++)
							{
								m_stream[i] = m_event.packet->data[i + streamOffsetInBytes];
							}
						}
					}
					else
					{
						printf("unknown status message received\n");
					}
					enet_packet_destroy(m_event.packet);
					hasStatus = true;
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					printf("%s disconnected.\n", (tuk)m_event.peer->data);

					break;
				}
				default:
				{
					printf("unknown event type: %d.\n", m_event.type);
				}
			}
		}
		else if (serviceResult > 0)
		{
			puts("Error with servicing the client");
		}

		return hasStatus;
	}
};

enum UDPThreadEnums
{
	eUDPRequestTerminate = 13,
	eUDPIsUnInitialized,
	eUDPIsInitialized,
	eUDPInitializationFailed,
	eUDPHasTerminated
};

enum UDPCommandEnums
{
	eUDPIdle = 13,
	eUDP_ConnectRequest,
	eUDP_Connected,
	eUDP_ConnectionFailed,
	eUDP_DisconnectRequest,
	eUDP_Disconnected,

};

void UDPThreadFunc(uk userPtr, uk lsMemory)
{
	printf("UDPThreadFunc thread started\n");
	//	UDPThreadLocalStorage* localStorage = (UDPThreadLocalStorage*)lsMemory;

	UdpNetworkedInternalData* args = (UdpNetworkedInternalData*)userPtr;
	//	i32 workLeft = true;
	b3Clock clock;
	clock.reset();
	bool init = true;
	if (init)
	{
		args->m_cs->lock();
		args->m_cs->setSharedParam(0, eUDPIsInitialized);
		args->m_cs->unlock();

		double deltaTimeInSeconds = 0;

		do
		{
			b3Clock::usleep(0);

			deltaTimeInSeconds += double(clock.getTimeMicroseconds()) / 1000000.;

			{
				clock.reset();
				deltaTimeInSeconds = 0.f;
				switch (args->m_cs->getSharedParam(1))
				{
					case eUDP_ConnectRequest:
					{
						bool connected = args->connectUDP();
						if (connected)
						{
							args->m_cs->setSharedParam(1, eUDP_Connected);
						}
						else
						{
							args->m_cs->setSharedParam(1, eUDP_ConnectionFailed);
						}
						break;
					}
					default:
					{
					}
				};

				if (args->m_isConnected)
				{
					args->m_cs->lock();
					bool hasCommand = args->m_hasCommand;
					args->m_cs->unlock();

					if (hasCommand)
					{
						i32 sz = 0;
						ENetPacket* packet = 0;

						if (args->m_clientCmd.m_type == CMD_STEP_FORWARD_SIMULATION)
						{
							sz = sizeof(i32);
							packet = enet_packet_create(&args->m_clientCmd.m_type, sz, ENET_PACKET_FLAG_RELIABLE);
						}
						else
						{
							sz = sizeof(SharedMemoryCommand);
							packet = enet_packet_create(&args->m_clientCmd, sz, ENET_PACKET_FLAG_RELIABLE);
						}
						i32 res;
						res = enet_peer_send(args->m_peer, 0, packet);
						args->m_cs->lock();
						args->m_hasCommand = false;
						args->m_cs->unlock();
					}

					bool hasNewStatus = args->checkData();
					if (hasNewStatus)
					{
						if (args->m_hasStatus)
						{
							//overflow: last status hasn't been processed yet
							drx3DAssert(0);
							printf("Ошибка: received new status but previous status not processed yet");
						}
						else
						{
							args->m_cs->lock();
							args->m_hasStatus = hasNewStatus;
							args->m_cs->unlock();
						}
					}
				}
			}

		} while (args->m_cs->getSharedParam(0) != eUDPRequestTerminate);
	}
	else
	{
		args->m_cs->lock();
		args->m_cs->setSharedParam(0, eUDPInitializationFailed);
		args->m_cs->unlock();
	}

	printf("finished\n");
}

uk UDPlsMemoryFunc()
{
	//don't create local store memory, just return 0
	return new UDPThreadLocalStorage;
}

void UDPlsMemoryReleaseFunc(uk ptr)
{
	UDPThreadLocalStorage* p = (UDPThreadLocalStorage*)ptr;
	delete p;
}

UdpNetworkedPhysicsProcessor::UdpNetworkedPhysicsProcessor(tukk hostName, i32 port)
{
	m_data = new UdpNetworkedInternalData;
	if (hostName)
	{
		m_data->m_hostName = hostName;
	}
	m_data->m_port = port;
}

UdpNetworkedPhysicsProcessor::~UdpNetworkedPhysicsProcessor()
{
	disconnect();
	delete m_data;
}

bool UdpNetworkedPhysicsProcessor::processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	if (gVerboseNetworkMessagesClient)
	{
		printf("PhysicsClientUDP::processCommand\n");
	}
	//	i32 sz = sizeof(SharedMemoryCommand);

	b3Clock clock;
	double startTime = clock.getTimeInSeconds();
	double timeOutInSeconds = m_data->m_timeOutInSeconds;

	m_data->m_cs->lock();
	m_data->m_clientCmd = clientCmd;
	m_data->m_hasCommand = true;
	m_data->m_cs->unlock();

	while ((m_data->m_hasCommand) && (clock.getTimeInSeconds() - startTime < timeOutInSeconds))
	{
		b3Clock::usleep(0);
	}

#if 0


	bool hasStatus = false;

	b3Clock clock;
	double startTime = clock.getTimeInSeconds();
	double timeOutInSeconds = m_data->m_timeOutInSeconds;

	const SharedMemoryStatus* stat = 0;
	while ((!hasStatus) && (clock.getTimeInSeconds() - startTime < timeOutInSeconds))
	{
		hasStatus = receiveStatus(serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		b3Clock::usleep(100);
	}
	return hasStatus;

#endif

	return false;
}

bool UdpNetworkedPhysicsProcessor::receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = false;
	if (m_data->m_hasStatus)
	{
		if (gVerboseNetworkMessagesClient)
		{
			printf("UdpNetworkedPhysicsProcessor::receiveStatus\n");
		}

		hasStatus = true;
		serverStatusOut = m_data->m_lastStatus;
		i32 numStreamBytes = m_data->m_stream.size();

		if (numStreamBytes < bufferSizeInBytes)
		{
			for (i32 i = 0; i < numStreamBytes; i++)
			{
				bufferServerToClient[i] = m_data->m_stream[i];
			}
		}
		else
		{
			printf("Ошибка: steam buffer overflow\n");
		}

		m_data->m_cs->lock();
		m_data->m_hasStatus = false;
		m_data->m_cs->unlock();
	}

	return hasStatus;
}

void UdpNetworkedPhysicsProcessor::renderScene(i32 renderFlags)
{
}

void UdpNetworkedPhysicsProcessor::physicsDebugDraw(i32 debugDrawFlags)
{
}

void UdpNetworkedPhysicsProcessor::setGuiHelper(struct GUIHelperInterface* guiHelper)
{
}

bool UdpNetworkedPhysicsProcessor::isConnected() const
{
	return m_data->m_isConnected;
}

bool UdpNetworkedPhysicsProcessor::connect()
{
	if (m_data->m_threadSupport == 0)
	{
		m_data->m_threadSupport = createUDPThreadSupport(1);

		m_data->m_cs = m_data->m_threadSupport->createCriticalSection();
		m_data->m_cs->setSharedParam(0, eUDPIsUnInitialized);
		m_data->m_threadSupport->runTask(D3_THREAD_SCHEDULE_TASK, (uk )m_data, 0);

		while (m_data->m_cs->getSharedParam(0) == eUDPIsUnInitialized)
		{
			b3Clock::usleep(1000);
		}

		m_data->m_cs->lock();
		m_data->m_cs->setSharedParam(1, eUDP_ConnectRequest);
		m_data->m_cs->unlock();

		while (m_data->m_cs->getSharedParam(1) == eUDP_ConnectRequest)
		{
			b3Clock::usleep(1000);
		}
	}
	u32 sharedParam = m_data->m_cs->getSharedParam(1);
	bool isConnected = (sharedParam == eUDP_Connected);
	return isConnected;
}

void UdpNetworkedPhysicsProcessor::disconnect()
{
	if (m_data->m_threadSupport)
	{
		m_data->m_cs->lock();
		m_data->m_cs->setSharedParam(0, eUDPRequestTerminate);
		m_data->m_cs->unlock();

		i32 numActiveThreads = 1;

		while (numActiveThreads)
		{
			i32 arg0, arg1;
			if (m_data->m_threadSupport->isTaskCompleted(&arg0, &arg1, 0))
			{
				numActiveThreads--;
				printf("numActiveThreads = %d\n", numActiveThreads);
			}
			else
			{
				b3Clock::usleep(1000);
			}
		};

		printf("stopping threads\n");

		delete m_data->m_threadSupport;
		m_data->m_threadSupport = 0;
		m_data->m_isConnected = false;
	}
}

void UdpNetworkedPhysicsProcessor::setTimeOut(double timeOutInSeconds)
{
	m_data->m_timeOutInSeconds = timeOutInSeconds;
}
