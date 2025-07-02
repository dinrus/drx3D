#include <drx3D/SharedMemory/PhysicsClientTCP.h>
#include <drx3D/Network2/ActiveSocket.h>

#include <stdio.h>
#include <string.h>
#include <drx3D/Common/b3Clock.h>
#include <drx3D/SharedMemory/PhysicsClient.h>
#include <drx3D/SharedMemory/SharedMemoryCommands.h>
#include <string>
#include <drx3D/Common/b3Logging.h>
#include <drx3D/Common/b3AlignedObjectArray.h>

u32 b3DeserializeInt2(u8k* input)
{
    u32 tmp = (input[3] << 24) + (input[2] << 16) + (input[1] << 8) + input[0];
    return tmp;
}
bool gVerboseNetworkMessagesClient2 = false;

struct TcpNetworkedInternalData
{
    /*
    ENetHost*   m_client;
    ENetAddress m_address;
    ENetPeer*   m_peer;
    ENetEvent   m_event;
     */
    CActiveSocket m_tcpSocket;

    bool m_isConnected;

    TcpNetworkedInternalData* m_tcpInternalData;

    SharedMemoryCommand m_clientCmd;
    bool m_hasCommand;

    SharedMemoryStatus m_lastStatus;
    b3AlignedObjectArray<char> m_stream;

    STxt m_hostName;
    i32 m_port;

    b3AlignedObjectArray<u8> m_tempBuffer;
    double m_timeOutInSeconds;

    TcpNetworkedInternalData()
        : m_isConnected(false),
          m_hasCommand(false),
          m_timeOutInSeconds(60)
    {
    }

    bool connectTCP()
    {
        if (m_isConnected)
            return true;

        m_tcpSocket.Initialize();

        m_isConnected = m_tcpSocket.Open(m_hostName.c_str(), m_port);
        if (m_isConnected)
        {
            m_tcpSocket.SetSendTimeout(m_timeOutInSeconds, 0);
            m_tcpSocket.SetReceiveTimeout(m_timeOutInSeconds, 0);
                        i32 key = SHARED_MEMORY_MAGIC_NUMBER;
                        m_tcpSocket.Send((u8*)&key, 4);
        }


        return m_isConnected;
    }

    bool checkData()
    {
        bool hasStatus = false;

        //i32 serviceResult = enet_host_service(m_client, &m_event, 0);
        i32 maxLen = 4 + sizeof(SharedMemoryStatus) + SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE;

        i32 rBytes = m_tcpSocket.Receive(maxLen);
        if (rBytes <= 0)
            return false;

        //append to tmp buffer
        //recBytes

        u8* d2 = (u8*)m_tcpSocket.GetData();

        i32 curSize = m_tempBuffer.size();
        m_tempBuffer.resize(curSize + rBytes);
        for (i32 i = 0; i < rBytes; i++)
        {
            m_tempBuffer[curSize + i] = d2[i];
        }

        i32 packetSizeInBytes = -1;

        if (m_tempBuffer.size() >= 4)
        {
            packetSizeInBytes = b3DeserializeInt2(&m_tempBuffer[0]);
        }

        if (m_tempBuffer.size() == packetSizeInBytes)
        {
            u8* data = &m_tempBuffer[0];
            if (gVerboseNetworkMessagesClient2)
            {
                printf("A packet of length %d bytes received\n", m_tempBuffer.size());
            }

            hasStatus = true;
            SharedMemoryStatus* statPtr = (SharedMemoryStatus*)&data[4];
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
                    m_stream[i] = data[i + streamOffsetInBytes];
                }
            }
            m_tempBuffer.clear();
        }
        return hasStatus;
    }
};

TcpNetworkedPhysicsProcessor::TcpNetworkedPhysicsProcessor(tukk hostName, i32 port)
{
    m_data = new TcpNetworkedInternalData;
    if (hostName)
    {
        m_data->m_hostName = hostName;
    }
    m_data->m_port = port;
}

TcpNetworkedPhysicsProcessor::~TcpNetworkedPhysicsProcessor()
{
    disconnect();
    delete m_data;
}

bool TcpNetworkedPhysicsProcessor::processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    if (gVerboseNetworkMessagesClient2)
    {
        printf("PhysicsClientTCP::processCommand\n");
    }

    {
        i32 sz = 0;
        u8* data = 0;
        m_data->m_tempBuffer.clear();

        if (clientCmd.m_type == CMD_STEP_FORWARD_SIMULATION)
        {
            sz = sizeof(i32);
            data = (u8*)&clientCmd.m_type;
        }
        else
        {
            if (clientCmd.m_type == CMD_REQUEST_VR_EVENTS_DATA)
            {
                sz = 3 * sizeof(i32) + sizeof(smUint64_t) + 16;
                data = (u8*)&clientCmd;
            }
            else
            {
                sz = sizeof(SharedMemoryCommand);
                data = (u8*)&clientCmd;
            }
        }

        m_data->m_tcpSocket.Send((u8k*)data, sz);
    }

    return false;
}

bool TcpNetworkedPhysicsProcessor::receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = m_data->checkData();

    if (hasStatus)
    {
        if (gVerboseNetworkMessagesClient2)
        {
            printf("TcpNetworkedPhysicsProcessor::receiveStatus\n");
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

void TcpNetworkedPhysicsProcessor::renderScene(i32 renderFlags)
{
}

void TcpNetworkedPhysicsProcessor::physicsDebugDraw(i32 debugDrawFlags)
{
}

void TcpNetworkedPhysicsProcessor::setGuiHelper(struct GUIHelperInterface* guiHelper)
{
}

bool TcpNetworkedPhysicsProcessor::isConnected() const
{
    return m_data->m_isConnected;
}

bool TcpNetworkedPhysicsProcessor::connect()
{
    bool isConnected = m_data->connectTCP();
    return isConnected;
}

void TcpNetworkedPhysicsProcessor::disconnect()
{
    const char msg[16] = "disconnect";
    m_data->m_tcpSocket.Send((u8k*)msg, 10);
    m_data->m_tcpSocket.Close();
    m_data->m_isConnected = false;
}

void TcpNetworkedPhysicsProcessor::setTimeOut(double timeOutInSeconds)
{
    m_data->m_timeOutInSeconds = timeOutInSeconds;
}
