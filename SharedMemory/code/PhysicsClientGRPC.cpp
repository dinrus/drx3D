#ifdef DRX3D_ENABLE_GRPC
#include "PhysicsClientGRPC.h"
#include <drx3D/SharedMemory/grpc/proto/pybullet.grpc.pb.h>
#include <grpc++/grpc++.h>
using grpc::Channel;
#include <stdio.h>
#include <string.h>
#include <drx3D/Common/b3Clock.h>
#include "PhysicsClient.h"
//#include <drx3D/Maths/Linear/Vec3.h"
#include "SharedMemoryCommands.h"
#include <string>
#include <drx3D/Common/b3Logging.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/SharedMemory/grpc/ConvertGRPCBullet.h>

using pybullet_grpc::grpc::PyBulletAPI;

static u32 b3DeserializeInt2(u8k* input)
{
	u32 tmp = (input[3] << 24) + (input[2] << 16) + (input[1] << 8) + input[0];
	return tmp;
}

bool gVerboseNetworkMessagesClient3 = false;

struct GRPCNetworkedInternalData
{
	std::shared_ptr<grpc::Channel> m_grpcChannel;
	std::unique_ptr<PyBulletAPI::Stub> m_stub;

	bool m_isConnected;

	SharedMemoryCommand m_clientCmd;
	bool m_hasCommand;

	SharedMemoryStatus m_lastStatus;
	b3AlignedObjectArray<char> m_stream;

	STxt m_hostName;
	i32 m_port;

	b3AlignedObjectArray<u8> m_tempBuffer;
	double m_timeOutInSeconds;

	GRPCNetworkedInternalData()
		: m_isConnected(false),
		  m_hasCommand(false),
		  m_timeOutInSeconds(60)
	{
	}

	void disconnect()
	{
		if (m_isConnected)
		{
			m_stub = 0;
			m_grpcChannel = 0;
			m_isConnected = false;
		}
	}
	bool connectGRPC()
	{
		if (m_isConnected)
			return true;
		STxt hostport = m_hostName;
		if (m_port >= 0)
		{
			hostport += ':' + std::to_string(m_port);
		}
		m_grpcChannel = grpc::CreateChannel(
			hostport, grpc::InsecureChannelCredentials());

		m_stub = PyBulletAPI::NewStub(m_grpcChannel);

		// Set timeout for API
		std::chrono::system_clock::time_point deadline =
			std::chrono::system_clock::now() + std::chrono::seconds((long long)m_timeOutInSeconds);
		grpc::ClientContext context;
		context.set_deadline(deadline);
		::pybullet_grpc::PyBulletCommand request;
		pybullet_grpc::CheckVersionCommand* cmd1 = request.mutable_checkversioncommand();
		cmd1->set_clientversion(SHARED_MEMORY_MAGIC_NUMBER);
		::pybullet_grpc::PyBulletStatus response;
		// The actual RPC.
		grpc::Status status = m_stub->SubmitCommand(&context, request, &response);
		if (response.has_checkversionstatus())
		{
			if (response.checkversionstatus().serverversion() == SHARED_MEMORY_MAGIC_NUMBER)
			{
				m_isConnected = true;
			}
			else
			{
				printf("Ошибка: Client version (%d) is different from server version (%d)", SHARED_MEMORY_MAGIC_NUMBER, response.checkversionstatus().serverversion());
			}
		}
		else
		{
			printf("Ошибка: cannot connect to GRPC server\n");
		}

		return m_isConnected;
	}

	bool checkData()
	{
		bool hasStatus = false;
		return hasStatus;
	}
};

GRPCNetworkedPhysicsProcessor::GRPCNetworkedPhysicsProcessor(tukk hostName, i32 port)
{
	m_data = new GRPCNetworkedInternalData;
	if (hostName)
	{
		m_data->m_hostName = hostName;
	}
	m_data->m_port = port;
}

GRPCNetworkedPhysicsProcessor::~GRPCNetworkedPhysicsProcessor()
{
	disconnect();
	delete m_data;
}

bool GRPCNetworkedPhysicsProcessor::processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	if (gVerboseNetworkMessagesClient3)
	{
		printf("GRPCNetworkedPhysicsProcessor::processCommand\n");
	}

	::pybullet_grpc::PyBulletCommand grpcCommand;
	pybullet_grpc::PyBulletCommand* grpcCmdPtr = convertBulletToGRPCCommand(clientCmd, grpcCommand);

	if (grpcCmdPtr)
	{
		grpc::ClientContext context;
		std::chrono::system_clock::time_point deadline =
			std::chrono::system_clock::now() + std::chrono::seconds((long long)m_data->m_timeOutInSeconds);
		context.set_deadline(deadline);
		::pybullet_grpc::PyBulletStatus status;
		// The actual RPC.
		grpc::Status grpcStatus = m_data->m_stub->SubmitCommand(&context, grpcCommand, &status);

		//convert grpc status to drx3D status
		bool convertedOk = convertGRPCToStatus(status, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
		if (!convertedOk)
		{
			disconnect();
		}
		return convertedOk;
	}

	return false;
}

bool GRPCNetworkedPhysicsProcessor::receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	bool hasStatus = m_data->checkData();

	if (hasStatus)
	{
		if (gVerboseNetworkMessagesClient3)
		{
			printf("GRPCNetworkedPhysicsProcessor::receiveStatus\n");
		}

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
	}

	return hasStatus;
}

void GRPCNetworkedPhysicsProcessor::renderScene(i32 renderFlags)
{
}

void GRPCNetworkedPhysicsProcessor::physicsDebugDraw(i32 debugDrawFlags)
{
}

void GRPCNetworkedPhysicsProcessor::setGuiHelper(struct GUIHelperInterface* guiHelper)
{
}

bool GRPCNetworkedPhysicsProcessor::isConnected() const
{
	return m_data->m_isConnected;
}

bool GRPCNetworkedPhysicsProcessor::connect()
{
	bool isConnected = m_data->connectGRPC();
	return isConnected;
}

void GRPCNetworkedPhysicsProcessor::disconnect()
{
	m_data->disconnect();
}

void GRPCNetworkedPhysicsProcessor::setTimeOut(double timeOutInSeconds)
{
	m_data->m_timeOutInSeconds = timeOutInSeconds;
}

#endif  //DRX3D_ENABLE_GRPC
