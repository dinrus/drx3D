#include "PassiveSocket.h"  // Include header for active socket object definition

#include <stdio.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/b3CommandLineArgs.h>

#ifdef NO_SHARED_MEMORY
#include "PhysicsServerCommandProcessor.h"
typedef PhysicsServerCommandProcessor MyCommandProcessor;
#else
#include "SharedMemoryCommandProcessor.h"
typedef SharedMemoryCommandProcessor MyCommandProcessor;
#endif  //NO_SHARED_MEMORY

#include "SharedMemoryCommands.h"
#include <drx3D/Common/b3AlignedObjectArray.h>
#include "PhysicsServerCommandProcessor.h"
#include <drx3D/Common/b3Clock.h>

bool gVerboseNetworkMessagesServer = true;

void MySerializeInt(u32 sz, u8* output)
{
	u32 tmp = sz;
	output[0] = tmp & 255;
	tmp = tmp >> 8;
	output[1] = tmp & 255;
	tmp = tmp >> 8;
	output[2] = tmp & 255;
	tmp = tmp >> 8;
	output[3] = tmp & 255;
}

i32 main(i32 argc, tuk argv[])
{
	b3CommandLineArgs parseArgs(argc, argv);
	b3Clock clock;
	double timeOutInSeconds = 10;

	DummyGUIHelper guiHelper;
	MyCommandProcessor* sm = new MyCommandProcessor;
	sm->setGuiHelper(&guiHelper);

	i32 port = 6667;
	parseArgs.GetCmdLineArgument("port", port);

	gVerboseNetworkMessagesServer = parseArgs.CheckCmdLineFlag("verbose");

#ifndef NO_SHARED_MEMORY
	i32 key = 0;
	if (parseArgs.GetCmdLineArgument("sharedMemoryKey", key))
	{
		sm->setSharedMemoryKey(key);
	}
#endif  //NO_SHARED_MEMORY

	bool isPhysicsClientConnected = sm->connect();
	bool exitRequested = false;

	if (isPhysicsClientConnected)
	{
		printf("Starting TCP server using port %d\n", port);

		CPassiveSocket socket;
		CActiveSocket* pClient = NULL;

		//--------------------------------------------------------------------------
		// Initialize our socket object
		//--------------------------------------------------------------------------
		socket.Initialize();

		socket.Listen("localhost", port);
		//socket.SetBlocking();

		i32 curNumErr = 0;

		while (!exitRequested)
		{
			b3Clock::usleep(0);

			if ((pClient = socket.Accept()) != NULL)
			{
				b3AlignedObjectArray<char> bytesReceived;

				i32 clientPort = socket.GetClientPort();
				printf("connected from %s:%d\n", socket.GetClientAddr(), clientPort);

				if (pClient->Receive(4))
				{
					i32 clientKey = *(i32*)pClient->GetData();

					if (clientKey == SHARED_MEMORY_MAGIC_NUMBER)
					{
						printf("Client version OK %d\n", clientKey);
					}
					else
					{
						printf("Server version (%d) mismatches Client Version (%d)\n", SHARED_MEMORY_MAGIC_NUMBER, clientKey);
						continue;
					}
				}

				//----------------------------------------------------------------------
				// Receive request from the client.
				//----------------------------------------------------------------------
				while (1)
				{
					//printf("try receive\n");
					bool receivedData = false;

					i32 maxLen = 4 + sizeof(SharedMemoryStatus) + SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE;

					if (pClient->Receive(maxLen))
					{
						//heuristic to detect disconnected clients
						CSimpleSocket::CSocketError err = pClient->GetSocketError();

						if (err != CSimpleSocket::SocketSuccess || !pClient->IsSocketValid())
						{
							b3Clock::usleep(100);

							curNumErr++;

							if (curNumErr > 100)
							{
								printf("TCP Connection error = %d, curNumErr = %d\n", (i32)err, curNumErr);

								break;
							}
						}

						curNumErr = 0;
						tuk msg2 = (tuk)pClient->GetData();
						i32 numBytesRec2 = pClient->GetBytesReceived();

						i32 curSize = bytesReceived.size();
						bytesReceived.resize(bytesReceived.size() + numBytesRec2);
						for (i32 i = 0; i < numBytesRec2; i++)
						{
							bytesReceived[curSize + i] = msg2[i];
						}

						if (bytesReceived.size() >= 4)
						{
							i32 numBytesRec = bytesReceived.size();
							if (numBytesRec >= 10)
							{
								if (strncmp(&bytesReceived[0], "disconnect", 10) == 0)
								{
									printf("Disconnect request received\n");
									bytesReceived.clear();
									break;
								}

								if (strncmp(&bytesReceived[0], "terminateserver", 10) == 0)
								{
									printf("Terminate server request received\n");
									exitRequested = true;
									bytesReceived.clear();
									break;
								}
							}

							if (gVerboseNetworkMessagesServer)
							{
								printf("received message length [%d]\n", numBytesRec);
							}

							receivedData = true;

							SharedMemoryCommand cmd;

							SharedMemoryCommand* cmdPtr = 0;

							i32 type = *(i32*)&bytesReceived[0];

							//performance test
							if (numBytesRec == sizeof(i32))
							{
								cmdPtr = &cmd;
								cmd.m_type = *(i32*)&bytesReceived[0];
							}
							else
							{
								if (numBytesRec == sizeof(SharedMemoryCommand))
								{
									cmdPtr = (SharedMemoryCommand*)&bytesReceived[0];
								}
								else
								{
									if (numBytesRec == 36)
									{
										cmdPtr = &cmd;
										memcpy(&cmd, &bytesReceived[0], numBytesRec);
									}
								}
							}
							if (cmdPtr)
							{
								SharedMemoryStatus serverStatus;
								b3AlignedObjectArray<char> buffer;
								buffer.resize(SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE);

								bool hasStatus = sm->processCommand(*cmdPtr, serverStatus, &buffer[0], buffer.size());

								double startTimeSeconds = clock.getTimeInSeconds();
								double curTimeSeconds = clock.getTimeInSeconds();

								while ((!hasStatus) && ((curTimeSeconds - startTimeSeconds) < timeOutInSeconds))
								{
									hasStatus = sm->receiveStatus(serverStatus, &buffer[0], buffer.size());
									curTimeSeconds = clock.getTimeInSeconds();
								}
								if (gVerboseNetworkMessagesServer)
								{
									//printf("buffer.size = %d\n", buffer.size());
									printf("serverStatus.m_numDataStreamBytes = %d\n", serverStatus.m_numDataStreamBytes);
								}
								if (hasStatus)
								{
									b3AlignedObjectArray<u8> packetData;
									u8* statBytes = (u8*)&serverStatus;

									if (cmdPtr->m_type == CMD_STEP_FORWARD_SIMULATION)
									{
										packetData.resize(4 + sizeof(i32));
										i32 sz = packetData.size();
										i32 curPos = 0;

										MySerializeInt(sz, &packetData[curPos]);
										curPos += 4;
										for (i32 i = 0; i < sizeof(i32); i++)
										{
											packetData[i + curPos] = statBytes[i];
										}
										curPos += sizeof(i32);

										pClient->Send(&packetData[0], packetData.size());
									}
									else
									{
										if (cmdPtr->m_type == CMD_REQUEST_VR_EVENTS_DATA)
										{
											i32 headerSize = 16 + 5 * sizeof(i32) + sizeof(smUint64_t) + sizeof(tuk) + sizeof(b3VRControllerEvent) * serverStatus.m_sendVREvents.m_numVRControllerEvents;
											packetData.resize(4 + headerSize);
											i32 sz = packetData.size();
											i32 curPos = 0;
											MySerializeInt(sz, &packetData[curPos]);
											curPos += 4;
											for (i32 i = 0; i < headerSize; i++)
											{
												packetData[i + curPos] = statBytes[i];
											}
											curPos += headerSize;
											pClient->Send(&packetData[0], packetData.size());
										}
										else
										{
											//create packetData with [i32 packetSizeInBytes, status, streamBytes)
											packetData.resize(4 + sizeof(SharedMemoryStatus) + serverStatus.m_numDataStreamBytes);
											i32 sz = packetData.size();
											i32 curPos = 0;

											if (gVerboseNetworkMessagesServer)
											{
												//printf("buffer.size = %d\n", buffer.size());
												printf("serverStatus packed size = %d\n", sz);
											}

											MySerializeInt(sz, &packetData[curPos]);
											curPos += 4;
											for (i32 i = 0; i < sizeof(SharedMemoryStatus); i++)
											{
												packetData[i + curPos] = statBytes[i];
											}
											curPos += sizeof(SharedMemoryStatus);

											for (i32 i = 0; i < serverStatus.m_numDataStreamBytes; i++)
											{
												packetData[i + curPos] = buffer[i];
											}

											pClient->Send(&packetData[0], packetData.size());
										}
									}
								}

								bytesReceived.clear();
							}
							else
							{
								//likely an incomplete packet, let's append more bytes
								//printf("received packet with unknown contents\n");
							}
						}
					}
					if (!receivedData)
					{
						//printf("Didn't receive data.\n");
					}
				}
				printf("Disconnecting client.\n");
				pClient->Close();
				delete pClient;
			}
		}

		socket.Close();
		socket.Shutdown(CSimpleSocket::Both);
	}
	else
	{
		printf("Ошибка: cannot connect to shared memory physics server.");
	}

	delete sm;

	return 0;
}
