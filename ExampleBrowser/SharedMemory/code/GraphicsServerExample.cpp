#include "../GraphicsServerExample.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/SharedMemory/PosixSharedMemory.h>
#include <drx3D/SharedMemory/Win32SharedMemory.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/SharedMemory/GraphicsSharedMemoryBlock.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/MT/ThreadSupportInterface.h>
#include <drx3D/Common/b3Clock.h>

//#ifdef DRX3D_ENABLE_CLSOCKET
#include <drx3D/Network2/ActiveSocket.h>
#include <drx3D/Network2/PassiveSocket.h>
#include <drx3D/Network2/SimpleSocket.h> // Include header for active socket object definition
#include <stdio.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/b3CommandLineArgs.h>
#include <drx3D/SharedMemory/RemoteGUIHelper.h>
#include <drx3D/SharedMemory/GraphicsSharedMemoryPublic.h>
#include <drx3D/SharedMemory/GraphicsSharedMemoryCommands.h>


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

void submitStatus(CActiveSocket* pClient, GraphicsSharedMemoryStatus& serverStatus, b3AlignedObjectArray<char>& buffer)
{
    b3AlignedObjectArray<u8> packetData;
    u8* statBytes = (u8*)&serverStatus;


    //create packetData with [i32 packetSizeInBytes, status, streamBytes)
    packetData.resize(4 + sizeof(GraphicsSharedMemoryStatus) + serverStatus.m_numDataStreamBytes);
    i32 sz = packetData.size();
    i32 curPos = 0;

    if (gVerboseNetworkMessagesServer)
    {
        printf("buffer.size = %d\n", buffer.size());
        printf("serverStatus packed size = %d\n", sz);
    }

    MySerializeInt(sz, &packetData[curPos]);
    curPos += 4;
    for (i32 i = 0; i < sizeof(GraphicsSharedMemoryStatus); i++)
    {
        packetData[i + curPos] = statBytes[i];
    }
    curPos += sizeof(GraphicsSharedMemoryStatus);
    if (gVerboseNetworkMessagesServer)
     printf("serverStatus.m_numDataStreamBytes=%d\n", serverStatus.m_numDataStreamBytes);
    for (i32 i = 0; i < serverStatus.m_numDataStreamBytes; i++)
    {
        packetData[i + curPos] = buffer[i];
    }

    pClient->Send(&packetData[0], packetData.size());
    if (gVerboseNetworkMessagesServer)
        printf("pClient->Send serverStatus: %d\n", packetData.size());
}


//#endif //DRX3D_ENABLE_CLSOCKET


#define MAX_GRAPHICS_SHARED_MEMORY_BLOCKS 1


struct TCPArgs
{
    TCPArgs()
        : m_cs(0),
        m_port(6667),
        m_numClientCommands(0),
        m_numServerCommands(0),
        m_cmdPtr(0)
    {
        m_dataSlots.resize(10);
    }

    void submitCommand()
    {
        m_cs->lock();
        m_serverStatus.m_type = GFX_CMD_CLIENT_COMMAND_FAILED;
        m_serverStatus.m_numDataStreamBytes = 0;
        Assert(m_numServerCommands == m_numClientCommands);
        m_numClientCommands++;
        m_cs->unlock();
    }
    void processCommand()
    {
        m_cs->lock();
        Assert(m_numServerCommands == (m_numClientCommands - 1));
        m_numServerCommands++;
        m_cs->unlock();
    }
    bool isCommandOutstanding()
    {
        m_cs->lock();
        bool result = m_numClientCommands > m_numServerCommands;
        m_cs->unlock();
        return result;
    }


    b3CriticalSection* m_cs;
    i32 m_port;
    b3AlignedObjectArray< b3AlignedObjectArray<u8> > m_dataSlots;
    i32 m_numClientCommands;
    i32 m_numServerCommands;
    GraphicsSharedMemoryCommand* m_cmdPtr;
    GraphicsSharedMemoryStatus m_serverStatus;
};

struct TCPThreadLocalStorage
{
    i32 threadId;
};


enum TCPCommunicationEnums
{
    eTCPRequestTerminate = 11,
    eTCPIsUnInitialized,
    eTCPIsInitialized,
    eTCPInitializationFailed,
    eTCPHasTerminated
};

void TCPThreadFunc(uk userPtr, uk lsMemory)
{
    printf("TCPThreadFunc thread started\n");

    TCPArgs* args = (TCPArgs*)userPtr;
    //i32 workLeft = true;
    b3Clock clock;
    clock.reset();
    b3Clock sleepClock;
    bool init = true;
    if (init)
    {
        u32 cachedSharedParam = eTCPIsInitialized;

        args->m_cs->lock();
        args->m_cs->setSharedParam(0, eTCPIsInitialized);
        args->m_cs->unlock();

        double deltaTimeInSeconds = 0;
        i32 numCmdSinceSleep1ms = 0;
        zu64 prevTime = clock.getTimeMicroseconds();

//#ifdef DRX3D_ENABLE_CLSOCKET
        b3Clock clock;
        double timeOutInSeconds = 10;


        bool isPhysicsClientConnected = true;
        bool exitRequested = false;


        if (!isPhysicsClientConnected)
        {
            printf("TCP thread error connecting to shared memory. Machine needs a reboot?\n");
        }
        Assert(isPhysicsClientConnected);

        printf("Starting TCP server using port %d\n", args->m_port);

        CPassiveSocket socket;
        CActiveSocket* pClient = NULL;

        //--------------------------------------------------------------------------
        // Initialize our socket object
        //--------------------------------------------------------------------------
        socket.Initialize();

        socket.Listen("localhost", args->m_port);

        socket.SetBlocking();

        i32 curNumErr = 0;

//#endif

        do
        {

            {
                b3Clock::usleep(0);
            }
            ///////////////////////////////

//#ifdef DRX3D_ENABLE_CLSOCKET

            {
                b3Clock::usleep(0);

                if ((pClient = socket.Accept()) != NULL)
                {
                    socket.SetReceiveTimeout(60, 0);// (1, 0);
                    socket.SetSendTimeout(60, 0);

                    b3AlignedObjectArray<char> bytesReceived;

                    i32 clientPort = socket.GetClientPort();
                    if (gVerboseNetworkMessagesServer)
                        printf("connected from %s:%d\n", socket.GetClientAddr(), clientPort);

                    if (pClient->Receive(4))
                    {
                        i32 clientKey = *(i32*)pClient->GetData();

                        if (clientKey == GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER)
                        {
                            printf("Client version OK %d\n", clientKey);
                        }
                        else
                        {
                            printf("Server version (%d) mismatches Client Version (%d)\n", GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER, clientKey);
                            continue;
                        }
                    }

                    //----------------------------------------------------------------------
                    // Receive request from the client.
                    //----------------------------------------------------------------------

                    while (cachedSharedParam != eTCPRequestTerminate)
                    {

                        bool receivedData = false;

                        i32 maxLen = 4 + sizeof(GraphicsSharedMemoryCommand) + GRAPHICS_SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE;


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
                                    ///printf("TCP Connection error = %d, curNumErr = %d\n", (i32)err, curNumErr);

                                }
                            }

                            curNumErr = 0;
                            tuk msg2 = (tuk)pClient->GetData();
                            i32 numBytesRec2 = pClient->GetBytesReceived();
                            if (gVerboseNetworkMessagesServer)
                              printf("numBytesRec2=%d\n", numBytesRec2);
                            if (numBytesRec2 < 0)
                            {
                                numBytesRec2 = 0;
                            }
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


                                }

                                if (gVerboseNetworkMessagesServer)
                                {
                                    printf("received message length [%d]\n", numBytesRec);
                                }

                                receivedData = true;


                                args->m_cmdPtr = 0;


                                i32 type = *(i32*)&bytesReceived[0];


                                if (numBytesRec == sizeof(GraphicsSharedMemoryCommand))
                                {
                                    args->m_cmdPtr = (GraphicsSharedMemoryCommand*)&bytesReceived[0];
                                }

                                if (args->m_cmdPtr)
                                {

                                    b3AlignedObjectArray<char> buffer;
                                    buffer.resize(GRAPHICS_SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE);
                                    bool hasStatus = true;
                                    if (gVerboseNetworkMessagesServer)
                                        printf("processing command:");
                                    switch (args->m_cmdPtr->m_type)
                                    {
                                    case GFX_CMD_0:
                                    {
                                        i32 axis = args->m_cmdPtr->m_upAxisYCommand.m_enableUpAxisY ? 1 : 2;
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        bool done = false;
                                        //guiHelper.setUpAxis(axis);


                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_0\n");
                                        break;
                                    }

                                    case GFX_CMD_SET_VISUALIZER_FLAG:
                                    {
                                        //disable single step rendering for GraphicsServer
                                        if (args->m_cmdPtr->m_visualizerFlagCommand.m_visualizerFlag == COV_ENABLE_SINGLE_STEP_RENDERING)
                                        {
                                            args->m_cmdPtr->m_visualizerFlagCommand.m_visualizerFlag = 0;
                                        }
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        //guiHelper.setVisualizerFlag(
                                        //  args->m_cmdPtr->m_visualizerFlagCommand.m_visualizerFlag,
                                        //  args->m_cmdPtr->m_visualizerFlagCommand.m_enable);
                                        //serverStatus.m_type = GFX_CMD_CLIENT_COMMAND_COMPLETED;
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_SET_VISUALIZER_FLAG\n");
                                        break;
                                    }
                                    case GFX_CMD_UPLOAD_DATA:
                                    {
                                        i32 slot = args->m_cmdPtr->m_uploadDataCommand.m_dataSlot;

                                        i32 numBytes = args->m_cmdPtr->m_uploadDataCommand.m_numBytes;


                                        submitStatus(pClient, args->m_serverStatus, buffer);

                                        //now receive numBytes
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_UPLOAD_DATA receiving data\n");
                                        i32 received = 0;
                                        i32 offset = 0;
                                        args->m_dataSlots[slot].resize(numBytes);
                                        while (received < numBytes)
                                        {
                                            if (pClient->Receive(args->m_cmdPtr->m_uploadDataCommand.m_numBytes))
                                            {
                                                //heuristic to detect disconnected clients
                                                CSimpleSocket::CSocketError err = pClient->GetSocketError();
                                                if (err != CSimpleSocket::SocketSuccess || !pClient->IsSocketValid())
                                                {
                                                    curNumErr++;
                                                    //printf("TCP Connection error = %d, curNumErr = %d\n", (i32)err, curNumErr);
                                                }
                                                tuk msg2 = (tuk)pClient->GetData();
                                                i32 numBytesRec2 = pClient->GetBytesReceived();
                                                if (gVerboseNetworkMessagesServer)
                                                    printf("received %d bytes (total=%d)\n", numBytesRec2, received);

                                                for (i32 i = 0; i < numBytesRec2; i++)
                                                {
                                                    args->m_dataSlots[slot][i+ offset] = msg2[i];
                                                }
                                                offset += numBytesRec2;
                                                received += numBytesRec2;
                                            }
                                        }
                                        if (gVerboseNetworkMessagesServer)
                                            printf("received all bytes!\n");
                                        args->m_serverStatus.m_type = GFX_CMD_CLIENT_COMMAND_COMPLETED;
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_UPLOAD_DATA\n");
                                        break;
                                    }
                                    case GFX_CMD_REGISTER_TEXTURE:
                                    {
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        //u8k* texels = (u8k*)&args->m_dataSlots[0][0];
                                        //args->m_serverStatus.m_registerTextureStatus.m_textureId = guiHelper.registerTexture(texels, args->m_cmdPtr->m_registerTextureCommand.m_width,
                                        //  args->m_cmdPtr->m_registerTextureCommand.m_height);
                                        //serverStatus.m_type = GFX_CMD_REGISTER_TEXTURE_COMPLETED;
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_REGISTER_TEXTURE\n");
                                        break;
                                    }
                                    case GFX_CMD_REGISTER_GRAPHICS_SHAPE:
                                    {
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_REGISTER_GRAPHICS_SHAPE\n");
                                        break;
                                    }
                                    case GFX_CMD_REGISTER_GRAPHICS_INSTANCE:
                                    {
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }

                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_REGISTER_GRAPHICS_INSTANCE\n");
                                        break;
                                    }
                                    case GFX_CMD_SYNCHRONIZE_TRANSFORMS:
                                    {
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_SYNCHRONIZE_TRANSFORMS\n");
                                        break;
                                    }
                                    case GFX_CMD_REMOVE_ALL_GRAPHICS_INSTANCES:
                                    {
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_REMOVE_ALL_GRAPHICS_INSTANCES\n");
                                        break;
                                    }
                                    case GFX_CMD_REMOVE_SINGLE_GRAPHICS_INSTANCE:
                                    {
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_REMOVE_SINGLE_GRAPHICS_INSTANCE\n");
                                        break;
                                    }
                                    case GFX_CMD_CHANGE_RGBA_COLOR:
                                    {
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_CHANGE_RGBA_COLOR\n");
                                        break;
                                    }
                                    case GFX_CMD_CHANGE_SCALING:
                                    {
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_CHANGE_SCALING\n");
                                        break;
                                    }

                                    case GFX_CMD_GET_CAMERA_INFO:
                                    {
                                        args->submitCommand();
                                        while (args->isCommandOutstanding())
                                        {
                                            clock.usleep(0);
                                        }
                                        //bool RemoteGUIHelper::getCameraInfo(i32* width, i32* height,
                                        //  float viewMatrix[16], float projectionMatrix[16],
                                        //  float camUp[3], float camForward[3], float hor[3], float vert[3],
                                        //  float* yaw, float* pitch, float* camDist, float camTarget[3]) const
#if 0
                                        guiHelper.getCameraInfo(&serverStatus.m_getCameraInfoStatus.width,
                                            &serverStatus.m_getCameraInfoStatus.height,
                                            serverStatus.m_getCameraInfoStatus.viewMatrix,
                                            serverStatus.m_getCameraInfoStatus.projectionMatrix,
                                            serverStatus.m_getCameraInfoStatus.camUp,
                                            serverStatus.m_getCameraInfoStatus.camForward,
                                            serverStatus.m_getCameraInfoStatus.hor,
                                            serverStatus.m_getCameraInfoStatus.vert,
                                            &serverStatus.m_getCameraInfoStatus.yaw,
                                            &serverStatus.m_getCameraInfoStatus.pitch,
                                            &serverStatus.m_getCameraInfoStatus.camDist,
                                            serverStatus.m_getCameraInfoStatus.camTarget);
                                        serverStatus.m_type = GFX_CMD_GET_CAMERA_INFO_COMPLETED;
#endif
                                        if (gVerboseNetworkMessagesServer)
                                            printf("GFX_CMD_GET_CAMERA_INFO\n");
                                        break;
                                    }

                                    case GFX_CMD_INVALID:
                                    case GFX_CMD_MAX_CLIENT_COMMANDS:
                                    default:
                                    {
                                        printf("UNKNOWN COMMAND!\n");
                                        Assert(0);
                                        hasStatus = false;
                                    }
                                    }


                                    double startTimeSeconds = clock.getTimeInSeconds();
                                    double curTimeSeconds = clock.getTimeInSeconds();

                                    if (gVerboseNetworkMessagesServer)
                                    {
                                        //printf("buffer.size = %d\n", buffer.size());
                                        printf("serverStatus.m_numDataStreamBytes = %d\n", args->m_serverStatus.m_numDataStreamBytes);
                                    }
                                    if (hasStatus)
                                    {
                                        submitStatus(pClient, args->m_serverStatus, buffer);
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

//#endif //DRX3D_ENABLE_CLSOCKET


            ///////////////////////////////
            args->m_cs->lock();
            cachedSharedParam = args->m_cs->getSharedParam(0);
            args->m_cs->unlock();

        } while (cachedSharedParam != eTCPRequestTerminate);
//#ifdef DRX3D_ENABLE_CLSOCKET
        socket.Close();
        socket.Shutdown(CSimpleSocket::Both);
//#endif
    }
    else
    {
        args->m_cs->lock();
        args->m_cs->setSharedParam(0, eTCPInitializationFailed);
        args->m_cs->unlock();
    }

    printf("TCPThreadFunc thread exit\n");
    //do nothing
}

uk TCPlsMemoryFunc()
{
    //don't create local store memory, just return 0
    return new TCPThreadLocalStorage;
}

void TCPlsMemoryReleaseFunc(uk ptr)
{
    TCPThreadLocalStorage* p = (TCPThreadLocalStorage*)ptr;
    delete p;
}


#ifndef _WIN32
#include <drx3D/MT/PosixThreadSupport.h>

ThreadSupportInterface* createTCPThreadSupport(i32 numThreads)
{
    PosixThreadSupport::ThreadConstructionInfo constructionInfo("TCPThreads",
        TCPThreadFunc,
        TCPlsMemoryFunc,
        TCPlsMemoryReleaseFunc,
        numThreads);
    ThreadSupportInterface* threadSupport = new PosixThreadSupport(constructionInfo);

    return threadSupport;
}

#elif defined(_WIN32)
#include "../MultiThreading/b3Win32ThreadSupport.h"

ThreadSupportInterface* createTCPThreadSupport(i32 numThreads)
{
    b3Win32ThreadSupport::Win32ThreadConstructionInfo threadConstructionInfo("TCPThreads", TCPThreadFunc, TCPlsMemoryFunc, TCPlsMemoryReleaseFunc, numThreads);
    b3Win32ThreadSupport* threadSupport = new b3Win32ThreadSupport(threadConstructionInfo);
    return threadSupport;
}
#endif


class GraphicsServerExample : public CommonExampleInterface
{
    CommonGraphicsApp* m_app;
    GUIHelperInterface* m_guiHelper;

    bool m_verboseOutput;

    float m_x;
    float m_y;
    float m_z;

    ThreadSupportInterface* m_threadSupport;
    TCPArgs m_args;

public:
    GraphicsServerExample(GUIHelperInterface* guiHelper)
        : m_guiHelper(guiHelper),
          m_x(0),
          m_y(0),
          m_z(0)
    {
        m_verboseOutput = true;

        m_app = guiHelper->getAppInterface();
        m_app->setUpAxis(2);

        m_threadSupport = createTCPThreadSupport(1);
        m_args.m_cs = m_threadSupport->createCriticalSection();
        m_args.m_cs->setSharedParam(0, eTCPIsUnInitialized);
        m_threadSupport->runTask(D3_THREAD_SCHEDULE_TASK, (uk )&this->m_args, 0);

        bool isUninitialized = true;

        while (isUninitialized)
        {
            m_args.m_cs->lock();
            isUninitialized = (m_args.m_cs->getSharedParam(0) == eTCPIsUnInitialized);
            m_args.m_cs->unlock();
#ifdef _WIN32
            b3Clock::usleep(1000);
#endif
    }

    }
    virtual ~GraphicsServerExample()
    {

        m_args.m_cs->setSharedParam(0, eTCPRequestTerminate);

        i32 numActiveThreads = 1;
        while (numActiveThreads)
        {
            i32 arg0, arg1;
            if (m_threadSupport->isTaskCompleted(&arg0, &arg1, 0))
            {
                numActiveThreads--;
                printf("numActiveThreads = %d\n", numActiveThreads);
            }
            else
            {
                b3Clock::usleep(0);
            }
        };

        m_threadSupport->deleteCriticalSection(m_args.m_cs);

        delete m_threadSupport;
        m_threadSupport = 0;

    }

    virtual void initPhysics()
    {
    }
    virtual void exitPhysics()
    {
    }

    void submitServerStatus(GraphicsSharedMemoryStatus& status, i32 blockIndex)
    {
    }

    bool processCommand(const struct GraphicsSharedMemoryCommand& clientCmd, struct GraphicsSharedMemoryStatus& serverStatusOut)
    {
        //printf("processed command of type:%d\n", clientCmd.m_type);
        D3_PROFILE("processCommand");
        switch (clientCmd.m_type)
        {
            case GFX_CMD_0:
            {
                //either Y or Z can be up axis
                i32 upAxis = (clientCmd.m_upAxisYCommand.m_enableUpAxisY) ? 1 : 2;
                m_guiHelper->setUpAxis(upAxis);
                serverStatusOut.m_type = GFX_CMD_CLIENT_COMMAND_COMPLETED;
                m_args.processCommand();
                break;
            }
            case GFX_CMD_SET_VISUALIZER_FLAG:
            {
                if ((clientCmd.m_visualizerFlagCommand.m_visualizerFlag != COV_ENABLE_RENDERING) &&
                    (clientCmd.m_visualizerFlagCommand.m_visualizerFlag != COV_ENABLE_SINGLE_STEP_RENDERING))
                {
                    //printf("clientCmd.m_visualizerFlag.m_visualizerFlag: %d, clientCmd.m_visualizerFlag.m_enable %d\n",
                    //  clientCmd.m_visualizerFlagCommand.m_visualizerFlag, clientCmd.m_visualizerFlagCommand.m_enable);

                    this->m_guiHelper->setVisualizerFlag(clientCmd.m_visualizerFlagCommand.m_visualizerFlag, clientCmd.m_visualizerFlagCommand.m_enable);
                }
                m_args.processCommand();
                break;

            }

            case GFX_CMD_UPLOAD_DATA:
            {
#if 0
                //printf("uploadData command: curSize=%d, offset=%d, slot=%d", clientCmd.m_uploadDataCommand.m_numBytes, clientCmd.m_uploadDataCommand.m_dataOffset, clientCmd.m_uploadDataCommand.m_dataSlot);
                i32 dataSlot = clientCmd.m_uploadDataCommand.m_dataSlot;
                i32 dataOffset = clientCmd.m_uploadDataCommand.m_dataOffset;
                m_dataSlots.resize(dataSlot + 1);
                Assert(m_dataSlots[dataSlot].size() >= dataOffset);
                m_dataSlots[dataSlot].resize(clientCmd.m_uploadDataCommand.m_numBytes + clientCmd.m_uploadDataCommand.m_dataOffset);
                for (i32 i = 0; i < clientCmd.m_uploadDataCommand.m_numBytes; i++)
                {
                    m_dataSlots[dataSlot][dataOffset + i] = bufferServerToClient[i];
                }
#endif
                break;
            }
            case GFX_CMD_REGISTER_TEXTURE:
            {

                i32 dataSlot = 0;
                i32 sizeData = m_args.m_dataSlots[dataSlot].size();
                Assert(sizeData > 0);
                serverStatusOut.m_type = GFX_CMD_REGISTER_TEXTURE_FAILED;
                if (sizeData)
                {
                    u8* texels = &m_args.m_dataSlots[dataSlot][0];
                    i32 textureId = this->m_guiHelper->registerTexture(texels, clientCmd.m_registerTextureCommand.m_width, clientCmd.m_registerTextureCommand.m_height);
                    serverStatusOut.m_type = GFX_CMD_REGISTER_TEXTURE_COMPLETED;
                    serverStatusOut.m_registerTextureStatus.m_textureId = textureId;
                }

                m_args.processCommand();
                break;
            }

            case GFX_CMD_REGISTER_GRAPHICS_SHAPE:
            {
                i32 verticesSlot = 0;
                i32 indicesSlot = 1;
                serverStatusOut.m_type = GFX_CMD_REGISTER_GRAPHICS_SHAPE_FAILED;
                const float* vertices = (const float*)&m_args.m_dataSlots[verticesSlot][0];
                i32k* indices = (i32k*)&m_args.m_dataSlots[indicesSlot][0];
                i32 numVertices = clientCmd.m_registerGraphicsShapeCommand.m_numVertices;
                i32 numIndices = clientCmd.m_registerGraphicsShapeCommand.m_numIndices;
                i32 primitiveType = clientCmd.m_registerGraphicsShapeCommand.m_primitiveType;
                i32 textureId = clientCmd.m_registerGraphicsShapeCommand.m_textureId;
                i32 shapeId = this->m_guiHelper->registerGraphicsShape(vertices, numVertices, indices, numIndices, primitiveType, textureId);
                serverStatusOut.m_registerGraphicsShapeStatus.m_shapeId = shapeId;
                serverStatusOut.m_type = GFX_CMD_REGISTER_GRAPHICS_SHAPE_COMPLETED;
                m_args.processCommand();
                break;
            }

            case GFX_CMD_REGISTER_GRAPHICS_INSTANCE:
            {
                i32 graphicsInstanceId = m_guiHelper->registerGraphicsInstance(clientCmd.m_registerGraphicsInstanceCommand.m_shapeIndex,
                    clientCmd.m_registerGraphicsInstanceCommand.m_position,
                    clientCmd.m_registerGraphicsInstanceCommand.m_quaternion,
                    clientCmd.m_registerGraphicsInstanceCommand.m_color,
                    clientCmd.m_registerGraphicsInstanceCommand.m_scaling);
                serverStatusOut.m_registerGraphicsInstanceStatus.m_graphicsInstanceId = graphicsInstanceId;
                serverStatusOut.m_type = GFX_CMD_REGISTER_GRAPHICS_INSTANCE_COMPLETED;
                m_args.processCommand();
                break;
            }
            case GFX_CMD_SYNCHRONIZE_TRANSFORMS:
            {
                GUISyncPosition* positions = (GUISyncPosition*)&m_args.m_dataSlots[0][0];
                for (i32 i = 0; i < clientCmd.m_syncTransformsCommand.m_numPositions; i++)
                {
                    m_app->m_renderer->writeSingleInstanceTransformToCPU(positions[i].m_pos, positions[i].m_orn, positions[i].m_graphicsInstanceId);
                }
                m_args.processCommand();
                break;
            }
            case GFX_CMD_REMOVE_ALL_GRAPHICS_INSTANCES:
            {
                m_guiHelper->removeAllGraphicsInstances();
                m_args.processCommand();
                break;
            }
            case GFX_CMD_REMOVE_SINGLE_GRAPHICS_INSTANCE:
            {
                m_app->m_renderer->removeGraphicsInstance(clientCmd.m_removeGraphicsInstanceCommand.m_graphicsUid);
                m_args.processCommand();
                break;
            }
            case GFX_CMD_CHANGE_RGBA_COLOR:
            {
                m_guiHelper->changeRGBAColor(clientCmd.m_changeRGBAColorCommand.m_graphicsUid, clientCmd.m_changeRGBAColorCommand.m_rgbaColor);
                m_args.processCommand();
                break;
            }

            case GFX_CMD_CHANGE_SCALING:
            {
                m_guiHelper->changeScaling(clientCmd.m_changeScalingCommand.m_graphicsUid, clientCmd.m_changeScalingCommand.m_scaling);
                m_args.processCommand();
                break;
            }

            case GFX_CMD_GET_CAMERA_INFO:
            {
                serverStatusOut.m_type = GFX_CMD_GET_CAMERA_INFO_FAILED;

                if (m_guiHelper->getCameraInfo(
                    &serverStatusOut.m_getCameraInfoStatus.width,
                    &serverStatusOut.m_getCameraInfoStatus.height,
                    serverStatusOut.m_getCameraInfoStatus.viewMatrix,
                    serverStatusOut.m_getCameraInfoStatus.projectionMatrix,
                    serverStatusOut.m_getCameraInfoStatus.camUp,
                    serverStatusOut.m_getCameraInfoStatus.camForward,
                    serverStatusOut.m_getCameraInfoStatus.hor,
                    serverStatusOut.m_getCameraInfoStatus.vert,
                    &serverStatusOut.m_getCameraInfoStatus.yaw,
                    &serverStatusOut.m_getCameraInfoStatus.pitch,
                    &serverStatusOut.m_getCameraInfoStatus.camDist,
                    serverStatusOut.m_getCameraInfoStatus.camTarget))
                {
                    serverStatusOut.m_type = GFX_CMD_GET_CAMERA_INFO_COMPLETED;
                }
                m_args.processCommand();

                break;
            }
            default:
            {
                printf("unsupported command:%d\n", clientCmd.m_type);
            }
        }
        return true;
    }

    void processClientCommands()
    {
        i32 timeStamp = 0;
        bool hasStatus = false;
        if (m_args.isCommandOutstanding())
        {
            processCommand(*m_args.m_cmdPtr, m_args.m_serverStatus);
        }
        //serverStatusOut.m_type = GFX_CMD_CLIENT_COMMAND_FAILED;
        //bool hasStatus = processCommand(clientCmd, serverStatusOut);
        if (hasStatus)
        {
            //submitServerStatus(serverStatusOut);
        }
    }

    virtual void stepSimulation(float deltaTime)
    {
        D3_PROFILE("stepSimulation");
        processClientCommands();
        m_x += 0.01f;
        m_y += 0.01f;
        m_z += 0.01f;
    }


    virtual void renderScene()
    {
        D3_PROFILE("renderScene");
        {

            D3_PROFILE("writeTransforms");
            m_app->m_renderer->writeTransforms();
        }
        {
            D3_PROFILE("m_renderer->renderScene");
            m_app->m_renderer->renderScene();
        }

    }



    virtual void physicsDebugDraw(i32 debugDrawFlags)
    {

    }

    virtual bool mouseMoveCallback(float x, float y)
    {
        return false;
    }
    virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
    {
        return false;
    }
    virtual bool keyboardCallback(i32 key, i32 state)
    {
        return false;
    }

    virtual void resetCamera()
    {
        float dist = 3.5;
        float pitch = -32;
        float yaw = 136;
        float targetPos[3] = {0, 0, 0};
        if (m_app->m_renderer && m_app->m_renderer->getActiveCamera())
        {
            m_app->m_renderer->getActiveCamera()->setCameraDistance(dist);
            m_app->m_renderer->getActiveCamera()->setCameraPitch(pitch);
            m_app->m_renderer->getActiveCamera()->setCameraYaw(yaw);
            m_app->m_renderer->getActiveCamera()->setCameraTargetPosition(targetPos[0], targetPos[1], targetPos[2]);
        }
    }


};







CommonExampleInterface* GraphicsServerCreateFuncBullet(struct CommonExampleOptions& options)
{
    return new GraphicsServerExample(options.m_guiHelper);
}
