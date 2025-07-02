#include <drx3D/SharedMemory/PhysicsServerCommandProcessor.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Plugins/b3PluginCollisionInterface.h>
#include <drx3D/Importers/URDF/UrdfImporter.h>
#include <drx3D/Importers/URDF/MyMultiBodyCreator.h>
#include <drx3D/Importers/URDF/URDF2Bullet.h>
#include <drx3D/Importers/URDF/UrdfFindMeshFile.h>
#include <drx3D/Physics/Dynamics/Inverse/MultiBodyTreeCreator.h>
#include <drx3D/Physics/Collision/Dispatch/InternalEdgeUtility.h>
#include <drx3D/Importers/MeshUtility/b3ImportMeshUtility.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/DantzigSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/SolveProjectedGaussSeidel.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolver.h>

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyMLCPConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySphericalJointMotor.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointLimitConstraint.h>

//#define USE_DISCRETE_DYNAMICS_WORLD
//#define SKIP_DEFORMABLE_BODY
//#define SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD

#include <drx3D/Common/DefaultFileIO.h>
#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyPoint2Point.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointFeedback.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyFixedConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyGearConstraint.h>
#include <drx3D/Importers/URDF/UrdfParser.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/b3FileUtils.h>
#include <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySliderConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyPoint2Point.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
#include <drx3D/Common/b3HashMap.h>
#include <drx3D/Common/ChromeTraceUtil.h>
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <X/stb/stb_image.h>
#include <drx3D/Physics/Dynamics/Inverse/MultiBodyTree.h>
#include <drx3D/SharedMemory/IKTrajectoryHelper.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Common/RobotLoggingUtil.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Importers/MJCF/MJCFImporter.h>
#include <drx3D/Importers/Obj/LoadMeshFromObj.h>
#include <drx3D/Importers/STL/LoadMeshFromSTL.h>
#include <drx3D/Importers/Bullet/MultiBodyWorldImporter.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointMotor.h>
#include <drx3D/Maths/Linear/Serializer.h>
#include <drx3D/Common/b3Logging.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/SharedMemory/SharedMemoryCommands.h>
#include <drx3D/Maths/Linear/Random.h>
#include <drx3D/Common/b3ResizablePool.h>
#include <drx3D/Common/b3Clock.h>
#include <drx3D/Plugins/b3PluginManager.h>
#include <drx3D/Importers/BulletFile.h>
#include <drx3D/Physics/Collision/NarrowPhase/RaycastCallback.h>
#include <drx3D/Maths/Linear/TaskScheduler/ThreadSupportInterface.h>
#include <drx3D/Wavefront/tiny_obj_loader.h>
#ifndef SKIP_COLLISION_FILTER_PLUGIN
#include <drx3D/Plugins/collisionFilterPlugin/collisionFilterPlugin.h>
#endif

#ifdef ENABLE_STATIC_GRPC_PLUGIN
#include <drx3D/Plugins/grpcPlugin/grpcPlugin.h>
#endif  //ENABLE_STATIC_GRPC_PLUGIN

#ifndef SKIP_STATIC_PD_CONTROL_PLUGIN
#include <drx3D/Plugins/pdControlPlugin/pdControlPlugin.h>
#endif  //SKIP_STATIC_PD_CONTROL_PLUGIN

#ifdef STATIC_LINK_SPD_PLUGIN
#include <drx3D/Plugins/stablePDPlugin/BulletConversion.h>
#include <drx3D/Plugins/stablePDPlugin/RBDModel.h>
#include <drx3D/Plugins/stablePDPlugin/RBDUtil.h>
#endif

#ifdef STATIC_LINK_VR_PLUGIN
#include <drx3D/Plugins/vrSyncPlugin/vrSyncPlugin.h>
#endif

#ifdef STATIC_EGLRENDERER_PLUGIN
#include <drx3D/Plugins/eglPlugin/eglRendererPlugin.h>
#endif  //STATIC_EGLRENDERER_PLUGIN

#ifndef SKIP_STATIC_TINYRENDERER_PLUGIN
#include <drx3D/Plugins/tinyRendererPlugin/tinyRendererPlugin.h>
#endif

#ifdef D3_ENABLE_FILEIO_PLUGIN
#include <drx3D/Plugins/fileIOPlugin/fileIOPlugin.h>
#endif  //D3_DISABLE_FILEIO_PLUGIN

#ifdef D3_ENABLE_TINY_AUDIO
#include <drx3D/Audio/Tiny/SoundEngine.h>
#endif

#ifdef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
#define SKIP_DEFORMABLE_BODY 1
#endif

#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/SoftBody/SoftBodySolvers.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Physics/SoftBody/SoftMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyConstraintSolver.h>
#include <drx3D/ExampleBrowser/SoftDemo/BunnyMesh.h>
#endif  //SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD

#ifndef SKIP_DEFORMABLE_BODY
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyConstraintSolver.h>

#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBody.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBodyHelpers.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBodySolver.h>
#endif  //SKIP_DEFORMABLE_BODY

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>

i32 gInternalSimFlags = 0;
bool gResetSimulation = 0;
i32 gVRTrackingObjectUniqueId = -1;
i32 gVRTrackingObjectFlag = VR_CAMERA_TRACK_OBJECT_ORIENTATION;

Transform2 gVRTrackingObjectTr = Transform2::getIdentity();

Vec3 gVRTeleportPos1(0, 0, 0);
Quat gVRTeleportOrn(0, 0, 0, 1);

Scalar simTimeScalingFactor = 1;
Scalar gRhsClamp = 1.f;

#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>

class b3ThreadPool
{
public:
    b3ThreadPool(tukk name = "b3ThreadPool")
    {
        ThreadSupportInterface::ConstructionInfo info(name, threadFunction);
        m_threadSupportInterface = ThreadSupportInterface::create(info);
    }

    ~b3ThreadPool()
    {
        delete m_threadSupportInterface;
    }

    i32k numWorkers() const { return m_threadSupportInterface->getNumWorkerThreads(); }

    void runTask(i32 threadIdx, ThreadSupportInterface::ThreadFunc func, uk arg)
    {
        FunctionContext& ctx = m_functionContexts[threadIdx];
        ctx.func = func;
        ctx.arg = arg;
        m_threadSupportInterface->runTask(threadIdx, (uk )&ctx);
    }

    void waitForAllTasks()
    {
        DRX3D_PROFILE("b3ThreadPool_waitForAllTasks");
        m_threadSupportInterface->waitForAllTasks();
    }

private:
    struct FunctionContext
    {
        ThreadSupportInterface::ThreadFunc func;
        uk arg;
    };

    static void threadFunction(uk userPtr)
    {
        DRX3D_PROFILE("b3ThreadPool_threadFunction");
        FunctionContext* ctx = (FunctionContext*)userPtr;
        ctx->func(ctx->arg);
    }

    ThreadSupportInterface* m_threadSupportInterface;
    FunctionContext m_functionContexts[DRX3D_MAX_THREAD_COUNT];
};

struct SharedMemoryDebugDrawer : public IDebugDraw
{
    i32 m_debugMode;
    AlignedObjectArray<SharedMemLines> m_lines2;

    SharedMemoryDebugDrawer()
        : m_debugMode(0)
    {
    }
    virtual void drawContactPoint(const Vec3& PointOnB, const Vec3& normalOnB, Scalar distance, i32 lifeTime, const Vec3& color)
    {
    }

    virtual void reportErrorWarning(tukk warningString)
    {
    }

    virtual void draw3dText(const Vec3& location, tukk textString)
    {
    }

    virtual void setDebugMode(i32 debugMode)
    {
        m_debugMode = debugMode;
    }

    virtual i32 getDebugMode() const
    {
        return m_debugMode;
    }
    virtual void drawLine(const Vec3& from, const Vec3& to, const Vec3& color)
    {
        SharedMemLines line;
        line.m_from = from;
        line.m_to = to;
        line.m_color = color;
        m_lines2.push_back(line);
    }
};

struct InternalVisualShapeData
{
    i32 m_tinyRendererVisualShapeIndex;
    i32 m_OpenGLGraphicsIndex;

    b3AlignedObjectArray<UrdfVisual> m_visualShapes;

    b3AlignedObjectArray<STxt> m_pathPrefixes;

    virtual ~InternalVisualShapeData()
    {
        clear();
    }
    void clear()
    {
        m_tinyRendererVisualShapeIndex = -1;
        m_OpenGLGraphicsIndex = -1;
        m_visualShapes.clear();
        m_pathPrefixes.clear();
    }
};

struct InternalCollisionShapeData
{
    CollisionShape* m_collisionShape;
    b3AlignedObjectArray<UrdfCollision> m_urdfCollisionObjects;
    i32 m_used;
    InternalCollisionShapeData()
        : m_collisionShape(0),
          m_used(0)
    {
    }

    virtual ~InternalCollisionShapeData()
    {
        clear();
    }
    void clear()
    {
        m_urdfCollisionObjects.clear();
        m_collisionShape = 0;
        m_used = 0;
    }
};

#include <drx3D/SharedMemory/SharedMemoryUserData.h>

struct InternalBodyData
{
    MultiBody* m_multiBody;
    RigidBody* m_rigidBody;
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    SoftBody* m_softBody;

#endif
    i32 m_testData;
    STxt m_bodyName;

    Transform2 m_rootLocalInertialFrame;
    AlignedObjectArray<Transform2> m_linkLocalInertialFrames;
    AlignedObjectArray<Generic6DofSpring2Constraint*> m_rigidBodyJoints;
    AlignedObjectArray<STxt> m_rigidBodyJointNames;
    AlignedObjectArray<STxt> m_rigidBodyLinkNames;
    AlignedObjectArray<i32> m_userDataHandles;

#ifdef D3_ENABLE_TINY_AUDIO
    b3HashMap<HashInt, SDFAudioSource> m_audioSources;
#endif  //D3_ENABLE_TINY_AUDIO

    InternalBodyData()
    {
        clear();
    }

    void clear()
    {
        m_multiBody = 0;
        m_rigidBody = 0;
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        m_softBody = 0;

#endif
        m_testData = 0;
        m_bodyName = "";
        m_rootLocalInertialFrame.setIdentity();
        m_linkLocalInertialFrames.clear();
        m_rigidBodyJoints.clear();
        m_rigidBodyJointNames.clear();
        m_rigidBodyLinkNames.clear();
        m_userDataHandles.clear();
    }
};

struct InteralUserConstraintData
{
    TypedConstraint* m_rbConstraint;
    MultiBodyConstraint* m_mbConstraint;

    b3UserConstraint m_userConstraintData;

    i32 m_sbHandle;
    i32 m_sbNodeIndex;
    Scalar m_sbNodeMass;

    InteralUserConstraintData()
        : m_rbConstraint(0),
          m_mbConstraint(0),
          m_sbHandle(-1),
          m_sbNodeIndex(-1),
          m_sbNodeMass(-1)
    {
    }
};

struct InternalTextureData
{
    i32 m_tinyRendererTextureId;
    i32 m_openglTextureId;
    void clear()
    {
        m_tinyRendererTextureId = -1;
        m_openglTextureId = -1;
    }
};

typedef b3PoolBodyHandle<InternalTextureData> InternalTextureHandle;
typedef b3PoolBodyHandle<InternalBodyData> InternalBodyHandle;
typedef b3PoolBodyHandle<InternalCollisionShapeData> InternalCollisionShapeHandle;
typedef b3PoolBodyHandle<InternalVisualShapeData> InternalVisualShapeHandle;

class CommandChunk
{
public:
    i32 m_chunkCode;
    i32 m_length;
    uk m_oldPtr;
    i32 m_dna_nr;
    i32 m_number;
};

class bCommandChunkPtr4
{
public:
    bCommandChunkPtr4() {}
    i32 code;
    i32 len;
    union {
        i32 m_uniqueInt;
    };
    i32 dna_nr;
    i32 nr;
};

// ----------------------------------------------------- //
class bCommandChunkPtr8
{
public:
    bCommandChunkPtr8() {}
    i32 code, len;
    union {
        i32 m_uniqueInts[2];
    };
    i32 dna_nr, nr;
};

struct CommandLogger
{
    FILE* m_file;

    void writeHeader(u8* buffer) const
    {
#ifdef DRX3D_USE_DOUBLE_PRECISION
        memcpy(buffer, "BT3CMDd", 7);
#else
        memcpy(buffer, "BT3CMDf", 7);
#endif  //DRX3D_USE_DOUBLE_PRECISION

        i32 littleEndian = 1;
        littleEndian = ((tuk)&littleEndian)[0];

        if (sizeof(uk ) == 8)
        {
            buffer[7] = '-';
        }
        else
        {
            buffer[7] = '_';
        }

        if (littleEndian)
        {
            buffer[8] = 'v';
        }
        else
        {
            buffer[8] = 'V';
        }

        buffer[9] = 0;
        buffer[10] = 0;
        buffer[11] = 0;

        i32 ver = GetVersion();
        if (ver >= 0 && ver < 999)
        {
            sprintf((tuk)&buffer[9], "%d", ver);
        }
    }

    void logCommand(const SharedMemoryCommand& command)
    {
        if (m_file)
        {
            CommandChunk chunk;
            chunk.m_chunkCode = command.m_type;
            chunk.m_oldPtr = 0;
            chunk.m_dna_nr = 0;
            chunk.m_length = sizeof(SharedMemoryCommand);
            chunk.m_number = 1;
            fwrite((tukk)&chunk, sizeof(CommandChunk), 1, m_file);

            switch (command.m_type)
            {
                case CMD_LOAD_MJCF:
                {
                    fwrite((tukk)&command.m_updateFlags, sizeof(i32), 1, m_file);
                    fwrite((tukk)&command.m_mjcfArguments, sizeof(MjcfArgs), 1, m_file);
                    break;
                }
                case CMD_REQUEST_BODY_INFO:
                {
                    fwrite((tukk)&command.m_updateFlags, sizeof(i32), 1, m_file);
                    fwrite((tukk)&command.m_sdfRequestInfoArgs, sizeof(SdfRequestInfoArgs), 1, m_file);
                    break;
                }
                case CMD_REQUEST_VISUAL_SHAPE_INFO:
                {
                    fwrite((tukk)&command.m_updateFlags, sizeof(i32), 1, m_file);
                    fwrite((tukk)&command.m_requestVisualShapeDataArguments, sizeof(RequestVisualShapeDataArgs), 1, m_file);
                    break;
                }
                case CMD_LOAD_URDF:
                {
                    fwrite((tukk)&command.m_updateFlags, sizeof(i32), 1, m_file);
                    fwrite((tukk)&command.m_urdfArguments, sizeof(UrdfArgs), 1, m_file);
                    break;
                }
                case CMD_INIT_POSE:
                {
                    fwrite((tukk)&command.m_updateFlags, sizeof(i32), 1, m_file);
                    fwrite((tukk)&command.m_initPoseArgs, sizeof(InitPoseArgs), 1, m_file);
                    break;
                };
                case CMD_REQUEST_ACTUAL_STATE:
                {
                    fwrite((tukk)&command.m_updateFlags, sizeof(i32), 1, m_file);
                    fwrite((tukk)&command.m_requestActualStateInformationCommandArgument,
                           sizeof(RequestActualStateArgs), 1, m_file);
                    break;
                };
                case CMD_SEND_DESIRED_STATE:
                {
                    fwrite((tukk)&command.m_updateFlags, sizeof(i32), 1, m_file);
                    fwrite((tukk)&command.m_sendDesiredStateCommandArgument, sizeof(SendDesiredStateArgs), 1, m_file);
                    break;
                }
                case CMD_SEND_PHYSICS_SIMULATION_PARAMETERS:
                {
                    fwrite((tukk)&command.m_updateFlags, sizeof(i32), 1, m_file);
                    fwrite((tukk)&command.m_physSimParamArgs, sizeof(b3PhysicsSimulationParameters), 1, m_file);
                    break;
                }
                case CMD_REQUEST_CONTACT_POINT_INFORMATION:
                {
                    fwrite((tukk)&command.m_updateFlags, sizeof(i32), 1, m_file);
                    fwrite((tukk)&command.m_requestContactPointArguments, sizeof(RequestContactDataArgs), 1, m_file);
                    break;
                }
                case CMD_STEP_FORWARD_SIMULATION:
                case CMD_RESET_SIMULATION:
                case CMD_REQUEST_INTERNAL_DATA:
                {
                    break;
                };
                default:
                {
                    fwrite((tukk)&command, sizeof(SharedMemoryCommand), 1, m_file);
                }
            };
        }
    }

    CommandLogger(tukk fileName)
    {
        m_file = fopen(fileName, "wb");
        if (m_file)
        {
            u8 buf[15];
            buf[12] = 12;
            buf[13] = 13;
            buf[14] = 14;
            writeHeader(buf);
            fwrite(buf, 12, 1, m_file);
        }
    }
    virtual ~CommandLogger()
    {
        if (m_file)
        {
            fclose(m_file);
        }
    }
};

struct CommandLogPlayback
{
    u8 m_header[12];
    FILE* m_file;
    bool m_bitsVary;
    bool m_fileIs64bit;

    CommandLogPlayback(tukk fileName)
    {
        m_file = fopen(fileName, "rb");
        if (m_file)
        {
            size_t bytesRead;
            bytesRead = fread(m_header, 12, 1, m_file);
        }
        u8 c = m_header[7];
        m_fileIs64bit = (c == '-');

        const bool VOID_IS_8 = ((sizeof(uk ) == 8));
        m_bitsVary = (VOID_IS_8 != m_fileIs64bit);
    }
    virtual ~CommandLogPlayback()
    {
        if (m_file)
        {
            fclose(m_file);
            m_file = 0;
        }
    }
    bool processNextCommand(SharedMemoryCommand* cmd)
    {
//for a little while, keep this flag to be able to read 'old' log files
//#define BACKWARD_COMPAT
#if BACKWARD_COMPAT
        SharedMemoryCommand unused;
#endif  //BACKWARD_COMPAT
        bool result = false;
        size_t s = 0;
        if (m_file)
        {
            i32 commandType = -1;

            if (m_fileIs64bit)
            {
                bCommandChunkPtr8 chunk8;
                s = fread((uk )&chunk8, sizeof(bCommandChunkPtr8), 1, m_file);
                commandType = chunk8.code;
            }
            else
            {
                bCommandChunkPtr4 chunk4;
                s = fread((uk )&chunk4, sizeof(bCommandChunkPtr4), 1, m_file);
                commandType = chunk4.code;
            }

            if (s == 1)
            {
                memset(cmd, 0, sizeof(SharedMemoryCommand));
                cmd->m_type = commandType;

#ifdef BACKWARD_COMPAT
                s = fread(&unused, sizeof(SharedMemoryCommand), 1, m_file);
                cmd->m_updateFlags = unused.m_updateFlags;
#endif

                switch (commandType)
                {
                    case CMD_LOAD_MJCF:
                    {
#ifdef BACKWARD_COMPAT
                        cmd->m_mjcfArguments = unused.m_mjcfArguments;
#else
                        s = fread(&cmd->m_updateFlags, sizeof(i32), 1, m_file);
                        s = fread(&cmd->m_mjcfArguments, sizeof(MjcfArgs), 1, m_file);
#endif
                        result = true;
                        break;
                    }
                    case CMD_REQUEST_BODY_INFO:
                    {
#ifdef BACKWARD_COMPAT
                        cmd->m_sdfRequestInfoArgs = unused.m_sdfRequestInfoArgs;
#else
                        s = fread(&cmd->m_updateFlags, sizeof(i32), 1, m_file);
                        s = fread(&cmd->m_sdfRequestInfoArgs, sizeof(SdfRequestInfoArgs), 1, m_file);
#endif
                        result = true;
                        break;
                    }
                    case CMD_REQUEST_VISUAL_SHAPE_INFO:
                    {
#ifdef BACKWARD_COMPAT
                        cmd->m_requestVisualShapeDataArguments = unused.m_requestVisualShapeDataArguments;
#else
                        s = fread(&cmd->m_updateFlags, sizeof(i32), 1, m_file);
                        s = fread(&cmd->m_requestVisualShapeDataArguments, sizeof(RequestVisualShapeDataArgs), 1, m_file);
#endif
                        result = true;
                        break;
                    }
                    case CMD_LOAD_URDF:
                    {
#ifdef BACKWARD_COMPAT
                        cmd->m_urdfArguments = unused.m_urdfArguments;
#else
                        s = fread(&cmd->m_updateFlags, sizeof(i32), 1, m_file);
                        s = fread(&cmd->m_urdfArguments, sizeof(UrdfArgs), 1, m_file);
#endif
                        result = true;
                        break;
                    }
                    case CMD_INIT_POSE:
                    {
#ifdef BACKWARD_COMPAT
                        cmd->m_initPoseArgs = unused.m_initPoseArgs;
#else
                        s = fread(&cmd->m_updateFlags, sizeof(i32), 1, m_file);
                        s = fread(&cmd->m_initPoseArgs, sizeof(InitPoseArgs), 1, m_file);

#endif
                        result = true;
                        break;
                    };
                    case CMD_REQUEST_ACTUAL_STATE:
                    {
#ifdef BACKWARD_COMPAT
                        cmd->m_requestActualStateInformationCommandArgument = unused.m_requestActualStateInformationCommandArgument;
#else
                        s = fread(&cmd->m_updateFlags, sizeof(i32), 1, m_file);
                        s = fread(&cmd->m_requestActualStateInformationCommandArgument, sizeof(RequestActualStateArgs), 1, m_file);
#endif
                        result = true;
                        break;
                    };
                    case CMD_SEND_DESIRED_STATE:
                    {
#ifdef BACKWARD_COMPAT
                        cmd->m_sendDesiredStateCommandArgument = unused.m_sendDesiredStateCommandArgument;
#else
                        s = fread(&cmd->m_updateFlags, sizeof(i32), 1, m_file);
                        s = fread(&cmd->m_sendDesiredStateCommandArgument, sizeof(SendDesiredStateArgs), 1, m_file);

#endif
                        result = true;
                        break;
                    }
                    case CMD_SEND_PHYSICS_SIMULATION_PARAMETERS:
                    {
#ifdef BACKWARD_COMPAT
                        cmd->m_physSimParamArgs = unused.m_physSimParamArgs;
#else
                        s = fread(&cmd->m_updateFlags, sizeof(i32), 1, m_file);
                        s = fread(&cmd->m_physSimParamArgs, sizeof(b3PhysicsSimulationParameters), 1, m_file);

#endif
                        result = true;
                        break;
                    }
                    case CMD_REQUEST_CONTACT_POINT_INFORMATION:
                    {
#ifdef BACKWARD_COMPAT
                        cmd->m_requestContactPointArguments = unused.m_requestContactPointArguments;
#else
                        s = fread(&cmd->m_updateFlags, sizeof(i32), 1, m_file);
                        s = fread(&cmd->m_requestContactPointArguments, sizeof(RequestContactDataArgs), 1, m_file);

#endif
                        result = true;
                        break;
                    }
                    case CMD_STEP_FORWARD_SIMULATION:
                    case CMD_RESET_SIMULATION:
                    case CMD_REQUEST_INTERNAL_DATA:
                    {
                        result = true;
                        break;
                    }
                    default:
                    {
                        s = fread(cmd, sizeof(SharedMemoryCommand), 1, m_file);
                        result = (s == 1);
                    }
                };
            }
        }
        return result;
    }
};

struct SaveWorldObjectData
{
    b3AlignedObjectArray<i32> m_bodyUniqueIds;
    STxt m_fileName;
};

struct MyBroadphaseCallback : public BroadphaseAabbCallback
{
    b3AlignedObjectArray<i32> m_bodyUniqueIds;
    b3AlignedObjectArray<i32> m_links;

    MyBroadphaseCallback()
    {
    }
    virtual ~MyBroadphaseCallback()
    {
    }
    void clear()
    {
        m_bodyUniqueIds.clear();
        m_links.clear();
    }
    virtual bool process(const BroadphaseProxy* proxy)
    {
        CollisionObject2* colObj = (CollisionObject2*)proxy->m_clientObject;
        MultiBodyLinkCollider* mbl = MultiBodyLinkCollider::upcast(colObj);
        if (mbl)
        {
            i32 bodyUniqueId = mbl->m_multiBody->getUserIndex2();
            m_bodyUniqueIds.push_back(bodyUniqueId);
            m_links.push_back(mbl->m_link);
            return true;
        }
        i32 bodyUniqueId = colObj->getUserIndex2();
        if (bodyUniqueId >= 0)
        {
            m_bodyUniqueIds.push_back(bodyUniqueId);
            //it is not a multibody, so use -1 otherwise
            m_links.push_back(-1);
        }
        return true;
    }
};

struct MyOverlapFilterCallback : public OverlapFilterCallback
{
    i32 m_filterMode;
    b3PluginManager* m_pluginManager;

    MyOverlapFilterCallback(b3PluginManager* pluginManager)
        : m_filterMode(D3_FILTER_GROUPAMASKB_AND_GROUPBMASKA),
          m_pluginManager(pluginManager)
    {
    }

    virtual ~MyOverlapFilterCallback()
    {
    }
    // return true when pairs need collision
    virtual bool needBroadphaseCollision(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1) const
    {
        b3PluginCollisionInterface* collisionInterface = m_pluginManager->getCollisionInterface();

        if (collisionInterface && collisionInterface->getNumRules())
        {
            i32 objectUniqueIdB = -1, linkIndexB = -1;
            CollisionObject2* colObjB = (CollisionObject2*)proxy1->m_clientObject;
            MultiBodyLinkCollider* mblB = MultiBodyLinkCollider::upcast(colObjB);
            if (mblB)
            {
                objectUniqueIdB = mblB->m_multiBody->getUserIndex2();
                linkIndexB = mblB->m_link;
            }
            else
            {
                objectUniqueIdB = colObjB->getUserIndex2();
                linkIndexB = -1;
            }
            i32 objectUniqueIdA = -1, linkIndexA = -1;
            CollisionObject2* colObjA = (CollisionObject2*)proxy0->m_clientObject;
            MultiBodyLinkCollider* mblA = MultiBodyLinkCollider::upcast(colObjA);
            if (mblA)
            {
                objectUniqueIdA = mblA->m_multiBody->getUserIndex2();
                linkIndexA = mblA->m_link;
            }
            else
            {
                objectUniqueIdA = colObjA->getUserIndex2();
                linkIndexA = -1;
            }
            i32 collisionFilterGroupA = proxy0->m_collisionFilterGroup;
            i32 collisionFilterMaskA = proxy0->m_collisionFilterMask;
            i32 collisionFilterGroupB = proxy1->m_collisionFilterGroup;
            i32 collisionFilterMaskB = proxy1->m_collisionFilterMask;

            return collisionInterface->needsBroadphaseCollision(objectUniqueIdA, linkIndexA,
                                                                collisionFilterGroupA, collisionFilterMaskA,
                                                                objectUniqueIdB, linkIndexB, collisionFilterGroupB, collisionFilterMaskB, m_filterMode);
        }
        else
        {
            if (m_filterMode == D3_FILTER_GROUPAMASKB_AND_GROUPBMASKA)
            {
                bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
                collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);
                return collides;
            }

            if (m_filterMode == D3_FILTER_GROUPAMASKB_OR_GROUPBMASKA)
            {
                bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
                collides = collides || (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);
                return collides;
            }
            return false;
        }
    }
};

struct InternalStateLogger
{
    i32 m_loggingUniqueId;
    i32 m_loggingType;

    InternalStateLogger()
        : m_loggingUniqueId(0),
          m_loggingType(0)
    {
    }
    virtual ~InternalStateLogger() {}

    virtual void stop() = 0;
    virtual void logState(Scalar timeStep) = 0;
};

struct VideoMP4Loggger : public InternalStateLogger
{
    struct GUIHelperInterface* m_guiHelper;
    STxt m_fileName;
    VideoMP4Loggger(i32 loggerUid, tukk fileName, GUIHelperInterface* guiHelper)
        : m_guiHelper(guiHelper)
    {
        m_fileName = fileName;
        m_loggingUniqueId = loggerUid;
        m_loggingType = STATE_LOGGING_VIDEO_MP4;
        m_guiHelper->dumpFramesToVideo(fileName);
    }

    virtual void stop()
    {
        m_guiHelper->dumpFramesToVideo(0);
    }
    virtual void logState(Scalar timeStep)
    {
        //dumping video frames happens in another thread
        //we could add some overlay of timestamp here, if needed/wanted
    }
};

struct MinitaurStateLogger : public InternalStateLogger
{
    i32 m_loggingTimeStamp;
    STxt m_fileName;
    i32 m_minitaurBodyUniqueId;
    FILE* m_logFileHandle;

    STxt m_structTypes;
    MultiBody* m_minitaurMultiBody;
    AlignedObjectArray<i32> m_motorIdList;

    MinitaurStateLogger(i32 loggingUniqueId, const STxt& fileName, MultiBody* minitaurMultiBody, AlignedObjectArray<i32>& motorIdList)
        : m_loggingTimeStamp(0),
          m_logFileHandle(0),
          m_minitaurMultiBody(minitaurMultiBody)
    {
        m_loggingUniqueId = loggingUniqueId;
        m_loggingType = STATE_LOGGING_MINITAUR;
        m_motorIdList.resize(motorIdList.size());
        for (i32 m = 0; m < motorIdList.size(); m++)
        {
            m_motorIdList[m] = motorIdList[m];
        }

        AlignedObjectArray<STxt> structNames;
        //'t', 'r', 'p', 'y', 'q0', 'q1', 'q2', 'q3', 'q4', 'q5', 'q6', 'q7', 'u0', 'u1', 'u2', 'u3', 'u4', 'u5', 'u6', 'u7', 'xd', 'mo'
        structNames.push_back("t");
        structNames.push_back("r");
        structNames.push_back("p");
        structNames.push_back("y");

        structNames.push_back("q0");
        structNames.push_back("q1");
        structNames.push_back("q2");
        structNames.push_back("q3");
        structNames.push_back("q4");
        structNames.push_back("q5");
        structNames.push_back("q6");
        structNames.push_back("q7");

        structNames.push_back("u0");
        structNames.push_back("u1");
        structNames.push_back("u2");
        structNames.push_back("u3");
        structNames.push_back("u4");
        structNames.push_back("u5");
        structNames.push_back("u6");
        structNames.push_back("u7");

        structNames.push_back("dx");
        structNames.push_back("mo");

        m_structTypes = "IffffffffffffffffffffB";
        tukk fileNameC = fileName.c_str();

        m_logFileHandle = createMinitaurLogFile(fileNameC, structNames, m_structTypes);
    }
    virtual void stop()
    {
        if (m_logFileHandle)
        {
            closeMinitaurLogFile(m_logFileHandle);
            m_logFileHandle = 0;
        }
    }

    virtual void logState(Scalar timeStep)
    {
        if (m_logFileHandle)
        {
            //Vec3 pos = m_minitaurMultiBody->getBasePos();

            MinitaurLogRecord logData;
            //'t', 'r', 'p', 'y', 'q0', 'q1', 'q2', 'q3', 'q4', 'q5', 'q6', 'q7', 'u0', 'u1', 'u2', 'u3', 'u4', 'u5', 'u6', 'u7', 'xd', 'mo'
            Scalar motorDir[8] = {1, 1, 1, 1, 1, 1, 1, 1};

            Quat orn = m_minitaurMultiBody->getBaseWorldTransform().getRotation();
            Matrix3x3 mat(orn);
            Scalar roll = 0;
            Scalar pitch = 0;
            Scalar yaw = 0;

            mat.getEulerZYX(yaw, pitch, roll);

            logData.m_values.push_back(m_loggingTimeStamp);
            logData.m_values.push_back((float)roll);
            logData.m_values.push_back((float)pitch);
            logData.m_values.push_back((float)yaw);

            for (i32 i = 0; i < 8; i++)
            {
                float jointAngle = (float)motorDir[i] * m_minitaurMultiBody->getJointPos(m_motorIdList[i]);
                logData.m_values.push_back(jointAngle);
            }
            for (i32 i = 0; i < 8; i++)
            {
                MultiBodyJointMotor* motor = (MultiBodyJointMotor*)m_minitaurMultiBody->getLink(m_motorIdList[i]).m_userPtr;

                if (motor && timeStep > Scalar(0))
                {
                    Scalar force = motor->getAppliedImpulse(0) / timeStep;
                    logData.m_values.push_back((float)force);
                }
            }
            //x is forward component, estimated speed forward
            float xd_speed = m_minitaurMultiBody->getBaseVel()[0];
            logData.m_values.push_back(xd_speed);
            char mode = 6;
            logData.m_values.push_back(mode);

            //at the moment, appendMinitaurLogData will directly write to disk (potential delay)
            //better to fill a huge memory buffer and once in a while write it to disk
            appendMinitaurLogData(m_logFileHandle, m_structTypes, logData);

            fflush(m_logFileHandle);

            m_loggingTimeStamp++;
        }
    }
};

struct b3VRControllerEvents
{
    b3VRControllerEvent m_vrEvents[MAX_VR_CONTROLLERS];

    b3VRControllerEvents()
    {
        init();
    }

    virtual ~b3VRControllerEvents()
    {
    }

    void init()
    {
        for (i32 i = 0; i < MAX_VR_CONTROLLERS; i++)
        {
            m_vrEvents[i].m_deviceType = 0;
            m_vrEvents[i].m_numButtonEvents = 0;
            m_vrEvents[i].m_numMoveEvents = 0;
            for (i32 b = 0; b < MAX_VR_BUTTONS; b++)
            {
                m_vrEvents[i].m_buttons[b] = 0;
            }
        }
    }

    void addNewVREvents(const struct b3VRControllerEvent* vrEvents, i32 numVREvents)
    {
        //update m_vrEvents
        for (i32 i = 0; i < numVREvents; i++)
        {
            i32 controlledId = vrEvents[i].m_controllerId;
            if (vrEvents[i].m_numMoveEvents)
            {
                m_vrEvents[controlledId].m_analogAxis = vrEvents[i].m_analogAxis;
                for (i32 a = 0; a < 10; a++)
                {
                    m_vrEvents[controlledId].m_auxAnalogAxis[a] = vrEvents[i].m_auxAnalogAxis[a];
                }
            }
            else
            {
                m_vrEvents[controlledId].m_analogAxis = 0;
                for (i32 a = 0; a < 10; a++)
                {
                    m_vrEvents[controlledId].m_auxAnalogAxis[a] = 0;
                }
            }
            if (vrEvents[i].m_numMoveEvents + vrEvents[i].m_numButtonEvents)
            {
                m_vrEvents[controlledId].m_controllerId = vrEvents[i].m_controllerId;
                m_vrEvents[controlledId].m_deviceType = vrEvents[i].m_deviceType;

                m_vrEvents[controlledId].m_pos[0] = vrEvents[i].m_pos[0];
                m_vrEvents[controlledId].m_pos[1] = vrEvents[i].m_pos[1];
                m_vrEvents[controlledId].m_pos[2] = vrEvents[i].m_pos[2];
                m_vrEvents[controlledId].m_orn[0] = vrEvents[i].m_orn[0];
                m_vrEvents[controlledId].m_orn[1] = vrEvents[i].m_orn[1];
                m_vrEvents[controlledId].m_orn[2] = vrEvents[i].m_orn[2];
                m_vrEvents[controlledId].m_orn[3] = vrEvents[i].m_orn[3];
            }

            m_vrEvents[controlledId].m_numButtonEvents += vrEvents[i].m_numButtonEvents;
            m_vrEvents[controlledId].m_numMoveEvents += vrEvents[i].m_numMoveEvents;
            for (i32 b = 0; b < MAX_VR_BUTTONS; b++)
            {
                m_vrEvents[controlledId].m_buttons[b] |= vrEvents[i].m_buttons[b];
                if (vrEvents[i].m_buttons[b] & eButtonIsDown)
                {
                    m_vrEvents[controlledId].m_buttons[b] |= eButtonIsDown;
                }
                else
                {
                    m_vrEvents[controlledId].m_buttons[b] &= ~eButtonIsDown;
                }
            }
        }
    };
};

struct VRControllerStateLogger : public InternalStateLogger
{
    b3VRControllerEvents m_vrEvents;
    i32 m_loggingTimeStamp;
    i32 m_deviceTypeFilter;
    STxt m_fileName;
    FILE* m_logFileHandle;
    STxt m_structTypes;

    VRControllerStateLogger(i32 loggingUniqueId, i32 deviceTypeFilter, const STxt& fileName)
        : m_loggingTimeStamp(0),
          m_deviceTypeFilter(deviceTypeFilter),
          m_fileName(fileName),
          m_logFileHandle(0)
    {
        m_loggingUniqueId = loggingUniqueId;
        m_loggingType = STATE_LOGGING_VR_CONTROLLERS;

        AlignedObjectArray<STxt> structNames;
        structNames.push_back("stepCount");
        structNames.push_back("timeStamp");
        structNames.push_back("controllerId");
        structNames.push_back("numMoveEvents");
        structNames.push_back("m_numButtonEvents");
        structNames.push_back("posX");
        structNames.push_back("posY");
        structNames.push_back("posZ");
        structNames.push_back("oriX");
        structNames.push_back("oriY");
        structNames.push_back("oriZ");
        structNames.push_back("oriW");
        structNames.push_back("analogAxis");
        structNames.push_back("buttons0");
        structNames.push_back("buttons1");
        structNames.push_back("buttons2");
        structNames.push_back("buttons3");
        structNames.push_back("buttons4");
        structNames.push_back("buttons5");
        structNames.push_back("buttons6");
        structNames.push_back("deviceType");
        m_structTypes = "IfIIIffffffffIIIIIIII";

        tukk fileNameC = fileName.c_str();
        m_logFileHandle = createMinitaurLogFile(fileNameC, structNames, m_structTypes);
    }
    virtual void stop()
    {
        if (m_logFileHandle)
        {
            closeMinitaurLogFile(m_logFileHandle);
            m_logFileHandle = 0;
        }
    }
    virtual void logState(Scalar timeStep)
    {
        if (m_logFileHandle)
        {
            i32 stepCount = m_loggingTimeStamp;
            float timeStamp = m_loggingTimeStamp * timeStep;

            for (i32 i = 0; i < MAX_VR_CONTROLLERS; i++)
            {
                b3VRControllerEvent& event = m_vrEvents.m_vrEvents[i];
                if (m_deviceTypeFilter & event.m_deviceType)
                {
                    if (event.m_numButtonEvents + event.m_numMoveEvents)
                    {
                        MinitaurLogRecord logData;

                        //serverStatusOut.m_sendVREvents.m_controllerEvents[serverStatusOut.m_sendVREvents.m_numVRControllerEvents++] = event;
                        //log the event
                        logData.m_values.push_back(stepCount);
                        logData.m_values.push_back(timeStamp);
                        logData.m_values.push_back(event.m_controllerId);
                        logData.m_values.push_back(event.m_numMoveEvents);
                        logData.m_values.push_back(event.m_numButtonEvents);
                        logData.m_values.push_back(event.m_pos[0]);
                        logData.m_values.push_back(event.m_pos[1]);
                        logData.m_values.push_back(event.m_pos[2]);
                        logData.m_values.push_back(event.m_orn[0]);
                        logData.m_values.push_back(event.m_orn[1]);
                        logData.m_values.push_back(event.m_orn[2]);
                        logData.m_values.push_back(event.m_orn[3]);
                        logData.m_values.push_back(event.m_analogAxis);
                        i32 packedButtons[7] = {0, 0, 0, 0, 0, 0, 0};

                        i32 packedButtonIndex = 0;
                        i32 packedButtonShift = 0;
                        //encode the 64 buttons into 7 i32 (3 bits each), each i32 stores 10 buttons
                        for (i32 b = 0; b < MAX_VR_BUTTONS; b++)
                        {
                            i32 buttonMask = event.m_buttons[b];
                            buttonMask = buttonMask << (packedButtonShift * 3);
                            packedButtons[packedButtonIndex] |= buttonMask;
                            packedButtonShift++;

                            if (packedButtonShift >= 10)
                            {
                                packedButtonShift = 0;
                                packedButtonIndex++;
                                if (packedButtonIndex >= 7)
                                {
                                    Assert(0);
                                    break;
                                }
                            }
                        }

                        for (i32 b = 0; b < 7; b++)
                        {
                            logData.m_values.push_back(packedButtons[b]);
                        }
                        logData.m_values.push_back(event.m_deviceType);
                        appendMinitaurLogData(m_logFileHandle, m_structTypes, logData);

                        event.m_numButtonEvents = 0;
                        event.m_numMoveEvents = 0;
                        for (i32 b = 0; b < MAX_VR_BUTTONS; b++)
                        {
                            event.m_buttons[b] = 0;
                        }
                    }
                }
            }

            fflush(m_logFileHandle);
            m_loggingTimeStamp++;
        }
    }
};

struct GenericRobotStateLogger : public InternalStateLogger
{
    float m_loggingTimeStamp;
    STxt m_fileName;
    FILE* m_logFileHandle;
    STxt m_structTypes;
    const MultiBodyDynamicsWorld* m_dynamicsWorld;
    AlignedObjectArray<i32> m_bodyIdList;
    bool m_filterObjectUniqueId;
    i32 m_maxLogDof;
    i32 m_logFlags;

    GenericRobotStateLogger(i32 loggingUniqueId, const STxt& fileName, const MultiBodyDynamicsWorld* dynamicsWorld, i32 maxLogDof, i32 logFlags)
        : m_loggingTimeStamp(0),
          m_logFileHandle(0),
          m_dynamicsWorld(dynamicsWorld),
          m_filterObjectUniqueId(false),
          m_maxLogDof(maxLogDof),
          m_logFlags(logFlags)
    {
        m_loggingUniqueId = loggingUniqueId;
        m_loggingType = STATE_LOGGING_GENERIC_ROBOT;

        AlignedObjectArray<STxt> structNames;
        structNames.push_back("stepCount");
        structNames.push_back("timeStamp");
        structNames.push_back("objectId");
        structNames.push_back("posX");
        structNames.push_back("posY");
        structNames.push_back("posZ");
        structNames.push_back("oriX");
        structNames.push_back("oriY");
        structNames.push_back("oriZ");
        structNames.push_back("oriW");
        structNames.push_back("velX");
        structNames.push_back("velY");
        structNames.push_back("velZ");
        structNames.push_back("omegaX");
        structNames.push_back("omegaY");
        structNames.push_back("omegaZ");
        structNames.push_back("qNum");

        m_structTypes = "IfifffffffffffffI";

        for (i32 i = 0; i < m_maxLogDof; i++)
        {
            m_structTypes.append("f");
            char jointName[256];
            sprintf(jointName, "q%d", i);
            structNames.push_back(jointName);
        }

        for (i32 i = 0; i < m_maxLogDof; i++)
        {
            m_structTypes.append("f");
            char jointName[256];
            sprintf(jointName, "u%d", i);
            structNames.push_back(jointName);
        }

        if (m_logFlags & STATE_LOG_JOINT_TORQUES)
        {
            for (i32 i = 0; i < m_maxLogDof; i++)
            {
                m_structTypes.append("f");
                char jointName[256];
                sprintf(jointName, "t%d", i);
                structNames.push_back(jointName);
            }
        }

        tukk fileNameC = fileName.c_str();

        m_logFileHandle = createMinitaurLogFile(fileNameC, structNames, m_structTypes);
    }
    virtual void stop()
    {
        if (m_logFileHandle)
        {
            closeMinitaurLogFile(m_logFileHandle);
            m_logFileHandle = 0;
        }
    }

    virtual void logState(Scalar timeStep)
    {
        if (m_logFileHandle)
        {
            for (i32 i = 0; i < m_dynamicsWorld->getNumMultibodies(); i++)
            {
                const MultiBody* mb = m_dynamicsWorld->getMultiBody(i);
                i32 objectUniqueId = mb->getUserIndex2();
                if (m_filterObjectUniqueId && m_bodyIdList.findLinearSearch2(objectUniqueId) < 0)
                {
                    continue;
                }

                MinitaurLogRecord logData;
                i32 stepCount = m_loggingTimeStamp;
                float timeStamp = m_loggingTimeStamp * m_dynamicsWorld->getSolverInfo().m_timeStep;
                logData.m_values.push_back(stepCount);
                logData.m_values.push_back(timeStamp);

                Vec3 pos = mb->getBasePos();
                Quat ori = mb->getWorldToBaseRot().inverse();
                Vec3 vel = mb->getBaseVel();
                Vec3 omega = mb->getBaseOmega();

                float posX = pos[0];
                float posY = pos[1];
                float posZ = pos[2];
                float oriX = ori.x();
                float oriY = ori.y();
                float oriZ = ori.z();
                float oriW = ori.w();
                float velX = vel[0];
                float velY = vel[1];
                float velZ = vel[2];
                float omegaX = omega[0];
                float omegaY = omega[1];
                float omegaZ = omega[2];

                logData.m_values.push_back(objectUniqueId);
                logData.m_values.push_back(posX);
                logData.m_values.push_back(posY);
                logData.m_values.push_back(posZ);
                logData.m_values.push_back(oriX);
                logData.m_values.push_back(oriY);
                logData.m_values.push_back(oriZ);
                logData.m_values.push_back(oriW);
                logData.m_values.push_back(velX);
                logData.m_values.push_back(velY);
                logData.m_values.push_back(velZ);
                logData.m_values.push_back(omegaX);
                logData.m_values.push_back(omegaY);
                logData.m_values.push_back(omegaZ);

                i32 numDofs = mb->getNumDofs();
                logData.m_values.push_back(numDofs);
                i32 numJoints = mb->getNumLinks();

                for (i32 j = 0; j < numJoints; ++j)
                {
                    if (mb->getLink(j).m_jointType == 0 || mb->getLink(j).m_jointType == 1)
                    {
                        float q = mb->getJointPos(j);
                        logData.m_values.push_back(q);
                    }
                }
                for (i32 j = numDofs; j < m_maxLogDof; ++j)
                {
                    float q = 0.0;
                    logData.m_values.push_back(q);
                }

                for (i32 j = 0; j < numJoints; ++j)
                {
                    if (mb->getLink(j).m_jointType == 0 || mb->getLink(j).m_jointType == 1)
                    {
                        float v = mb->getJointVel(j);
                        logData.m_values.push_back(v);
                    }
                }
                for (i32 j = numDofs; j < m_maxLogDof; ++j)
                {
                    float v = 0.0;
                    logData.m_values.push_back(v);
                }

                if (m_logFlags & STATE_LOG_JOINT_TORQUES)
                {
                    for (i32 j = 0; j < numJoints; ++j)
                    {
                        if (mb->getLink(j).m_jointType == 0 || mb->getLink(j).m_jointType == 1)
                        {
                            float jointTorque = 0;
                            if (m_logFlags & STATE_LOG_JOINT_MOTOR_TORQUES)
                            {
                                MultiBodyJointMotor* motor = (MultiBodyJointMotor*)mb->getLink(j).m_userPtr;
                                if (motor)
                                {
                                    jointTorque += motor->getAppliedImpulse(0) / timeStep;
                                }
                            }
                            if (m_logFlags & STATE_LOG_JOINT_USER_TORQUES)
                            {
                                if (mb->getLink(j).m_jointType == 0 || mb->getLink(j).m_jointType == 1)
                                {
                                    jointTorque += mb->getJointTorque(j);  //these are the 'user' applied external torques
                                }
                            }
                            logData.m_values.push_back(jointTorque);
                        }
                    }
                    for (i32 j = numDofs; j < m_maxLogDof; ++j)
                    {
                        float u = 0.0;
                        logData.m_values.push_back(u);
                    }
                }

                //at the moment, appendMinitaurLogData will directly write to disk (potential delay)
                //better to fill a huge memory buffer and once in a while write it to disk
                appendMinitaurLogData(m_logFileHandle, m_structTypes, logData);
                fflush(m_logFileHandle);
            }

            m_loggingTimeStamp++;
        }
    }
};
struct ContactPointsStateLogger : public InternalStateLogger
{
    i32 m_loggingTimeStamp;

    STxt m_fileName;
    FILE* m_logFileHandle;
    STxt m_structTypes;
    MultiBodyDynamicsWorld* m_dynamicsWorld;
    bool m_filterLinkA;
    bool m_filterLinkB;
    i32 m_linkIndexA;
    i32 m_linkIndexB;
    i32 m_bodyUniqueIdA;
    i32 m_bodyUniqueIdB;

    ContactPointsStateLogger(i32 loggingUniqueId, const STxt& fileName, MultiBodyDynamicsWorld* dynamicsWorld)
        : m_loggingTimeStamp(0),
          m_fileName(fileName),
          m_logFileHandle(0),
          m_dynamicsWorld(dynamicsWorld),
          m_filterLinkA(false),
          m_filterLinkB(false),
          m_linkIndexA(-2),
          m_linkIndexB(-2),
          m_bodyUniqueIdA(-1),
          m_bodyUniqueIdB(-1)
    {
        m_loggingUniqueId = loggingUniqueId;
        m_loggingType = STATE_LOGGING_CONTACT_POINTS;

        AlignedObjectArray<STxt> structNames;
        structNames.push_back("stepCount");
        structNames.push_back("timeStamp");
        structNames.push_back("contactFlag");
        structNames.push_back("bodyUniqueIdA");
        structNames.push_back("bodyUniqueIdB");
        structNames.push_back("linkIndexA");
        structNames.push_back("linkIndexB");
        structNames.push_back("positionOnAX");
        structNames.push_back("positionOnAY");
        structNames.push_back("positionOnAZ");
        structNames.push_back("positionOnBX");
        structNames.push_back("positionOnBY");
        structNames.push_back("positionOnBZ");
        structNames.push_back("contactNormalOnBX");
        structNames.push_back("contactNormalOnBY");
        structNames.push_back("contactNormalOnBZ");
        structNames.push_back("contactDistance");
        structNames.push_back("normalForce");
        m_structTypes = "IfIiiiifffffffffff";

        tukk fileNameC = fileName.c_str();
        m_logFileHandle = createMinitaurLogFile(fileNameC, structNames, m_structTypes);
    }
    virtual void stop()
    {
        if (m_logFileHandle)
        {
            closeMinitaurLogFile(m_logFileHandle);
            m_logFileHandle = 0;
        }
    }
    virtual void logState(Scalar timeStep)
    {
        if (m_logFileHandle)
        {
            i32 numContactManifolds = m_dynamicsWorld->getDispatcher()->getNumManifolds();
            for (i32 i = 0; i < numContactManifolds; i++)
            {
                const PersistentManifold* manifold = m_dynamicsWorld->getDispatcher()->getInternalManifoldPointer()[i];
                i32 linkIndexA = -1;
                i32 linkIndexB = -1;

                i32 objectIndexB = -1;

                const RigidBody* bodyB = RigidBody::upcast(manifold->getBody1());
                if (bodyB)
                {
                    objectIndexB = bodyB->getUserIndex2();
                }
                const MultiBodyLinkCollider* mblB = MultiBodyLinkCollider::upcast(manifold->getBody1());
                if (mblB && mblB->m_multiBody)
                {
                    linkIndexB = mblB->m_link;
                    objectIndexB = mblB->m_multiBody->getUserIndex2();
                    if (m_filterLinkB && (m_linkIndexB != linkIndexB))
                    {
                        continue;
                    }
                }

                i32 objectIndexA = -1;
                const RigidBody* bodyA = RigidBody::upcast(manifold->getBody0());
                if (bodyA)
                {
                    objectIndexA = bodyA->getUserIndex2();
                }
                const MultiBodyLinkCollider* mblA = MultiBodyLinkCollider::upcast(manifold->getBody0());
                if (mblA && mblA->m_multiBody)
                {
                    linkIndexA = mblA->m_link;
                    objectIndexA = mblA->m_multiBody->getUserIndex2();
                    if (m_filterLinkA && (m_linkIndexA != linkIndexA))
                    {
                        continue;
                    }
                }

                Assert(bodyA || mblA);

                //apply the filter, if the user provides it
                if (m_bodyUniqueIdA >= 0)
                {
                    if ((m_bodyUniqueIdA != objectIndexA) &&
                        (m_bodyUniqueIdA != objectIndexB))
                        continue;
                }

                //apply the second object filter, if the user provides it
                if (m_bodyUniqueIdB >= 0)
                {
                    if ((m_bodyUniqueIdB != objectIndexA) &&
                        (m_bodyUniqueIdB != objectIndexB))
                        continue;
                }

                for (i32 p = 0; p < manifold->getNumContacts(); p++)
                {
                    MinitaurLogRecord logData;
                    i32 stepCount = m_loggingTimeStamp;
                    float timeStamp = m_loggingTimeStamp * timeStep;
                    logData.m_values.push_back(stepCount);
                    logData.m_values.push_back(timeStamp);

                    const ManifoldPoint& srcPt = manifold->getContactPoint(p);

                    logData.m_values.push_back(0);  // reserved contact flag
                    logData.m_values.push_back(objectIndexA);
                    logData.m_values.push_back(objectIndexB);
                    logData.m_values.push_back(linkIndexA);
                    logData.m_values.push_back(linkIndexB);
                    logData.m_values.push_back((float)(srcPt.getPositionWorldOnA()[0]));
                    logData.m_values.push_back((float)(srcPt.getPositionWorldOnA()[1]));
                    logData.m_values.push_back((float)(srcPt.getPositionWorldOnA()[2]));
                    logData.m_values.push_back((float)(srcPt.getPositionWorldOnB()[0]));
                    logData.m_values.push_back((float)(srcPt.getPositionWorldOnB()[1]));
                    logData.m_values.push_back((float)(srcPt.getPositionWorldOnB()[2]));
                    logData.m_values.push_back((float)(srcPt.m_normalWorldOnB[0]));
                    logData.m_values.push_back((float)(srcPt.m_normalWorldOnB[1]));
                    logData.m_values.push_back((float)(srcPt.m_normalWorldOnB[2]));
                    logData.m_values.push_back((float)(srcPt.getDistance()));
                    logData.m_values.push_back((float)(srcPt.getAppliedImpulse() / timeStep));

                    appendMinitaurLogData(m_logFileHandle, m_structTypes, logData);
                    fflush(m_logFileHandle);
                }
            }
            m_loggingTimeStamp++;
        }
    }
};

struct SaveStateData
{
    bParse::BulletFile* m_bulletFile;
    Serializer* m_serializer;
};

struct PhysicsServerCommandProcessorInternalData
{
    ///handle management
    b3ResizablePool<InternalTextureHandle> m_textureHandles;
    b3ResizablePool<InternalBodyHandle> m_bodyHandles;
    b3ResizablePool<InternalCollisionShapeHandle> m_userCollisionShapeHandles;
    b3ResizablePool<InternalVisualShapeHandle> m_userVisualShapeHandles;
    b3ResizablePool<b3PoolBodyHandle<SharedMemoryUserData> > m_userDataHandles;
    HashMap<SharedMemoryUserDataHashKey, i32> m_userDataHandleLookup;

    b3PluginManager m_pluginManager;

    bool m_useRealTimeSimulation;

    b3VRControllerEvents m_vrControllerEvents;

    AlignedObjectArray<SaveStateData> m_savedStates;

    AlignedObjectArray<b3KeyboardEvent> m_keyboardEvents;
    AlignedObjectArray<b3MouseEvent> m_mouseEvents;

    CommandLogger* m_commandLogger;
    i32 m_commandLoggingUid;

    CommandLogPlayback* m_logPlayback;
    i32 m_logPlaybackUid;

    Scalar m_physicsDeltaTime;
    Scalar m_numSimulationSubSteps;
    Scalar m_simulationTimestamp;
    AlignedObjectArray<MultiBodyJointFeedback*> m_multiBodyJointFeedbacks;
    b3HashMap<HashPtr, drx3d_inverse::MultiBodyTree*> m_inverseDynamicsBodies;
    b3HashMap<HashPtr, IKTrajectoryHelper*> m_inverseKinematicsHelpers;

    i32 m_userConstraintUIDGenerator;
    b3HashMap<HashInt, InteralUserConstraintData> m_userConstraints;

    b3AlignedObjectArray<SaveWorldObjectData> m_saveWorldBodyData;

#ifdef USE_DISCRETE_DYNAMICS_WORLD
    AlignedObjectArray<btWorldImporter*> m_worldImporters;
#else
    AlignedObjectArray<MultiBodyWorldImporter*> m_worldImporters;
#endif

    AlignedObjectArray<STxt*> m_strings;

    AlignedObjectArray<CollisionShape*> m_collisionShapes;
    AlignedObjectArray<u8k*> m_heightfieldDatas;
    AlignedObjectArray<i32> m_allocatedTextures;
    AlignedObjectArray<u8*> m_allocatedTexturesRequireFree;
    AlignedObjectArray<double*> m_debugPointsDatas;
    HashMap<HashPtr, UrdfCollision> m_bulletCollisionShape2UrdfCollision;
    AlignedObjectArray<StridingMeshInterface*> m_meshInterfaces;

    MyOverlapFilterCallback* m_broadphaseCollisionFilterCallback;
    HashedOverlappingPairCache* m_pairCache;
    BroadphaseInterface* m_broadphase;
    CollisionDispatcher* m_dispatcher;

#ifdef USE_DISCRETE_DYNAMICS_WORLD
    SequentialImpulseConstraintSolver* m_solver;
#else
    MultiBodyConstraintSolver* m_solver;
#endif

    DefaultCollisionConfiguration* m_collisionConfiguration;

#ifndef SKIP_DEFORMABLE_BODY
    SoftBody* m_pickedSoftBody;
    DeformableMousePickingForce* m_mouseForce;
    Scalar m_maxPickingForce;
    DeformableBodySolver* m_deformablebodySolver;
    ReducedDeformableBodySolver* m_reducedSoftBodySolver;
    AlignedObjectArray<DeformableLagrangianForce*> m_lf;
#endif

#ifdef USE_DISCRETE_DYNAMICS_WORLD
    DiscreteDynamicsWorld* m_dynamicsWorld;
#else
    MultiBodyDynamicsWorld* m_dynamicsWorld;
#endif


    i32 m_constraintSolverType;
    SharedMemoryDebugDrawer* m_remoteDebugDrawer;

    AlignedObjectArray<b3ContactPointData> m_cachedContactPoints;
    MyBroadphaseCallback m_cachedOverlappingObjects;

    AlignedObjectArray<i32> m_sdfRecentLoadedBodies;
    AlignedObjectArray<i32> m_graphicsIndexToSegmentationMask;

    AlignedObjectArray<InternalStateLogger*> m_stateLoggers;
    i32 m_stateLoggersUniqueId;
    i32 m_profileTimingLoggingUid;
    STxt m_profileTimingFileName;

    struct GUIHelperInterface* m_guiHelper;

    i32 m_sharedMemoryKey;
    bool m_enableTinyRenderer;

    bool m_verboseOutput;

    //data for picking objects
    class RigidBody* m_pickedBody;

    i32 m_savedActivationState;
    class TypedConstraint* m_pickedConstraint;
    class MultiBodyPoint2Point* m_pickingMultiBodyPoint2Point;
    Vec3 m_oldPickingPos;
    Vec3 m_hitPos;
    Scalar m_oldPickingDist;
    bool m_prevCanSleep;
    i32 m_pdControlPlugin;
    i32 m_collisionFilterPlugin;
    i32 m_grpcPlugin;

#ifdef D3_ENABLE_TINY_AUDIO
    SoundEngine m_soundEngine;
#endif

    b3HashMap<b3HashString, tuk> m_profileEvents;
    b3HashMap<b3HashString, UrdfVisualShapeCache> m_cachedVUrdfisualShapes;

    b3ThreadPool* m_threadPool;
    Scalar m_defaultCollisionMargin;

    double m_remoteSyncTransformTime;
    double m_remoteSyncTransformInterval;
    bool m_useAlternativeDeformableIndexing;

    PhysicsServerCommandProcessorInternalData(PhysicsCommandProcessorInterface* proc)
        : m_pluginManager(proc),
          m_useRealTimeSimulation(false),
          m_commandLogger(0),
          m_commandLoggingUid(-1),
          m_logPlayback(0),
          m_logPlaybackUid(-1),
          m_physicsDeltaTime(1. / 240.),
          m_numSimulationSubSteps(0),
          m_simulationTimestamp(0),
          m_userConstraintUIDGenerator(1),
          m_broadphaseCollisionFilterCallback(0),
          m_pairCache(0),
          m_broadphase(0),
          m_dispatcher(0),
          m_solver(0),
          m_collisionConfiguration(0),
#ifndef SKIP_DEFORMABLE_BODY
          m_pickedSoftBody(0),
          m_mouseForce(0),
          m_maxPickingForce(0.3),
          m_deformablebodySolver(0),
#endif
          m_dynamicsWorld(0),
          m_constraintSolverType(-1),
          m_remoteDebugDrawer(0),
          m_stateLoggersUniqueId(0),
          m_profileTimingLoggingUid(-1),
          m_guiHelper(0),
          m_sharedMemoryKey(SHARED_MEMORY_KEY),
          m_enableTinyRenderer(true),
          m_verboseOutput(false),
          m_pickedBody(0),
          m_pickedConstraint(0),
          m_pickingMultiBodyPoint2Point(0),
          m_pdControlPlugin(-1),
          m_collisionFilterPlugin(-1),
          m_grpcPlugin(-1),
          m_threadPool(0),
          m_defaultCollisionMargin(0.001),
          m_remoteSyncTransformTime(1. / 30.),
          m_remoteSyncTransformInterval(1. / 30.),
        m_useAlternativeDeformableIndexing(false)
    {
        {
            //register static plugins:
#ifdef STATIC_LINK_VR_PLUGIN
            b3PluginFunctions funcs(initPlugin_vrSyncPlugin, exitPlugin_vrSyncPlugin, executePluginCommand_vrSyncPlugin);
            funcs.m_preTickFunc = preTickPluginCallback_vrSyncPlugin;
            m_pluginManager.registerStaticLinkedPlugin("vrSyncPlugin", funcs);
#endif  //STATIC_LINK_VR_PLUGIN
        }
#ifndef SKIP_STATIC_PD_CONTROL_PLUGIN
        {
            //i32 b3PluginManager::registerStaticLinkedPlugin(tukk pluginPath, PFN_INIT initFunc, PFN_EXIT exitFunc, PFN_EXECUTE executeCommandFunc, PFN_TICK preTickFunc, PFN_TICK postTickFunc, PFN_GET_RENDER_INTERFACE getRendererFunc, PFN_TICK processClientCommandsFunc, PFN_GET_COLLISION_INTERFACE getCollisionFunc, bool initPlugin)
            b3PluginFunctions funcs(initPlugin_pdControlPlugin, exitPlugin_pdControlPlugin, executePluginCommand_pdControlPlugin);
            funcs.m_preTickFunc = preTickPluginCallback_pdControlPlugin;
            m_pdControlPlugin = m_pluginManager.registerStaticLinkedPlugin("pdControlPlugin", funcs);
        }
#endif  //SKIP_STATIC_PD_CONTROL_PLUGIN

#ifndef SKIP_COLLISION_FILTER_PLUGIN
        {
            b3PluginFunctions funcs(initPlugin_collisionFilterPlugin, exitPlugin_collisionFilterPlugin, executePluginCommand_collisionFilterPlugin);
            funcs.m_getCollisionFunc = getCollisionInterface_collisionFilterPlugin;
            m_collisionFilterPlugin = m_pluginManager.registerStaticLinkedPlugin("collisionFilterPlugin", funcs);
            m_pluginManager.selectCollisionPlugin(m_collisionFilterPlugin);
        }
#endif

#ifdef ENABLE_STATIC_GRPC_PLUGIN
        {
            b3PluginFunctions funcs(initPlugin_grpcPlugin, exitPlugin_grpcPlugin, executePluginCommand_grpcPlugin);
            funcs.m_processClientCommandsFunc = processClientCommands_grpcPlugin;
            m_grpcPlugin = m_pluginManager.registerStaticLinkedPlugin("grpcPlugin", funcs);
        }
#endif  //ENABLE_STATIC_GRPC_PLUGIN

#ifdef STATIC_EGLRENDERER_PLUGIN
        {
            bool initPlugin = false;
            b3PluginFunctions funcs(initPlugin_eglRendererPlugin, exitPlugin_eglRendererPlugin, executePluginCommand_eglRendererPlugin);
            funcs.m_getRendererFunc = getRenderInterface_eglRendererPlugin;
            i32 renderPluginId = m_pluginManager.registerStaticLinkedPlugin("eglRendererPlugin", funcs, initPlugin);
            m_pluginManager.selectPluginRenderer(renderPluginId);
        }
#endif  //STATIC_EGLRENDERER_PLUGIN

#ifndef SKIP_STATIC_TINYRENDERER_PLUGIN
        {
            b3PluginFunctions funcs(initPlugin_tinyRendererPlugin, exitPlugin_tinyRendererPlugin, executePluginCommand_tinyRendererPlugin);
            funcs.m_getRendererFunc = getRenderInterface_tinyRendererPlugin;
            i32 renderPluginId = m_pluginManager.registerStaticLinkedPlugin("tinyRendererPlugin", funcs);
            m_pluginManager.selectPluginRenderer(renderPluginId);
        }
#endif

#ifdef D3_ENABLE_FILEIO_PLUGIN
        {
            b3PluginFunctions funcs(initPlugin_fileIOPlugin, exitPlugin_fileIOPlugin, executePluginCommand_fileIOPlugin);
            funcs.m_fileIoFunc = getFileIOFunc_fileIOPlugin;
            i32 renderPluginId = m_pluginManager.registerStaticLinkedPlugin("fileIOPlugin", funcs);
            m_pluginManager.selectFileIOPlugin(renderPluginId);
        }
#endif

        m_vrControllerEvents.init();

        m_bodyHandles.exitHandles();
        m_bodyHandles.initHandles();
        m_userCollisionShapeHandles.exitHandles();
        m_userCollisionShapeHandles.initHandles();

        m_userVisualShapeHandles.exitHandles();
        m_userVisualShapeHandles.initHandles();
    }

#ifdef STATIC_LINK_SPD_PLUGIN
    b3HashMap<btHashPtr, cRBDModel*> m_rbdModels;

    static void convertPose(const btMultiBody* multiBody, const double* jointPositionsQ, const double* jointVelocitiesQdot, Eigen::VectorXd& pose, Eigen::VectorXd& vel)
    {
        i32 baseDofQ = multiBody->hasFixedBase() ? 0 : 7;
        i32 baseDofQdot = multiBody->hasFixedBase() ? 0 : 6;

        pose.resize(7 + multiBody->getNumPosVars());
        vel.resize(7 + multiBody->getNumPosVars());  //??

        Transform2 tr = multiBody->getBaseWorldTransform();
        i32 dofsrc = 0;
        i32 velsrcdof = 0;
        if (baseDofQ == 7)
        {
            pose[0] = jointPositionsQ[dofsrc++];
            pose[1] = jointPositionsQ[dofsrc++];
            pose[2] = jointPositionsQ[dofsrc++];

            double quatXYZW[4];
            quatXYZW[0] = jointPositionsQ[dofsrc++];
            quatXYZW[1] = jointPositionsQ[dofsrc++];
            quatXYZW[2] = jointPositionsQ[dofsrc++];
            quatXYZW[3] = jointPositionsQ[dofsrc++];

            pose[3] = quatXYZW[3];
            pose[4] = quatXYZW[0];
            pose[5] = quatXYZW[1];
            pose[6] = quatXYZW[2];
        }
        else
        {
            pose[0] = tr.getOrigin()[0];
            pose[1] = tr.getOrigin()[1];
            pose[2] = tr.getOrigin()[2];
            pose[3] = tr.getRotation()[3];
            pose[4] = tr.getRotation()[0];
            pose[5] = tr.getRotation()[1];
            pose[6] = tr.getRotation()[2];
        }
        if (baseDofQdot == 6)
        {
            vel[0] = jointVelocitiesQdot[velsrcdof++];
            vel[1] = jointVelocitiesQdot[velsrcdof++];
            vel[2] = jointVelocitiesQdot[velsrcdof++];
            vel[3] = jointVelocitiesQdot[velsrcdof++];
            vel[4] = jointVelocitiesQdot[velsrcdof++];
            vel[5] = jointVelocitiesQdot[velsrcdof++];
            vel[6] = jointVelocitiesQdot[velsrcdof++];
            vel[6] = 0;
        }
        else
        {
            vel[0] = multiBody->getBaseVel()[0];
            vel[1] = multiBody->getBaseVel()[1];
            vel[2] = multiBody->getBaseVel()[2];
            vel[3] = multiBody->getBaseOmega()[0];
            vel[4] = multiBody->getBaseOmega()[1];
            vel[5] = multiBody->getBaseOmega()[2];
            vel[6] = 0;
        }
        i32 dof = 7;
        i32 veldof = 7;

        for (i32 l = 0; l < multiBody->getNumLinks(); l++)
        {
            switch (multiBody->getLink(l).m_jointType)
            {
                case MultibodyLink::eRevolute:
                case MultibodyLink::ePrismatic:
                {
                    pose[dof++] = jointPositionsQ[dofsrc++];
                    vel[veldof++] = jointVelocitiesQdot[velsrcdof++];
                    break;
                }
                case MultibodyLink::eSpherical:
                {
                    double quatXYZW[4];
                    quatXYZW[0] = jointPositionsQ[dofsrc++];
                    quatXYZW[1] = jointPositionsQ[dofsrc++];
                    quatXYZW[2] = jointPositionsQ[dofsrc++];
                    quatXYZW[3] = jointPositionsQ[dofsrc++];

                    pose[dof++] = quatXYZW[3];
                    pose[dof++] = quatXYZW[0];
                    pose[dof++] = quatXYZW[1];
                    pose[dof++] = quatXYZW[2];
                    vel[veldof++] = jointVelocitiesQdot[velsrcdof++];
                    vel[veldof++] = jointVelocitiesQdot[velsrcdof++];
                    vel[veldof++] = jointVelocitiesQdot[velsrcdof++];
                    vel[veldof++] = jointVelocitiesQdot[velsrcdof++];
                    break;
                }
                case MultibodyLink::eFixed:
                {
                    break;
                }
                default:
                {
                    assert(0);
                }
            }
        }
    }

    cRBDModel* findOrCreateRBDModel(MultiBody* multiBody, const double* jointPositionsQ, const double* jointVelocitiesQdot)
    {
        cRBDModel* rbdModel = 0;
        cRBDModel** rbdModelPtr = m_rbdModels.find(multiBody);
        if (rbdModelPtr)
        {
            rbdModel = *rbdModelPtr;
        }
        else
        {
            rbdModel = new cRBDModel();
            Eigen::MatrixXd bodyDefs;
            Eigen::MatrixXd jointMat;
            btExtractJointBodyFromBullet(multiBody, bodyDefs, jointMat);
            Vec3 grav = m_dynamicsWorld->getGravity();
            tVector3 gravity(grav[0], grav[1], grav[2], 0);
            rbdModel->Init(jointMat, bodyDefs, gravity);
            m_rbdModels.insert(multiBody, rbdModel);
        }

        //sync pose and vel

        Eigen::VectorXd pose, vel;
        PhysicsServerCommandProcessorInternalData::convertPose(multiBody, jointPositionsQ, jointVelocitiesQdot, pose, vel);

        Vec3 gravOrg = m_dynamicsWorld->getGravity();
        tVector grav(gravOrg[0], gravOrg[1], gravOrg[2], 0);
        rbdModel->SetGravity(grav);
        {
            DRX3D_PROFILE("rbdModel::Update");
            rbdModel->Update(pose, vel);
        }

        return rbdModel;
    }

#endif

    drx3d_inverse::MultiBodyTree* findOrCreateTree(MultiBody* multiBody)
    {
        drx3d_inverse::MultiBodyTree* tree = 0;

        drx3d_inverse::MultiBodyTree** treePtrPtr =
            m_inverseDynamicsBodies.find(multiBody);

        if (treePtrPtr)
        {
            tree = *treePtrPtr;
        }
        else
        {
            drx3d_inverse::MultiBodyTreeCreator id_creator;
            if (-1 == id_creator.createFromBtMultiBody(multiBody, false))
            {
            }
            else
            {
                tree = drx3d_inverse::CreateMultiBodyTree(id_creator);
                m_inverseDynamicsBodies.insert(multiBody, tree);
            }
        }

        return tree;
    }
};

void PhysicsServerCommandProcessor::setGuiHelper(struct GUIHelperInterface* guiHelper)
{
    if (guiHelper)
    {
        guiHelper->createPhysicsDebugDrawer(m_data->m_dynamicsWorld);
    }
    else
    {
        //state loggers use guiHelper, so remove them before the guiHelper is deleted
        deleteStateLoggers();
        if (m_data->m_guiHelper && m_data->m_dynamicsWorld && m_data->m_dynamicsWorld->getDebugDrawer())
        {
            m_data->m_dynamicsWorld->setDebugDrawer(0);
        }
    }
    m_data->m_guiHelper = guiHelper;
}

PhysicsServerCommandProcessor::PhysicsServerCommandProcessor()
    : m_data(0)
{
    m_data = new PhysicsServerCommandProcessorInternalData(this);

    createEmptyDynamicsWorld();
}

PhysicsServerCommandProcessor::~PhysicsServerCommandProcessor()
{
    deleteDynamicsWorld();
    if (m_data->m_commandLogger)
    {
        delete m_data->m_commandLogger;
        m_data->m_commandLogger = 0;
    }
    for (i32 i = 0; i < m_data->m_profileEvents.size(); i++)
    {
        tuk event = *m_data->m_profileEvents.getAtIndex(i);
        delete[] event;
    }
    if (m_data->m_threadPool)
        delete m_data->m_threadPool;

    for (i32 i = 0; i < m_data->m_savedStates.size(); i++)
    {
        delete m_data->m_savedStates[i].m_bulletFile;
        delete m_data->m_savedStates[i].m_serializer;
    }

    delete m_data;
}

void preTickCallback(DynamicsWorld* world, Scalar timeStep)
{
    PhysicsServerCommandProcessor* proc = (PhysicsServerCommandProcessor*)world->getWorldUserInfo();

    proc->tickPlugins(timeStep, true);
}

void logCallback(DynamicsWorld* world, Scalar timeStep)
{
    //handle the logging and playing sounds
    PhysicsServerCommandProcessor* proc = (PhysicsServerCommandProcessor*)world->getWorldUserInfo();
    proc->processCollisionForces(timeStep);
    proc->logObjectStates(timeStep);

    proc->tickPlugins(timeStep, false);
}

bool MyContactAddedCallback(ManifoldPoint& cp, const CollisionObject2Wrapper* colObj0Wrap, i32 partId0, i32 index0, const CollisionObject2Wrapper* colObj1Wrap, i32 partId1, i32 index1)
{
    AdjustInternalEdgeContacts(cp, colObj1Wrap, colObj0Wrap, partId1, index1);
    return true;
}

bool MyContactDestroyedCallback(uk userPersistentData)
{
    //printf("destroyed\n");
    return false;
}

bool MyContactProcessedCallback(ManifoldPoint& cp, uk body0, uk body1)
{
    //printf("processed\n");
    return false;
}
void MyContactStartedCallback(PersistentManifold* const& manifold)
{
    //printf("started\n");
}
void MyContactEndedCallback(PersistentManifold* const& manifold)
{
    //  printf("ended\n");
}

void PhysicsServerCommandProcessor::processCollisionForces(Scalar timeStep)
{
#ifdef D3_ENABLE_TINY_AUDIO
    //this is experimental at the moment: impulse thresholds, sound parameters will be exposed in C-API/pybullet.
    //audio will go into a wav file, as well as real-time output to speakers/headphones using RtAudio/DAC.

    i32 numContactManifolds = m_data->m_dynamicsWorld->getDispatcher()->getNumManifolds();
    for (i32 i = 0; i < numContactManifolds; i++)
    {
        const PersistentManifold* manifold = m_data->m_dynamicsWorld->getDispatcher()->getInternalManifoldPointer()[i];

        bool objHasSound[2];
        objHasSound[0] = (0 != (manifold->getBody0()->getCollisionFlags() & CollisionObject2::CF_HAS_COLLISION_SOUND_TRIGGER));
        objHasSound[1] = (0 != (manifold->getBody1()->getCollisionFlags() & CollisionObject2::CF_HAS_COLLISION_SOUND_TRIGGER));
        const CollisionObject2* colObjs[2] = {manifold->getBody0(), manifold->getBody1()};

        for (i32 ob = 0; ob < 2; ob++)
        {
            if (objHasSound[ob])
            {
                i32 uid0 = -1;
                i32 linkIndex = -2;

                const MultiBodyLinkCollider* mblB = MultiBodyLinkCollider::upcast(colObjs[ob]);
                if (mblB && mblB->m_multiBody)
                {
                    linkIndex = mblB->m_link;
                    uid0 = mblB->m_multiBody->getUserIndex2();
                }
                const RigidBody* bodyB = RigidBody::upcast(colObjs[ob]);
                if (bodyB)
                {
                    uid0 = bodyB->getUserIndex2();
                    linkIndex = -1;
                }

                if ((uid0 < 0) || (linkIndex < -1))
                    continue;

                InternalBodyHandle* bodyHandle0 = m_data->m_bodyHandles.getHandle(uid0);
                SDFAudioSource* audioSrc = bodyHandle0->m_audioSources[linkIndex];
                if (audioSrc == 0)
                    continue;

                for (i32 p = 0; p < manifold->getNumContacts(); p++)
                {
                    double imp = manifold->getContactPoint(p).getAppliedImpulse();
                    //printf ("manifold %d, contact %d, lifeTime:%d, appliedImpulse:%f\n",i,p, manifold->getContactPoint(p).getLifeTime(),imp);

                    if (imp > audioSrc->m_collisionForceThreshold && manifold->getContactPoint(p).getLifeTime() == 1)
                    {
                        i32 soundSourceIndex = m_data->m_soundEngine.getAvailableSoundSource();
                        if (soundSourceIndex >= 0)
                        {
                            SoundMessage msg;
                            msg.m_attackRate = audioSrc->m_attackRate;
                            msg.m_decayRate = audioSrc->m_decayRate;
                            msg.m_sustainLevel = audioSrc->m_sustainLevel;
                            msg.m_releaseRate = audioSrc->m_releaseRate;
                            msg.m_amplitude = audioSrc->m_gain;
                            msg.m_frequency = audioSrc->m_pitch;
                            msg.m_type = D3_SOUND_SOURCE_WAV_FILE;
                            msg.m_wavId = audioSrc->m_userIndex;
                            msg.m_autoKeyOff = true;
                            m_data->m_soundEngine.startSound(soundSourceIndex, msg);
                        }
                    }
                }
            }
        }
    }
#endif  //D3_ENABLE_TINY_AUDIO
}

void PhysicsServerCommandProcessor::processClientCommands()
{
    m_data->m_pluginManager.tickPlugins(0, D3_PROCESS_CLIENT_COMMANDS_TICK);
}

void PhysicsServerCommandProcessor::reportNotifications()
{
    m_data->m_pluginManager.reportNotifications();
}

void PhysicsServerCommandProcessor::tickPlugins(Scalar timeStep, bool isPreTick)
{
    b3PluginManagerTickMode tickMode = isPreTick ? D3_PRE_TICK_MODE : D3_POST_TICK_MODE;
    m_data->m_pluginManager.tickPlugins(timeStep, tickMode);
    if (!isPreTick)
    {
        //clear events after each postTick, so we don't receive events multiple ticks
        m_data->m_pluginManager.clearEvents();
    }
}

void PhysicsServerCommandProcessor::logObjectStates(Scalar timeStep)
{
    for (i32 i = 0; i < m_data->m_stateLoggers.size(); i++)
    {
        m_data->m_stateLoggers[i]->logState(timeStep);
    }
}

struct ProgrammaticUrdfInterface : public URDFImporterInterface
{
    i32 m_bodyUniqueId;

    const b3CreateMultiBodyArgs& m_createBodyArgs;
    mutable b3AlignedObjectArray<CollisionShape*> m_allocatedCollisionShapes;
    PhysicsServerCommandProcessorInternalData* m_data;
    i32 m_flags;

    ProgrammaticUrdfInterface(const b3CreateMultiBodyArgs& bodyArgs, PhysicsServerCommandProcessorInternalData* data, i32 flags)
        : m_bodyUniqueId(-1),
          m_createBodyArgs(bodyArgs),
          m_data(data),
          m_flags(flags)
    {
    }

    virtual ~ProgrammaticUrdfInterface()
    {
    }

    virtual bool loadURDF(tukk fileName, bool forceFixedBase = false)
    {
        drx3DAssert(0);
        return false;
    }

    virtual tukk getPathPrefix()
    {
        return "";
    }

    ///return >=0 for the root link index, -1 if there is no root link
    virtual i32 getRootLinkIndex() const
    {
        return m_createBodyArgs.m_baseLinkIndex;
    }

    ///pure virtual interfaces, precondition is a valid linkIndex (you can assert/terminate if the linkIndex is out of range)
    virtual STxt getLinkName(i32 linkIndex) const
    {

        STxt linkName = "link";
        char numstr[21];  // enough to hold all numbers up to 64-bits
        sprintf(numstr, "%d", linkIndex);
        linkName = linkName + numstr;
        return linkName;
    }

    //various derived class in internal source code break with new pure virtual methods, so provide some default implementation
    virtual STxt getBodyName() const
    {
        return m_createBodyArgs.m_bodyName;
    }

    /// optional method to provide the link color. return true if the color is available and copied into colorRGBA, return false otherwise
    virtual bool getLinkColor(i32 linkIndex, Vec4& colorRGBA) const
    {
        drx3DAssert(0);
        return false;
    }

    mutable HashMap<HashInt, UrdfMaterialColor> m_linkColors;

    virtual bool getLinkColor2(i32 linkIndex, struct UrdfMaterialColor& matCol) const
    {
        if (m_flags & URDF_USE_MATERIAL_COLORS_FROM_MTL)
        {
            const UrdfMaterialColor* matColPtr = m_linkColors[linkIndex];
            if (matColPtr)
            {
                matCol = *matColPtr;
                if ((m_flags & CUF_USE_MATERIAL_TRANSPARANCY_FROM_MTL) == 0)
                {
                    matCol.m_rgbaColor[3] = 1;
                }

                return true;
            }
        }
        else
        {
            if (m_createBodyArgs.m_linkVisualShapeUniqueIds[linkIndex] >= 0)
            {
                const InternalVisualShapeHandle* visHandle = m_data->m_userVisualShapeHandles.getHandle(m_createBodyArgs.m_linkVisualShapeUniqueIds[linkIndex]);
                if (visHandle)
                {
                    for (i32 i = 0; i < visHandle->m_visualShapes.size(); i++)
                    {
                        if (visHandle->m_visualShapes[i].m_geometry.m_hasLocalMaterial)
                        {
                            matCol = visHandle->m_visualShapes[i].m_geometry.m_localMaterial.m_matColor;
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    virtual i32 getCollisionGroupAndMask(i32 linkIndex, i32& colGroup, i32& colMask) const
    {
        return 0;
    }
    ///this API will likely change, don't override it!
    virtual bool getLinkContactInfo(i32 linkIndex, URDFLinkContactInfo& contactInfo) const
    {
        return false;
    }

    virtual bool getLinkAudioSource(i32 linkIndex, SDFAudioSource& audioSource) const
    {
        drx3DAssert(0);
        return false;
    }

    virtual STxt getJointName(i32 linkIndex) const
    {
        STxt jointName = "joint";
        char numstr[21];  // enough to hold all numbers up to 64-bits
        sprintf(numstr, "%d", linkIndex);
        jointName = jointName + numstr;
        return jointName;
    }

    //fill mass and inertial data. If inertial data is missing, please initialize mass, inertia to sensitive values, and inertialFrame to identity.
    virtual void getMassAndInertia(i32 urdfLinkIndex, Scalar& mass, Vec3& localInertiaDiagonal, Transform2& inertialFrame) const
    {
        if (urdfLinkIndex >= 0 && urdfLinkIndex < m_createBodyArgs.m_numLinks)
        {
            mass = m_createBodyArgs.m_linkMasses[urdfLinkIndex];
            localInertiaDiagonal.setVal(
                m_createBodyArgs.m_linkInertias[urdfLinkIndex * 3 + 0],
                m_createBodyArgs.m_linkInertias[urdfLinkIndex * 3 + 1],
                m_createBodyArgs.m_linkInertias[urdfLinkIndex * 3 + 2]);
            inertialFrame.setOrigin(Vec3(
                m_createBodyArgs.m_linkInertialFramePositions[urdfLinkIndex * 3 + 0],
                m_createBodyArgs.m_linkInertialFramePositions[urdfLinkIndex * 3 + 1],
                m_createBodyArgs.m_linkInertialFramePositions[urdfLinkIndex * 3 + 2]));
            inertialFrame.setRotation(Quat(
                m_createBodyArgs.m_linkInertialFrameOrientations[urdfLinkIndex * 4 + 0],
                m_createBodyArgs.m_linkInertialFrameOrientations[urdfLinkIndex * 4 + 1],
                m_createBodyArgs.m_linkInertialFrameOrientations[urdfLinkIndex * 4 + 2],
                m_createBodyArgs.m_linkInertialFrameOrientations[urdfLinkIndex * 4 + 3]));
        }
        else
        {
            mass = 0;
            localInertiaDiagonal.setVal(0, 0, 0);
            inertialFrame.setIdentity();
        }
    }

    ///fill an array of child link indices for this link, AlignedObjectArray behaves like a std::vector so just use push_back and resize(0) if needed
    virtual void getLinkChildIndices(i32 urdfLinkIndex, AlignedObjectArray<i32>& childLinkIndices) const
    {
        for (i32 i = 0; i < m_createBodyArgs.m_numLinks; i++)
        {
            if (m_createBodyArgs.m_linkParentIndices[i] == urdfLinkIndex)
            {
                childLinkIndices.push_back(i);
            }
        }
    }

    virtual bool getJointInfo(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction) const
    {
        return false;
    };

    virtual bool getJointInfo2(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction, Scalar& jointMaxForce, Scalar& jointMaxVelocity) const
    {
        bool isValid = false;

        i32 jointTypeOrg = m_createBodyArgs.m_linkJointTypes[urdfLinkIndex];

        switch (jointTypeOrg)
        {
            case eRevoluteType:
            {
                isValid = true;
                jointType = URDFRevoluteJoint;
                break;
            }
            case ePrismaticType:
            {
                isValid = true;
                jointType = URDFPrismaticJoint;
                break;
            }
            case eFixedType:
            {
                isValid = true;
                jointType = URDFFixedJoint;
                break;
            }
            case eSphericalType:
            {
                isValid = true;
                jointType = URDFSphericalJoint;
                break;
            }
            //case  ePlanarType:
            //case  eFixedType:
            //case ePoint2PointType:
            //case eGearType:
            default:
            {
            }
        };

        if (isValid)
        {
            //backwards compatibility for custom file importers
            jointMaxForce = 0;
            jointMaxVelocity = 0;
            jointFriction = 0;
            jointDamping = 0;
            jointLowerLimit = 1;
            jointUpperLimit = -1;

            parent2joint.setOrigin(Vec3(
                m_createBodyArgs.m_linkPositions[urdfLinkIndex * 3 + 0],
                m_createBodyArgs.m_linkPositions[urdfLinkIndex * 3 + 1],
                m_createBodyArgs.m_linkPositions[urdfLinkIndex * 3 + 2]));
            parent2joint.setRotation(Quat(
                m_createBodyArgs.m_linkOrientations[urdfLinkIndex * 4 + 0],
                m_createBodyArgs.m_linkOrientations[urdfLinkIndex * 4 + 1],
                m_createBodyArgs.m_linkOrientations[urdfLinkIndex * 4 + 2],
                m_createBodyArgs.m_linkOrientations[urdfLinkIndex * 4 + 3]));

            linkTransformInWorld.setIdentity();

            jointAxisInJointSpace.setVal(
                m_createBodyArgs.m_linkJointAxis[3 * urdfLinkIndex + 0],
                m_createBodyArgs.m_linkJointAxis[3 * urdfLinkIndex + 1],
                m_createBodyArgs.m_linkJointAxis[3 * urdfLinkIndex + 2]);
        }
        return isValid;
    };

    virtual bool getRootTransformInWorld(Transform2& rootTransformInWorld) const
    {
        i32 baseLinkIndex = m_createBodyArgs.m_baseLinkIndex;

        rootTransformInWorld.setOrigin(Vec3(
            m_createBodyArgs.m_linkPositions[baseLinkIndex * 3 + 0],
            m_createBodyArgs.m_linkPositions[baseLinkIndex * 3 + 1],
            m_createBodyArgs.m_linkPositions[baseLinkIndex * 3 + 2]));
        rootTransformInWorld.setRotation(Quat(
            m_createBodyArgs.m_linkOrientations[baseLinkIndex * 4 + 0],
            m_createBodyArgs.m_linkOrientations[baseLinkIndex * 4 + 1],
            m_createBodyArgs.m_linkOrientations[baseLinkIndex * 4 + 2],
            m_createBodyArgs.m_linkOrientations[baseLinkIndex * 4 + 3]));
        return true;
    }
    virtual void setRootTransformInWorld(const Transform2& rootTransformInWorld)
    {
        drx3DAssert(0);
    }

    virtual i32 convertLinkVisualShapes(i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame) const
    {
        i32 graphicsIndex = -1;
        double globalScaling = 1.f;  //todo!
        i32 flags = 0;
        CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();

        URDFImporter u2b(m_data->m_guiHelper, m_data->m_pluginManager.getRenderInterface(), fileIO, globalScaling, flags);
        u2b.setEnableTinyRenderer(m_data->m_enableTinyRenderer);

        AlignedObjectArray<GLInstanceVertex> vertices;
        AlignedObjectArray<i32> indices;
        Transform2 startTrans;
        startTrans.setIdentity();
        AlignedObjectArray<BulletURDFTexture> textures;

        if (m_createBodyArgs.m_linkVisualShapeUniqueIds[linkIndex] >= 0)
        {
            InternalVisualShapeHandle* visHandle = m_data->m_userVisualShapeHandles.getHandle(m_createBodyArgs.m_linkVisualShapeUniqueIds[linkIndex]);
            if (visHandle)
            {
                if (visHandle->m_OpenGLGraphicsIndex >= 0)
                {
                    //instancing. assume the inertial frame is identical
                    graphicsIndex = visHandle->m_OpenGLGraphicsIndex;
                }
                else
                {
                    for (i32 v = 0; v < visHandle->m_visualShapes.size(); v++)
                    {
                        b3ImportMeshData meshData;
                        u2b.convertURDFToVisualShapeInternal(&visHandle->m_visualShapes[v], pathPrefix, localInertiaFrame.inverse() * visHandle->m_visualShapes[v].m_linkLocalFrame, vertices, indices, textures, meshData);
                        if ((meshData.m_flags & D3_IMPORT_MESH_HAS_RGBA_COLOR) &&
                            (meshData.m_flags & D3_IMPORT_MESH_HAS_SPECULAR_COLOR))
                        {
                            UrdfMaterialColor matCol;
                            matCol.m_rgbaColor.setVal(meshData.m_rgbaColor[0],
                                                        meshData.m_rgbaColor[1],
                                                        meshData.m_rgbaColor[2],
                                                        meshData.m_rgbaColor[3]);
                            matCol.m_specularColor.setVal(meshData.m_specularColor[0],
                                                            meshData.m_specularColor[1],
                                                            meshData.m_specularColor[2]);
                            m_linkColors.insert(linkIndex, matCol);
                        }
                    }

                    if (vertices.size() && indices.size())
                    {
                        if (1)
                        {
                            i32 textureIndex = -1;
                            if (textures.size())
                            {
                                textureIndex = m_data->m_guiHelper->registerTexture(textures[0].textureData1, textures[0].m_width, textures[0].m_height);
                            }

                            {
                                D3_PROFILE("registerGraphicsShape");
                                graphicsIndex = m_data->m_guiHelper->registerGraphicsShape(&vertices[0].xyzw[0], vertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES, textureIndex);
                                visHandle->m_OpenGLGraphicsIndex = graphicsIndex;
                            }
                        }
                    }
                }
            }
        }

        //delete textures
        for (i32 i = 0; i < textures.size(); i++)
        {
            D3_PROFILE("free textureData");
            if (!textures[i].m_isCached)
            {
                m_data->m_allocatedTexturesRequireFree.push_back(textures[i].textureData1);
            }
        }

        return graphicsIndex;
    }

    virtual void convertLinkVisualShapes2(i32 linkIndex, i32 urdfIndex, tukk pathPrefix, const Transform2& localInertiaFrame, class CollisionObject2* colObj, i32 bodyUniqueId) const
    {
        //if there is a visual, use it, otherwise convert collision shape back into UrdfCollision...

        UrdfModel model;  // = m_data->m_urdfParser.getModel();
        UrdfLink link;

        if (m_createBodyArgs.m_linkVisualShapeUniqueIds[urdfIndex] >= 0)
        {
            const InternalVisualShapeHandle* visHandle = m_data->m_userVisualShapeHandles.getHandle(m_createBodyArgs.m_linkVisualShapeUniqueIds[urdfIndex]);
            if (visHandle)
            {
                for (i32 i = 0; i < visHandle->m_visualShapes.size(); i++)
                {
                    link.m_visualArray.push_back(visHandle->m_visualShapes[i]);
                }
            }
        }

        if (link.m_visualArray.size() == 0)
        {
            i32 colShapeUniqueId = m_createBodyArgs.m_linkCollisionShapeUniqueIds[urdfIndex];
            if (colShapeUniqueId >= 0)
            {
                InternalCollisionShapeHandle* handle = m_data->m_userCollisionShapeHandles.getHandle(colShapeUniqueId);
                if (handle)
                {
                    for (i32 i = 0; i < handle->m_urdfCollisionObjects.size(); i++)
                    {
                        link.m_collisionArray.push_back(handle->m_urdfCollisionObjects[i]);
                    }
                }
            }
        }
        //UrdfVisual vis;
        //link.m_visualArray.push_back(vis);
        //UrdfLink*const* linkPtr = model.m_links.getAtIndex(urdfIndex);
        if (m_data->m_pluginManager.getRenderInterface())
        {
            CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
            i32 visualShapeUniqueid = m_data->m_pluginManager.getRenderInterface()->convertVisualShapes(
                linkIndex,
                pathPrefix,
                localInertiaFrame,
                &link,
                &model,
                colObj->getBroadphaseHandle()->getUid(),
                bodyUniqueId,
                fileIO);

            colObj->getCollisionShape()->setUserIndex2(visualShapeUniqueid);
            colObj->setUserIndex3(visualShapeUniqueid);
        }
    }
    virtual void setBodyUniqueId(i32 bodyId)
    {
        m_bodyUniqueId = bodyId;
    }
    virtual i32 getBodyUniqueId() const
    {
        return m_bodyUniqueId;
    }

    //default implementation for backward compatibility
    virtual class CompoundShape* convertLinkCollisionShapes(i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame) const
    {
        CompoundShape* compound = new CompoundShape();

        i32 colShapeUniqueId = m_createBodyArgs.m_linkCollisionShapeUniqueIds[linkIndex];
        if (colShapeUniqueId >= 0)
        {
            InternalCollisionShapeHandle* handle = m_data->m_userCollisionShapeHandles.getHandle(colShapeUniqueId);
            if (handle && handle->m_collisionShape)
            {
                handle->m_used++;
                if (handle->m_collisionShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
                {
                    CompoundShape* childCompound = (CompoundShape*)handle->m_collisionShape;
                    for (i32 c = 0; c < childCompound->getNumChildShapes(); c++)
                    {
                        Transform2 childTrans = childCompound->getChildTransform(c);
                        CollisionShape* childShape = childCompound->getChildShape(c);
                        Transform2 tr = localInertiaFrame.inverse() * childTrans;
                        compound->addChildShape(tr, childShape);
                    }
                }
                else
                {
                    Transform2 childTrans;
                    childTrans.setIdentity();
                    compound->addChildShape(localInertiaFrame.inverse() * childTrans, handle->m_collisionShape);
                }
            }
        }
        m_allocatedCollisionShapes.push_back(compound);
        return compound;
    }

    virtual i32 getNumAllocatedCollisionShapes() const
    {
        return m_allocatedCollisionShapes.size();
    }

    virtual class CollisionShape* getAllocatedCollisionShape(i32 index)
    {
        return m_allocatedCollisionShapes[index];
    }
    virtual i32 getNumModels() const
    {
        return 1;
    }
    virtual void activateModel(i32 /*modelIndex*/)
    {
    }
};

DeformableMultiBodyDynamicsWorld* PhysicsServerCommandProcessor::getDeformableWorld()
{
    DeformableMultiBodyDynamicsWorld* world = 0;
    if (m_data->m_dynamicsWorld && m_data->m_dynamicsWorld->getWorldType() == DRX3D_DEFORMABLE_MULTIBODY_DYNAMICS_WORLD)
    {
        world = (DeformableMultiBodyDynamicsWorld*)m_data->m_dynamicsWorld;
    }
    return world;
}

SoftMultiBodyDynamicsWorld* PhysicsServerCommandProcessor::getSoftWorld()
{
    SoftMultiBodyDynamicsWorld* world = 0;
    if (m_data->m_dynamicsWorld && m_data->m_dynamicsWorld->getWorldType() == DRX3D_SOFT_MULTIBODY_DYNAMICS_WORLD)
    {
        world = (SoftMultiBodyDynamicsWorld*)m_data->m_dynamicsWorld;
    }
    return world;
}

void PhysicsServerCommandProcessor::createEmptyDynamicsWorld(i32 flags)
{
    m_data->m_constraintSolverType = eConstraintSolverLCP_SI;
    ///collision configuration contains default setup for memory, collision setup
    //m_collisionConfiguration->setConvexConvexMultipointIterations();
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    m_data->m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();
#else
    m_data->m_collisionConfiguration = new btDefaultCollisionConfiguration();
#endif
    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_data->m_dispatcher = new CollisionDispatcher(m_data->m_collisionConfiguration);

    m_data->m_broadphaseCollisionFilterCallback = new MyOverlapFilterCallback(&m_data->m_pluginManager);
    m_data->m_broadphaseCollisionFilterCallback->m_filterMode = D3_FILTER_GROUPAMASKB_OR_GROUPBMASKA;

    m_data->m_pairCache = new HashedOverlappingPairCache();

    m_data->m_pairCache->setOverlapFilterCallback(m_data->m_broadphaseCollisionFilterCallback);

    //i32 maxProxies = 32768;
    if (flags & RESET_USE_SIMPLE_BROADPHASE)
    {
        m_data->m_broadphase = new SimpleBroadphase(65536, m_data->m_pairCache);
    }
    else
    {
        DbvtBroadphase* bv = new DbvtBroadphase(m_data->m_pairCache);
        bv->setVelocityPrediction(0);
        m_data->m_broadphase = bv;
    }

#ifndef SKIP_DEFORMABLE_BODY
    if (flags & RESET_USE_DEFORMABLE_WORLD)
    {
        // deformable
        m_data->m_deformablebodySolver = new DeformableBodySolver();
        DeformableMultiBodyConstraintSolver* solver = new DeformableMultiBodyConstraintSolver;
        m_data->m_solver = solver;
        solver->setDeformableSolver(m_data->m_deformablebodySolver);
        m_data->m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_data->m_dispatcher, m_data->m_broadphase, solver, m_data->m_collisionConfiguration, m_data->m_deformablebodySolver);
    }
    else if (flags & RESET_USE_REDUCED_DEFORMABLE_WORLD)
    {
        // reduced deformable
        m_data->m_reducedSoftBodySolver = new ReducedDeformableBodySolver();
        DeformableMultiBodyConstraintSolver* solver = new DeformableMultiBodyConstraintSolver;
        m_data->m_solver = solver;
        solver->setDeformableSolver(m_data->m_reducedSoftBodySolver);
        m_data->m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_data->m_dispatcher, m_data->m_broadphase, solver, m_data->m_collisionConfiguration, m_data->m_reducedSoftBodySolver);
    }
#endif



    if ((0 == m_data->m_dynamicsWorld) && (0 == (flags & RESET_USE_DISCRETE_DYNAMICS_WORLD)))
    {

#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        m_data->m_solver = new MultiBodyConstraintSolver;
        m_data->m_dynamicsWorld = new SoftMultiBodyDynamicsWorld(m_data->m_dispatcher, m_data->m_broadphase, m_data->m_solver, m_data->m_collisionConfiguration);
#else
#ifdef USE_DISCRETE_DYNAMICS_WORLD
        m_data->m_solver = new SequentialImpulseConstraintSolver;
        m_data->m_dynamicsWorld = new DiscreteDynamicsWorld(m_data->m_dispatcher, m_data->m_broadphase, m_data->m_solver, m_data->m_collisionConfiguration);
#else
        m_data->m_solver = new btMultiBodyConstraintSolver;
        m_data->m_dynamicsWorld = new btMultiBodyDynamicsWorld(m_data->m_dispatcher, m_data->m_broadphase, m_data->m_solver, m_data->m_collisionConfiguration);
        #endif
#endif
    }

    if (0 == m_data->m_dynamicsWorld)
    {
#ifdef USE_DISCRETE_DYNAMICS_WORLD
        m_data->m_solver = new SequentialImpulseConstraintSolver;
        m_data->m_dynamicsWorld = new DiscreteDynamicsWorld(m_data->m_dispatcher, m_data->m_broadphase, m_data->m_solver, m_data->m_collisionConfiguration);
#else

        m_data->m_solver = new MultiBodyConstraintSolver;
        m_data->m_dynamicsWorld = new MultiBodyDynamicsWorld(m_data->m_dispatcher, m_data->m_broadphase, m_data->m_solver, m_data->m_collisionConfiguration);
#endif
    }

     //may help improve run-time performance for many sleeping objects
    m_data->m_dynamicsWorld->setForceUpdateAllAabbs(false);

    //Workaround: in a VR application, where we avoid synchronizing between GFX/Physics threads, we don't want to resize this array, so pre-allocate it
    m_data->m_dynamicsWorld->getCollisionObjectArray().reserve(128 * 1024);

    m_data->m_remoteDebugDrawer = new SharedMemoryDebugDrawer();

    m_data->m_dynamicsWorld->setGravity(Vec3(0, 0, 0));
    m_data->m_dynamicsWorld->getSolverInfo().m_erp2 = 0.08;

    m_data->m_dynamicsWorld->getSolverInfo().m_frictionERP = 0.2;  //need to check if there are artifacts with frictionERP
    m_data->m_dynamicsWorld->getSolverInfo().m_linearSlop = 0.00001;
    m_data->m_dynamicsWorld->getSolverInfo().m_numIterations = 50;
    if (flags & RESET_USE_REDUCED_DEFORMABLE_WORLD)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 128;
    }
    else
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 0;
    }
    m_data->m_dynamicsWorld->getSolverInfo().m_warmstartingFactor = 0.1;
    gDbvtMargin = Scalar(0);
    m_data->m_dynamicsWorld->getSolverInfo().m_leastSquaresResidualThreshold = 1e-7;

    if (m_data->m_guiHelper)
    {
        m_data->m_guiHelper->createPhysicsDebugDrawer(m_data->m_dynamicsWorld);
    }
    bool isPreTick = false;
    m_data->m_dynamicsWorld->setInternalTickCallback(logCallback, this, isPreTick);
    isPreTick = true;
    m_data->m_dynamicsWorld->setInternalTickCallback(preTickCallback, this, isPreTick);

    gContactAddedCallback = MyContactAddedCallback;

#ifdef D3_ENABLE_TINY_AUDIO
    m_data->m_soundEngine.init(16, true);

//we don't use those callbacks (yet), experimental

//  gContactDestroyedCallback = MyContactDestroyedCallback;
//  gContactProcessedCallback = MyContactProcessedCallback;
//  gContactStartedCallback = MyContactStartedCallback;
//  gContactEndedCallback = MyContactEndedCallback;
#endif
}

void PhysicsServerCommandProcessor::deleteStateLoggers()
{
    for (i32 i = 0; i < m_data->m_stateLoggers.size(); i++)
    {
        m_data->m_stateLoggers[i]->stop();
        delete m_data->m_stateLoggers[i];
    }
    m_data->m_stateLoggers.clear();
}

void PhysicsServerCommandProcessor::deleteCachedInverseKinematicsBodies()
{
    for (i32 i = 0; i < m_data->m_inverseKinematicsHelpers.size(); i++)
    {
        IKTrajectoryHelper** ikHelperPtr = m_data->m_inverseKinematicsHelpers.getAtIndex(i);
        if (ikHelperPtr)
        {
            IKTrajectoryHelper* ikHelper = *ikHelperPtr;
            delete ikHelper;
        }
    }
    m_data->m_inverseKinematicsHelpers.clear();
}
void PhysicsServerCommandProcessor::deleteCacheddrx3d_inverseBodies()
{
    for (i32 i = 0; i < m_data->m_inverseDynamicsBodies.size(); i++)
    {
        drx3d_inverse::MultiBodyTree** treePtrPtr = m_data->m_inverseDynamicsBodies.getAtIndex(i);
        if (treePtrPtr)
        {
            drx3d_inverse::MultiBodyTree* tree = *treePtrPtr;
            delete tree;
        }
    }
    m_data->m_inverseDynamicsBodies.clear();
}

void PhysicsServerCommandProcessor::deleteDynamicsWorld()
{
#ifdef D3_ENABLE_TINY_AUDIO
    m_data->m_soundEngine.exit();
    //gContactDestroyedCallback = 0;
    //gContactProcessedCallback = 0;
    //gContactStartedCallback = 0;
    //gContactEndedCallback = 0;
#endif

    deleteCacheddrx3d_inverseBodies();
    deleteCachedInverseKinematicsBodies();
    deleteStateLoggers();

    m_data->m_userConstraints.clear();
    m_data->m_saveWorldBodyData.clear();

    for (i32 i = 0; i < m_data->m_multiBodyJointFeedbacks.size(); i++)
    {
        delete m_data->m_multiBodyJointFeedbacks[i];
    }
    m_data->m_multiBodyJointFeedbacks.clear();

    for (i32 i = 0; i < m_data->m_worldImporters.size(); i++)
    {
        m_data->m_worldImporters[i]->deleteAllData();
        delete m_data->m_worldImporters[i];
    }
    m_data->m_worldImporters.clear();

#ifdef ENABLE_LINK_MAPPER
    for (i32 i = 0; i < m_data->m_urdfLinkNameMapper.size(); i++)
    {
        delete m_data->m_urdfLinkNameMapper[i];
    }
    m_data->m_urdfLinkNameMapper.clear();
#endif  //ENABLE_LINK_MAPPER

    for (i32 i = 0; i < m_data->m_strings.size(); i++)
    {
        delete m_data->m_strings[i];
    }
    m_data->m_strings.clear();

    AlignedObjectArray<TypedConstraint*> constraints;
    AlignedObjectArray<MultiBodyConstraint*> mbconstraints;

    if (m_data->m_dynamicsWorld)
    {
        i32 i;
        for (i = m_data->m_dynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
        {
            TypedConstraint* constraint = m_data->m_dynamicsWorld->getConstraint(i);
            constraints.push_back(constraint);
            m_data->m_dynamicsWorld->removeConstraint(constraint);
        }
#ifndef USE_DISCRETE_DYNAMICS_WORLD
        for (i = m_data->m_dynamicsWorld->getNumMultiBodyConstraints() - 1; i >= 0; i--)
        {
            MultiBodyConstraint* mbconstraint = m_data->m_dynamicsWorld->getMultiBodyConstraint(i);
            mbconstraints.push_back(mbconstraint);
            m_data->m_dynamicsWorld->removeMultiBodyConstraint(mbconstraint);
        }
#endif
        for (i = m_data->m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            CollisionObject2* obj = m_data->m_dynamicsWorld->getCollisionObjectArray()[i];
            RigidBody* body = RigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            m_data->m_dynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }
#ifndef USE_DISCRETE_DYNAMICS_WORLD
        for (i = m_data->m_dynamicsWorld->getNumMultibodies() - 1; i >= 0; i--)
        {
            MultiBody* mb = m_data->m_dynamicsWorld->getMultiBody(i);
            m_data->m_dynamicsWorld->removeMultiBody(mb);
            delete mb;
        }
#endif
#ifndef SKIP_DEFORMABLE_BODY
        for (i32 j = 0; j < m_data->m_lf.size(); j++)
        {
            DeformableLagrangianForce* force = m_data->m_lf[j];
            delete force;
        }
        m_data->m_lf.clear();
#endif
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        {
            SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
            if (softWorld)
            {
                for (i = softWorld->getSoftBodyArray().size() - 1; i >= 0; i--)
                {
                    SoftBody* sb = softWorld->getSoftBodyArray()[i];
                    softWorld->removeSoftBody(sb);
                    delete sb;
                }
            }
        }
#endif  //SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD

#ifndef SKIP_DEFORMABLE_BODY
        {
            DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
            if (deformWorld)
            {
                for (i = deformWorld->getSoftBodyArray().size() - 1; i >= 0; i--)
                {
                    SoftBody* sb = deformWorld->getSoftBodyArray()[i];
                    deformWorld->removeSoftBody(sb);
                    delete sb;
                }
            }
        }
#endif
    }

    for (i32 i = 0; i < constraints.size(); i++)
    {
        delete constraints[i];
    }
    constraints.clear();
    for (i32 i = 0; i < mbconstraints.size(); i++)
    {
        delete mbconstraints[i];
    }
    mbconstraints.clear();
    //delete collision shapes
    for (i32 j = 0; j < m_data->m_collisionShapes.size(); j++)
    {
        CollisionShape* shape = m_data->m_collisionShapes[j];

        //check for internal edge utility, delete memory
        if (shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
        {
            BvhTriangleMeshShape* trimesh = (BvhTriangleMeshShape*)shape;
            if (trimesh->getTriangleInfoMap())
            {
                delete trimesh->getTriangleInfoMap();
            }
        }
        if (shape->getShapeType() == TERRAIN_SHAPE_PROXYTYPE)
        {
            HeightfieldTerrainShape* terrain = (HeightfieldTerrainShape*)shape;
            if (terrain->getTriangleInfoMap())
            {
                delete terrain->getTriangleInfoMap();
            }
        }
        delete shape;
    }
    for (i32 j = 0; j < m_data->m_heightfieldDatas.size(); j++)
    {
        delete[] m_data->m_heightfieldDatas[j];
    }

    for (i32 j = 0; j < m_data->m_meshInterfaces.size(); j++)
    {
        delete m_data->m_meshInterfaces[j];
    }

    if (m_data->m_guiHelper)
    {
        for (i32 j = 0; j < m_data->m_allocatedTextures.size(); j++)
        {
            i32 texId = m_data->m_allocatedTextures[j];
            m_data->m_guiHelper->removeTexture(texId);
        }
    }

    for (i32 i = 0; i < m_data->m_allocatedTexturesRequireFree.size(); i++)
    {
        //we can't free them right away, due to caching based on memory pointer in PhysicsServerExample
        free(m_data->m_allocatedTexturesRequireFree[i]);
    }

    for (i32 i = 0; i < m_data->m_debugPointsDatas.size(); i++)
    {
        free(m_data->m_debugPointsDatas[i]);
    }

    m_data->m_heightfieldDatas.clear();
    m_data->m_allocatedTextures.clear();
    m_data->m_allocatedTexturesRequireFree.clear();
    m_data->m_debugPointsDatas.clear();
    m_data->m_meshInterfaces.clear();
    m_data->m_collisionShapes.clear();
    m_data->m_bulletCollisionShape2UrdfCollision.clear();
    m_data->m_graphicsIndexToSegmentationMask.clear();

    delete m_data->m_dynamicsWorld;
    m_data->m_dynamicsWorld = 0;

    delete m_data->m_remoteDebugDrawer;
    m_data->m_remoteDebugDrawer = 0;

#if !defined(SKIP_DEFORMABLE_BODY) && !defined(SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD)
    delete m_data->m_deformablebodySolver;
    m_data->m_deformablebodySolver = 0;
#endif

    delete m_data->m_solver;
    m_data->m_solver = 0;

    delete m_data->m_broadphase;
    m_data->m_broadphase = 0;

    delete m_data->m_pairCache;
    m_data->m_pairCache = 0;

    delete m_data->m_broadphaseCollisionFilterCallback;
    m_data->m_broadphaseCollisionFilterCallback = 0;

    delete m_data->m_dispatcher;
    m_data->m_dispatcher = 0;

    delete m_data->m_collisionConfiguration;
    m_data->m_collisionConfiguration = 0;
    m_data->m_userConstraintUIDGenerator = 1;
#ifdef STATIC_LINK_SPD_PLUGIN
    for (i32 i = 0; i < m_data->m_rbdModels.size(); i++)
    {
        delete *(m_data->m_rbdModels.getAtIndex(i));
    }
    m_data->m_rbdModels.clear();
#endif  //STATIC_LINK_SPD_PLUGIN
}

bool PhysicsServerCommandProcessor::supportsJointMotor(MultiBody* mb, i32 mbLinkIndex)
{
    bool canHaveMotor = (mb->getLink(mbLinkIndex).m_jointType == MultibodyLink::eRevolute || mb->getLink(mbLinkIndex).m_jointType == MultibodyLink::ePrismatic);
    return canHaveMotor;
}

//for testing, create joint motors for revolute and prismatic joints
void PhysicsServerCommandProcessor::createJointMotors(MultiBody* mb)
{
    i32 numLinks = mb->getNumLinks();
    for (i32 i = 0; i < numLinks; i++)
    {
        i32 mbLinkIndex = i;
        float maxMotorImpulse = 1.f;

        if (supportsJointMotor(mb, mbLinkIndex))
        {
            i32 dof = 0;
            Scalar desiredVelocity = 0.f;
            MultiBodyJointMotor* motor = new MultiBodyJointMotor(mb, mbLinkIndex, dof, desiredVelocity, maxMotorImpulse);
            motor->setPositionTarget(0, 0);
            motor->setVelocityTarget(0, 1);
            //motor->setRhsClamp(gRhsClamp);
            //motor->setMaxAppliedImpulse(0);
            mb->getLink(mbLinkIndex).m_userPtr = motor;
#ifndef USE_DISCRETE_DYNAMICS_WORLD
            m_data->m_dynamicsWorld->addMultiBodyConstraint(motor);
#endif
            motor->finalizeMultiDof();
        }
        if (mb->getLink(mbLinkIndex).m_jointType == MultibodyLink::eSpherical)
        {
            MultiBodySphericalJointMotor* motor = new MultiBodySphericalJointMotor(mb, mbLinkIndex, 1000 * maxMotorImpulse);
            mb->getLink(mbLinkIndex).m_userPtr = motor;
#ifndef USE_DISCRETE_DYNAMICS_WORLD
            m_data->m_dynamicsWorld->addMultiBodyConstraint(motor);
#endif
            motor->finalizeMultiDof();
        }
    }
}

i32 PhysicsServerCommandProcessor::addUserData(i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key, tukk valueBytes, i32 valueLength, i32 valueType)
{
    InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
    if (!body)
    {
        return -1;
    }

    SharedMemoryUserDataHashKey userDataIdentifier(key, bodyUniqueId, linkIndex, visualShapeIndex);

    i32* userDataHandlePtr = m_data->m_userDataHandleLookup.find(userDataIdentifier);
    i32 userDataHandle = userDataHandlePtr ? *userDataHandlePtr : m_data->m_userDataHandles.allocHandle();

    SharedMemoryUserData* userData = m_data->m_userDataHandles.getHandle(userDataHandle);
    if (!userData)
    {
        return -1;
    }

    if (!userDataHandlePtr)
    {
        userData->m_key = key;
        userData->m_bodyUniqueId = bodyUniqueId;
        userData->m_linkIndex = linkIndex;
        userData->m_visualShapeIndex = visualShapeIndex;
        m_data->m_userDataHandleLookup.insert(userDataIdentifier, userDataHandle);
        body->m_userDataHandles.push_back(userDataHandle);
    }
    userData->replaceValue(valueBytes, valueLength, valueType);
    return userDataHandle;
}

void PhysicsServerCommandProcessor::addUserData(const HashMap<HashString, STxt>& user_data_entries, i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex)
{
    for (i32 i = 0; i < user_data_entries.size(); ++i)
    {
        const STxt key = user_data_entries.getKeyAtIndex(i).m_string1;
        const STxt* value = user_data_entries.getAtIndex(i);
        if (value)
        {
            addUserData(bodyUniqueId, linkIndex, visualShapeIndex, key.c_str(), value->c_str(),
                        value->size() + 1, USER_DATA_VALUE_TYPE_STRING);
        }
    }
}

bool PhysicsServerCommandProcessor::processImportedObjects(tukk fileName, tuk bufferServerToClient, i32 bufferSizeInBytes, bool useMultiBody, i32 flags, URDFImporterInterface& u2b)
{
    bool loadOk = true;

    Transform2 rootTrans;
    rootTrans.setIdentity();
    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("loaded %s OK!", fileName);
    }
    SaveWorldObjectData sd;
    sd.m_fileName = fileName;

    for (i32 m = 0; m < u2b.getNumModels(); m++)
    {
        u2b.activateModel(m);
        MultiBody* mb = 0;
        RigidBody* rb = 0;

        //get a body index
        i32 bodyUniqueId = m_data->m_bodyHandles.allocHandle();

        InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);

        sd.m_bodyUniqueIds.push_back(bodyUniqueId);

        u2b.setBodyUniqueId(bodyUniqueId);
        {
            Scalar mass = 0;
            bodyHandle->m_rootLocalInertialFrame.setIdentity();
            bodyHandle->m_bodyName = u2b.getBodyName();
            Vec3 localInertiaDiagonal(0, 0, 0);
            i32 urdfLinkIndex = u2b.getRootLinkIndex();
            u2b.getMassAndInertia2(urdfLinkIndex, mass, localInertiaDiagonal, bodyHandle->m_rootLocalInertialFrame, flags);
        }

        //todo: move these internal API called inside the 'ConvertURDF2Bullet' call, hidden from the user
        //i32 rootLinkIndex = u2b.getRootLinkIndex();
        //drx3DPrintf("urdf root link index = %d\n",rootLinkIndex);
        MyMultiBodyCreator creation(m_data->m_guiHelper);

        u2b.getRootTransformInWorld(rootTrans);
        //CUF_RESERVED is a temporary flag, for backward compatibility purposes
        flags |= CUF_RESERVED;

        if (flags & CUF_ENABLE_CACHED_GRAPHICS_SHAPES)
        {
            {
                UrdfVisualShapeCache* tmpPtr = m_data->m_cachedVUrdfisualShapes[fileName];
                if (tmpPtr == 0)
                {
                    m_data->m_cachedVUrdfisualShapes.insert(fileName, UrdfVisualShapeCache());
                }
            }
            UrdfVisualShapeCache* cachedVisualShapesPtr = m_data->m_cachedVUrdfisualShapes[fileName];

            ConvertURDF2Bullet(u2b, creation, rootTrans, m_data->m_dynamicsWorld, useMultiBody, u2b.getPathPrefix(), flags, cachedVisualShapesPtr);

        }
        else
        {

            ConvertURDF2Bullet(u2b, creation, rootTrans, m_data->m_dynamicsWorld, useMultiBody, u2b.getPathPrefix(), flags);

        }

        mb = creation.getBulletMultiBody();
        rb = creation.getRigidBody();
        if (rb)
            rb->setUserIndex2(bodyUniqueId);

        if (mb)
            mb->setUserIndex2(bodyUniqueId);

        if (mb)
        {
            bodyHandle->m_multiBody = mb;

            m_data->m_sdfRecentLoadedBodies.push_back(bodyUniqueId);

            i32 segmentationMask = bodyUniqueId;

            {
                i32 graphicsIndex = -1;
                if (mb->getBaseCollider())
                {
                    graphicsIndex = mb->getBaseCollider()->getUserIndex();
                }
                if (graphicsIndex >= 0)
                {
                    if (m_data->m_graphicsIndexToSegmentationMask.size() < (graphicsIndex + 1))
                    {
                        m_data->m_graphicsIndexToSegmentationMask.resize(graphicsIndex + 1);
                    }
                    m_data->m_graphicsIndexToSegmentationMask[graphicsIndex] = segmentationMask;
                }
            }

            createJointMotors(mb);

#ifdef D3_ENABLE_TINY_AUDIO
            {
                SDFAudioSource audioSource;
                i32 urdfRootLink = u2b.getRootLinkIndex();  //LinkIndex = creation.m_mb2urdfLink[-1];
                if (u2b.getLinkAudioSource(urdfRootLink, audioSource))
                {
                    i32 flags = mb->getBaseCollider()->getCollisionFlags();
                    mb->getBaseCollider()->setCollisionFlags(flags | CollisionObject2::CF_HAS_COLLISION_SOUND_TRIGGER);
                    audioSource.m_userIndex = m_data->m_soundEngine.loadWavFile(audioSource.m_uri.c_str());
                    if (audioSource.m_userIndex >= 0)
                    {
                        bodyHandle->m_audioSources.insert(-1, audioSource);
                    }
                }
            }
#endif
            //disable serialization of the collision objects (they are too big, and the client likely doesn't need them);

            bodyHandle->m_linkLocalInertialFrames.reserve(mb->getNumLinks());
            for (i32 i = 0; i < mb->getNumLinks(); i++)
            {
                //disable serialization of the collision objects

                i32 urdfLinkIndex = creation.m_mb2urdfLink[i];
                Scalar mass;
                Vec3 localInertiaDiagonal(0, 0, 0);
                Transform2 localInertialFrame;
                u2b.getMassAndInertia2(urdfLinkIndex, mass, localInertiaDiagonal, localInertialFrame, flags);
                bodyHandle->m_linkLocalInertialFrames.push_back(localInertialFrame);

                STxt* linkName = new STxt(u2b.getLinkName(urdfLinkIndex).c_str());
                m_data->m_strings.push_back(linkName);

                mb->getLink(i).m_linkName = linkName->c_str();

                {
                    i32 graphicsIndex = -1;
                    if (mb->getLinkCollider(i))
                    {
                        graphicsIndex = mb->getLinkCollider(i)->getUserIndex();
                    }
                    if (graphicsIndex >= 0)
                    {
                        i32 linkIndex = i;
                        if (m_data->m_graphicsIndexToSegmentationMask.size() < (graphicsIndex + 1))
                        {
                            m_data->m_graphicsIndexToSegmentationMask.resize(graphicsIndex + 1);
                        }
                        i32 segmentationMask = bodyUniqueId + ((linkIndex + 1) << 24);
                        m_data->m_graphicsIndexToSegmentationMask[graphicsIndex] = segmentationMask;
                    }
                }

                STxt* jointName = new STxt(u2b.getJointName(urdfLinkIndex).c_str());
                m_data->m_strings.push_back(jointName);

                mb->getLink(i).m_jointName = jointName->c_str();

#ifdef D3_ENABLE_TINY_AUDIO
                {
                    SDFAudioSource audioSource;
                    i32 urdfLinkIndex = creation.m_mb2urdfLink[i];
                    if (u2b.getLinkAudioSource(urdfLinkIndex, audioSource))
                    {
                        i32 flags = mb->getLink(i).m_collider->getCollisionFlags();
                        mb->getLink(i).m_collider->setCollisionFlags(flags | CollisionObject2::CF_HAS_COLLISION_SOUND_TRIGGER);
                        audioSource.m_userIndex = m_data->m_soundEngine.loadWavFile(audioSource.m_uri.c_str());
                        if (audioSource.m_userIndex >= 0)
                        {
                            bodyHandle->m_audioSources.insert(i, audioSource);
                        }
                    }
                }
#endif
            }
            STxt* baseName = new STxt(u2b.getLinkName(u2b.getRootLinkIndex()));
            m_data->m_strings.push_back(baseName);
            mb->setBaseName(baseName->c_str());

#if 0
            AlignedObjectArray<char> urdf;
            mb->dumpUrdf(urdf);
            FILE* f = fopen("e:/pybullet.urdf", "w");
            if (f)
            {
                fwrite(&urdf[0], urdf.size(), 1, f);
                fclose(f);
            }
#endif
        }
        else
        {
            i32 segmentationMask = bodyUniqueId;
            if (rb)
            {
                i32 graphicsIndex = -1;
                {
                    graphicsIndex = rb->getUserIndex();
                }
                if (graphicsIndex >= 0)
                {
                    if (m_data->m_graphicsIndexToSegmentationMask.size() < (graphicsIndex + 1))
                    {
                        m_data->m_graphicsIndexToSegmentationMask.resize(graphicsIndex + 1);
                    }
                    m_data->m_graphicsIndexToSegmentationMask[graphicsIndex] = segmentationMask;
                }
            }

            //drx3DWarning("No multibody loaded from URDF. Could add RigidBody+btTypedConstraint solution later.");
            bodyHandle->m_rigidBody = rb;
            rb->setUserIndex2(bodyUniqueId);
            m_data->m_sdfRecentLoadedBodies.push_back(bodyUniqueId);

            STxt* baseName = new STxt(u2b.getLinkName(u2b.getRootLinkIndex()));
            m_data->m_strings.push_back(baseName);
            bodyHandle->m_bodyName = *baseName;

            i32 numJoints = creation.getNum6DofConstraints();
            bodyHandle->m_rigidBodyJoints.reserve(numJoints);
            bodyHandle->m_rigidBodyJointNames.reserve(numJoints);
            bodyHandle->m_rigidBodyLinkNames.reserve(numJoints);
            for (i32 i = 0; i < numJoints; i++)
            {
                i32 urdfLinkIndex = creation.m_mb2urdfLink[i];

                Generic6DofSpring2Constraint* con = creation.get6DofConstraint(i);

                STxt* linkName = new STxt(u2b.getLinkName(urdfLinkIndex).c_str());
                m_data->m_strings.push_back(linkName);

                STxt* jointName = new STxt(u2b.getJointName(urdfLinkIndex).c_str());
                m_data->m_strings.push_back(jointName);

                bodyHandle->m_rigidBodyJointNames.push_back(*jointName);
                bodyHandle->m_rigidBodyLinkNames.push_back(*linkName);

                bodyHandle->m_rigidBodyJoints.push_back(con);
            }
        }

        {
            if (m_data->m_pluginManager.getRenderInterface())
            {
                i32 currentOpenGLTextureIndex = 0;
                i32 totalNumVisualShapes = m_data->m_pluginManager.getRenderInterface()->getNumVisualShapes(bodyUniqueId);

                for (i32 shapeIndex = 0; shapeIndex < totalNumVisualShapes; shapeIndex++)
                {
                    b3VisualShapeData tmpShape;
                    i32 success = m_data->m_pluginManager.getRenderInterface()->getVisualShapesData(bodyUniqueId, shapeIndex, &tmpShape);
                    if (success)
                    {
                        if (tmpShape.m_tinyRendererTextureId >= 0)
                        {
                            i32 openglTextureUniqueId = -1;

                            //find companion opengl texture unique id and create a 'textureUid'
                            if (currentOpenGLTextureIndex < u2b.getNumAllocatedTextures())
                            {
                                openglTextureUniqueId = u2b.getAllocatedTexture(currentOpenGLTextureIndex++);
                            }
                            //if (openglTextureUniqueId>=0)
                            {
                                i32 texHandle = m_data->m_textureHandles.allocHandle();
                                InternalTextureHandle* texH = m_data->m_textureHandles.getHandle(texHandle);
                                if (texH)
                                {
                                    texH->m_tinyRendererTextureId = tmpShape.m_tinyRendererTextureId;
                                    texH->m_openglTextureId = openglTextureUniqueId;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Add user data specified in URDF to the added body.
        const UrdfModel* urdfModel = u2b.getUrdfModel();
        if (urdfModel)
        {
            addUserData(urdfModel->m_userData, bodyUniqueId);
            if (bodyHandle->m_multiBody)
            {
                MultiBody* mb = bodyHandle->m_multiBody;
                // Because the link order between UrdfModel and MultiBody may be different,
                // create a mapping from link name to link index in order to apply the user
                // data to the correct link in the MultiBody.
                HashMap<HashString, i32> linkNameToIndexMap;
                linkNameToIndexMap.insert(mb->getBaseName(), -1);
                for (i32 linkIndex = 0; linkIndex < mb->getNumLinks(); ++linkIndex)
                {
                    linkNameToIndexMap.insert(mb->getLink(linkIndex).m_linkName, linkIndex);
                }
                for (i32 i = 0; i < urdfModel->m_links.size(); ++i)
                {
                    const UrdfLink* link = *urdfModel->m_links.getAtIndex(i);
                    i32* linkIndex = linkNameToIndexMap.find(link->m_name.c_str());
                    if (linkIndex)
                    {
                        addUserData(link->m_userData, bodyUniqueId, *linkIndex);
                        for (i32 visualShapeIndex = 0; visualShapeIndex < link->m_visualArray.size(); ++visualShapeIndex)
                        {
                            addUserData(link->m_visualArray.at(visualShapeIndex).m_userData, bodyUniqueId, *linkIndex, visualShapeIndex);
                        }
                    }
                }
            }
            else if (bodyHandle->m_rigidBody)
            {
                for (i32 i = 0; i < urdfModel->m_links.size(); ++i)
                {
                    const UrdfLink* link = *urdfModel->m_links.getAtIndex(i);
                    addUserData(link->m_userData, bodyUniqueId, -1);
                    for (i32 visualShapeIndex = 0; visualShapeIndex < link->m_visualArray.size(); ++visualShapeIndex)
                    {
                        addUserData(link->m_visualArray.at(visualShapeIndex).m_userData, bodyUniqueId, -1, visualShapeIndex);
                    }
                }
            }
        }

        b3Notification notification;
        notification.m_notificationType = BODY_ADDED;
        notification.m_bodyArgs.m_bodyUniqueId = bodyUniqueId;
        m_data->m_pluginManager.addNotification(notification);
    }

    for (i32 i = 0; i < u2b.getNumAllocatedTextures(); i++)
    {
        i32 texId = u2b.getAllocatedTexture(i);
        m_data->m_allocatedTextures.push_back(texId);
    }

    /*
 i32 texHandle = m_data->m_textureHandles.allocHandle();
 InternalTextureHandle* texH = m_data->m_textureHandles.getHandle(texHandle);
 if(texH)
 {
 texH->m_tinyRendererTextureId = -1;
 texH->m_openglTextureId = -1;
 */

    for (i32 i = 0; i < u2b.getNumAllocatedMeshInterfaces(); i++)
    {
        m_data->m_meshInterfaces.push_back(u2b.getAllocatedMeshInterface(i));
    }

    for (i32 i = 0; i < u2b.getNumAllocatedCollisionShapes(); i++)
    {
        CollisionShape* shape = u2b.getAllocatedCollisionShape(i);
        m_data->m_collisionShapes.push_back(shape);
        UrdfCollision urdfCollision;
        if (u2b.getUrdfFromCollisionShape(shape, urdfCollision))
        {
            m_data->m_bulletCollisionShape2UrdfCollision.insert(shape, urdfCollision);
        }
        if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
        {
            CompoundShape* compound = (CompoundShape*)shape;
            for (i32 c = 0; c < compound->getNumChildShapes(); c++)
            {
                CollisionShape* childShape = compound->getChildShape(c);
                if (u2b.getUrdfFromCollisionShape(childShape, urdfCollision))
                {
                    m_data->m_bulletCollisionShape2UrdfCollision.insert(childShape, urdfCollision);
                }
            }
        }
    }

    m_data->m_saveWorldBodyData.push_back(sd);

    syncPhysicsToGraphics2();
    return loadOk;
}

struct MyMJCFLogger2 : public MJCFErrorLogger
{
    virtual void reportError(tukk error)
    {
        drx3DError(error);
    }
    virtual void reportWarning(tukk warning)
    {
        drx3DWarning(warning);
    }
    virtual void printMessage(tukk msg)
    {
        drx3DPrintf(msg);
    }
};

bool PhysicsServerCommandProcessor::loadMjcf(tukk fileName, tuk bufferServerToClient, i32 bufferSizeInBytes, bool useMultiBody, i32 flags)
{
    Assert(m_data->m_dynamicsWorld);
    if (!m_data->m_dynamicsWorld)
    {
        drx3DError("loadSdf: No valid m_dynamicsWorld");
        return false;
    }

    m_data->m_sdfRecentLoadedBodies.clear();

    CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
    BulletMJCFImporter u2b(m_data->m_guiHelper, m_data->m_pluginManager.getRenderInterface(), fileIO, flags);

    bool useFixedBase = false;
    MyMJCFLogger2 logger;
    bool loadOk = u2b.loadMJCF(fileName, &logger, useFixedBase);
    if (loadOk)
    {
        processImportedObjects(fileName, bufferServerToClient, bufferSizeInBytes, useMultiBody, flags, u2b);
    }
    return loadOk;
}

bool PhysicsServerCommandProcessor::loadSdf(tukk fileName, tuk bufferServerToClient, i32 bufferSizeInBytes, bool useMultiBody, i32 flags, Scalar globalScaling)
{
    Assert(m_data->m_dynamicsWorld);
    if (!m_data->m_dynamicsWorld)
    {
        drx3DError("loadSdf: No valid m_dynamicsWorld");
        return false;
    }

    m_data->m_sdfRecentLoadedBodies.clear();
    CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
    URDFImporter u2b(m_data->m_guiHelper, m_data->m_pluginManager.getRenderInterface(), fileIO, globalScaling, flags);
    u2b.setEnableTinyRenderer(m_data->m_enableTinyRenderer);

    bool forceFixedBase = false;
    bool loadOk = u2b.loadSDF(fileName, forceFixedBase);

    if (loadOk)
    {
        processImportedObjects(fileName, bufferServerToClient, bufferSizeInBytes, useMultiBody, flags, u2b);
    }
    return loadOk;
}

bool PhysicsServerCommandProcessor::loadUrdf(tukk fileName, const Vec3& pos, const Quat& orn,
                                             bool useMultiBody, bool useFixedBase, i32* bodyUniqueIdPtr, tuk bufferServerToClient, i32 bufferSizeInBytes, i32 orgFlags, Scalar globalScaling)
{
    //clear the LOAD_SDF_FILE=1 bit, which is reserved for internal use of loadSDF command.
    i32 flags = orgFlags & ~1;
    m_data->m_sdfRecentLoadedBodies.clear();
    *bodyUniqueIdPtr = -1;

    DRX3D_PROFILE("loadURDF");
    Assert(m_data->m_dynamicsWorld);
    if (!m_data->m_dynamicsWorld)
    {
        drx3DError("loadUrdf: No valid m_dynamicsWorld");
        return false;
    }

    CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
    URDFImporter u2b(m_data->m_guiHelper, m_data->m_pluginManager.getRenderInterface(), fileIO, globalScaling, flags);
    u2b.setEnableTinyRenderer(m_data->m_enableTinyRenderer);
    bool loadOk = u2b.loadURDF(fileName, useFixedBase);

    if (loadOk)
    {
        Transform2 rootTrans;
        rootTrans.setOrigin(pos);
        rootTrans.setRotation(orn);
        u2b.setRootTransformInWorld(rootTrans);
        if (!(u2b.getDeformableModel().m_visualFileName.empty()))
        {
            bool use_self_collision = false;
            use_self_collision = (flags & CUF_USE_SELF_COLLISION);
            bool ok = processDeformable(u2b.getDeformableModel(), pos, orn, bodyUniqueIdPtr, bufferServerToClient, bufferSizeInBytes, globalScaling, use_self_collision);
            if (ok)
            {
                const UrdfModel* urdfModel = u2b.getUrdfModel();
                if (urdfModel)
                {
                    addUserData(urdfModel->m_userData, *bodyUniqueIdPtr);
                }
                return true;
            }
            else
            {
                return false;
            }
        }
        if (!(u2b.getReducedDeformableModel().m_visualFileName.empty()))
        {
            bool use_self_collision = false;
            return processReducedDeformable(u2b.getReducedDeformableModel(), pos, orn, bodyUniqueIdPtr, bufferServerToClient, bufferSizeInBytes, globalScaling, use_self_collision);
        }
        bool ok = processImportedObjects(fileName, bufferServerToClient, bufferSizeInBytes, useMultiBody, flags, u2b);
        if (ok)
        {
            if (m_data->m_sdfRecentLoadedBodies.size() == 1)
            {
                *bodyUniqueIdPtr = m_data->m_sdfRecentLoadedBodies[0];
            }
            m_data->m_sdfRecentLoadedBodies.clear();
        }
        return ok;
    }
    return false;
}

void PhysicsServerCommandProcessor::replayLogCommand(tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    if (m_data->m_logPlayback)
    {
        SharedMemoryCommand clientCmd;
        SharedMemoryStatus serverStatus;

        bool hasCommand = m_data->m_logPlayback->processNextCommand(&clientCmd);
        if (hasCommand)
        {
            processCommand(clientCmd, serverStatus, bufferServerToClient, bufferSizeInBytes);
        }
    }
}

i32 PhysicsServerCommandProcessor::createBodyInfoStream(i32 bodyUniqueId, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    i32 streamSizeInBytes = 0;
    //serialize the btMultiBody and send the data to the client. This is one way to get the link/joint names across the (shared memory) wire

    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);
    if (!bodyHandle) return 0;
    if (bodyHandle->m_multiBody)
    {
        MultiBody* mb = bodyHandle->m_multiBody;
        DefaultSerializer ser(bufferSizeInBytes, (u8*)bufferServerToClient);

        ser.startSerialization();

        //disable serialization of the collision objects (they are too big, and the client likely doesn't need them);
        ser.m_skipPointers.insert(mb->getBaseCollider(), 0);
        if (mb->getBaseName())
        {
            ser.registerNameForPointer(mb->getBaseName(), mb->getBaseName());
        }

        bodyHandle->m_linkLocalInertialFrames.reserve(mb->getNumLinks());
        for (i32 i = 0; i < mb->getNumLinks(); i++)
        {
            //disable serialization of the collision objects
            ser.m_skipPointers.insert(mb->getLink(i).m_collider, 0);
            ser.registerNameForPointer(mb->getLink(i).m_linkName, mb->getLink(i).m_linkName);
            ser.registerNameForPointer(mb->getLink(i).m_jointName, mb->getLink(i).m_jointName);
        }

        ser.registerNameForPointer(mb->getBaseName(), mb->getBaseName());

        i32 len = mb->calculateSerializeBufferSize();
        Chunk* chunk = ser.allocate(len, 1);
        tukk structType = mb->serialize(chunk->m_oldPtr, &ser);
        ser.finalizeChunk(chunk, structType, DRX3D_MULTIBODY_CODE, mb);
        streamSizeInBytes = ser.getCurrentBufferSize();
    }
    else if (bodyHandle->m_rigidBody)
    {
        RigidBody* rb = bodyHandle->m_rigidBody;
        DefaultSerializer ser(bufferSizeInBytes, (u8*)bufferServerToClient);

        ser.startSerialization();
        ser.registerNameForPointer(bodyHandle->m_rigidBody, bodyHandle->m_bodyName.c_str());
        //disable serialization of the collision objects (they are too big, and the client likely doesn't need them);
        for (i32 i = 0; i < bodyHandle->m_rigidBodyJoints.size(); i++)
        {
            const Generic6DofSpring2Constraint* con = bodyHandle->m_rigidBodyJoints.at(i);

            ser.registerNameForPointer(con, bodyHandle->m_rigidBodyJointNames[i].c_str());
            ser.registerNameForPointer(&con->getRigidBodyB(), bodyHandle->m_rigidBodyLinkNames[i].c_str());

            const RigidBody& bodyA = con->getRigidBodyA();

            i32 len = con->calculateSerializeBufferSize();
            Chunk* chunk = ser.allocate(len, 1);
            tukk structType = con->serialize(chunk->m_oldPtr, &ser);
            ser.finalizeChunk(chunk, structType, DRX3D_CONSTRAINT_CODE, (uk )con);
        }
        streamSizeInBytes = ser.getCurrentBufferSize();
    }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    else if (bodyHandle->m_softBody)
    {
        //minimum serialization, registerNameForPointer
        SoftBody* sb = bodyHandle->m_softBody;
        DefaultSerializer ser(bufferSizeInBytes, (u8*)bufferServerToClient);

        ser.startSerialization();
        i32 len = sb->calculateSerializeBufferSize();
        Chunk* chunk = ser.allocate(len, 1);
        tukk structType = sb->serialize(chunk->m_oldPtr, &ser);
        ser.finalizeChunk(chunk, structType, DRX3D_SOFTBODY_CODE, sb);
        streamSizeInBytes = ser.getCurrentBufferSize();
    }
#endif
    return streamSizeInBytes;
}

bool PhysicsServerCommandProcessor::processStateLoggingCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    DRX3D_PROFILE("CMD_STATE_LOGGING");

    serverStatusOut.m_type = CMD_STATE_LOGGING_FAILED;
    bool hasStatus = true;

    if (clientCmd.m_updateFlags & STATE_LOGGING_START_LOG)
    {
        if (clientCmd.m_stateLoggingArguments.m_logType == STATE_LOGGING_ALL_COMMANDS)
        {
            if (m_data->m_commandLogger == 0)
            {
                enableCommandLogging(true, clientCmd.m_stateLoggingArguments.m_fileName);
                serverStatusOut.m_type = CMD_STATE_LOGGING_START_COMPLETED;
                i32 loggerUid = m_data->m_stateLoggersUniqueId++;
                m_data->m_commandLoggingUid = loggerUid;
                serverStatusOut.m_stateLoggingResultArgs.m_loggingUniqueId = loggerUid;
            }
        }

        if (clientCmd.m_stateLoggingArguments.m_logType == STATE_REPLAY_ALL_COMMANDS)
        {
            if (m_data->m_logPlayback == 0)
            {
                replayFromLogFile(clientCmd.m_stateLoggingArguments.m_fileName);
                serverStatusOut.m_type = CMD_STATE_LOGGING_START_COMPLETED;
                i32 loggerUid = m_data->m_stateLoggersUniqueId++;
                m_data->m_logPlaybackUid = loggerUid;
                serverStatusOut.m_stateLoggingResultArgs.m_loggingUniqueId = loggerUid;
            }
        }

        if (clientCmd.m_stateLoggingArguments.m_logType == STATE_LOGGING_PROFILE_TIMINGS)
        {
            if (m_data->m_profileTimingLoggingUid < 0)
            {
                b3ChromeUtilsStartTimings();
                m_data->m_profileTimingFileName = clientCmd.m_stateLoggingArguments.m_fileName;
                i32 loggerUid = m_data->m_stateLoggersUniqueId++;
                serverStatusOut.m_type = CMD_STATE_LOGGING_START_COMPLETED;
                serverStatusOut.m_stateLoggingResultArgs.m_loggingUniqueId = loggerUid;
                m_data->m_profileTimingLoggingUid = loggerUid;
            }
        }
        if (clientCmd.m_stateLoggingArguments.m_logType == STATE_LOGGING_VIDEO_MP4)
        {
            //if (clientCmd.m_stateLoggingArguments.m_fileName)
            {
                i32 loggerUid = m_data->m_stateLoggersUniqueId++;
                VideoMP4Loggger* logger = new VideoMP4Loggger(loggerUid, clientCmd.m_stateLoggingArguments.m_fileName, this->m_data->m_guiHelper);
                m_data->m_stateLoggers.push_back(logger);
                serverStatusOut.m_type = CMD_STATE_LOGGING_START_COMPLETED;
                serverStatusOut.m_stateLoggingResultArgs.m_loggingUniqueId = loggerUid;
            }
        }

        if (clientCmd.m_stateLoggingArguments.m_logType == STATE_LOGGING_MINITAUR)
        {
            STxt fileName = clientCmd.m_stateLoggingArguments.m_fileName;
            //either provide the minitaur by object unique Id, or search for first multibody with 8 motors...

            if ((clientCmd.m_updateFlags & STATE_LOGGING_FILTER_OBJECT_UNIQUE_ID) && (clientCmd.m_stateLoggingArguments.m_numBodyUniqueIds > 0))
            {
                i32 bodyUniqueId = clientCmd.m_stateLoggingArguments.m_bodyUniqueIds[0];
                InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
                if (body)
                {
                    if (body->m_multiBody)
                    {
                        AlignedObjectArray<STxt> motorNames;
                        motorNames.push_back("motor_front_leftR_joint");
                        motorNames.push_back("motor_front_leftL_joint");
                        motorNames.push_back("motor_back_leftR_joint");
                        motorNames.push_back("motor_back_leftL_joint");
                        motorNames.push_back("motor_front_rightL_joint");
                        motorNames.push_back("motor_front_rightR_joint");
                        motorNames.push_back("motor_back_rightL_joint");
                        motorNames.push_back("motor_back_rightR_joint");

                        AlignedObjectArray<i32> motorIdList;
                        for (i32 m = 0; m < motorNames.size(); m++)
                        {
                            for (i32 i = 0; i < body->m_multiBody->getNumLinks(); i++)
                            {
                                STxt jointName;
                                if (body->m_multiBody->getLink(i).m_jointName)
                                {
                                    jointName = body->m_multiBody->getLink(i).m_jointName;
                                }
                                if (motorNames[m] == jointName)
                                {
                                    motorIdList.push_back(i);
                                }
                            }
                        }

                        if (motorIdList.size() == 8)
                        {
                            i32 loggerUid = m_data->m_stateLoggersUniqueId++;
                            MinitaurStateLogger* logger = new MinitaurStateLogger(loggerUid, fileName, body->m_multiBody, motorIdList);
                            m_data->m_stateLoggers.push_back(logger);
                            serverStatusOut.m_type = CMD_STATE_LOGGING_START_COMPLETED;
                            serverStatusOut.m_stateLoggingResultArgs.m_loggingUniqueId = loggerUid;
                        }
                    }
                }
            }
        }
#ifndef USE_DISCRETE_DYNAMICS_WORLD
        if (clientCmd.m_stateLoggingArguments.m_logType == STATE_LOGGING_GENERIC_ROBOT)
        {

            STxt fileName = clientCmd.m_stateLoggingArguments.m_fileName;

            i32 loggerUid = m_data->m_stateLoggersUniqueId++;
            i32 maxLogDof = 12;
            if ((clientCmd.m_updateFlags & STATE_LOGGING_MAX_LOG_DOF))
            {
                maxLogDof = clientCmd.m_stateLoggingArguments.m_maxLogDof;
            }

            i32 logFlags = 0;
            if (clientCmd.m_updateFlags & STATE_LOGGING_LOG_FLAGS)
            {
                logFlags = clientCmd.m_stateLoggingArguments.m_logFlags;
            }
            GenericRobotStateLogger* logger = new GenericRobotStateLogger(loggerUid, fileName, m_data->m_dynamicsWorld, maxLogDof, logFlags);

            if ((clientCmd.m_updateFlags & STATE_LOGGING_FILTER_OBJECT_UNIQUE_ID) && (clientCmd.m_stateLoggingArguments.m_numBodyUniqueIds > 0))
            {
                logger->m_filterObjectUniqueId = true;
                for (i32 i = 0; i < clientCmd.m_stateLoggingArguments.m_numBodyUniqueIds; ++i)
                {
                    i32 objectUniqueId = clientCmd.m_stateLoggingArguments.m_bodyUniqueIds[i];
                    logger->m_bodyIdList.push_back(objectUniqueId);
                }
            }

            m_data->m_stateLoggers.push_back(logger);
            serverStatusOut.m_type = CMD_STATE_LOGGING_START_COMPLETED;
            serverStatusOut.m_stateLoggingResultArgs.m_loggingUniqueId = loggerUid;
        }
        if (clientCmd.m_stateLoggingArguments.m_logType == STATE_LOGGING_CONTACT_POINTS)
        {
            STxt fileName = clientCmd.m_stateLoggingArguments.m_fileName;
            i32 loggerUid = m_data->m_stateLoggersUniqueId++;
            ContactPointsStateLogger* logger = new ContactPointsStateLogger(loggerUid, fileName, m_data->m_dynamicsWorld);
            if ((clientCmd.m_updateFlags & STATE_LOGGING_FILTER_LINK_INDEX_A) && clientCmd.m_stateLoggingArguments.m_linkIndexA >= -1)
            {
                logger->m_filterLinkA = true;
                logger->m_linkIndexA = clientCmd.m_stateLoggingArguments.m_linkIndexA;
            }
            if ((clientCmd.m_updateFlags & STATE_LOGGING_FILTER_LINK_INDEX_B) && clientCmd.m_stateLoggingArguments.m_linkIndexB >= -1)
            {
                logger->m_filterLinkB = true;
                logger->m_linkIndexB = clientCmd.m_stateLoggingArguments.m_linkIndexB;
            }
            if ((clientCmd.m_updateFlags & STATE_LOGGING_FILTER_BODY_UNIQUE_ID_A) && clientCmd.m_stateLoggingArguments.m_bodyUniqueIdA > -1)
            {
                logger->m_bodyUniqueIdA = clientCmd.m_stateLoggingArguments.m_bodyUniqueIdA;
            }
            if ((clientCmd.m_updateFlags & STATE_LOGGING_FILTER_BODY_UNIQUE_ID_B) && clientCmd.m_stateLoggingArguments.m_bodyUniqueIdB > -1)
            {
                logger->m_bodyUniqueIdB = clientCmd.m_stateLoggingArguments.m_bodyUniqueIdB;
            }
            m_data->m_stateLoggers.push_back(logger);
            serverStatusOut.m_type = CMD_STATE_LOGGING_START_COMPLETED;
            serverStatusOut.m_stateLoggingResultArgs.m_loggingUniqueId = loggerUid;
        }
#endif
        if (clientCmd.m_stateLoggingArguments.m_logType == STATE_LOGGING_VR_CONTROLLERS)
        {
            STxt fileName = clientCmd.m_stateLoggingArguments.m_fileName;
            i32 loggerUid = m_data->m_stateLoggersUniqueId++;
            i32 deviceFilterType = VR_DEVICE_CONTROLLER;
            if (clientCmd.m_updateFlags & STATE_LOGGING_FILTER_DEVICE_TYPE)
            {
                deviceFilterType = clientCmd.m_stateLoggingArguments.m_deviceFilterType;
            }
            VRControllerStateLogger* logger = new VRControllerStateLogger(loggerUid, deviceFilterType, fileName);
            m_data->m_stateLoggers.push_back(logger);
            serverStatusOut.m_type = CMD_STATE_LOGGING_START_COMPLETED;
            serverStatusOut.m_stateLoggingResultArgs.m_loggingUniqueId = loggerUid;
        }
    }
    if ((clientCmd.m_updateFlags & STATE_LOGGING_STOP_LOG) && clientCmd.m_stateLoggingArguments.m_loggingUniqueId >= 0)
    {
        if (clientCmd.m_stateLoggingArguments.m_loggingUniqueId == m_data->m_logPlaybackUid)
        {
            if (m_data->m_logPlayback)
            {
                delete m_data->m_logPlayback;
                m_data->m_logPlayback = 0;
                m_data->m_logPlaybackUid = -1;
            }
        }

        if (clientCmd.m_stateLoggingArguments.m_loggingUniqueId == m_data->m_commandLoggingUid)
        {
            if (m_data->m_commandLogger)
            {
                enableCommandLogging(false, 0);
                serverStatusOut.m_type = CMD_STATE_LOGGING_COMPLETED;
                m_data->m_commandLoggingUid = -1;
            }
        }

        if (clientCmd.m_stateLoggingArguments.m_loggingUniqueId == m_data->m_profileTimingLoggingUid)
        {
            serverStatusOut.m_type = CMD_STATE_LOGGING_COMPLETED;
            b3ChromeUtilsStopTimingsAndWriteJsonFile(m_data->m_profileTimingFileName.c_str());
            m_data->m_profileTimingLoggingUid = -1;
        }
        else
        {
            serverStatusOut.m_type = CMD_STATE_LOGGING_COMPLETED;
            for (i32 i = 0; i < m_data->m_stateLoggers.size(); i++)
            {
                if (m_data->m_stateLoggers[i]->m_loggingUniqueId == clientCmd.m_stateLoggingArguments.m_loggingUniqueId)
                {
                    m_data->m_stateLoggers[i]->stop();
                    delete m_data->m_stateLoggers[i];
                    m_data->m_stateLoggers.removeAtIndex(i);
                }
            }
        }
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestCameraImageCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REQUEST_CAMERA_IMAGE_DATA");
    i32 startPixelIndex = clientCmd.m_requestPixelDataArguments.m_startPixelIndex;
    i32 width = clientCmd.m_requestPixelDataArguments.m_pixelWidth;
    i32 height = clientCmd.m_requestPixelDataArguments.m_pixelHeight;
    i32 numPixelsCopied = 0;

    if ((clientCmd.m_requestPixelDataArguments.m_startPixelIndex == 0) &&
        (clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_PIXEL_WIDTH_HEIGHT) != 0)
    {
        if (m_data->m_pluginManager.getRenderInterface())
        {
            m_data->m_pluginManager.getRenderInterface()->setWidthAndHeight(clientCmd.m_requestPixelDataArguments.m_pixelWidth,
                                                                            clientCmd.m_requestPixelDataArguments.m_pixelHeight);
        }
    }
    i32 flags = 0;
    if (clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_HAS_FLAGS)
    {
        flags = clientCmd.m_requestPixelDataArguments.m_flags;
    }
    if (m_data->m_pluginManager.getRenderInterface())
    {
        m_data->m_pluginManager.getRenderInterface()->setFlags(flags);
    }

    i32 numTotalPixels = width * height;
    i32 numRemainingPixels = numTotalPixels - startPixelIndex;

    if (numRemainingPixels > 0)
    {
        i32 totalBytesPerPixel = 4 + 4 + 4;  //4 for rgb, 4 for depth, 4 for segmentation mask
        i32 maxNumPixels = bufferSizeInBytes / totalBytesPerPixel - 1;
        u8* pixelRGBA = (u8*)bufferServerToClient;
        i32 numRequestedPixels = d3Min(maxNumPixels, numRemainingPixels);

        float* depthBuffer = (float*)(bufferServerToClient + numRequestedPixels * 4);
        i32* segmentationMaskBuffer = (i32*)(bufferServerToClient + numRequestedPixels * 8);

        serverStatusOut.m_numDataStreamBytes = numRequestedPixels * totalBytesPerPixel;
        float viewMat[16];
        float projMat[16];
        float projTextureViewMat[16];
        float projTextureProjMat[16];
        for (i32 i = 0; i < 16; i++)
        {
            viewMat[i] = clientCmd.m_requestPixelDataArguments.m_viewMatrix[i];
            projMat[i] = clientCmd.m_requestPixelDataArguments.m_projectionMatrix[i];
        }
        if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES) == 0)
        {
            b3OpenGLVisualizerCameraInfo tmpCamResult;
            bool result = this->m_data->m_guiHelper->getCameraInfo(
                &tmpCamResult.m_width,
                &tmpCamResult.m_height,
                tmpCamResult.m_viewMatrix,
                tmpCamResult.m_projectionMatrix,
                tmpCamResult.m_camUp,
                tmpCamResult.m_camForward,
                tmpCamResult.m_horizontal,
                tmpCamResult.m_vertical,
                &tmpCamResult.m_yaw,
                &tmpCamResult.m_pitch,
                &tmpCamResult.m_dist,
                tmpCamResult.m_target);
            if (result)
            {
                for (i32 i = 0; i < 16; i++)
                {
                    viewMat[i] = tmpCamResult.m_viewMatrix[i];
                    projMat[i] = tmpCamResult.m_projectionMatrix[i];
                }
            }
        }
        bool handled = false;

        if ((clientCmd.m_updateFlags & ER_DRX3D_HARDWARE_OPENGL) != 0)
        {
            if ((flags & ER_USE_PROJECTIVE_TEXTURE) != 0)
            {
                this->m_data->m_guiHelper->setProjectiveTexture(true);
                if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_HAS_PROJECTIVE_TEXTURE_MATRICES) != 0)
                {
                    for (i32 i = 0; i < 16; i++)
                    {
                        projTextureViewMat[i] = clientCmd.m_requestPixelDataArguments.m_projectiveTextureViewMatrix[i];
                        projTextureProjMat[i] = clientCmd.m_requestPixelDataArguments.m_projectiveTextureProjectionMatrix[i];
                    }
                }
                else  // If no specified matrices for projective texture, then use the camera matrices.
                {
                    for (i32 i = 0; i < 16; i++)
                    {
                        projTextureViewMat[i] = viewMat[i];
                        projTextureProjMat[i] = projMat[i];
                    }
                }
                this->m_data->m_guiHelper->setProjectiveTextureMatrices(projTextureViewMat, projTextureProjMat);
            }
            else
            {
                this->m_data->m_guiHelper->setProjectiveTexture(false);
            }

            if ((flags & ER_NO_SEGMENTATION_MASK) != 0)
            {
                segmentationMaskBuffer = 0;
            }
            m_data->m_guiHelper->copyCameraImageData(viewMat,
                                                     projMat, pixelRGBA, numRequestedPixels,
                                                     depthBuffer, numRequestedPixels,
                                                     segmentationMaskBuffer, numRequestedPixels,
                                                     startPixelIndex, width, height, &numPixelsCopied);

            if (numPixelsCopied > 0)
            {
                //convert segmentation mask

                if (segmentationMaskBuffer)
                {
                    for (i32 i = 0; i < numPixelsCopied; i++)
                    {
                        i32 graphicsSegMask = segmentationMaskBuffer[i];
                        i32 segMask = -1;
                        if ((graphicsSegMask >= 0) && (graphicsSegMask < m_data->m_graphicsIndexToSegmentationMask.size()))
                        {
                            segMask = m_data->m_graphicsIndexToSegmentationMask[graphicsSegMask];
                        }
                        if ((flags & ER_SEGMENTATION_MASK_OBJECT_AND_LINKINDEX) == 0)
                        {
                            if (segMask >= 0)
                            {
                                segMask &= ((1 << 24) - 1);
                            }
                        }
                        segmentationMaskBuffer[i] = segMask;
                    }
                }

                handled = true;
                m_data->m_guiHelper->debugDisplayCameraImageData(viewMat,
                                                                 projMat, pixelRGBA, numRequestedPixels,
                                                                 depthBuffer, numRequestedPixels,
                                                                 segmentationMaskBuffer, numRequestedPixels,
                                                                 startPixelIndex, width, height, &numPixelsCopied);
            }
        }
        if (!handled)
        {
            if (m_data->m_pluginManager.getRenderInterface())
            {
                if (clientCmd.m_requestPixelDataArguments.m_startPixelIndex == 0)
                {
                    //   printf("-------------------------------\nRendering\n");

                    if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_LIGHT_DIRECTION) != 0)
                    {
                        m_data->m_pluginManager.getRenderInterface()->setLightDirection(clientCmd.m_requestPixelDataArguments.m_lightDirection[0], clientCmd.m_requestPixelDataArguments.m_lightDirection[1], clientCmd.m_requestPixelDataArguments.m_lightDirection[2]);
                    }

                    if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_LIGHT_COLOR) != 0)
                    {
                        m_data->m_pluginManager.getRenderInterface()->setLightColor(clientCmd.m_requestPixelDataArguments.m_lightColor[0], clientCmd.m_requestPixelDataArguments.m_lightColor[1], clientCmd.m_requestPixelDataArguments.m_lightColor[2]);
                    }

                    if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_LIGHT_DISTANCE) != 0)
                    {
                        m_data->m_pluginManager.getRenderInterface()->setLightDistance(clientCmd.m_requestPixelDataArguments.m_lightDistance);
                    }

                    if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_SHADOW) != 0)
                    {
                        m_data->m_pluginManager.getRenderInterface()->setShadow((clientCmd.m_requestPixelDataArguments.m_hasShadow != 0));
                    }

                    if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_AMBIENT_COEFF) != 0)
                    {
                        m_data->m_pluginManager.getRenderInterface()->setLightAmbientCoeff(clientCmd.m_requestPixelDataArguments.m_lightAmbientCoeff);
                    }

                    if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_DIFFUSE_COEFF) != 0)
                    {
                        m_data->m_pluginManager.getRenderInterface()->setLightDiffuseCoeff(clientCmd.m_requestPixelDataArguments.m_lightDiffuseCoeff);
                    }

                    if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_SET_SPECULAR_COEFF) != 0)
                    {
                        m_data->m_pluginManager.getRenderInterface()->setLightSpecularCoeff(clientCmd.m_requestPixelDataArguments.m_lightSpecularCoeff);
                    }

                    for (i32 i = 0; i < m_data->m_dynamicsWorld->getNumCollisionObjects(); i++)
                    {
                        const CollisionObject2* colObj = m_data->m_dynamicsWorld->getCollisionObjectArray()[i];
                        Vec3 localScaling(1, 1, 1);
                        m_data->m_pluginManager.getRenderInterface()->syncTransform(colObj->getUserIndex3(), colObj->getWorldTransform(), localScaling);

                        const CollisionShape* collisionShape = colObj->getCollisionShape();
                        if (collisionShape->getShapeType() == SOFTBODY_SHAPE_PROXYTYPE)
                        {
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
                            const SoftBody* psb = (const SoftBody*)colObj;
                            if (psb->getUserIndex3() >= 0)
                            {

                                AlignedObjectArray<Vec3> vertices;
                                AlignedObjectArray<Vec3> normals;
                                if (psb->m_renderNodes.size() == 0)
                                {

                                    vertices.resize(psb->m_faces.size() * 3);
                                    normals.resize(psb->m_faces.size() * 3);

                                    for (i32 i = 0; i < psb->m_faces.size(); i++)  // Foreach face
                                    {
                                        for (i32 k = 0; k < 3; k++)  // Foreach vertex on a face
                                        {
                                            i32 currentIndex = i * 3 + k;
                                            vertices[currentIndex] = psb->m_faces[i].m_n[k]->m_x;
                                            normals[currentIndex] = psb->m_faces[i].m_n[k]->m_n;
                                        }
                                    }
                                }
                                else
                                {
                                    vertices.resize(psb->m_renderNodes.size());

                                    for (i32 i = 0; i < psb->m_renderNodes.size(); i++)  // Foreach face
                                    {
                                        vertices[i] = psb->m_renderNodes[i].m_x;
                                    }
                                }
                                m_data->m_pluginManager.getRenderInterface()->updateShape(psb->getUserIndex3(), &vertices[0], vertices.size(), &normals[0],normals.size());
                            }
#endif //SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
                        }
                    }

                    if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES) != 0)
                    {
                        m_data->m_pluginManager.getRenderInterface()->render(
                            clientCmd.m_requestPixelDataArguments.m_viewMatrix,
                            clientCmd.m_requestPixelDataArguments.m_projectionMatrix);
                    }
                    else
                    {
                        b3OpenGLVisualizerCameraInfo tmpCamResult;
                        bool result = this->m_data->m_guiHelper->getCameraInfo(
                            &tmpCamResult.m_width,
                            &tmpCamResult.m_height,
                            tmpCamResult.m_viewMatrix,
                            tmpCamResult.m_projectionMatrix,
                            tmpCamResult.m_camUp,
                            tmpCamResult.m_camForward,
                            tmpCamResult.m_horizontal,
                            tmpCamResult.m_vertical,
                            &tmpCamResult.m_yaw,
                            &tmpCamResult.m_pitch,
                            &tmpCamResult.m_dist,
                            tmpCamResult.m_target);
                        if (result)
                        {
                            m_data->m_pluginManager.getRenderInterface()->render(tmpCamResult.m_viewMatrix, tmpCamResult.m_projectionMatrix);
                        }
                        else
                        {
                            m_data->m_pluginManager.getRenderInterface()->render();
                        }
                    }
                }
            }

            if (m_data->m_pluginManager.getRenderInterface())
            {
                if ((flags & ER_USE_PROJECTIVE_TEXTURE) != 0)
                {
                    m_data->m_pluginManager.getRenderInterface()->setProjectiveTexture(true);
                    if ((clientCmd.m_updateFlags & REQUEST_PIXEL_ARGS_HAS_PROJECTIVE_TEXTURE_MATRICES) != 0)
                    {
                        for (i32 i = 0; i < 16; i++)
                        {
                            projTextureViewMat[i] = clientCmd.m_requestPixelDataArguments.m_projectiveTextureViewMatrix[i];
                            projTextureProjMat[i] = clientCmd.m_requestPixelDataArguments.m_projectiveTextureProjectionMatrix[i];
                        }
                    }
                    else  // If no specified matrices for projective texture, then use the camera matrices.
                    {
                        for (i32 i = 0; i < 16; i++)
                        {
                            projTextureViewMat[i] = viewMat[i];
                            projTextureProjMat[i] = projMat[i];
                        }
                    }
                    m_data->m_pluginManager.getRenderInterface()->setProjectiveTextureMatrices(projTextureViewMat, projTextureProjMat);
                }
                else
                {
                    m_data->m_pluginManager.getRenderInterface()->setProjectiveTexture(false);
                }

                if ((flags & ER_NO_SEGMENTATION_MASK) != 0)
                {
                    segmentationMaskBuffer = 0;
                }

                m_data->m_pluginManager.getRenderInterface()->copyCameraImageData(pixelRGBA, numRequestedPixels,
                                                                                  depthBuffer, numRequestedPixels,
                                                                                  segmentationMaskBuffer, numRequestedPixels,
                                                                                  startPixelIndex, &width, &height, &numPixelsCopied);
                m_data->m_pluginManager.getRenderInterface()->setProjectiveTexture(false);
            }

            m_data->m_guiHelper->debugDisplayCameraImageData(clientCmd.m_requestPixelDataArguments.m_viewMatrix,
                                                             clientCmd.m_requestPixelDataArguments.m_projectionMatrix, pixelRGBA, numRequestedPixels,
                                                             depthBuffer, numRequestedPixels,
                                                             segmentationMaskBuffer, numRequestedPixels,
                                                             startPixelIndex, width, height, &numPixelsCopied);
        }

        //each pixel takes 4 RGBA values and 1 float = 8 bytes
    }
    else
    {
    }

    serverStatusOut.m_type = CMD_CAMERA_IMAGE_COMPLETED;

    serverStatusOut.m_sendPixelDataArguments.m_numPixelsCopied = numPixelsCopied;
    serverStatusOut.m_sendPixelDataArguments.m_numRemainingPixels = numRemainingPixels - numPixelsCopied;
    serverStatusOut.m_sendPixelDataArguments.m_startingPixelIndex = startPixelIndex;
    serverStatusOut.m_sendPixelDataArguments.m_imageWidth = width;
    serverStatusOut.m_sendPixelDataArguments.m_imageHeight = height;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processSaveWorldCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = false;
    DRX3D_PROFILE("CMD_SAVE_WORLD");
    serverStatusOut.m_type = CMD_SAVE_WORLD_FAILED;

    ///this is a very rudimentary way to save the state of the world, for scene authoring
    ///many todo's, for example save the state of motor controllers etc.

    {
        //saveWorld(clientCmd.m_sdfArguments.m_sdfFileName);
        i32 constraintCount = 0;
        FILE* f = fopen(clientCmd.m_sdfArguments.m_sdfFileName, "w");
        if (f)
        {
            char line[2048];
            {
                sprintf(line, "import pybullet as p\n");
                i32 len = strlen(line);
                fwrite(line, len, 1, f);
            }
            {
                sprintf(line, "cin = p.connect(p.SHARED_MEMORY)\n");
                i32 len = strlen(line);
                fwrite(line, len, 1, f);
            }
            {
                sprintf(line, "if (cin < 0):\n");
                i32 len = strlen(line);
                fwrite(line, len, 1, f);
            }
            {
                sprintf(line, "    cin = p.connect(p.GUI)\n");
                i32 len = strlen(line);
                fwrite(line, len, 1, f);
            }

            //for each objects ...
            for (i32 i = 0; i < m_data->m_saveWorldBodyData.size(); i++)
            {
                SaveWorldObjectData& sd = m_data->m_saveWorldBodyData[i];

                for (i32 i = 0; i < sd.m_bodyUniqueIds.size(); i++)
                {
                    {
                        i32 bodyUniqueId = sd.m_bodyUniqueIds[i];
                        InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
                        if (body)
                        {
                            if (body->m_multiBody)
                            {
                                MultiBody* mb = body->m_multiBody;

                                Transform2 comTr = mb->getBaseWorldTransform();
                                Transform2 tr = comTr * body->m_rootLocalInertialFrame.inverse();

                                if (strstr(sd.m_fileName.c_str(), ".urdf"))
                                {
                                    sprintf(line, "objects = [p.loadURDF(\"%s\", %f,%f,%f,%f,%f,%f,%f)]\n", sd.m_fileName.c_str(),
                                            tr.getOrigin()[0], tr.getOrigin()[1], tr.getOrigin()[2],
                                            tr.getRotation()[0], tr.getRotation()[1], tr.getRotation()[2], tr.getRotation()[3]);
                                    i32 len = strlen(line);
                                    fwrite(line, len, 1, f);
                                }

                                if (strstr(sd.m_fileName.c_str(), ".sdf") && i == 0)
                                {
                                    sprintf(line, "objects = p.loadSDF(\"%s\")\n", sd.m_fileName.c_str());
                                    i32 len = strlen(line);
                                    fwrite(line, len, 1, f);
                                }
                                if (strstr(sd.m_fileName.c_str(), ".xml") && i == 0)
                                {
                                    sprintf(line, "objects = p.loadMJCF(\"%s\")\n", sd.m_fileName.c_str());
                                    i32 len = strlen(line);
                                    fwrite(line, len, 1, f);
                                }

                                if (strstr(sd.m_fileName.c_str(), ".sdf") || strstr(sd.m_fileName.c_str(), ".xml") || ((strstr(sd.m_fileName.c_str(), ".urdf")) && mb->getNumLinks()))
                                {
                                    sprintf(line, "ob = objects[%d]\n", i);
                                    i32 len = strlen(line);
                                    fwrite(line, len, 1, f);
                                }

                                if (strstr(sd.m_fileName.c_str(), ".sdf") || strstr(sd.m_fileName.c_str(), ".xml"))
                                {
                                    sprintf(line, "p.resetBasePositionAndOrientation(ob,[%f,%f,%f],[%f,%f,%f,%f])\n",
                                            comTr.getOrigin()[0], comTr.getOrigin()[1], comTr.getOrigin()[2],
                                            comTr.getRotation()[0], comTr.getRotation()[1], comTr.getRotation()[2], comTr.getRotation()[3]);
                                    i32 len = strlen(line);
                                    fwrite(line, len, 1, f);
                                }

                                if (mb->getNumLinks())
                                {
                                    {
                                        sprintf(line, "jointPositions=[");
                                        i32 len = strlen(line);
                                        fwrite(line, len, 1, f);
                                    }

                                    for (i32 i = 0; i < mb->getNumLinks(); i++)
                                    {
                                        Scalar jointPos = mb->getJointPosMultiDof(i)[0];
                                        if (i < mb->getNumLinks() - 1)
                                        {
                                            sprintf(line, " %f,", jointPos);
                                            i32 len = strlen(line);
                                            fwrite(line, len, 1, f);
                                        }
                                        else
                                        {
                                            sprintf(line, " %f ", jointPos);
                                            i32 len = strlen(line);
                                            fwrite(line, len, 1, f);
                                        }
                                    }

                                    {
                                        sprintf(line, "]\nfor jointIndex in range (p.getNumJoints(ob)):\n\tp.resetJointState(ob,jointIndex,jointPositions[jointIndex])\n\n");
                                        i32 len = strlen(line);
                                        fwrite(line, len, 1, f);
                                    }
                                }
                            }
                            else
                            {
                                //todo: RigidBody/SoftBody etc case
                            }
                        }
                    }
                }

                //for URDF, load at origin, then reposition...

                struct SaveWorldObjectData
                {
                    b3AlignedObjectArray<i32> m_bodyUniqueIds;
                    STxt m_fileName;
                };
            }

            //user constraints
            {
                for (i32 i = 0; i < m_data->m_userConstraints.size(); i++)
                {
                    InteralUserConstraintData* ucptr = m_data->m_userConstraints.getAtIndex(i);
                    b3UserConstraint& uc = ucptr->m_userConstraintData;

                    i32 parentBodyIndex = uc.m_parentBodyIndex;
                    i32 parentJointIndex = uc.m_parentJointIndex;
                    i32 childBodyIndex = uc.m_childBodyIndex;
                    i32 childJointIndex = uc.m_childJointIndex;
                    Vec3 jointAxis(uc.m_jointAxis[0], uc.m_jointAxis[1], uc.m_jointAxis[2]);
                    Vec3 pivotParent(uc.m_parentFrame[0], uc.m_parentFrame[1], uc.m_parentFrame[2]);
                    Vec3 pivotChild(uc.m_childFrame[0], uc.m_childFrame[1], uc.m_childFrame[2]);
                    Quat ornFrameParent(uc.m_parentFrame[3], uc.m_parentFrame[4], uc.m_parentFrame[5], uc.m_parentFrame[6]);
                    Quat ornFrameChild(uc.m_childFrame[3], uc.m_childFrame[4], uc.m_childFrame[5], uc.m_childFrame[6]);
                    {
                        char jointTypeStr[1024] = "FIXED";
                        bool hasKnownJointType = true;

                        switch (uc.m_jointType)
                        {
                            case eRevoluteType:
                            {
                                sprintf(jointTypeStr, "p.JOINT_REVOLUTE");
                                break;
                            }
                            case ePrismaticType:
                            {
                                sprintf(jointTypeStr, "p.JOINT_PRISMATIC");
                                break;
                            }
                            case eSphericalType:
                            {
                                sprintf(jointTypeStr, "p.JOINT_SPHERICAL");
                                break;
                            }
                            case ePlanarType:
                            {
                                sprintf(jointTypeStr, "p.JOINT_PLANAR");
                                break;
                            }
                            case eFixedType:
                            {
                                sprintf(jointTypeStr, "p.JOINT_FIXED");
                                break;
                            }
                            case ePoint2PointType:
                            {
                                sprintf(jointTypeStr, "p.JOINT_POINT2POINT");
                                break;
                            }
                            case eGearType:
                            {
                                sprintf(jointTypeStr, "p.JOINT_GEAR");
                                break;
                            }
                            default:
                            {
                                hasKnownJointType = false;
                                drx3DWarning("unknown constraint type in SAVE_WORLD");
                            }
                        };
                        if (hasKnownJointType)
                        {
                            {
                                sprintf(line, "cid%d = p.createConstraint(%d,%d,%d,%d,%s,[%f,%f,%f],[%f,%f,%f],[%f,%f,%f],[%f,%f,%f,%f],[%f,%f,%f,%f])\n",
                                        constraintCount,
                                        parentBodyIndex,
                                        parentJointIndex,
                                        childBodyIndex,
                                        childJointIndex,
                                        jointTypeStr,
                                        jointAxis[0], jointAxis[1], jointAxis[2],
                                        pivotParent[0], pivotParent[1], pivotParent[2],
                                        pivotChild[0], pivotChild[1], pivotChild[2],
                                        ornFrameParent[0], ornFrameParent[1], ornFrameParent[2], ornFrameParent[3],
                                        ornFrameChild[0], ornFrameChild[1], ornFrameChild[2], ornFrameChild[3]);
                                i32 len = strlen(line);
                                fwrite(line, len, 1, f);
                            }
                            {
                                sprintf(line, "p.changeConstraint(cid%d,maxForce=%f)\n", constraintCount, uc.m_maxAppliedForce);
                                i32 len = strlen(line);
                                fwrite(line, len, 1, f);
                                constraintCount++;
                            }
                        }
                    }
                }
            }

            {
                Vec3 grav = this->m_data->m_dynamicsWorld->getGravity();
                sprintf(line, "p.setGravity(%f,%f,%f)\n", grav[0], grav[1], grav[2]);
                i32 len = strlen(line);
                fwrite(line, len, 1, f);
            }

            {
                sprintf(line, "p.stepSimulation()\np.disconnect()\n");
                i32 len = strlen(line);
                fwrite(line, len, 1, f);
            }
            fclose(f);
        }

        serverStatusOut.m_type = CMD_SAVE_WORLD_COMPLETED;
        hasStatus = true;
    }

    return hasStatus;
}

#define MYLINELENGTH 16 * 32768

static u8* MyGetRawHeightfieldData(CommonFileIOInterface& fileIO, PHY_ScalarType type, tukk fileName, i32& width, i32& height,
                                              Scalar& minHeight,
                                              Scalar& maxHeight)
{
    STxt ext;
    STxt fn(fileName);
    STxt ext_ = fn.substr(fn.size() - 4);
    for (STxt::iterator i = ext_.begin(); i != ext_.end(); ++i)
    {
        ext += char(tolower(*i));
    }

    if (ext != ".txt")
    {
        char relativeFileName[1024];
        i32 found = fileIO.findResourcePath(fileName, relativeFileName, 1024);

        b3AlignedObjectArray<char> buffer;
        buffer.reserve(1024);
        i32 fileId = fileIO.fileOpen(relativeFileName, "rb");
        if (fileId >= 0)
        {
            i32 size = fileIO.getFileSize(fileId);
            if (size > 0)
            {
                buffer.resize(size);
                i32 actual = fileIO.fileRead(fileId, &buffer[0], size);
                if (actual != size)
                {
                    drx3DWarning("STL filesize mismatch!\n");
                    buffer.resize(0);
                }
            }
            fileIO.fileClose(fileId);
        }

        if (buffer.size())
        {
            i32 n;

            u8* image = stbi_load_from_memory((u8k*)&buffer[0], buffer.size(), &width, &height, &n, 3);
            if (image)
            {
                fileIO.fileClose(fileId);
                i32 nElements = width * height;
                i32 bytesPerElement = sizeof(Scalar);
                Assert(bytesPerElement > 0 && "bad bytes per element");

                i32 nBytes = nElements * bytesPerElement;
                u8* raw = new u8[nBytes];
                Assert(raw && "out of memory");

                u8* p = raw;
                for (i32 j = 0; j < height; ++j)

                {
                    for (i32 i = 0; i < width; ++i)
                    {
                        float z = double(image[(width - 1 - i) * 3 + width * j * 3]) * (1. / 255.);
                        Scalar* pf = (Scalar*)p;
                        *pf = z;
                        p += bytesPerElement;
                        // update min/max
                        if (!i && !j)
                        {
                            minHeight = z;
                            maxHeight = z;
                        }
                        else
                        {
                            if (z < minHeight)
                            {
                                minHeight = z;
                            }
                            if (z > maxHeight)
                            {
                                maxHeight = z;
                            }
                        }
                    }
                }
                free(image);

                return raw;
            }
        }
    }

    if (ext == ".txt")
    {
        //read a csv file as used in DeepLoco
        {
            char relativePath[1024];
            i32 found = fileIO.findResourcePath(fileName, relativePath, 1024);
            AlignedObjectArray<char> lineBuffer;
            lineBuffer.resize(MYLINELENGTH);
            i32 slot = fileIO.fileOpen(relativePath, "r");
            i32 rows = 0;
            i32 cols = 0;

            AlignedObjectArray<double> allValues;
            if (slot >= 0)
            {
                tuk lineChar;
                while ((lineChar = fileIO.readLine(slot, &lineBuffer[0], MYLINELENGTH)) != nullptr)
                {
                    rows = 0;
                    STxt line(lineChar);
                    i32 pos = 0;
                    while (pos < line.length())
                    {
                        i32 nextPos = pos + 1;
                        while (nextPos < line.length())
                        {
                            if (line[nextPos - 1] == ',')
                            {
                                break;
                            }
                            nextPos++;
                        }
                        STxt substr = line.substr(pos, nextPos - pos - 1);

                        double v;
                        if (sscanf(substr.c_str(), "%lf", &v) == 1)
                        {
                            allValues.push_back(v);
                            rows++;
                        }
                        pos = nextPos;
                    }
                    cols++;
                }
                width = rows;
                height = cols;

                fileIO.fileClose(slot);
                i32 nElements = width * height;
                //  std::cerr << "  nElements = " << nElements << "\n";

                i32 bytesPerElement = sizeof(Scalar);
                //  std::cerr << "  bytesPerElement = " << bytesPerElement << "\n";
                Assert(bytesPerElement > 0 && "bad bytes per element");

                long nBytes = nElements * bytesPerElement;
                //  std::cerr << "  nBytes = " << nBytes << "\n";
                u8* raw = new u8[nBytes];
                Assert(raw && "out of memory");

                u8* p = raw;
                for (i32 i = 0; i < width; ++i)
                {
                    for (i32 j = 0; j < height; ++j)
                    {
                        double z = allValues[i + width * j];
                        //convertFromFloat(p, z, type);
                        Scalar* pf = (Scalar*)p;
                        *pf = z;
                        p += bytesPerElement;
                        // update min/max
                        if (!i && !j)
                        {
                            minHeight = z;
                            maxHeight = z;
                        }
                        else
                        {
                            if (z < minHeight)
                            {
                                minHeight = z;
                            }
                            if (z > maxHeight)
                            {
                                maxHeight = z;
                            }
                        }
                    }
                }
                return raw;
            }
        }
    }

    return 0;
}

class MyTriangleCollector4 : public TriangleCallback
{
public:
    AlignedObjectArray<GLInstanceVertex>* m_pVerticesOut;
    AlignedObjectArray<i32>* m_pIndicesOut;
    Vec3 m_aabbMin, m_aabbMax;
    Scalar m_textureScaling;

    MyTriangleCollector4(const Vec3& aabbMin, const Vec3& aabbMax)
        :m_aabbMin(aabbMin), m_aabbMax(aabbMax), m_textureScaling(1)
    {
        m_pVerticesOut = 0;
        m_pIndicesOut = 0;
    }

    virtual void processTriangle(Vec3* tris, i32 partId, i32 triangleIndex)
    {
        for (i32 k = 0; k < 3; k++)
        {
            GLInstanceVertex v;
            v.xyzw[3] = 0;

            Vec3 normal = (tris[0] - tris[1]).cross(tris[0] - tris[2]);
            normal.safeNormalize();
            for (i32 l = 0; l < 3; l++)
            {
                v.xyzw[l] = tris[k][l];
                v.normal[l] = normal[l];
            }

            Vec3 extents = m_aabbMax - m_aabbMin;

            v.uv[0] = (1. - ((v.xyzw[0] - m_aabbMin[0]) / (m_aabbMax[0] - m_aabbMin[0]))) * m_textureScaling;
            v.uv[1] = (1. - (v.xyzw[1] - m_aabbMin[1]) / (m_aabbMax[1] - m_aabbMin[1])) * m_textureScaling;

            m_pIndicesOut->push_back(m_pVerticesOut->size());
            m_pVerticesOut->push_back(v);
        }
    }
};
bool PhysicsServerCommandProcessor::processCreateCollisionShapeCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    serverStatusOut.m_type = CMD_CREATE_COLLISION_SHAPE_FAILED;

#ifdef USE_DISCRETE_DYNAMICS_WORLD
    WorldImporter* worldImporter = new WorldImporter(m_data->m_dynamicsWorld);
#else
    MultiBodyWorldImporter* worldImporter = new MultiBodyWorldImporter(m_data->m_dynamicsWorld);
#endif

    CollisionShape* shape = 0;
    b3AlignedObjectArray<UrdfCollision> urdfCollisionObjects;

    CompoundShape* compound = 0;

    if (clientCmd.m_createUserShapeArgs.m_numUserShapes > 1)
    {
        compound = worldImporter->createCompoundShape();
        compound->setMargin(m_data->m_defaultCollisionMargin);
    }
    for (i32 i = 0; i < clientCmd.m_createUserShapeArgs.m_numUserShapes; i++)
    {
        GLInstanceGraphicsShape* glmesh = 0;
        char pathPrefix[1024] = "";
        char relativeFileName[1024] = "";
        UrdfCollision urdfColObj;

        Transform2 childTransform;
        childTransform.setIdentity();
        if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_hasChildTransform)
        {
            childTransform.setOrigin(Vec3(clientCmd.m_createUserShapeArgs.m_shapes[i].m_childPosition[0],
                                               clientCmd.m_createUserShapeArgs.m_shapes[i].m_childPosition[1],
                                               clientCmd.m_createUserShapeArgs.m_shapes[i].m_childPosition[2]));
            childTransform.setRotation(Quat(
                clientCmd.m_createUserShapeArgs.m_shapes[i].m_childOrientation[0],
                clientCmd.m_createUserShapeArgs.m_shapes[i].m_childOrientation[1],
                clientCmd.m_createUserShapeArgs.m_shapes[i].m_childOrientation[2],
                clientCmd.m_createUserShapeArgs.m_shapes[i].m_childOrientation[3]));
            if (compound == 0)
            {
                compound = worldImporter->createCompoundShape();
            }
        }

        urdfColObj.m_linkLocalFrame = childTransform;
        urdfColObj.m_sourceFileLocation = "memory";
        urdfColObj.m_name = "memory";
        urdfColObj.m_geometry.m_type = URDF_GEOM_UNKNOWN;

        switch (clientCmd.m_createUserShapeArgs.m_shapes[i].m_type)
        {
            case GEOM_SPHERE:
            {
                double radius = clientCmd.m_createUserShapeArgs.m_shapes[i].m_sphereRadius;
                shape = worldImporter->createSphereShape(radius);
                shape->setMargin(m_data->m_defaultCollisionMargin);
                if (compound)
                {
                    compound->addChildShape(childTransform, shape);
                }
                urdfColObj.m_geometry.m_type = URDF_GEOM_SPHERE;
                urdfColObj.m_geometry.m_sphereRadius = radius;
                break;
            }
            case GEOM_BOX:
            {
                //double halfExtents[3] = clientCmd.m_createUserShapeArgs.m_shapes[i].m_sphereRadius;
                Vec3 halfExtents(
                    clientCmd.m_createUserShapeArgs.m_shapes[i].m_boxHalfExtents[0],
                    clientCmd.m_createUserShapeArgs.m_shapes[i].m_boxHalfExtents[1],
                    clientCmd.m_createUserShapeArgs.m_shapes[i].m_boxHalfExtents[2]);
                shape = worldImporter->createBoxShape(halfExtents);
                shape->setMargin(m_data->m_defaultCollisionMargin);
                if (compound)
                {
                    compound->addChildShape(childTransform, shape);
                }
                urdfColObj.m_geometry.m_type = URDF_GEOM_BOX;
                urdfColObj.m_geometry.m_boxSize = 2. * halfExtents;
                break;
            }
            case GEOM_CAPSULE:
            {
                shape = worldImporter->createCapsuleShapeZ(clientCmd.m_createUserShapeArgs.m_shapes[i].m_capsuleRadius,
                                                           clientCmd.m_createUserShapeArgs.m_shapes[i].m_capsuleHeight);
                shape->setMargin(m_data->m_defaultCollisionMargin);
                if (compound)
                {
                    compound->addChildShape(childTransform, shape);
                }
                urdfColObj.m_geometry.m_type = URDF_GEOM_CAPSULE;
                urdfColObj.m_geometry.m_capsuleRadius = clientCmd.m_createUserShapeArgs.m_shapes[i].m_capsuleRadius;
                urdfColObj.m_geometry.m_capsuleHeight = clientCmd.m_createUserShapeArgs.m_shapes[i].m_capsuleHeight;

                break;
            }
            case GEOM_CYLINDER:
            {
                shape = worldImporter->createCylinderShapeZ(clientCmd.m_createUserShapeArgs.m_shapes[i].m_capsuleRadius,
                                                            0.5 * clientCmd.m_createUserShapeArgs.m_shapes[i].m_capsuleHeight);
                shape->setMargin(m_data->m_defaultCollisionMargin);
                if (compound)
                {
                    compound->addChildShape(childTransform, shape);
                }
                urdfColObj.m_geometry.m_type = URDF_GEOM_CYLINDER;
                urdfColObj.m_geometry.m_capsuleRadius = clientCmd.m_createUserShapeArgs.m_shapes[i].m_capsuleRadius;
                urdfColObj.m_geometry.m_capsuleHeight = clientCmd.m_createUserShapeArgs.m_shapes[i].m_capsuleHeight;

                break;
            }
            case GEOM_HEIGHTFIELD:
            {
                i32 width;
                i32 height;
                Scalar minHeight, maxHeight;
                PHY_ScalarType scalarType = PHY_FLOAT;
                CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
                u8k* heightfieldData = 0;
                if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_numHeightfieldColumns > 0 &&
                    clientCmd.m_createUserShapeArgs.m_shapes[i].m_numHeightfieldRows > 0)
                {
                    width = clientCmd.m_createUserShapeArgs.m_shapes[i].m_numHeightfieldRows;
                    height = clientCmd.m_createUserShapeArgs.m_shapes[i].m_numHeightfieldColumns;
                    float* heightfieldDataSrc = (float*)bufferServerToClient;
                    heightfieldData = new u8[width * height * sizeof(Scalar)];
                    Scalar* datafl = (Scalar*)heightfieldData;
                    minHeight = heightfieldDataSrc[0];
                    maxHeight = heightfieldDataSrc[0];
                    for (i32 i = 0; i < width * height; i++)
                    {
                        datafl[i] = heightfieldDataSrc[i];
                        minHeight = d3Min(minHeight, (Scalar)datafl[i]);
                        maxHeight = d3Max(maxHeight, (Scalar)datafl[i]);
                    }
                }
                else
                {
                    heightfieldData = MyGetRawHeightfieldData(*fileIO, scalarType, clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshFileName, width, height, minHeight, maxHeight);
                }
                if (heightfieldData)
                {
                    //replace heightfield data or create new heightfield
                    if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_replaceHeightfieldIndex >= 0)
                    {
                        i32 collisionShapeUid = clientCmd.m_createUserShapeArgs.m_shapes[i].m_replaceHeightfieldIndex;

                        InternalCollisionShapeHandle* handle = m_data->m_userCollisionShapeHandles.getHandle(collisionShapeUid);
                        if (handle && handle->m_collisionShape && handle->m_collisionShape->getShapeType() == TERRAIN_SHAPE_PROXYTYPE)
                        {
                            HeightfieldTerrainShape* terrainShape = (HeightfieldTerrainShape*)handle->m_collisionShape;
                            Scalar* heightfieldDest = (Scalar*)terrainShape->getHeightfieldRawData();
                            //replace the data

                            Scalar* datafl = (Scalar*)heightfieldData;

                            for (i32 i = 0; i < width * height; i++)
                            {
                                heightfieldDest[i] = datafl[i];
                            }
                            //update graphics

                            AlignedObjectArray<GLInstanceVertex> gfxVertices;
                            AlignedObjectArray<i32> indices;
                            i32 strideInBytes = 9 * sizeof(float);

                            Vec3 aabbMin, aabbMax;
                            Transform2 tr;
                            tr.setIdentity();
                            terrainShape->getAabb(tr, aabbMin, aabbMax);
                            MyTriangleCollector4 col(aabbMin, aabbMax);
                            col.m_pVerticesOut = &gfxVertices;
                            col.m_pIndicesOut = &indices;

                            terrainShape->processAllTriangles(&col, aabbMin, aabbMax);
                            if (gfxVertices.size() && indices.size())
                            {
                                m_data->m_guiHelper->updateShape(terrainShape->getUserIndex(), &gfxVertices[0].xyzw[0], gfxVertices.size());
                            }

                            AlignedObjectArray<Vec3> vts;
                            AlignedObjectArray<Vec3> normals;
                            vts.resize(gfxVertices.size());
                            normals.resize(gfxVertices.size());

                            for (i32 v = 0; v < gfxVertices.size(); v++)
                            {
                                vts[v].setVal(gfxVertices[v].xyzw[0], gfxVertices[v].xyzw[1], gfxVertices[v].xyzw[2]);
                                normals[v].setVal(gfxVertices[v].normal[0], gfxVertices[v].normal[1], gfxVertices[v].normal[2]);
                            }

                            m_data->m_pluginManager.getRenderInterface()->updateShape(terrainShape->getUserIndex2(), &vts[0], vts.size(), &normals[0], normals.size());


                            terrainShape->clearAccelerator();
                            terrainShape->buildAccelerator();

                            TriangleInfoMap* oldTriangleInfoMap = terrainShape->getTriangleInfoMap();
                            delete (oldTriangleInfoMap);
                            terrainShape->setTriangleInfoMap(0);

                            if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_collisionFlags & GEOM_CONCAVE_INTERNAL_EDGE)
                            {
                                TriangleInfoMap* triangleInfoMap = new TriangleInfoMap();
                                GenerateInternalEdgeInfo(terrainShape, triangleInfoMap);
                            }
                            serverStatusOut.m_createUserShapeResultArgs.m_userShapeUniqueId = collisionShapeUid;
                            delete worldImporter;

                            serverStatusOut.m_type = CMD_CREATE_COLLISION_SHAPE_COMPLETED;
                        }

                        delete heightfieldData;
                        return hasStatus;
                    }
                    else
                    {
                        Scalar gridSpacing = 0.5;
                        Scalar gridHeightScale = 1. / 256.;

                        bool flipQuadEdges = false;
                        i32 upAxis = 2;
                        /*HeightfieldTerrainShape* heightfieldShape = worldImporter->createHeightfieldShape( width, height,
                            heightfieldData,
                                gridHeightScale,
                                minHeight, maxHeight,
                                upAxis, i32(scalarType), flipQuadEdges);
                        */
                        HeightfieldTerrainShape* heightfieldShape = new HeightfieldTerrainShape(width, height,
                                                                                                    heightfieldData,
                                                                                                    gridHeightScale,
                                                                                                    minHeight, maxHeight,
                                                                                                    upAxis, scalarType, flipQuadEdges);
                        m_data->m_collisionShapes.push_back(heightfieldShape);
                        double textureScaling = clientCmd.m_createUserShapeArgs.m_shapes[i].m_heightfieldTextureScaling;
                        heightfieldShape->setUserValue3(textureScaling);
                        shape = heightfieldShape;
                        if (upAxis == 2)
                            heightfieldShape->setFlipTriangleWinding(true);

                        // scale the shape
                        Vec3 localScaling(clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshScale[0],
                                               clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshScale[1],
                                               clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshScale[2]);

                        heightfieldShape->setLocalScaling(localScaling);
                        //buildAccelerator is optional, it may not support all features.
                        heightfieldShape->buildAccelerator();

                        if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_collisionFlags & GEOM_CONCAVE_INTERNAL_EDGE)
                        {
                            TriangleInfoMap* triangleInfoMap = new TriangleInfoMap();
                            GenerateInternalEdgeInfo(heightfieldShape, triangleInfoMap);
                        }
                        this->m_data->m_heightfieldDatas.push_back(heightfieldData);


                        AlignedObjectArray<GLInstanceVertex> gfxVertices;
                        AlignedObjectArray<i32> indices;
                        i32 strideInBytes = 9 * sizeof(float);

                        Transform2 tr;
                        tr.setIdentity();
                        Vec3 aabbMin, aabbMax;
                        heightfieldShape->getAabb(tr, aabbMin, aabbMax);

                        MyTriangleCollector4 col(aabbMin, aabbMax);
                        col.m_pVerticesOut = &gfxVertices;
                        col.m_pIndicesOut = &indices;


                        heightfieldShape->processAllTriangles(&col, aabbMin, aabbMax);
                        if (gfxVertices.size() && indices.size())
                        {

                            urdfColObj.m_geometry.m_type = URDF_GEOM_HEIGHTFIELD;
                            urdfColObj.m_geometry.m_meshFileType = UrdfGeometry::MEMORY_VERTICES;
                            urdfColObj.m_geometry.m_normals.resize(gfxVertices.size());
                            urdfColObj.m_geometry.m_vertices.resize(gfxVertices.size());
                            urdfColObj.m_geometry.m_uvs.resize(gfxVertices.size());
                            for (i32 v = 0; v < gfxVertices.size(); v++)
                            {
                                urdfColObj.m_geometry.m_vertices[v].setVal(gfxVertices[v].xyzw[0], gfxVertices[v].xyzw[1], gfxVertices[v].xyzw[2]);
                                urdfColObj.m_geometry.m_uvs[v].setVal(gfxVertices[v].uv[0], gfxVertices[v].uv[1], 0);
                                urdfColObj.m_geometry.m_normals[v].setVal(gfxVertices[v].normal[0], gfxVertices[v].normal[1], gfxVertices[v].normal[2]);
                            }
                            urdfColObj.m_geometry.m_indices.resize(indices.size());
                            for (i32 ii = 0; ii < indices.size(); ii++)
                            {
                                urdfColObj.m_geometry.m_indices[ii] = indices[ii];
                            }
                        }
                    }
                }
                break;
            }
            case GEOM_PLANE:
            {
                Vec3 planeNormal(clientCmd.m_createUserShapeArgs.m_shapes[i].m_planeNormal[0],
                                      clientCmd.m_createUserShapeArgs.m_shapes[i].m_planeNormal[1],
                                      clientCmd.m_createUserShapeArgs.m_shapes[i].m_planeNormal[2]);

                shape = worldImporter->createPlaneShape(planeNormal, clientCmd.m_createUserShapeArgs.m_shapes[i].m_planeConstant);
                shape->setMargin(m_data->m_defaultCollisionMargin);
                if (compound)
                {
                    compound->addChildShape(childTransform, shape);
                }
                urdfColObj.m_geometry.m_type = URDF_GEOM_PLANE;
                urdfColObj.m_geometry.m_planeNormal.setVal(
                    clientCmd.m_createUserShapeArgs.m_shapes[i].m_planeNormal[0],
                    clientCmd.m_createUserShapeArgs.m_shapes[i].m_planeNormal[1],
                    clientCmd.m_createUserShapeArgs.m_shapes[i].m_planeNormal[2]);

                break;
            }
            case GEOM_MESH:
            {
                Vec3 meshScale(clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshScale[0],
                                    clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshScale[1],
                                    clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshScale[2]);

                const STxt& urdf_path = "";

                STxt fileName = clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshFileName;
                urdfColObj.m_geometry.m_type = URDF_GEOM_MESH;
                urdfColObj.m_geometry.m_meshFileName = fileName;

                urdfColObj.m_geometry.m_meshScale = meshScale;
                CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
                pathPrefix[0] = 0;
                if (fileIO->findResourcePath(fileName.c_str(), relativeFileName, 1024))
                {
                    b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);
                }

                const STxt& error_message_prefix = "";
                STxt out_found_filename;
                i32 out_type;

                if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_numVertices)
                {
                    i32 numVertices = clientCmd.m_createUserShapeArgs.m_shapes[i].m_numVertices;
                    i32 numIndices = clientCmd.m_createUserShapeArgs.m_shapes[i].m_numIndices;

                    //i32 totalUploadSizeInBytes = numVertices * sizeof(double) * 3 + numIndices * sizeof(i32);

                    tuk data = bufferServerToClient;
                    double* vertexUpload = (double*)data;
                    i32* indexUpload = (i32*)(data + numVertices * sizeof(double) * 3);

                    if (compound == 0)
                    {
                        compound = worldImporter->createCompoundShape();
                    }
                    compound->setMargin(m_data->m_defaultCollisionMargin);

                    if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_numIndices)
                    {
                        DRX3D_PROFILE("convert trimesh2");
                        TriangleMesh* meshInterface = new TriangleMesh();
                        this->m_data->m_meshInterfaces.push_back(meshInterface);
                        {
                            DRX3D_PROFILE("convert vertices2");

                            for (i32 j = 0; j < clientCmd.m_createUserShapeArgs.m_shapes[i].m_numIndices / 3; j++)
                            {
                                i32 i0 = indexUpload[j * 3 + 0];
                                i32 i1 = indexUpload[j * 3 + 1];
                                i32 i2 = indexUpload[j * 3 + 2];

                                Vec3 v0(vertexUpload[i0 * 3 + 0],
                                             vertexUpload[i0 * 3 + 1],
                                             vertexUpload[i0 * 3 + 2]);
                                Vec3 v1(vertexUpload[i1 * 3 + 0],
                                             vertexUpload[i1 * 3 + 1],
                                             vertexUpload[i1 * 3 + 2]);
                                Vec3 v2(vertexUpload[i2 * 3 + 0],
                                             vertexUpload[i2 * 3 + 1],
                                             vertexUpload[i2 * 3 + 2]);
                                meshInterface->addTriangle(v0 * meshScale, v1 * meshScale, v2 * meshScale);
                            }
                        }

                        {
                            DRX3D_PROFILE("create BvhTriangleMeshShape");
                            BvhTriangleMeshShape* trimesh = new BvhTriangleMeshShape(meshInterface, true, true);
                            m_data->m_collisionShapes.push_back(trimesh);

                            if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_collisionFlags & GEOM_CONCAVE_INTERNAL_EDGE)
                            {
                                TriangleInfoMap* triangleInfoMap = new TriangleInfoMap();
                                GenerateInternalEdgeInfo(trimesh, triangleInfoMap);
                            }
                            shape = trimesh;
                            if (compound)
                            {
                                compound->addChildShape(childTransform, shape);
                                shape->setMargin(m_data->m_defaultCollisionMargin);
                            }
                        }
                    }
                    else
                    {
                        ConvexHullShape* convexHull = worldImporter->createConvexHullShape();
                        convexHull->setMargin(m_data->m_defaultCollisionMargin);

                        for (i32 v = 0; v < clientCmd.m_createUserShapeArgs.m_shapes[i].m_numVertices; v++)
                        {
                            Vec3 pt(vertexUpload[v * 3 + 0],
                                         vertexUpload[v * 3 + 1],
                                         vertexUpload[v * 3 + 2]);
                            convexHull->addPoint(pt * meshScale, false);
                        }

                        convexHull->recalcLocalAabb();
                        convexHull->optimizeConvexHull();
                        compound->addChildShape(childTransform, convexHull);
                    }
                    urdfColObj.m_geometry.m_meshFileType = UrdfGeometry::MEMORY_VERTICES;
                    break;
                }

                bool foundFile = UrdfFindMeshFile(fileIO, pathPrefix, relativeFileName, error_message_prefix, &out_found_filename, &out_type);
                if (foundFile)
                {
                    urdfColObj.m_geometry.m_meshFileType = out_type;

                    if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_collisionFlags & GEOM_FORCE_CONCAVE_TRIMESH)
                    {
                        CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
                        if (out_type == UrdfGeometry::FILE_STL)
                        {
                            CommonFileIOInterface* fileIO(m_data->m_pluginManager.getFileIOInterface());
                            glmesh = LoadMeshFromSTL(relativeFileName, fileIO);
                        }

                        if (out_type == UrdfGeometry::FILE_OBJ)
                        {
                            CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
                            glmesh = LoadMeshFromObj(relativeFileName, pathPrefix, fileIO);
                        }
                        //BvhTriangleMeshShape is created below
                    }
                    else
                    {
                        if (out_type == UrdfGeometry::FILE_STL)
                        {
                            CommonFileIOInterface* fileIO(m_data->m_pluginManager.getFileIOInterface());
                            glmesh = LoadMeshFromSTL(relativeFileName, fileIO);

                            D3_PROFILE("createConvexHullFromShapes");
                            if (compound == 0)
                            {
                                compound = worldImporter->createCompoundShape();
                            }
                            ConvexHullShape* convexHull = worldImporter->createConvexHullShape();
                            convexHull->setMargin(m_data->m_defaultCollisionMargin);
                            for (i32 vv = 0; vv < glmesh->m_numvertices; vv++)
                            {
                                Vec3 pt(
                                    glmesh->m_vertices->at(vv).xyzw[0],
                                    glmesh->m_vertices->at(vv).xyzw[1],
                                    glmesh->m_vertices->at(vv).xyzw[2]);
                                convexHull->addPoint(pt * meshScale, false);
                            }
                            if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_collisionFlags & GEOM_INITIALIZE_SAT_FEATURES)
                            {
                                convexHull->initializePolyhedralFeatures();
                            }

                            convexHull->recalcLocalAabb();
                            convexHull->optimizeConvexHull();

                            compound->addChildShape(childTransform, convexHull);
                            delete glmesh;
                            glmesh = 0;
                        }
                        if (out_type == UrdfGeometry::FILE_OBJ)
                        {
                            //create a convex hull for each shape, and store it in a CompoundShape

                            std::vector<bt_tinyobj::shape_t> shapes;
                            bt_tinyobj::attrib_t attribute;
                            STxt err = bt_tinyobj::LoadObj(attribute, shapes, out_found_filename.c_str(), "", fileIO);

                            //shape = createConvexHullFromShapes(shapes, collision->m_geometry.m_meshScale);
                            //static CollisionShape* createConvexHullFromShapes(std::vector<bt_tinyobj::shape_t>& shapes, const Vec3& geomScale)
                            D3_PROFILE("createConvexHullFromShapes");
                            if (compound == 0)
                            {
                                compound = worldImporter->createCompoundShape();
                            }
                            compound->setMargin(m_data->m_defaultCollisionMargin);

                            for (i32 s = 0; s < (i32)shapes.size(); s++)
                            {
                                ConvexHullShape* convexHull = worldImporter->createConvexHullShape();
                                convexHull->setMargin(m_data->m_defaultCollisionMargin);
                                bt_tinyobj::shape_t& shape = shapes[s];
                                i32 faceCount = shape.mesh.indices.size();

                                for (i32 f = 0; f < faceCount; f += 3)
                                {
                                    Vec3 pt;
                                    pt.setVal(attribute.vertices[3 * shape.mesh.indices[f + 0].vertex_index + 0],
                                                attribute.vertices[3 * shape.mesh.indices[f + 0].vertex_index + 1],
                                                attribute.vertices[3 * shape.mesh.indices[f + 0].vertex_index + 2]);

                                    convexHull->addPoint(pt * meshScale, false);

                                    pt.setVal(attribute.vertices[3 * shape.mesh.indices[f + 1].vertex_index + 0],
                                                attribute.vertices[3 * shape.mesh.indices[f + 1].vertex_index + 1],
                                                attribute.vertices[3 * shape.mesh.indices[f + 1].vertex_index + 2]);
                                    convexHull->addPoint(pt * meshScale, false);

                                    pt.setVal(attribute.vertices[3 * shape.mesh.indices[f + 2].vertex_index + 0],
                                                attribute.vertices[3 * shape.mesh.indices[f + 2].vertex_index + 1],
                                                attribute.vertices[3 * shape.mesh.indices[f + 2].vertex_index + 2]);
                                    convexHull->addPoint(pt * meshScale, false);
                                }

                                if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_collisionFlags & GEOM_INITIALIZE_SAT_FEATURES)
                                {
                                    convexHull->initializePolyhedralFeatures();
                                }
                                convexHull->recalcLocalAabb();
                                convexHull->optimizeConvexHull();

                                compound->addChildShape(childTransform, convexHull);
                            }
                        }
                    }
                }
                break;
            }

            default:
            {
            }
        }
        if (urdfColObj.m_geometry.m_type != URDF_GEOM_UNKNOWN)
        {
            urdfCollisionObjects.push_back(urdfColObj);
        }

        if (glmesh)
        {
            Vec3 meshScale(clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshScale[0],
                                clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshScale[1],
                                clientCmd.m_createUserShapeArgs.m_shapes[i].m_meshScale[2]);

            if (!glmesh || glmesh->m_numvertices <= 0)
            {
                drx3DWarning("%s: cannot extract mesh from '%s'\n", pathPrefix, relativeFileName);
                delete glmesh;
            }
            else
            {
                AlignedObjectArray<Vec3> convertedVerts;
                convertedVerts.reserve(glmesh->m_numvertices);

                for (i32 v = 0; v < glmesh->m_numvertices; v++)
                {
                    convertedVerts.push_back(Vec3(
                        glmesh->m_vertices->at(v).xyzw[0] * meshScale[0],
                        glmesh->m_vertices->at(v).xyzw[1] * meshScale[1],
                        glmesh->m_vertices->at(v).xyzw[2] * meshScale[2]));
                }

                if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_collisionFlags & GEOM_FORCE_CONCAVE_TRIMESH)
                {
                    DRX3D_PROFILE("convert trimesh");
                    TriangleMesh* meshInterface = new TriangleMesh();
                    this->m_data->m_meshInterfaces.push_back(meshInterface);
                    {
                        DRX3D_PROFILE("convert vertices");

                        for (i32 i = 0; i < glmesh->m_numIndices / 3; i++)
                        {
                            const Vec3& v0 = convertedVerts[glmesh->m_indices->at(i * 3)];
                            const Vec3& v1 = convertedVerts[glmesh->m_indices->at(i * 3 + 1)];
                            const Vec3& v2 = convertedVerts[glmesh->m_indices->at(i * 3 + 2)];
                            meshInterface->addTriangle(v0, v1, v2);
                        }
                    }

                    {
                        DRX3D_PROFILE("create BvhTriangleMeshShape");
                        BvhTriangleMeshShape* trimesh = new BvhTriangleMeshShape(meshInterface, true, true);
                        m_data->m_collisionShapes.push_back(trimesh);

                        if (clientCmd.m_createUserShapeArgs.m_shapes[i].m_collisionFlags & GEOM_CONCAVE_INTERNAL_EDGE)
                        {
                            TriangleInfoMap* triangleInfoMap = new TriangleInfoMap();
                            GenerateInternalEdgeInfo(trimesh, triangleInfoMap);
                        }
                        //trimesh->setLocalScaling(collision->m_geometry.m_meshScale);
                        shape = trimesh;
                        if (compound)
                        {
                            compound->addChildShape(childTransform, shape);
                            shape->setMargin(m_data->m_defaultCollisionMargin);
                        }
                    }
                    delete glmesh;
                }
                else
                {
                    //convex mesh

                    if (compound == 0)
                    {
                        compound = worldImporter->createCompoundShape();
                    }
                    compound->setMargin(m_data->m_defaultCollisionMargin);

                    {
                        ConvexHullShape* convexHull = worldImporter->createConvexHullShape();
                        convexHull->setMargin(m_data->m_defaultCollisionMargin);

                        for (i32 v = 0; v < convertedVerts.size(); v++)
                        {
                            Vec3 pt = convertedVerts[v];
                            convexHull->addPoint(pt, false);
                        }

                        convexHull->recalcLocalAabb();
                        convexHull->optimizeConvexHull();
                        compound->addChildShape(childTransform, convexHull);
                    }
                }
            }
        }
    }
    if (compound && compound->getNumChildShapes())
    {
        shape = compound;
    }

    if (shape)
    {
        i32 collisionShapeUid = m_data->m_userCollisionShapeHandles.allocHandle();
        InternalCollisionShapeHandle* handle = m_data->m_userCollisionShapeHandles.getHandle(collisionShapeUid);
        handle->m_collisionShape = shape;
        for (i32 i = 0; i < urdfCollisionObjects.size(); i++)
        {
            handle->m_urdfCollisionObjects.push_back(urdfCollisionObjects[i]);
        }
        serverStatusOut.m_createUserShapeResultArgs.m_userShapeUniqueId = collisionShapeUid;
        m_data->m_worldImporters.push_back(worldImporter);
        serverStatusOut.m_type = CMD_CREATE_COLLISION_SHAPE_COMPLETED;
    }
    else
    {
        delete worldImporter;
    }

    return hasStatus;
}

static void gatherVertices(const Transform2& trans, const CollisionShape* colShape, AlignedObjectArray<Vec3>& verticesOut, i32 collisionShapeIndex)
{
    switch (colShape->getShapeType())
    {
        case COMPOUND_SHAPE_PROXYTYPE:
        {
            const CompoundShape* compound = (const CompoundShape*)colShape;
            for (i32 i = 0; i < compound->getNumChildShapes(); i++)
            {
                Transform2 childTr = trans * compound->getChildTransform(i);
                if ((collisionShapeIndex < 0) || (collisionShapeIndex == i))
                {
                    gatherVertices(childTr, compound->getChildShape(i), verticesOut, collisionShapeIndex);
                }
            }
            break;
        }
        case CONVEX_HULL_SHAPE_PROXYTYPE:
        {
            const ConvexHullShape* convex = (const ConvexHullShape*)colShape;
            Vec3 vtx;
            for (i32 i = 0; i < convex->getNumVertices(); i++)
            {
                convex->getVertex(i, vtx);
                Vec3 trVertex = trans * vtx;
                verticesOut.push_back(trVertex);
            }
            break;
        }
        default:
        {
            printf("?\n");
        }
    }
}

bool PhysicsServerCommandProcessor::processResetMeshDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REQUEST_MESH_DATA");
    serverStatusOut.m_type = CMD_RESET_MESH_DATA_FAILED;
    i32 sizeInBytes = 0;

    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_requestMeshDataArgs.m_bodyUniqueId);
    if (bodyHandle)
    {
        i32 totalBytesPerVertex = sizeof(Vec3);
        double* vertexUpload = (double*)bufferServerToClient;

#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD

        if (bodyHandle->m_softBody)
        {
            SoftBody* psb = bodyHandle->m_softBody;

            i32 numVertices = psb->m_nodes.size();
            if (clientCmd.m_resetMeshDataArgs.m_numVertices == numVertices)
            {
                if (clientCmd.m_updateFlags & D3_MESH_DATA_SIMULATION_MESH_VELOCITY)
                {
                    for (i32 i = 0; i < numVertices; ++i)
                    {
                        SoftBody::Node& n = psb->m_nodes[i];
                        n.m_v.setVal(vertexUpload[i * 3 + 0], vertexUpload[i * 3 + 1], vertexUpload[i * 3 + 2]);
                        n.m_vn.setVal(vertexUpload[i * 3 + 0], vertexUpload[i * 3 + 1], vertexUpload[i * 3 + 2]);
                    }
                }
                else
                {
                    for (i32 i = 0; i < numVertices; ++i)
                    {
                        SoftBody::Node& n = psb->m_nodes[i];
                        n.m_x.setVal(vertexUpload[i * 3 + 0], vertexUpload[i * 3 + 1], vertexUpload[i * 3 + 2]);
                        n.m_q.setVal(vertexUpload[i * 3 + 0], vertexUpload[i * 3 + 1], vertexUpload[i * 3 + 2]);
                    }
                }
                serverStatusOut.m_type = CMD_RESET_MESH_DATA_COMPLETED;
            }
        }
#endif  //SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    }
    serverStatusOut.m_numDataStreamBytes = 0;

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestMeshDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REQUEST_MESH_DATA");
    serverStatusOut.m_type = CMD_REQUEST_MESH_DATA_FAILED;
    serverStatusOut.m_numDataStreamBytes = 0;
    i32 sizeInBytes = 0;

    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_requestMeshDataArgs.m_bodyUniqueId);
    if (bodyHandle)
    {
        i32 totalBytesPerVertex = sizeof(Vec3);
        Vec3* verticesOut = (Vec3*)bufferServerToClient;
        const CollisionShape* colShape = 0;

        if (bodyHandle->m_multiBody)
        {
            //collision shape

            if (clientCmd.m_requestMeshDataArgs.m_linkIndex == -1)
            {
                colShape = bodyHandle->m_multiBody->getBaseCollider()->getCollisionShape();
            }
            else
            {
                colShape = bodyHandle->m_multiBody->getLinkCollider(clientCmd.m_requestMeshDataArgs.m_linkIndex)->getCollisionShape();
            }
        }
        if (bodyHandle->m_rigidBody)
        {
            colShape = bodyHandle->m_rigidBody->getCollisionShape();
        }

        if (colShape)
        {
            AlignedObjectArray<Vec3> vertices;
            Transform2 tr;
            tr.setIdentity();
            i32 collisionShapeIndex = -1;
            if (clientCmd.m_updateFlags & D3_MESH_DATA_COLLISIONSHAPEINDEX)
            {
                collisionShapeIndex = clientCmd.m_requestMeshDataArgs.m_collisionShapeIndex;
            }
            gatherVertices(tr, colShape, vertices, collisionShapeIndex);

            i32 numVertices = vertices.size();
            i32 maxNumVertices = bufferSizeInBytes / totalBytesPerVertex - 1;
            i32 numVerticesRemaining = numVertices - clientCmd.m_requestMeshDataArgs.m_startingVertex;
            i32 verticesCopied = d3Min(maxNumVertices, numVerticesRemaining);

            if (verticesCopied > 0)
            {
                memcpy(verticesOut, &vertices[0], sizeof(Vec3) * verticesCopied);
            }

            sizeInBytes = verticesCopied * sizeof(Vec3);
            serverStatusOut.m_type = CMD_REQUEST_MESH_DATA_COMPLETED;
            serverStatusOut.m_sendMeshDataArgs.m_numVerticesCopied = verticesCopied;
            serverStatusOut.m_sendMeshDataArgs.m_startingVertex = clientCmd.m_requestMeshDataArgs.m_startingVertex;
            serverStatusOut.m_sendMeshDataArgs.m_numVerticesRemaining = numVerticesRemaining - verticesCopied;
        }

#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD

        if (bodyHandle->m_softBody)
        {
            SoftBody* psb = bodyHandle->m_softBody;

            i32 flags = 0;
            if (clientCmd.m_updateFlags & D3_MESH_DATA_FLAGS)
            {
                flags = clientCmd.m_requestMeshDataArgs.m_flags;
            }

            bool separateRenderMesh = false;
            if ((clientCmd.m_updateFlags & D3_MESH_DATA_SIMULATION_MESH) == 0 && (flags & D3_MESH_DATA_SIMULATION_MESH) == 0)
            {
                separateRenderMesh = (psb->m_renderNodes.size() != 0);
            }
            bool requestVelocity = clientCmd.m_updateFlags & D3_MESH_DATA_SIMULATION_MESH_VELOCITY;

            i32 numVertices = separateRenderMesh ? psb->m_renderNodes.size() : psb->m_nodes.size();
            i32 maxNumVertices = bufferSizeInBytes / totalBytesPerVertex - 1;
            i32 numVerticesRemaining = numVertices - clientCmd.m_requestMeshDataArgs.m_startingVertex;
            i32 verticesCopied = d3Min(maxNumVertices, numVerticesRemaining);
            for (i32 i = 0; i < verticesCopied; ++i)
            {
                if (separateRenderMesh)
                {
                    const SoftBody::RenderNode& n = psb->m_renderNodes[i + clientCmd.m_requestMeshDataArgs.m_startingVertex];
                    if(requestVelocity){
                        drx3DWarning("Request mesh velocity not implemented for Render Mesh.");
                        return hasStatus;
                    }
                    verticesOut[i].setVal(n.m_x.x(), n.m_x.y(), n.m_x.z());
                }
                else
                {
                    const SoftBody::Node& n = psb->m_nodes[i + clientCmd.m_requestMeshDataArgs.m_startingVertex];
                    if(!requestVelocity){
                        verticesOut[i].setVal(n.m_x.x(), n.m_x.y(), n.m_x.z());
                    }
                    else{
                        verticesOut[i].setVal(n.m_v.x(), n.m_v.y(), n.m_v.z());
                    }
                }
            }
            sizeInBytes = verticesCopied * sizeof(Vec3);
            serverStatusOut.m_type = CMD_REQUEST_MESH_DATA_COMPLETED;
            serverStatusOut.m_sendMeshDataArgs.m_numVerticesCopied = verticesCopied;
            serverStatusOut.m_sendMeshDataArgs.m_startingVertex = clientCmd.m_requestMeshDataArgs.m_startingVertex;
            serverStatusOut.m_sendMeshDataArgs.m_numVerticesRemaining = numVerticesRemaining - verticesCopied;
        }
#endif  //SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    }

    serverStatusOut.m_numDataStreamBytes = sizeInBytes;

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCreateVisualShapeCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    serverStatusOut.m_type = CMD_CREATE_VISUAL_SHAPE_FAILED;
    double globalScaling = 1.f;
    i32 flags = 0;
    CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
    URDFImporter u2b(m_data->m_guiHelper, m_data->m_pluginManager.getRenderInterface(), fileIO, globalScaling, flags);
    u2b.setEnableTinyRenderer(m_data->m_enableTinyRenderer);
    Transform2 localInertiaFrame;
    localInertiaFrame.setIdentity();

    tukk pathPrefix = "";
    i32 visualShapeUniqueId = -1;

    UrdfVisual visualShape;
    for (i32 userShapeIndex = 0; userShapeIndex < clientCmd.m_createUserShapeArgs.m_numUserShapes; userShapeIndex++)
    {
        Transform2 childTrans;
        childTrans.setIdentity();
        visualShape.m_geometry.m_type = (UrdfGeomTypes)clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_type;
        char relativeFileName[1024];
        char pathPrefix[1024];
        pathPrefix[0] = 0;

        const b3CreateUserShapeData& visShape = clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex];

        switch (visualShape.m_geometry.m_type)
        {
            case URDF_GEOM_CYLINDER:
            {
                visualShape.m_geometry.m_capsuleHeight = visShape.m_capsuleHeight;
                visualShape.m_geometry.m_capsuleRadius = visShape.m_capsuleRadius;
                break;
            }
            case URDF_GEOM_BOX:
            {
                visualShape.m_geometry.m_boxSize.setVal(2. * visShape.m_boxHalfExtents[0],
                                                          2. * visShape.m_boxHalfExtents[1],
                                                          2. * visShape.m_boxHalfExtents[2]);
                break;
            }
            case URDF_GEOM_SPHERE:
            {
                visualShape.m_geometry.m_sphereRadius = visShape.m_sphereRadius;
                break;
            }
            case URDF_GEOM_CAPSULE:
            {
                visualShape.m_geometry.m_hasFromTo = visShape.m_hasFromTo;
                if (visualShape.m_geometry.m_hasFromTo)
                {
                    visualShape.m_geometry.m_capsuleFrom.setVal(visShape.m_capsuleFrom[0],
                                                                  visShape.m_capsuleFrom[1],
                                                                  visShape.m_capsuleFrom[2]);
                    visualShape.m_geometry.m_capsuleTo.setVal(visShape.m_capsuleTo[0],
                                                                visShape.m_capsuleTo[1],
                                                                visShape.m_capsuleTo[2]);
                }
                else
                {
                    visualShape.m_geometry.m_capsuleHeight = visShape.m_capsuleHeight;
                    visualShape.m_geometry.m_capsuleRadius = visShape.m_capsuleRadius;
                }
                break;
            }
            case URDF_GEOM_MESH:
            {
                STxt fileName = clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_meshFileName;
                if (fileName.length())
                {
                    const STxt& error_message_prefix = "";
                    STxt out_found_filename;
                    i32 out_type;
                    if (fileIO->findResourcePath(fileName.c_str(), relativeFileName, 1024))
                    {
                        b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);
                    }

                    bool foundFile = UrdfFindMeshFile(fileIO, pathPrefix, relativeFileName, error_message_prefix, &out_found_filename, &out_type);
                    if (foundFile)
                    {
                        visualShape.m_geometry.m_meshFileType = out_type;
                        visualShape.m_geometry.m_meshFileName = fileName;
                    }
                    else
                    {
                    }
                }
                else
                {
                    visualShape.m_geometry.m_meshFileType = UrdfGeometry::MEMORY_VERTICES;
                    i32 numVertices = clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_numVertices;
                    i32 numIndices = clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_numIndices;
                    i32 numUVs = clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_numUVs;
                    i32 numNormals = clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_numNormals;

                    if (numVertices > 0 && numIndices > 0)
                    {
                        tuk data = bufferServerToClient;
                        double* vertexUpload = (double*)data;
                        i32* indexUpload = (i32*)(data + numVertices * sizeof(double) * 3);
                        double* normalUpload = (double*)(data + numVertices * sizeof(double) * 3 + numIndices * sizeof(i32));
                        double* uvUpload = (double*)(data + numVertices * sizeof(double) * 3 + numIndices * sizeof(i32) + numNormals * sizeof(double) * 3);

                        for (i32 i = 0; i < numIndices; i++)
                        {
                            visualShape.m_geometry.m_indices.push_back(indexUpload[i]);
                        }
                        for (i32 i = 0; i < numVertices; i++)
                        {
                            Vec3 v0(vertexUpload[i * 3 + 0],
                                         vertexUpload[i * 3 + 1],
                                         vertexUpload[i * 3 + 2]);
                            visualShape.m_geometry.m_vertices.push_back(v0);
                        }
                        for (i32 i = 0; i < numNormals; i++)
                        {
                            Vec3 normal(normalUpload[i * 3 + 0],
                                             normalUpload[i * 3 + 1],
                                             normalUpload[i * 3 + 2]);
                            visualShape.m_geometry.m_normals.push_back(normal);
                        }
                        for (i32 i = 0; i < numUVs; i++)
                        {
                            Vec3 uv(uvUpload[i * 2 + 0], uvUpload[i * 2 + 1], 0);
                            visualShape.m_geometry.m_uvs.push_back(uv);
                        }
                    }
                }
                visualShape.m_geometry.m_meshScale.setVal(clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_meshScale[0],
                                                            clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_meshScale[1],
                                                            clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_meshScale[2]);
                break;
            }

            default:
            {
            }
        };
        visualShape.m_name = "in_memory";
        visualShape.m_materialName = "";
        visualShape.m_sourceFileLocation = "in_memory_unknown_line";
        visualShape.m_linkLocalFrame.setIdentity();
        visualShape.m_geometry.m_hasLocalMaterial = false;

        bool hasRGBA = (clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_visualFlags & GEOM_VISUAL_HAS_RGBA_COLOR) != 0;
        ;
        bool hasSpecular = (clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_visualFlags & GEOM_VISUAL_HAS_SPECULAR_COLOR) != 0;
        ;
        visualShape.m_geometry.m_hasLocalMaterial = hasRGBA | hasSpecular;
        if (visualShape.m_geometry.m_hasLocalMaterial)
        {
            if (hasRGBA)
            {
                visualShape.m_geometry.m_localMaterial.m_matColor.m_rgbaColor.setVal(
                    clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_rgbaColor[0],
                    clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_rgbaColor[1],
                    clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_rgbaColor[2],
                    clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_rgbaColor[3]);
            }
            else
            {
                visualShape.m_geometry.m_localMaterial.m_matColor.m_rgbaColor.setVal(1, 1, 1, 1);
            }
            if (hasSpecular)
            {
                visualShape.m_geometry.m_localMaterial.m_matColor.m_specularColor.setVal(
                    clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_specularColor[0],
                    clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_specularColor[1],
                    clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_specularColor[2]);
            }
            else
            {
                visualShape.m_geometry.m_localMaterial.m_matColor.m_specularColor.setVal(0.4, 0.4, 0.4);
            }
        }

        if (clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_hasChildTransform != 0)
        {
            childTrans.setOrigin(Vec3(clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_childPosition[0],
                                           clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_childPosition[1],
                                           clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_childPosition[2]));
            childTrans.setRotation(Quat(
                clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_childOrientation[0],
                clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_childOrientation[1],
                clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_childOrientation[2],
                clientCmd.m_createUserShapeArgs.m_shapes[userShapeIndex].m_childOrientation[3]));
        }

        if (visualShapeUniqueId < 0)
        {
            visualShapeUniqueId = m_data->m_userVisualShapeHandles.allocHandle();
        }
        InternalVisualShapeHandle* visualHandle = m_data->m_userVisualShapeHandles.getHandle(visualShapeUniqueId);
        visualHandle->m_OpenGLGraphicsIndex = -1;
        visualHandle->m_tinyRendererVisualShapeIndex = -1;
        //tinyrenderer doesn't separate shape versus instance, so create it when creating the multibody instance
        //store needed info for tinyrenderer

        visualShape.m_linkLocalFrame = childTrans;
        visualHandle->m_visualShapes.push_back(visualShape);
        visualHandle->m_pathPrefixes.push_back(pathPrefix[0] ? pathPrefix : "");

        serverStatusOut.m_createUserShapeResultArgs.m_userShapeUniqueId = visualShapeUniqueId;
        serverStatusOut.m_type = CMD_CREATE_VISUAL_SHAPE_COMPLETED;
    }

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCustomCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CUSTOM_COMMAND_FAILED;
    serverCmd.m_customCommandResultArgs.m_returnDataSizeInBytes = 0;
    serverCmd.m_customCommandResultArgs.m_returnDataType = -1;
    serverCmd.m_customCommandResultArgs.m_returnDataStart = 0;

    serverCmd.m_customCommandResultArgs.m_pluginUniqueId = -1;

    if (clientCmd.m_updateFlags & CMD_CUSTOM_COMMAND_LOAD_PLUGIN)
    {
        //pluginPath could be registered or load from disk
        tukk postFix = "";
        if (clientCmd.m_updateFlags & CMD_CUSTOM_COMMAND_LOAD_PLUGIN_POSTFIX)
        {
            postFix = clientCmd.m_customCommandArgs.m_postFix;
        }

        i32 pluginUniqueId = m_data->m_pluginManager.loadPlugin(clientCmd.m_customCommandArgs.m_pluginPath, postFix);
        if (pluginUniqueId >= 0)
        {
            serverCmd.m_customCommandResultArgs.m_pluginUniqueId = pluginUniqueId;

            serverCmd.m_type = CMD_CUSTOM_COMMAND_COMPLETED;
        }
    }
    if (clientCmd.m_updateFlags & CMD_CUSTOM_COMMAND_UNLOAD_PLUGIN)
    {
        m_data->m_pluginManager.unloadPlugin(clientCmd.m_customCommandArgs.m_pluginUniqueId);
        serverCmd.m_type = CMD_CUSTOM_COMMAND_COMPLETED;
    }

    if (clientCmd.m_updateFlags & CMD_CUSTOM_COMMAND_EXECUTE_PLUGIN_COMMAND)
    {
        i32 startBytes = clientCmd.m_customCommandArgs.m_startingReturnBytes;
        if (startBytes == 0)
        {
            i32 result = m_data->m_pluginManager.executePluginCommand(clientCmd.m_customCommandArgs.m_pluginUniqueId, &clientCmd.m_customCommandArgs.m_arguments);
            serverCmd.m_customCommandResultArgs.m_executeCommandResult = result;
        }
        const b3UserDataValue* returnData = m_data->m_pluginManager.getReturnData(clientCmd.m_customCommandArgs.m_pluginUniqueId);
        if (returnData)
        {
            i32 totalRemain = returnData->m_length - startBytes;
            i32 numBytes = totalRemain <= bufferSizeInBytes ? totalRemain : bufferSizeInBytes;
            serverStatusOut.m_numDataStreamBytes = numBytes;
            for (i32 i = 0; i < numBytes; i++)
            {
                bufferServerToClient[i] = returnData->m_data1[i+ startBytes];
            }
            serverCmd.m_customCommandResultArgs.m_returnDataSizeInBytes = returnData->m_length;
            serverCmd.m_customCommandResultArgs.m_returnDataType = returnData->m_type;
            serverCmd.m_customCommandResultArgs.m_returnDataStart = startBytes;
        }
        else
        {
            serverStatusOut.m_numDataStreamBytes = 0;
        }

        serverCmd.m_type = CMD_CUSTOM_COMMAND_COMPLETED;
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processUserDebugDrawCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_USER_DEBUG_DRAW");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_USER_DEBUG_DRAW_FAILED;

    i32 trackingVisualShapeIndex = -1;

    if (clientCmd.m_userDebugDrawArgs.m_parentObjectUniqueId >= 0)
    {
        InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_userDebugDrawArgs.m_parentObjectUniqueId);
        if (bodyHandle)
        {
            i32 linkIndex = -1;

            if (bodyHandle->m_multiBody)
            {
                i32 linkIndex = clientCmd.m_userDebugDrawArgs.m_parentLinkIndex;
                if (linkIndex == -1)
                {
                    if (bodyHandle->m_multiBody->getBaseCollider())
                    {
                        trackingVisualShapeIndex = bodyHandle->m_multiBody->getBaseCollider()->getUserIndex();
                    }
                }
                else
                {
                    if (linkIndex >= 0 && linkIndex < bodyHandle->m_multiBody->getNumLinks())
                    {
                        if (bodyHandle->m_multiBody->getLink(linkIndex).m_collider)
                        {
                            trackingVisualShapeIndex = bodyHandle->m_multiBody->getLink(linkIndex).m_collider->getUserIndex();
                        }
                    }
                }
            }
            if (bodyHandle->m_rigidBody)
            {
                trackingVisualShapeIndex = bodyHandle->m_rigidBody->getUserIndex();
            }
        }
    }

    if (clientCmd.m_updateFlags & USER_DEBUG_ADD_PARAMETER)
    {
        i32 uid = m_data->m_guiHelper->addUserDebugParameter(
            clientCmd.m_userDebugDrawArgs.m_text,
            clientCmd.m_userDebugDrawArgs.m_rangeMin,
            clientCmd.m_userDebugDrawArgs.m_rangeMax,
            clientCmd.m_userDebugDrawArgs.m_startValue);
        serverCmd.m_userDebugDrawArgs.m_debugItemUniqueId = uid;
        serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
    }
    if (clientCmd.m_updateFlags & USER_DEBUG_READ_PARAMETER)
    {
        i32 ok = m_data->m_guiHelper->readUserDebugParameter(
            clientCmd.m_userDebugDrawArgs.m_itemUniqueId,
            &serverCmd.m_userDebugDrawArgs.m_parameterValue);
        if (ok)
        {
            serverCmd.m_type = CMD_USER_DEBUG_DRAW_PARAMETER_COMPLETED;
        }
    }
    if ((clientCmd.m_updateFlags & USER_DEBUG_SET_CUSTOM_OBJECT_COLOR) || (clientCmd.m_updateFlags & USER_DEBUG_REMOVE_CUSTOM_OBJECT_COLOR))
    {
        i32 bodyUniqueId = clientCmd.m_userDebugDrawArgs.m_objectUniqueId;
        InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
        if (body)
        {
            CollisionObject2* destColObj = 0;

            if (body->m_multiBody)
            {
                if (clientCmd.m_userDebugDrawArgs.m_linkIndex == -1)
                {
                    destColObj = body->m_multiBody->getBaseCollider();
                }
                else
                {
                    if (clientCmd.m_userDebugDrawArgs.m_linkIndex >= 0 && clientCmd.m_userDebugDrawArgs.m_linkIndex < body->m_multiBody->getNumLinks())
                    {
                        destColObj = body->m_multiBody->getLink(clientCmd.m_userDebugDrawArgs.m_linkIndex).m_collider;
                    }
                }
            }
            if (body->m_rigidBody)
            {
                destColObj = body->m_rigidBody;
            }

            if (destColObj)
            {
                if (clientCmd.m_updateFlags & USER_DEBUG_REMOVE_CUSTOM_OBJECT_COLOR)
                {
                    destColObj->removeCustomDebugColor();
                    serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
                }
                if (clientCmd.m_updateFlags & USER_DEBUG_SET_CUSTOM_OBJECT_COLOR)
                {
                    Vec3 objectColorRGB;
                    objectColorRGB.setVal(clientCmd.m_userDebugDrawArgs.m_objectDebugColorRGB[0],
                                            clientCmd.m_userDebugDrawArgs.m_objectDebugColorRGB[1],
                                            clientCmd.m_userDebugDrawArgs.m_objectDebugColorRGB[2]);
                    destColObj->setCustomDebugColor(objectColorRGB);
                    serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
                }
            }
        }
    }

    if (clientCmd.m_updateFlags & USER_DEBUG_HAS_TEXT)
    {
        //addUserDebugText3D( const double orientation[4], const double textColorRGB[3], double size, double lifeTime, i32 trackingObjectUniqueId, i32 optionFlags){return -1;}

        i32 optionFlags = clientCmd.m_userDebugDrawArgs.m_optionFlags;

        if ((clientCmd.m_updateFlags & USER_DEBUG_HAS_TEXT_ORIENTATION) == 0)
        {
            optionFlags |= DEB_DEBUG_TEXT_ALWAYS_FACE_CAMERA;
        }

        i32 replaceItemUniqueId = -1;
        if ((clientCmd.m_updateFlags & USER_DEBUG_HAS_REPLACE_ITEM_UNIQUE_ID) != 0)
        {
            replaceItemUniqueId = clientCmd.m_userDebugDrawArgs.m_replaceItemUniqueId;
        }

        i32 uid = m_data->m_guiHelper->addUserDebugText3D(clientCmd.m_userDebugDrawArgs.m_text,
                                                          clientCmd.m_userDebugDrawArgs.m_textPositionXYZ,
                                                          clientCmd.m_userDebugDrawArgs.m_textOrientation,
                                                          clientCmd.m_userDebugDrawArgs.m_textColorRGB,
                                                          clientCmd.m_userDebugDrawArgs.m_textSize,
                                                          clientCmd.m_userDebugDrawArgs.m_lifeTime,
                                                          trackingVisualShapeIndex,
                                                          optionFlags,
                                                          replaceItemUniqueId);

        if (uid >= 0)
        {
            serverCmd.m_userDebugDrawArgs.m_debugItemUniqueId = uid;
            serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
        }
    }

    if (clientCmd.m_updateFlags & USER_DEBUG_HAS_LINE)
    {
        i32 replaceItemUid = -1;
        if (clientCmd.m_updateFlags & USER_DEBUG_HAS_REPLACE_ITEM_UNIQUE_ID)
        {
            replaceItemUid = clientCmd.m_userDebugDrawArgs.m_replaceItemUniqueId;
        }

        i32 uid = m_data->m_guiHelper->addUserDebugLine(
            clientCmd.m_userDebugDrawArgs.m_debugLineFromXYZ,
            clientCmd.m_userDebugDrawArgs.m_debugLineToXYZ,
            clientCmd.m_userDebugDrawArgs.m_debugLineColorRGB,
            clientCmd.m_userDebugDrawArgs.m_lineWidth,
            clientCmd.m_userDebugDrawArgs.m_lifeTime,
            trackingVisualShapeIndex,
            replaceItemUid);

        if (uid >= 0)
        {
            serverCmd.m_userDebugDrawArgs.m_debugItemUniqueId = uid;
            serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
        }
    }

    if (clientCmd.m_updateFlags & USER_DEBUG_HAS_POINTS)
    {
        i32 replaceItemUid = -1;
        if (clientCmd.m_updateFlags & USER_DEBUG_HAS_REPLACE_ITEM_UNIQUE_ID)
        {
            replaceItemUid = clientCmd.m_userDebugDrawArgs.m_replaceItemUniqueId;
        }

        i32 pointNum = clientCmd.m_userDebugDrawArgs.m_debugPointNum;

        double* pointPositionsUpload = (double*)bufferServerToClient;
        double* pointPositions = (double*)malloc(pointNum * 3 * sizeof(double));
        double* pointColorsUpload = (double*)(bufferServerToClient + pointNum * 3 * sizeof(double));
        double* pointColors = (double*)malloc(pointNum * 3 * sizeof(double));
        for (i32 i = 0; i < pointNum; i++) {
            pointPositions[i * 3 + 0] = pointPositionsUpload[i * 3 + 0];
            pointPositions[i * 3 + 1] = pointPositionsUpload[i * 3 + 1];
            pointPositions[i * 3 + 2] = pointPositionsUpload[i * 3 + 2];
            pointColors[i * 3 + 0] = pointColorsUpload[i * 3 + 0];
            pointColors[i * 3 + 1] = pointColorsUpload[i * 3 + 1];
            pointColors[i * 3 + 2] = pointColorsUpload[i * 3 + 2];
        }
        m_data->m_debugPointsDatas.push_back(pointPositions);
        m_data->m_debugPointsDatas.push_back(pointColors);

        i32 uid = m_data->m_guiHelper->addUserDebugPoints(
            pointPositions,
            pointColors,
            clientCmd.m_userDebugDrawArgs.m_pointSize,
            clientCmd.m_userDebugDrawArgs.m_lifeTime,
            trackingVisualShapeIndex,
            replaceItemUid,
            clientCmd.m_userDebugDrawArgs.m_debugPointNum);

        if (uid >= 0)
        {
            serverCmd.m_userDebugDrawArgs.m_debugItemUniqueId = uid;
            serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
        }
    }

    if (clientCmd.m_updateFlags & USER_DEBUG_REMOVE_ALL)
    {
        m_data->m_guiHelper->removeAllUserDebugItems();
        serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
    }

    if (clientCmd.m_updateFlags & USER_DEBUG_REMOVE_ALL_PARAMETERS)
    {
        m_data->m_guiHelper->removeAllUserParameters();
        serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
    }

    if (clientCmd.m_updateFlags & USER_DEBUG_REMOVE_ONE_ITEM)
    {
        m_data->m_guiHelper->removeUserDebugItem(clientCmd.m_userDebugDrawArgs.m_itemUniqueId);
        serverCmd.m_type = CMD_USER_DEBUG_DRAW_COMPLETED;
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processSetVRCameraStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_SET_VR_CAMERA_STATE");

    if (clientCmd.m_updateFlags & VR_CAMERA_ROOT_POSITION)
    {
        gVRTeleportPos1[0] = clientCmd.m_vrCameraStateArguments.m_rootPosition[0];
        gVRTeleportPos1[1] = clientCmd.m_vrCameraStateArguments.m_rootPosition[1];
        gVRTeleportPos1[2] = clientCmd.m_vrCameraStateArguments.m_rootPosition[2];
    }
    if (clientCmd.m_updateFlags & VR_CAMERA_ROOT_ORIENTATION)
    {
        gVRTeleportOrn[0] = clientCmd.m_vrCameraStateArguments.m_rootOrientation[0];
        gVRTeleportOrn[1] = clientCmd.m_vrCameraStateArguments.m_rootOrientation[1];
        gVRTeleportOrn[2] = clientCmd.m_vrCameraStateArguments.m_rootOrientation[2];
        gVRTeleportOrn[3] = clientCmd.m_vrCameraStateArguments.m_rootOrientation[3];
    }

    if (clientCmd.m_updateFlags & VR_CAMERA_ROOT_TRACKING_OBJECT)
    {
        gVRTrackingObjectUniqueId = clientCmd.m_vrCameraStateArguments.m_trackingObjectUniqueId;
    }

    if (clientCmd.m_updateFlags & VR_CAMERA_FLAG)
    {
        gVRTrackingObjectFlag = clientCmd.m_vrCameraStateArguments.m_trackingObjectFlag;
    }

    serverStatusOut.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestVREventsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    //DRX3D_PROFILE("CMD_REQUEST_VR_EVENTS_DATA");
    serverStatusOut.m_sendVREvents.m_numVRControllerEvents = 0;

    for (i32 i = 0; i < MAX_VR_CONTROLLERS; i++)
    {
        b3VRControllerEvent& event = m_data->m_vrControllerEvents.m_vrEvents[i];

        if (clientCmd.m_updateFlags & event.m_deviceType)
        {
            if (event.m_numButtonEvents + event.m_numMoveEvents)
            {
                serverStatusOut.m_sendVREvents.m_controllerEvents[serverStatusOut.m_sendVREvents.m_numVRControllerEvents++] = event;
                event.m_numButtonEvents = 0;
                event.m_numMoveEvents = 0;
                for (i32 b = 0; b < MAX_VR_BUTTONS; b++)
                {
                    event.m_buttons[b] = 0;
                }
            }
        }
    }
    serverStatusOut.m_type = CMD_REQUEST_VR_EVENTS_DATA_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestMouseEventsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    serverStatusOut.m_sendMouseEvents.m_numMouseEvents = m_data->m_mouseEvents.size();
    if (serverStatusOut.m_sendMouseEvents.m_numMouseEvents > MAX_MOUSE_EVENTS)
    {
        serverStatusOut.m_sendMouseEvents.m_numMouseEvents = MAX_MOUSE_EVENTS;
    }
    for (i32 i = 0; i < serverStatusOut.m_sendMouseEvents.m_numMouseEvents; i++)
    {
        serverStatusOut.m_sendMouseEvents.m_mouseEvents[i] = m_data->m_mouseEvents[i];
    }

    m_data->m_mouseEvents.resize(0);
    serverStatusOut.m_type = CMD_REQUEST_MOUSE_EVENTS_DATA_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestKeyboardEventsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    //DRX3D_PROFILE("CMD_REQUEST_KEYBOARD_EVENTS_DATA");
    bool hasStatus = true;
    serverStatusOut.m_sendKeyboardEvents.m_numKeyboardEvents = m_data->m_keyboardEvents.size();
    if (serverStatusOut.m_sendKeyboardEvents.m_numKeyboardEvents > MAX_KEYBOARD_EVENTS)
    {
        serverStatusOut.m_sendKeyboardEvents.m_numKeyboardEvents = MAX_KEYBOARD_EVENTS;
    }
    for (i32 i = 0; i < serverStatusOut.m_sendKeyboardEvents.m_numKeyboardEvents; i++)
    {
        serverStatusOut.m_sendKeyboardEvents.m_keyboardEvents[i] = m_data->m_keyboardEvents[i];
    }

    AlignedObjectArray<b3KeyboardEvent> events;

    //remove out-of-date events
    for (i32 i = 0; i < m_data->m_keyboardEvents.size(); i++)
    {
        b3KeyboardEvent event = m_data->m_keyboardEvents[i];
        if (event.m_keyState & eButtonIsDown)
        {
            event.m_keyState = eButtonIsDown;
            events.push_back(event);
        }
    }
    m_data->m_keyboardEvents.resize(events.size());
    for (i32 i = 0; i < events.size(); i++)
    {
        m_data->m_keyboardEvents[i] = events[i];
    }

    serverStatusOut.m_type = CMD_REQUEST_KEYBOARD_EVENTS_DATA_COMPLETED;
    return hasStatus;
}

#if __cplusplus >= 201103L
#include <atomic>

struct CastSyncInfo
{
    std::atomic<i32> m_nextTaskNumber;

    CastSyncInfo() : m_nextTaskNumber(0) {}

    inline i32 getNextTask()
    {
        return m_nextTaskNumber++;
    }
};
#else   // __cplusplus >= 201103L
struct CastSyncInfo
{
     i32 m_nextTaskNumber;
    SpinMutex m_taskLock;

    CastSyncInfo() : m_nextTaskNumber(0) {}

    inline i32 getNextTask()
    {
        m_taskLock.lock();
        i32k taskNr = m_nextTaskNumber++;
        m_taskLock.unlock();
        return taskNr;
    }
};
#endif  // __cplusplus >= 201103L

struct FilteredClosestRayResultCallback : public CollisionWorld::ClosestRayResultCallback
{
    FilteredClosestRayResultCallback(const Vec3& rayFromWorld, const Vec3& rayToWorld, i32 collisionFilterMask)
        : CollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld),
          m_collisionFilterMask(collisionFilterMask)
    {
    }

    i32 m_collisionFilterMask;

    virtual Scalar addSingleResult(CollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
    {
        bool collides = (rayResult.m_collisionObject->getBroadphaseHandle()->m_collisionFilterGroup & m_collisionFilterMask) != 0;
        if (!collides)
            return m_closestHitFraction;
        return CollisionWorld::ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
    }
};

struct FilteredAllHitsRayResultCallback : public CollisionWorld::AllHitsRayResultCallback
{
    FilteredAllHitsRayResultCallback(const Vec3& rayFromWorld, const Vec3& rayToWorld, i32 collisionFilterMask, Scalar fractionEpsilon)
        : CollisionWorld::AllHitsRayResultCallback(rayFromWorld, rayToWorld),
          m_collisionFilterMask(collisionFilterMask),
          m_fractionEpsilon(fractionEpsilon)
    {
    }

    i32 m_collisionFilterMask;
    Scalar m_fractionEpsilon;

    virtual Scalar addSingleResult(CollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
    {
        bool collides = (rayResult.m_collisionObject->getBroadphaseHandle()->m_collisionFilterGroup & m_collisionFilterMask) != 0;
        if (!collides)
            return m_closestHitFraction;
        //remove duplicate hits:
        //same collision object, link index and hit fraction
        bool isDuplicate = false;

        for (i32 i = 0; i < m_collisionObjects.size(); i++)
        {
            if (m_collisionObjects[i] == rayResult.m_collisionObject)
            {
                Scalar diffFraction = m_hitFractions[i] - rayResult.m_hitFraction;
                if (Equal(diffFraction, m_fractionEpsilon))
                {
                    isDuplicate = true;
                    break;
                }
            }
        }
        if (isDuplicate)
            return m_closestHitFraction;

        return CollisionWorld::AllHitsRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
    }
};

struct BatchRayCaster
{
    b3ThreadPool* m_threadPool;
    CastSyncInfo* m_syncInfo;
    const CollisionWorld* m_world;
    const b3RayData* m_rayInputBuffer;
    b3RayHitInfo* m_hitInfoOutputBuffer;
    i32 m_numRays;
    i32 m_reportHitNumber;
    i32 m_collisionFilterMask;
    Scalar m_fractionEpsilon;

    BatchRayCaster(b3ThreadPool* threadPool, const CollisionWorld* world, const b3RayData* rayInputBuffer, b3RayHitInfo* hitInfoOutputBuffer, i32 numRays, i32 reportHitNumber, i32 collisionFilterMask, Scalar fractionEpsilon)
        : m_threadPool(threadPool), m_world(world), m_rayInputBuffer(rayInputBuffer), m_hitInfoOutputBuffer(hitInfoOutputBuffer), m_numRays(numRays), m_reportHitNumber(reportHitNumber), m_collisionFilterMask(collisionFilterMask), m_fractionEpsilon(fractionEpsilon)
    {
        m_syncInfo = new CastSyncInfo;
    }

    ~BatchRayCaster()
    {
        delete m_syncInfo;
    }

    void castRays(i32 numWorkers)
    {
#if DRX3D_THREADSAFE
        if (numWorkers <= 1)
        {
            castSequentially();
        }
        else
        {
            {
                DRX3D_PROFILE("BatchRayCaster_startingWorkerThreads");
                i32 numTasks = d3Min(m_threadPool->numWorkers(), numWorkers - 1);
                for (i32 i = 0; i < numTasks; i++)
                {
                    m_threadPool->runTask(i, BatchRayCaster::rayCastWorker, this);
                }
            }
            rayCastWorker(this);
            m_threadPool->waitForAllTasks();
        }
#else   // DRX3D_THREADSAFE
        castSequentially();
#endif  // DRX3D_THREADSAFE
    }

    static void rayCastWorker(uk arg)
    {
        DRX3D_PROFILE("BatchRayCaster_raycastWorker");
        BatchRayCaster* const obj = (BatchRayCaster*)arg;
        i32k numRays = obj->m_numRays;
        i32 taskNr;
        while (true)
        {
            {
                DRX3D_PROFILE("CastSyncInfo_getNextTask");
                taskNr = obj->m_syncInfo->getNextTask();
            }
            if (taskNr >= numRays)
                return;
            obj->processRay(taskNr);
        }
    }

    void castSequentially()
    {
        for (i32 i = 0; i < m_numRays; i++)
        {
            processRay(i);
        }
    }

    void processRay(i32 ray)
    {
        DRX3D_PROFILE("BatchRayCaster_processRay");
        const double* from = m_rayInputBuffer[ray].m_rayFromPosition;
        const double* to = m_rayInputBuffer[ray].m_rayToPosition;
        Vec3 rayFromWorld(from[0], from[1], from[2]);
        Vec3 rayToWorld(to[0], to[1], to[2]);

        FilteredClosestRayResultCallback rayResultCallback(rayFromWorld, rayToWorld, m_collisionFilterMask);
        rayResultCallback.m_flags |= TriangleRaycastCallback::kF_UseGjkConvexCastRaytest;
        if (m_reportHitNumber >= 0)
        {
            //compute all hits, and select the m_reportHitNumber, if available
            FilteredAllHitsRayResultCallback allResultsCallback(rayFromWorld, rayToWorld, m_collisionFilterMask, m_fractionEpsilon);
            allResultsCallback.m_flags |= TriangleRaycastCallback::kF_UseGjkConvexCastRaytest;
            m_world->rayTest(rayFromWorld, rayToWorld, allResultsCallback);
            if (allResultsCallback.m_collisionObjects.size() > m_reportHitNumber)
            {
                rayResultCallback.m_collisionObject = allResultsCallback.m_collisionObjects[m_reportHitNumber];
                rayResultCallback.m_closestHitFraction = allResultsCallback.m_hitFractions[m_reportHitNumber];
                rayResultCallback.m_hitNormalWorld = allResultsCallback.m_hitNormalWorld[m_reportHitNumber];
                rayResultCallback.m_hitPointWorld = allResultsCallback.m_hitPointWorld[m_reportHitNumber];
            }
        }
        else
        {
            m_world->rayTest(rayFromWorld, rayToWorld, rayResultCallback);
        }

        b3RayHitInfo& hit = m_hitInfoOutputBuffer[ray];
        if (rayResultCallback.hasHit())
        {
            hit.m_hitFraction = rayResultCallback.m_closestHitFraction;

            i32 objectUniqueId = -1;
            i32 linkIndex = -1;

            const RigidBody* body = RigidBody::upcast(rayResultCallback.m_collisionObject);
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
            const SoftBody* softBody = SoftBody::upcast(rayResultCallback.m_collisionObject);
            if (softBody)
            {
                objectUniqueId = rayResultCallback.m_collisionObject->getUserIndex2();
            }
#endif  //SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
            if (body)
            {
                objectUniqueId = rayResultCallback.m_collisionObject->getUserIndex2();
            }
            else
            {
                const MultiBodyLinkCollider* mblB = MultiBodyLinkCollider::upcast(rayResultCallback.m_collisionObject);
                if (mblB && mblB->m_multiBody)
                {
                    linkIndex = mblB->m_link;
                    objectUniqueId = mblB->m_multiBody->getUserIndex2();
                }
            }

            hit.m_hitObjectUniqueId = objectUniqueId;
            hit.m_hitObjectLinkIndex = linkIndex;

            hit.m_hitPositionWorld[0] = rayResultCallback.m_hitPointWorld[0];
            hit.m_hitPositionWorld[1] = rayResultCallback.m_hitPointWorld[1];
            hit.m_hitPositionWorld[2] = rayResultCallback.m_hitPointWorld[2];
            hit.m_hitNormalWorld[0] = rayResultCallback.m_hitNormalWorld[0];
            hit.m_hitNormalWorld[1] = rayResultCallback.m_hitNormalWorld[1];
            hit.m_hitNormalWorld[2] = rayResultCallback.m_hitNormalWorld[2];
        }
        else
        {
            hit.m_hitFraction = 1;
            hit.m_hitObjectUniqueId = -1;
            hit.m_hitObjectLinkIndex = -1;
            hit.m_hitPositionWorld[0] = 0;
            hit.m_hitPositionWorld[1] = 0;
            hit.m_hitPositionWorld[2] = 0;
            hit.m_hitNormalWorld[0] = 0;
            hit.m_hitNormalWorld[1] = 0;
            hit.m_hitNormalWorld[2] = 0;
        }
    }
};

void PhysicsServerCommandProcessor::createThreadPool()
{
#ifdef DRX3D_THREADSAFE
    if (m_data->m_threadPool == 0)
    {
        m_data->m_threadPool = new b3ThreadPool("PhysicsServerCommandProcessorThreadPool");
    }
#endif  //DRX3D_THREADSAFE
}

bool PhysicsServerCommandProcessor::processRequestRaycastIntersectionsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REQUEST_RAY_CAST_INTERSECTIONS");
    serverStatusOut.m_raycastHits.m_numRaycastHits = 0;

    i32k numCommandRays = clientCmd.m_requestRaycastIntersections.m_numCommandRays;
    i32k numStreamingRays = clientCmd.m_requestRaycastIntersections.m_numStreamingRays;
    i32k totalRays = numCommandRays + numStreamingRays;
    i32 numThreads = clientCmd.m_requestRaycastIntersections.m_numThreads;
    i32 reportHitNumber = clientCmd.m_requestRaycastIntersections.m_reportHitNumber;
    i32 collisionFilterMask = clientCmd.m_requestRaycastIntersections.m_collisionFilterMask;
    Scalar fractionEpsilon = clientCmd.m_requestRaycastIntersections.m_fractionEpsilon;
    if (numThreads == 0)
    {
        // When 0 is specified, drx3D can decide how many threads to use.
        // About 16 rays per thread seems to work reasonably well.
        numThreads = d3Max(1, totalRays / 16);
    }
    if (numThreads > 1)
    {
        createThreadPool();
    }

    AlignedObjectArray<b3RayData> rays;
    rays.resize(totalRays);
    if (numCommandRays)
    {
        memcpy(&rays[0], &clientCmd.m_requestRaycastIntersections.m_fromToRays[0], numCommandRays * sizeof(b3RayData));
    }
    if (numStreamingRays)
    {
        memcpy(&rays[numCommandRays], bufferServerToClient, numStreamingRays * sizeof(b3RayData));
    }

    if (clientCmd.m_requestRaycastIntersections.m_parentObjectUniqueId >= 0)
    {
        Transform2 tr;
        tr.setIdentity();

        InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_requestRaycastIntersections.m_parentObjectUniqueId);
        if (bodyHandle)
        {
            i32 linkIndex = -1;
            if (bodyHandle->m_multiBody)
            {
                i32 linkIndex = clientCmd.m_requestRaycastIntersections.m_parentLinkIndex;
                if (linkIndex == -1)
                {
                    tr = bodyHandle->m_multiBody->getBaseWorldTransform();
                }
                else
                {
                    if (linkIndex >= 0 && linkIndex < bodyHandle->m_multiBody->getNumLinks())
                    {
                        tr = bodyHandle->m_multiBody->getLink(linkIndex).m_cachedWorldTransform;
                    }
                }
            }
            if (bodyHandle->m_rigidBody)
            {
                tr = bodyHandle->m_rigidBody->getWorldTransform();
            }
            //convert all rays into world space
            for (i32 i = 0; i < totalRays; i++)
            {
                Vec3 localPosTo(rays[i].m_rayToPosition[0], rays[i].m_rayToPosition[1], rays[i].m_rayToPosition[2]);
                Vec3 worldPosTo = tr * localPosTo;

                Vec3 localPosFrom(rays[i].m_rayFromPosition[0], rays[i].m_rayFromPosition[1], rays[i].m_rayFromPosition[2]);
                Vec3 worldPosFrom = tr * localPosFrom;
                rays[i].m_rayFromPosition[0] = worldPosFrom[0];
                rays[i].m_rayFromPosition[1] = worldPosFrom[1];
                rays[i].m_rayFromPosition[2] = worldPosFrom[2];
                rays[i].m_rayToPosition[0] = worldPosTo[0];
                rays[i].m_rayToPosition[1] = worldPosTo[1];
                rays[i].m_rayToPosition[2] = worldPosTo[2];
            }
        }
    }

    BatchRayCaster batchRayCaster(m_data->m_threadPool, m_data->m_dynamicsWorld, &rays[0], (b3RayHitInfo*)bufferServerToClient, totalRays, reportHitNumber, collisionFilterMask, fractionEpsilon);
    batchRayCaster.castRays(numThreads);

    serverStatusOut.m_numDataStreamBytes = totalRays * sizeof(b3RayData);
    serverStatusOut.m_raycastHits.m_numRaycastHits = totalRays;
    serverStatusOut.m_type = CMD_REQUEST_RAY_CAST_INTERSECTIONS_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestDebugLinesCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REQUEST_DEBUG_LINES");
    i32 curFlags = m_data->m_remoteDebugDrawer->getDebugMode();

    i32 debugMode = clientCmd.m_requestDebugLinesArguments.m_debugMode;  //clientCmd.btIDebugDraw::DBG_DrawWireframe|btIDebugDraw::DBG_DrawAabb;
    i32 startingLineIndex = clientCmd.m_requestDebugLinesArguments.m_startingLineIndex;
    if (startingLineIndex < 0)
    {
        drx3DWarning("startingLineIndex should be non-negative");
        startingLineIndex = 0;
    }

    if (clientCmd.m_requestDebugLinesArguments.m_startingLineIndex == 0)
    {
        m_data->m_remoteDebugDrawer->m_lines2.resize(0);
        //|btIDebugDraw::DBG_DrawAabb|
        //  btIDebugDraw::DBG_DrawConstraints |btIDebugDraw::DBG_DrawConstraintLimits ;
        m_data->m_remoteDebugDrawer->setDebugMode(debugMode);
        IDebugDraw* oldDebugDrawer = m_data->m_dynamicsWorld->getDebugDrawer();
        m_data->m_dynamicsWorld->setDebugDrawer(m_data->m_remoteDebugDrawer);
        m_data->m_dynamicsWorld->debugDrawWorld();
        m_data->m_dynamicsWorld->setDebugDrawer(oldDebugDrawer);
        m_data->m_remoteDebugDrawer->setDebugMode(curFlags);
    }

    //9 floats per line: 3 floats for 'from', 3 floats for 'to' and 3 floats for 'color'
    i32 bytesPerLine = (sizeof(float) * 9);
    i32 maxNumLines = bufferSizeInBytes / bytesPerLine - 1;
    if (startingLineIndex > m_data->m_remoteDebugDrawer->m_lines2.size())
    {
        drx3DWarning("m_startingLineIndex exceeds total number of debug lines");
        startingLineIndex = m_data->m_remoteDebugDrawer->m_lines2.size();
    }

    i32 numLines = d3Min(maxNumLines, m_data->m_remoteDebugDrawer->m_lines2.size() - startingLineIndex);

    if (numLines)
    {
        float* linesFrom = (float*)bufferServerToClient;
        float* linesTo = (float*)(bufferServerToClient + numLines * 3 * sizeof(float));
        float* linesColor = (float*)(bufferServerToClient + 2 * numLines * 3 * sizeof(float));

        for (i32 i = 0; i < numLines; i++)
        {
            linesFrom[i * 3] = m_data->m_remoteDebugDrawer->m_lines2[i + startingLineIndex].m_from.x();
            linesTo[i * 3] = m_data->m_remoteDebugDrawer->m_lines2[i + startingLineIndex].m_to.x();
            linesColor[i * 3] = m_data->m_remoteDebugDrawer->m_lines2[i + startingLineIndex].m_color.x();

            linesFrom[i * 3 + 1] = m_data->m_remoteDebugDrawer->m_lines2[i + startingLineIndex].m_from.y();
            linesTo[i * 3 + 1] = m_data->m_remoteDebugDrawer->m_lines2[i + startingLineIndex].m_to.y();
            linesColor[i * 3 + 1] = m_data->m_remoteDebugDrawer->m_lines2[i + startingLineIndex].m_color.y();

            linesFrom[i * 3 + 2] = m_data->m_remoteDebugDrawer->m_lines2[i + startingLineIndex].m_from.z();
            linesTo[i * 3 + 2] = m_data->m_remoteDebugDrawer->m_lines2[i + startingLineIndex].m_to.z();
            linesColor[i * 3 + 2] = m_data->m_remoteDebugDrawer->m_lines2[i + startingLineIndex].m_color.z();
        }
    }

    serverStatusOut.m_type = CMD_DEBUG_LINES_COMPLETED;
    serverStatusOut.m_numDataStreamBytes = numLines * bytesPerLine;
    serverStatusOut.m_sendDebugLinesArgs.m_numDebugLines = numLines;
    serverStatusOut.m_sendDebugLinesArgs.m_startingLineIndex = startingLineIndex;
    serverStatusOut.m_sendDebugLinesArgs.m_numRemainingDebugLines = m_data->m_remoteDebugDrawer->m_lines2.size() - (startingLineIndex + numLines);

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processSyncBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_SYNC_BODY_INFO");

    b3AlignedObjectArray<i32> usedHandles;
    m_data->m_bodyHandles.getUsedHandles(usedHandles);
    i32 actualNumBodies = 0;
    i32* bodyUids = (i32*)bufferServerToClient;

    for (i32 i = 0; i < usedHandles.size(); i++)
    {
        i32 usedHandle = usedHandles[i];
        InternalBodyData* body = m_data->m_bodyHandles.getHandle(usedHandle);
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        if (body && (body->m_multiBody || body->m_rigidBody || body->m_softBody))
#else
        if (body && (body->m_multiBody || body->m_rigidBody))
#endif
        {
            bodyUids[actualNumBodies++] = usedHandle;
        }
    }
    serverStatusOut.m_sdfLoadedArgs.m_numBodies = actualNumBodies;

    i32 usz = m_data->m_userConstraints.size();
    i32* constraintUid = bodyUids + actualNumBodies;
    serverStatusOut.m_sdfLoadedArgs.m_numUserConstraints = usz;

    for (i32 i = 0; i < usz; i++)
    {
        i32 key = m_data->m_userConstraints.getKeyAtIndex(i).getUid1();
        constraintUid[i] = key;
    }

    serverStatusOut.m_numDataStreamBytes = sizeof(i32) * (actualNumBodies + usz);

    serverStatusOut.m_type = CMD_SYNC_BODY_INFO_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processSyncUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_SYNC_USER_DATA");

    b3AlignedObjectArray<i32> userDataHandles;
    if (clientCmd.m_syncUserDataRequestArgs.m_numRequestedBodies == 0)
    {
        m_data->m_userDataHandles.getUsedHandles(userDataHandles);
    }
    else
    {
        for (i32 i = 0; i < clientCmd.m_syncUserDataRequestArgs.m_numRequestedBodies; ++i)
        {
            i32k bodyUniqueId = clientCmd.m_syncUserDataRequestArgs.m_requestedBodyIds[i];
            InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
            if (!body)
            {
                return hasStatus;
            }
            for (i32 j = 0; j < body->m_userDataHandles.size(); ++j)
            {
                userDataHandles.push_back(body->m_userDataHandles[j]);
            }
        }
    }
    i32 sizeInBytes = sizeof(i32) * userDataHandles.size();
    if (userDataHandles.size())
    {
        memcpy(bufferServerToClient, &userDataHandles[0], sizeInBytes);
    }
    // Only clear the client-side cache when a full sync is requested
    serverStatusOut.m_syncUserDataArgs.m_clearCachedUserDataEntries = clientCmd.m_syncUserDataRequestArgs.m_numRequestedBodies == 0;
    serverStatusOut.m_syncUserDataArgs.m_numUserDataIdentifiers = userDataHandles.size();
    serverStatusOut.m_numDataStreamBytes = sizeInBytes;
    serverStatusOut.m_type = CMD_SYNC_USER_DATA_COMPLETED;

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REQUEST_USER_DATA");
    serverStatusOut.m_type = CMD_REQUEST_USER_DATA_FAILED;

    SharedMemoryUserData* userData = m_data->m_userDataHandles.getHandle(clientCmd.m_userDataRequestArgs.m_userDataId);
    if (!userData)
    {
        return hasStatus;
    }

    Assert(bufferSizeInBytes >= userData->m_bytes.size());
    serverStatusOut.m_userDataResponseArgs.m_userDataId = clientCmd.m_userDataRequestArgs.m_userDataId;
    serverStatusOut.m_userDataResponseArgs.m_bodyUniqueId = userData->m_bodyUniqueId;
    serverStatusOut.m_userDataResponseArgs.m_linkIndex = userData->m_linkIndex;
    serverStatusOut.m_userDataResponseArgs.m_visualShapeIndex = userData->m_visualShapeIndex;
    serverStatusOut.m_userDataResponseArgs.m_valueType = userData->m_type;
    serverStatusOut.m_userDataResponseArgs.m_valueLength = userData->m_bytes.size();
    serverStatusOut.m_type = CMD_REQUEST_USER_DATA_COMPLETED;

    strcpy(serverStatusOut.m_userDataResponseArgs.m_key, userData->m_key.c_str());
    if (userData->m_bytes.size())
    {
        memcpy(bufferServerToClient, &userData->m_bytes[0], userData->m_bytes.size());
    }
    serverStatusOut.m_numDataStreamBytes = userData->m_bytes.size();
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processAddUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_ADD_USER_DATA");
    serverStatusOut.m_type = CMD_ADD_USER_DATA_FAILED;

    const AddUserDataRequestArgs& addUserDataArgs = clientCmd.m_addUserDataRequestArgs;
    if (addUserDataArgs.m_bodyUniqueId < 0 || addUserDataArgs.m_bodyUniqueId >= m_data->m_bodyHandles.getNumHandles())
    {
        return hasStatus;
    }
    i32 userDataHandle = addUserData(
        addUserDataArgs.m_bodyUniqueId, addUserDataArgs.m_linkIndex,
        addUserDataArgs.m_visualShapeIndex, addUserDataArgs.m_key,
        bufferServerToClient, addUserDataArgs.m_valueLength,
        addUserDataArgs.m_valueType);
    if (userDataHandle < 0)
    {
        return hasStatus;
    }

    serverStatusOut.m_type = CMD_ADD_USER_DATA_COMPLETED;
    UserDataResponseArgs& userDataResponseArgs = serverStatusOut.m_userDataResponseArgs;
    userDataResponseArgs.m_userDataId = userDataHandle;
    userDataResponseArgs.m_bodyUniqueId = addUserDataArgs.m_bodyUniqueId;
    userDataResponseArgs.m_linkIndex = addUserDataArgs.m_linkIndex;
    userDataResponseArgs.m_visualShapeIndex = addUserDataArgs.m_visualShapeIndex;
    userDataResponseArgs.m_valueLength = addUserDataArgs.m_valueLength;
    userDataResponseArgs.m_valueType = addUserDataArgs.m_valueType;
    strcpy(userDataResponseArgs.m_key, addUserDataArgs.m_key);

    b3Notification notification;
    notification.m_notificationType = USER_DATA_ADDED;
    b3UserDataNotificationArgs& userDataArgs = notification.m_userDataArgs;
    userDataArgs.m_userDataId = userDataHandle;
    userDataArgs.m_bodyUniqueId = addUserDataArgs.m_bodyUniqueId;
    userDataArgs.m_linkIndex = addUserDataArgs.m_linkIndex;
    userDataArgs.m_visualShapeIndex = addUserDataArgs.m_visualShapeIndex;
    strcpy(userDataArgs.m_key, addUserDataArgs.m_key);
    m_data->m_pluginManager.addNotification(notification);

    // Keep bufferServerToClient as-is.
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCollisionFilterCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    serverStatusOut.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    b3PluginCollisionInterface* collisionInterface = m_data->m_pluginManager.getCollisionInterface();
    if (collisionInterface)
    {
        if (clientCmd.m_updateFlags & D3_COLLISION_FILTER_PAIR)
        {
            collisionInterface->setBroadphaseCollisionFilter(clientCmd.m_collisionFilterArgs.m_bodyUniqueIdA,
                                                             clientCmd.m_collisionFilterArgs.m_bodyUniqueIdB,
                                                             clientCmd.m_collisionFilterArgs.m_linkIndexA,
                                                             clientCmd.m_collisionFilterArgs.m_linkIndexB,
                                                             clientCmd.m_collisionFilterArgs.m_enableCollision);

            AlignedObjectArray<InternalBodyData*> bodies;

            //now also 'refresh' the broadphase collision pairs involved
            if (clientCmd.m_collisionFilterArgs.m_bodyUniqueIdA >= 0)
            {
                bodies.push_back(m_data->m_bodyHandles.getHandle(clientCmd.m_collisionFilterArgs.m_bodyUniqueIdA));
            }
            if (clientCmd.m_collisionFilterArgs.m_bodyUniqueIdB >= 0)
            {
                bodies.push_back(m_data->m_bodyHandles.getHandle(clientCmd.m_collisionFilterArgs.m_bodyUniqueIdB));
            }
            for (i32 i = 0; i < bodies.size(); i++)
            {
                InternalBodyData* body = bodies[i];
                if (body)
                {
                    if (body->m_multiBody)
                    {
                        if (body->m_multiBody->getBaseCollider())
                        {
                            m_data->m_dynamicsWorld->refreshBroadphaseProxy(body->m_multiBody->getBaseCollider());
                        }
                        for (i32 i = 0; i < body->m_multiBody->getNumLinks(); i++)
                        {
                            if (body->m_multiBody->getLinkCollider(i))
                            {
                                m_data->m_dynamicsWorld->refreshBroadphaseProxy(body->m_multiBody->getLinkCollider(i));
                            }
                        }
                    }
                    else
                    {
                        //RigidBody case
                        if (body->m_rigidBody)
                        {
                            m_data->m_dynamicsWorld->refreshBroadphaseProxy(body->m_rigidBody);
                        }
                    }
                }
            }
        }
        if (clientCmd.m_updateFlags & D3_COLLISION_FILTER_GROUP_MASK)
        {
            InternalBodyData* body = m_data->m_bodyHandles.getHandle(clientCmd.m_collisionFilterArgs.m_bodyUniqueIdA);
            if (body)
            {
                CollisionObject2* colObj = 0;
                if (body->m_multiBody)
                {
                    if (clientCmd.m_collisionFilterArgs.m_linkIndexA == -1)
                    {
                        colObj = body->m_multiBody->getBaseCollider();
                    }
                    else
                    {
                        if (clientCmd.m_collisionFilterArgs.m_linkIndexA >= 0 && clientCmd.m_collisionFilterArgs.m_linkIndexA < body->m_multiBody->getNumLinks())
                        {
                            colObj = body->m_multiBody->getLinkCollider(clientCmd.m_collisionFilterArgs.m_linkIndexA);
                        }
                    }
                }
                else
                {
                    if (body->m_rigidBody)
                    {
                        colObj = body->m_rigidBody;
                    }
                }
                if (colObj)
                {
                    colObj->getBroadphaseHandle()->m_collisionFilterGroup = clientCmd.m_collisionFilterArgs.m_collisionFilterGroup;
                    colObj->getBroadphaseHandle()->m_collisionFilterMask = clientCmd.m_collisionFilterArgs.m_collisionFilterMask;
                    m_data->m_dynamicsWorld->refreshBroadphaseProxy(colObj);
                }
            }
        }
    }
    return true;
}

bool PhysicsServerCommandProcessor::processRemoveUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REMOVE_USER_DATA");
    serverStatusOut.m_type = CMD_REMOVE_USER_DATA_FAILED;

    SharedMemoryUserData* userData = m_data->m_userDataHandles.getHandle(clientCmd.m_removeUserDataRequestArgs.m_userDataId);
    if (!userData)
    {
        return hasStatus;
    }

    InternalBodyData* body = m_data->m_bodyHandles.getHandle(userData->m_bodyUniqueId);
    if (!body)
    {
        return hasStatus;
    }
    body->m_userDataHandles.remove(clientCmd.m_removeUserDataRequestArgs.m_userDataId);

    b3Notification notification;
    notification.m_notificationType = USER_DATA_REMOVED;
    b3UserDataNotificationArgs& userDataArgs = notification.m_userDataArgs;
    userDataArgs.m_userDataId = clientCmd.m_removeUserDataRequestArgs.m_userDataId;
    userDataArgs.m_bodyUniqueId = userData->m_bodyUniqueId;
    userDataArgs.m_linkIndex = userData->m_linkIndex;
    userDataArgs.m_visualShapeIndex = userData->m_visualShapeIndex;
    strcpy(userDataArgs.m_key, userData->m_key.c_str());

    m_data->m_userDataHandleLookup.remove(SharedMemoryUserDataHashKey(userData));
    m_data->m_userDataHandles.freeHandle(clientCmd.m_removeUserDataRequestArgs.m_userDataId);

    serverStatusOut.m_removeUserDataResponseArgs = clientCmd.m_removeUserDataRequestArgs;
    serverStatusOut.m_type = CMD_REMOVE_USER_DATA_COMPLETED;

    m_data->m_pluginManager.addNotification(notification);
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processSendDesiredStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_SEND_DESIRED_STATE");
    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("Processed CMD_SEND_DESIRED_STATE");
    }

    i32 bodyUniqueId = clientCmd.m_sendDesiredStateCommandArgument.m_bodyUniqueId;
    InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);

    if (body && body->m_multiBody)
    {
        MultiBody* mb = body->m_multiBody;
        Assert(mb);

        switch (clientCmd.m_sendDesiredStateCommandArgument.m_controlMode)
        {
            case CONTROL_MODE_PD:
            {
                if (m_data->m_verboseOutput)
                {
                    drx3DPrintf("Using CONTROL_MODE_PD");
                }

                b3PluginArguments args;
                args.m_ints[1] = bodyUniqueId;
                //find the joint motors and apply the desired velocity and maximum force/torque
                {
                    args.m_numInts = 0;
                    args.m_numFloats = 0;
                    //syncBodies is expensive/slow, use it only once
                    m_data->m_pluginManager.executePluginCommand(m_data->m_pdControlPlugin, &args);

                    i32 velIndex = 6;  //skip the 3 linear + 3 angular degree of freedom velocity entries of the base
                    i32 posIndex = 7;  //skip 3 positional and 4 orientation (quaternion) positional degrees of freedom of the base
                    for (i32 link = 0; link < mb->getNumLinks(); link++)
                    {
                        if (supportsJointMotor(mb, link))
                        {
                            bool hasDesiredPosOrVel = false;
                            Scalar desiredVelocity = 0.f;
                            if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_QDOT) != 0)
                            {
                                hasDesiredPosOrVel = true;
                                desiredVelocity = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot[velIndex];
                                args.m_floats[2] = 0.1;  // kd
                            }
                            Scalar desiredPosition = 0.f;
                            if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[posIndex] & SIM_DESIRED_STATE_HAS_Q) != 0)
                            {
                                hasDesiredPosOrVel = true;
                                desiredPosition = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQ[posIndex];
                                args.m_floats[3] = 0.1;  // kp
                            }

                            if (hasDesiredPosOrVel)
                            {
                                if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_KP) != 0)
                                {
                                    args.m_floats[3] = clientCmd.m_sendDesiredStateCommandArgument.m_Kp[velIndex];
                                }
                                if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_KD) != 0)
                                {
                                    args.m_floats[2] = clientCmd.m_sendDesiredStateCommandArgument.m_Kd[velIndex];
                                }

                                args.m_floats[1] = desiredVelocity;
                                //clamp position
                                if (mb->getLink(link).m_jointLowerLimit <= mb->getLink(link).m_jointUpperLimit)
                                {
                                    Clamp(desiredPosition, mb->getLink(link).m_jointLowerLimit, mb->getLink(link).m_jointUpperLimit);
                                }
                                args.m_floats[0] = desiredPosition;

                                Scalar maxImp = 1000000.f;
                                if ((clientCmd.m_updateFlags & SIM_DESIRED_STATE_HAS_MAX_FORCE) != 0)
                                    maxImp = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[velIndex];
                                args.m_floats[4] = maxImp;

                                args.m_ints[2] = link;
                                args.m_numInts = 3;
                                args.m_numFloats = 5;

                                args.m_ints[0] = eSetPDControl;
                                if (args.m_floats[4] < D3_EPSILON)
                                {
                                    args.m_ints[0] = eRemovePDControl;
                                }
                                m_data->m_pluginManager.executePluginCommand(m_data->m_pdControlPlugin, &args);
                            }
                        }
                        velIndex += mb->getLink(link).m_dofCount;
                        posIndex += mb->getLink(link).m_posVarCount;
                    }
                }
                break;
            }
            case CONTROL_MODE_TORQUE:
            {
                if (m_data->m_verboseOutput)
                {
                    drx3DPrintf("Using CONTROL_MODE_TORQUE");
                }
                //  mb->clearForcesAndTorques();
                i32 torqueIndex = 6;
                if ((clientCmd.m_updateFlags & SIM_DESIRED_STATE_HAS_MAX_FORCE) != 0)
                {
                    for (i32 link = 0; link < mb->getNumLinks(); link++)
                    {
                        for (i32 dof = 0; dof < mb->getLink(link).m_dofCount; dof++)
                        {
                            double torque = 0.f;
                            if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[torqueIndex] & SIM_DESIRED_STATE_HAS_MAX_FORCE) != 0)
                            {
                                torque = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[torqueIndex];
                                mb->addJointTorqueMultiDof(link, dof, torque);
                            }
                            torqueIndex++;
                        }
                    }
                }
                break;
            }
            case CONTROL_MODE_VELOCITY:
            {
                if (m_data->m_verboseOutput)
                {
                    drx3DPrintf("Using CONTROL_MODE_VELOCITY");
                }

                i32 numMotors = 0;
                //find the joint motors and apply the desired velocity and maximum force/torque
                {
                    i32 dofIndex = 6;  //skip the 3 linear + 3 angular degree of freedom entries of the base
                    for (i32 link = 0; link < mb->getNumLinks(); link++)
                    {
                        if (supportsJointMotor(mb, link))
                        {
                            MultiBodyJointMotor* motor = (MultiBodyJointMotor*)mb->getLink(link).m_userPtr;

                            if (motor)
                            {
                                Scalar desiredVelocity = 0.f;
                                bool hasDesiredVelocity = false;

                                if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_QDOT) != 0)
                                {
                                    desiredVelocity = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot[dofIndex];
                                    Scalar kd = 0.1f;
                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_KD) != 0)
                                    {
                                        kd = clientCmd.m_sendDesiredStateCommandArgument.m_Kd[dofIndex];
                                    }

                                    motor->setVelocityTarget(desiredVelocity, kd);

                                    Scalar kp = 0.f;
                                    motor->setPositionTarget(0, kp);
                                    hasDesiredVelocity = true;
                                }
                                if (hasDesiredVelocity)
                                {
                                    //disable velocity clamp in velocity mode
                                    motor->setRhsClamp(SIMD_INFINITY);

                                    Scalar maxImp = 1000000.f * m_data->m_physicsDeltaTime;
                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] & SIM_DESIRED_STATE_HAS_MAX_FORCE) != 0)
                                    {
                                        maxImp = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[dofIndex] * m_data->m_physicsDeltaTime;
                                    }
                                    motor->setMaxAppliedImpulse(maxImp);
                                }
                                numMotors++;
                            }
                        }
                        dofIndex += mb->getLink(link).m_dofCount;
                    }
                }
                break;
            }

            case CONTROL_MODE_POSITION_VELOCITY_PD:
            {
                if (m_data->m_verboseOutput)
                {
                    drx3DPrintf("Using CONTROL_MODE_POSITION_VELOCITY_PD");
                }
                //compute the force base on PD control

                i32 numMotors = 0;
                //find the joint motors and apply the desired velocity and maximum force/torque
                {
                    i32 velIndex = 6;  //skip the 3 linear + 3 angular degree of freedom velocity entries of the base
                    i32 posIndex = 7;  //skip 3 positional and 4 orientation (quaternion) positional degrees of freedom of the base
                    for (i32 link = 0; link < mb->getNumLinks(); link++)
                    {
                        if (supportsJointMotor(mb, link))
                        {
                            MultiBodyJointMotor* motor = (MultiBodyJointMotor*)mb->getLink(link).m_userPtr;

                            if (motor)
                            {
                                if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_RHS_CLAMP) != 0)
                                {
                                    motor->setRhsClamp(clientCmd.m_sendDesiredStateCommandArgument.m_rhsClamp[velIndex]);
                                }

                                bool hasDesiredPosOrVel = false;
                                Scalar kp = 0.f;
                                Scalar kd = 0.f;
                                Scalar desiredVelocity = 0.f;
                                if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_QDOT) != 0)
                                {
                                    hasDesiredPosOrVel = true;
                                    desiredVelocity = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot[velIndex];
                                    kd = 0.1;
                                }
                                Scalar desiredPosition = 0.f;
                                if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[posIndex] & SIM_DESIRED_STATE_HAS_Q) != 0)
                                {
                                    hasDesiredPosOrVel = true;
                                    desiredPosition = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQ[posIndex];
                                    kp = 0.1;
                                }

                                if (hasDesiredPosOrVel)
                                {
                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_KP) != 0)
                                    {
                                        kp = clientCmd.m_sendDesiredStateCommandArgument.m_Kp[velIndex];
                                    }

                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_KD) != 0)
                                    {
                                        kd = clientCmd.m_sendDesiredStateCommandArgument.m_Kd[velIndex];
                                    }

                                    motor->setVelocityTarget(desiredVelocity, kd);
                                    //todo: instead of clamping, combine the motor and limit
                                    //and combine handling of limit force and motor force.

                                    //clamp position
                                    if (mb->getLink(link).m_jointLowerLimit <= mb->getLink(link).m_jointUpperLimit)
                                    {
                                        Clamp(desiredPosition, mb->getLink(link).m_jointLowerLimit, mb->getLink(link).m_jointUpperLimit);
                                    }
                                    motor->setPositionTarget(desiredPosition, kp);

                                    Scalar maxImp = 1000000.f * m_data->m_physicsDeltaTime;

                                    if ((clientCmd.m_updateFlags & SIM_DESIRED_STATE_HAS_MAX_FORCE) != 0)
                                        maxImp = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[velIndex] * m_data->m_physicsDeltaTime;

                                    motor->setMaxAppliedImpulse(maxImp);
                                }
                                numMotors++;
                            }
                        }

                        if (mb->getLink(link).m_jointType == MultibodyLink::eSpherical)
                        {
                            MultiBodySphericalJointMotor* motor = (MultiBodySphericalJointMotor*)mb->getLink(link).m_userPtr;
                            if (motor)
                            {
                                if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_RHS_CLAMP) != 0)
                                {
                                    motor->setRhsClamp(clientCmd.m_sendDesiredStateCommandArgument.m_rhsClamp[velIndex]);
                                }
                                bool hasDesiredPosOrVel = false;
                                Vec3 kp(0, 0, 0);
                                Vec3 kd(0, 0, 0);
                                Vec3 desiredVelocity(0, 0, 0);
                                if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_QDOT) != 0)
                                {
                                    hasDesiredPosOrVel = true;
                                    desiredVelocity.setVal(
                                        clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot[velIndex + 0],
                                        clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot[velIndex + 1],
                                        clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot[velIndex + 2]);
                                    kd.setVal(0.1, 0.1, 0.1);
                                }
                                Quat desiredPosition(0, 0, 0, 1);
                                if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[posIndex] & SIM_DESIRED_STATE_HAS_Q) != 0)
                                {
                                    hasDesiredPosOrVel = true;
                                    desiredPosition.setVal(
                                        clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQ[posIndex + 0],
                                        clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQ[posIndex + 1],
                                        clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQ[posIndex + 2],
                                        clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQ[posIndex + 3]);
                                    kp.setVal(0.1, 0.1, 0.1);
                                }

                                if (hasDesiredPosOrVel)
                                {
                                    bool useMultiDof = true;

                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_KP) != 0)
                                    {
                                        kp.setVal(
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kp[velIndex + 0],
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kp[velIndex + 0],
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kp[velIndex + 0]);
                                    }
                                    if (((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+0] & SIM_DESIRED_STATE_HAS_KP) != 0) &&
                                        ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+1] & SIM_DESIRED_STATE_HAS_KP) != 0) &&
                                        ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+2] & SIM_DESIRED_STATE_HAS_KP) != 0)
                                        )
                                    {
                                        kp.setVal(
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kp[velIndex + 0],
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kp[velIndex + 1],
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kp[velIndex + 2]);
                                    } else
                                    {
                                        useMultiDof = false;
                                    }

                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_KD) != 0)
                                    {
                                        kd.setVal(
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kd[velIndex + 0],
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kd[velIndex + 0],
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kd[velIndex + 0]);
                                    }

                                    if (((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+0] & SIM_DESIRED_STATE_HAS_KD) != 0) &&
                                        ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+1] & SIM_DESIRED_STATE_HAS_KD) != 0) &&
                                        ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+2] & SIM_DESIRED_STATE_HAS_KD) != 0))
                                    {
                                        kd.setVal(
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kd[velIndex + 0],
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kd[velIndex + 1],
                                            clientCmd.m_sendDesiredStateCommandArgument.m_Kd[velIndex + 2]);
                                    } else
                                    {
                                        useMultiDof = false;
                                    }

                                    Vec3 maxImp(
                                        1000000.f * m_data->m_physicsDeltaTime,
                                        1000000.f * m_data->m_physicsDeltaTime,
                                        1000000.f * m_data->m_physicsDeltaTime);

                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_MAX_FORCE)!=0)
                                    {
                                        maxImp.setVal(
                                            clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[velIndex + 0] * m_data->m_physicsDeltaTime,
                                            clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[velIndex + 0] * m_data->m_physicsDeltaTime,
                                            clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[velIndex + 0] * m_data->m_physicsDeltaTime);
                                    }

                                    if (((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+0] & SIM_DESIRED_STATE_HAS_MAX_FORCE)!=0) &&
                                        ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+1] & SIM_DESIRED_STATE_HAS_MAX_FORCE)!=0) &&
                                        ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+2] & SIM_DESIRED_STATE_HAS_MAX_FORCE)!=0))
                                    {
                                        maxImp.setVal(
                                            clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[velIndex + 0] * m_data->m_physicsDeltaTime,
                                            clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[velIndex + 1] * m_data->m_physicsDeltaTime,
                                            clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[velIndex + 2] * m_data->m_physicsDeltaTime);
                                    } else
                                    {
                                        useMultiDof = false;
                                    }

                                    if (useMultiDof)
                                    {
                                        motor->setVelocityTargetMultiDof(desiredVelocity, kd);
                                        motor->setPositionTargetMultiDof(desiredPosition, kp);
                                        motor->setMaxAppliedImpulseMultiDof(maxImp);
                                    } else
                                    {
                                        motor->setVelocityTarget(desiredVelocity, kd[0]);
                                        //todo: instead of clamping, combine the motor and limit
                                        //and combine handling of limit force and motor force.

                                        //clamp position
                                        //if (mb->getLink(link).m_jointLowerLimit <= mb->getLink(link).m_jointUpperLimit)
                                        //{
                                        //  btClamp(desiredPosition, mb->getLink(link).m_jointLowerLimit, mb->getLink(link).m_jointUpperLimit);
                                        //}
                                        motor->setPositionTarget(desiredPosition, kp[0]);
                                        motor->setMaxAppliedImpulse(maxImp[0]);
                                    }

                                    Vec3 damping(1.f, 1.f, 1.f);
                                    if ((clientCmd.m_updateFlags & SIM_DESIRED_STATE_HAS_DAMPING) != 0) {
                                        if (
                        (clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+0] & SIM_DESIRED_STATE_HAS_DAMPING)&&
                        (clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+1] & SIM_DESIRED_STATE_HAS_DAMPING)&&
                        (clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex+2] & SIM_DESIRED_STATE_HAS_DAMPING)
                                            )
                                        {
                                            damping.setVal(
                                                clientCmd.m_sendDesiredStateCommandArgument.m_damping[velIndex + 0],
                                                clientCmd.m_sendDesiredStateCommandArgument.m_damping[velIndex + 1],
                                                clientCmd.m_sendDesiredStateCommandArgument.m_damping[velIndex + 2]);
                                        } else
                                        {
                                            damping.setVal(
                                                clientCmd.m_sendDesiredStateCommandArgument.m_damping[velIndex + 0],
                                                clientCmd.m_sendDesiredStateCommandArgument.m_damping[velIndex + 0],
                                                clientCmd.m_sendDesiredStateCommandArgument.m_damping[velIndex + 0]);
                                        }
                                    }
                                    motor->setDamping(damping);
                                }
                                numMotors++;
                            }
                        }
                        velIndex += mb->getLink(link).m_dofCount;
                        posIndex += mb->getLink(link).m_posVarCount;
                    }
                }

                break;
            }
#ifdef STATIC_LINK_SPD_PLUGIN
            case CONTROL_MODE_STABLE_PD:
            {
                i32 posVal = body->m_multiBody->getNumPosVars();
                AlignedObjectArray<double> zeroVel;
                i32 dof = 7 + posVal;
                zeroVel.resize(dof);
                //clientCmd.m_sendDesiredStateCommandArgument.
                //current positions and velocities
                AlignedObjectArray<double> jointPositionsQ;
                AlignedObjectArray<double> jointVelocitiesQdot;
                Transform2 baseTr = body->m_multiBody->getBaseWorldTransform();
#if 1
                jointPositionsQ.push_back(baseTr.getOrigin()[0]);
                jointPositionsQ.push_back(baseTr.getOrigin()[1]);
                jointPositionsQ.push_back(baseTr.getOrigin()[2]);
                jointPositionsQ.push_back(baseTr.getRotation()[0]);
                jointPositionsQ.push_back(baseTr.getRotation()[1]);
                jointPositionsQ.push_back(baseTr.getRotation()[2]);
                jointPositionsQ.push_back(baseTr.getRotation()[3]);
                jointVelocitiesQdot.push_back(body->m_multiBody->getBaseVel()[0]);
                jointVelocitiesQdot.push_back(body->m_multiBody->getBaseVel()[1]);
                jointVelocitiesQdot.push_back(body->m_multiBody->getBaseVel()[2]);
                jointVelocitiesQdot.push_back(body->m_multiBody->getBaseOmega()[0]);
                jointVelocitiesQdot.push_back(body->m_multiBody->getBaseOmega()[1]);
                jointVelocitiesQdot.push_back(body->m_multiBody->getBaseOmega()[2]);
                jointVelocitiesQdot.push_back(0);
#else
                for (i32 i = 0; i < 7; i++)
                {
                    jointPositionsQ.push_back(0);
                    jointVelocitiesQdot.push_back(0);
                }
                jointPositionsQ[6] = 1;
#endif
                for (i32 i = 0; i < body->m_multiBody->getNumLinks(); i++)
                {
                    switch (body->m_multiBody->getLink(i).m_jointType)
                    {
                        case MultibodyLink::eSpherical:
                        {
                            Scalar* jointPos = body->m_multiBody->getJointPosMultiDof(i);
                            jointPositionsQ.push_back(jointPos[0]);
                            jointPositionsQ.push_back(jointPos[1]);
                            jointPositionsQ.push_back(jointPos[2]);
                            jointPositionsQ.push_back(jointPos[3]);
                            Scalar* jointVel = body->m_multiBody->getJointVelMultiDof(i);
                            jointVelocitiesQdot.push_back(jointVel[0]);
                            jointVelocitiesQdot.push_back(jointVel[1]);
                            jointVelocitiesQdot.push_back(jointVel[2]);
                            jointVelocitiesQdot.push_back(0);
                            break;
                        }
                        case MultibodyLink::ePrismatic:
                        case MultibodyLink::eRevolute:
                        {
                            Scalar* jointPos = body->m_multiBody->getJointPosMultiDof(i);
                            jointPositionsQ.push_back(jointPos[0]);
                            Scalar* jointVel = body->m_multiBody->getJointVelMultiDof(i);
                            jointVelocitiesQdot.push_back(jointVel[0]);
                            break;
                        }
                        case MultibodyLink::eFixed:
                        {
                            //skip
                            break;
                        }
                        default:
                        {
                            drx3DError("Unsupported joint type");
                            Assert(0);
                        }
                    }
                }

                cRBDModel* rbdModel = 0;

                {
                    DRX3D_PROFILE("findOrCreateRBDModel");
                    rbdModel = m_data->findOrCreateRBDModel(body->m_multiBody, &jointPositionsQ[0], &jointVelocitiesQdot[0]);
                }
                if (rbdModel)
                {
                    i32 num_dof = jointPositionsQ.size();
                    const Eigen::VectorXd& pose = rbdModel->GetPose();
                    const Eigen::VectorXd& vel = rbdModel->GetVel();

                    Eigen::Map<Eigen::VectorXd> mKp((double*)clientCmd.m_sendDesiredStateCommandArgument.m_Kp, num_dof);
                    Eigen::Map<Eigen::VectorXd> mKd((double*)clientCmd.m_sendDesiredStateCommandArgument.m_Kd, num_dof);
                    Eigen::Map<Eigen::VectorXd> maxForce((double*)clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque, num_dof);

                    Eigen::DiagonalMatrix<double, Eigen::Dynamic> Kp_mat = mKp.asDiagonal();
                    Eigen::DiagonalMatrix<double, Eigen::Dynamic> Kd_mat = mKd.asDiagonal();

                    Eigen::MatrixXd M = rbdModel->GetMassMat();
                    //rbdModel->UpdateBiasForce();
                    const Eigen::VectorXd& C = rbdModel->GetBiasForce();
                    M.diagonal() += m_data->m_physicsDeltaTime * mKd;

                    Eigen::VectorXd pose_inc;

                    const Eigen::MatrixXd& joint_mat = rbdModel->GetJointMat();
                    {
                        DRX3D_PROFILE("cKinTree::VelToPoseDiff");
                        cKinTree::VelToPoseDiff(joint_mat, rbdModel->GetPose(), rbdModel->GetVel(), pose_inc);
                    }

                    //tar_pose needs to be reshuffled?
                    Eigen::VectorXd tar_pose, tar_vel;

                    {
                        DRX3D_PROFILE("convertPose");
                        PhysicsServerCommandProcessorInternalData::convertPose(body->m_multiBody,
                                                                               (double*)clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQ,
                                                                               (double*)clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot,
                                                                               tar_pose, tar_vel);
                    }

                    pose_inc = pose + m_data->m_physicsDeltaTime * pose_inc;

                    {
                        DRX3D_PROFILE("cKinTree::PostProcessPose");
                        cKinTree::PostProcessPose(joint_mat, pose_inc);
                    }

                    Eigen::VectorXd pose_err;
                    {
                        DRX3D_PROFILE("cKinTree::CalcVel");
                        cKinTree::CalcVel(joint_mat, pose_inc, tar_pose, 1, pose_err);
                    }
                    for (i32 i = 0; i < 7; i++)
                    {
                        pose_err[i] = 0;
                    }

                    Eigen::VectorXd vel_err = tar_vel - vel;
                    Eigen::VectorXd acc;

                    {
                        DRX3D_PROFILE("acc");
                        acc = Kp_mat * pose_err + Kd_mat * vel_err - C;
                    }

                    {
                        DRX3D_PROFILE("M.ldlt().solve");
                        acc = M.ldlt().solve(acc);
                    }
                    Eigen::VectorXd out_tau = Eigen::VectorXd::Zero(num_dof);
                    out_tau += Kp_mat * pose_err + Kd_mat * (vel_err - m_data->m_physicsDeltaTime * acc);
                    //clamp the forces
                    out_tau = out_tau.cwiseMax(-maxForce);
                    out_tau = out_tau.cwiseMin(maxForce);
                    //apply the forces
                    i32 torqueIndex = 7;
                    for (i32 link = 0; link < mb->getNumLinks(); link++)
                    {
                        i32 dofCount = mb->getLink(link).m_dofCount;
                        i32 dofOffset = mb->getLink(link).m_dofOffset;
                        if (dofCount == 3)
                        {
                            for (i32 dof = 0; dof < 3; dof++)
                            {
                                double torque = out_tau[torqueIndex + dof];
                                mb->addJointTorqueMultiDof(link, dof, torque);
                            }
                            torqueIndex += 4;
                        }
                        if (dofCount == 1)
                        {
                            double torque = out_tau[torqueIndex];
                            mb->addJointTorqueMultiDof(link, 0, torque);
                            torqueIndex++;
                        }
                    }
                }

                break;
            }
#endif
            default:
            {
                drx3DWarning("m_controlMode not implemented yet");
                break;
            }
        }
    }
    else
    {
        //support for non-btMultiBody, such as RigidBody

        if (body && body->m_rigidBody)
        {
            RigidBody* rb = body->m_rigidBody;
            Assert(rb);

            //switch (clientCmd.m_sendDesiredStateCommandArgument.m_controlMode)
            {
                //case CONTROL_MODE_TORQUE:
                {
                    if (m_data->m_verboseOutput)
                    {
                        drx3DPrintf("Using CONTROL_MODE_TORQUE");
                    }
                    //  mb->clearForcesAndTorques();
                    ///see addJointInfoFromConstraint
                    i32 velIndex = 6;
                    i32 posIndex = 7;
                    //if ((clientCmd.m_updateFlags&SIM_DESIRED_STATE_HAS_MAX_FORCE)!=0)
                    {
                        for (i32 link = 0; link < body->m_rigidBodyJoints.size(); link++)
                        {
                            Generic6DofSpring2Constraint* con = body->m_rigidBodyJoints[link];

                            Vec3 linearLowerLimit;
                            Vec3 linearUpperLimit;
                            Vec3 angularLowerLimit;
                            Vec3 angularUpperLimit;

                            //for (i32 dof=0;dof<mb->getLink(link).m_dofCount;dof++)
                            {
                                {
                                    i32 torqueIndex = velIndex;
                                    double torque = 100;
                                    bool hasDesiredTorque = false;
                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_MAX_FORCE) != 0)
                                    {
                                        torque = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[velIndex];
                                        hasDesiredTorque = true;
                                    }

                                    bool hasDesiredPosOrVel = false;
                                    Scalar qdotTarget = 0.f;
                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[velIndex] & SIM_DESIRED_STATE_HAS_QDOT) != 0)
                                    {
                                        hasDesiredPosOrVel = true;
                                        qdotTarget = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQdot[velIndex];
                                    }
                                    Scalar qTarget = 0.f;
                                    if ((clientCmd.m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[posIndex] & SIM_DESIRED_STATE_HAS_Q) != 0)
                                    {
                                        hasDesiredPosOrVel = true;
                                        qTarget = clientCmd.m_sendDesiredStateCommandArgument.m_desiredStateQ[posIndex];
                                    }

                                    con->getLinearLowerLimit(linearLowerLimit);
                                    con->getLinearUpperLimit(linearUpperLimit);
                                    con->getAngularLowerLimit(angularLowerLimit);
                                    con->getAngularUpperLimit(angularUpperLimit);

                                    if (linearLowerLimit.isZero() && linearUpperLimit.isZero() && angularLowerLimit.isZero() && angularUpperLimit.isZero())
                                    {
                                        //fixed, don't do anything
                                    }
                                    else
                                    {
                                        con->calculateTransforms();

                                        if (linearLowerLimit.isZero() && linearUpperLimit.isZero())
                                        {
                                            //eRevoluteType;
                                            Vec3 limitRange = angularLowerLimit.absolute() + angularUpperLimit.absolute();
                                            i32 limitAxis = limitRange.maxAxis();
                                            const Transform2& transA = con->getCalculatedTransform2A();
                                            const Transform2& transB = con->getCalculatedTransform2B();
                                            Vec3 axisA = transA.getBasis().getColumn(limitAxis);
                                            Vec3 axisB = transB.getBasis().getColumn(limitAxis);

                                            switch (clientCmd.m_sendDesiredStateCommandArgument.m_controlMode)
                                            {
                                                case CONTROL_MODE_TORQUE:
                                                {
                                                    if (hasDesiredTorque)
                                                    {
                                                        con->getRigidBodyA().applyTorque(torque * axisA);
                                                        con->getRigidBodyB().applyTorque(-torque * axisB);
                                                    }
                                                    break;
                                                }
                                                case CONTROL_MODE_VELOCITY:
                                                {
                                                    if (hasDesiredPosOrVel)
                                                    {
                                                        con->enableMotor(3 + limitAxis, true);
                                                        con->setTargetVelocity(3 + limitAxis, qdotTarget);
                                                        con->setMaxMotorForce(3 + limitAxis, torque);
                                                    }
                                                    break;
                                                }
                                                case CONTROL_MODE_POSITION_VELOCITY_PD:
                                                {
                                                    if (hasDesiredPosOrVel)
                                                    {
                                                        con->setServo(3 + limitAxis, true);
                                                        con->setServoTarget(3 + limitAxis, -qTarget);
                                                        //next one is the maximum velocity to reach target position.
                                                        //the maximum velocity is limited by maxMotorForce
                                                        con->setTargetVelocity(3 + limitAxis, 100);
                                                        con->setMaxMotorForce(3 + limitAxis, torque);
                                                        con->enableMotor(3 + limitAxis, true);
                                                    }
                                                    break;
                                                }
                                                default:
                                                {
                                                }
                                            };
                                        }
                                        else
                                        {
                                            //ePrismaticType; @todo
                                            Vec3 limitRange = linearLowerLimit.absolute() + linearUpperLimit.absolute();
                                            i32 limitAxis = limitRange.maxAxis();

                                            const Transform2& transA = con->getCalculatedTransform2A();
                                            const Transform2& transB = con->getCalculatedTransform2B();
                                            Vec3 axisA = transA.getBasis().getColumn(limitAxis);
                                            Vec3 axisB = transB.getBasis().getColumn(limitAxis);

                                            switch (clientCmd.m_sendDesiredStateCommandArgument.m_controlMode)
                                            {
                                                case CONTROL_MODE_TORQUE:
                                                {
                                                    con->getRigidBodyA().applyForce(-torque * axisA, Vec3(0, 0, 0));
                                                    con->getRigidBodyB().applyForce(torque * axisB, Vec3(0, 0, 0));
                                                    break;
                                                }
                                                case CONTROL_MODE_VELOCITY:
                                                {
                                                    con->enableMotor(limitAxis, true);
                                                    con->setTargetVelocity(limitAxis, -qdotTarget);
                                                    con->setMaxMotorForce(limitAxis, torque);
                                                    break;
                                                }
                                                case CONTROL_MODE_POSITION_VELOCITY_PD:
                                                {
                                                    con->setServo(limitAxis, true);
                                                    con->setServoTarget(limitAxis, qTarget);
                                                    //next one is the maximum velocity to reach target position.
                                                    //the maximum velocity is limited by maxMotorForce
                                                    con->setTargetVelocity(limitAxis, 100);
                                                    con->setMaxMotorForce(limitAxis, torque);
                                                    con->enableMotor(limitAxis, true);
                                                    break;
                                                }
                                                default:
                                                {
                                                }
                                            };
                                        }
                                    }
                                }  //fi
                                ///see addJointInfoFromConstraint
                                velIndex++;  //info.m_uIndex
                                posIndex++;  //info.m_qIndex
                            }
                        }
                    }  //fi
                    //break;
                }
            }
        }  //if (body && body->m_rigidBody)
    }

    serverStatusOut.m_type = CMD_DESIRED_STATE_RECEIVED_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestActualStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    serverStatusOut.m_type = CMD_ACTUAL_STATE_UPDATE_FAILED;

    DRX3D_PROFILE("CMD_REQUEST_ACTUAL_STATE");
    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("Sending the actual state (Q,U)");
    }
    i32 bodyUniqueId = clientCmd.m_requestActualStateInformationCommandArgument.m_bodyUniqueId;
    InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);

    //we store the state details in the shared memory block, to reduce status size
    SendActualStateSharedMemoryStorage* stateDetails = (SendActualStateSharedMemoryStorage*)bufferServerToClient;

    //make sure the storage fits, otherwise
    Assert(sizeof(SendActualStateSharedMemoryStorage) < bufferSizeInBytes);
    if (sizeof(SendActualStateSharedMemoryStorage) > bufferSizeInBytes)
    {
        //this forces an error report
        body = 0;
    }
    if (body && body->m_multiBody)
    {
        MultiBody* mb = body->m_multiBody;
        SharedMemoryStatus& serverCmd = serverStatusOut;

        serverStatusOut.m_type = CMD_ACTUAL_STATE_UPDATE_COMPLETED;

        serverCmd.m_sendActualStateArgs.m_bodyUniqueId = bodyUniqueId;
        serverCmd.m_sendActualStateArgs.m_numLinks = body->m_multiBody->getNumLinks();
        serverCmd.m_numDataStreamBytes = sizeof(SendActualStateSharedMemoryStorage);
        serverCmd.m_sendActualStateArgs.m_stateDetails = 0;
        i32 totalDegreeOfFreedomQ = 0;
        i32 totalDegreeOfFreedomU = 0;

        if (mb->getNumLinks() >= MAX_DEGREE_OF_FREEDOM)
        {
            serverStatusOut.m_type = CMD_ACTUAL_STATE_UPDATE_FAILED;
            hasStatus = true;
            return hasStatus;
        }

        //always add the base, even for static (non-moving objects)
        //so that we can easily move the 'fixed' base when needed
        //do we don't use this conditional "if (!mb->hasFixedBase())"
        {
            Transform2 tr;
            tr.setOrigin(mb->getBasePos());
            tr.setRotation(mb->getWorldToBaseRot().inverse());

            serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[0] =
                body->m_rootLocalInertialFrame.getOrigin()[0];
            serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[1] =
                body->m_rootLocalInertialFrame.getOrigin()[1];
            serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[2] =
                body->m_rootLocalInertialFrame.getOrigin()[2];

            serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[3] =
                body->m_rootLocalInertialFrame.getRotation()[0];
            serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[4] =
                body->m_rootLocalInertialFrame.getRotation()[1];
            serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[5] =
                body->m_rootLocalInertialFrame.getRotation()[2];
            serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[6] =
                body->m_rootLocalInertialFrame.getRotation()[3];

            //base position in world space, cartesian
            stateDetails->m_actualStateQ[0] = tr.getOrigin()[0];
            stateDetails->m_actualStateQ[1] = tr.getOrigin()[1];
            stateDetails->m_actualStateQ[2] = tr.getOrigin()[2];

            //base orientation, quaternion x,y,z,w, in world space, cartesian
            stateDetails->m_actualStateQ[3] = tr.getRotation()[0];
            stateDetails->m_actualStateQ[4] = tr.getRotation()[1];
            stateDetails->m_actualStateQ[5] = tr.getRotation()[2];
            stateDetails->m_actualStateQ[6] = tr.getRotation()[3];
            totalDegreeOfFreedomQ += 7;  //pos + quaternion

            //base linear velocity (in world space, cartesian)
            stateDetails->m_actualStateQdot[0] = mb->getBaseVel()[0];
            stateDetails->m_actualStateQdot[1] = mb->getBaseVel()[1];
            stateDetails->m_actualStateQdot[2] = mb->getBaseVel()[2];

            //base angular velocity (in world space, cartesian)
            stateDetails->m_actualStateQdot[3] = mb->getBaseOmega()[0];
            stateDetails->m_actualStateQdot[4] = mb->getBaseOmega()[1];
            stateDetails->m_actualStateQdot[5] = mb->getBaseOmega()[2];
            totalDegreeOfFreedomU += 6;  //3 linear and 3 angular DOF
        }

        AlignedObjectArray<Vec3> omega;
        AlignedObjectArray<Vec3> linVel;

        bool computeForwardKinematics = ((clientCmd.m_updateFlags & ACTUAL_STATE_COMPUTE_FORWARD_KINEMATICS) != 0);
        if (computeForwardKinematics)
        {
            D3_PROFILE("compForwardKinematics");
            AlignedObjectArray<Quat> world_to_local;
            AlignedObjectArray<Vec3> local_origin;
            world_to_local.resize(mb->getNumLinks() + 1);
            local_origin.resize(mb->getNumLinks() + 1);
            mb->forwardKinematics(world_to_local, local_origin);
        }

        bool computeLinkVelocities = ((clientCmd.m_updateFlags & ACTUAL_STATE_COMPUTE_LINKVELOCITY) != 0);
        if (computeLinkVelocities)
        {
            omega.resize(mb->getNumLinks() + 1);
            linVel.resize(mb->getNumLinks() + 1);
            {
                D3_PROFILE("compTreeLinkVelocities");
                mb->compTreeLinkVelocities(&omega[0], &linVel[0]);
            }
        }
        for (i32 l = 0; l < mb->getNumLinks(); l++)
        {
            for (i32 d = 0; d < mb->getLink(l).m_posVarCount; d++)
            {
                stateDetails->m_actualStateQ[totalDegreeOfFreedomQ++] = mb->getJointPosMultiDof(l)[d];
            }
            for (i32 d = 0; d < mb->getLink(l).m_dofCount; d++)
            {
                stateDetails->m_jointMotorForceMultiDof[totalDegreeOfFreedomU] = 0;

                if (mb->getLink(l).m_jointType == MultibodyLink::eSpherical)
                {
                    MultiBodySphericalJointMotor* motor = (MultiBodySphericalJointMotor*)mb->getLink(l).m_userPtr;
                    if (motor)
                    {
                        Scalar impulse = motor->getAppliedImpulse(d);
                        Scalar force = impulse / m_data->m_physicsDeltaTime;
                        stateDetails->m_jointMotorForceMultiDof[totalDegreeOfFreedomU] = force;
                    }
                }
                else
                {
                    if (supportsJointMotor(mb, l))
                    {
                        MultiBodyJointMotor* motor = (MultiBodyJointMotor*)body->m_multiBody->getLink(l).m_userPtr;

                        if (motor && m_data->m_physicsDeltaTime > Scalar(0))
                        {
                            Scalar force = motor->getAppliedImpulse(0) / m_data->m_physicsDeltaTime;
                            stateDetails->m_jointMotorForceMultiDof[totalDegreeOfFreedomU] = force;
                        }
                    }
                }

                stateDetails->m_actualStateQdot[totalDegreeOfFreedomU++] = mb->getJointVelMultiDof(l)[d];
            }

            if (0 == mb->getLink(l).m_jointFeedback)
            {
                for (i32 d = 0; d < 6; d++)
                {
                    stateDetails->m_jointReactionForces[l * 6 + d] = 0;
                }
            }
            else
            {
                Vec3 sensedForce = mb->getLink(l).m_jointFeedback->m_reactionForces.getLinear();
                Vec3 sensedTorque = mb->getLink(l).m_jointFeedback->m_reactionForces.getAngular();

                stateDetails->m_jointReactionForces[l * 6 + 0] = sensedForce[0];
                stateDetails->m_jointReactionForces[l * 6 + 1] = sensedForce[1];
                stateDetails->m_jointReactionForces[l * 6 + 2] = sensedForce[2];

                stateDetails->m_jointReactionForces[l * 6 + 3] = sensedTorque[0];
                stateDetails->m_jointReactionForces[l * 6 + 4] = sensedTorque[1];
                stateDetails->m_jointReactionForces[l * 6 + 5] = sensedTorque[2];
            }

            stateDetails->m_jointMotorForce[l] = 0;

            if (supportsJointMotor(mb, l))
            {
                MultiBodyJointMotor* motor = (MultiBodyJointMotor*)body->m_multiBody->getLink(l).m_userPtr;

                if (motor && m_data->m_physicsDeltaTime > Scalar(0))
                {
                    Scalar force = motor->getAppliedImpulse(0) / m_data->m_physicsDeltaTime;
                    stateDetails->m_jointMotorForce[l] =
                        force;
                    //if (force>0)
                    //{
                    //   drx3DPrintf("force = %f\n", force);
                    //}
                }
            }
            Vec3 linkLocalInertialOrigin = body->m_linkLocalInertialFrames[l].getOrigin();
            Quat linkLocalInertialRotation = body->m_linkLocalInertialFrames[l].getRotation();

            Vec3 linkCOMOrigin = mb->getLink(l).m_cachedWorldTransform.getOrigin();
            Quat linkCOMRotation = mb->getLink(l).m_cachedWorldTransform.getRotation();

            stateDetails->m_linkState[l * 7 + 0] = linkCOMOrigin.getX();
            stateDetails->m_linkState[l * 7 + 1] = linkCOMOrigin.getY();
            stateDetails->m_linkState[l * 7 + 2] = linkCOMOrigin.getZ();
            stateDetails->m_linkState[l * 7 + 3] = linkCOMRotation.x();
            stateDetails->m_linkState[l * 7 + 4] = linkCOMRotation.y();
            stateDetails->m_linkState[l * 7 + 5] = linkCOMRotation.z();
            stateDetails->m_linkState[l * 7 + 6] = linkCOMRotation.w();

            Vec3 worldLinVel(0, 0, 0);
            Vec3 worldAngVel(0, 0, 0);

            if (computeLinkVelocities)
            {
                const Matrix3x3& linkRotMat = mb->getLink(l).m_cachedWorldTransform.getBasis();
                worldLinVel = linkRotMat * linVel[l + 1];
                worldAngVel = linkRotMat * omega[l + 1];
            }

            stateDetails->m_linkWorldVelocities[l * 6 + 0] = worldLinVel[0];
            stateDetails->m_linkWorldVelocities[l * 6 + 1] = worldLinVel[1];
            stateDetails->m_linkWorldVelocities[l * 6 + 2] = worldLinVel[2];
            stateDetails->m_linkWorldVelocities[l * 6 + 3] = worldAngVel[0];
            stateDetails->m_linkWorldVelocities[l * 6 + 4] = worldAngVel[1];
            stateDetails->m_linkWorldVelocities[l * 6 + 5] = worldAngVel[2];

            stateDetails->m_linkLocalInertialFrames[l * 7 + 0] = linkLocalInertialOrigin.getX();
            stateDetails->m_linkLocalInertialFrames[l * 7 + 1] = linkLocalInertialOrigin.getY();
            stateDetails->m_linkLocalInertialFrames[l * 7 + 2] = linkLocalInertialOrigin.getZ();

            stateDetails->m_linkLocalInertialFrames[l * 7 + 3] = linkLocalInertialRotation.x();
            stateDetails->m_linkLocalInertialFrames[l * 7 + 4] = linkLocalInertialRotation.y();
            stateDetails->m_linkLocalInertialFrames[l * 7 + 5] = linkLocalInertialRotation.z();
            stateDetails->m_linkLocalInertialFrames[l * 7 + 6] = linkLocalInertialRotation.w();
        }

        serverCmd.m_sendActualStateArgs.m_numDegreeOfFreedomQ = totalDegreeOfFreedomQ;
        serverCmd.m_sendActualStateArgs.m_numDegreeOfFreedomU = totalDegreeOfFreedomU;

        hasStatus = true;
    }
    else if (body && body->m_rigidBody)
    {
        RigidBody* rb = body->m_rigidBody;
        SharedMemoryStatus& serverCmd = serverStatusOut;
        serverCmd.m_type = CMD_ACTUAL_STATE_UPDATE_COMPLETED;

        serverCmd.m_sendActualStateArgs.m_bodyUniqueId = bodyUniqueId;
        serverCmd.m_sendActualStateArgs.m_numLinks = 0;
        serverCmd.m_numDataStreamBytes = sizeof(SendActualStateSharedMemoryStorage);
        serverCmd.m_sendActualStateArgs.m_stateDetails = 0;

        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[0] =
            body->m_rootLocalInertialFrame.getOrigin()[0];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[1] =
            body->m_rootLocalInertialFrame.getOrigin()[1];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[2] =
            body->m_rootLocalInertialFrame.getOrigin()[2];

        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[3] =
            body->m_rootLocalInertialFrame.getRotation()[0];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[4] =
            body->m_rootLocalInertialFrame.getRotation()[1];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[5] =
            body->m_rootLocalInertialFrame.getRotation()[2];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[6] =
            body->m_rootLocalInertialFrame.getRotation()[3];

        Transform2 tr = rb->getWorldTransform();
        //base position in world space, cartesian
        stateDetails->m_actualStateQ[0] = tr.getOrigin()[0];
        stateDetails->m_actualStateQ[1] = tr.getOrigin()[1];
        stateDetails->m_actualStateQ[2] = tr.getOrigin()[2];

        //base orientation, quaternion x,y,z,w, in world space, cartesian
        stateDetails->m_actualStateQ[3] = tr.getRotation()[0];
        stateDetails->m_actualStateQ[4] = tr.getRotation()[1];
        stateDetails->m_actualStateQ[5] = tr.getRotation()[2];
        stateDetails->m_actualStateQ[6] = tr.getRotation()[3];
        i32 totalDegreeOfFreedomQ = 7;  //pos + quaternion

        //base linear velocity (in world space, cartesian)
        stateDetails->m_actualStateQdot[0] = rb->getLinearVelocity()[0];
        stateDetails->m_actualStateQdot[1] = rb->getLinearVelocity()[1];
        stateDetails->m_actualStateQdot[2] = rb->getLinearVelocity()[2];

        //base angular velocity (in world space, cartesian)
        stateDetails->m_actualStateQdot[3] = rb->getAngularVelocity()[0];
        stateDetails->m_actualStateQdot[4] = rb->getAngularVelocity()[1];
        stateDetails->m_actualStateQdot[5] = rb->getAngularVelocity()[2];
        i32 totalDegreeOfFreedomU = 6;  //3 linear and 3 angular DOF

        serverCmd.m_sendActualStateArgs.m_numDegreeOfFreedomQ = totalDegreeOfFreedomQ;
        serverCmd.m_sendActualStateArgs.m_numDegreeOfFreedomU = totalDegreeOfFreedomU;

        hasStatus = true;
    }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    else if (body && body->m_softBody)
    {
        SoftBody* sb = body->m_softBody;
        SharedMemoryStatus& serverCmd = serverStatusOut;
        serverCmd.m_type = CMD_ACTUAL_STATE_UPDATE_COMPLETED;
        serverCmd.m_sendActualStateArgs.m_bodyUniqueId = bodyUniqueId;
        serverCmd.m_sendActualStateArgs.m_numLinks = 0;
        serverCmd.m_numDataStreamBytes = sizeof(SendActualStateSharedMemoryStorage);
        serverCmd.m_sendActualStateArgs.m_stateDetails = 0;

        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[0] =
            body->m_rootLocalInertialFrame.getOrigin()[0];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[1] =
            body->m_rootLocalInertialFrame.getOrigin()[1];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[2] =
            body->m_rootLocalInertialFrame.getOrigin()[2];

        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[3] =
            body->m_rootLocalInertialFrame.getRotation()[0];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[4] =
            body->m_rootLocalInertialFrame.getRotation()[1];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[5] =
            body->m_rootLocalInertialFrame.getRotation()[2];
        serverCmd.m_sendActualStateArgs.m_rootLocalInertialFrame[6] =
            body->m_rootLocalInertialFrame.getRotation()[3];

        Vec3 center_of_mass(sb->getCenterOfMass());
        Transform2 tr = sb->getRigidTransform();
        //base position in world space, cartesian
        stateDetails->m_actualStateQ[0] = center_of_mass[0];
        stateDetails->m_actualStateQ[1] = center_of_mass[1];
        stateDetails->m_actualStateQ[2] = center_of_mass[2];

        //base orientation, quaternion x,y,z,w, in world space, cartesian
        stateDetails->m_actualStateQ[3] = tr.getRotation()[0];
        stateDetails->m_actualStateQ[4] = tr.getRotation()[1];
        stateDetails->m_actualStateQ[5] = tr.getRotation()[2];
        stateDetails->m_actualStateQ[6] = tr.getRotation()[3];

        i32 totalDegreeOfFreedomQ = 7;  //pos + quaternion
        i32 totalDegreeOfFreedomU = 6;  //3 linear and 3 angular DOF

        serverCmd.m_sendActualStateArgs.m_numDegreeOfFreedomQ = totalDegreeOfFreedomQ;
        serverCmd.m_sendActualStateArgs.m_numDegreeOfFreedomU = totalDegreeOfFreedomU;

        hasStatus = true;
    }
#endif
    else
    {
        //drx3DWarning("Request state but no multibody or rigid body available");
        SharedMemoryStatus& serverCmd = serverStatusOut;
        serverCmd.m_type = CMD_ACTUAL_STATE_UPDATE_FAILED;
        hasStatus = true;
    }
    return hasStatus;
}

bool RequestFiltered(const struct SharedMemoryCommand& clientCmd, i32& linkIndexA, i32& linkIndexB, i32& objectIndexA, i32& objectIndexB, bool& swap){

    if (clientCmd.m_requestContactPointArguments.m_objectAIndexFilter >= 0)
    {
        if (clientCmd.m_requestContactPointArguments.m_objectAIndexFilter == objectIndexA)
        {
            swap = false;
        }
        else if (clientCmd.m_requestContactPointArguments.m_objectAIndexFilter == objectIndexB)
        {
            swap = true;
        }
        else
        {
            return true;
        }
    }

    if (swap)
    {
        std::swap(objectIndexA, objectIndexB);
        std::swap(linkIndexA, linkIndexB);
    }

    //apply the second object filter, if the user provides it
    if (clientCmd.m_requestContactPointArguments.m_objectBIndexFilter >= 0)
    {
        if (clientCmd.m_requestContactPointArguments.m_objectBIndexFilter != objectIndexB)
        {
            return true;
        }
    }

    if (
        (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_A_FILTER) &&
        clientCmd.m_requestContactPointArguments.m_linkIndexAIndexFilter != linkIndexA)
    {
        return true;
    }

    if (
        (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_B_FILTER) &&
        clientCmd.m_requestContactPointArguments.m_linkIndexBIndexFilter != linkIndexB)
    {
        return true;
    }

    return false;
}

bool PhysicsServerCommandProcessor::processRequestDeformableDeformableContactpointHelper(const struct SharedMemoryCommand& clientCmd){
#ifndef SKIP_DEFORMABLE_BODY
    DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
    if (!deformWorld)
    {
        return false;
    }
    i32k max_contacts_per_object = 4;
    for (i32 i = deformWorld->getSoftBodyArray().size() - 1; i >= 0; i--)
    {
        i32 num_contacts_reported = 0;
        SoftBody* psb = deformWorld->getSoftBodyArray()[i];
        for (i32 c = 0; c < psb->m_faceNodeContacts.size(); c++)
        {
            const SoftBody::DeformableFaceNodeContact* contact = &psb->m_faceNodeContacts[c];
            //apply the filter, if the user provides it
            i32 linkIndexA = -1;
            i32 linkIndexB = -1;
            i32 objectIndexA = psb->getUserIndex2();
            i32 objectIndexB = -1;
            const SoftBody* bodyB = SoftBody::upcast(contact->m_colObj);
            if (bodyB)
            {
                objectIndexB = bodyB->getUserIndex2();
            }
            bool swap = false;
            if(RequestFiltered(clientCmd, linkIndexA, linkIndexB, objectIndexA, objectIndexB, swap)==true){
                continue;
            }
            if (++num_contacts_reported > max_contacts_per_object)
            {
                break;
            }
            //Convert contact info
            b3ContactPointData pt;
            Vec3 l = contact->m_node->m_x - BaryEval(contact->m_face->m_n[0]->m_x, contact->m_face->m_n[1]->m_x, contact->m_face->m_n[2]->m_x, contact->m_normal);
            pt.m_contactDistance = -contact->m_margin + contact->m_normal.dot(l);
            pt.m_bodyUniqueIdA = objectIndexA;
            pt.m_bodyUniqueIdB = objectIndexB;
            pt.m_contactFlags = 0;
            pt.m_linkIndexA = linkIndexA;
            pt.m_linkIndexB = linkIndexB;
            for (i32 j = 0; j < 3; j++)
            {
                if (swap)
                {
                    pt.m_contactNormalOnBInWS[j] = -contact->m_normal[j];
                }
                else
                {
                    pt.m_contactNormalOnBInWS[j] = contact->m_normal[j];
                }
                pt.m_positionOnAInWS[j] = contact->m_node->m_x[j];
                pt.m_positionOnBInWS[j] = contact->m_node->m_x[j];
                pt.m_linearFrictionDirection1[j] = 0;
                pt.m_linearFrictionDirection2[j] = 0;
            }
            pt.m_normalForce = 0;
            pt.m_linearFrictionForce1 = 0;
            pt.m_linearFrictionForce2 = 0;
           m_data->m_cachedContactPoints.push_back(pt);
        }
    }
#endif
    return true;
}



bool PhysicsServerCommandProcessor::processRequestDeformableContactpointHelper(const struct SharedMemoryCommand& clientCmd)
{
#ifndef SKIP_DEFORMABLE_BODY
    DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
    if (!deformWorld)
    {
        return false;
    }
    i32 numSoftbodyContact = 0;
    for (i32 i = deformWorld->getSoftBodyArray().size() - 1; i >= 0; i--)
    {
        numSoftbodyContact += deformWorld->getSoftBodyArray()[i]->m_faceRigidContacts.size();
    }
    i32 num_contact_points = m_data->m_cachedContactPoints.size();
    m_data->m_cachedContactPoints.reserve(num_contact_points + numSoftbodyContact);

    for (i32 i = deformWorld->getSoftBodyArray().size() - 1; i >= 0; i--)
    {
        SoftBody* psb = deformWorld->getSoftBodyArray()[i];
        for (i32 c = 0; c < psb->m_faceRigidContacts.size(); c++)
        {
            const SoftBody::DeformableFaceRigidContact* contact = &psb->m_faceRigidContacts[c];
            //convert rigidbody contact
            i32 linkIndexA = -1;
            i32 linkIndexB = -1;
            i32 objectIndexA = psb->getUserIndex2();

            i32 objectIndexB = -1;
            const RigidBody* bodyB = RigidBody::upcast(contact->m_cti.m_colObj);
            if (bodyB)
            {
                objectIndexB = bodyB->getUserIndex2();
            }
            const MultiBodyLinkCollider* mblB = MultiBodyLinkCollider::upcast(contact->m_cti.m_colObj);
            if (mblB && mblB->m_multiBody)
            {
                linkIndexB = mblB->m_link;
                objectIndexB = mblB->m_multiBody->getUserIndex2();
            }

            //apply the filter, if the user provides it
            bool swap = false;
            if (clientCmd.m_requestContactPointArguments.m_objectAIndexFilter >= 0)
            {
                if (clientCmd.m_requestContactPointArguments.m_objectAIndexFilter == objectIndexA)
                {
                    swap = false;
                }
                else if (clientCmd.m_requestContactPointArguments.m_objectAIndexFilter == objectIndexB)
                {
                    swap = true;
                }
                else
                {
                    continue;
                }
            }

            if (swap)
            {
                std::swap(objectIndexA, objectIndexB);
                std::swap(linkIndexA, linkIndexB);
            }

            //apply the second object filter, if the user provides it
            if (clientCmd.m_requestContactPointArguments.m_objectBIndexFilter >= 0)
            {
                if (clientCmd.m_requestContactPointArguments.m_objectBIndexFilter != objectIndexB)
                {
                    continue;
                }
            }

            if (
                (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_A_FILTER) &&
                clientCmd.m_requestContactPointArguments.m_linkIndexAIndexFilter != linkIndexA)
            {
                continue;
            }

            if (
                (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_B_FILTER) &&
                clientCmd.m_requestContactPointArguments.m_linkIndexBIndexFilter != linkIndexB)
            {
                continue;
            }
            b3ContactPointData pt;
            pt.m_bodyUniqueIdA = objectIndexA;
            pt.m_bodyUniqueIdB = objectIndexB;
            pt.m_contactDistance = contact->m_cti.m_offset;
            pt.m_contactFlags = 0;
            pt.m_linkIndexA = linkIndexA;
            pt.m_linkIndexB = linkIndexB;
            for (i32 j = 0; j < 3; j++)
            {
                if (swap)
                {
                    pt.m_contactNormalOnBInWS[j] = -contact->m_cti.m_normal[j];
                    pt.m_positionOnAInWS[j] = contact->m_cti.m_normal[j];
                    pt.m_positionOnBInWS[j] = -contact->m_cti.m_normal[j];
                }
                else
                {
                    pt.m_contactNormalOnBInWS[j] = contact->m_cti.m_normal[j];
                    pt.m_positionOnAInWS[j] = -contact->m_cti.m_normal[j];
                    pt.m_positionOnBInWS[j] = contact->m_cti.m_normal[j];
                }
            }
            pt.m_normalForce = 1;
            pt.m_linearFrictionForce1 = 0;
            pt.m_linearFrictionForce2 = 0;
            for (i32 j = 0; j < 3; j++)
            {
                pt.m_linearFrictionDirection1[j] = 0;
                pt.m_linearFrictionDirection2[j] = 0;
            }
            m_data->m_cachedContactPoints.push_back(pt);
        }
    }
#endif
    return true;
}

bool PhysicsServerCommandProcessor::processRequestContactpointInformationCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REQUEST_CONTACT_POINT_INFORMATION");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_sendContactPointArgs.m_numContactPointsCopied = 0;

    //make a snapshot of the contact manifolds into individual contact points
    if (clientCmd.m_requestContactPointArguments.m_startingContactPointIndex == 0)
    {
        m_data->m_cachedContactPoints.resize(0);

        i32 mode = CONTACT_QUERY_MODE_REPORT_EXISTING_CONTACT_POINTS;

        if (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_QUERY_MODE)
        {
            mode = clientCmd.m_requestContactPointArguments.m_mode;
        }

        switch (mode)
        {
            case CONTACT_QUERY_MODE_REPORT_EXISTING_CONTACT_POINTS:
            {
                i32 numContactManifolds = m_data->m_dynamicsWorld->getDispatcher()->getNumManifolds();
                m_data->m_cachedContactPoints.reserve(numContactManifolds * 4);
                for (i32 i = 0; i < numContactManifolds; i++)
                {
                    const PersistentManifold* manifold = m_data->m_dynamicsWorld->getDispatcher()->getInternalManifoldPointer()[i];
                    i32 linkIndexA = -1;
                    i32 linkIndexB = -1;

                    i32 objectIndexB = -1;
                    const RigidBody* bodyB = RigidBody::upcast(manifold->getBody1());
                    if (bodyB)
                    {
                        objectIndexB = bodyB->getUserIndex2();
                    }
                    const MultiBodyLinkCollider* mblB = MultiBodyLinkCollider::upcast(manifold->getBody1());
                    if (mblB && mblB->m_multiBody)
                    {
                        linkIndexB = mblB->m_link;
                        objectIndexB = mblB->m_multiBody->getUserIndex2();
                    }

                    i32 objectIndexA = -1;
                    const RigidBody* bodyA = RigidBody::upcast(manifold->getBody0());
                    if (bodyA)
                    {
                        objectIndexA = bodyA->getUserIndex2();
                    }
                    const MultiBodyLinkCollider* mblA = MultiBodyLinkCollider::upcast(manifold->getBody0());
                    if (mblA && mblA->m_multiBody)
                    {
                        linkIndexA = mblA->m_link;
                        objectIndexA = mblA->m_multiBody->getUserIndex2();
                    }
                    Assert(bodyA || mblA);

                    //apply the filter, if the user provides it
                    bool swap = false;
                    if (clientCmd.m_requestContactPointArguments.m_objectAIndexFilter >= 0)
                    {
                        if (clientCmd.m_requestContactPointArguments.m_objectAIndexFilter == objectIndexA)
                        {
                            swap = false;
                        }
                        else if (clientCmd.m_requestContactPointArguments.m_objectAIndexFilter == objectIndexB)
                        {
                            swap = true;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    if (swap)
                    {
                        std::swap(objectIndexA, objectIndexB);
                        std::swap(linkIndexA, linkIndexB);
                        std::swap(bodyA, bodyB);
                    }

                    //apply the second object filter, if the user provides it
                    if (clientCmd.m_requestContactPointArguments.m_objectBIndexFilter >= 0)
                    {
                        if (clientCmd.m_requestContactPointArguments.m_objectBIndexFilter != objectIndexB)
                        {
                            continue;
                        }
                    }

                    if (
                        (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_A_FILTER) &&
                        clientCmd.m_requestContactPointArguments.m_linkIndexAIndexFilter != linkIndexA)
                    {
                        continue;
                    }

                    if (
                        (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_B_FILTER) &&
                        clientCmd.m_requestContactPointArguments.m_linkIndexBIndexFilter != linkIndexB)
                    {
                        continue;
                    }

                    for (i32 p = 0; p < manifold->getNumContacts(); p++)
                    {
                        b3ContactPointData pt;
                        pt.m_bodyUniqueIdA = objectIndexA;
                        pt.m_bodyUniqueIdB = objectIndexB;
                        const ManifoldPoint& srcPt = manifold->getContactPoint(p);
                        pt.m_contactDistance = srcPt.getDistance();
                        pt.m_contactFlags = 0;
                        pt.m_linkIndexA = linkIndexA;
                        pt.m_linkIndexB = linkIndexB;
                        for (i32 j = 0; j < 3; j++)
                        {
                            if (swap)
                            {
                                pt.m_contactNormalOnBInWS[j] = -srcPt.m_normalWorldOnB[j];
                                pt.m_positionOnAInWS[j] = srcPt.getPositionWorldOnB()[j];
                                pt.m_positionOnBInWS[j] = srcPt.getPositionWorldOnA()[j];
                            }
                            else
                            {
                                pt.m_contactNormalOnBInWS[j] = srcPt.m_normalWorldOnB[j];
                                pt.m_positionOnAInWS[j] = srcPt.getPositionWorldOnA()[j];
                                pt.m_positionOnBInWS[j] = srcPt.getPositionWorldOnB()[j];
                            }
                        }
                        pt.m_normalForce = srcPt.getAppliedImpulse() / m_data->m_physicsDeltaTime;
                        pt.m_linearFrictionForce1 = srcPt.m_appliedImpulseLateral1 / m_data->m_physicsDeltaTime;
                        pt.m_linearFrictionForce2 = srcPt.m_appliedImpulseLateral2 / m_data->m_physicsDeltaTime;
                        for (i32 j = 0; j < 3; j++)
                        {
                            pt.m_linearFrictionDirection1[j] = srcPt.m_lateralFrictionDir1[j];
                            pt.m_linearFrictionDirection2[j] = srcPt.m_lateralFrictionDir2[j];
                        }
                        m_data->m_cachedContactPoints.push_back(pt);
                    }
                }

#ifndef SKIP_DEFORMABLE_BODY
                processRequestDeformableContactpointHelper(clientCmd);
                processRequestDeformableDeformableContactpointHelper(clientCmd);
#endif
                break;
            }

            case CONTACT_QUERY_MODE_COMPUTE_CLOSEST_POINTS:
            {
                //todo(erwincoumans) compute closest points between all, and vs all, pair
                Scalar closestDistanceThreshold = 0.f;

                if (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_CLOSEST_DISTANCE_THRESHOLD)
                {
                    closestDistanceThreshold = clientCmd.m_requestContactPointArguments.m_closestDistanceThreshold;
                }

                i32 bodyUniqueIdA = clientCmd.m_requestContactPointArguments.m_objectAIndexFilter;
                i32 bodyUniqueIdB = clientCmd.m_requestContactPointArguments.m_objectBIndexFilter;

                bool hasLinkIndexAFilter = (0 != (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_A_FILTER));
                bool hasLinkIndexBFilter = (0 != (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_B_FILTER));

                i32 linkIndexA = clientCmd.m_requestContactPointArguments.m_linkIndexAIndexFilter;
                i32 linkIndexB = clientCmd.m_requestContactPointArguments.m_linkIndexBIndexFilter;

                AlignedObjectArray<CollisionObject2*> setA;
                AlignedObjectArray<CollisionObject2*> setB;
                AlignedObjectArray<i32> setALinkIndex;
                AlignedObjectArray<i32> setBLinkIndex;

                CollisionObject2 colObA;
                CollisionObject2 colObB;

                i32 collisionShapeA = (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_A) ? clientCmd.m_requestContactPointArguments.m_collisionShapeA : -1;
                i32 collisionShapeB = (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_B) ? clientCmd.m_requestContactPointArguments.m_collisionShapeB : -1;

                if (collisionShapeA >= 0)
                {
                    Vec3 posA(0, 0, 0);
                    if (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_POSITION_A)
                    {
                        posA.setVal(clientCmd.m_requestContactPointArguments.m_collisionShapePositionA[0],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapePositionA[1],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapePositionA[2]);
                    }
                    Quat ornA(0, 0, 0, 1);
                    if (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_ORIENTATION_A)
                    {
                        ornA.setVal(clientCmd.m_requestContactPointArguments.m_collisionShapeOrientationA[0],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapeOrientationA[1],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapeOrientationA[2],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapeOrientationA[3]);
                    }
                    InternalCollisionShapeHandle* handle = m_data->m_userCollisionShapeHandles.getHandle(collisionShapeA);

                    if (handle && handle->m_collisionShape)
                    {
                        colObA.setCollisionShape(handle->m_collisionShape);
                        Transform2 tr;
                        tr.setIdentity();
                        tr.setOrigin(posA);
                        tr.setRotation(ornA);
                        colObA.setWorldTransform(tr);
                        setA.push_back(&colObA);
                        setALinkIndex.push_back(-2);
                    }
                    else
                    {
                        drx3DWarning("collisionShapeA provided is not valid.");
                    }
                }
                if (collisionShapeB >= 0)
                {
                    Vec3 posB(0, 0, 0);
                    if (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_POSITION_B)
                    {
                        posB.setVal(clientCmd.m_requestContactPointArguments.m_collisionShapePositionB[0],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapePositionB[1],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapePositionB[2]);
                    }
                    Quat ornB(0, 0, 0, 1);
                    if (clientCmd.m_updateFlags & CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_ORIENTATION_B)
                    {
                        ornB.setVal(clientCmd.m_requestContactPointArguments.m_collisionShapeOrientationB[0],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapeOrientationB[1],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapeOrientationB[2],
                                      clientCmd.m_requestContactPointArguments.m_collisionShapeOrientationB[3]);
                    }

                    InternalCollisionShapeHandle* handle = m_data->m_userCollisionShapeHandles.getHandle(collisionShapeB);
                    if (handle && handle->m_collisionShape)
                    {
                        colObB.setCollisionShape(handle->m_collisionShape);
                        Transform2 tr;
                        tr.setIdentity();
                        tr.setOrigin(posB);
                        tr.setRotation(ornB);
                        colObB.setWorldTransform(tr);
                        setB.push_back(&colObB);
                        setBLinkIndex.push_back(-2);
                    }
                    else
                    {
                        drx3DWarning("collisionShapeB provided is not valid.");
                    }
                }

                if (bodyUniqueIdA >= 0)
                {
                    InternalBodyData* bodyA = m_data->m_bodyHandles.getHandle(bodyUniqueIdA);
                    if (bodyA)
                    {
                        if (bodyA->m_multiBody)
                        {
                            if (bodyA->m_multiBody->getBaseCollider())
                            {
                                if (!hasLinkIndexAFilter || (linkIndexA == -1))
                                {
                                    setA.push_back(bodyA->m_multiBody->getBaseCollider());
                                    setALinkIndex.push_back(-1);
                                }
                            }
                            for (i32 i = 0; i < bodyA->m_multiBody->getNumLinks(); i++)
                            {
                                if (bodyA->m_multiBody->getLink(i).m_collider)
                                {
                                    if (!hasLinkIndexAFilter || (linkIndexA == i))
                                    {
                                        setA.push_back(bodyA->m_multiBody->getLink(i).m_collider);
                                        setALinkIndex.push_back(i);
                                    }
                                }
                            }
                        }
                        if (bodyA->m_rigidBody)
                        {
                            setA.push_back(bodyA->m_rigidBody);
                            setALinkIndex.push_back(-1);
                        }
                    }
                }
                if (bodyUniqueIdB >= 0)
                {
                    InternalBodyData* bodyB = m_data->m_bodyHandles.getHandle(bodyUniqueIdB);
                    if (bodyB)
                    {
                        if (bodyB->m_multiBody)
                        {
                            if (bodyB->m_multiBody->getBaseCollider())
                            {
                                if (!hasLinkIndexBFilter || (linkIndexB == -1))
                                {
                                    setB.push_back(bodyB->m_multiBody->getBaseCollider());
                                    setBLinkIndex.push_back(-1);
                                }
                            }
                            for (i32 i = 0; i < bodyB->m_multiBody->getNumLinks(); i++)
                            {
                                if (bodyB->m_multiBody->getLink(i).m_collider)
                                {
                                    if (!hasLinkIndexBFilter || (linkIndexB == i))
                                    {
                                        setB.push_back(bodyB->m_multiBody->getLink(i).m_collider);
                                        setBLinkIndex.push_back(i);
                                    }
                                }
                            }
                        }
                        if (bodyB->m_rigidBody)
                        {
                            setB.push_back(bodyB->m_rigidBody);
                            setBLinkIndex.push_back(-1);
                        }
                    }
                }

                {
                    ///ContactResultCallback is used to report contact points
                    struct MyContactResultCallback : public CollisionWorld::ContactResultCallback
                    {
                        i32 m_bodyUniqueIdA;
                        i32 m_bodyUniqueIdB;
                        i32 m_linkIndexA;
                        i32 m_linkIndexB;
                        Scalar m_deltaTime;

                        AlignedObjectArray<b3ContactPointData>& m_cachedContactPoints;

                        MyContactResultCallback(AlignedObjectArray<b3ContactPointData>& pointCache)
                            : m_cachedContactPoints(pointCache)
                        {
                        }

                        virtual ~MyContactResultCallback()
                        {
                        }

                        virtual bool needsCollision(BroadphaseProxy* proxy0) const
                        {
                            //bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
                            //collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
                            //return collides;
                            return true;
                        }

                        virtual Scalar addSingleResult(ManifoldPoint& cp, const CollisionObject2Wrapper* colObj0Wrap, i32 partId0, i32 index0, const CollisionObject2Wrapper* colObj1Wrap, i32 partId1, i32 index1)
                        {
                            const CollisionObject2* colObj = (CollisionObject2*)colObj0Wrap->getCollisionObject();
                            const MultiBodyLinkCollider* mbl = MultiBodyLinkCollider::upcast(colObj);
                            i32 bodyUniqueId = -1;
                            if (mbl)
                            {
                                bodyUniqueId = mbl->m_multiBody->getUserIndex2();
                            }
                            else
                            {
                                bodyUniqueId = colObj->getUserIndex2();
                            }


                            bool isSwapped = m_bodyUniqueIdA != bodyUniqueId;

                            if (cp.m_distance1 <= m_closestDistanceThreshold)
                            {
                                b3ContactPointData pt;
                                pt.m_bodyUniqueIdA = m_bodyUniqueIdA;
                                pt.m_bodyUniqueIdB = m_bodyUniqueIdB;
                                const ManifoldPoint& srcPt = cp;
                                pt.m_contactDistance = srcPt.getDistance();
                                pt.m_contactFlags = 0;
                                pt.m_linkIndexA = m_linkIndexA;
                                pt.m_linkIndexB = m_linkIndexB;
                                for (i32 j = 0; j < 3; j++)
                                {
                                    if (isSwapped)
                                    {
                                        pt.m_contactNormalOnBInWS[j] = -srcPt.m_normalWorldOnB[j];
                                        pt.m_positionOnAInWS[j] = srcPt.getPositionWorldOnB()[j];
                                        pt.m_positionOnBInWS[j] = srcPt.getPositionWorldOnA()[j];
                                    }
                                    else
                                    {
                                        pt.m_contactNormalOnBInWS[j] = srcPt.m_normalWorldOnB[j];
                                        pt.m_positionOnAInWS[j] = srcPt.getPositionWorldOnA()[j];
                                        pt.m_positionOnBInWS[j] = srcPt.getPositionWorldOnB()[j];
                                    }
                                }
                                pt.m_normalForce = srcPt.getAppliedImpulse() / m_deltaTime;
                                pt.m_linearFrictionForce1 = srcPt.m_appliedImpulseLateral1 / m_deltaTime;
                                pt.m_linearFrictionForce2 = srcPt.m_appliedImpulseLateral2 / m_deltaTime;
                                for (i32 j = 0; j < 3; j++)
                                {
                                    pt.m_linearFrictionDirection1[j] = srcPt.m_lateralFrictionDir1[j];
                                    pt.m_linearFrictionDirection2[j] = srcPt.m_lateralFrictionDir2[j];
                                }
                                m_cachedContactPoints.push_back(pt);
                            }
                            return 1;
                        }
                    };

                    MyContactResultCallback cb(m_data->m_cachedContactPoints);

                    cb.m_bodyUniqueIdA = bodyUniqueIdA;
                    cb.m_bodyUniqueIdB = bodyUniqueIdB;
                    cb.m_deltaTime = m_data->m_numSimulationSubSteps > 0 ? m_data->m_physicsDeltaTime / m_data->m_numSimulationSubSteps : m_data->m_physicsDeltaTime;

                    for (i32 i = 0; i < setA.size(); i++)
                    {
                        cb.m_linkIndexA = setALinkIndex[i];
                        for (i32 j = 0; j < setB.size(); j++)
                        {
                            cb.m_linkIndexB = setBLinkIndex[j];
                            cb.m_closestDistanceThreshold = closestDistanceThreshold;
                            this->m_data->m_dynamicsWorld->contactPairTest(setA[i], setB[j], cb);
                        }
                    }
                }

                break;
            }
            default:
            {
                drx3DWarning("Unknown contact query mode: %d", mode);
            }
        }
    }

    i32 numContactPoints = m_data->m_cachedContactPoints.size();

    //b3ContactPoint
    //struct b3ContactPointDynamics

    i32 totalBytesPerContact = sizeof(b3ContactPointData);
    i32 contactPointStorage = bufferSizeInBytes / totalBytesPerContact - 1;

    b3ContactPointData* contactData = (b3ContactPointData*)bufferServerToClient;

    i32 startContactPointIndex = clientCmd.m_requestContactPointArguments.m_startingContactPointIndex;
    i32 numContactPointBatch = d3Min(numContactPoints, contactPointStorage);

    i32 endContactPointIndex = startContactPointIndex + numContactPointBatch;

    for (i32 i = startContactPointIndex; i < endContactPointIndex; i++)
    {
        const b3ContactPointData& srcPt = m_data->m_cachedContactPoints[i];
        b3ContactPointData& destPt = contactData[serverCmd.m_sendContactPointArgs.m_numContactPointsCopied];
        destPt = srcPt;
        serverCmd.m_sendContactPointArgs.m_numContactPointsCopied++;
    }

    serverCmd.m_sendContactPointArgs.m_startingContactPointIndex = clientCmd.m_requestContactPointArguments.m_startingContactPointIndex;
    serverCmd.m_sendContactPointArgs.m_numRemainingContactPoints = numContactPoints - clientCmd.m_requestContactPointArguments.m_startingContactPointIndex - serverCmd.m_sendContactPointArgs.m_numContactPointsCopied;
    serverCmd.m_numDataStreamBytes = totalBytesPerContact * serverCmd.m_sendContactPointArgs.m_numContactPointsCopied;
    serverCmd.m_type = CMD_CONTACT_POINT_INFORMATION_COMPLETED;  //CMD_CONTACT_POINT_INFORMATION_FAILED,

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REQUEST_BODY_INFO");

    const SdfRequestInfoArgs& sdfInfoArgs = clientCmd.m_sdfRequestInfoArgs;
    //stream info into memory
    i32 streamSizeInBytes = createBodyInfoStream(sdfInfoArgs.m_bodyUniqueId, bufferServerToClient, bufferSizeInBytes);

    serverStatusOut.m_type = CMD_BODY_INFO_COMPLETED;
    serverStatusOut.m_dataStreamArguments.m_bodyUniqueId = sdfInfoArgs.m_bodyUniqueId;
    serverStatusOut.m_dataStreamArguments.m_bodyName[0] = 0;

    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(sdfInfoArgs.m_bodyUniqueId);
    if (bodyHandle)
    {
        strcpy(serverStatusOut.m_dataStreamArguments.m_bodyName, bodyHandle->m_bodyName.c_str());
    }

    serverStatusOut.m_numDataStreamBytes = streamSizeInBytes;

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processLoadSDFCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_LOAD_SDF");

    const SdfArgs& sdfArgs = clientCmd.m_sdfArguments;
    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("Processed CMD_LOAD_SDF:%s", sdfArgs.m_sdfFileName);
    }
    bool useMultiBody = (clientCmd.m_updateFlags & URDF_ARGS_USE_MULTIBODY) ? (sdfArgs.m_useMultiBody != 0) : true;

    i32 flags = CUF_USE_SDF;  //CUF_USE_URDF_INERTIA
    Scalar globalScaling = 1.f;
    if (clientCmd.m_updateFlags & URDF_ARGS_USE_GLOBAL_SCALING)
    {
        globalScaling = sdfArgs.m_globalScaling;
    }
    bool completedOk = loadSdf(sdfArgs.m_sdfFileName, bufferServerToClient, bufferSizeInBytes, useMultiBody, flags, globalScaling);
    if (completedOk)
    {
        m_data->m_guiHelper->autogenerateGraphicsObjects(this->m_data->m_dynamicsWorld);

        //serverStatusOut.m_type = CMD_SDF_LOADING_FAILED;
        serverStatusOut.m_sdfLoadedArgs.m_numBodies = m_data->m_sdfRecentLoadedBodies.size();
        serverStatusOut.m_sdfLoadedArgs.m_numUserConstraints = 0;
        i32 maxBodies = d3Min(MAX_SDF_BODIES, serverStatusOut.m_sdfLoadedArgs.m_numBodies);
        for (i32 i = 0; i < maxBodies; i++)
        {
            serverStatusOut.m_sdfLoadedArgs.m_bodyUniqueIds[i] = m_data->m_sdfRecentLoadedBodies[i];
        }

        serverStatusOut.m_type = CMD_SDF_LOADING_COMPLETED;
    }
    else
    {
        serverStatusOut.m_type = CMD_SDF_LOADING_FAILED;
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCreateMultiBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    if (clientCmd.m_createMultiBodyArgs.m_numBatchObjects > 0)
    {
        //batch of objects, to speed up creation time
        bool result = false;
        SharedMemoryCommand clientCmd2 = clientCmd;
        i32 baseLinkIndex = clientCmd.m_createMultiBodyArgs.m_baseLinkIndex;
        double* basePositionAndOrientations = (double*)bufferServerToClient;
        for (i32 i = 0; i < clientCmd2.m_createMultiBodyArgs.m_numBatchObjects; i++)
        {
            clientCmd2.m_createMultiBodyArgs.m_linkPositions[baseLinkIndex * 3 + 0] = basePositionAndOrientations[0 + i * 3];
            clientCmd2.m_createMultiBodyArgs.m_linkPositions[baseLinkIndex * 3 + 1] = basePositionAndOrientations[1 + i * 3];
            clientCmd2.m_createMultiBodyArgs.m_linkPositions[baseLinkIndex * 3 + 2] = basePositionAndOrientations[2 + i * 3];
            if (i == (clientCmd2.m_createMultiBodyArgs.m_numBatchObjects - 1))
            {
                result = processCreateMultiBodyCommandSingle(clientCmd2, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            }
            else
            {
                result = processCreateMultiBodyCommandSingle(clientCmd2, serverStatusOut, 0, 0);
            }
        }
        m_data->m_guiHelper->autogenerateGraphicsObjects(this->m_data->m_dynamicsWorld);
        return result;
    }
    return processCreateMultiBodyCommandSingle(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
}

bool PhysicsServerCommandProcessor::processCreateMultiBodyCommandSingle(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    DRX3D_PROFILE("processCreateMultiBodyCommand2");
    bool hasStatus = true;

    serverStatusOut.m_type = CMD_CREATE_MULTI_BODY_FAILED;
    if (clientCmd.m_createMultiBodyArgs.m_baseLinkIndex >= 0)
    {
        m_data->m_sdfRecentLoadedBodies.clear();

        i32 flags = 0;

        if (clientCmd.m_updateFlags & MULT_BODY_HAS_FLAGS)
        {
            flags = clientCmd.m_createMultiBodyArgs.m_flags;
        }

        ProgrammaticUrdfInterface u2b(clientCmd.m_createMultiBodyArgs, m_data, flags);

        bool useMultiBody = true;
        if (clientCmd.m_updateFlags & MULT_BODY_USE_MAXIMAL_COORDINATES)
        {
            useMultiBody = false;
        }

        bool ok = 0;
        {
            DRX3D_PROFILE("processImportedObjects");
            ok = processImportedObjects("memory", bufferServerToClient, bufferSizeInBytes, useMultiBody, flags, u2b);
        }

        if (ok)
        {
            DRX3D_PROFILE("post process");
            i32 bodyUniqueId = -1;

            if (m_data->m_sdfRecentLoadedBodies.size() == 1)
            {
                bodyUniqueId = m_data->m_sdfRecentLoadedBodies[0];
            }
            m_data->m_sdfRecentLoadedBodies.clear();
            if (bodyUniqueId >= 0)
            {
                serverStatusOut.m_type = CMD_CREATE_MULTI_BODY_COMPLETED;
                if (bufferSizeInBytes > 0 && serverStatusOut.m_numDataStreamBytes == 0)
                {
                    {
                        DRX3D_PROFILE("autogenerateGraphicsObjects");
                        m_data->m_guiHelper->autogenerateGraphicsObjects(this->m_data->m_dynamicsWorld);
                    }

                    DRX3D_PROFILE("createBodyInfoStream");
                    i32 streamSizeInBytes = createBodyInfoStream(bodyUniqueId, bufferServerToClient, bufferSizeInBytes);
                    serverStatusOut.m_numDataStreamBytes = streamSizeInBytes;

                    serverStatusOut.m_dataStreamArguments.m_bodyUniqueId = bodyUniqueId;
                    InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
                    strcpy(serverStatusOut.m_dataStreamArguments.m_bodyName, body->m_bodyName.c_str());
                }
            }
        }

        //ConvertURDF2Bullet(u2b,creation, rootTrans,m_data->m_dynamicsWorld,useMultiBody,u2b.getPathPrefix(),flags);
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processLoadURDFCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    serverStatusOut.m_type = CMD_URDF_LOADING_FAILED;

    DRX3D_PROFILE("CMD_LOAD_URDF");
    const UrdfArgs& urdfArgs = clientCmd.m_urdfArguments;
    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("Processed CMD_LOAD_URDF:%s", urdfArgs.m_urdfFileName);
    }
    Assert((clientCmd.m_updateFlags & URDF_ARGS_FILE_NAME) != 0);
    Assert(urdfArgs.m_urdfFileName);
    Vec3 initialPos(0, 0, 0);
    Quat initialOrn(0, 0, 0, 1);
    if (clientCmd.m_updateFlags & URDF_ARGS_INITIAL_POSITION)
    {
        initialPos[0] = urdfArgs.m_initialPosition[0];
        initialPos[1] = urdfArgs.m_initialPosition[1];
        initialPos[2] = urdfArgs.m_initialPosition[2];
    }
    i32 urdfFlags = 0;
    if (clientCmd.m_updateFlags & URDF_ARGS_HAS_CUSTOM_URDF_FLAGS)
    {
        urdfFlags = urdfArgs.m_urdfFlags;
    }
    if (clientCmd.m_updateFlags & URDF_ARGS_INITIAL_ORIENTATION)
    {
        initialOrn[0] = urdfArgs.m_initialOrientation[0];
        initialOrn[1] = urdfArgs.m_initialOrientation[1];
        initialOrn[2] = urdfArgs.m_initialOrientation[2];
        initialOrn[3] = urdfArgs.m_initialOrientation[3];
    }
    bool useMultiBody = (clientCmd.m_updateFlags & URDF_ARGS_USE_MULTIBODY) ? (urdfArgs.m_useMultiBody != 0) : true;
    bool useFixedBase = (clientCmd.m_updateFlags & URDF_ARGS_USE_FIXED_BASE) ? (urdfArgs.m_useFixedBase != 0) : false;
    i32 bodyUniqueId;
    Scalar globalScaling = 1.f;
    if (clientCmd.m_updateFlags & URDF_ARGS_USE_GLOBAL_SCALING)
    {
        globalScaling = urdfArgs.m_globalScaling;
    }
    //load the actual URDF and send a report: completed or failed
    bool completedOk = loadUrdf(urdfArgs.m_urdfFileName,
                                initialPos, initialOrn,
                                useMultiBody, useFixedBase, &bodyUniqueId, bufferServerToClient, bufferSizeInBytes, urdfFlags, globalScaling);

    if (completedOk && bodyUniqueId >= 0)
    {
        m_data->m_guiHelper->autogenerateGraphicsObjects(this->m_data->m_dynamicsWorld);

        serverStatusOut.m_type = CMD_URDF_LOADING_COMPLETED;

        i32 streamSizeInBytes = createBodyInfoStream(bodyUniqueId, bufferServerToClient, bufferSizeInBytes);
        serverStatusOut.m_numDataStreamBytes = streamSizeInBytes;

#ifdef ENABLE_LINK_MAPPER
        if (m_data->m_urdfLinkNameMapper.size())
        {
            serverStatusOut.m_numDataStreamBytes = m_data->m_urdfLinkNameMapper.at(m_data->m_urdfLinkNameMapper.size() - 1)->m_memSerializer->getCurrentBufferSize();
        }
#endif
        serverStatusOut.m_dataStreamArguments.m_bodyUniqueId = bodyUniqueId;
        InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
        strcpy(serverStatusOut.m_dataStreamArguments.m_bodyName, body->m_bodyName.c_str());
    }

    return hasStatus;
}

void constructUrdfDeformable(const struct SharedMemoryCommand& clientCmd, UrdfDeformable& deformable, bool verbose)
{
    const LoadSoftBodyArgs& loadSoftBodyArgs = clientCmd.m_loadSoftBodyArguments;
    if (verbose)
    {
        drx3DPrintf("Processed CMD_LOAD_SOFT_BODY:%s", loadSoftBodyArgs.m_fileName);
    }
    Assert((clientCmd.m_updateFlags & LOAD_SOFT_BODY_FILE_NAME) != 0);
    Assert(loadSoftBodyArgs.m_fileName);

    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_UPDATE_MASS)
    {
        deformable.m_mass = loadSoftBodyArgs.m_mass;
    }
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_UPDATE_COLLISION_MARGIN)
    {
        deformable.m_collisionMargin = loadSoftBodyArgs.m_collisionMargin;
    }
    deformable.m_visualFileName = loadSoftBodyArgs.m_fileName;
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_SIM_MESH)
    {
        deformable.m_simFileName = loadSoftBodyArgs.m_simFileName;
    }
    else
    {
        deformable.m_simFileName = "";
    }
#ifndef SKIP_DEFORMABLE_BODY
    deformable.m_springCoefficients.elastic_stiffness = loadSoftBodyArgs.m_springElasticStiffness;
    deformable.m_springCoefficients.damping_stiffness = loadSoftBodyArgs.m_springDampingStiffness;
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_ADD_BENDING_SPRINGS)
    {
        deformable.m_springCoefficients.bending_stiffness = loadSoftBodyArgs.m_springBendingStiffness;
    }

    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_SET_DAMPING_SPRING_MODE)
    {
        deformable.m_springCoefficients.damp_all_directions = loadSoftBodyArgs.m_dampAllDirections;
    }

    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_ADD_COROTATED_FORCE)
    {
        deformable.m_corotatedCoefficients.mu = loadSoftBodyArgs.m_corotatedMu;
        deformable.m_corotatedCoefficients.lambda = loadSoftBodyArgs.m_corotatedLambda;
    }

    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_ADD_NEOHOOKEAN_FORCE)
    {
        deformable.m_neohookeanCoefficients.mu = loadSoftBodyArgs.m_NeoHookeanMu;
        deformable.m_neohookeanCoefficients.lambda = loadSoftBodyArgs.m_NeoHookeanLambda;
        deformable.m_neohookeanCoefficients.damping = loadSoftBodyArgs.m_NeoHookeanDamping;
    }
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_SET_FRICTION_COEFFICIENT)
    {
        deformable.m_friction = loadSoftBodyArgs.m_frictionCoeff;
    }
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_SET_REPULSION_STIFFNESS)
    {
        deformable.m_repulsionStiffness = loadSoftBodyArgs.m_repulsionStiffness;
    }
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_SET_GRAVITY_FACTOR)
    {
        deformable.m_gravFactor = loadSoftBodyArgs.m_gravFactor;
    }
#endif
}

bool PhysicsServerCommandProcessor::processDeformable(const UrdfDeformable& deformable, const Vec3& pos, const Quat& orn, i32* bodyUniqueId, tuk bufferServerToClient, i32 bufferSizeInBytes, Scalar scale, bool useSelfCollision)
{
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    SoftBody* psb = nullptr;
    CommonFileIOInterface* fileIO(m_data->m_pluginManager.getFileIOInterface());
    char relativeFileName[1024];
    char pathPrefix[1024];
    pathPrefix[0] = 0;
    if (fileIO->findResourcePath(deformable.m_visualFileName.c_str(), relativeFileName, 1024))
    {
        b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);
    }
    const STxt& error_message_prefix = "";
    STxt out_found_filename, out_found_sim_filename;
    i32 out_type(0), out_sim_type(0);

    bool foundFile = UrdfFindMeshFile(fileIO, pathPrefix, relativeFileName, error_message_prefix, &out_found_filename, &out_type);
    if (!deformable.m_simFileName.empty())
    {
        bool foundSimMesh = UrdfFindMeshFile(fileIO, pathPrefix, deformable.m_simFileName, error_message_prefix, &out_found_sim_filename, &out_sim_type);
    }
    else
    {
        out_sim_type = out_type;
        out_found_sim_filename = out_found_filename;
    }
    if (out_sim_type == UrdfGeometry::FILE_OBJ)
    {
        std::vector<bt_tinyobj::shape_t> shapes;
        bt_tinyobj::attrib_t attribute;
        STxt err = bt_tinyobj::LoadObj(attribute, shapes, out_found_sim_filename.c_str(), "", fileIO);
        if (!shapes.empty())
        {
            const bt_tinyobj::shape_t& shape = shapes[0];
            AlignedObjectArray<Scalar> vertices;
            AlignedObjectArray<i32> indices;
            for (i32 i = 0; i < attribute.vertices.size(); i++)
            {
                vertices.push_back(attribute.vertices[i]);
            }
            for (i32 i = 0; i < shape.mesh.indices.size(); i++)
            {
                indices.push_back(shape.mesh.indices[i].vertex_index);
            }
            i32 numTris = shape.mesh.indices.size() / 3;
            if (numTris > 0)
            {
                {
                    SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
                    if (softWorld)
                    {
                        psb = SoftBodyHelpers::CreateFromTriMesh(softWorld->getWorldInfo(), &vertices[0], &indices[0], numTris);
                        if (!psb)
                        {
                            printf("Load deformable failed\n");
                            return false;
                        }
                    }
                }
                {
                    DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
                    if (deformWorld)
                    {
                        psb = SoftBodyHelpers::CreateFromTriMesh(deformWorld->getWorldInfo(), &vertices[0], &indices[0], numTris);
                        if (!psb)
                        {
                            printf("Load deformable failed\n");
                            return false;
                        }
                    }
                }
            }
        }
#ifndef SKIP_DEFORMABLE_BODY
        DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
        if (deformWorld && deformable.m_springCoefficients.elastic_stiffness > 0.)
        {
            DeformableLagrangianForce* springForce =
                new DeformableMassSpringForce(deformable.m_springCoefficients.elastic_stiffness,
                                                deformable.m_springCoefficients.damping_stiffness,
                                                !deformable.m_springCoefficients.damp_all_directions,
                                                deformable.m_springCoefficients.bending_stiffness);
            deformWorld->addForce(psb, springForce);
            m_data->m_lf.push_back(springForce);
        }
#endif
    }
    else if (out_sim_type == UrdfGeometry::FILE_VTK)
    {
#ifndef SKIP_DEFORMABLE_BODY
        DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
        if (deformWorld)
        {
            psb = SoftBodyHelpers::CreateFromVtkFile(deformWorld->getWorldInfo(), out_found_sim_filename.c_str());
            if (!psb)
            {
                printf("Load deformable failed\n");
                return false;
            }
            Scalar corotated_mu(0.), corotated_lambda(0.);
            corotated_mu = deformable.m_corotatedCoefficients.mu;
            corotated_lambda = deformable.m_corotatedCoefficients.lambda;
            if (corotated_mu > 0 || corotated_lambda > 0)
            {
                DeformableLagrangianForce* corotatedForce = new DeformableCorotatedForce(corotated_mu, corotated_lambda);
                deformWorld->addForce(psb, corotatedForce);
                m_data->m_lf.push_back(corotatedForce);
            }
            Scalar neohookean_mu, neohookean_lambda, neohookean_damping;
            neohookean_mu = deformable.m_neohookeanCoefficients.mu;
            neohookean_lambda = deformable.m_neohookeanCoefficients.lambda;
            neohookean_damping = deformable.m_neohookeanCoefficients.damping;
            if (neohookean_mu > 0 || neohookean_lambda > 0)
            {
                DeformableLagrangianForce* neohookeanForce = new DeformableNeoHookeanForce(neohookean_mu, neohookean_lambda, neohookean_damping);
                deformWorld->addForce(psb, neohookeanForce);
                m_data->m_lf.push_back(neohookeanForce);
            }

            Scalar spring_elastic_stiffness, spring_damping_stiffness, spring_bending_stiffness;
            spring_elastic_stiffness = deformable.m_springCoefficients.elastic_stiffness;
            spring_damping_stiffness = deformable.m_springCoefficients.damping_stiffness;
            spring_bending_stiffness = deformable.m_springCoefficients.bending_stiffness;
            if (spring_elastic_stiffness > 0.)
            {
                DeformableLagrangianForce* springForce = new DeformableMassSpringForce(spring_elastic_stiffness, spring_damping_stiffness, true, spring_bending_stiffness);
                deformWorld->addForce(psb, springForce);
                m_data->m_lf.push_back(springForce);
            }
        }
#endif
    }
    b3ImportMeshData meshData;

    if (psb != NULL)
    {
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        // load render mesh
        if ((out_found_sim_filename != out_found_filename) || ((out_sim_type == UrdfGeometry::FILE_OBJ)))
        {
            // load render mesh
            if (!m_data->m_useAlternativeDeformableIndexing)
            {

                float rgbaColor[4] = { 1,1,1,1 };

                if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(
                    out_found_filename.c_str(), meshData, fileIO))
                {

                    for (i32 v = 0; v < meshData.m_gfxShape->m_numvertices; v++)
                    {
                        SoftBody::RenderNode n;
                        n.m_x.setVal(
                            meshData.m_gfxShape->m_vertices->at(v).xyzw[0],
                            meshData.m_gfxShape->m_vertices->at(v).xyzw[1],
                            meshData.m_gfxShape->m_vertices->at(v).xyzw[2]);
                        n.m_uv1.setVal(meshData.m_gfxShape->m_vertices->at(v).uv[0],
                            meshData.m_gfxShape->m_vertices->at(v).uv[1],
                            0.);
                        n.m_normal.setVal(meshData.m_gfxShape->m_vertices->at(v).normal[0],
                            meshData.m_gfxShape->m_vertices->at(v).normal[1],
                            meshData.m_gfxShape->m_vertices->at(v).normal[2]);
                        psb->m_renderNodes.push_back(n);
                    }
                    for (i32 f = 0; f < meshData.m_gfxShape->m_numIndices; f += 3)
                    {
                        SoftBody::RenderFace ff;
                        ff.m_n[0] = &psb->m_renderNodes[meshData.m_gfxShape->m_indices->at(f + 0)];
                        ff.m_n[1] = &psb->m_renderNodes[meshData.m_gfxShape->m_indices->at(f + 1)];
                        ff.m_n[2] = &psb->m_renderNodes[meshData.m_gfxShape->m_indices->at(f + 2)];
                        psb->m_renderFaces.push_back(ff);
                    }
                }
            }
            else
            {
                bt_tinyobj::attrib_t attribute;
                std::vector<bt_tinyobj::shape_t> shapes;

                STxt err = bt_tinyobj::LoadObj(attribute, shapes, out_found_filename.c_str(), pathPrefix, m_data->m_pluginManager.getFileIOInterface());

                for (i32 s = 0; s < (i32)shapes.size(); s++)
                {
                    bt_tinyobj::shape_t& shape = shapes[s];
                    i32 faceCount = shape.mesh.indices.size();
                    i32 vertexCount = attribute.vertices.size() / 3;
                    for (i32 v = 0; v < vertexCount; v++)
                    {
                        SoftBody::RenderNode n;
                        n.m_x = Vec3(attribute.vertices[3 * v], attribute.vertices[3 * v + 1], attribute.vertices[3 * v + 2]);
                        psb->m_renderNodes.push_back(n);
                    }
                    for (i32 f = 0; f < faceCount; f += 3)
                    {
                        if (f < 0 && f >= i32(shape.mesh.indices.size()))
                        {
                            continue;
                        }
                        bt_tinyobj::index_t v_0 = shape.mesh.indices[f];
                        bt_tinyobj::index_t v_1 = shape.mesh.indices[f + 1];
                        bt_tinyobj::index_t v_2 = shape.mesh.indices[f + 2];
                        SoftBody::RenderFace ff;
                        ff.m_n[0] = &psb->m_renderNodes[v_0.vertex_index];
                        ff.m_n[1] = &psb->m_renderNodes[v_1.vertex_index];
                        ff.m_n[2] = &psb->m_renderNodes[v_2.vertex_index];
                        psb->m_renderFaces.push_back(ff);
                    }
                }
            }
            if (out_sim_type == UrdfGeometry::FILE_VTK)
            {
                SoftBodyHelpers::interpolateBarycentricWeights(psb);
            }
            else if (out_sim_type == UrdfGeometry::FILE_OBJ)
            {
                SoftBodyHelpers::extrapolateBarycentricWeights(psb);
            }
        }
        else
        {
            psb->m_renderNodes.resize(0);
        }
#endif
#ifndef SKIP_DEFORMABLE_BODY
        DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
        if (deformWorld)
        {

            Vec3 gravity = m_data->m_dynamicsWorld->getGravity();
            DeformableLagrangianForce* gravityForce = new DeformableGravityForce(gravity);
            deformWorld->addForce(psb, gravityForce);
            m_data->m_lf.push_back(gravityForce);
            Scalar collision_hardness = 1;
            psb->m_cfg.kKHR = collision_hardness;
            psb->m_cfg.kCHR = collision_hardness;

            psb->m_cfg.kDF = deformable.m_friction;
            if (deformable.m_springCoefficients.bending_stiffness)
            {
                psb->generateBendingConstraints(deformable.m_springCoefficients.bending_stride);
            }
            SoftBody::Material* pm = psb->appendMaterial();
            pm->m_flags -= SoftBody::fMaterial::DebugDraw;

            // turn on the collision flag for deformable
            // collision between deformable and rigid
            psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
            // turn on face contact for multibodies
            psb->m_cfg.collisions |= SoftBody::fCollision::SDF_MDF;
            /// turn on face contact for rigid body
            psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
            // collion between deformable and deformable and self-collision
            psb->m_cfg.collisions |= SoftBody::fCollision::VF_DD;
            psb->setCollisionFlags(0);
            psb->setTotalMass(deformable.m_mass);
            psb->setSelfCollision(useSelfCollision);
            psb->setSpringStiffness(deformable.m_repulsionStiffness);
            psb->setGravityFactor(deformable.m_gravFactor);
            psb->setCacheBarycenter(deformable.m_cache_barycenter);
            psb->initializeFaceTree();

        }
#endif  //SKIP_DEFORMABLE_BODY
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
        if (softWorld)
        {
            SoftBody::Material* pm = psb->appendMaterial();
            pm->m_kLST = 0.5;
            pm->m_flags -= SoftBody::fMaterial::DebugDraw;
            psb->generateBendingConstraints(2, pm);
            psb->m_cfg.piterations = 20;
            psb->m_cfg.kDF = 0.5;
            //turn on softbody vs softbody collision
            psb->m_cfg.collisions |= SoftBody::fCollision::VF_SS;
            psb->randomizeConstraints();
            psb->setTotalMass(deformable.m_mass, true);
        }
#endif  //SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        psb->scale(Vec3(scale, scale, scale));
        psb->rotate(orn);
        psb->translate(pos);

        psb->getCollisionShape()->setMargin(deformable.m_collisionMargin);
        psb->getCollisionShape()->setUserPointer(psb);
#ifndef SKIP_DEFORMABLE_BODY
        if (deformWorld)
        {
            deformWorld->addSoftBody(psb);
        }
        else
#endif  //SKIP_DEFORMABLE_BODY
        {
            SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
            if (softWorld)
            {
                softWorld->addSoftBody(psb);
            }
        }

        *bodyUniqueId = m_data->m_bodyHandles.allocHandle();
        InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(*bodyUniqueId);
        bodyHandle->m_softBody = psb;
        psb->setUserIndex2(*bodyUniqueId);

        b3VisualShapeData visualShape;

        visualShape.m_objectUniqueId = *bodyUniqueId;
        visualShape.m_linkIndex = -1;
        visualShape.m_visualGeometryType = URDF_GEOM_MESH;
        //dimensions just contains the scale
        visualShape.m_dimensions[0] = 1;
        visualShape.m_dimensions[1] = 1;
        visualShape.m_dimensions[2] = 1;
        //filename
        strncpy(visualShape.m_meshAssetFileName, relativeFileName, VISUAL_SHAPE_MAX_PATH_LEN);
        visualShape.m_meshAssetFileName[VISUAL_SHAPE_MAX_PATH_LEN - 1] = 0;
        //position and orientation
        visualShape.m_localVisualFrame[0] = 0;
        visualShape.m_localVisualFrame[1] = 0;
        visualShape.m_localVisualFrame[2] = 0;
        visualShape.m_localVisualFrame[3] = 0;
        visualShape.m_localVisualFrame[4] = 0;
        visualShape.m_localVisualFrame[5] = 0;
        visualShape.m_localVisualFrame[6] = 1;
        //color and ids to be set by the renderer
        visualShape.m_rgbaColor[0] = 1;
        visualShape.m_rgbaColor[1] = 1;
        visualShape.m_rgbaColor[2] = 1;
        visualShape.m_rgbaColor[3] = 1;
        visualShape.m_tinyRendererTextureId = -1;
        visualShape.m_textureUniqueId = -1;
        visualShape.m_openglTextureId = -1;

        if (meshData.m_gfxShape)
        {
            i32 texUid1 = -1;
            if (meshData.m_textureHeight > 0 && meshData.m_textureWidth > 0 && meshData.m_textureImage1)
            {
                texUid1 = m_data->m_guiHelper->registerTexture(meshData.m_textureImage1, meshData.m_textureWidth, meshData.m_textureHeight);
            }
            visualShape.m_openglTextureId = texUid1;
            i32 shapeUid1 = m_data->m_guiHelper->registerGraphicsShape(&meshData.m_gfxShape->m_vertices->at(0).xyzw[0], meshData.m_gfxShape->m_numvertices, &meshData.m_gfxShape->m_indices->at(0), meshData.m_gfxShape->m_numIndices, D3_GL_TRIANGLES, texUid1);
            psb->getCollisionShape()->setUserIndex(shapeUid1);
            float position[4] = { 0,0,0,1 };
            float orientation[4] = { 0,0,0,1 };
            float color[4] = { 1,1,1,1 };
            float scaling[4] = { 1,1,1,1 };
            i32 instanceUid = m_data->m_guiHelper->registerGraphicsInstance(shapeUid1, position, orientation, color, scaling);
            psb->setUserIndex(instanceUid);

            if (m_data->m_enableTinyRenderer)
            {
                i32 texUid2 = m_data->m_pluginManager.getRenderInterface()->registerTexture(meshData.m_textureImage1, meshData.m_textureWidth, meshData.m_textureHeight);
                visualShape.m_tinyRendererTextureId = texUid2;
                i32 linkIndex = -1;
                i32 softBodyGraphicsShapeUid = m_data->m_pluginManager.getRenderInterface()->registerShapeAndInstance(
                    visualShape,
                    &meshData.m_gfxShape->m_vertices->at(0).xyzw[0],
                    meshData.m_gfxShape->m_numvertices,
                    &meshData.m_gfxShape->m_indices->at(0),
                    meshData.m_gfxShape->m_numIndices,
                    D3_GL_TRIANGLES,
                    texUid2,
                    psb->getBroadphaseHandle()->getUid(),
                    *bodyUniqueId,
                    linkIndex);

                psb->setUserIndex3(softBodyGraphicsShapeUid);
            }
            delete meshData.m_gfxShape;
            meshData.m_gfxShape = 0;
        }
        else
        {
            //m_data->m_guiHelper->createCollisionShapeGraphicsObject(psb->getCollisionShape());

            AlignedObjectArray<GLInstanceVertex> gfxVertices;
            AlignedObjectArray<i32> indices;
            i32 strideInBytes = 9 * sizeof(float);
            gfxVertices.resize(psb->m_faces.size() * 3);
            for (i32 i = 0; i < psb->m_faces.size(); i++)  // Foreach face
            {
                for (i32 k = 0; k < 3; k++)  // Foreach vertex on a face
                {
                    i32 currentIndex = i * 3 + k;
                    for (i32 j = 0; j < 3; j++)
                    {
                        gfxVertices[currentIndex].xyzw[j] = psb->m_faces[i].m_n[k]->m_x[j];
                    }
                    for (i32 j = 0; j < 3; j++)
                    {
                        gfxVertices[currentIndex].normal[j] = psb->m_faces[i].m_n[k]->m_n[j];
                    }
                    for (i32 j = 0; j < 2; j++)
                    {
                        gfxVertices[currentIndex].uv[j] = Fabs(Fabs(10. * psb->m_faces[i].m_n[k]->m_x[j]));
                    }
                    indices.push_back(currentIndex);
                }
            }
            if (gfxVertices.size() && indices.size())
            {
                i32 red = 173;
                i32 green = 199;
                i32 blue = 255;

                i32 texWidth = 256;
                i32 texHeight = 256;
                AlignedObjectArray<u8> texels;
                texels.resize(texWidth* texHeight * 3);
                for (i32 i = 0; i < texWidth * texHeight * 3; i++)
                    texels[i] = 255;
                for (i32 i = 0; i < texWidth; i++)
                {
                    for (i32 j = 0; j < texHeight; j++)
                    {
                        i32 a = i < texWidth / 2 ? 1 : 0;
                        i32 b = j < texWidth / 2 ? 1 : 0;

                        if (a == b)
                        {
                            texels[(i + j * texWidth) * 3 + 0] = red;
                            texels[(i + j * texWidth) * 3 + 1] = green;
                            texels[(i + j * texWidth) * 3 + 2] = blue;
                        }
                    }
                }

                i32 texId = m_data->m_guiHelper->registerTexture(&texels[0], texWidth, texHeight);
                visualShape.m_openglTextureId = texId;
                i32 shapeId = m_data->m_guiHelper->registerGraphicsShape(&gfxVertices[0].xyzw[0], gfxVertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES, texId);
                drx3DAssert(shapeId >= 0);
                psb->getCollisionShape()->setUserIndex(shapeId);
                if (m_data->m_enableTinyRenderer)
                {

                    i32 texUid2 = m_data->m_pluginManager.getRenderInterface()->registerTexture(&texels[0], texWidth, texHeight);
                    visualShape.m_tinyRendererTextureId = texUid2;
                    i32 linkIndex = -1;
                    i32 softBodyGraphicsShapeUid = m_data->m_pluginManager.getRenderInterface()->registerShapeAndInstance(
                        visualShape,
                        &gfxVertices[0].xyzw[0], gfxVertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES, texUid2,
                        psb->getBroadphaseHandle()->getUid(),
                        *bodyUniqueId,
                        linkIndex);
                    psb->setUserIndex3(softBodyGraphicsShapeUid);
                }
            }
        }



        AlignedObjectArray<Vec3> vertices;
        AlignedObjectArray<Vec3> normals;
        if (psb->m_renderNodes.size() == 0)
        {
            psb->m_renderNodes.resize(psb->m_faces.size()*3);
            vertices.resize(psb->m_faces.size() * 3);
            normals.resize(psb->m_faces.size() * 3);

            for (i32 i = 0; i < psb->m_faces.size(); i++)  // Foreach face
            {

                for (i32 k = 0; k < 3; k++)  // Foreach vertex on a face
                {
                    i32 currentIndex = i * 3 + k;
                    for (i32 j = 0; j < 3; j++)
                    {
                        psb->m_renderNodes[currentIndex].m_x[j] = psb->m_faces[i].m_n[k]->m_x[j];
                    }
                    for (i32 j = 0; j < 3; j++)
                    {
                        psb->m_renderNodes[currentIndex].m_normal[j] = psb->m_faces[i].m_n[k]->m_n[j];
                    }
                    for (i32 j = 0; j < 2; j++)
                    {
                        psb->m_renderNodes[currentIndex].m_uv1[j] = Fabs(10*psb->m_faces[i].m_n[k]->m_x[j]);
                    }
                    psb->m_renderNodes[currentIndex].m_uv1[2] = 0;
                    vertices[currentIndex] = psb->m_faces[i].m_n[k]->m_x;
                    normals[currentIndex] = psb->m_faces[i].m_n[k]->m_n;
                }
            }
            SoftBodyHelpers::extrapolateBarycentricWeights(psb);
        }
        else
        {
            vertices.resize(psb->m_renderNodes.size());
            normals.resize(psb->m_renderNodes.size());
            for (i32 i = 0; i < psb->m_renderNodes.size(); i++)  // Foreach face
            {
                vertices[i] = psb->m_renderNodes[i].m_x;
                normals[i] = psb->m_renderNodes[i].m_normal;
            }
        }
        m_data->m_pluginManager.getRenderInterface()->updateShape(psb->getUserIndex3(), &vertices[0], vertices.size(), &normals[0], normals.size());

        if (!deformable.m_name.empty())
        {
            bodyHandle->m_bodyName = deformable.m_name;
        }
        else
        {
            i32 pos = strlen(relativeFileName) - 1;
            while (pos >= 0 && relativeFileName[pos] != '/')
            {
                pos--;
            }
            Assert(strlen(relativeFileName) - pos - 5 > 0);
            STxt object_name(STxt(relativeFileName).substr(pos + 1, strlen(relativeFileName) - 5 - pos));
            bodyHandle->m_bodyName = object_name;
        }
        b3Notification notification;
        notification.m_notificationType = BODY_ADDED;
        notification.m_bodyArgs.m_bodyUniqueId = *bodyUniqueId;
        m_data->m_pluginManager.addNotification(notification);
    }
#endif
    return true;
}

bool PhysicsServerCommandProcessor::processReducedDeformable(const UrdfReducedDeformable& reduced_deformable, const Vec3& pos, const Quat& orn, i32* bodyUniqueId, tuk bufferServerToClient, i32 bufferSizeInBytes, Scalar scale, bool useSelfCollision)
{
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    ReducedDeformableBody* rsb = nullptr;
    CommonFileIOInterface* fileIO(m_data->m_pluginManager.getFileIOInterface());
    char relativeFileName[1024];
    char pathPrefix[1024];
    pathPrefix[0] = 0;
    if (fileIO->findResourcePath(reduced_deformable.m_visualFileName.c_str(), relativeFileName, 1024))
    {
        b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);
    }
    const STxt& error_message_prefix = "";
    STxt out_found_filename, out_found_sim_filename;
    i32 out_type(0), out_sim_type(0);

    bool foundFile = UrdfFindMeshFile(fileIO, pathPrefix, relativeFileName, error_message_prefix, &out_found_filename, &out_type);
    if (!reduced_deformable.m_simFileName.empty())
    {
        bool foundSimMesh = UrdfFindMeshFile(fileIO, pathPrefix, reduced_deformable.m_simFileName, error_message_prefix, &out_found_sim_filename, &out_sim_type);
    }
    else
    {
        out_sim_type = out_type;
        out_found_sim_filename = out_found_filename;
    }

    if (out_sim_type == UrdfGeometry::FILE_OBJ)
    {
        printf("Obj file is currently unsupported\n");
        return false;
    }
    else if (out_sim_type == UrdfGeometry::FILE_VTK)
    {
#ifndef SKIP_DEFORMABLE_BODY
        DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
        if (deformWorld)
        {
            rsb = ReducedDeformableBodyHelpers::createFromVtkFile(deformWorld->getWorldInfo(), out_found_sim_filename.c_str());
            if (!rsb)
            {
                printf("Load reduced deformable failed\n");
                return false;
            }

            // load modes, reduced stiffness matrix
            rsb->setReducedModes(reduced_deformable.m_numModes, rsb->m_nodes.size());
            rsb->setStiffnessScale(reduced_deformable.m_stiffnessScale);
            rsb->setDamping(0, reduced_deformable.m_damping); // damping alpha is set to 0 by default
            ReducedDeformableBodyHelpers::readReducedDeformableInfoFromFiles(rsb, pathPrefix);
            // set total mass
            rsb->setTotalMass(reduced_deformable.m_mass);
        }
#endif
    }
    b3ImportMeshData meshData;

    if (rsb != NULL)
    {
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        // load render mesh
        if ((out_found_sim_filename != out_found_filename) || ((out_sim_type == UrdfGeometry::FILE_OBJ)))
        {
            // load render mesh
            if (!m_data->m_useAlternativeDeformableIndexing)
            {

                float rgbaColor[4] = { 1,1,1,1 };

                if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(
                    out_found_filename.c_str(), meshData, fileIO))
                {

                    for (i32 v = 0; v < meshData.m_gfxShape->m_numvertices; v++)
                    {
                        SoftBody::RenderNode n;
                        n.m_x.setVal(
                            meshData.m_gfxShape->m_vertices->at(v).xyzw[0],
                            meshData.m_gfxShape->m_vertices->at(v).xyzw[1],
                            meshData.m_gfxShape->m_vertices->at(v).xyzw[2]);
                        n.m_uv1.setVal(meshData.m_gfxShape->m_vertices->at(v).uv[0],
                            meshData.m_gfxShape->m_vertices->at(v).uv[1],
                            0.);
                        n.m_normal.setVal(meshData.m_gfxShape->m_vertices->at(v).normal[0],
                            meshData.m_gfxShape->m_vertices->at(v).normal[1],
                            meshData.m_gfxShape->m_vertices->at(v).normal[2]);
                        rsb->m_renderNodes.push_back(n);
                    }
                    for (i32 f = 0; f < meshData.m_gfxShape->m_numIndices; f += 3)
                    {
                        SoftBody::RenderFace ff;
                        ff.m_n[0] = &rsb->m_renderNodes[meshData.m_gfxShape->m_indices->at(f + 0)];
                        ff.m_n[1] = &rsb->m_renderNodes[meshData.m_gfxShape->m_indices->at(f + 1)];
                        ff.m_n[2] = &rsb->m_renderNodes[meshData.m_gfxShape->m_indices->at(f + 2)];
                        rsb->m_renderFaces.push_back(ff);
                    }
                }
            }
            else
            {
                bt_tinyobj::attrib_t attribute;
                std::vector<bt_tinyobj::shape_t> shapes;

                STxt err = bt_tinyobj::LoadObj(attribute, shapes, out_found_filename.c_str(), pathPrefix, m_data->m_pluginManager.getFileIOInterface());

                for (i32 s = 0; s < (i32)shapes.size(); s++)
                {
                    bt_tinyobj::shape_t& shape = shapes[s];
                    i32 faceCount = shape.mesh.indices.size();
                    i32 vertexCount = attribute.vertices.size() / 3;
                    for (i32 v = 0; v < vertexCount; v++)
                    {
                        SoftBody::RenderNode n;
                        n.m_x = Vec3(attribute.vertices[3 * v], attribute.vertices[3 * v + 1], attribute.vertices[3 * v + 2]);
                        rsb->m_renderNodes.push_back(n);
                    }
                    for (i32 f = 0; f < faceCount; f += 3)
                    {
                        if (f < 0 && f >= i32(shape.mesh.indices.size()))
                        {
                            continue;
                        }
                        bt_tinyobj::index_t v_0 = shape.mesh.indices[f];
                        bt_tinyobj::index_t v_1 = shape.mesh.indices[f + 1];
                        bt_tinyobj::index_t v_2 = shape.mesh.indices[f + 2];
                        SoftBody::RenderFace ff;
                        ff.m_n[0] = &rsb->m_renderNodes[v_0.vertex_index];
                        ff.m_n[1] = &rsb->m_renderNodes[v_1.vertex_index];
                        ff.m_n[2] = &rsb->m_renderNodes[v_2.vertex_index];
                        rsb->m_renderFaces.push_back(ff);
                    }
                }
            }
            if (out_sim_type == UrdfGeometry::FILE_VTK)
            {
                SoftBodyHelpers::interpolateBarycentricWeights(rsb);
            }
            else if (out_sim_type == UrdfGeometry::FILE_OBJ)
            {
                SoftBodyHelpers::extrapolateBarycentricWeights(rsb);
            }
        }
        else
        {
            rsb->m_renderNodes.resize(0);
        }
#endif
#ifndef SKIP_DEFORMABLE_BODY
        DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
        if (deformWorld)
        {
            Scalar collision_hardness = 1;
            rsb->m_cfg.kKHR = collision_hardness;
            rsb->m_cfg.kCHR = collision_hardness;

            rsb->m_cfg.kDF = reduced_deformable.m_friction;
            SoftBody::Material* pm = rsb->appendMaterial();
            pm->m_flags -= SoftBody::fMaterial::DebugDraw;

            // turn on the collision flag for deformable
            // collision between deformable and rigid
            rsb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
            /// turn on node contact for rigid body
            rsb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
            // turn on face contact for multibodies
            // rsb->m_cfg.collisions |= SoftBody::fCollision::SDF_MDF;
            // collion between deformable and deformable and self-collision
            // rsb->m_cfg.collisions |= SoftBody::fCollision::VF_DD;
            rsb->setCollisionFlags(0);
            rsb->setSelfCollision(useSelfCollision);
            rsb->initializeFaceTree();
        }
#endif  //SKIP_DEFORMABLE_BODY
// #ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
//      SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
//      if (softWorld)
//      {
//          SoftBody::Material* pm = rsb->appendMaterial();
//          pm->m_kLST = 0.5;
//          pm->m_flags -= SoftBody::fMaterial::DebugDraw;
//          rsb->generateBendingConstraints(2, pm);
//          rsb->m_cfg.piterations = 20;
//          rsb->m_cfg.kDF = 0.5;
//          //turn on softbody vs softbody collision
//          rsb->m_cfg.collisions |= SoftBody::fCollision::VF_SS;
//          rsb->randomizeConstraints();
//          rsb->setTotalMass(reduced_deformable.m_mass, true);
//      }
// #endif  //SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        rsb->scale(Vec3(scale, scale, scale));
        Transform2 init_transform;
        init_transform.setOrigin(pos);
        init_transform.setRotation(orn);
        rsb->transform(init_transform);

        rsb->getCollisionShape()->setMargin(reduced_deformable.m_collisionMargin);
        rsb->getCollisionShape()->setUserPointer(rsb);
#ifndef SKIP_DEFORMABLE_BODY
        if (deformWorld)
        {
            deformWorld->addSoftBody(rsb);
            deformWorld->getSolverInfo().m_deformable_erp = reduced_deformable.m_erp;
            deformWorld->getSolverInfo().m_deformable_cfm = reduced_deformable.m_cfm;
            deformWorld->getSolverInfo().m_friction = reduced_deformable.m_friction;
            deformWorld->getSolverInfo().m_splitImpulse = false;
            deformWorld->setImplicit(false);
            deformWorld->setLineSearch(false);
            deformWorld->setUseProjection(false);
        }
        else
#endif  //SKIP_DEFORMABLE_BODY
        {
            SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
            if (softWorld)
            {
                softWorld->addSoftBody(rsb);
            }
        }

        *bodyUniqueId = m_data->m_bodyHandles.allocHandle();
        InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(*bodyUniqueId);
        bodyHandle->m_softBody = rsb;
        rsb->setUserIndex2(*bodyUniqueId);

        b3VisualShapeData visualShape;

        visualShape.m_objectUniqueId = *bodyUniqueId;
        visualShape.m_linkIndex = -1;
        visualShape.m_visualGeometryType = URDF_GEOM_MESH;
        //dimensions just contains the scale
        visualShape.m_dimensions[0] = 1;
        visualShape.m_dimensions[1] = 1;
        visualShape.m_dimensions[2] = 1;
        //filename
        strncpy(visualShape.m_meshAssetFileName, relativeFileName, VISUAL_SHAPE_MAX_PATH_LEN);
        visualShape.m_meshAssetFileName[VISUAL_SHAPE_MAX_PATH_LEN - 1] = 0;
        //position and orientation
        visualShape.m_localVisualFrame[0] = 0;
        visualShape.m_localVisualFrame[1] = 0;
        visualShape.m_localVisualFrame[2] = 0;
        visualShape.m_localVisualFrame[3] = 0;
        visualShape.m_localVisualFrame[4] = 0;
        visualShape.m_localVisualFrame[5] = 0;
        visualShape.m_localVisualFrame[6] = 1;
        //color and ids to be set by the renderer
        visualShape.m_rgbaColor[0] = 1;
        visualShape.m_rgbaColor[1] = 1;
        visualShape.m_rgbaColor[2] = 1;
        visualShape.m_rgbaColor[3] = 1;
        visualShape.m_tinyRendererTextureId = -1;
        visualShape.m_textureUniqueId = -1;
        visualShape.m_openglTextureId = -1;

        if (meshData.m_gfxShape)
        {
            i32 texUid1 = -1;
            if (meshData.m_textureHeight > 0 && meshData.m_textureWidth > 0 && meshData.m_textureImage1)
            {
                texUid1 = m_data->m_guiHelper->registerTexture(meshData.m_textureImage1, meshData.m_textureWidth, meshData.m_textureHeight);
            }
            visualShape.m_openglTextureId = texUid1;
            i32 shapeUid1 = m_data->m_guiHelper->registerGraphicsShape(&meshData.m_gfxShape->m_vertices->at(0).xyzw[0], meshData.m_gfxShape->m_numvertices, &meshData.m_gfxShape->m_indices->at(0), meshData.m_gfxShape->m_numIndices, D3_GL_TRIANGLES, texUid1);
            rsb->getCollisionShape()->setUserIndex(shapeUid1);
            float position[4] = { 0,0,0,1 };
            float orientation[4] = { 0,0,0,1 };
            float color[4] = { 1,1,1,1 };
            float scaling[4] = { 1,1,1,1 };
            i32 instanceUid = m_data->m_guiHelper->registerGraphicsInstance(shapeUid1, position, orientation, color, scaling);
            rsb->setUserIndex(instanceUid);

            if (m_data->m_enableTinyRenderer)
            {
                i32 texUid2 = m_data->m_pluginManager.getRenderInterface()->registerTexture(meshData.m_textureImage1, meshData.m_textureWidth, meshData.m_textureHeight);
                visualShape.m_tinyRendererTextureId = texUid2;
                i32 linkIndex = -1;
                i32 softBodyGraphicsShapeUid = m_data->m_pluginManager.getRenderInterface()->registerShapeAndInstance(
                    visualShape,
                    &meshData.m_gfxShape->m_vertices->at(0).xyzw[0],
                    meshData.m_gfxShape->m_numvertices,
                    &meshData.m_gfxShape->m_indices->at(0),
                    meshData.m_gfxShape->m_numIndices,
                    D3_GL_TRIANGLES,
                    texUid2,
                    rsb->getBroadphaseHandle()->getUid(),
                    *bodyUniqueId,
                    linkIndex);

                rsb->setUserIndex3(softBodyGraphicsShapeUid);
            }
            delete meshData.m_gfxShape;
            meshData.m_gfxShape = 0;
        }
        else
        {
            //m_data->m_guiHelper->createCollisionShapeGraphicsObject(psb->getCollisionShape());

            AlignedObjectArray<GLInstanceVertex> gfxVertices;
            AlignedObjectArray<i32> indices;
            i32 strideInBytes = 9 * sizeof(float);
            gfxVertices.resize(rsb->m_faces.size() * 3);
            for (i32 i = 0; i < rsb->m_faces.size(); i++)  // Foreach face
            {
                for (i32 k = 0; k < 3; k++)  // Foreach vertex on a face
                {
                    i32 currentIndex = i * 3 + k;
                    for (i32 j = 0; j < 3; j++)
                    {
                        gfxVertices[currentIndex].xyzw[j] = rsb->m_faces[i].m_n[k]->m_x[j];
                    }
                    for (i32 j = 0; j < 3; j++)
                    {
                        gfxVertices[currentIndex].normal[j] = rsb->m_faces[i].m_n[k]->m_n[j];
                    }
                    for (i32 j = 0; j < 2; j++)
                    {
                        gfxVertices[currentIndex].uv[j] = Fabs(Fabs(10. * rsb->m_faces[i].m_n[k]->m_x[j]));
                    }
                    indices.push_back(currentIndex);
                }
            }
            if (gfxVertices.size() && indices.size())
            {
                i32 red = 173;
                i32 green = 199;
                i32 blue = 255;

                i32 texWidth = 256;
                i32 texHeight = 256;
                AlignedObjectArray<u8> texels;
                texels.resize(texWidth* texHeight * 3);
                for (i32 i = 0; i < texWidth * texHeight * 3; i++)
                    texels[i] = 255;
                for (i32 i = 0; i < texWidth; i++)
                {
                    for (i32 j = 0; j < texHeight; j++)
                    {
                        i32 a = i < texWidth / 2 ? 1 : 0;
                        i32 b = j < texWidth / 2 ? 1 : 0;

                        if (a == b)
                        {
                            texels[(i + j * texWidth) * 3 + 0] = red;
                            texels[(i + j * texWidth) * 3 + 1] = green;
                            texels[(i + j * texWidth) * 3 + 2] = blue;
                        }
                    }
                }

                i32 texId = m_data->m_guiHelper->registerTexture(&texels[0], texWidth, texHeight);
                visualShape.m_openglTextureId = texId;
                i32 shapeId = m_data->m_guiHelper->registerGraphicsShape(&gfxVertices[0].xyzw[0], gfxVertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES, texId);
                drx3DAssert(shapeId >= 0);
                rsb->getCollisionShape()->setUserIndex(shapeId);
                if (m_data->m_enableTinyRenderer)
                {

                    i32 texUid2 = m_data->m_pluginManager.getRenderInterface()->registerTexture(&texels[0], texWidth, texHeight);
                    visualShape.m_tinyRendererTextureId = texUid2;
                    i32 linkIndex = -1;
                    i32 softBodyGraphicsShapeUid = m_data->m_pluginManager.getRenderInterface()->registerShapeAndInstance(
                        visualShape,
                        &gfxVertices[0].xyzw[0], gfxVertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES, texUid2,
                        rsb->getBroadphaseHandle()->getUid(),
                        *bodyUniqueId,
                        linkIndex);
                    rsb->setUserIndex3(softBodyGraphicsShapeUid);
                }
            }
        }



        AlignedObjectArray<Vec3> vertices;
        AlignedObjectArray<Vec3> normals;
        if (rsb->m_renderNodes.size() == 0)
        {
            rsb->m_renderNodes.resize(rsb->m_faces.size()*3);
            vertices.resize(rsb->m_faces.size() * 3);
            normals.resize(rsb->m_faces.size() * 3);

            for (i32 i = 0; i < rsb->m_faces.size(); i++)  // Foreach face
            {

                for (i32 k = 0; k < 3; k++)  // Foreach vertex on a face
                {
                    i32 currentIndex = i * 3 + k;
                    for (i32 j = 0; j < 3; j++)
                    {
                        rsb->m_renderNodes[currentIndex].m_x[j] = rsb->m_faces[i].m_n[k]->m_x[j];
                    }
                    for (i32 j = 0; j < 3; j++)
                    {
                        rsb->m_renderNodes[currentIndex].m_normal[j] = rsb->m_faces[i].m_n[k]->m_n[j];
                    }
                    for (i32 j = 0; j < 2; j++)
                    {
                        rsb->m_renderNodes[currentIndex].m_uv1[j] = Fabs(10*rsb->m_faces[i].m_n[k]->m_x[j]);
                    }
                    rsb->m_renderNodes[currentIndex].m_uv1[2] = 0;
                    vertices[currentIndex] = rsb->m_faces[i].m_n[k]->m_x;
                    normals[currentIndex] = rsb->m_faces[i].m_n[k]->m_n;
                }
            }
            SoftBodyHelpers::extrapolateBarycentricWeights(rsb);
        }
        else
        {
            vertices.resize(rsb->m_renderNodes.size());
            normals.resize(rsb->m_renderNodes.size());
            for (i32 i = 0; i < rsb->m_renderNodes.size(); i++)  // Foreach face
            {
                vertices[i] = rsb->m_renderNodes[i].m_x;
                normals[i] = rsb->m_renderNodes[i].m_normal;
            }
        }
        m_data->m_pluginManager.getRenderInterface()->updateShape(rsb->getUserIndex3(), &vertices[0], vertices.size(), &normals[0], normals.size());

        if (!reduced_deformable.m_name.empty())
        {
            bodyHandle->m_bodyName = reduced_deformable.m_name;
        }
        else
        {
            i32 pos = strlen(relativeFileName) - 1;
            while (pos >= 0 && relativeFileName[pos] != '/')
            {
                pos--;
            }
            Assert(strlen(relativeFileName) - pos - 5 > 0);
            STxt object_name(STxt(relativeFileName).substr(pos + 1, strlen(relativeFileName) - 5 - pos));
            bodyHandle->m_bodyName = object_name;
        }
        b3Notification notification;
        notification.m_notificationType = BODY_ADDED;
        notification.m_bodyArgs.m_bodyUniqueId = *bodyUniqueId;
        m_data->m_pluginManager.addNotification(notification);
    }
#endif
    return true;
}

bool PhysicsServerCommandProcessor::processLoadSoftBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    serverStatusOut.m_type = CMD_LOAD_SOFT_BODY_FAILED;
    bool hasStatus = true;
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    UrdfDeformable deformable;

    constructUrdfDeformable(clientCmd, deformable, m_data->m_verboseOutput);
    // const LoadSoftBodyArgs& loadSoftBodyArgs = clientCmd.m_loadSoftBodyArguments;
    Vec3 initialPos(0, 0, 0);
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_INITIAL_POSITION)
    {
        initialPos[0] = clientCmd.m_loadSoftBodyArguments.m_initialPosition[0];
        initialPos[1] = clientCmd.m_loadSoftBodyArguments.m_initialPosition[1];
        initialPos[2] = clientCmd.m_loadSoftBodyArguments.m_initialPosition[2];
    }
    Quat initialOrn(0, 0, 0, 1);
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_INITIAL_ORIENTATION)
    {
        initialOrn[0] = clientCmd.m_loadSoftBodyArguments.m_initialOrientation[0];
        initialOrn[1] = clientCmd.m_loadSoftBodyArguments.m_initialOrientation[1];
        initialOrn[2] = clientCmd.m_loadSoftBodyArguments.m_initialOrientation[2];
        initialOrn[3] = clientCmd.m_loadSoftBodyArguments.m_initialOrientation[3];
    }

    double scale = 1;
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_UPDATE_SCALE)
    {
        scale = clientCmd.m_loadSoftBodyArguments.m_scale;
    }
    bool use_self_collision = false;
    if (clientCmd.m_updateFlags & LOAD_SOFT_BODY_USE_SELF_COLLISION)
    {
        use_self_collision = clientCmd.m_loadSoftBodyArguments.m_useSelfCollision;
    }

    i32 bodyUniqueId = -1;
    bool completedOk = processDeformable(deformable, initialPos, initialOrn, &bodyUniqueId, bufferServerToClient, bufferSizeInBytes, scale, use_self_collision);
    if (completedOk && bodyUniqueId >= 0)
    {
        m_data->m_guiHelper->autogenerateGraphicsObjects(m_data->m_dynamicsWorld);
        serverStatusOut.m_type = CMD_LOAD_SOFT_BODY_COMPLETED;

        i32 streamSizeInBytes = createBodyInfoStream(bodyUniqueId, bufferServerToClient, bufferSizeInBytes);
        serverStatusOut.m_numDataStreamBytes = streamSizeInBytes;

#ifdef ENABLE_LINK_MAPPER
        if (m_data->m_urdfLinkNameMapper.size())
        {
            serverStatusOut.m_numDataStreamBytes = m_data->m_urdfLinkNameMapper.at(m_data->m_urdfLinkNameMapper.size() - 1)->m_memSerializer->getCurrentBufferSize();
        }
#endif
        serverStatusOut.m_dataStreamArguments.m_bodyUniqueId = bodyUniqueId;
        InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
        strcpy(serverStatusOut.m_dataStreamArguments.m_bodyName, body->m_bodyName.c_str());
        serverStatusOut.m_loadSoftBodyResultArguments.m_objectUniqueId = bodyUniqueId;
    }
#endif
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCreateSensorCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_CREATE_SENSOR");

    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("Processed CMD_CREATE_SENSOR");
    }
    i32 bodyUniqueId = clientCmd.m_createSensorArguments.m_bodyUniqueId;
    InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
    if (body && body->m_multiBody)
    {
        MultiBody* mb = body->m_multiBody;
        Assert(mb);
        for (i32 i = 0; i < clientCmd.m_createSensorArguments.m_numJointSensorChanges; i++)
        {
            i32 jointIndex = clientCmd.m_createSensorArguments.m_jointIndex[i];
            if (clientCmd.m_createSensorArguments.m_enableJointForceSensor[i])
            {
                if (mb->getLink(jointIndex).m_jointFeedback)
                {
                    drx3DWarning("CMD_CREATE_SENSOR: sensor for joint [%d] already enabled", jointIndex);
                }
                else
                {
                    MultiBodyJointFeedback* fb = new MultiBodyJointFeedback();
                    fb->m_reactionForces.setZero();
                    mb->getLink(jointIndex).m_jointFeedback = fb;
                    m_data->m_multiBodyJointFeedbacks.push_back(fb);
                };
            }
            else
            {
                if (mb->getLink(jointIndex).m_jointFeedback)
                {
                    m_data->m_multiBodyJointFeedbacks.remove(mb->getLink(jointIndex).m_jointFeedback);
                    delete mb->getLink(jointIndex).m_jointFeedback;
                    mb->getLink(jointIndex).m_jointFeedback = 0;
                }
                else
                {
                    drx3DWarning("CMD_CREATE_SENSOR: cannot perform sensor removal request, no sensor on joint [%d]", jointIndex);
                };
            }
        }
    }
    else
    {
        drx3DWarning("No btMultiBody in the world. RigidBody/TypedConstraint sensor not hooked up yet");
    }

#if 0
    //todo(erwincoumans) here is some sample code to hook up a force/torque sensor for btTypedConstraint/RigidBody
    /*
        for (i32 i=0;i<m_data->m_dynamicsWorld->getNumConstraints();i++)
        {
        btTypedConstraint* c = m_data->m_dynamicsWorld->getConstraint(i);
        JointFeedback* fb = new JointFeedback();
        m_data->m_jointFeedbacks.push_back(fb);
        c->setJointFeedback(fb);


        }
        */
#endif

    serverStatusOut.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processProfileTimingCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    {
        if (clientCmd.m_profile.m_type == 0)
        {
            tuk* eventNamePtr = m_data->m_profileEvents[clientCmd.m_profile.m_name];
            tuk eventName = 0;
            if (eventNamePtr)
            {
                eventName = *eventNamePtr;
            }
            else
            {
                i32 len = strlen(clientCmd.m_profile.m_name);
                eventName = new char[len + 1];
                strcpy(eventName, clientCmd.m_profile.m_name);
                eventName[len] = 0;
                m_data->m_profileEvents.insert(eventName, eventName);
            }

            b3EnterProfileZone(eventName);
        }
        if (clientCmd.m_profile.m_type == 1)
        {
            b3LeaveProfileZone();
        }
    }

    serverStatusOut.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    hasStatus = true;
    return hasStatus;
}

void setDefaultRootWorldAABB(SharedMemoryStatus& serverCmd)
{
    serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[0] = 0;
    serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[1] = 0;
    serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[2] = 0;

    serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[0] = -1;
    serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[1] = -1;
    serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[2] = -1;
}

bool PhysicsServerCommandProcessor::processRequestCollisionInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverStatusOut.m_type = CMD_REQUEST_COLLISION_INFO_FAILED;
    hasStatus = true;
    i32 bodyUniqueId = clientCmd.m_requestCollisionInfoArgs.m_bodyUniqueId;
    InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);

    if (body && body->m_multiBody)
    {
        MultiBody* mb = body->m_multiBody;
        serverStatusOut.m_type = CMD_REQUEST_COLLISION_INFO_COMPLETED;
        serverCmd.m_sendCollisionInfoArgs.m_numLinks = body->m_multiBody->getNumLinks();
        setDefaultRootWorldAABB(serverCmd);

        if (body->m_multiBody->getBaseCollider())
        {
            Transform2 tr;
            tr.setOrigin(mb->getBasePos());
            tr.setRotation(mb->getWorldToBaseRot().inverse());

            Vec3 aabbMin, aabbMax;
            body->m_multiBody->getBaseCollider()->getCollisionShape()->getAabb(tr, aabbMin, aabbMax);
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[0] = aabbMin[0];
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[1] = aabbMin[1];
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[2] = aabbMin[2];

            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[0] = aabbMax[0];
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[1] = aabbMax[1];
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[2] = aabbMax[2];
        }
        for (i32 l = 0; l < mb->getNumLinks(); l++)
        {
            serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMin[3 * l + 0] = 0;
            serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMin[3 * l + 1] = 0;
            serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMin[3 * l + 2] = 0;

            serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMax[3 * l + 0] = -1;
            serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMax[3 * l + 1] = -1;
            serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMax[3 * l + 2] = -1;

            if (body->m_multiBody->getLink(l).m_collider)
            {
                Vec3 aabbMin, aabbMax;
                body->m_multiBody->getLinkCollider(l)->getCollisionShape()->getAabb(mb->getLink(l).m_cachedWorldTransform, aabbMin, aabbMax);

                serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMin[3 * l + 0] = aabbMin[0];
                serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMin[3 * l + 1] = aabbMin[1];
                serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMin[3 * l + 2] = aabbMin[2];
                serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMax[3 * l + 0] = aabbMax[0];
                serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMax[3 * l + 1] = aabbMax[1];
                serverCmd.m_sendCollisionInfoArgs.m_linkWorldAABBsMax[3 * l + 2] = aabbMax[2];
            }
        }
    }
    else if (body && body->m_rigidBody)
    {
        RigidBody* rb = body->m_rigidBody;
        SharedMemoryStatus& serverCmd = serverStatusOut;
        serverStatusOut.m_type = CMD_REQUEST_COLLISION_INFO_COMPLETED;
        serverCmd.m_sendCollisionInfoArgs.m_numLinks = 0;
        setDefaultRootWorldAABB(serverCmd);

        if (rb->getCollisionShape())
        {
            Transform2 tr = rb->getWorldTransform();

            Vec3 aabbMin, aabbMax;
            rb->getCollisionShape()->getAabb(tr, aabbMin, aabbMax);
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[0] = aabbMin[0];
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[1] = aabbMin[1];
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[2] = aabbMin[2];

            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[0] = aabbMax[0];
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[1] = aabbMax[1];
            serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[2] = aabbMax[2];
        }
    }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    else if (body && body->m_softBody)
    {
        SoftBody* sb = body->m_softBody;
        serverCmd.m_sendCollisionInfoArgs.m_numLinks = 0;
        Vec3 aabbMin, aabbMax;
        sb->getAabb(aabbMin, aabbMax);

        serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[0] = aabbMin[0];
        serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[1] = aabbMin[1];
        serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMin[2] = aabbMin[2];

        serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[0] = aabbMax[0];
        serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[1] = aabbMax[1];
        serverCmd.m_sendCollisionInfoArgs.m_rootWorldAABBMax[2] = aabbMax[2];

        SharedMemoryStatus& serverCmd = serverStatusOut;
        serverStatusOut.m_type = CMD_REQUEST_COLLISION_INFO_COMPLETED;
    }
#endif
    return hasStatus;
}

bool PhysicsServerCommandProcessor::performCollisionDetectionCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_PERFORM_COLLISION_DETECTION");

    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("Perform Collision Detection command");
        drx3DPrintf("CMD_PERFORM_COLLISION_DETECTION clientCmd = %d\n", clientCmd.m_sequenceNumber);
    }

     m_data->m_dynamicsWorld->performDiscreteCollisionDetection();
     SharedMemoryStatus& serverCmd = serverStatusOut;
     serverCmd.m_type = CMD_PERFORM_COLLISION_DETECTION_COMPLETED;
     return true;
}

bool PhysicsServerCommandProcessor::processForwardDynamicsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_STEP_FORWARD_SIMULATION");

    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("Step simulation request");
        drx3DPrintf("CMD_STEP_FORWARD_SIMULATION clientCmd = %d\n", clientCmd.m_sequenceNumber);
    }
#ifndef USE_DISCRETE_DYNAMICS_WORLD
    ///todo(erwincoumans) move this damping inside drx3D
    for (i32 i = 0; i < m_data->m_dynamicsWorld->getNumMultibodies(); i++)
    {
        MultiBody* mb = m_data->m_dynamicsWorld->getMultiBody(i);
        for (i32 l = 0; l < mb->getNumLinks(); l++)
        {
            for (i32 d = 0; d < mb->getLink(l).m_dofCount; d++)
            {
                double damping_coefficient = mb->getLink(l).m_jointDamping;
                double damping = -damping_coefficient * mb->getJointVelMultiDof(l)[d];
                mb->addJointTorqueMultiDof(l, d, damping);
            }
        }
    }
    #endif
    Scalar deltaTimeScaled = m_data->m_physicsDeltaTime * simTimeScalingFactor;

    i32 numSteps = 0;
    if (m_data->m_numSimulationSubSteps > 0)
    {
        numSteps = m_data->m_dynamicsWorld->stepSimulation(deltaTimeScaled, m_data->m_numSimulationSubSteps, m_data->m_physicsDeltaTime / m_data->m_numSimulationSubSteps);
        m_data->m_simulationTimestamp += deltaTimeScaled;
    }
    else
    {
        numSteps = m_data->m_dynamicsWorld->stepSimulation(deltaTimeScaled, 0);
        m_data->m_simulationTimestamp += deltaTimeScaled;
    }

    if (numSteps > 0)
    {
        addBodyChangedNotifications();
    }

    SharedMemoryStatus& serverCmd = serverStatusOut;

    serverCmd.m_forwardDynamicsAnalyticsArgs.m_numSteps = numSteps;

    AlignedObjectArray<SolverAnalyticsData> islandAnalyticsData;
#ifndef USE_DISCRETE_DYNAMICS_WORLD
    m_data->m_dynamicsWorld->getAnalyticsData(islandAnalyticsData);
#endif
    serverCmd.m_forwardDynamicsAnalyticsArgs.m_numIslands = islandAnalyticsData.size();
    i32 numIslands = d3Min(islandAnalyticsData.size(), MAX_ISLANDS_ANALYTICS);

    for (i32 i = 0; i < numIslands; i++)
    {
        serverCmd.m_forwardDynamicsAnalyticsArgs.m_numSolverCalls = islandAnalyticsData[i].m_numSolverCalls;
        serverCmd.m_forwardDynamicsAnalyticsArgs.m_islandData[i].m_islandId = islandAnalyticsData[i].m_islandId;
        serverCmd.m_forwardDynamicsAnalyticsArgs.m_islandData[i].m_numBodies = islandAnalyticsData[i].m_numBodies;
        serverCmd.m_forwardDynamicsAnalyticsArgs.m_islandData[i].m_numIterationsUsed = islandAnalyticsData[i].m_numIterationsUsed;
        serverCmd.m_forwardDynamicsAnalyticsArgs.m_islandData[i].m_remainingLeastSquaresResidual = islandAnalyticsData[i].m_remainingLeastSquaresResidual;
        serverCmd.m_forwardDynamicsAnalyticsArgs.m_islandData[i].m_numContactManifolds = islandAnalyticsData[i].m_numContactManifolds;
    }
    serverCmd.m_type = CMD_STEP_FORWARD_SIMULATION_COMPLETED;

    m_data->m_remoteSyncTransformTime += deltaTimeScaled;
    if (m_data->m_remoteSyncTransformTime >= m_data->m_remoteSyncTransformInterval)
    {
        m_data->m_remoteSyncTransformTime = 0;
        syncPhysicsToGraphics2();
    }

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestInternalDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_REQUEST_INTERNAL_DATA");

    //todo: also check version etc?

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_REQUEST_INTERNAL_DATA_FAILED;

    i32 sz = DefaultSerializer::getMemoryDnaSizeInBytes();
    tukk memDna = DefaultSerializer::getMemoryDna();
    if (sz < bufferSizeInBytes)
    {
        for (i32 i = 0; i < sz; i++)
        {
            bufferServerToClient[i] = memDna[i];
        }
        serverCmd.m_type = CMD_REQUEST_INTERNAL_DATA_COMPLETED;
        serverCmd.m_numDataStreamBytes = sz;
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processChangeDynamicsInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_CHANGE_DYNAMICS_INFO");

    i32 bodyUniqueId = clientCmd.m_changeDynamicsInfoArgs.m_bodyUniqueId;
    i32 linkIndex = clientCmd.m_changeDynamicsInfoArgs.m_linkIndex;
    double mass = clientCmd.m_changeDynamicsInfoArgs.m_mass;
    double lateralFriction = clientCmd.m_changeDynamicsInfoArgs.m_lateralFriction;
    double spinningFriction = clientCmd.m_changeDynamicsInfoArgs.m_spinningFriction;
    double rollingFriction = clientCmd.m_changeDynamicsInfoArgs.m_rollingFriction;
    double restitution = clientCmd.m_changeDynamicsInfoArgs.m_restitution;
    Vec3 newLocalInertiaDiagonal(clientCmd.m_changeDynamicsInfoArgs.m_localInertiaDiagonal[0],
                                      clientCmd.m_changeDynamicsInfoArgs.m_localInertiaDiagonal[1],
                                      clientCmd.m_changeDynamicsInfoArgs.m_localInertiaDiagonal[2]);
    Vec3 anisotropicFriction(clientCmd.m_changeDynamicsInfoArgs.m_anisotropicFriction[0],
                                  clientCmd.m_changeDynamicsInfoArgs.m_anisotropicFriction[1],
                                  clientCmd.m_changeDynamicsInfoArgs.m_anisotropicFriction[2]);

    if (bodyUniqueId >= 0)
    {
        InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
#ifndef USE_DISCRETE_DYNAMICS_WORLD
        if (body && body->m_multiBody)
        {
            MultiBody* mb = body->m_multiBody;

            if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ACTIVATION_STATE)
            {
                if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateWakeUp)
                {
                    mb->wakeUp();
                }
                if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateSleep)
                {
                    mb->goToSleep();
                }
                if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateEnableSleeping)
                {
                    mb->setCanSleep(true);
                }
                if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateDisableSleeping)
                {
                    mb->setCanSleep(false);
                }
                if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateEnableWakeup)
                {
                    mb->setCanWakeup(true);
                }
                if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateDisableWakeup)
                {
                    mb->setCanWakeup(false);
                }
            }

            if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LINEAR_DAMPING)
            {
                mb->setLinearDamping(clientCmd.m_changeDynamicsInfoArgs.m_linearDamping);
            }
            if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ANGULAR_DAMPING)
            {
                mb->setAngularDamping(clientCmd.m_changeDynamicsInfoArgs.m_angularDamping);
            }

            if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_SLEEP_THRESHOLD)
            {
                mb->setSleepThreshold(clientCmd.m_changeDynamicsInfoArgs.m_sleepThreshold);
            }

            if (linkIndex == -1)
            {
                if (mb->getBaseCollider())
                {
                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_RESTITUTION)
                    {
                        mb->getBaseCollider()->setRestitution(restitution);
                    }

                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_STIFFNESS_AND_DAMPING)
                    {
                        mb->getBaseCollider()->setContactStiffnessAndDamping(clientCmd.m_changeDynamicsInfoArgs.m_contactStiffness, clientCmd.m_changeDynamicsInfoArgs.m_contactDamping);
                    }
                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LATERAL_FRICTION)
                    {
                        mb->getBaseCollider()->setFriction(lateralFriction);
                    }
                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_SPINNING_FRICTION)
                    {
                        mb->getBaseCollider()->setSpinningFriction(spinningFriction);
                    }
                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ROLLING_FRICTION)
                    {
                        mb->getBaseCollider()->setRollingFriction(rollingFriction);
                    }

                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_FRICTION_ANCHOR)
                    {
                        if (clientCmd.m_changeDynamicsInfoArgs.m_frictionAnchor)
                        {
                            mb->getBaseCollider()->setCollisionFlags(mb->getBaseCollider()->getCollisionFlags() | CollisionObject2::CF_HAS_FRICTION_ANCHOR);
                        }
                        else
                        {
                            mb->getBaseCollider()->setCollisionFlags(mb->getBaseCollider()->getCollisionFlags() & ~CollisionObject2::CF_HAS_FRICTION_ANCHOR);
                        }
                    }
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_MASS)
                {
                    mb->setBaseMass(mass);
                    if (mb->getBaseCollider() && mb->getBaseCollider()->getCollisionShape())
                    {
                        Vec3 localInertia;
                        mb->getBaseCollider()->getCollisionShape()->calculateLocalInertia(mass, localInertia);
                        mb->setBaseInertia(localInertia);
                    }

                    //handle switch from static/fixedBase to dynamic and vise-versa
                    if (mass > 0)
                    {
                        bool isDynamic = true;
                        if (mb->hasFixedBase())
                        {
                            i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
                            i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

                            m_data->m_dynamicsWorld->removeCollisionObject(mb->getBaseCollider());
                            i32 oldFlags = mb->getBaseCollider()->getCollisionFlags();
                            mb->getBaseCollider()->setCollisionFlags(oldFlags & ~CollisionObject2::CF_STATIC_OBJECT);
                            mb->setFixedBase(false);
                            m_data->m_dynamicsWorld->addCollisionObject(mb->getBaseCollider(), collisionFilterGroup, collisionFilterMask);
                        }
                    }
                    else
                    {
                        if (!mb->hasFixedBase())
                        {
                            bool isDynamic = false;
                            i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
                            i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);
                            i32 oldFlags = mb->getBaseCollider()->getCollisionFlags();
                            mb->getBaseCollider()->setCollisionFlags(oldFlags | CollisionObject2::CF_STATIC_OBJECT);
                            m_data->m_dynamicsWorld->removeCollisionObject(mb->getBaseCollider());
                            mb->setFixedBase(true);
                            m_data->m_dynamicsWorld->addCollisionObject(mb->getBaseCollider(), collisionFilterGroup, collisionFilterMask);
                        }
                    }
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LOCAL_INERTIA_DIAGONAL)
                {
                    mb->setBaseInertia(newLocalInertiaDiagonal);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ANISOTROPIC_FRICTION)
                {
                    mb->getBaseCollider()->setAnisotropicFriction(anisotropicFriction);
                }

                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_PROCESSING_THRESHOLD)
                {
                    mb->getBaseCollider()->setContactProcessingThreshold(clientCmd.m_changeDynamicsInfoArgs.m_contactProcessingThreshold);
                }

                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_MAX_JOINT_VELOCITY)
                {
                    mb->setMaxCoordinateVelocity(clientCmd.m_changeDynamicsInfoArgs.m_maxJointVelocity);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_COLLISION_MARGIN)
                {
                    mb->getBaseCollider()->getCollisionShape()->setMargin(clientCmd.m_changeDynamicsInfoArgs.m_collisionMargin);
                    if (mb->getBaseCollider()->getCollisionShape()->isCompound())
                    {
                        CompoundShape* compound = (CompoundShape*)mb->getBaseCollider()->getCollisionShape();
                        for (i32 s = 0; s < compound->getNumChildShapes(); s++)
                        {
                            compound->getChildShape(s)->setMargin(clientCmd.m_changeDynamicsInfoArgs.m_collisionMargin);
                        }
                    }
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_DYNAMIC_TYPE)
                {
                    i32 dynamic_type = clientCmd.m_changeDynamicsInfoArgs.m_dynamicType;
                    mb->setBaseDynamicType(dynamic_type);

                    bool isDynamic = dynamic_type == eDynamic;
                    i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
                    i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);
                    m_data->m_dynamicsWorld->removeCollisionObject(mb->getBaseCollider());
                    m_data->m_dynamicsWorld->addCollisionObject(mb->getBaseCollider(), collisionFilterGroup, collisionFilterMask);
                }
            }
            else
            {
                if (linkIndex >= 0 && linkIndex < mb->getNumLinks())
                {

                    if ((clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_JOINT_LIMIT_MAX_FORCE) ||
                        (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_JOINT_LIMITS))
                    {

                        MultiBodyJointLimitConstraint* limC = 0;

                        i32 numConstraints = m_data->m_dynamicsWorld->getNumMultiBodyConstraints();
                        for (i32 c = 0; c < numConstraints; c++)
                        {
                            MultiBodyConstraint* mbc = m_data->m_dynamicsWorld->getMultiBodyConstraint(c);
                            if (mbc->getConstraintType() == MULTIBODY_CONSTRAINT_LIMIT)
                            {
                                if ((mbc->getMultiBodyA() == mb) && (mbc->getLinkA() == linkIndex))
                                {
                                    limC = (MultiBodyJointLimitConstraint*)mbc;
                                }
                            }
                        }


                        if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_JOINT_LIMITS)
                        {
                            //find a joint limit
                            Scalar prevUpper = mb->getLink(linkIndex).m_jointUpperLimit;
                            Scalar prevLower = mb->getLink(linkIndex).m_jointLowerLimit;
                            Scalar lower = clientCmd.m_changeDynamicsInfoArgs.m_jointLowerLimit;
                            Scalar upper = clientCmd.m_changeDynamicsInfoArgs.m_jointUpperLimit;
                            bool enableLimit = lower <= upper;

                            if (enableLimit)
                            {
                                if (limC == 0)
                                {
                                    limC = new MultiBodyJointLimitConstraint(mb, linkIndex, lower, upper);
                                    m_data->m_dynamicsWorld->addMultiBodyConstraint(limC);
                                }
                                else
                                {
                                    limC->setLowerBound(lower);
                                    limC->setUpperBound(upper);
                                }
                                mb->getLink(linkIndex).m_jointLowerLimit = lower;
                                mb->getLink(linkIndex).m_jointUpperLimit = upper;
                            }
                            else
                            {
                                if (limC)
                                {
                                    m_data->m_dynamicsWorld->removeMultiBodyConstraint(limC);
                                    delete limC;
                                    limC = 0;
                                }
                                mb->getLink(linkIndex).m_jointLowerLimit = 1;
                                mb->getLink(linkIndex).m_jointUpperLimit = -1;
                            }
                        }

                        if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_JOINT_LIMIT_MAX_FORCE)
                        {
                            Scalar fixedTimeSubStep = m_data->m_numSimulationSubSteps > 0 ? m_data->m_physicsDeltaTime / m_data->m_numSimulationSubSteps : m_data->m_physicsDeltaTime;
                            Scalar maxImpulse = clientCmd.m_changeDynamicsInfoArgs.m_jointLimitForce * fixedTimeSubStep;
                            if (limC)
                            {
                                //convert from force to impulse
                                limC->setMaxAppliedImpulse(maxImpulse);
                            }
                        }
                    }

                    if (mb->getLinkCollider(linkIndex))
                    {
                        if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_RESTITUTION)
                        {
                            mb->getLinkCollider(linkIndex)->setRestitution(restitution);
                        }
                        if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_SPINNING_FRICTION)
                        {
                            mb->getLinkCollider(linkIndex)->setSpinningFriction(spinningFriction);
                        }
                        if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ROLLING_FRICTION)
                        {
                            mb->getLinkCollider(linkIndex)->setRollingFriction(rollingFriction);
                        }

                        if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_FRICTION_ANCHOR)
                        {
                            if (clientCmd.m_changeDynamicsInfoArgs.m_frictionAnchor)
                            {
                                mb->getLinkCollider(linkIndex)->setCollisionFlags(mb->getLinkCollider(linkIndex)->getCollisionFlags() | CollisionObject2::CF_HAS_FRICTION_ANCHOR);
                            }
                            else
                            {
                                mb->getLinkCollider(linkIndex)->setCollisionFlags(mb->getLinkCollider(linkIndex)->getCollisionFlags() & ~CollisionObject2::CF_HAS_FRICTION_ANCHOR);
                            }
                        }

                        if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LATERAL_FRICTION)
                        {
                            mb->getLinkCollider(linkIndex)->setFriction(lateralFriction);
                        }

                        if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_STIFFNESS_AND_DAMPING)
                        {
                            mb->getLinkCollider(linkIndex)->setContactStiffnessAndDamping(clientCmd.m_changeDynamicsInfoArgs.m_contactStiffness, clientCmd.m_changeDynamicsInfoArgs.m_contactDamping);
                        }
                        if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_COLLISION_MARGIN)
                        {
                            mb->getLinkCollider(linkIndex)->getCollisionShape()->setMargin(clientCmd.m_changeDynamicsInfoArgs.m_collisionMargin);
                        }
                    }

                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_JOINT_DAMPING)
                    {
                        mb->getLink(linkIndex).m_jointDamping = clientCmd.m_changeDynamicsInfoArgs.m_jointDamping;
                    }

                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_MASS)
                    {
                        mb->getLink(linkIndex).m_mass = mass;
                        if (mb->getLinkCollider(linkIndex) && mb->getLinkCollider(linkIndex)->getCollisionShape())
                        {
                            Vec3 localInertia;
                            mb->getLinkCollider(linkIndex)->getCollisionShape()->calculateLocalInertia(mass, localInertia);
                            mb->getLink(linkIndex).m_inertiaLocal = localInertia;
                        }
                    }
                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LOCAL_INERTIA_DIAGONAL)
                    {
                        mb->getLink(linkIndex).m_inertiaLocal = newLocalInertiaDiagonal;
                    }
                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ANISOTROPIC_FRICTION)
                    {
                        mb->getLinkCollider(linkIndex)->setAnisotropicFriction(anisotropicFriction);
                    }
                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_PROCESSING_THRESHOLD)
                    {
                        mb->getLinkCollider(linkIndex)->setContactProcessingThreshold(clientCmd.m_changeDynamicsInfoArgs.m_contactProcessingThreshold);
                    }
                    if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_DYNAMIC_TYPE)
                    {
                        i32 dynamic_type = clientCmd.m_changeDynamicsInfoArgs.m_dynamicType;
                        mb->setLinkDynamicType(linkIndex, dynamic_type);

                        bool isDynamic = dynamic_type == eDynamic;
                        i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
                        i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);
                        m_data->m_dynamicsWorld->removeCollisionObject(mb->getLinkCollider(linkIndex));
                        m_data->m_dynamicsWorld->addCollisionObject(mb->getLinkCollider(linkIndex), collisionFilterGroup, collisionFilterMask);
                    }
                }
            }
        }
        else
#endif
        {
            RigidBody* rb = 0;
            if (body && body->m_rigidBody)
            {

                if (linkIndex == -1)
                {
                    rb = body->m_rigidBody;
                }
                else
                {
                    if (linkIndex >= 0 && linkIndex < body->m_rigidBodyJoints.size())
                    {
                        RigidBody* parentRb = &body->m_rigidBodyJoints[linkIndex]->getRigidBodyA();
                        RigidBody* childRb = &body->m_rigidBodyJoints[linkIndex]->getRigidBodyB();
                        rb = childRb;
                    }
                }
            }

            if (rb)
            {
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ACTIVATION_STATE)
                {
                    if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateEnableSleeping)
                    {
                        rb->forceActivationState(ACTIVE_TAG);
                    }
                    if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateDisableSleeping)
                    {
                        rb->forceActivationState(DISABLE_DEACTIVATION);
                    }
                    if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateWakeUp)
                    {
                        rb->forceActivationState(ACTIVE_TAG);
                        rb->setDeactivationTime(0.0);
                    }
                    if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateSleep)
                    {
                        rb->forceActivationState(ISLAND_SLEEPING);
                    }
                }

                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LINEAR_DAMPING)
                {
                    Scalar angDamping = rb->getAngularDamping();
                    rb->setDamping(clientCmd.m_changeDynamicsInfoArgs.m_linearDamping, angDamping);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ANGULAR_DAMPING)
                {
                    Scalar linDamping = rb->getLinearDamping();
                    rb->setDamping(linDamping, clientCmd.m_changeDynamicsInfoArgs.m_angularDamping);
                }

                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_STIFFNESS_AND_DAMPING)
                {
                    rb->setContactStiffnessAndDamping(clientCmd.m_changeDynamicsInfoArgs.m_contactStiffness, clientCmd.m_changeDynamicsInfoArgs.m_contactDamping);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_RESTITUTION)
                {
                    rb->setRestitution(restitution);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LATERAL_FRICTION)
                {
                    rb->setFriction(lateralFriction);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_SPINNING_FRICTION)
                {
                    rb->setSpinningFriction(spinningFriction);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ROLLING_FRICTION)
                {
                    rb->setRollingFriction(rollingFriction);
                }

                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_FRICTION_ANCHOR)
                {
                    if (clientCmd.m_changeDynamicsInfoArgs.m_frictionAnchor)
                    {
                        rb->setCollisionFlags(rb->getCollisionFlags() | CollisionObject2::CF_HAS_FRICTION_ANCHOR);
                    }
                    else
                    {
                        rb->setCollisionFlags(rb->getCollisionFlags() & ~CollisionObject2::CF_HAS_FRICTION_ANCHOR);
                    }
                }

                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_MASS)
                {
                    Vec3 localInertia;
                    if (rb->getCollisionShape())
                    {
                        rb->getCollisionShape()->calculateLocalInertia(mass, localInertia);
                    }
                    rb->setMassProps(mass, localInertia);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_LOCAL_INERTIA_DIAGONAL)
                {
                    Scalar orgMass = rb->getInvMass();
                    if (orgMass > 0)
                    {
                        rb->setMassProps(mass, newLocalInertiaDiagonal);
                    }
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ANISOTROPIC_FRICTION)
                {
                    rb->setAnisotropicFriction(anisotropicFriction);
                }

                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CONTACT_PROCESSING_THRESHOLD)
                {
                    rb->setContactProcessingThreshold(clientCmd.m_changeDynamicsInfoArgs.m_contactProcessingThreshold);
                }

                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_CCD_SWEPT_SPHERE_RADIUS)
                {
                    rb->setCcdSweptSphereRadius(clientCmd.m_changeDynamicsInfoArgs.m_ccdSweptSphereRadius);
                    //for a given sphere radius, use a motion threshold of half the radius, before the ccd algorithm is enabled
                    rb->setCcdMotionThreshold(clientCmd.m_changeDynamicsInfoArgs.m_ccdSweptSphereRadius / 2.);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_COLLISION_MARGIN)
                {
                    rb->getCollisionShape()->setMargin(clientCmd.m_changeDynamicsInfoArgs.m_collisionMargin);
                }
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_DYNAMIC_TYPE)
                {
                    i32 dynamic_type = clientCmd.m_changeDynamicsInfoArgs.m_dynamicType;
                    // If mass is zero, the object cannot be set to be dynamic.
                    if (!(rb->getInvMass() != Scalar(0.) || dynamic_type != eDynamic)) {
                        i32 collision_flags = rb->getCollisionFlags();
                        collision_flags &= ~(CollisionObject2::CF_STATIC_OBJECT | CollisionObject2::CF_KINEMATIC_OBJECT);
                        collision_flags |= dynamic_type;
                        rb->setCollisionFlags(collision_flags);
                        bool isDynamic = dynamic_type == eDynamic;
                        i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
                        i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);
                        m_data->m_dynamicsWorld->removeCollisionObject(rb);
                        m_data->m_dynamicsWorld->addCollisionObject(rb, collisionFilterGroup, collisionFilterMask);
                    }
                }

                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_SLEEP_THRESHOLD)
                {
                    Scalar threshold2 = Sqrt(clientCmd.m_changeDynamicsInfoArgs.m_sleepThreshold);
                    rb->setSleepingThresholds(threshold2,threshold2);
                }

            }
        }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        if (body && body->m_softBody)
        {
            SoftBody* psb = body->m_softBody;
            if (psb)
            {
                if (clientCmd.m_updateFlags & CHANGE_DYNAMICS_INFO_SET_ACTIVATION_STATE)
                {
                    if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateEnableSleeping)
                    {
                        psb->forceActivationState(ACTIVE_TAG);
                    }
                    if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateDisableSleeping)
                    {
                        psb->forceActivationState(DISABLE_DEACTIVATION);
                    }
                    if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateWakeUp)
                    {
                        psb->forceActivationState(ACTIVE_TAG);
                        psb->setDeactivationTime(0.0);
                    }
                    if (clientCmd.m_changeDynamicsInfoArgs.m_activationState & eActivationStateSleep)
                    {
                        psb->forceActivationState(ISLAND_SLEEPING);
                    }
                }
            }
        }
#endif
    }
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;

    b3Notification notification;
    notification.m_notificationType = LINK_DYNAMICS_CHANGED;
    notification.m_linkArgs.m_bodyUniqueId = bodyUniqueId;
    notification.m_linkArgs.m_linkIndex = linkIndex;
    m_data->m_pluginManager.addNotification(notification);

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processSetAdditionalSearchPathCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_SET_ADDITIONAL_SEARCH_PATH");
    ResourcePath::setAdditionalSearchPath(clientCmd.m_searchPathArgs.m_path);
    serverStatusOut.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processGetDynamicsInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_GET_DYNAMICS_INFO_FAILED;

    i32 bodyUniqueId = clientCmd.m_getDynamicsInfoArgs.m_bodyUniqueId;
    i32 linkIndex = clientCmd.m_getDynamicsInfoArgs.m_linkIndex;
    InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);
    if (body && body->m_multiBody)
    {
        SharedMemoryStatus& serverCmd = serverStatusOut;
        serverCmd.m_type = CMD_GET_DYNAMICS_INFO_COMPLETED;
        serverCmd.m_dynamicsInfo.m_bodyType = DRX3D_MULTI_BODY;

        MultiBody* mb = body->m_multiBody;
        if (linkIndex == -1)
        {
            serverCmd.m_dynamicsInfo.m_mass = mb->getBaseMass();
            if (mb->getBaseCollider())
            {
                serverCmd.m_dynamicsInfo.m_activationState = mb->getBaseCollider()->getActivationState();
                serverCmd.m_dynamicsInfo.m_contactProcessingThreshold = mb->getBaseCollider()->getContactProcessingThreshold();
                serverCmd.m_dynamicsInfo.m_ccdSweptSphereRadius = mb->getBaseCollider()->getCcdSweptSphereRadius();
                serverCmd.m_dynamicsInfo.m_frictionAnchor = mb->getBaseCollider()->getCollisionFlags() & CollisionObject2::CF_HAS_FRICTION_ANCHOR;
                serverCmd.m_dynamicsInfo.m_collisionMargin = mb->getBaseCollider()->getCollisionShape()->getMargin();
                serverCmd.m_dynamicsInfo.m_dynamicType = mb->getBaseCollider()->getCollisionFlags() & (CollisionObject2::CF_STATIC_OBJECT | CollisionObject2::CF_KINEMATIC_OBJECT);
            }
            else
            {
                serverCmd.m_dynamicsInfo.m_activationState = 0;
                serverCmd.m_dynamicsInfo.m_contactProcessingThreshold = 0;
                serverCmd.m_dynamicsInfo.m_ccdSweptSphereRadius = 0;
                serverCmd.m_dynamicsInfo.m_frictionAnchor = 0;
                serverCmd.m_dynamicsInfo.m_collisionMargin = 0;
                serverCmd.m_dynamicsInfo.m_dynamicType = 0;
            }
            serverCmd.m_dynamicsInfo.m_localInertialDiagonal[0] = mb->getBaseInertia()[0];
            serverCmd.m_dynamicsInfo.m_localInertialDiagonal[1] = mb->getBaseInertia()[1];
            serverCmd.m_dynamicsInfo.m_localInertialDiagonal[2] = mb->getBaseInertia()[2];
            serverCmd.m_dynamicsInfo.m_lateralFrictionCoeff = mb->getBaseCollider()->getFriction();

            serverCmd.m_dynamicsInfo.m_localInertialFrame[0] = body->m_rootLocalInertialFrame.getOrigin()[0];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[1] = body->m_rootLocalInertialFrame.getOrigin()[1];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[2] = body->m_rootLocalInertialFrame.getOrigin()[2];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[3] = body->m_rootLocalInertialFrame.getRotation()[0];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[4] = body->m_rootLocalInertialFrame.getRotation()[1];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[5] = body->m_rootLocalInertialFrame.getRotation()[2];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[6] = body->m_rootLocalInertialFrame.getRotation()[3];

            serverCmd.m_dynamicsInfo.m_angularDamping = body->m_multiBody->getAngularDamping();
            serverCmd.m_dynamicsInfo.m_linearDamping = body->m_multiBody->getLinearDamping();

            serverCmd.m_dynamicsInfo.m_restitution = mb->getBaseCollider()->getRestitution();
            serverCmd.m_dynamicsInfo.m_rollingFrictionCoeff = mb->getBaseCollider()->getRollingFriction();
            serverCmd.m_dynamicsInfo.m_spinningFrictionCoeff = mb->getBaseCollider()->getSpinningFriction();

            if (mb->getBaseCollider()->getCollisionFlags() & CollisionObject2::CF_HAS_CONTACT_STIFFNESS_DAMPING)
            {
                serverCmd.m_dynamicsInfo.m_contactStiffness = mb->getBaseCollider()->getContactStiffness();
                serverCmd.m_dynamicsInfo.m_contactDamping = mb->getBaseCollider()->getContactDamping();
            }
            else
            {
                serverCmd.m_dynamicsInfo.m_contactStiffness = -1;
                serverCmd.m_dynamicsInfo.m_contactDamping = -1;
            }
        }
        else
        {
            serverCmd.m_dynamicsInfo.m_mass = mb->getLinkMass(linkIndex);

            if (mb->getLinkCollider(linkIndex))
            {
                serverCmd.m_dynamicsInfo.m_activationState = mb->getLinkCollider(linkIndex)->getActivationState();
                serverCmd.m_dynamicsInfo.m_contactProcessingThreshold = mb->getLinkCollider(linkIndex)->getContactProcessingThreshold();
                serverCmd.m_dynamicsInfo.m_ccdSweptSphereRadius = mb->getLinkCollider(linkIndex)->getCcdSweptSphereRadius();
                serverCmd.m_dynamicsInfo.m_frictionAnchor = mb->getLinkCollider(linkIndex)->getCollisionFlags() & CollisionObject2::CF_HAS_FRICTION_ANCHOR;
                serverCmd.m_dynamicsInfo.m_collisionMargin = mb->getLinkCollider(linkIndex)->getCollisionShape()->getMargin();
                serverCmd.m_dynamicsInfo.m_dynamicType = mb->getLinkCollider(linkIndex)->getCollisionFlags() & (CollisionObject2::CF_STATIC_OBJECT | CollisionObject2::CF_KINEMATIC_OBJECT);
            }
            else
            {
                serverCmd.m_dynamicsInfo.m_activationState = 0;
                serverCmd.m_dynamicsInfo.m_contactProcessingThreshold = 0;
                serverCmd.m_dynamicsInfo.m_ccdSweptSphereRadius = 0;
                serverCmd.m_dynamicsInfo.m_frictionAnchor = 0;
                serverCmd.m_dynamicsInfo.m_collisionMargin = 0;
                serverCmd.m_dynamicsInfo.m_dynamicType = 0;
            }

            serverCmd.m_dynamicsInfo.m_localInertialDiagonal[0] = mb->getLinkInertia(linkIndex)[0];
            serverCmd.m_dynamicsInfo.m_localInertialDiagonal[1] = mb->getLinkInertia(linkIndex)[1];
            serverCmd.m_dynamicsInfo.m_localInertialDiagonal[2] = mb->getLinkInertia(linkIndex)[2];

            serverCmd.m_dynamicsInfo.m_localInertialFrame[0] = body->m_linkLocalInertialFrames[linkIndex].getOrigin()[0];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[1] = body->m_linkLocalInertialFrames[linkIndex].getOrigin()[1];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[2] = body->m_linkLocalInertialFrames[linkIndex].getOrigin()[2];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[3] = body->m_linkLocalInertialFrames[linkIndex].getRotation()[0];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[4] = body->m_linkLocalInertialFrames[linkIndex].getRotation()[1];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[5] = body->m_linkLocalInertialFrames[linkIndex].getRotation()[2];
            serverCmd.m_dynamicsInfo.m_localInertialFrame[6] = body->m_linkLocalInertialFrames[linkIndex].getRotation()[3];

            serverCmd.m_dynamicsInfo.m_angularDamping = body->m_multiBody->getAngularDamping();
            serverCmd.m_dynamicsInfo.m_linearDamping = body->m_multiBody->getLinearDamping();

            if (mb->getLinkCollider(linkIndex))
            {
                serverCmd.m_dynamicsInfo.m_lateralFrictionCoeff = mb->getLinkCollider(linkIndex)->getFriction();
                serverCmd.m_dynamicsInfo.m_restitution = mb->getLinkCollider(linkIndex)->getRestitution();
                serverCmd.m_dynamicsInfo.m_rollingFrictionCoeff = mb->getLinkCollider(linkIndex)->getRollingFriction();
                serverCmd.m_dynamicsInfo.m_spinningFrictionCoeff = mb->getLinkCollider(linkIndex)->getSpinningFriction();

                if (mb->getLinkCollider(linkIndex)->getCollisionFlags() & CollisionObject2::CF_HAS_CONTACT_STIFFNESS_DAMPING)
                {
                    serverCmd.m_dynamicsInfo.m_contactStiffness = mb->getLinkCollider(linkIndex)->getContactStiffness();
                    serverCmd.m_dynamicsInfo.m_contactDamping = mb->getLinkCollider(linkIndex)->getContactDamping();
                }
                else
                {
                    serverCmd.m_dynamicsInfo.m_contactStiffness = -1;
                    serverCmd.m_dynamicsInfo.m_contactDamping = -1;
                }
            }
            else
            {
                drx3DWarning("The dynamic info requested is not available");
                serverCmd.m_type = CMD_GET_DYNAMICS_INFO_FAILED;
            }
        }
        hasStatus = true;
    }
    else if (body && body->m_rigidBody)
    {
        SharedMemoryStatus& serverCmd = serverStatusOut;
        serverCmd.m_type = CMD_GET_DYNAMICS_INFO_COMPLETED;
        serverCmd.m_dynamicsInfo.m_bodyType = DRX3D_RIGID_BODY;

        RigidBody* rb = body->m_rigidBody;


        serverCmd.m_dynamicsInfo.m_localInertialDiagonal[0] = rb->getLocalInertia()[0];
        serverCmd.m_dynamicsInfo.m_localInertialDiagonal[1] = rb->getLocalInertia()[1];
        serverCmd.m_dynamicsInfo.m_localInertialDiagonal[2] = rb->getLocalInertia()[2];

        serverCmd.m_dynamicsInfo.m_lateralFrictionCoeff = rb->getFriction();
        serverCmd.m_dynamicsInfo.m_rollingFrictionCoeff = rb->getRollingFriction();
        serverCmd.m_dynamicsInfo.m_spinningFrictionCoeff = rb->getSpinningFriction();
        serverCmd.m_dynamicsInfo.m_angularDamping = rb->getAngularDamping();
        serverCmd.m_dynamicsInfo.m_linearDamping = rb->getLinearDamping();
        serverCmd.m_dynamicsInfo.m_mass = rb->getMass();
        serverCmd.m_dynamicsInfo.m_collisionMargin = rb->getCollisionShape() ? rb->getCollisionShape()->getMargin() : 0;
        serverCmd.m_dynamicsInfo.m_dynamicType = rb->getCollisionFlags() & (CollisionObject2::CF_STATIC_OBJECT | CollisionObject2::CF_KINEMATIC_OBJECT);
    }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    else if (body && body->m_softBody)
    {
        SharedMemoryStatus& serverCmd = serverStatusOut;
        serverCmd.m_type = CMD_GET_DYNAMICS_INFO_COMPLETED;
        serverCmd.m_dynamicsInfo.m_bodyType = DRX3D_SOFT_BODY;
        serverCmd.m_dynamicsInfo.m_collisionMargin = 0;
    }
#endif
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestPhysicsSimulationParametersCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_REQUEST_PHYSICS_SIMULATION_PARAMETERS_COMPLETED;

    serverCmd.m_simulationParameterResultArgs.m_allowedCcdPenetration = m_data->m_dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration;
    serverCmd.m_simulationParameterResultArgs.m_collisionFilterMode = m_data->m_broadphaseCollisionFilterCallback->m_filterMode;
    serverCmd.m_simulationParameterResultArgs.m_deltaTime = m_data->m_physicsDeltaTime;
    serverCmd.m_simulationParameterResultArgs.m_simulationTimestamp = m_data->m_simulationTimestamp;
    serverCmd.m_simulationParameterResultArgs.m_contactBreakingThreshold = gContactBreakingThreshold;
    serverCmd.m_simulationParameterResultArgs.m_contactSlop = m_data->m_dynamicsWorld->getSolverInfo().m_linearSlop;
    serverCmd.m_simulationParameterResultArgs.m_enableSAT = m_data->m_dynamicsWorld->getDispatchInfo().m_enableSatConvex;

    serverCmd.m_simulationParameterResultArgs.m_defaultGlobalCFM = m_data->m_dynamicsWorld->getSolverInfo().m_globalCfm;
    serverCmd.m_simulationParameterResultArgs.m_defaultContactERP = m_data->m_dynamicsWorld->getSolverInfo().m_erp2;
    serverCmd.m_simulationParameterResultArgs.m_defaultNonContactERP = m_data->m_dynamicsWorld->getSolverInfo().m_erp;
    serverCmd.m_simulationParameterResultArgs.m_deltaTime = m_data->m_physicsDeltaTime;
    serverCmd.m_simulationParameterResultArgs.m_deterministicOverlappingPairs = m_data->m_dynamicsWorld->getDispatchInfo().m_deterministicOverlappingPairs;
    serverCmd.m_simulationParameterResultArgs.m_enableConeFriction = (m_data->m_dynamicsWorld->getSolverInfo().m_solverMode & SOLVER_DISABLE_IMPLICIT_CONE_FRICTION) ? 0 : 1;
    serverCmd.m_simulationParameterResultArgs.m_enableFileCaching = b3IsFileCachingEnabled();
    serverCmd.m_simulationParameterResultArgs.m_frictionCFM = m_data->m_dynamicsWorld->getSolverInfo().m_frictionCFM;
    serverCmd.m_simulationParameterResultArgs.m_frictionERP = m_data->m_dynamicsWorld->getSolverInfo().m_frictionERP;
    Vec3 grav = m_data->m_dynamicsWorld->getGravity();
    serverCmd.m_simulationParameterResultArgs.m_gravityAcceleration[0] = grav[0];
    serverCmd.m_simulationParameterResultArgs.m_gravityAcceleration[1] = grav[1];
    serverCmd.m_simulationParameterResultArgs.m_gravityAcceleration[2] = grav[2];
    serverCmd.m_simulationParameterResultArgs.m_internalSimFlags = gInternalSimFlags;
    serverCmd.m_simulationParameterResultArgs.m_jointFeedbackMode = 0;
    if (m_data->m_dynamicsWorld->getSolverInfo().m_jointFeedbackInWorldSpace)
    {
        serverCmd.m_simulationParameterResultArgs.m_jointFeedbackMode |= JOINT_FEEDBACK_IN_WORLD_SPACE;
    }
    if (m_data->m_dynamicsWorld->getSolverInfo().m_jointFeedbackInJointFrame)
    {
        serverCmd.m_simulationParameterResultArgs.m_jointFeedbackMode |= JOINT_FEEDBACK_IN_JOINT_FRAME;
    }

    serverCmd.m_simulationParameterResultArgs.m_numSimulationSubSteps = m_data->m_numSimulationSubSteps;
    serverCmd.m_simulationParameterResultArgs.m_numSolverIterations = m_data->m_dynamicsWorld->getSolverInfo().m_numIterations;
    serverCmd.m_simulationParameterResultArgs.m_numNonContactInnerIterations = m_data->m_dynamicsWorld->getSolverInfo().m_numNonContactInnerIterations;
    serverCmd.m_simulationParameterResultArgs.m_restitutionVelocityThreshold = m_data->m_dynamicsWorld->getSolverInfo().m_restitutionVelocityThreshold;

    serverCmd.m_simulationParameterResultArgs.m_solverResidualThreshold = m_data->m_dynamicsWorld->getSolverInfo().m_leastSquaresResidualThreshold;
    serverCmd.m_simulationParameterResultArgs.m_splitImpulsePenetrationThreshold = m_data->m_dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold;
    serverCmd.m_simulationParameterResultArgs.m_useRealTimeSimulation = m_data->m_useRealTimeSimulation;
    serverCmd.m_simulationParameterResultArgs.m_useSplitImpulse = m_data->m_dynamicsWorld->getSolverInfo().m_splitImpulse;

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processSendPhysicsParametersCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_SEND_PHYSICS_SIMULATION_PARAMETERS");

    if (clientCmd.m_updateFlags & SIM_PARAM_ENABLE_CONE_FRICTION)
    {
        if (clientCmd.m_physSimParamArgs.m_enableConeFriction)
        {
            m_data->m_dynamicsWorld->getSolverInfo().m_solverMode &= ~SOLVER_DISABLE_IMPLICIT_CONE_FRICTION;
        }
        else
        {
            m_data->m_dynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_DISABLE_IMPLICIT_CONE_FRICTION;
        }
    }
    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_DETERMINISTIC_OVERLAPPING_PAIRS)
    {
        m_data->m_dynamicsWorld->getDispatchInfo().m_deterministicOverlappingPairs = (clientCmd.m_physSimParamArgs.m_deterministicOverlappingPairs != 0);
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_CCD_ALLOWED_PENETRATION)
    {
        m_data->m_dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration = clientCmd.m_physSimParamArgs.m_allowedCcdPenetration;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_JOINT_FEEDBACK_MODE)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_jointFeedbackInWorldSpace = (clientCmd.m_physSimParamArgs.m_jointFeedbackMode & JOINT_FEEDBACK_IN_WORLD_SPACE) != 0;
        m_data->m_dynamicsWorld->getSolverInfo().m_jointFeedbackInJointFrame = (clientCmd.m_physSimParamArgs.m_jointFeedbackMode & JOINT_FEEDBACK_IN_JOINT_FRAME) != 0;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_DELTA_TIME)
    {
        m_data->m_physicsDeltaTime = clientCmd.m_physSimParamArgs.m_deltaTime;
    }
    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_REAL_TIME_SIMULATION)
    {
        m_data->m_useRealTimeSimulation = (clientCmd.m_physSimParamArgs.m_useRealTimeSimulation != 0);
    }

    //see
    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_INTERNAL_SIMULATION_FLAGS)
    {
        //these flags are for internal/temporary/easter-egg/experimental demo purposes, use at own risk
        gInternalSimFlags = clientCmd.m_physSimParamArgs.m_internalSimFlags;
        m_data->m_useAlternativeDeformableIndexing =
                (clientCmd.m_physSimParamArgs.m_internalSimFlags & eDeformableAlternativeIndexing) != 0;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_GRAVITY)
    {
        Vec3 grav(clientCmd.m_physSimParamArgs.m_gravityAcceleration[0],
                       clientCmd.m_physSimParamArgs.m_gravityAcceleration[1],
                       clientCmd.m_physSimParamArgs.m_gravityAcceleration[2]);
        this->m_data->m_dynamicsWorld->setGravity(grav);
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
        if (softWorld)
        {
            softWorld->getWorldInfo().m_gravity = grav;
        }
        DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
        if (deformWorld)
        {
            // deformWorld->getWorldInfo().m_gravity = grav;
            deformWorld->setGravity(grav);
            for (i32 i = 0; i < m_data->m_lf.size(); ++i)
            {
                DeformableLagrangianForce* force = m_data->m_lf[i];
                if (force->getForceType() == DRX3D_GRAVITY_FORCE)
                {
                    DeformableGravityForce* gforce = (DeformableGravityForce*)force;
                    gforce->m_gravity = grav;
                }
            }
        }

#endif
        if (m_data->m_verboseOutput)
        {
            drx3DPrintf("Updated Gravity: %f,%f,%f", grav[0], grav[1], grav[2]);
        }
    }
    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_NUM_SOLVER_ITERATIONS)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_numIterations = clientCmd.m_physSimParamArgs.m_numSolverIterations;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_NUM_NONCONTACT_INNER_ITERATIONS)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_numNonContactInnerIterations = clientCmd.m_physSimParamArgs.m_numNonContactInnerIterations;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_SOLVER_RESIDULAL_THRESHOLD)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_leastSquaresResidualThreshold = clientCmd.m_physSimParamArgs.m_solverResidualThreshold;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_CONTACT_BREAKING_THRESHOLD)
    {
        gContactBreakingThreshold = clientCmd.m_physSimParamArgs.m_contactBreakingThreshold;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_CONTACT_SLOP)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_linearSlop = clientCmd.m_physSimParamArgs.m_contactSlop;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_ENABLE_SAT)
    {
        m_data->m_dynamicsWorld->getDispatchInfo().m_enableSatConvex = clientCmd.m_physSimParamArgs.m_enableSAT != 0;
    }
    if (clientCmd.m_updateFlags & SIM_PARAM_CONSTRAINT_SOLVER_TYPE)
    {
        //check if the current type is different from requested one
        if (m_data->m_constraintSolverType != clientCmd.m_physSimParamArgs.m_constraintSolverType)
        {
            m_data->m_constraintSolverType = clientCmd.m_physSimParamArgs.m_constraintSolverType;

            ConstraintSolver* oldSolver = m_data->m_dynamicsWorld->getConstraintSolver();

#ifdef USE_DISCRETE_DYNAMICS_WORLD
            SequentialImpulseConstraintSolver* newSolver = 0;
#else
            MultiBodyConstraintSolver* newSolver = 0;
#endif

            switch (clientCmd.m_physSimParamArgs.m_constraintSolverType)
            {
                case eConstraintSolverLCP_SI:
                {
                    newSolver = new MultiBodyConstraintSolver;
                    drx3DPrintf("PyBullet: Constraint Solver: btMultiBodyConstraintSolver\n");
                    break;
                }
                case eConstraintSolverLCP_PGS:
                {
                    SolveProjectedGaussSeidel* mlcp = new SolveProjectedGaussSeidel();
#ifdef USE_DISCRETE_DYNAMICS_WORLD
                    newSolver = new btMLCPSolver(mlcp);
#else
                    newSolver = new MultiBodyMLCPConstraintSolver(mlcp);
#endif

                    drx3DPrintf("PyBullet: Constraint Solver: MLCP + PGS\n");
                    break;
                }
                case eConstraintSolverLCP_DANTZIG:
                {
                    DantzigSolver* mlcp = new DantzigSolver();
                    newSolver = new MultiBodyMLCPConstraintSolver(mlcp);
                    drx3DPrintf("PyBullet: Constraint Solver: MLCP + Dantzig\n");
                    break;
                }
                case eConstraintSolverLCP_BLOCK_PGS:
                {
                    break;
                }
                default:
                {
                }
            };

            if (newSolver)
            {
                delete oldSolver;

#ifdef USE_DISCRETE_DYNAMICS_WORLD
                m_data->m_dynamicsWorld->setConstraintSolver(newSolver);
#else
                m_data->m_dynamicsWorld->setMultiBodyConstraintSolver(newSolver);
#endif
                m_data->m_solver = newSolver;
                printf("switched solver\n");
            }
        }
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_CONSTRAINT_MIN_SOLVER_ISLAND_SIZE)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = clientCmd.m_physSimParamArgs.m_minimumSolverIslandSize;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_COLLISION_FILTER_MODE)
    {
        m_data->m_broadphaseCollisionFilterCallback->m_filterMode = clientCmd.m_physSimParamArgs.m_collisionFilterMode;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_USE_SPLIT_IMPULSE)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_splitImpulse = clientCmd.m_physSimParamArgs.m_useSplitImpulse;
    }
    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_SPLIT_IMPULSE_PENETRATION_THRESHOLD)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold = clientCmd.m_physSimParamArgs.m_splitImpulsePenetrationThreshold;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_NUM_SIMULATION_SUB_STEPS)
    {
        m_data->m_numSimulationSubSteps = clientCmd.m_physSimParamArgs.m_numSimulationSubSteps;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_DEFAULT_CONTACT_ERP)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_erp2 = clientCmd.m_physSimParamArgs.m_defaultContactERP;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_DEFAULT_NON_CONTACT_ERP)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_erp = clientCmd.m_physSimParamArgs.m_defaultNonContactERP;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_DEFAULT_FRICTION_ERP)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_frictionERP = clientCmd.m_physSimParamArgs.m_frictionERP;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_DEFAULT_GLOBAL_CFM)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_globalCfm = clientCmd.m_physSimParamArgs.m_defaultGlobalCFM;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_DEFAULT_FRICTION_CFM)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_frictionCFM = clientCmd.m_physSimParamArgs.m_frictionCFM;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_SPARSE_SDF)
    {
#ifndef SKIP_DEFORMABLE_BODY
        {
            DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
            if (deformWorld)
            {
                deformWorld->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(clientCmd.m_physSimParamArgs.m_sparseSdfVoxelSize);
                deformWorld->getWorldInfo().m_sparsesdf.Reset();
            }
        }
#endif
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        {
            SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
            if (softWorld)
            {
                softWorld->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(clientCmd.m_physSimParamArgs.m_sparseSdfVoxelSize);
                softWorld->getWorldInfo().m_sparsesdf.Reset();
            }
        }
#endif
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_RESTITUTION_VELOCITY_THRESHOLD)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_restitutionVelocityThreshold = clientCmd.m_physSimParamArgs.m_restitutionVelocityThreshold;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_ENABLE_FILE_CACHING)
    {
        b3EnableFileCaching(clientCmd.m_physSimParamArgs.m_enableFileCaching);
        m_data->m_pluginManager.getFileIOInterface()->enableFileCaching(clientCmd.m_physSimParamArgs.m_enableFileCaching != 0);
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_REPORT_CONSTRAINT_SOLVER_ANALYTICS)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_reportSolverAnalytics = clientCmd.m_physSimParamArgs.m_reportSolverAnalytics;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_WARM_STARTING_FACTOR)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_warmstartingFactor = clientCmd.m_physSimParamArgs.m_warmStartingFactor;
    }

    if (clientCmd.m_updateFlags & SIM_PARAM_UPDATE_ARTICULATED_WARM_STARTING_FACTOR)
    {
        m_data->m_dynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_USE_ARTICULATED_WARMSTARTING;
        m_data->m_dynamicsWorld->getSolverInfo().m_articulatedWarmstartingFactor = clientCmd.m_physSimParamArgs.m_articulatedWarmStartingFactor;
    }
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processInitPoseCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_INIT_POSE");

    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("Server Init Pose not implemented yet");
    }
    i32 bodyUniqueId = clientCmd.m_initPoseArgs.m_bodyUniqueId;
    InternalBodyData* body = m_data->m_bodyHandles.getHandle(bodyUniqueId);

    Vec3 baseLinVel(0, 0, 0);
    Vec3 baseAngVel(0, 0, 0);

    if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_LINEAR_VELOCITY)
    {
        baseLinVel.setVal(clientCmd.m_initPoseArgs.m_initialStateQdot[0],
                            clientCmd.m_initPoseArgs.m_initialStateQdot[1],
                            clientCmd.m_initPoseArgs.m_initialStateQdot[2]);
    }
    if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_ANGULAR_VELOCITY)
    {
        baseAngVel.setVal(clientCmd.m_initPoseArgs.m_initialStateQdot[3],
                            clientCmd.m_initPoseArgs.m_initialStateQdot[4],
                            clientCmd.m_initPoseArgs.m_initialStateQdot[5]);
    }
    Vec3 basePos(0, 0, 0);
    if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_POSITION)
    {
        basePos = Vec3(
            clientCmd.m_initPoseArgs.m_initialStateQ[0],
            clientCmd.m_initPoseArgs.m_initialStateQ[1],
            clientCmd.m_initPoseArgs.m_initialStateQ[2]);
    }
    Quat baseOrn(0, 0, 0, 1);
    if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_ORIENTATION)
    {
        baseOrn.setVal(clientCmd.m_initPoseArgs.m_initialStateQ[3],
                         clientCmd.m_initPoseArgs.m_initialStateQ[4],
                         clientCmd.m_initPoseArgs.m_initialStateQ[5],
                         clientCmd.m_initPoseArgs.m_initialStateQ[6]);
    }
    if (body && body->m_multiBody)
    {
        MultiBody* mb = body->m_multiBody;

        if (clientCmd.m_updateFlags & INIT_POSE_HAS_SCALING)
        {
            Vec3 scaling(clientCmd.m_initPoseArgs.m_scaling[0], clientCmd.m_initPoseArgs.m_scaling[1], clientCmd.m_initPoseArgs.m_scaling[2]);

            mb->getBaseCollider()->getCollisionShape()->setLocalScaling(scaling);
            //refresh broadphase
            m_data->m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(
                mb->getBaseCollider()->getBroadphaseHandle(),
                m_data->m_dynamicsWorld->getDispatcher());
            //also visuals
            i32 graphicsIndex = mb->getBaseCollider()->getUserIndex();
            m_data->m_guiHelper->changeScaling(graphicsIndex, clientCmd.m_initPoseArgs.m_scaling);
        }

        if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_LINEAR_VELOCITY)
        {
            mb->setBaseVel(baseLinVel);
        }

        if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_ANGULAR_VELOCITY)
        {
            mb->setBaseOmega(baseAngVel);
        }

        if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_POSITION)
        {
            Vec3 zero(0, 0, 0);
            Assert(clientCmd.m_initPoseArgs.m_hasInitialStateQ[0] &&
                     clientCmd.m_initPoseArgs.m_hasInitialStateQ[1] &&
                     clientCmd.m_initPoseArgs.m_hasInitialStateQ[2]);

            mb->setBaseVel(baseLinVel);
            mb->setBasePos(basePos);
        }
        if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_ORIENTATION)
        {
            Assert(clientCmd.m_initPoseArgs.m_hasInitialStateQ[3] &&
                     clientCmd.m_initPoseArgs.m_hasInitialStateQ[4] &&
                     clientCmd.m_initPoseArgs.m_hasInitialStateQ[5] &&
                     clientCmd.m_initPoseArgs.m_hasInitialStateQ[6]);

            mb->setBaseOmega(baseAngVel);
            Quat invOrn(baseOrn);

            mb->setWorldToBaseRot(invOrn.inverse());
        }
        if (clientCmd.m_updateFlags & INIT_POSE_HAS_JOINT_STATE)
        {
            i32 uDofIndex = 6;
            i32 posVarCountIndex = 7;
            for (i32 i = 0; i < mb->getNumLinks(); i++)
            {
                i32 posVarCount = mb->getLink(i).m_posVarCount;
                bool hasPosVar = posVarCount > 0;

                for (i32 j = 0; j < posVarCount; j++)
                {
                    if (clientCmd.m_initPoseArgs.m_hasInitialStateQ[posVarCountIndex + j] == 0)
                    {
                        hasPosVar = false;
                        break;
                    }
                }
                if (hasPosVar)
                {
                    if (mb->getLink(i).m_dofCount == 1)
                    {
                        mb->setJointPos(i, clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex]);
                        mb->setJointVel(i, 0);  //backwards compatibility
                    }
                    if (mb->getLink(i).m_dofCount == 3)
                    {
                        Quat q(
                            clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex],
                            clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex + 1],
                            clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex + 2],
                            clientCmd.m_initPoseArgs.m_initialStateQ[posVarCountIndex + 3]);
                        q.normalize();
                        mb->setJointPosMultiDof(i, &q[0]);
                        double vel[6] = {0, 0, 0, 0, 0, 0};
                        mb->setJointVelMultiDof(i, vel);
                    }
                }

                bool hasVel = true;
                for (i32 j = 0; j < mb->getLink(i).m_dofCount; j++)
                {
                    if (clientCmd.m_initPoseArgs.m_hasInitialStateQdot[uDofIndex + j] == 0)
                    {
                        hasVel = false;
                        break;
                    }
                }

                if (hasVel)
                {
                    if (mb->getLink(i).m_dofCount == 1)
                    {
                        Scalar vel = clientCmd.m_initPoseArgs.m_initialStateQdot[uDofIndex];
                        mb->setJointVel(i, vel);
                    }
                    if (mb->getLink(i).m_dofCount == 3)
                    {
                        mb->setJointVelMultiDof(i, &clientCmd.m_initPoseArgs.m_initialStateQdot[uDofIndex]);
                    }
                }

                posVarCountIndex += mb->getLink(i).m_posVarCount;
                uDofIndex += mb->getLink(i).m_dofCount;
            }
        }

        AlignedObjectArray<Quat> scratch_q;
        AlignedObjectArray<Vec3> scratch_m;

        mb->forwardKinematics(scratch_q, scratch_m);
        i32 nLinks = mb->getNumLinks();
        scratch_q.resize(nLinks + 1);
        scratch_m.resize(nLinks + 1);

        mb->updateCollisionObjectWorldTransforms(scratch_q, scratch_m);

        m_data->m_dynamicsWorld->updateSingleAabb(mb->getBaseCollider());
        for (i32 i=0;i<mb->getNumLinks();i++)
        {
            m_data->m_dynamicsWorld->updateSingleAabb(mb->getLinkCollider(i));
        }
    }

    if (body && body->m_rigidBody)
    {
        if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_LINEAR_VELOCITY)
        {
            body->m_rigidBody->setLinearVelocity(baseLinVel);
        }
        if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_ANGULAR_VELOCITY)
        {
            body->m_rigidBody->setAngularVelocity(baseAngVel);
        }

        if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_POSITION)
        {
            body->m_rigidBody->getWorldTransform().setOrigin(basePos);
            body->m_rigidBody->setLinearVelocity(baseLinVel);
        }

        if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_ORIENTATION)
        {
            body->m_rigidBody->getWorldTransform().setRotation(baseOrn);
            body->m_rigidBody->setAngularVelocity(baseAngVel);
        }
        m_data->m_dynamicsWorld->updateSingleAabb(body->m_rigidBody);
    }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    if (body && body->m_softBody)
    {
        if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_LINEAR_VELOCITY)
        {
            body->m_softBody->setLinearVelocity(baseLinVel);
        }
        if (clientCmd.m_updateFlags & INIT_POSE_HAS_BASE_ANGULAR_VELOCITY)
        {
            body->m_softBody->setAngularVelocity(baseAngVel);
        }
        if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_POSITION || clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_ORIENTATION)
        {
            Transform2 tr;
            tr.setIdentity();
            if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_POSITION)
            {
                tr.setOrigin(basePos);
            }
            if (clientCmd.m_updateFlags & INIT_POSE_HAS_INITIAL_ORIENTATION)
            {
                tr.setRotation(baseOrn);
            }
            body->m_softBody->transformTo(tr);
        }
        m_data->m_dynamicsWorld->updateSingleAabb(body->m_softBody);
    }
#endif
    syncPhysicsToGraphics2();

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processResetSimulationCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_RESET_SIMULATION");
    m_data->m_guiHelper->setVisualizerFlag(COV_ENABLE_SYNC_RENDERING_INTERNAL, 0);

    resetSimulation(clientCmd.m_updateFlags);
    m_data->m_guiHelper->setVisualizerFlag(COV_ENABLE_SYNC_RENDERING_INTERNAL, 1);

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_RESET_SIMULATION_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCreateRigidBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_RIGID_BODY_CREATION_COMPLETED;

    DRX3D_PROFILE("CMD_CREATE_RIGID_BODY");

    Vec3 halfExtents(1, 1, 1);
    if (clientCmd.m_updateFlags & BOX_SHAPE_HAS_HALF_EXTENTS)
    {
        halfExtents = Vec3(
            clientCmd.m_createBoxShapeArguments.m_halfExtentsX,
            clientCmd.m_createBoxShapeArguments.m_halfExtentsY,
            clientCmd.m_createBoxShapeArguments.m_halfExtentsZ);
    }
    Transform2 startTrans;
    startTrans.setIdentity();
    if (clientCmd.m_updateFlags & BOX_SHAPE_HAS_INITIAL_POSITION)
    {
        startTrans.setOrigin(Vec3(
            clientCmd.m_createBoxShapeArguments.m_initialPosition[0],
            clientCmd.m_createBoxShapeArguments.m_initialPosition[1],
            clientCmd.m_createBoxShapeArguments.m_initialPosition[2]));
    }

    if (clientCmd.m_updateFlags & BOX_SHAPE_HAS_INITIAL_ORIENTATION)
    {
        startTrans.setRotation(Quat(
            clientCmd.m_createBoxShapeArguments.m_initialOrientation[0],
            clientCmd.m_createBoxShapeArguments.m_initialOrientation[1],
            clientCmd.m_createBoxShapeArguments.m_initialOrientation[2],
            clientCmd.m_createBoxShapeArguments.m_initialOrientation[3]));
    }

    Scalar mass = 0.f;
    if (clientCmd.m_updateFlags & BOX_SHAPE_HAS_MASS)
    {
        mass = clientCmd.m_createBoxShapeArguments.m_mass;
    }

    i32 shapeType = COLLISION_SHAPE_TYPE_BOX;

    if (clientCmd.m_updateFlags & BOX_SHAPE_HAS_COLLISION_SHAPE_TYPE)
    {
        shapeType = clientCmd.m_createBoxShapeArguments.m_collisionShapeType;
    }

#ifdef USE_DISCRETE_DYNAMICS_WORLD
    btWorldImporter* worldImporter = new btWorldImporter(m_data->m_dynamicsWorld);
#else
    MultiBodyWorldImporter* worldImporter = new MultiBodyWorldImporter(m_data->m_dynamicsWorld);
#endif
    m_data->m_worldImporters.push_back(worldImporter);

    CollisionShape* shape = 0;

    switch (shapeType)
    {
        case COLLISION_SHAPE_TYPE_CYLINDER_X:
        {
            Scalar radius = halfExtents[1];
            Scalar height = halfExtents[0];
            shape = worldImporter->createCylinderShapeX(radius, height);
            break;
        }
        case COLLISION_SHAPE_TYPE_CYLINDER_Y:
        {
            Scalar radius = halfExtents[0];
            Scalar height = halfExtents[1];
            shape = worldImporter->createCylinderShapeY(radius, height);
            break;
        }
        case COLLISION_SHAPE_TYPE_CYLINDER_Z:
        {
            Scalar radius = halfExtents[1];
            Scalar height = halfExtents[2];
            shape = worldImporter->createCylinderShapeZ(radius, height);
            break;
        }
        case COLLISION_SHAPE_TYPE_CAPSULE_X:
        {
            Scalar radius = halfExtents[1];
            Scalar height = halfExtents[0];
            shape = worldImporter->createCapsuleShapeX(radius, height);
            break;
        }
        case COLLISION_SHAPE_TYPE_CAPSULE_Y:
        {
            Scalar radius = halfExtents[0];
            Scalar height = halfExtents[1];
            shape = worldImporter->createCapsuleShapeY(radius, height);
            break;
        }
        case COLLISION_SHAPE_TYPE_CAPSULE_Z:
        {
            Scalar radius = halfExtents[1];
            Scalar height = halfExtents[2];
            shape = worldImporter->createCapsuleShapeZ(radius, height);
            break;
        }
        case COLLISION_SHAPE_TYPE_SPHERE:
        {
            Scalar radius = halfExtents[0];
            shape = worldImporter->createSphereShape(radius);
            break;
        }
        case COLLISION_SHAPE_TYPE_BOX:
        default:
        {
            shape = worldImporter->createBoxShape(halfExtents);
        }
    }

    bool isDynamic = (mass > 0);
    RigidBody* rb = worldImporter->createRigidBody(isDynamic, mass, startTrans, shape, 0);
    //m_data->m_guiHelper->autogenerateGraphicsObjects(this->m_data->m_dynamicsWorld);
    Vec4 colorRGBA(1, 0, 0, 1);
    if (clientCmd.m_updateFlags & BOX_SHAPE_HAS_COLOR)
    {
        colorRGBA[0] = clientCmd.m_createBoxShapeArguments.m_colorRGBA[0];
        colorRGBA[1] = clientCmd.m_createBoxShapeArguments.m_colorRGBA[1];
        colorRGBA[2] = clientCmd.m_createBoxShapeArguments.m_colorRGBA[2];
        colorRGBA[3] = clientCmd.m_createBoxShapeArguments.m_colorRGBA[3];
    }
    m_data->m_guiHelper->createCollisionShapeGraphicsObject(rb->getCollisionShape());
    m_data->m_guiHelper->createCollisionObjectGraphicsObject(rb, colorRGBA);

    i32 bodyUniqueId = m_data->m_bodyHandles.allocHandle();
    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);
    serverCmd.m_rigidBodyCreateArgs.m_bodyUniqueId = bodyUniqueId;
    rb->setUserIndex2(bodyUniqueId);
    bodyHandle->m_rootLocalInertialFrame.setIdentity();
    bodyHandle->m_rigidBody = rb;

    b3Notification notification;
    notification.m_notificationType = BODY_ADDED;
    notification.m_bodyArgs.m_bodyUniqueId = bodyUniqueId;
    m_data->m_pluginManager.addNotification(notification);

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processPickBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_PICK_BODY");

    pickBody(Vec3(clientCmd.m_pickBodyArguments.m_rayFromWorld[0],
                       clientCmd.m_pickBodyArguments.m_rayFromWorld[1],
                       clientCmd.m_pickBodyArguments.m_rayFromWorld[2]),
             Vec3(clientCmd.m_pickBodyArguments.m_rayToWorld[0],
                       clientCmd.m_pickBodyArguments.m_rayToWorld[1],
                       clientCmd.m_pickBodyArguments.m_rayToWorld[2]));

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processMovePickedBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_MOVE_PICKED_BODY");

    movePickedBody(Vec3(clientCmd.m_pickBodyArguments.m_rayFromWorld[0],
                             clientCmd.m_pickBodyArguments.m_rayFromWorld[1],
                             clientCmd.m_pickBodyArguments.m_rayFromWorld[2]),
                   Vec3(clientCmd.m_pickBodyArguments.m_rayToWorld[0],
                             clientCmd.m_pickBodyArguments.m_rayToWorld[1],
                             clientCmd.m_pickBodyArguments.m_rayToWorld[2]));

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRemovePickingConstraintCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_REMOVE_PICKING_CONSTRAINT_BODY");
    removePickingConstraint();

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestAabbOverlapCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_REQUEST_AABB_OVERLAP");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    i32 curObjectIndex = clientCmd.m_requestOverlappingObjectsArgs.m_startingOverlappingObjectIndex;

    if (0 == curObjectIndex)
    {
        //clientCmd.m_requestContactPointArguments.m_aabbQueryMin
        Vec3 aabbMin, aabbMax;
        aabbMin.setVal(clientCmd.m_requestOverlappingObjectsArgs.m_aabbQueryMin[0],
                         clientCmd.m_requestOverlappingObjectsArgs.m_aabbQueryMin[1],
                         clientCmd.m_requestOverlappingObjectsArgs.m_aabbQueryMin[2]);
        aabbMax.setVal(clientCmd.m_requestOverlappingObjectsArgs.m_aabbQueryMax[0],
                         clientCmd.m_requestOverlappingObjectsArgs.m_aabbQueryMax[1],
                         clientCmd.m_requestOverlappingObjectsArgs.m_aabbQueryMax[2]);

        m_data->m_cachedOverlappingObjects.clear();

        m_data->m_dynamicsWorld->getBroadphase()->aabbTest(aabbMin, aabbMax, m_data->m_cachedOverlappingObjects);
    }

    i32 totalBytesPerObject = sizeof(b3OverlappingObject);
    i32 overlapCapacity = bufferSizeInBytes / totalBytesPerObject - 1;
    i32 numOverlap = m_data->m_cachedOverlappingObjects.m_bodyUniqueIds.size();
    i32 remainingObjects = numOverlap - curObjectIndex;

    i32 curNumObjects = d3Min(overlapCapacity, remainingObjects);

    if (numOverlap < overlapCapacity)
    {
        b3OverlappingObject* overlapStorage = (b3OverlappingObject*)bufferServerToClient;
        for (i32 i = 0; i < m_data->m_cachedOverlappingObjects.m_bodyUniqueIds.size(); i++)
        {
            overlapStorage[i].m_objectUniqueId = m_data->m_cachedOverlappingObjects.m_bodyUniqueIds[i];
            overlapStorage[i].m_linkIndex = m_data->m_cachedOverlappingObjects.m_links[i];
        }
        serverCmd.m_numDataStreamBytes = numOverlap * totalBytesPerObject;
        serverCmd.m_type = CMD_REQUEST_AABB_OVERLAP_COMPLETED;

        //i32 m_startingOverlappingObjectIndex;
        //i32 m_numOverlappingObjectsCopied;
        //i32 m_numRemainingOverlappingObjects;
        serverCmd.m_sendOverlappingObjectsArgs.m_startingOverlappingObjectIndex = clientCmd.m_requestOverlappingObjectsArgs.m_startingOverlappingObjectIndex;
        serverCmd.m_sendOverlappingObjectsArgs.m_numOverlappingObjectsCopied = m_data->m_cachedOverlappingObjects.m_bodyUniqueIds.size();
        serverCmd.m_sendOverlappingObjectsArgs.m_numRemainingOverlappingObjects = remainingObjects - curNumObjects;
    }
    else
    {
        serverCmd.m_type = CMD_REQUEST_AABB_OVERLAP_FAILED;
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRequestOpenGLVisualizeCameraCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_REQUEST_OPENGL_VISUALIZER_CAMERA");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    bool result = this->m_data->m_guiHelper->getCameraInfo(
        &serverCmd.m_visualizerCameraResultArgs.m_width,
        &serverCmd.m_visualizerCameraResultArgs.m_height,
        serverCmd.m_visualizerCameraResultArgs.m_viewMatrix,
        serverCmd.m_visualizerCameraResultArgs.m_projectionMatrix,
        serverCmd.m_visualizerCameraResultArgs.m_camUp,
        serverCmd.m_visualizerCameraResultArgs.m_camForward,
        serverCmd.m_visualizerCameraResultArgs.m_horizontal,
        serverCmd.m_visualizerCameraResultArgs.m_vertical,
        &serverCmd.m_visualizerCameraResultArgs.m_yaw,
        &serverCmd.m_visualizerCameraResultArgs.m_pitch,
        &serverCmd.m_visualizerCameraResultArgs.m_dist,
        serverCmd.m_visualizerCameraResultArgs.m_target);
    serverCmd.m_type = result ? CMD_REQUEST_OPENGL_VISUALIZER_CAMERA_COMPLETED : CMD_REQUEST_OPENGL_VISUALIZER_CAMERA_FAILED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processConfigureOpenGLVisualizerCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_CONFIGURE_OPENGL_VISUALIZER");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;

    hasStatus = true;

    if (clientCmd.m_updateFlags & COV_SET_FLAGS)
    {
        if (clientCmd.m_configureOpenGLVisualizerArguments.m_setFlag == COV_ENABLE_TINY_RENDERER)
        {
            m_data->m_enableTinyRenderer = clientCmd.m_configureOpenGLVisualizerArguments.m_setEnabled != 0;
        }
        m_data->m_guiHelper->setVisualizerFlag(clientCmd.m_configureOpenGLVisualizerArguments.m_setFlag,
                                               clientCmd.m_configureOpenGLVisualizerArguments.m_setEnabled);
    }
    if (clientCmd.m_updateFlags & COV_SET_CAMERA_VIEW_MATRIX)
    {
        m_data->m_guiHelper->resetCamera(clientCmd.m_configureOpenGLVisualizerArguments.m_cameraDistance,
                                         clientCmd.m_configureOpenGLVisualizerArguments.m_cameraYaw,
                                         clientCmd.m_configureOpenGLVisualizerArguments.m_cameraPitch,
                                         clientCmd.m_configureOpenGLVisualizerArguments.m_cameraTargetPosition[0],
                                         clientCmd.m_configureOpenGLVisualizerArguments.m_cameraTargetPosition[1],
                                         clientCmd.m_configureOpenGLVisualizerArguments.m_cameraTargetPosition[2]);
    }
    if (m_data->m_guiHelper->getRenderInterface())
    {
        if (clientCmd.m_updateFlags & COV_SET_LIGHT_POSITION)
        {
            m_data->m_guiHelper->getRenderInterface()->setLightPosition(clientCmd.m_configureOpenGLVisualizerArguments.m_lightPosition);
        }
        if (clientCmd.m_updateFlags & COV_SET_RGB_BACKGROUND)
        {
            m_data->m_guiHelper->setBackgroundColor(clientCmd.m_configureOpenGLVisualizerArguments.m_rgbBackground);
        }
        if (clientCmd.m_updateFlags & COV_SET_SHADOWMAP_RESOLUTION)
        {
            m_data->m_guiHelper->getRenderInterface()->setShadowMapResolution(clientCmd.m_configureOpenGLVisualizerArguments.m_shadowMapResolution);
        }

        if (clientCmd.m_updateFlags & COV_SET_SHADOWMAP_INTENSITY)
        {
            m_data->m_guiHelper->getRenderInterface()->setShadowMapIntensity(clientCmd.m_configureOpenGLVisualizerArguments.m_shadowMapIntensity);
        }


        if (clientCmd.m_updateFlags & COV_SET_SHADOWMAP_WORLD_SIZE)
        {
            float worldSize = clientCmd.m_configureOpenGLVisualizerArguments.m_shadowMapWorldSize;
            m_data->m_guiHelper->getRenderInterface()->setShadowMapWorldSize(worldSize);
        }
    }

    if (clientCmd.m_updateFlags & COV_SET_REMOTE_SYNC_TRANSFORM_INTERVAL)
    {
        m_data->m_remoteSyncTransformInterval = clientCmd.m_configureOpenGLVisualizerArguments.m_remoteSyncTransformInterval;
    }

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processdrx3d_inverseCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_CALCULATE_INVERSE_DYNAMICS");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_calculatedrx3d_inverseArguments.m_bodyUniqueId);
    serverCmd.m_type = CMD_CALCULATED_INVERSE_DYNAMICS_FAILED;
    if (bodyHandle && bodyHandle->m_multiBody)
    {
        if (clientCmd.m_calculatedrx3d_inverseArguments.m_flags & 1)
        {
#ifdef STATIC_LINK_SPD_PLUGIN
            {
                cRBDModel* rbdModel = m_data->findOrCreateRBDModel(bodyHandle->m_multiBody,
                                                                   clientCmd.m_calculatedrx3d_inverseArguments.m_jointPositionsQ,
                                                                   clientCmd.m_calculatedrx3d_inverseArguments.m_jointVelocitiesQdot);
                if (rbdModel)
                {
                    i32 posVal = bodyHandle->m_multiBody->getNumPosVars();

                    Eigen::VectorXd acc2 = Eigen::VectorXd::Zero(7 + posVal);
                    Eigen::VectorXd out_tau = Eigen::VectorXd::Zero(7 + posVal);
                    cRBDUtil::SolveInvDyna(*rbdModel, acc2, out_tau);
                    i32 dof = 7 + bodyHandle->m_multiBody->getNumPosVars();
                    for (i32 i = 0; i < dof; i++)
                    {
                        serverCmd.m_inverseDynamicsResultArgs.m_jointForces[i] = out_tau[i];
                    }
                    serverCmd.m_inverseDynamicsResultArgs.m_bodyUniqueId = clientCmd.m_calculatedrx3d_inverseArguments.m_bodyUniqueId;
                    serverCmd.m_inverseDynamicsResultArgs.m_dofCount = dof;

                    serverCmd.m_type = CMD_CALCULATED_INVERSE_DYNAMICS_COMPLETED;
                }
            }
#endif
        }
        else
        {
            drx3d_inverse::MultiBodyTree* tree = m_data->findOrCreateTree(bodyHandle->m_multiBody);

            i32 baseDofQ = bodyHandle->m_multiBody->hasFixedBase() ? 0 : 7;
            i32 baseDofQdot = bodyHandle->m_multiBody->hasFixedBase() ? 0 : 6;
            i32k num_dofs = bodyHandle->m_multiBody->getNumDofs();

            if (tree && clientCmd.m_calculatedrx3d_inverseArguments.m_dofCountQ == (baseDofQ + num_dofs) &&
                clientCmd.m_calculatedrx3d_inverseArguments.m_dofCountQdot == (baseDofQdot + num_dofs))
            {
                drx3d_inverse::vecx nu(num_dofs + baseDofQdot), qdot(num_dofs + baseDofQdot), q(num_dofs + baseDofQdot), joint_force(num_dofs + baseDofQdot);

                //for floating base, inverse dynamics expects euler angle x,y,z and position x,y,z in that order
                //PyBullet expects xyz and quaternion in that order, so convert and swap to have a more consistent PyBullet API
                if (baseDofQ)
                {
                    Vec3 pos(clientCmd.m_calculatedrx3d_inverseArguments.m_jointPositionsQ[0],
                                  clientCmd.m_calculatedrx3d_inverseArguments.m_jointPositionsQ[1],
                                  clientCmd.m_calculatedrx3d_inverseArguments.m_jointPositionsQ[2]);

                    Quat orn(clientCmd.m_calculatedrx3d_inverseArguments.m_jointPositionsQ[3],
                                     clientCmd.m_calculatedrx3d_inverseArguments.m_jointPositionsQ[4],
                                     clientCmd.m_calculatedrx3d_inverseArguments.m_jointPositionsQ[5],
                                     clientCmd.m_calculatedrx3d_inverseArguments.m_jointPositionsQ[6]);
                    Scalar yawZ, pitchY, rollX;
                    orn.getEulerZYX(yawZ, pitchY, rollX);
                    q[0] = rollX;
                    q[1] = pitchY;
                    q[2] = yawZ;
                    q[3] = pos[0];
                    q[4] = pos[1];
                    q[5] = pos[2];
                }
                for (i32 i = 0; i < num_dofs; i++)
                {
                    q[i + baseDofQ] = clientCmd.m_calculatedrx3d_inverseArguments.m_jointPositionsQ[i + baseDofQ];
                }
                for (i32 i = 0; i < num_dofs + baseDofQdot; i++)
                {
                    qdot[i] = clientCmd.m_calculatedrx3d_inverseArguments.m_jointVelocitiesQdot[i];
                    nu[i] = clientCmd.m_calculatedrx3d_inverseArguments.m_jointAccelerations[i];
                }

                // Set the gravity to correspond to the world gravity
                drx3d_inverse::vec3 id_grav(m_data->m_dynamicsWorld->getGravity());

                if (-1 != tree->setGravityInWorldFrame(id_grav) &&
                    -1 != tree->calculatedrx3d_inverse(q, qdot, nu, &joint_force))
                {
                    serverCmd.m_inverseDynamicsResultArgs.m_bodyUniqueId = clientCmd.m_calculatedrx3d_inverseArguments.m_bodyUniqueId;
                    serverCmd.m_inverseDynamicsResultArgs.m_dofCount = num_dofs + baseDofQdot;

                    //inverse dynamics stores angular before linear, swap it to have a consistent PyBullet API.
                    if (baseDofQdot)
                    {
                        serverCmd.m_inverseDynamicsResultArgs.m_jointForces[0] = joint_force[3];
                        serverCmd.m_inverseDynamicsResultArgs.m_jointForces[1] = joint_force[4];
                        serverCmd.m_inverseDynamicsResultArgs.m_jointForces[2] = joint_force[5];
                        serverCmd.m_inverseDynamicsResultArgs.m_jointForces[3] = joint_force[0];
                        serverCmd.m_inverseDynamicsResultArgs.m_jointForces[4] = joint_force[1];
                        serverCmd.m_inverseDynamicsResultArgs.m_jointForces[5] = joint_force[2];
                    }

                    for (i32 i = baseDofQdot; i < num_dofs + baseDofQdot; i++)
                    {
                        serverCmd.m_inverseDynamicsResultArgs.m_jointForces[i] = joint_force[i];
                    }
                    serverCmd.m_type = CMD_CALCULATED_INVERSE_DYNAMICS_COMPLETED;
                }
                else
                {
                    serverCmd.m_type = CMD_CALCULATED_INVERSE_DYNAMICS_FAILED;
                }
            }
        }
    }
    else
    {
        serverCmd.m_type = CMD_CALCULATED_INVERSE_DYNAMICS_FAILED;
    }

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCalculateJacobianCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_CALCULATE_JACOBIAN");

    SharedMemoryStatus& serverCmd = serverStatusOut;
    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_calculateJacobianArguments.m_bodyUniqueId);
    if (bodyHandle && bodyHandle->m_multiBody)
    {
        serverCmd.m_type = CMD_CALCULATED_JACOBIAN_FAILED;

        drx3d_inverse::MultiBodyTree* tree = m_data->findOrCreateTree(bodyHandle->m_multiBody);

        if (tree)
        {
            i32 baseDofs = bodyHandle->m_multiBody->hasFixedBase() ? 0 : 6;
            i32k numDofs = bodyHandle->m_multiBody->getNumDofs();
            drx3d_inverse::vecx q(numDofs + baseDofs);
            drx3d_inverse::vecx qdot(numDofs + baseDofs);
            drx3d_inverse::vecx nu(numDofs + baseDofs);
            drx3d_inverse::vecx joint_force(numDofs + baseDofs);
            for (i32 i = 0; i < numDofs; i++)
            {
                q[i + baseDofs] = clientCmd.m_calculateJacobianArguments.m_jointPositionsQ[i];
                qdot[i + baseDofs] = clientCmd.m_calculateJacobianArguments.m_jointVelocitiesQdot[i];
                nu[i + baseDofs] = clientCmd.m_calculateJacobianArguments.m_jointAccelerations[i];
            }
            // Set the gravity to correspond to the world gravity
            drx3d_inverse::vec3 id_grav(m_data->m_dynamicsWorld->getGravity());
            if (-1 != tree->setGravityInWorldFrame(id_grav) &&
                -1 != tree->calculatedrx3d_inverse(q, qdot, nu, &joint_force))
            {
                serverCmd.m_jacobianResultArgs.m_dofCount = numDofs + baseDofs;
                // Set jacobian value
                tree->calculateJacobians(q);
                drx3d_inverse::mat3x jac_t(3, numDofs + baseDofs);
                drx3d_inverse::mat3x jac_r(3, numDofs + baseDofs);

                // Note that inverse dynamics uses zero-based indexing of bodies, not starting from -1 for the base link.
                tree->getBodyJacobianTrans(clientCmd.m_calculateJacobianArguments.m_linkIndex + 1, &jac_t);
                tree->getBodyJacobianRot(clientCmd.m_calculateJacobianArguments.m_linkIndex + 1, &jac_r);
                // Update the translational jacobian based on the desired local point.
                // v_pt = v_frame + w x pt
                // v_pt = J_t * qd + (J_r * qd) x pt
                // v_pt = J_t * qd - pt x (J_r * qd)
                // v_pt = J_t * qd - pt_x * J_r * qd)
                // v_pt = (J_t - pt_x * J_r) * qd
                // J_t_new = J_t - pt_x * J_r
                drx3d_inverse::vec3 localPosition;
                for (i32 i = 0; i < 3; ++i)
                {
                    localPosition(i) = clientCmd.m_calculateJacobianArguments.m_localPosition[i];
                }
                // Only calculate if the localPosition is non-zero.
                if (drx3d_inverse::maxAbs(localPosition) > 0.0)
                {
                    // Write the localPosition into world coordinates.
                    drx3d_inverse::mat33 world_rotation_body;
                    tree->getBodyTransform(clientCmd.m_calculateJacobianArguments.m_linkIndex + 1, &world_rotation_body);
                    localPosition = world_rotation_body * localPosition;
                    // Correct the translational jacobian.
                    drx3d_inverse::mat33 skewCrossProduct;
                    drx3d_inverse::skew(localPosition, &skewCrossProduct);
                    drx3d_inverse::mat3x jac_l(3, numDofs + baseDofs);
                    drx3d_inverse::mul(skewCrossProduct, jac_r, &jac_l);
                    drx3d_inverse::mat3x jac_t_new(3, numDofs + baseDofs);
                    drx3d_inverse::sub(jac_t, jac_l, &jac_t_new);
                    jac_t = jac_t_new;
                }
                // Fill in the result into the shared memory.
                for (i32 i = 0; i < 3; ++i)
                {
                    for (i32 j = 0; j < (numDofs + baseDofs); ++j)
                    {
                        i32 element = (numDofs + baseDofs) * i + j;
                        serverCmd.m_jacobianResultArgs.m_linearJacobian[element] = jac_t(i, j);
                        serverCmd.m_jacobianResultArgs.m_angularJacobian[element] = jac_r(i, j);
                    }
                }
                serverCmd.m_type = CMD_CALCULATED_JACOBIAN_COMPLETED;
            }
            else
            {
                serverCmd.m_type = CMD_CALCULATED_JACOBIAN_FAILED;
            }
        }
    }
    else
    {
        serverCmd.m_type = CMD_CALCULATED_JACOBIAN_FAILED;
    }

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCalculateMassMatrixCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;
    DRX3D_PROFILE("CMD_CALCULATE_MASS_MATRIX");

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CALCULATED_MASS_MATRIX_FAILED;
    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_calculateMassMatrixArguments.m_bodyUniqueId);
    if (bodyHandle && bodyHandle->m_multiBody)
    {
        if (clientCmd.m_calculateMassMatrixArguments.m_flags & 1)
        {
#ifdef STATIC_LINK_SPD_PLUGIN
            {
                i32 posVal = bodyHandle->m_multiBody->getNumPosVars();
                AlignedObjectArray<double> zeroVel;
                i32 dof = 7 + posVal;
                zeroVel.resize(dof);
                cRBDModel* rbdModel = m_data->findOrCreateRBDModel(bodyHandle->m_multiBody, clientCmd.m_calculateMassMatrixArguments.m_jointPositionsQ,
                                                                   &zeroVel[0]);
                if (rbdModel)
                {
                    //Eigen::MatrixXd out_mass;
                    //cRBDUtil::BuildMassMat(*rbdModel, out_mass);
                    const Eigen::MatrixXd& out_mass = rbdModel->GetMassMat();
                    i32 skipDofs = 0;  // 7 - baseDofQ;
                    i32 totDofs = dof;
                    serverCmd.m_massMatrixResultArgs.m_dofCount = totDofs - skipDofs;
                    // Fill in the result into the shared memory.
                    double* sharedBuf = (double*)bufferServerToClient;
                    i32 sizeInBytes = totDofs * totDofs * sizeof(double);
                    if (sizeInBytes < bufferSizeInBytes)
                    {
                        for (i32 i = skipDofs; i < (totDofs); ++i)
                        {
                            for (i32 j = skipDofs; j < (totDofs); ++j)
                            {
                                i32 element = (totDofs - skipDofs) * (i - skipDofs) + (j - skipDofs);
                                double v = out_mass(i, j);
                                if (i == j && v == 0)
                                {
                                    v = 1;
                                }
                                sharedBuf[element] = v;
                            }
                        }
                        serverCmd.m_type = CMD_CALCULATED_MASS_MATRIX_COMPLETED;
                    }
                }
            }
#endif
        }
        else
        {
            drx3d_inverse::MultiBodyTree* tree = m_data->findOrCreateTree(bodyHandle->m_multiBody);

            if (tree)
            {
                i32 baseDofs = bodyHandle->m_multiBody->hasFixedBase() ? 0 : 6;
                i32k numDofs = bodyHandle->m_multiBody->getNumDofs();
                i32k totDofs = numDofs + baseDofs;
                drx3d_inverse::vecx q(totDofs);
                drx3d_inverse::matxx massMatrix(totDofs, totDofs);
                for (i32 i = 0; i < numDofs; i++)
                {
                    q[i + baseDofs] = clientCmd.m_calculateMassMatrixArguments.m_jointPositionsQ[i];
                }
                if (-1 != tree->calculateMassMatrix(q, &massMatrix))
                {
                    serverCmd.m_massMatrixResultArgs.m_dofCount = totDofs;
                    // Fill in the result into the shared memory.
                    double* sharedBuf = (double*)bufferServerToClient;
                    i32 sizeInBytes = totDofs * totDofs * sizeof(double);
                    if (sizeInBytes < bufferSizeInBytes)
                    {
                        for (i32 i = 0; i < (totDofs); ++i)
                        {
                            for (i32 j = 0; j < (totDofs); ++j)
                            {
                                i32 element = (totDofs)*i + j;

                                sharedBuf[element] = massMatrix(i, j);
                            }
                        }
                        serverCmd.m_numDataStreamBytes = sizeInBytes;
                        serverCmd.m_type = CMD_CALCULATED_MASS_MATRIX_COMPLETED;
                    }
                }
            }
        }
    }
    else
    {
        serverCmd.m_type = CMD_CALCULATED_MASS_MATRIX_FAILED;
    }

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processApplyExternalForceCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_APPLY_EXTERNAL_FORCE");

    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("CMD_APPLY_EXTERNAL_FORCE clientCmd = %d\n", clientCmd.m_sequenceNumber);
    }
    for (i32 i = 0; i < clientCmd.m_externalForceArguments.m_numForcesAndTorques; ++i)
    {
        InternalBodyData* body = m_data->m_bodyHandles.getHandle(clientCmd.m_externalForceArguments.m_bodyUniqueIds[i]);
        bool isLinkFrame = ((clientCmd.m_externalForceArguments.m_forceFlags[i] & EF_LINK_FRAME) != 0);

        if (body && body->m_multiBody)
        {
            MultiBody* mb = body->m_multiBody;

            if ((clientCmd.m_externalForceArguments.m_forceFlags[i] & EF_FORCE) != 0)
            {
                Vec3 tmpForce(clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 0],
                                   clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 1],
                                   clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 2]);
                Vec3 tmpPosition(
                    clientCmd.m_externalForceArguments.m_positions[i * 3 + 0],
                    clientCmd.m_externalForceArguments.m_positions[i * 3 + 1],
                    clientCmd.m_externalForceArguments.m_positions[i * 3 + 2]);

                if (clientCmd.m_externalForceArguments.m_linkIds[i] == -1)
                {
                    Vec3 forceWorld = isLinkFrame ? mb->getBaseWorldTransform().getBasis() * tmpForce : tmpForce;
                    Vec3 relPosWorld = isLinkFrame ? mb->getBaseWorldTransform().getBasis() * tmpPosition : tmpPosition - mb->getBaseWorldTransform().getOrigin();
                    mb->addBaseForce(forceWorld);
                    mb->addBaseTorque(relPosWorld.cross(forceWorld));
                    //drx3DPrintf("apply base force of %f,%f,%f at %f,%f,%f\n", forceWorld[0],forceWorld[1],forceWorld[2],positionLocal[0],positionLocal[1],positionLocal[2]);
                }
                else
                {
                    i32 link = clientCmd.m_externalForceArguments.m_linkIds[i];
                    Vec3 forceWorld = isLinkFrame ? mb->getLink(link).m_cachedWorldTransform.getBasis() * tmpForce : tmpForce;
                    Vec3 relPosWorld = isLinkFrame ? mb->getLink(link).m_cachedWorldTransform.getBasis() * tmpPosition : tmpPosition - mb->getLink(link).m_cachedWorldTransform.getOrigin();
                    mb->addLinkForce(link, forceWorld);
                    mb->addLinkTorque(link, relPosWorld.cross(forceWorld));
                    //drx3DPrintf("apply link force of %f,%f,%f at %f,%f,%f\n", forceWorld[0],forceWorld[1],forceWorld[2], positionLocal[0],positionLocal[1],positionLocal[2]);
                }
            }
            if ((clientCmd.m_externalForceArguments.m_forceFlags[i] & EF_TORQUE) != 0)
            {
                Vec3 tmpTorque(clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 0],
                                    clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 1],
                                    clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 2]);

                if (clientCmd.m_externalForceArguments.m_linkIds[i] == -1)
                {
                    Vec3 torqueWorld = isLinkFrame ? mb->getBaseWorldTransform().getBasis() * tmpTorque : tmpTorque;
                    mb->addBaseTorque(torqueWorld);
                    //drx3DPrintf("apply base torque of %f,%f,%f\n", torqueWorld[0],torqueWorld[1],torqueWorld[2]);
                }
                else
                {
                    i32 link = clientCmd.m_externalForceArguments.m_linkIds[i];
                    Vec3 torqueWorld = isLinkFrame ? mb->getLink(link).m_cachedWorldTransform.getBasis() * tmpTorque : tmpTorque;
                    mb->addLinkTorque(link, torqueWorld);
                    //drx3DPrintf("apply link torque of %f,%f,%f\n", torqueWorld[0],torqueWorld[1],torqueWorld[2]);
                }
            }
        }

        if (body && body->m_rigidBody)
        {
            RigidBody* rb = body->m_rigidBody;
            if ((clientCmd.m_externalForceArguments.m_forceFlags[i] & EF_FORCE) != 0)
            {
                Vec3 tmpForce(clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 0],
                                   clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 1],
                                   clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 2]);
                Vec3 tmpPosition(
                    clientCmd.m_externalForceArguments.m_positions[i * 3 + 0],
                    clientCmd.m_externalForceArguments.m_positions[i * 3 + 1],
                    clientCmd.m_externalForceArguments.m_positions[i * 3 + 2]);

                Vec3 forceWorld = isLinkFrame ? rb->getWorldTransform().getBasis() * tmpForce : tmpForce;
                Vec3 relPosWorld = isLinkFrame ? rb->getWorldTransform().getBasis() * tmpPosition : tmpPosition - rb->getWorldTransform().getOrigin();
                rb->applyForce(forceWorld, relPosWorld);
            }

            if ((clientCmd.m_externalForceArguments.m_forceFlags[i] & EF_TORQUE) != 0)
            {
                Vec3 tmpTorque(clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 0],
                                    clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 1],
                                    clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 2]);

                Vec3 torqueWorld = isLinkFrame ? rb->getWorldTransform().getBasis() * tmpTorque : tmpTorque;
                rb->applyTorque(torqueWorld);
            }
        }

#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        if (body && body->m_softBody)
        {
            SoftBody* sb = body->m_softBody;
            i32 link = clientCmd.m_externalForceArguments.m_linkIds[i];
            if ((clientCmd.m_externalForceArguments.m_forceFlags[i] & EF_FORCE) != 0)
            {
                Vec3 tmpForce(clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 0],
                                   clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 1],
                                   clientCmd.m_externalForceArguments.m_forcesAndTorques[i * 3 + 2]);
                Vec3 tmpPosition(
                    clientCmd.m_externalForceArguments.m_positions[i * 3 + 0],
                    clientCmd.m_externalForceArguments.m_positions[i * 3 + 1],
                    clientCmd.m_externalForceArguments.m_positions[i * 3 + 2]);

                Vec3 forceWorld = isLinkFrame ? sb->getWorldTransform().getBasis() * tmpForce : tmpForce;
                Vec3 relPosWorld = isLinkFrame ? sb->getWorldTransform().getBasis() * tmpPosition : tmpPosition - sb->getWorldTransform().getOrigin();
                if (link >= 0 && link < sb->m_nodes.size())
                {
                    sb->addForce(forceWorld, link);
                }
            }
        }
#endif

    }

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRemoveBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_REMOVE_BODY_FAILED;
    serverCmd.m_removeObjectArgs.m_numBodies = 0;
    serverCmd.m_removeObjectArgs.m_numUserConstraints = 0;

    m_data->m_guiHelper->setVisualizerFlag(COV_ENABLE_SYNC_RENDERING_INTERNAL, 0);

    for (i32 i = 0; i < clientCmd.m_removeObjectArgs.m_numBodies; i++)
    {
        i32 bodyUniqueId = clientCmd.m_removeObjectArgs.m_bodyUniqueIds[i];
        InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);
        if (bodyHandle)
        {
#ifndef USE_DISCRETE_DYNAMICS_WORLD
            if (bodyHandle->m_multiBody)
            {
                serverCmd.m_removeObjectArgs.m_bodyUniqueIds[serverCmd.m_removeObjectArgs.m_numBodies++] = bodyUniqueId;

                if (m_data->m_pickingMultiBodyPoint2Point && m_data->m_pickingMultiBodyPoint2Point->getMultiBodyA() == bodyHandle->m_multiBody)
                {
                    //memory will be deleted in the code that follows
                    m_data->m_pickingMultiBodyPoint2Point = 0;
                }

                //also remove user constraints...
                for (i32 i = m_data->m_dynamicsWorld->getNumMultiBodyConstraints() - 1; i >= 0; i--)
                {
                    MultiBodyConstraint* mbc = m_data->m_dynamicsWorld->getMultiBodyConstraint(i);
                    if ((mbc->getMultiBodyA() == bodyHandle->m_multiBody) || (mbc->getMultiBodyB() == bodyHandle->m_multiBody))
                    {
                        m_data->m_dynamicsWorld->removeMultiBodyConstraint(mbc);

                        //also remove user constraint and submit it as removed
                        for (i32 c = m_data->m_userConstraints.size() - 1; c >= 0; c--)
                        {
                            InteralUserConstraintData* userConstraintPtr = m_data->m_userConstraints.getAtIndex(c);
                            i32 userConstraintKey = m_data->m_userConstraints.getKeyAtIndex(c).getUid1();

                            if (userConstraintPtr->m_mbConstraint == mbc)
                            {
                                m_data->m_userConstraints.remove(userConstraintKey);
                                serverCmd.m_removeObjectArgs.m_userConstraintUniqueIds[serverCmd.m_removeObjectArgs.m_numUserConstraints++] = userConstraintKey;
                            }
                        }

                        delete mbc;
                    }
                }

                if (bodyHandle->m_multiBody->getBaseCollider())
                {
                    if (m_data->m_pluginManager.getRenderInterface())
                    {
                        m_data->m_pluginManager.getRenderInterface()->removeVisualShape(bodyHandle->m_multiBody->getBaseCollider()->getUserIndex3());
                    }
                    m_data->m_dynamicsWorld->removeCollisionObject(bodyHandle->m_multiBody->getBaseCollider());
                    i32 graphicsIndex = bodyHandle->m_multiBody->getBaseCollider()->getUserIndex();
                    m_data->m_guiHelper->removeGraphicsInstance(graphicsIndex);
                    delete bodyHandle->m_multiBody->getBaseCollider();
                }
                for (i32 link = 0; link < bodyHandle->m_multiBody->getNumLinks(); link++)
                {
                    CollisionObject2* colObj = bodyHandle->m_multiBody->getLink(link).m_collider;
                    if (colObj)
                    {
                        if (m_data->m_pluginManager.getRenderInterface())
                        {
                            m_data->m_pluginManager.getRenderInterface()->removeVisualShape(bodyHandle->m_multiBody->getLink(link).m_collider->getUserIndex3());
                        }
                        m_data->m_dynamicsWorld->removeCollisionObject(bodyHandle->m_multiBody->getLink(link).m_collider);
                        i32 graphicsIndex = bodyHandle->m_multiBody->getLink(link).m_collider->getUserIndex();
                        m_data->m_guiHelper->removeGraphicsInstance(graphicsIndex);
                        delete colObj;
                    }
                }
                i32 numCollisionObjects = m_data->m_dynamicsWorld->getNumCollisionObjects();
                m_data->m_dynamicsWorld->removeMultiBody(bodyHandle->m_multiBody);
                numCollisionObjects = m_data->m_dynamicsWorld->getNumCollisionObjects();

                delete bodyHandle->m_multiBody;
                bodyHandle->m_multiBody = 0;
                serverCmd.m_type = CMD_REMOVE_BODY_COMPLETED;
            }
#endif
            if (bodyHandle->m_rigidBody)
            {
                if (m_data->m_pluginManager.getRenderInterface())
                {
                    m_data->m_pluginManager.getRenderInterface()->removeVisualShape(bodyHandle->m_rigidBody->getUserIndex3());
                }
                serverCmd.m_removeObjectArgs.m_bodyUniqueIds[serverCmd.m_removeObjectArgs.m_numBodies++] = bodyUniqueId;

                if (m_data->m_pickedConstraint && m_data->m_pickedBody == bodyHandle->m_rigidBody)
                {
                    m_data->m_pickedConstraint = 0;
                    m_data->m_pickedBody = 0;
                }

                //todo: clear all other remaining data...
                m_data->m_dynamicsWorld->removeRigidBody(bodyHandle->m_rigidBody);
                i32 graphicsInstance = bodyHandle->m_rigidBody->getUserIndex2();
                m_data->m_guiHelper->removeGraphicsInstance(graphicsInstance);
                delete bodyHandle->m_rigidBody;
                bodyHandle->m_rigidBody = 0;
                serverCmd.m_type = CMD_REMOVE_BODY_COMPLETED;
            }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
            if (bodyHandle->m_softBody)
            {
                SoftBody* psb = bodyHandle->m_softBody;
                if (m_data->m_pluginManager.getRenderInterface())
                {
                    m_data->m_pluginManager.getRenderInterface()->removeVisualShape(psb->getUserIndex3());
                }
                serverCmd.m_removeObjectArgs.m_bodyUniqueIds[serverCmd.m_removeObjectArgs.m_numBodies++] = bodyUniqueId;
                SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
                if (softWorld)
                {
                    softWorld->removeSoftBody(psb);
                }
                DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
                if (deformWorld)
                {
                    deformWorld->removeSoftBody(psb);
                }

                i32 graphicsInstance = psb->getUserIndex2();
                m_data->m_guiHelper->removeGraphicsInstance(graphicsInstance);
                delete psb;
                psb = 0;
                serverCmd.m_type = CMD_REMOVE_BODY_COMPLETED;
            }
#endif
            for (i32 i = 0; i < bodyHandle->m_userDataHandles.size(); i++)
            {
                i32 userDataHandle = bodyHandle->m_userDataHandles[i];
                SharedMemoryUserData* userData = m_data->m_userDataHandles.getHandle(userDataHandle);
                m_data->m_userDataHandleLookup.remove(SharedMemoryUserDataHashKey(userData));
                m_data->m_userDataHandles.freeHandle(userDataHandle);
            }
            m_data->m_bodyHandles.freeHandle(bodyUniqueId);
        }
    }

    for (i32 i = 0; i < clientCmd.m_removeObjectArgs.m_numUserCollisionShapes; i++)
    {
        i32 removeCollisionShapeId = clientCmd.m_removeObjectArgs.m_userCollisionShapes[i];
        InternalCollisionShapeHandle* handle = m_data->m_userCollisionShapeHandles.getHandle(removeCollisionShapeId);
        if (handle && handle->m_collisionShape)
        {
            if (handle->m_used)
            {
                drx3DWarning("Don't remove collision shape: it is used.");
            }
            else
            {
                drx3DWarning("TODO: dealloc");
                i32 foundIndex = -1;

                for (i32 i = 0; i < m_data->m_worldImporters.size(); i++)
                {
#ifdef USE_DISCRETE_DYNAMICS_WORLD
                    btWorldImporter* importer = m_data->m_worldImporters[i];
#else
                    MultiBodyWorldImporter* importer = m_data->m_worldImporters[i];
#endif
                    for (i32 c = 0; c < importer->getNumCollisionShapes(); c++)
                    {
                        if (importer->getCollisionShapeByIndex(c) == handle->m_collisionShape)
                        {
                            if ((importer->getNumRigidBodies() == 0) &&
                                (importer->getNumConstraints() == 0))
                            {
                                foundIndex = i;
                                break;
                            }
                        }
                    }
                }
                if (foundIndex >= 0)
                {
#ifdef USE_DISCRETE_DYNAMICS_WORLD
                    btWorldImporter* importer = m_data->m_worldImporters[foundIndex];
#else
                    MultiBodyWorldImporter* importer = m_data->m_worldImporters[foundIndex];
#endif
                    m_data->m_worldImporters.removeAtIndex(foundIndex);
                    importer->deleteAllData();
                    delete importer;
                    m_data->m_userCollisionShapeHandles.freeHandle(removeCollisionShapeId);
                    serverCmd.m_type = CMD_REMOVE_BODY_COMPLETED;
                }
            }
        }
    }

    m_data->m_guiHelper->setVisualizerFlag(COV_ENABLE_SYNC_RENDERING_INTERNAL, 1);

    for (i32 i = 0; i < serverCmd.m_removeObjectArgs.m_numBodies; i++)
    {
        b3Notification notification;
        notification.m_notificationType = BODY_REMOVED;
        notification.m_bodyArgs.m_bodyUniqueId = serverCmd.m_removeObjectArgs.m_bodyUniqueIds[i];
        m_data->m_pluginManager.addNotification(notification);
    }

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCreateUserConstraintCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_USER_CONSTRAINT");

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_USER_CONSTRAINT_FAILED;
    hasStatus = true;
    if (clientCmd.m_updateFlags & USER_CONSTRAINT_ADD_SOFT_BODY_ANCHOR)
    {
#ifndef SKIP_DEFORMABLE_BODY
        InternalBodyHandle* sbodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_userConstraintArguments.m_parentBodyIndex);
        if (sbodyHandle)
        {
            if (sbodyHandle->m_softBody)
            {
                i32 nodeIndex = clientCmd.m_userConstraintArguments.m_parentJointIndex;
                if (nodeIndex >= 0 && nodeIndex < sbodyHandle->m_softBody->m_nodes.size())
                {
                    i32 bodyUniqueId = clientCmd.m_userConstraintArguments.m_childBodyIndex;
                    if (bodyUniqueId < 0)
                    {
                        //fixed anchor (mass = 0)
                        InteralUserConstraintData userConstraintData;
                        userConstraintData.m_sbHandle = clientCmd.m_userConstraintArguments.m_parentBodyIndex;
                        userConstraintData.m_sbNodeIndex = nodeIndex;
                        userConstraintData.m_sbNodeMass = sbodyHandle->m_softBody->getMass(nodeIndex);
                        sbodyHandle->m_softBody->setMass(nodeIndex, 0.0);
                        i32 uid = m_data->m_userConstraintUIDGenerator++;
                        m_data->m_userConstraints.insert(uid, userConstraintData);
                        serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                        serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                    }
                    else
                    {
                        InternalBodyHandle* mbodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);
                        if (mbodyHandle && mbodyHandle->m_multiBody)
                        {
                            DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
                            if (deformWorld)
                            {
                                i32 linkIndex = clientCmd.m_userConstraintArguments.m_childJointIndex;
                                if (linkIndex < 0)
                                {
                                    sbodyHandle->m_softBody->appendDeformableAnchor(nodeIndex, mbodyHandle->m_multiBody->getBaseCollider());
                                }
                                else
                                {
                                    if (linkIndex < mbodyHandle->m_multiBody->getNumLinks())
                                    {
                                        sbodyHandle->m_softBody->appendDeformableAnchor(nodeIndex, mbodyHandle->m_multiBody->getLinkCollider(linkIndex));
                                    }
                                }
                            }
                        }
                        if (mbodyHandle && mbodyHandle->m_rigidBody)
                        {
                            DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
                            if (deformWorld)
                            {
                                //todo: expose those values
                                bool disableCollisionBetweenLinkedBodies = true;
                                //Vec3 localPivot(0,0,0);
                                sbodyHandle->m_softBody->appendDeformableAnchor(nodeIndex, mbodyHandle->m_rigidBody);
                            }

#if 1
                            SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
                            if (softWorld)
                            {
                                bool disableCollisionBetweenLinkedBodies = true;
                                Vec3 localPivot(clientCmd.m_userConstraintArguments.m_childFrame[0],
                                                     clientCmd.m_userConstraintArguments.m_childFrame[1],
                                                     clientCmd.m_userConstraintArguments.m_childFrame[2]);

                                sbodyHandle->m_softBody->appendAnchor(nodeIndex, mbodyHandle->m_rigidBody, localPivot, disableCollisionBetweenLinkedBodies);
                            }
#endif
                        }
                        i32 uid = m_data->m_userConstraintUIDGenerator++;
                        serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                        InteralUserConstraintData userConstraintData;
                        userConstraintData.m_sbHandle = clientCmd.m_userConstraintArguments.m_parentBodyIndex;
                        userConstraintData.m_sbNodeIndex = nodeIndex;
                        m_data->m_userConstraints.insert(uid, userConstraintData);
                        serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                    }
                }
            }
        }
#endif
    }
    if (clientCmd.m_updateFlags & USER_CONSTRAINT_REQUEST_STATE)
    {
        i32 constraintUid = clientCmd.m_userConstraintArguments.m_userConstraintUniqueId;
        InteralUserConstraintData* userConstraintPtr = m_data->m_userConstraints.find(constraintUid);
        if (userConstraintPtr)
        {
            serverCmd.m_userConstraintStateResultArgs.m_numDofs = 0;
            for (i32 i = 0; i < 6; i++)
            {
                serverCmd.m_userConstraintStateResultArgs.m_appliedConstraintForces[i] = 0;
            }
            if (userConstraintPtr->m_mbConstraint)
            {
                serverCmd.m_userConstraintStateResultArgs.m_numDofs = userConstraintPtr->m_mbConstraint->getNumRows();
                for (i32 i = 0; i < userConstraintPtr->m_mbConstraint->getNumRows(); i++)
                {
                    serverCmd.m_userConstraintStateResultArgs.m_appliedConstraintForces[i] = userConstraintPtr->m_mbConstraint->getAppliedImpulse(i) / m_data->m_dynamicsWorld->getSolverInfo().m_timeStep;
                }
                serverCmd.m_type = CMD_USER_CONSTRAINT_REQUEST_STATE_COMPLETED;
            }
        }
    };
    if (clientCmd.m_updateFlags & USER_CONSTRAINT_REQUEST_INFO)
    {
        i32 userConstraintUidChange = clientCmd.m_userConstraintArguments.m_userConstraintUniqueId;
        InteralUserConstraintData* userConstraintPtr = m_data->m_userConstraints.find(userConstraintUidChange);
        if (userConstraintPtr)
        {
            serverCmd.m_userConstraintResultArgs = userConstraintPtr->m_userConstraintData;

            serverCmd.m_type = CMD_USER_CONSTRAINT_INFO_COMPLETED;
        }
    }
    if (clientCmd.m_updateFlags & USER_CONSTRAINT_ADD_CONSTRAINT)
    {
        Scalar defaultMaxForce = 500.0;
        InternalBodyData* parentBody = m_data->m_bodyHandles.getHandle(clientCmd.m_userConstraintArguments.m_parentBodyIndex);
#ifndef USE_DISCRETE_DYNAMICS_WORLD
        if (parentBody && parentBody->m_multiBody)
        {
            if ((clientCmd.m_userConstraintArguments.m_parentJointIndex >= -1) && clientCmd.m_userConstraintArguments.m_parentJointIndex < parentBody->m_multiBody->getNumLinks())
            {
                InternalBodyData* childBody = clientCmd.m_userConstraintArguments.m_childBodyIndex >= 0 ? m_data->m_bodyHandles.getHandle(clientCmd.m_userConstraintArguments.m_childBodyIndex) : 0;
                //also create a constraint with just a single multibody/rigid body without child
                //if (childBody)
                {
                    Vec3 pivotInParent(clientCmd.m_userConstraintArguments.m_parentFrame[0], clientCmd.m_userConstraintArguments.m_parentFrame[1], clientCmd.m_userConstraintArguments.m_parentFrame[2]);
                    Vec3 pivotInChild(clientCmd.m_userConstraintArguments.m_childFrame[0], clientCmd.m_userConstraintArguments.m_childFrame[1], clientCmd.m_userConstraintArguments.m_childFrame[2]);
                    Matrix3x3 frameInParent(Quat(clientCmd.m_userConstraintArguments.m_parentFrame[3], clientCmd.m_userConstraintArguments.m_parentFrame[4], clientCmd.m_userConstraintArguments.m_parentFrame[5], clientCmd.m_userConstraintArguments.m_parentFrame[6]));
                    Matrix3x3 frameInChild(Quat(clientCmd.m_userConstraintArguments.m_childFrame[3], clientCmd.m_userConstraintArguments.m_childFrame[4], clientCmd.m_userConstraintArguments.m_childFrame[5], clientCmd.m_userConstraintArguments.m_childFrame[6]));
                    Vec3 jointAxis(clientCmd.m_userConstraintArguments.m_jointAxis[0], clientCmd.m_userConstraintArguments.m_jointAxis[1], clientCmd.m_userConstraintArguments.m_jointAxis[2]);

                    if (clientCmd.m_userConstraintArguments.m_jointType == eGearType)
                    {
                        if (childBody && childBody->m_multiBody)
                        {
                            if ((clientCmd.m_userConstraintArguments.m_childJointIndex >= -1) && (clientCmd.m_userConstraintArguments.m_childJointIndex < childBody->m_multiBody->getNumLinks()))
                            {
                                MultiBodyGearConstraint* multibodyGear = new MultiBodyGearConstraint(parentBody->m_multiBody, clientCmd.m_userConstraintArguments.m_parentJointIndex, childBody->m_multiBody, clientCmd.m_userConstraintArguments.m_childJointIndex, pivotInParent, pivotInChild, frameInParent, frameInChild);
                                multibodyGear->setMaxAppliedImpulse(defaultMaxForce);
                                m_data->m_dynamicsWorld->addMultiBodyConstraint(multibodyGear);
                                InteralUserConstraintData userConstraintData;
                                userConstraintData.m_mbConstraint = multibodyGear;
                                i32 uid = m_data->m_userConstraintUIDGenerator++;
                                serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                                serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                                serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                                userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                                m_data->m_userConstraints.insert(uid, userConstraintData);
                                serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                            }
                        }
                    }
                    else if (clientCmd.m_userConstraintArguments.m_jointType == eFixedType)
                    {
                        if (childBody && childBody->m_multiBody)
                        {
                            if ((clientCmd.m_userConstraintArguments.m_childJointIndex >= -1) && (clientCmd.m_userConstraintArguments.m_childJointIndex < childBody->m_multiBody->getNumLinks()))
                            {
                                MultiBodyFixedConstraint* multibodyFixed = new MultiBodyFixedConstraint(parentBody->m_multiBody, clientCmd.m_userConstraintArguments.m_parentJointIndex, childBody->m_multiBody, clientCmd.m_userConstraintArguments.m_childJointIndex, pivotInParent, pivotInChild, frameInParent, frameInChild);
                                multibodyFixed->setMaxAppliedImpulse(defaultMaxForce);
                                m_data->m_dynamicsWorld->addMultiBodyConstraint(multibodyFixed);
                                InteralUserConstraintData userConstraintData;
                                userConstraintData.m_mbConstraint = multibodyFixed;
                                i32 uid = m_data->m_userConstraintUIDGenerator++;
                                serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                                serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                                serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                                userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                                m_data->m_userConstraints.insert(uid, userConstraintData);
                                serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                            }
                        }
                        else
                        {
                            RigidBody* rb = childBody ? childBody->m_rigidBody : 0;
                            MultiBodyFixedConstraint* rigidbodyFixed = new MultiBodyFixedConstraint(parentBody->m_multiBody, clientCmd.m_userConstraintArguments.m_parentJointIndex, rb, pivotInParent, pivotInChild, frameInParent, frameInChild);
                            rigidbodyFixed->setMaxAppliedImpulse(defaultMaxForce);
                            MultiBodyDynamicsWorld* world = (MultiBodyDynamicsWorld*)m_data->m_dynamicsWorld;
                            world->addMultiBodyConstraint(rigidbodyFixed);
                            InteralUserConstraintData userConstraintData;
                            userConstraintData.m_mbConstraint = rigidbodyFixed;
                            i32 uid = m_data->m_userConstraintUIDGenerator++;
                            serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                            serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                            serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                            userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                            m_data->m_userConstraints.insert(uid, userConstraintData);
                            serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                        }
                    }
                    else if (clientCmd.m_userConstraintArguments.m_jointType == ePrismaticType)
                    {
                        if (childBody && childBody->m_multiBody)
                        {
                            MultiBodySliderConstraint* multibodySlider = new MultiBodySliderConstraint(parentBody->m_multiBody, clientCmd.m_userConstraintArguments.m_parentJointIndex, childBody->m_multiBody, clientCmd.m_userConstraintArguments.m_childJointIndex, pivotInParent, pivotInChild, frameInParent, frameInChild, jointAxis);
                            multibodySlider->setMaxAppliedImpulse(defaultMaxForce);
                            m_data->m_dynamicsWorld->addMultiBodyConstraint(multibodySlider);
                            InteralUserConstraintData userConstraintData;
                            userConstraintData.m_mbConstraint = multibodySlider;
                            i32 uid = m_data->m_userConstraintUIDGenerator++;
                            serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                            serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                            serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                            userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                            m_data->m_userConstraints.insert(uid, userConstraintData);
                            serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                        }
                        else
                        {
                            RigidBody* rb = childBody ? childBody->m_rigidBody : 0;

                            MultiBodySliderConstraint* rigidbodySlider = new MultiBodySliderConstraint(parentBody->m_multiBody, clientCmd.m_userConstraintArguments.m_parentJointIndex, rb, pivotInParent, pivotInChild, frameInParent, frameInChild, jointAxis);
                            rigidbodySlider->setMaxAppliedImpulse(defaultMaxForce);
                            MultiBodyDynamicsWorld* world = (MultiBodyDynamicsWorld*)m_data->m_dynamicsWorld;
                            world->addMultiBodyConstraint(rigidbodySlider);
                            InteralUserConstraintData userConstraintData;
                            userConstraintData.m_mbConstraint = rigidbodySlider;
                            i32 uid = m_data->m_userConstraintUIDGenerator++;
                            serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                            serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                            serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                            userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                            m_data->m_userConstraints.insert(uid, userConstraintData);
                            serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                        }
                    }
                    else if (clientCmd.m_userConstraintArguments.m_jointType == ePoint2PointType)
                    {
                        if (childBody && childBody->m_multiBody)
                        {
                            MultiBodyPoint2Point* p2p = new MultiBodyPoint2Point(parentBody->m_multiBody, clientCmd.m_userConstraintArguments.m_parentJointIndex, childBody->m_multiBody, clientCmd.m_userConstraintArguments.m_childJointIndex, pivotInParent, pivotInChild);
                            p2p->setMaxAppliedImpulse(defaultMaxForce);
                            m_data->m_dynamicsWorld->addMultiBodyConstraint(p2p);
                            InteralUserConstraintData userConstraintData;
                            userConstraintData.m_mbConstraint = p2p;
                            i32 uid = m_data->m_userConstraintUIDGenerator++;
                            serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                            serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                            serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                            userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                            m_data->m_userConstraints.insert(uid, userConstraintData);
                            serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                        }
                        else
                        {
                            RigidBody* rb = childBody ? childBody->m_rigidBody : 0;

                            MultiBodyPoint2Point* p2p = new MultiBodyPoint2Point(parentBody->m_multiBody, clientCmd.m_userConstraintArguments.m_parentJointIndex, rb, pivotInParent, pivotInChild);
                            p2p->setMaxAppliedImpulse(defaultMaxForce);
                            MultiBodyDynamicsWorld* world = (MultiBodyDynamicsWorld*)m_data->m_dynamicsWorld;
                            world->addMultiBodyConstraint(p2p);
                            InteralUserConstraintData userConstraintData;
                            userConstraintData.m_mbConstraint = p2p;
                            i32 uid = m_data->m_userConstraintUIDGenerator++;
                            serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                            serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                            serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                            userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                            m_data->m_userConstraints.insert(uid, userConstraintData);
                            serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                        }
                    }
                    else
                    {
                        drx3DWarning("unknown constraint type");
                    }
                }
            }
        }
        else
#endif
        {
            InternalBodyData* childBody = clientCmd.m_userConstraintArguments.m_childBodyIndex >= 0 ? m_data->m_bodyHandles.getHandle(clientCmd.m_userConstraintArguments.m_childBodyIndex) : 0;

            if (parentBody && childBody)
            {
                if (parentBody->m_rigidBody)
                {
                    RigidBody* parentRb = 0;
                    if (clientCmd.m_userConstraintArguments.m_parentJointIndex == -1)
                    {
                        parentRb = parentBody->m_rigidBody;
                    }
                    else
                    {
                        if ((clientCmd.m_userConstraintArguments.m_parentJointIndex >= 0) &&
                            (clientCmd.m_userConstraintArguments.m_parentJointIndex < parentBody->m_rigidBodyJoints.size()))
                        {
                            parentRb = &parentBody->m_rigidBodyJoints[clientCmd.m_userConstraintArguments.m_parentJointIndex]->getRigidBodyB();
                        }
                    }

                    RigidBody* childRb = 0;
                    if (childBody->m_rigidBody)
                    {
                        if (clientCmd.m_userConstraintArguments.m_childJointIndex == -1)
                        {
                            childRb = childBody->m_rigidBody;
                        }
                        else
                        {
                            if ((clientCmd.m_userConstraintArguments.m_childJointIndex >= 0) && (clientCmd.m_userConstraintArguments.m_childJointIndex < childBody->m_rigidBodyJoints.size()))
                            {
                                childRb = &childBody->m_rigidBodyJoints[clientCmd.m_userConstraintArguments.m_childJointIndex]->getRigidBodyB();
                            }
                        }
                    }

                    switch (clientCmd.m_userConstraintArguments.m_jointType)
                    {
                        case eRevoluteType:
                        {
                            break;
                        }
                        case ePrismaticType:
                        {
                            break;
                        }

                        case eFixedType:
                        {
                            if (childRb && parentRb && (childRb != parentRb))
                            {
                                Vec3 pivotInParent(clientCmd.m_userConstraintArguments.m_parentFrame[0], clientCmd.m_userConstraintArguments.m_parentFrame[1], clientCmd.m_userConstraintArguments.m_parentFrame[2]);
                                Vec3 pivotInChild(clientCmd.m_userConstraintArguments.m_childFrame[0], clientCmd.m_userConstraintArguments.m_childFrame[1], clientCmd.m_userConstraintArguments.m_childFrame[2]);

                                Transform2 offsetTrA, offsetTrB;
                                offsetTrA.setIdentity();
                                offsetTrA.setOrigin(pivotInParent);
                                offsetTrB.setIdentity();
                                offsetTrB.setOrigin(pivotInChild);

                                Generic6DofSpring2Constraint* dof6 = new Generic6DofSpring2Constraint(*parentRb, *childRb, offsetTrA, offsetTrB);

                                dof6->setLinearLowerLimit(Vec3(0, 0, 0));
                                dof6->setLinearUpperLimit(Vec3(0, 0, 0));

                                dof6->setAngularLowerLimit(Vec3(0, 0, 0));
                                dof6->setAngularUpperLimit(Vec3(0, 0, 0));
                                m_data->m_dynamicsWorld->addConstraint(dof6);
                                InteralUserConstraintData userConstraintData;
                                userConstraintData.m_rbConstraint = dof6;
                                i32 uid = m_data->m_userConstraintUIDGenerator++;
                                serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                                serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                                serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                                userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                                m_data->m_userConstraints.insert(uid, userConstraintData);
                                serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                            }

                            break;
                        }

                        case ePoint2PointType:
                        {
                            if (childRb && parentRb && (childRb != parentRb))
                            {
                                Vec3 pivotInParent(clientCmd.m_userConstraintArguments.m_parentFrame[0], clientCmd.m_userConstraintArguments.m_parentFrame[1], clientCmd.m_userConstraintArguments.m_parentFrame[2]);
                                Vec3 pivotInChild(clientCmd.m_userConstraintArguments.m_childFrame[0], clientCmd.m_userConstraintArguments.m_childFrame[1], clientCmd.m_userConstraintArguments.m_childFrame[2]);

                                Point2PointConstraint* p2p = new Point2PointConstraint(*parentRb, *childRb, pivotInParent, pivotInChild);
                                p2p->m_setting.m_impulseClamp = defaultMaxForce;
                                m_data->m_dynamicsWorld->addConstraint(p2p);
                                InteralUserConstraintData userConstraintData;
                                userConstraintData.m_rbConstraint = p2p;
                                i32 uid = m_data->m_userConstraintUIDGenerator++;
                                serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                                serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                                serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                                userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                                m_data->m_userConstraints.insert(uid, userConstraintData);
                                serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                            }
                            break;
                        }

                        case eGearType:
                        {
                            if (childRb && parentRb && (childRb != parentRb))
                            {
                                Vec3 axisA(clientCmd.m_userConstraintArguments.m_jointAxis[0],
                                                clientCmd.m_userConstraintArguments.m_jointAxis[1],
                                                clientCmd.m_userConstraintArguments.m_jointAxis[2]);
                                //for now we use the same local axis for both objects
                                Vec3 axisB(clientCmd.m_userConstraintArguments.m_jointAxis[0],
                                                clientCmd.m_userConstraintArguments.m_jointAxis[1],
                                                clientCmd.m_userConstraintArguments.m_jointAxis[2]);
                                Scalar ratio = 1;
                                GearConstraint* gear = new GearConstraint(*parentRb, *childRb, axisA, axisB, ratio);
                                m_data->m_dynamicsWorld->addConstraint(gear, true);
                                InteralUserConstraintData userConstraintData;
                                userConstraintData.m_rbConstraint = gear;
                                i32 uid = m_data->m_userConstraintUIDGenerator++;
                                serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
                                serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = uid;
                                serverCmd.m_userConstraintResultArgs.m_maxAppliedForce = defaultMaxForce;
                                userConstraintData.m_userConstraintData = serverCmd.m_userConstraintResultArgs;
                                m_data->m_userConstraints.insert(uid, userConstraintData);
                                serverCmd.m_type = CMD_USER_CONSTRAINT_COMPLETED;
                            }
                            break;
                        }
                        case eSphericalType:
                        {
                            drx3DWarning("constraint type not handled yet");
                            break;
                        }
                        case ePlanarType:
                        {
                            drx3DWarning("constraint type not handled yet");
                            break;
                        }
                        default:
                        {
                            drx3DWarning("unknown constraint type");
                        }
                    };
                }
            }
        }
    }

    if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_CONSTRAINT)
    {
        Scalar fixedTimeSubStep = m_data->m_numSimulationSubSteps > 0 ? m_data->m_physicsDeltaTime / m_data->m_numSimulationSubSteps : m_data->m_physicsDeltaTime;

        serverCmd.m_type = CMD_CHANGE_USER_CONSTRAINT_FAILED;
        i32 userConstraintUidChange = clientCmd.m_userConstraintArguments.m_userConstraintUniqueId;
        InteralUserConstraintData* userConstraintPtr = m_data->m_userConstraints.find(userConstraintUidChange);
        if (userConstraintPtr)
        {
            if (userConstraintPtr->m_mbConstraint)
            {
                if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_PIVOT_IN_B)
                {
                    Vec3 pivotInB(clientCmd.m_userConstraintArguments.m_childFrame[0],
                                       clientCmd.m_userConstraintArguments.m_childFrame[1],
                                       clientCmd.m_userConstraintArguments.m_childFrame[2]);
                    userConstraintPtr->m_userConstraintData.m_childFrame[0] = clientCmd.m_userConstraintArguments.m_childFrame[0];
                    userConstraintPtr->m_userConstraintData.m_childFrame[1] = clientCmd.m_userConstraintArguments.m_childFrame[1];
                    userConstraintPtr->m_userConstraintData.m_childFrame[2] = clientCmd.m_userConstraintArguments.m_childFrame[2];
                    userConstraintPtr->m_mbConstraint->setPivotInB(pivotInB);
                }
                if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_FRAME_ORN_IN_B)
                {
                    Quat childFrameOrn(clientCmd.m_userConstraintArguments.m_childFrame[3],
                                               clientCmd.m_userConstraintArguments.m_childFrame[4],
                                               clientCmd.m_userConstraintArguments.m_childFrame[5],
                                               clientCmd.m_userConstraintArguments.m_childFrame[6]);
                    userConstraintPtr->m_userConstraintData.m_childFrame[3] = clientCmd.m_userConstraintArguments.m_childFrame[3];
                    userConstraintPtr->m_userConstraintData.m_childFrame[4] = clientCmd.m_userConstraintArguments.m_childFrame[4];
                    userConstraintPtr->m_userConstraintData.m_childFrame[5] = clientCmd.m_userConstraintArguments.m_childFrame[5];
                    userConstraintPtr->m_userConstraintData.m_childFrame[6] = clientCmd.m_userConstraintArguments.m_childFrame[6];
                    Matrix3x3 childFrameBasis(childFrameOrn);
                    userConstraintPtr->m_mbConstraint->setFrameInB(childFrameBasis);
                }
                if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_MAX_FORCE)
                {
                    Scalar maxImp = clientCmd.m_userConstraintArguments.m_maxAppliedForce * fixedTimeSubStep;

                    userConstraintPtr->m_userConstraintData.m_maxAppliedForce = clientCmd.m_userConstraintArguments.m_maxAppliedForce;
                    userConstraintPtr->m_mbConstraint->setMaxAppliedImpulse(maxImp);
                }

                if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_GEAR_RATIO)
                {
                    userConstraintPtr->m_mbConstraint->setGearRatio(clientCmd.m_userConstraintArguments.m_gearRatio);
                    userConstraintPtr->m_userConstraintData.m_gearRatio = clientCmd.m_userConstraintArguments.m_gearRatio;
                }
                if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_RELATIVE_POSITION_TARGET)
                {
                    userConstraintPtr->m_mbConstraint->setRelativePositionTarget(clientCmd.m_userConstraintArguments.m_relativePositionTarget);
                    userConstraintPtr->m_userConstraintData.m_relativePositionTarget = clientCmd.m_userConstraintArguments.m_relativePositionTarget;
                }
                if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_ERP)
                {
                    userConstraintPtr->m_mbConstraint->setErp(clientCmd.m_userConstraintArguments.m_erp);
                    userConstraintPtr->m_userConstraintData.m_erp = clientCmd.m_userConstraintArguments.m_erp;
                }

                if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_GEAR_AUX_LINK)
                {
                    userConstraintPtr->m_mbConstraint->setGearAuxLink(clientCmd.m_userConstraintArguments.m_gearAuxLink);
                    userConstraintPtr->m_userConstraintData.m_gearAuxLink = clientCmd.m_userConstraintArguments.m_gearAuxLink;
                }
            }
            if (userConstraintPtr->m_rbConstraint)
            {
                if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_MAX_FORCE)
                {
                    Scalar maxImp = clientCmd.m_userConstraintArguments.m_maxAppliedForce * fixedTimeSubStep;
                    userConstraintPtr->m_userConstraintData.m_maxAppliedForce = clientCmd.m_userConstraintArguments.m_maxAppliedForce;
                    //userConstraintPtr->m_rbConstraint->setMaxAppliedImpulse(maxImp);
                }

                if (clientCmd.m_updateFlags & USER_CONSTRAINT_CHANGE_GEAR_RATIO)
                {
                    if (userConstraintPtr->m_rbConstraint->getObjectType() == GEAR_CONSTRAINT_TYPE)
                    {
                        GearConstraint* gear = (GearConstraint*)userConstraintPtr->m_rbConstraint;
                        gear->setRatio(clientCmd.m_userConstraintArguments.m_gearRatio);
                    }
                }
            }
            serverCmd.m_userConstraintResultArgs = clientCmd.m_userConstraintArguments;
            serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = userConstraintUidChange;
            serverCmd.m_updateFlags = clientCmd.m_updateFlags;
            serverCmd.m_type = CMD_CHANGE_USER_CONSTRAINT_COMPLETED;
        }
    }
    if (clientCmd.m_updateFlags & USER_CONSTRAINT_REMOVE_CONSTRAINT)
    {
        serverCmd.m_type = CMD_REMOVE_USER_CONSTRAINT_FAILED;
        i32 userConstraintUidRemove = clientCmd.m_userConstraintArguments.m_userConstraintUniqueId;
        InteralUserConstraintData* userConstraintPtr = m_data->m_userConstraints.find(userConstraintUidRemove);
        if (userConstraintPtr)
        {
#ifndef USE_DISCRETE_DYNAMICS_WORLD
            if (userConstraintPtr->m_mbConstraint)
            {
                m_data->m_dynamicsWorld->removeMultiBodyConstraint(userConstraintPtr->m_mbConstraint);
                delete userConstraintPtr->m_mbConstraint;
                m_data->m_userConstraints.remove(userConstraintUidRemove);
            }
#endif//USE_DISCRETE_DYNAMICS_WORLD
            if (userConstraintPtr->m_rbConstraint)
            {
                m_data->m_dynamicsWorld->removeConstraint(userConstraintPtr->m_rbConstraint);
                delete userConstraintPtr->m_rbConstraint;
                m_data->m_userConstraints.remove(userConstraintUidRemove);
            }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD

            if (userConstraintPtr->m_sbHandle >= 0)
            {
                InternalBodyHandle* sbodyHandle = m_data->m_bodyHandles.getHandle(userConstraintPtr->m_sbHandle);
                if (sbodyHandle)
                {
                    if (sbodyHandle->m_softBody)
                    {
                        if (userConstraintPtr->m_sbNodeMass >= 0)
                        {
                            sbodyHandle->m_softBody->setMass(userConstraintPtr->m_sbNodeIndex, userConstraintPtr->m_sbNodeMass);
                        }
                        else
                        {
                            sbodyHandle->m_softBody->removeAnchor(userConstraintPtr->m_sbNodeIndex);
                        }
                    }
                }
            }
#endif
            serverCmd.m_userConstraintResultArgs.m_userConstraintUniqueId = userConstraintUidRemove;
            serverCmd.m_type = CMD_REMOVE_USER_CONSTRAINT_COMPLETED;
        }
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCalculateInverseKinematicsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_CALCULATE_INVERSE_KINEMATICS");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CALCULATE_INVERSE_KINEMATICS_FAILED;

    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_calculateInverseKinematicsArguments.m_bodyUniqueId);
    if (bodyHandle && bodyHandle->m_multiBody)
    {
        IKTrajectoryHelper** ikHelperPtrPtr = m_data->m_inverseKinematicsHelpers.find(bodyHandle->m_multiBody);
        IKTrajectoryHelper* ikHelperPtr = 0;

        if (ikHelperPtrPtr)
        {
            ikHelperPtr = *ikHelperPtrPtr;
        }
        else
        {
            IKTrajectoryHelper* tmpHelper = new IKTrajectoryHelper;
            m_data->m_inverseKinematicsHelpers.insert(bodyHandle->m_multiBody, tmpHelper);
            ikHelperPtr = tmpHelper;
        }

        i32 endEffectorLinkIndex = clientCmd.m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[0];

        AlignedObjectArray<double> startingPositions;
        startingPositions.reserve(bodyHandle->m_multiBody->getNumLinks());

        Vec3 targetPosWorld(clientCmd.m_calculateInverseKinematicsArguments.m_targetPositions[0],
                                 clientCmd.m_calculateInverseKinematicsArguments.m_targetPositions[1],
                                 clientCmd.m_calculateInverseKinematicsArguments.m_targetPositions[2]);

        Quat targetOrnWorld(clientCmd.m_calculateInverseKinematicsArguments.m_targetOrientation[0],
                                    clientCmd.m_calculateInverseKinematicsArguments.m_targetOrientation[1],
                                    clientCmd.m_calculateInverseKinematicsArguments.m_targetOrientation[2],
                                    clientCmd.m_calculateInverseKinematicsArguments.m_targetOrientation[3]);

        Transform2 targetBaseCoord;
        if (clientCmd.m_updateFlags & IK_HAS_CURRENT_JOINT_POSITIONS)
        {
            targetBaseCoord.setOrigin(targetPosWorld);
            targetBaseCoord.setRotation(targetOrnWorld);
        }
        else
        {
            Transform2 targetWorld;
            targetWorld.setOrigin(targetPosWorld);
            targetWorld.setRotation(targetOrnWorld);
            Transform2 tr = bodyHandle->m_multiBody->getBaseWorldTransform();
            targetBaseCoord = tr.inverse() * targetWorld;
        }

        {
            i32 DofIndex = 0;
            for (i32 i = 0; i < bodyHandle->m_multiBody->getNumLinks(); ++i)
            {
                if (bodyHandle->m_multiBody->getLink(i).m_jointType >= 0 && bodyHandle->m_multiBody->getLink(i).m_jointType <= 2)
                {
                    // 0, 1, 2 represent revolute, prismatic, and spherical joint types respectively. Skip the fixed joints.
                    double curPos = 0;
                    if (clientCmd.m_updateFlags & IK_HAS_CURRENT_JOINT_POSITIONS)
                    {
                        curPos = clientCmd.m_calculateInverseKinematicsArguments.m_currentPositions[DofIndex];
                    }
                    else
                    {
                        curPos = bodyHandle->m_multiBody->getJointPos(i);
                    }
                    startingPositions.push_back(curPos);
                    DofIndex++;
                }
            }
        }

        i32 numIterations = 20;
        if (clientCmd.m_updateFlags & IK_HAS_MAX_ITERATIONS)
        {
            numIterations = clientCmd.m_calculateInverseKinematicsArguments.m_maxNumIterations;
        }
        double residualThreshold = 1e-4;
        if (clientCmd.m_updateFlags & IK_HAS_RESIDUAL_THRESHOLD)
        {
            residualThreshold = clientCmd.m_calculateInverseKinematicsArguments.m_residualThreshold;
        }

        Scalar currentDiff = 1e30f;
        b3AlignedObjectArray<double> jacobian_linear;
        b3AlignedObjectArray<double> jacobian_angular;
        AlignedObjectArray<double> q_current;
        AlignedObjectArray<double> q_new;
        AlignedObjectArray<double> lower_limit;
        AlignedObjectArray<double> upper_limit;
        AlignedObjectArray<double> joint_range;
        AlignedObjectArray<double> rest_pose;
        i32k numDofs = bodyHandle->m_multiBody->getNumDofs();
        i32 baseDofs = bodyHandle->m_multiBody->hasFixedBase() ? 0 : 6;
        drx3d_inverse::vecx nu(numDofs + baseDofs), qdot(numDofs + baseDofs), q(numDofs + baseDofs), joint_force(numDofs + baseDofs);

        for (i32 i = 0; i < numIterations && currentDiff > residualThreshold; i++)
        {
            DRX3D_PROFILE("InverseKinematics1Step");
            if (ikHelperPtr && (endEffectorLinkIndex < bodyHandle->m_multiBody->getNumLinks()))
            {
                jacobian_linear.resize(3 * numDofs);
                jacobian_angular.resize(3 * numDofs);
                i32 jacSize = 0;

                drx3d_inverse::MultiBodyTree* tree = m_data->findOrCreateTree(bodyHandle->m_multiBody);

                q_current.resize(numDofs);

                if (tree && ((numDofs + baseDofs) == tree->numDoFs()))
                {
                    drx3d_inverse::vec3 world_origin;
                    drx3d_inverse::mat33 world_rot;

                    jacSize = jacobian_linear.size();
                    // Set jacobian value

                    i32 DofIndex = 0;
                    for (i32 i = 0; i < bodyHandle->m_multiBody->getNumLinks(); ++i)
                    {
                        if (bodyHandle->m_multiBody->getLink(i).m_jointType >= 0 && bodyHandle->m_multiBody->getLink(i).m_jointType <= 2)
                        {
                            // 0, 1, 2 represent revolute, prismatic, and spherical joint types respectively. Skip the fixed joints.

                            double curPos = startingPositions[DofIndex];
                            q_current[DofIndex] = curPos;
                            q[DofIndex + baseDofs] = curPos;
                            qdot[DofIndex + baseDofs] = 0;
                            nu[DofIndex + baseDofs] = 0;
                            DofIndex++;
                        }
                    }  // Set the gravity to correspond to the world gravity
                    drx3d_inverse::vec3 id_grav(m_data->m_dynamicsWorld->getGravity());

                    {
                        DRX3D_PROFILE("calculatedrx3d_inverse");
                        if (-1 != tree->setGravityInWorldFrame(id_grav) &&
                            -1 != tree->calculatedrx3d_inverse(q, qdot, nu, &joint_force))
                        {
                            tree->calculateJacobians(q);
                            drx3d_inverse::mat3x jac_t(3, numDofs + baseDofs);
                            drx3d_inverse::mat3x jac_r(3, numDofs + baseDofs);
                            // Note that inverse dynamics uses zero-based indexing of bodies, not starting from -1 for the base link.
                            tree->getBodyJacobianTrans(endEffectorLinkIndex + 1, &jac_t);
                            tree->getBodyJacobianRot(endEffectorLinkIndex + 1, &jac_r);

                            //calculatePositionKinematics is already done inside calculatedrx3d_inverse
                            tree->getBodyOrigin(endEffectorLinkIndex + 1, &world_origin);
                            tree->getBodyTransform(endEffectorLinkIndex + 1, &world_rot);

                            for (i32 i = 0; i < 3; ++i)
                            {
                                for (i32 j = 0; j < numDofs; ++j)
                                {
                                    jacobian_linear[i * numDofs + j] = jac_t(i, (baseDofs + j));
                                    jacobian_angular[i * numDofs + j] = jac_r(i, (baseDofs + j));
                                }
                            }
                        }
                    }

                    q_new.resize(numDofs);
                    i32 ikMethod = 0;
                    if ((clientCmd.m_updateFlags & IK_HAS_TARGET_ORIENTATION) && (clientCmd.m_updateFlags & IK_HAS_NULL_SPACE_VELOCITY))
                    {
                        //Nullspace task only works with DLS now. TODO: add nullspace task to SDLS.
                        ikMethod = IK2_VEL_DLS_WITH_ORIENTATION_NULLSPACE;
                    }
                    else if (clientCmd.m_updateFlags & IK_HAS_TARGET_ORIENTATION)
                    {
                        if (clientCmd.m_updateFlags & IK_SDLS)
                        {
                            ikMethod = IK2_VEL_SDLS_WITH_ORIENTATION;
                        }
                        else
                        {
                            ikMethod = IK2_VEL_DLS_WITH_ORIENTATION;
                        }
                    }
                    else if (clientCmd.m_updateFlags & IK_HAS_NULL_SPACE_VELOCITY)
                    {
                        //Nullspace task only works with DLS now. TODO: add nullspace task to SDLS.
                        ikMethod = IK2_VEL_DLS_WITH_NULLSPACE;
                    }
                    else
                    {
                        if (clientCmd.m_updateFlags & IK_SDLS)
                        {
                            ikMethod = IK2_VEL_SDLS;
                        }
                        else
                        {
                            ikMethod = IK2_VEL_DLS;
                            ;
                        }
                    }

                    if (clientCmd.m_updateFlags & IK_HAS_NULL_SPACE_VELOCITY)
                    {
                        lower_limit.resize(numDofs);
                        upper_limit.resize(numDofs);
                        joint_range.resize(numDofs);
                        rest_pose.resize(numDofs);
                        for (i32 i = 0; i < numDofs; ++i)
                        {
                            lower_limit[i] = clientCmd.m_calculateInverseKinematicsArguments.m_lowerLimit[i];
                            upper_limit[i] = clientCmd.m_calculateInverseKinematicsArguments.m_upperLimit[i];
                            joint_range[i] = clientCmd.m_calculateInverseKinematicsArguments.m_jointRange[i];
                            rest_pose[i] = clientCmd.m_calculateInverseKinematicsArguments.m_restPose[i];
                        }
                        {
                            DRX3D_PROFILE("computeNullspaceVel");
                            ikHelperPtr->computeNullspaceVel(numDofs, &q_current[0], &lower_limit[0], &upper_limit[0], &joint_range[0], &rest_pose[0]);
                        }
                    }

                    //Transform2 endEffectorTransformWorld = bodyHandle->m_multiBody->getLink(endEffectorLinkIndex).m_cachedWorldTransform * bodyHandle->m_linkLocalInertialFrames[endEffectorLinkIndex].inverse();

                    Vec3DoubleData endEffectorWorldPosition;
                    QuatDoubleData endEffectorWorldOrientation;

                    //get the position from the inverse dynamics (based on q) instead of endEffectorTransformWorld
                    Vec3 endEffectorPosWorldOrg = world_origin;
                    Quat endEffectorOriWorldOrg;
                    world_rot.getRotation(endEffectorOriWorldOrg);

                    Transform2 endEffectorBaseCoord;
                    endEffectorBaseCoord.setOrigin(endEffectorPosWorldOrg);
                    endEffectorBaseCoord.setRotation(endEffectorOriWorldOrg);

                    //don't need the next two lines
                    //Transform2 linkInertiaInv = bodyHandle->m_linkLocalInertialFrames[endEffectorLinkIndex].inverse();
                    //endEffectorBaseCoord = endEffectorBaseCoord * linkInertiaInv;

                    //Transform2 tr = bodyHandle->m_multiBody->getBaseWorldTransform();
                    //endEffectorBaseCoord = tr.inverse()*endEffectorTransformWorld;
                    //endEffectorBaseCoord = tr.inverse()*endEffectorTransformWorld;

                    Quat endEffectorOriBaseCoord = endEffectorBaseCoord.getRotation();

                    //Vec4 endEffectorOri(endEffectorOriBaseCoord.x(), endEffectorOriBaseCoord.y(), endEffectorOriBaseCoord.z(), endEffectorOriBaseCoord.w());

                    endEffectorBaseCoord.getOrigin().serializeDouble(endEffectorWorldPosition);
                    endEffectorBaseCoord.getRotation().serializeDouble(endEffectorWorldOrientation);

                    //diff
                    currentDiff = (endEffectorBaseCoord.getOrigin() - targetBaseCoord.getOrigin()).length();

                    Vec3DoubleData targetPosBaseCoord;
                    QuatDoubleData targetOrnBaseCoord;
                    targetBaseCoord.getOrigin().serializeDouble(targetPosBaseCoord);
                    targetBaseCoord.getRotation().serializeDouble(targetOrnBaseCoord);

                    // Set joint damping coefficents. A small default
                    // damping constant is added to prevent singularity
                    // with pseudo inverse. The user can set joint damping
                    // coefficients differently for each joint. The larger
                    // the damping coefficient is, the less we rely on
                    // this joint to achieve the IK target.
                    AlignedObjectArray<double> joint_damping;
                    joint_damping.resize(numDofs, 0.5);
                    if (clientCmd.m_updateFlags & IK_HAS_JOINT_DAMPING)
                    {
                        for (i32 i = 0; i < numDofs; ++i)
                        {
                            joint_damping[i] = clientCmd.m_calculateInverseKinematicsArguments.m_jointDamping[i];
                        }
                    }
                    ikHelperPtr->setDampingCoeff(numDofs, &joint_damping[0]);

                    double targetDampCoeff[6] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

                    {
                        DRX3D_PROFILE("computeIK");
                        ikHelperPtr->computeIK(targetPosBaseCoord.m_floats, targetOrnBaseCoord.m_floats,
                                               endEffectorWorldPosition.m_floats, endEffectorWorldOrientation.m_floats,
                                               &q_current[0],
                                               numDofs, clientCmd.m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[0],
                                               &q_new[0], ikMethod, &jacobian_linear[0], &jacobian_angular[0], jacSize * 2, targetDampCoeff);
                    }
                    serverCmd.m_inverseKinematicsResultArgs.m_bodyUniqueId = clientCmd.m_calculatedrx3d_inverseArguments.m_bodyUniqueId;
                    for (i32 i = 0; i < numDofs; i++)
                    {
                        serverCmd.m_inverseKinematicsResultArgs.m_jointPositions[i] = q_new[i];
                    }
                    serverCmd.m_inverseKinematicsResultArgs.m_dofCount = numDofs;
                    serverCmd.m_type = CMD_CALCULATE_INVERSE_KINEMATICS_COMPLETED;
                    for (i32 i = 0; i < numDofs; i++)
                    {
                        startingPositions[i] = q_new[i];
                    }
                }
            }
        }
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCalculateInverseKinematicsCommand2(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_CALCULATE_INVERSE_KINEMATICS");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CALCULATE_INVERSE_KINEMATICS_FAILED;

    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(clientCmd.m_calculateInverseKinematicsArguments.m_bodyUniqueId);
    if (bodyHandle && bodyHandle->m_multiBody)
    {
        IKTrajectoryHelper** ikHelperPtrPtr = m_data->m_inverseKinematicsHelpers.find(bodyHandle->m_multiBody);
        IKTrajectoryHelper* ikHelperPtr = 0;

        if (ikHelperPtrPtr)
        {
            ikHelperPtr = *ikHelperPtrPtr;
        }
        else
        {
            IKTrajectoryHelper* tmpHelper = new IKTrajectoryHelper;
            m_data->m_inverseKinematicsHelpers.insert(bodyHandle->m_multiBody, tmpHelper);
            ikHelperPtr = tmpHelper;
        }

        AlignedObjectArray<double> startingPositions;
        startingPositions.reserve(bodyHandle->m_multiBody->getNumLinks());

        {
            i32 DofIndex = 0;
            for (i32 i = 0; i < bodyHandle->m_multiBody->getNumLinks(); ++i)
            {
                if (bodyHandle->m_multiBody->getLink(i).m_jointType >= 0 && bodyHandle->m_multiBody->getLink(i).m_jointType <= 2)
                {
                    // 0, 1, 2 represent revolute, prismatic, and spherical joint types respectively. Skip the fixed joints.
                    double curPos = 0;
                    if (clientCmd.m_updateFlags & IK_HAS_CURRENT_JOINT_POSITIONS)
                    {
                        curPos = clientCmd.m_calculateInverseKinematicsArguments.m_currentPositions[DofIndex];
                    }
                    else
                    {
                        curPos = bodyHandle->m_multiBody->getJointPos(i);
                    }
                    startingPositions.push_back(curPos);
                    DofIndex++;
                }
            }
        }

        i32 numIterations = 20;
        if (clientCmd.m_updateFlags & IK_HAS_MAX_ITERATIONS)
        {
            numIterations = clientCmd.m_calculateInverseKinematicsArguments.m_maxNumIterations;
        }
        double residualThreshold = 1e-4;
        if (clientCmd.m_updateFlags & IK_HAS_RESIDUAL_THRESHOLD)
        {
            residualThreshold = clientCmd.m_calculateInverseKinematicsArguments.m_residualThreshold;
        }

        Scalar currentDiff = 1e30f;
        b3AlignedObjectArray<double> endEffectorTargetWorldPositions;
        b3AlignedObjectArray<double> endEffectorTargetWorldOrientations;
        b3AlignedObjectArray<double> endEffectorCurrentWorldPositions;
        b3AlignedObjectArray<double> jacobian_linear;
        b3AlignedObjectArray<double> jacobian_angular;
        AlignedObjectArray<double> q_current;
        AlignedObjectArray<double> q_new;
        AlignedObjectArray<double> lower_limit;
        AlignedObjectArray<double> upper_limit;
        AlignedObjectArray<double> joint_range;
        AlignedObjectArray<double> rest_pose;
        i32k numDofs = bodyHandle->m_multiBody->getNumDofs();
        i32 baseDofs = bodyHandle->m_multiBody->hasFixedBase() ? 0 : 6;
        drx3d_inverse::vecx nu(numDofs + baseDofs), qdot(numDofs + baseDofs), q(numDofs + baseDofs), joint_force(numDofs + baseDofs);

        endEffectorTargetWorldPositions.resize(0);
        endEffectorTargetWorldPositions.reserve(clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices * 3);
        endEffectorTargetWorldOrientations.resize(0);
        endEffectorTargetWorldOrientations.reserve(clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices * 4);

        bool validEndEffectorLinkIndices = true;

        for (i32 ne = 0; ne < clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices; ne++)
        {
            i32 endEffectorLinkIndex = clientCmd.m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[ne];
            validEndEffectorLinkIndices = validEndEffectorLinkIndices && (endEffectorLinkIndex < bodyHandle->m_multiBody->getNumLinks());

            Vec3 targetPosWorld(clientCmd.m_calculateInverseKinematicsArguments.m_targetPositions[ne * 3 + 0],
                                     clientCmd.m_calculateInverseKinematicsArguments.m_targetPositions[ne * 3 + 1],
                                     clientCmd.m_calculateInverseKinematicsArguments.m_targetPositions[ne * 3 + 2]);

            Quat targetOrnWorld(clientCmd.m_calculateInverseKinematicsArguments.m_targetOrientation[ne * 4 + 0],
                                        clientCmd.m_calculateInverseKinematicsArguments.m_targetOrientation[ne * 4 + 1],
                                        clientCmd.m_calculateInverseKinematicsArguments.m_targetOrientation[ne * 4 + 2],
                                        clientCmd.m_calculateInverseKinematicsArguments.m_targetOrientation[ne * 4 + 3]);

            Transform2 targetBaseCoord;
            if (clientCmd.m_updateFlags & IK_HAS_CURRENT_JOINT_POSITIONS)
            {
                targetBaseCoord.setOrigin(targetPosWorld);
                targetBaseCoord.setRotation(targetOrnWorld);
            }
            else
            {
                Transform2 targetWorld;
                targetWorld.setOrigin(targetPosWorld);
                targetWorld.setRotation(targetOrnWorld);
                Transform2 tr = bodyHandle->m_multiBody->getBaseWorldTransform();
                targetBaseCoord = tr.inverse() * targetWorld;
            }

            Vec3DoubleData targetPosBaseCoord;
            QuatDoubleData targetOrnBaseCoord;
            targetBaseCoord.getOrigin().serializeDouble(targetPosBaseCoord);
            targetBaseCoord.getRotation().serializeDouble(targetOrnBaseCoord);

            endEffectorTargetWorldPositions.push_back(targetPosBaseCoord.m_floats[0]);
            endEffectorTargetWorldPositions.push_back(targetPosBaseCoord.m_floats[1]);
            endEffectorTargetWorldPositions.push_back(targetPosBaseCoord.m_floats[2]);

            endEffectorTargetWorldOrientations.push_back(targetOrnBaseCoord.m_floats[0]);
            endEffectorTargetWorldOrientations.push_back(targetOrnBaseCoord.m_floats[1]);
            endEffectorTargetWorldOrientations.push_back(targetOrnBaseCoord.m_floats[2]);
            endEffectorTargetWorldOrientations.push_back(targetOrnBaseCoord.m_floats[3]);
        }
        for (i32 i = 0; i < numIterations && currentDiff > residualThreshold; i++)
        {
            DRX3D_PROFILE("InverseKinematics1Step");
            if (ikHelperPtr && validEndEffectorLinkIndices)
            {
                jacobian_linear.resize(clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices * 3 * numDofs);
                jacobian_angular.resize(clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices * 3 * numDofs);
                i32 jacSize = 0;

                drx3d_inverse::MultiBodyTree* tree = m_data->findOrCreateTree(bodyHandle->m_multiBody);

                q_current.resize(numDofs);

                if (tree && ((numDofs + baseDofs) == tree->numDoFs()))
                {
                    drx3d_inverse::vec3 world_origin;
                    drx3d_inverse::mat33 world_rot;

                    jacSize = jacobian_linear.size();
                    // Set jacobian value

                    i32 DofIndex = 0;
                    for (i32 i = 0; i < bodyHandle->m_multiBody->getNumLinks(); ++i)
                    {
                        if (bodyHandle->m_multiBody->getLink(i).m_jointType >= 0 && bodyHandle->m_multiBody->getLink(i).m_jointType <= 2)
                        {
                            // 0, 1, 2 represent revolute, prismatic, and spherical joint types respectively. Skip the fixed joints.
                            double curPos = startingPositions[DofIndex];
                            q_current[DofIndex] = curPos;
                            q[DofIndex + baseDofs] = curPos;
                            qdot[DofIndex + baseDofs] = 0;
                            nu[DofIndex + baseDofs] = 0;
                            DofIndex++;
                        }
                    }  // Set the gravity to correspond to the world gravity
                    drx3d_inverse::vec3 id_grav(m_data->m_dynamicsWorld->getGravity());

                    {
                        DRX3D_PROFILE("calculatedrx3d_inverse");
                        if (-1 != tree->setGravityInWorldFrame(id_grav) &&
                            -1 != tree->calculatedrx3d_inverse(q, qdot, nu, &joint_force))
                        {
                            tree->calculateJacobians(q);
                            drx3d_inverse::mat3x jac_t(3, numDofs + baseDofs);
                            drx3d_inverse::mat3x jac_r(3, numDofs + baseDofs);
                            currentDiff = 0;

                            endEffectorCurrentWorldPositions.resize(0);
                            endEffectorCurrentWorldPositions.reserve(clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices * 3);

                            for (i32 ne = 0; ne < clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices; ne++)
                            {
                                i32 endEffectorLinkIndex2 = clientCmd.m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[ne];

                                // Note that inverse dynamics uses zero-based indexing of bodies, not starting from -1 for the base link.
                                tree->getBodyJacobianTrans(endEffectorLinkIndex2 + 1, &jac_t);
                                tree->getBodyJacobianRot(endEffectorLinkIndex2 + 1, &jac_r);

                                //calculatePositionKinematics is already done inside calculatedrx3d_inverse

                                tree->getBodyOrigin(endEffectorLinkIndex2 + 1, &world_origin);
                                tree->getBodyTransform(endEffectorLinkIndex2 + 1, &world_rot);

                                for (i32 i = 0; i < 3; ++i)
                                {
                                    for (i32 j = 0; j < numDofs; ++j)
                                    {
                                        jacobian_linear[(ne * 3 + i) * numDofs + j] = jac_t(i, (baseDofs + j));
                                        jacobian_angular[(ne * 3 + i) * numDofs + j] = jac_r(i, (baseDofs + j));
                                    }
                                }

                                endEffectorCurrentWorldPositions.push_back(world_origin[0]);
                                endEffectorCurrentWorldPositions.push_back(world_origin[1]);
                                endEffectorCurrentWorldPositions.push_back(world_origin[2]);

                                drx3d_inverse::vec3 targetPos(Vec3(endEffectorTargetWorldPositions[ne * 3 + 0],
                                                                            endEffectorTargetWorldPositions[ne * 3 + 1],
                                                                            endEffectorTargetWorldPositions[ne * 3 + 2]));
                                //diff
                                currentDiff = d3Max(currentDiff, (world_origin - targetPos).length());
                            }
                        }
                    }

                    q_new.resize(numDofs);
                    i32 ikMethod = 0;
                    if ((clientCmd.m_updateFlags & IK_HAS_TARGET_ORIENTATION) && (clientCmd.m_updateFlags & IK_HAS_NULL_SPACE_VELOCITY))
                    {
                        //Nullspace task only works with DLS now. TODO: add nullspace task to SDLS.
                        ikMethod = IK2_VEL_DLS_WITH_ORIENTATION_NULLSPACE;
                    }
                    else if (clientCmd.m_updateFlags & IK_HAS_TARGET_ORIENTATION)
                    {
                        if (clientCmd.m_updateFlags & IK_SDLS)
                        {
                            ikMethod = IK2_VEL_SDLS_WITH_ORIENTATION;
                        }
                        else
                        {
                            ikMethod = IK2_VEL_DLS_WITH_ORIENTATION;
                        }
                    }
                    else if (clientCmd.m_updateFlags & IK_HAS_NULL_SPACE_VELOCITY)
                    {
                        //Nullspace task only works with DLS now. TODO: add nullspace task to SDLS.
                        ikMethod = IK2_VEL_DLS_WITH_NULLSPACE;
                    }
                    else
                    {
                        if (clientCmd.m_updateFlags & IK_SDLS)
                        {
                            ikMethod = IK2_VEL_SDLS;
                        }
                        else
                        {
                            ikMethod = IK2_VEL_DLS;
                            ;
                        }
                    }

                    if (clientCmd.m_updateFlags & IK_HAS_NULL_SPACE_VELOCITY)
                    {
                        lower_limit.resize(numDofs);
                        upper_limit.resize(numDofs);
                        joint_range.resize(numDofs);
                        rest_pose.resize(numDofs);
                        for (i32 i = 0; i < numDofs; ++i)
                        {
                            lower_limit[i] = clientCmd.m_calculateInverseKinematicsArguments.m_lowerLimit[i];
                            upper_limit[i] = clientCmd.m_calculateInverseKinematicsArguments.m_upperLimit[i];
                            joint_range[i] = clientCmd.m_calculateInverseKinematicsArguments.m_jointRange[i];
                            rest_pose[i] = clientCmd.m_calculateInverseKinematicsArguments.m_restPose[i];
                        }
                        {
                            DRX3D_PROFILE("computeNullspaceVel");
                            ikHelperPtr->computeNullspaceVel(numDofs, &q_current[0], &lower_limit[0], &upper_limit[0], &joint_range[0], &rest_pose[0]);
                        }
                    }

                    //Transform2 endEffectorTransformWorld = bodyHandle->m_multiBody->getLink(endEffectorLinkIndex).m_cachedWorldTransform * bodyHandle->m_linkLocalInertialFrames[endEffectorLinkIndex].inverse();

                    Vec3DoubleData endEffectorWorldPosition;
                    QuatDoubleData endEffectorWorldOrientation;

                    //get the position from the inverse dynamics (based on q) instead of endEffectorTransformWorld
                    Vec3 endEffectorPosWorldOrg = world_origin;
                    Quat endEffectorOriWorldOrg;
                    world_rot.getRotation(endEffectorOriWorldOrg);

                    Transform2 endEffectorBaseCoord;
                    endEffectorBaseCoord.setOrigin(endEffectorPosWorldOrg);
                    endEffectorBaseCoord.setRotation(endEffectorOriWorldOrg);

                    //don't need the next two lines
                    //Transform2 linkInertiaInv = bodyHandle->m_linkLocalInertialFrames[endEffectorLinkIndex].inverse();
                    //endEffectorBaseCoord = endEffectorBaseCoord * linkInertiaInv;

                    //Transform2 tr = bodyHandle->m_multiBody->getBaseWorldTransform();
                    //endEffectorBaseCoord = tr.inverse()*endEffectorTransformWorld;
                    //endEffectorBaseCoord = tr.inverse()*endEffectorTransformWorld;

                    Quat endEffectorOriBaseCoord = endEffectorBaseCoord.getRotation();

                    //Vec4 endEffectorOri(endEffectorOriBaseCoord.x(), endEffectorOriBaseCoord.y(), endEffectorOriBaseCoord.z(), endEffectorOriBaseCoord.w());

                    endEffectorBaseCoord.getOrigin().serializeDouble(endEffectorWorldPosition);
                    endEffectorBaseCoord.getRotation().serializeDouble(endEffectorWorldOrientation);

                    // Set joint damping coefficents. A small default
                    // damping constant is added to prevent singularity
                    // with pseudo inverse. The user can set joint damping
                    // coefficients differently for each joint. The larger
                    // the damping coefficient is, the less we rely on
                    // this joint to achieve the IK target.
                    AlignedObjectArray<double> joint_damping;
                    joint_damping.resize(numDofs, 0.5);
                    if (clientCmd.m_updateFlags & IK_HAS_JOINT_DAMPING)
                    {
                        for (i32 i = 0; i < numDofs; ++i)
                        {
                            joint_damping[i] = clientCmd.m_calculateInverseKinematicsArguments.m_jointDamping[i];
                        }
                    }
                    ikHelperPtr->setDampingCoeff(numDofs, &joint_damping[0]);

                    double targetDampCoeff[6] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
                    bool performedIK = false;

                    if (clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices == 1)
                    {
                        DRX3D_PROFILE("computeIK");
                        ikHelperPtr->computeIK(&endEffectorTargetWorldPositions[0],
                                               &endEffectorTargetWorldOrientations[0],
                                               endEffectorWorldPosition.m_floats, endEffectorWorldOrientation.m_floats,
                                               &q_current[0],
                                               numDofs, clientCmd.m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[0],
                                               &q_new[0], ikMethod, &jacobian_linear[0], &jacobian_angular[0], jacSize * 2, targetDampCoeff);
                        performedIK = true;
                    }
                    else
                    {
                        if (clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices > 1)
                        {
                            ikHelperPtr->computeIK2(&endEffectorTargetWorldPositions[0],
                                                    &endEffectorCurrentWorldPositions[0],
                                                    clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices,
                                                    //endEffectorWorldOrientation.m_floats,
                                                    &q_current[0],
                                                    numDofs,
                                                    &q_new[0], ikMethod, &jacobian_linear[0], targetDampCoeff);
                            performedIK = true;
                        }
                    }
                    if (performedIK)
                    {
                        serverCmd.m_inverseKinematicsResultArgs.m_bodyUniqueId = clientCmd.m_calculatedrx3d_inverseArguments.m_bodyUniqueId;
                        for (i32 i = 0; i < numDofs; i++)
                        {
                            serverCmd.m_inverseKinematicsResultArgs.m_jointPositions[i] = q_new[i];
                        }
                        serverCmd.m_inverseKinematicsResultArgs.m_dofCount = numDofs;
                        serverCmd.m_type = CMD_CALCULATE_INVERSE_KINEMATICS_COMPLETED;
                        for (i32 i = 0; i < numDofs; i++)
                        {
                            startingPositions[i] = q_new[i];
                        }
                    }
                }
            }
        }
    }
    return hasStatus;
}

//      PyModule_AddIntConstant(m, "GEOM_SPHERE", GEOM_SPHERE);
//      PyModule_AddIntConstant(m, "GEOM_BOX", GEOM_BOX);
//      PyModule_AddIntConstant(m, "GEOM_CYLINDER", GEOM_CYLINDER);
//      PyModule_AddIntConstant(m, "GEOM_MESH", GEOM_MESH);
//      PyModule_AddIntConstant(m, "GEOM_PLANE", GEOM_PLANE);
//      PyModule_AddIntConstant(m, "GEOM_CAPSULE", GEOM_CAPSULE);

i32 PhysicsServerCommandProcessor::extractCollisionShapes(const CollisionShape* colShape, const Transform2& transform, b3CollisionShapeData* collisionShapeBuffer, i32 maxCollisionShapes)
{
    if (maxCollisionShapes <= 0)
    {
        drx3DWarning("No space in buffer");
        return 0;
    }

    i32 numConverted = 0;

    collisionShapeBuffer[0].m_localCollisionFrame[0] = transform.getOrigin()[0];
    collisionShapeBuffer[0].m_localCollisionFrame[1] = transform.getOrigin()[1];
    collisionShapeBuffer[0].m_localCollisionFrame[2] = transform.getOrigin()[2];
    collisionShapeBuffer[0].m_localCollisionFrame[3] = transform.getRotation()[0];
    collisionShapeBuffer[0].m_localCollisionFrame[4] = transform.getRotation()[1];
    collisionShapeBuffer[0].m_localCollisionFrame[5] = transform.getRotation()[2];
    collisionShapeBuffer[0].m_localCollisionFrame[6] = transform.getRotation()[3];
    collisionShapeBuffer[0].m_meshAssetFileName[0] = 0;

    switch (colShape->getShapeType())
    {
        case MULTI_SPHERE_SHAPE_PROXYTYPE:
        {
            CapsuleShapeZ* capsule = (CapsuleShapeZ*)colShape;
            collisionShapeBuffer[0].m_collisionGeometryType = GEOM_CAPSULE;
            collisionShapeBuffer[0].m_dimensions[0] = 2. * capsule->getHalfHeight();
            collisionShapeBuffer[0].m_dimensions[1] = capsule->getRadius();
            collisionShapeBuffer[0].m_dimensions[2] = 0;
            numConverted++;
            break;
            break;
        }
        case STATIC_PLANE_PROXYTYPE:
        {
            StaticPlaneShape* plane = (StaticPlaneShape*)colShape;
            collisionShapeBuffer[0].m_collisionGeometryType = GEOM_PLANE;
            collisionShapeBuffer[0].m_dimensions[0] = plane->getPlaneNormal()[0];
            collisionShapeBuffer[0].m_dimensions[1] = plane->getPlaneNormal()[1];
            collisionShapeBuffer[0].m_dimensions[2] = plane->getPlaneNormal()[2];
            numConverted += 1;
            break;
        }
        case TRIANGLE_MESH_SHAPE_PROXYTYPE:
        case SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE:
        case CONVEX_HULL_SHAPE_PROXYTYPE:
        {
            UrdfCollision* urdfCol = m_data->m_bulletCollisionShape2UrdfCollision.find(colShape);
            if (urdfCol && (urdfCol->m_geometry.m_type == URDF_GEOM_MESH))
            {
                collisionShapeBuffer[0].m_collisionGeometryType = GEOM_MESH;
                collisionShapeBuffer[0].m_dimensions[0] = urdfCol->m_geometry.m_meshScale[0];
                collisionShapeBuffer[0].m_dimensions[1] = urdfCol->m_geometry.m_meshScale[1];
                collisionShapeBuffer[0].m_dimensions[2] = urdfCol->m_geometry.m_meshScale[2];
                strcpy(collisionShapeBuffer[0].m_meshAssetFileName, urdfCol->m_geometry.m_meshFileName.c_str());
                numConverted += 1;
            }
            else
            {
                collisionShapeBuffer[0].m_collisionGeometryType = GEOM_MESH;
                sprintf(collisionShapeBuffer[0].m_meshAssetFileName, "unknown_file");
                collisionShapeBuffer[0].m_dimensions[0] = 1;
                collisionShapeBuffer[0].m_dimensions[1] = 1;
                collisionShapeBuffer[0].m_dimensions[2] = 1;
                numConverted++;
            }

            break;
        }
        case CAPSULE_SHAPE_PROXYTYPE:
        {
            CapsuleShapeZ* capsule = (CapsuleShapeZ*)colShape;
            collisionShapeBuffer[0].m_collisionGeometryType = GEOM_CAPSULE;
            collisionShapeBuffer[0].m_dimensions[0] = 2. * capsule->getHalfHeight();
            collisionShapeBuffer[0].m_dimensions[1] = capsule->getRadius();
            collisionShapeBuffer[0].m_dimensions[2] = 0;
            numConverted++;
            break;
        }
        case CYLINDER_SHAPE_PROXYTYPE:
        {
            CylinderShapeZ* cyl = (CylinderShapeZ*)colShape;
            collisionShapeBuffer[0].m_collisionGeometryType = GEOM_CYLINDER;
            collisionShapeBuffer[0].m_dimensions[0] = 2. * cyl->getHalfExtentsWithMargin().getZ();
            collisionShapeBuffer[0].m_dimensions[1] = cyl->getHalfExtentsWithMargin().getX();
            collisionShapeBuffer[0].m_dimensions[2] = 0;
            numConverted++;
            break;
        }
        case BOX_SHAPE_PROXYTYPE:
        {
            BoxShape* box = (BoxShape*)colShape;
            Vec3 halfExtents = box->getHalfExtentsWithMargin();
            collisionShapeBuffer[0].m_collisionGeometryType = GEOM_BOX;
            collisionShapeBuffer[0].m_dimensions[0] = 2. * halfExtents[0];
            collisionShapeBuffer[0].m_dimensions[1] = 2. * halfExtents[1];
            collisionShapeBuffer[0].m_dimensions[2] = 2. * halfExtents[2];
            numConverted++;
            break;
        }
        case SPHERE_SHAPE_PROXYTYPE:
        {
            SphereShape* sphere = (SphereShape*)colShape;
            collisionShapeBuffer[0].m_collisionGeometryType = GEOM_SPHERE;
            collisionShapeBuffer[0].m_dimensions[0] = sphere->getRadius();
            collisionShapeBuffer[0].m_dimensions[1] = sphere->getRadius();
            collisionShapeBuffer[0].m_dimensions[2] = sphere->getRadius();
            numConverted++;
            break;
        }
        case COMPOUND_SHAPE_PROXYTYPE:
        {
            //it could be a compound mesh from a wavefront OBJ, check it
            UrdfCollision* urdfCol = m_data->m_bulletCollisionShape2UrdfCollision.find(colShape);
            if (urdfCol && (urdfCol->m_geometry.m_type == URDF_GEOM_MESH))
            {
                collisionShapeBuffer[0].m_collisionGeometryType = GEOM_MESH;
                collisionShapeBuffer[0].m_dimensions[0] = urdfCol->m_geometry.m_meshScale[0];
                collisionShapeBuffer[0].m_dimensions[1] = urdfCol->m_geometry.m_meshScale[1];
                collisionShapeBuffer[0].m_dimensions[2] = urdfCol->m_geometry.m_meshScale[2];
                strcpy(collisionShapeBuffer[0].m_meshAssetFileName, urdfCol->m_geometry.m_meshFileName.c_str());
                numConverted += 1;
            }
            else
            {
                //recurse, accumulate childTransform
                CompoundShape* compound = (CompoundShape*)colShape;
                for (i32 i = 0; i < compound->getNumChildShapes(); i++)
                {
                    Transform2 childTrans = transform * compound->getChildTransform(i);
                    i32 remain = maxCollisionShapes - numConverted;
                    i32 converted = extractCollisionShapes(compound->getChildShape(i), childTrans, &collisionShapeBuffer[numConverted], remain);
                    numConverted += converted;
                }
            }
            break;
        }
        default:
        {
            drx3DWarning("Unexpected collision shape type in PhysicsServerCommandProcessor::extractCollisionShapes");
        }
    };

    return numConverted;
}

bool PhysicsServerCommandProcessor::processRequestCollisionShapeInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_REQUEST_COLLISION_SHAPE_INFO");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_COLLISION_SHAPE_INFO_FAILED;
    i32 bodyUniqueId = clientCmd.m_requestCollisionShapeDataArguments.m_bodyUniqueId;
    i32 linkIndex = clientCmd.m_requestCollisionShapeDataArguments.m_linkIndex;
    InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);
    if (bodyHandle)
    {
        if (bodyHandle->m_multiBody)
        {
            b3CollisionShapeData* collisionShapeStoragePtr = (b3CollisionShapeData*)bufferServerToClient;
            collisionShapeStoragePtr->m_objectUniqueId = bodyUniqueId;
            collisionShapeStoragePtr->m_linkIndex = linkIndex;
            i32 totalBytesPerObject = sizeof(b3CollisionShapeData);
            i32 maxNumColObjects = bufferSizeInBytes / totalBytesPerObject - 1;
            Transform2 childTrans;
            childTrans.setIdentity();
            serverCmd.m_sendCollisionShapeArgs.m_bodyUniqueId = bodyUniqueId;
            serverCmd.m_sendCollisionShapeArgs.m_linkIndex = linkIndex;

            if (linkIndex == -1)
            {
                if (bodyHandle->m_multiBody->getBaseCollider())
                {
                    //extract shape info from base collider
                    i32 numConvertedCollisionShapes = extractCollisionShapes(bodyHandle->m_multiBody->getBaseCollider()->getCollisionShape(), childTrans, collisionShapeStoragePtr, maxNumColObjects);
                    serverCmd.m_numDataStreamBytes = numConvertedCollisionShapes * sizeof(b3CollisionShapeData);
                    serverCmd.m_sendCollisionShapeArgs.m_numCollisionShapes = numConvertedCollisionShapes;
                    serverCmd.m_type = CMD_COLLISION_SHAPE_INFO_COMPLETED;
                }
            }
            else
            {
                if (linkIndex >= 0 && linkIndex < bodyHandle->m_multiBody->getNumLinks() && bodyHandle->m_multiBody->getLinkCollider(linkIndex))
                {
                    i32 numConvertedCollisionShapes = extractCollisionShapes(bodyHandle->m_multiBody->getLinkCollider(linkIndex)->getCollisionShape(), childTrans, collisionShapeStoragePtr, maxNumColObjects);
                    serverCmd.m_numDataStreamBytes = numConvertedCollisionShapes * sizeof(b3CollisionShapeData);
                    serverCmd.m_sendCollisionShapeArgs.m_numCollisionShapes = numConvertedCollisionShapes;
                    serverCmd.m_type = CMD_COLLISION_SHAPE_INFO_COMPLETED;
                }
            }
        }
    }

    return hasStatus;
}
bool PhysicsServerCommandProcessor::processRequestVisualShapeInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_REQUEST_VISUAL_SHAPE_INFO");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_VISUAL_SHAPE_INFO_FAILED;
    //retrieve the visual shape information for a specific body

    if (m_data->m_pluginManager.getRenderInterface())
    {
        i32 totalNumVisualShapes = m_data->m_pluginManager.getRenderInterface()->getNumVisualShapes(clientCmd.m_requestVisualShapeDataArguments.m_bodyUniqueId);
        //i32 totalBytesPerVisualShape = sizeof (b3VisualShapeData);
        //i32 visualShapeStorage = bufferSizeInBytes / totalBytesPerVisualShape - 1;

        //set serverCmd.m_sendVisualShapeArgs when totalNumVisualShapes is zero
        if (totalNumVisualShapes == 0)
        {
            serverCmd.m_sendVisualShapeArgs.m_numRemainingVisualShapes = 0;
            serverCmd.m_sendVisualShapeArgs.m_numVisualShapesCopied = 0;
            serverCmd.m_sendVisualShapeArgs.m_startingVisualShapeIndex = clientCmd.m_requestVisualShapeDataArguments.m_startingVisualShapeIndex;
            serverCmd.m_sendVisualShapeArgs.m_bodyUniqueId = clientCmd.m_requestVisualShapeDataArguments.m_bodyUniqueId;
            serverCmd.m_numDataStreamBytes = sizeof(b3VisualShapeData) * serverCmd.m_sendVisualShapeArgs.m_numVisualShapesCopied;
            serverCmd.m_type = CMD_VISUAL_SHAPE_INFO_COMPLETED;
        }

        else
        {
            b3VisualShapeData* visualShapeStoragePtr = (b3VisualShapeData*)bufferServerToClient;

            i32 remain = totalNumVisualShapes - clientCmd.m_requestVisualShapeDataArguments.m_startingVisualShapeIndex;
            i32 shapeIndex = clientCmd.m_requestVisualShapeDataArguments.m_startingVisualShapeIndex;

            i32 success = m_data->m_pluginManager.getRenderInterface()->getVisualShapesData(clientCmd.m_requestVisualShapeDataArguments.m_bodyUniqueId,
                                                                                            shapeIndex,
                                                                                            visualShapeStoragePtr);
            if (success)
            {
                //find the matching texture unique ids.
                if (visualShapeStoragePtr->m_tinyRendererTextureId >= 0)
                {
                    b3AlignedObjectArray<i32> usedHandles;
                    m_data->m_textureHandles.getUsedHandles(usedHandles);

                    for (i32 i = 0; i < usedHandles.size(); i++)
                    {
                        i32 texHandle = usedHandles[i];
                        InternalTextureHandle* texH = m_data->m_textureHandles.getHandle(texHandle);
                        if (texH && (texH->m_tinyRendererTextureId == visualShapeStoragePtr->m_tinyRendererTextureId))
                        {
                            visualShapeStoragePtr->m_openglTextureId = texH->m_openglTextureId;
                            visualShapeStoragePtr->m_textureUniqueId = texHandle;
                        }
                    }
                }

                serverCmd.m_sendVisualShapeArgs.m_numRemainingVisualShapes = remain - 1;
                serverCmd.m_sendVisualShapeArgs.m_numVisualShapesCopied = 1;
                serverCmd.m_sendVisualShapeArgs.m_startingVisualShapeIndex = clientCmd.m_requestVisualShapeDataArguments.m_startingVisualShapeIndex;
                serverCmd.m_sendVisualShapeArgs.m_bodyUniqueId = clientCmd.m_requestVisualShapeDataArguments.m_bodyUniqueId;
                serverCmd.m_numDataStreamBytes = sizeof(b3VisualShapeData) * serverCmd.m_sendVisualShapeArgs.m_numVisualShapesCopied;
                serverCmd.m_type = CMD_VISUAL_SHAPE_INFO_COMPLETED;
            }
            else
            {
                drx3DWarning("failed to get shape info");
            }
        }
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processUpdateVisualShapeCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_UPDATE_VISUAL_SHAPE");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_VISUAL_SHAPE_UPDATE_FAILED;
    InternalTextureHandle* texHandle = 0;

    if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_TEXTURE)
    {
        if (clientCmd.m_updateVisualShapeDataArguments.m_textureUniqueId >= 0)
        {
            texHandle = m_data->m_textureHandles.getHandle(clientCmd.m_updateVisualShapeDataArguments.m_textureUniqueId);
        }

        if (clientCmd.m_updateVisualShapeDataArguments.m_textureUniqueId >= -1)
        {
            if (texHandle)
            {
                if (m_data->m_pluginManager.getRenderInterface())
                {
                    m_data->m_pluginManager.getRenderInterface()->changeShapeTexture(clientCmd.m_updateVisualShapeDataArguments.m_bodyUniqueId,
                     clientCmd.m_updateVisualShapeDataArguments.m_jointIndex,
                     clientCmd.m_updateVisualShapeDataArguments.m_shapeIndex,
                     texHandle->m_tinyRendererTextureId);
                }
            }
            else
            {
                m_data->m_pluginManager.getRenderInterface()->changeShapeTexture(clientCmd.m_updateVisualShapeDataArguments.m_bodyUniqueId,
                clientCmd.m_updateVisualShapeDataArguments.m_jointIndex,
                clientCmd.m_updateVisualShapeDataArguments.m_shapeIndex,-1);
            }
        }
    }

    {
        i32 bodyUniqueId = clientCmd.m_updateVisualShapeDataArguments.m_bodyUniqueId;
        i32 linkIndex = clientCmd.m_updateVisualShapeDataArguments.m_jointIndex;

        InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);
        if (bodyHandle)
        {
            if (bodyHandle->m_multiBody)
            {
                if (linkIndex == -1)
                {
                    if (bodyHandle->m_multiBody->getBaseCollider())
                    {
                        i32 graphicsIndex = bodyHandle->m_multiBody->getBaseCollider()->getUserIndex();
                        if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_TEXTURE)
                        {
                            i32 shapeIndex = m_data->m_guiHelper->getShapeIndexFromInstance(graphicsIndex);
                            if (texHandle)
                            {
                                m_data->m_guiHelper->replaceTexture(shapeIndex, texHandle->m_openglTextureId);
                            }
                            else
                            {
                                m_data->m_guiHelper->replaceTexture(shapeIndex, -1);
                            }
                        }
                        if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_RGBA_COLOR)
                        {
                            if (m_data->m_pluginManager.getRenderInterface())
                            {
                                m_data->m_pluginManager.getRenderInterface()->changeRGBAColor(bodyUniqueId, linkIndex,
                                                                                              clientCmd.m_updateVisualShapeDataArguments.m_shapeIndex,
                                                                                              clientCmd.m_updateVisualShapeDataArguments.m_rgbaColor);
                            }
                            m_data->m_guiHelper->changeRGBAColor(graphicsIndex, clientCmd.m_updateVisualShapeDataArguments.m_rgbaColor);
                        }
                        if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_SPECULAR_COLOR)
                        {
                            m_data->m_guiHelper->changeSpecularColor(graphicsIndex, clientCmd.m_updateVisualShapeDataArguments.m_specularColor);
                        }
                    }
                }
                else
                {
                    if (linkIndex < bodyHandle->m_multiBody->getNumLinks())
                    {
                        if (bodyHandle->m_multiBody->getLink(linkIndex).m_collider)
                        {
                            i32 graphicsIndex = bodyHandle->m_multiBody->getLink(linkIndex).m_collider->getUserIndex();
                            if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_TEXTURE)
                            {
                                i32 shapeIndex = m_data->m_guiHelper->getShapeIndexFromInstance(graphicsIndex);
                                if (texHandle)
                                {
                                    m_data->m_guiHelper->replaceTexture(shapeIndex, texHandle->m_openglTextureId);
                                }
                                else
                                {
                                    m_data->m_guiHelper->replaceTexture(shapeIndex, -1);
                                }
                            }
                            if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_RGBA_COLOR)
                            {
                                if (m_data->m_pluginManager.getRenderInterface())
                                {
                                    m_data->m_pluginManager.getRenderInterface()->changeRGBAColor(bodyUniqueId, linkIndex,
                                                                                                  clientCmd.m_updateVisualShapeDataArguments.m_shapeIndex, clientCmd.m_updateVisualShapeDataArguments.m_rgbaColor);
                                }
                                m_data->m_guiHelper->changeRGBAColor(graphicsIndex, clientCmd.m_updateVisualShapeDataArguments.m_rgbaColor);
                            }
                            if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_SPECULAR_COLOR)
                            {
                                m_data->m_guiHelper->changeSpecularColor(graphicsIndex, clientCmd.m_updateVisualShapeDataArguments.m_specularColor);
                            }
                        }
                    }
                }
            }
            else
            {
                if (bodyHandle->m_rigidBody)
                {
                    i32 graphicsIndex = bodyHandle->m_rigidBody->getUserIndex();
                    if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_TEXTURE)
                    {
                        if (texHandle)
                        {
                            i32 shapeIndex = m_data->m_guiHelper->getShapeIndexFromInstance(graphicsIndex);
                            m_data->m_guiHelper->replaceTexture(shapeIndex, texHandle->m_openglTextureId);
                        }
                    }
                    if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_RGBA_COLOR)
                    {
                        if (m_data->m_pluginManager.getRenderInterface())
                        {
                            m_data->m_pluginManager.getRenderInterface()->changeRGBAColor(bodyUniqueId, linkIndex,
                                                                                          clientCmd.m_updateVisualShapeDataArguments.m_shapeIndex, clientCmd.m_updateVisualShapeDataArguments.m_rgbaColor);
                        }
                        m_data->m_guiHelper->changeRGBAColor(graphicsIndex, clientCmd.m_updateVisualShapeDataArguments.m_rgbaColor);
                    }
                    if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_SPECULAR_COLOR)
                    {
                        m_data->m_guiHelper->changeSpecularColor(graphicsIndex, clientCmd.m_updateVisualShapeDataArguments.m_specularColor);
                    }
                }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD

                else if (bodyHandle->m_softBody)
                {
                    i32 graphicsIndex = bodyHandle->m_softBody->getUserIndex();
                    if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_TEXTURE)
                                        {
                        i32 shapeIndex = m_data->m_guiHelper->getShapeIndexFromInstance(graphicsIndex);
                                                if (texHandle)
                                                {
                                                         m_data->m_guiHelper->replaceTexture(shapeIndex, texHandle->m_openglTextureId);
                                                }
                                                else
                                                {
                                                         m_data->m_guiHelper->replaceTexture(shapeIndex, -1);
                                                }
                                        }

                    if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_RGBA_COLOR)
                    {
                        if (m_data->m_pluginManager.getRenderInterface())
                        {
                            m_data->m_pluginManager.getRenderInterface()->changeRGBAColor(bodyUniqueId, linkIndex,
                              clientCmd.m_updateVisualShapeDataArguments.m_shapeIndex, clientCmd.m_updateVisualShapeDataArguments.m_rgbaColor);
                        }
                        m_data->m_guiHelper->changeRGBAColor(graphicsIndex, clientCmd.m_updateVisualShapeDataArguments.m_rgbaColor);
                    }

                    if (clientCmd.m_updateFlags & CMD_UPDATE_VISUAL_SHAPE_FLAGS)
                    {
                        if (m_data->m_pluginManager.getRenderInterface())
                        {
                            m_data->m_pluginManager.getRenderInterface()->changeInstanceFlags(bodyUniqueId, linkIndex,
                                clientCmd.m_updateVisualShapeDataArguments.m_shapeIndex,
                                clientCmd.m_updateVisualShapeDataArguments.m_flags);
                        }
                        m_data->m_guiHelper->changeInstanceFlags(graphicsIndex,
                            clientCmd.m_updateVisualShapeDataArguments.m_flags);
                    }

                }
#endif
            }
        }
    }

    serverCmd.m_type = CMD_VISUAL_SHAPE_UPDATE_COMPLETED;

    b3Notification notification;
    notification.m_notificationType = VISUAL_SHAPE_CHANGED;
    notification.m_visualShapeArgs.m_bodyUniqueId = clientCmd.m_updateVisualShapeDataArguments.m_bodyUniqueId;
    notification.m_visualShapeArgs.m_linkIndex = clientCmd.m_updateVisualShapeDataArguments.m_jointIndex;
    notification.m_visualShapeArgs.m_visualShapeIndex = clientCmd.m_updateVisualShapeDataArguments.m_shapeIndex;
    m_data->m_pluginManager.addNotification(notification);

    return hasStatus;
}

bool PhysicsServerCommandProcessor::processChangeTextureCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_CHANGE_TEXTURE_COMMAND_FAILED;

    InternalTextureHandle* texH = m_data->m_textureHandles.getHandle(clientCmd.m_changeTextureArgs.m_textureUniqueId);
    if (texH)
    {
        i32 gltex = texH->m_openglTextureId;
        m_data->m_guiHelper->changeTexture(gltex,
                                           (u8k*)bufferServerToClient, clientCmd.m_changeTextureArgs.m_width, clientCmd.m_changeTextureArgs.m_height);

        serverCmd.m_type = CMD_CLIENT_COMMAND_COMPLETED;
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processLoadTextureCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_LOAD_TEXTURE");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_LOAD_TEXTURE_FAILED;

    char relativeFileName[1024];
    char pathPrefix[1024];

    CommonFileIOInterface* fileIO(m_data->m_pluginManager.getFileIOInterface());
    if (fileIO->findResourcePath(clientCmd.m_loadTextureArguments.m_textureFileName, relativeFileName, 1024))
    {
        b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);

        i32 texHandle = m_data->m_textureHandles.allocHandle();
        InternalTextureHandle* texH = m_data->m_textureHandles.getHandle(texHandle);
        if (texH)
        {
            texH->m_tinyRendererTextureId = -1;
            texH->m_openglTextureId = -1;

            i32 uid = -1;
            if (m_data->m_pluginManager.getRenderInterface())
            {
                uid = m_data->m_pluginManager.getRenderInterface()->loadTextureFile(relativeFileName, fileIO);
            }
            if (uid >= 0)
            {
                texH->m_tinyRendererTextureId = uid;
            }

            {
                i32 width, height, n;
                u8* imageData = 0;

                CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
                if (fileIO)
                {
                    b3AlignedObjectArray<char> buffer;
                    buffer.reserve(1024);
                    i32 fileId = fileIO->fileOpen(relativeFileName, "rb");
                    if (fileId >= 0)
                    {
                        i32 size = fileIO->getFileSize(fileId);
                        if (size > 0)
                        {
                            buffer.resize(size);
                            i32 actual = fileIO->fileRead(fileId, &buffer[0], size);
                            if (actual != size)
                            {
                                drx3DWarning("image filesize mismatch!\n");
                                buffer.resize(0);
                            }
                        }
                        fileIO->fileClose(fileId);
                    }
                    if (buffer.size())
                    {
                        imageData = stbi_load_from_memory((u8k*)&buffer[0], buffer.size(), &width, &height, &n, 3);
                    }
                }
                else
                {
                    imageData = stbi_load(relativeFileName, &width, &height, &n, 3);
                }

                if (imageData)
                {
                    texH->m_openglTextureId = m_data->m_guiHelper->registerTexture(imageData, width, height);
                    m_data->m_allocatedTexturesRequireFree.push_back(imageData);
                }
                else
                {
                    drx3DWarning("Unsupported texture image format [%s]\n", relativeFileName);
                }
            }
            serverCmd.m_loadTextureResultArguments.m_textureUniqueId = texHandle;
            serverCmd.m_type = CMD_LOAD_TEXTURE_COMPLETED;
        }
    }
    else
    {
        serverCmd.m_type = CMD_LOAD_TEXTURE_FAILED;
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processSaveStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    DRX3D_PROFILE("CMD_SAVE_STATE");
    bool hasStatus = true;
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_SAVE_STATE_FAILED;

    DefaultSerializer* ser = new DefaultSerializer();
    i32 currentFlags = ser->getSerializationFlags();
    ser->setSerializationFlags(currentFlags | DRX3D_SERIALIZE_CONTACT_MANIFOLDS);
    m_data->m_dynamicsWorld->serialize(ser);
    bParse::BulletFile* bulletFile = new bParse::BulletFile((tuk)ser->getBufferPointer(), ser->getCurrentBufferSize());
    bulletFile->parse(false);
    if (bulletFile->ok())
    {
        serverCmd.m_type = CMD_SAVE_STATE_COMPLETED;
        //re-use state if available
        i32 reuseStateId = -1;
        for (i32 i = 0; i < m_data->m_savedStates.size(); i++)
        {
            if (m_data->m_savedStates[i].m_bulletFile == 0)
            {
                reuseStateId = i;
                break;
            }
        }
        SaveStateData sd;
        sd.m_bulletFile = bulletFile;
        sd.m_serializer = ser;
        if (reuseStateId >= 0)
        {
            serverCmd.m_saveStateResultArgs.m_stateId = reuseStateId;
            m_data->m_savedStates[reuseStateId] = sd;
        }
        else
        {
            serverCmd.m_saveStateResultArgs.m_stateId = m_data->m_savedStates.size();
            m_data->m_savedStates.push_back(sd);
        }
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRemoveStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    DRX3D_PROFILE("CMD_REMOVE_STATE");
    bool hasStatus = true;
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_REMOVE_STATE_FAILED;

    if (clientCmd.m_loadStateArguments.m_stateId >= 0)
    {
        if (clientCmd.m_loadStateArguments.m_stateId < m_data->m_savedStates.size())
        {
            SaveStateData& sd = m_data->m_savedStates[clientCmd.m_loadStateArguments.m_stateId];
            delete sd.m_bulletFile;
            delete sd.m_serializer;
            sd.m_bulletFile = 0;
            sd.m_serializer = 0;
            serverCmd.m_type = CMD_REMOVE_STATE_COMPLETED;
        }
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processRestoreStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{

    DRX3D_PROFILE("CMD_RESTORE_STATE");
    bool hasStatus = true;
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_RESTORE_STATE_FAILED;
#ifndef USE_DISCRETE_DYNAMICS_WORLD
    MultiBodyWorldImporter* importer = new MultiBodyWorldImporter(m_data->m_dynamicsWorld);

    importer->setImporterFlags(eRESTORE_EXISTING_OBJECTS);

    bool ok = false;

    if (clientCmd.m_loadStateArguments.m_stateId >= 0)
    {
        if (clientCmd.m_loadStateArguments.m_stateId < m_data->m_savedStates.size())
        {
            bParse::BulletFile* bulletFile = m_data->m_savedStates[clientCmd.m_loadStateArguments.m_stateId].m_bulletFile;
            if (bulletFile)
            {
                ok = importer->convertAllObjects(bulletFile);
            }
        }
    }
    else
    {
        bool found = false;
        char fileName[1024];
        fileName[0] = 0;

        CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
        b3AlignedObjectArray<char> buffer;
        buffer.reserve(1024);
        if (fileIO)
        {
            i32 fileId = -1;
            found = fileIO->findResourcePath(clientCmd.m_fileArguments.m_fileName, fileName, 1024);
            if (found)
            {
                fileId = fileIO->fileOpen(fileName, "rb");
            }
            if (fileId >= 0)
            {
                i32 size = fileIO->getFileSize(fileId);
                if (size > 0)
                {
                    buffer.resize(size);
                    i32 actual = fileIO->fileRead(fileId, &buffer[0], size);
                    if (actual != size)
                    {
                        drx3DWarning("image filesize mismatch!\n");
                        buffer.resize(0);
                    }
                    else
                    {
                        found = true;
                    }
                }
                fileIO->fileClose(fileId);
            }
        }

        if (found && buffer.size())
        {
            ok = importer->loadFileFromMemory(&buffer[0], buffer.size());
        }
        else
        {
            drx3DError("Error in restoreState: cannot load file %s\n", clientCmd.m_fileArguments.m_fileName);
        }
    }
    delete importer;
    if (ok)
    {
        serverCmd.m_type = CMD_RESTORE_STATE_COMPLETED;
    }
#endif
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processLoadBulletCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    DRX3D_PROFILE("CMD_LOAD_BULLET");

    bool hasStatus = true;
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_DRX3D_LOADING_FAILED;

#ifndef USE_DISCRETE_DYNAMICS_WORLD
    //BulletWorldImporter* importer = new BulletWorldImporter(m_data->m_dynamicsWorld);
    MultiBodyWorldImporter* importer = new MultiBodyWorldImporter(m_data->m_dynamicsWorld);

    bool found = false;

    CommonFileIOInterface* fileIO = m_data->m_pluginManager.getFileIOInterface();
    b3AlignedObjectArray<char> buffer;
    buffer.reserve(1024);
    if (fileIO)
    {
        char fileName[1024];
        i32 fileId = -1;
        found = fileIO->findResourcePath(clientCmd.m_fileArguments.m_fileName, fileName, 1024);
        if (found)
        {
            fileId = fileIO->fileOpen(fileName, "rb");
        }
        if (fileId >= 0)
        {
            i32 size = fileIO->getFileSize(fileId);
            if (size > 0)
            {
                buffer.resize(size);
                i32 actual = fileIO->fileRead(fileId, &buffer[0], size);
                if (actual != size)
                {
                    drx3DWarning("image filesize mismatch!\n");
                    buffer.resize(0);
                }
                else
                {
                    found = true;
                }
            }
            fileIO->fileClose(fileId);
        }
    }

    if (found && buffer.size())
    {
        bool ok = importer->loadFileFromMemory(&buffer[0], buffer.size());
        if (ok)
        {
            i32 numRb = importer->getNumRigidBodies();
            serverStatusOut.m_sdfLoadedArgs.m_numBodies = 0;
            serverStatusOut.m_sdfLoadedArgs.m_numUserConstraints = 0;

            for (i32 i = 0; i < numRb; i++)
            {
                CollisionObject2* colObj = importer->getRigidBodyByIndex(i);
                if (colObj)
                {
                    RigidBody* rb = RigidBody::upcast(colObj);
                    if (rb)
                    {
                        i32 bodyUniqueId = m_data->m_bodyHandles.allocHandle();
                        InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(bodyUniqueId);
                        colObj->setUserIndex2(bodyUniqueId);
                        bodyHandle->m_rigidBody = rb;

                        if (serverStatusOut.m_sdfLoadedArgs.m_numBodies < MAX_SDF_BODIES)
                        {
                            serverStatusOut.m_sdfLoadedArgs.m_numBodies++;
                            serverStatusOut.m_sdfLoadedArgs.m_bodyUniqueIds[i] = bodyUniqueId;
                        }

                        b3Notification notification;
                        notification.m_notificationType = BODY_ADDED;
                        notification.m_bodyArgs.m_bodyUniqueId = bodyUniqueId;
                        m_data->m_pluginManager.addNotification(notification);
                    }
                }
            }

            serverCmd.m_type = CMD_DRX3D_LOADING_COMPLETED;
            m_data->m_guiHelper->autogenerateGraphicsObjects(m_data->m_dynamicsWorld);
        }
    }
#endif
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processLoadMJCFCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_LOAD_MJCF");
    SharedMemoryStatus& serverCmd = serverStatusOut;
    serverCmd.m_type = CMD_MJCF_LOADING_FAILED;
    const MjcfArgs& mjcfArgs = clientCmd.m_mjcfArguments;
    if (m_data->m_verboseOutput)
    {
        drx3DPrintf("Processed CMD_LOAD_MJCF:%s", mjcfArgs.m_mjcfFileName);
    }
    bool useMultiBody = (clientCmd.m_updateFlags & URDF_ARGS_USE_MULTIBODY) ? (mjcfArgs.m_useMultiBody != 0) : true;
    i32 flags = CUF_USE_MJCF;
    if (clientCmd.m_updateFlags & URDF_ARGS_HAS_CUSTOM_URDF_FLAGS)
    {
        flags |= clientCmd.m_mjcfArguments.m_flags;
    }

    bool completedOk = loadMjcf(mjcfArgs.m_mjcfFileName, bufferServerToClient, bufferSizeInBytes, useMultiBody, flags);
    if (completedOk)
    {
        m_data->m_guiHelper->autogenerateGraphicsObjects(this->m_data->m_dynamicsWorld);

        serverStatusOut.m_sdfLoadedArgs.m_numBodies = m_data->m_sdfRecentLoadedBodies.size();
        serverStatusOut.m_sdfLoadedArgs.m_numUserConstraints = 0;
        i32 maxBodies = d3Min(MAX_SDF_BODIES, serverStatusOut.m_sdfLoadedArgs.m_numBodies);
        for (i32 i = 0; i < maxBodies; i++)
        {
            serverStatusOut.m_sdfLoadedArgs.m_bodyUniqueIds[i] = m_data->m_sdfRecentLoadedBodies[i];
        }

        serverStatusOut.m_type = CMD_MJCF_LOADING_COMPLETED;
    }
    else
    {
        serverStatusOut.m_type = CMD_MJCF_LOADING_FAILED;
    }
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processSaveBulletCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    bool hasStatus = true;

    DRX3D_PROFILE("CMD_SAVE_BULLET");
    SharedMemoryStatus& serverCmd = serverStatusOut;

    FILE* f = fopen(clientCmd.m_fileArguments.m_fileName, "wb");
    if (f)
    {
        DefaultSerializer* ser = new DefaultSerializer();
        i32 currentFlags = ser->getSerializationFlags();
        ser->setSerializationFlags(currentFlags | DRX3D_SERIALIZE_CONTACT_MANIFOLDS);

        m_data->m_dynamicsWorld->serialize(ser);
        fwrite(ser->getBufferPointer(), ser->getCurrentBufferSize(), 1, f);
        fclose(f);
        serverCmd.m_type = CMD_DRX3D_SAVING_COMPLETED;
        delete ser;
        return hasStatus;
    }
    serverCmd.m_type = CMD_DRX3D_SAVING_FAILED;
    return hasStatus;
}

bool PhysicsServerCommandProcessor::processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
    //  DRX3D_PROFILE("processCommand");

    i32 sz = sizeof(SharedMemoryStatus);
    i32 sz2 = sizeof(SharedMemoryCommand);

    bool hasStatus = false;

    if (m_data->m_commandLogger)
    {
        m_data->m_commandLogger->logCommand(clientCmd);
    }
    serverStatusOut.m_type = CMD_INVALID_STATUS;
    serverStatusOut.m_numDataStreamBytes = 0;
    serverStatusOut.m_dataStream = 0;

    //consume the command
    switch (clientCmd.m_type)
    {
        case CMD_STATE_LOGGING:
        {
            hasStatus = processStateLoggingCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_SET_VR_CAMERA_STATE:
        {
            hasStatus = processSetVRCameraStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_VR_EVENTS_DATA:
        {
            hasStatus = processRequestVREventsCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        };
        case CMD_REQUEST_MOUSE_EVENTS_DATA:
        {
            hasStatus = processRequestMouseEventsCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        };
        case CMD_REQUEST_KEYBOARD_EVENTS_DATA:
        {
            hasStatus = processRequestKeyboardEventsCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        };

        case CMD_REQUEST_RAY_CAST_INTERSECTIONS:
        {
            hasStatus = processRequestRaycastIntersectionsCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        };
        case CMD_REQUEST_DEBUG_LINES:
        {
            hasStatus = processRequestDebugLinesCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }

        case CMD_REQUEST_CAMERA_IMAGE_DATA:
        {
            hasStatus = processRequestCameraImageCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_SYNC_BODY_INFO:
        {
            hasStatus = processSyncBodyInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_BODY_INFO:
        {
            hasStatus = processRequestBodyInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_SAVE_WORLD:
        {
            hasStatus = processSaveWorldCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_LOAD_SDF:
        {
            hasStatus = processLoadSDFCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CREATE_COLLISION_SHAPE:
        {
            hasStatus = processCreateCollisionShapeCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CREATE_VISUAL_SHAPE:
        {
            hasStatus = processCreateVisualShapeCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_MESH_DATA:
        {
            hasStatus = processRequestMeshDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_RESET_MESH_DATA:
        {
            hasStatus = processResetMeshDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }

        case CMD_CREATE_MULTI_BODY:
        {
            hasStatus = processCreateMultiBodyCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_SET_ADDITIONAL_SEARCH_PATH:
        {
            hasStatus = processSetAdditionalSearchPathCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_LOAD_URDF:
        {
            hasStatus = processLoadURDFCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_LOAD_SOFT_BODY:
        {
            hasStatus = processLoadSoftBodyCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CREATE_SENSOR:
        {
            hasStatus = processCreateSensorCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_PROFILE_TIMING:
        {
            hasStatus = processProfileTimingCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }

        case CMD_SEND_DESIRED_STATE:
        {
            hasStatus = processSendDesiredStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_COLLISION_INFO:
        {
            hasStatus = processRequestCollisionInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_ACTUAL_STATE:
        {
            hasStatus = processRequestActualStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_STEP_FORWARD_SIMULATION:
        {
            hasStatus = processForwardDynamicsCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_PERFORM_COLLISION_DETECTION:
        {
            hasStatus = performCollisionDetectionCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }

        case CMD_REQUEST_INTERNAL_DATA:
        {
            hasStatus = processRequestInternalDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        };
        case CMD_CHANGE_DYNAMICS_INFO:
        {
            hasStatus = processChangeDynamicsInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        };
        case CMD_GET_DYNAMICS_INFO:
        {
            hasStatus = processGetDynamicsInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }

        case CMD_REQUEST_PHYSICS_SIMULATION_PARAMETERS:
        {
            hasStatus = processRequestPhysicsSimulationParametersCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }

        case CMD_SEND_PHYSICS_SIMULATION_PARAMETERS:
        {
            hasStatus = processSendPhysicsParametersCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        };
        case CMD_INIT_POSE:
        {
            hasStatus = processInitPoseCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_RESET_SIMULATION:
        {
            hasStatus = processResetSimulationCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CREATE_RIGID_BODY:
        {
            hasStatus = processCreateRigidBodyCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CREATE_BOX_COLLISION_SHAPE:
        {
            //for backward compatibility, CMD_CREATE_BOX_COLLISION_SHAPE is the same as CMD_CREATE_RIGID_BODY
            hasStatus = processCreateRigidBodyCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_PICK_BODY:
        {
            hasStatus = processPickBodyCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_MOVE_PICKED_BODY:
        {
            hasStatus = processMovePickedBodyCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REMOVE_PICKING_CONSTRAINT_BODY:
        {
            hasStatus = processRemovePickingConstraintCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_AABB_OVERLAP:
        {
            hasStatus = processRequestAabbOverlapCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_OPENGL_VISUALIZER_CAMERA:
        {
            hasStatus = processRequestOpenGLVisualizeCameraCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CONFIGURE_OPENGL_VISUALIZER:
        {
            hasStatus = processConfigureOpenGLVisualizerCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_CONTACT_POINT_INFORMATION:
        {
            hasStatus = processRequestContactpointInformationCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CALCULATE_INVERSE_DYNAMICS:
        {
            hasStatus = processdrx3d_inverseCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CALCULATE_JACOBIAN:
        {
            hasStatus = processCalculateJacobianCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CALCULATE_MASS_MATRIX:
        {
            hasStatus = processCalculateMassMatrixCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_APPLY_EXTERNAL_FORCE:
        {
            hasStatus = processApplyExternalForceCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REMOVE_BODY:
        {
            hasStatus = processRemoveBodyCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_USER_CONSTRAINT:
        {
            hasStatus = processCreateUserConstraintCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CALCULATE_INVERSE_KINEMATICS:
        {
            if (clientCmd.m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices == 1)
            {
                hasStatus = processCalculateInverseKinematicsCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            }
            else
            {
                hasStatus = processCalculateInverseKinematicsCommand2(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            }
            break;
        }
        case CMD_REQUEST_VISUAL_SHAPE_INFO:
        {
            hasStatus = processRequestVisualShapeInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_COLLISION_SHAPE_INFO:
        {
            hasStatus = processRequestCollisionShapeInfoCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_UPDATE_VISUAL_SHAPE:
        {
            hasStatus = processUpdateVisualShapeCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CHANGE_TEXTURE:
        {
            hasStatus = processChangeTextureCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_LOAD_TEXTURE:
        {
            hasStatus = processLoadTextureCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_RESTORE_STATE:
        {
            hasStatus = processRestoreStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }

        case CMD_SAVE_STATE:
        {
            hasStatus = processSaveStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REMOVE_STATE:
        {
            hasStatus = processRemoveStateCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }

        case CMD_LOAD_BULLET:
        {
            hasStatus = processLoadBulletCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_SAVE_BULLET:
        {
            hasStatus = processSaveBulletCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_LOAD_MJCF:
        {
            hasStatus = processLoadMJCFCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_USER_DEBUG_DRAW:
        {
            hasStatus = processUserDebugDrawCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_CUSTOM_COMMAND:
        {
            hasStatus = processCustomCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_SYNC_USER_DATA:
        {
            hasStatus = processSyncUserDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REQUEST_USER_DATA:
        {
            hasStatus = processRequestUserDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_ADD_USER_DATA:
        {
            hasStatus = processAddUserDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_REMOVE_USER_DATA:
        {
            hasStatus = processRemoveUserDataCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        case CMD_COLLISION_FILTER:
        {
            hasStatus = processCollisionFilterCommand(clientCmd, serverStatusOut, bufferServerToClient, bufferSizeInBytes);
            break;
        }
        default:
        {
            DRX3D_PROFILE("CMD_UNKNOWN");
            drx3DError("Unknown command encountered");
            SharedMemoryStatus& serverCmd = serverStatusOut;
            serverCmd.m_type = CMD_UNKNOWN_COMMAND_FLUSHED;
            hasStatus = true;
        }
    };

    return hasStatus;
}

void PhysicsServerCommandProcessor::syncPhysicsToGraphics()
{
    m_data->m_guiHelper->syncPhysicsToGraphics(m_data->m_dynamicsWorld);
}

void PhysicsServerCommandProcessor::syncPhysicsToGraphics2()
{
    m_data->m_guiHelper->syncPhysicsToGraphics2(m_data->m_dynamicsWorld);
}

void PhysicsServerCommandProcessor::renderScene(i32 renderFlags)
{
    if (m_data->m_guiHelper)
    {
        if (0 == (renderFlags & COV_DISABLE_SYNC_RENDERING))
        {
            m_data->m_guiHelper->syncPhysicsToGraphics(m_data->m_dynamicsWorld);
        }

        m_data->m_guiHelper->render(m_data->m_dynamicsWorld);
    }
}

void PhysicsServerCommandProcessor::physicsDebugDraw(i32 debugDrawFlags)
{
    if (m_data->m_dynamicsWorld)
    {
        if (m_data->m_dynamicsWorld->getDebugDrawer())
        {
            m_data->m_dynamicsWorld->getDebugDrawer()->setDebugMode(debugDrawFlags);
            m_data->m_dynamicsWorld->debugDrawWorld();

#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
            {
                DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
                if (deformWorld)
                {
                    for (i32 i = 0; i < deformWorld->getSoftBodyArray().size(); i++)
                    {
                        SoftBody* psb = (SoftBody*)deformWorld->getSoftBodyArray()[i];
                        if (m_data->m_dynamicsWorld->getDebugDrawer() && !(m_data->m_dynamicsWorld->getDebugDrawer()->getDebugMode() & (IDebugDraw::DBG_DrawWireframe)))
                        {
                            //SoftBodyHelpers::DrawFrame(psb,m_data->m_dynamicsWorld->getDebugDrawer());
                            SoftBodyHelpers::Draw(psb, m_data->m_dynamicsWorld->getDebugDrawer(), deformWorld->getDrawFlags());
                        }
                    }
                }
            }
            {
                SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
                if (softWorld)
                {
                    for (i32 i = 0; i < softWorld->getSoftBodyArray().size(); i++)
                    {
                        SoftBody* psb = (SoftBody*)softWorld->getSoftBodyArray()[i];
                        if (m_data->m_dynamicsWorld->getDebugDrawer() && !(m_data->m_dynamicsWorld->getDebugDrawer()->getDebugMode() & (IDebugDraw::DBG_DrawWireframe)))
                        {
                            //SoftBodyHelpers::DrawFrame(psb,m_data->m_dynamicsWorld->getDebugDrawer());
                            SoftBodyHelpers::Draw(psb, m_data->m_dynamicsWorld->getDebugDrawer(), softWorld->getDrawFlags());
                        }
                    }
                }
            }
#endif
        }
    }
}

struct MyResultCallback : public CollisionWorld::ClosestRayResultCallback
{
    i32 m_faceId;

    MyResultCallback(const Vec3& rayFromWorld, const Vec3& rayToWorld)
        : CollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld),
          m_faceId(-1)
    {
    }

    virtual bool needsCollision(BroadphaseProxy* proxy0) const
    {
        return true;
    }

    virtual Scalar addSingleResult(CollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
    {
        //caller already does the filter on the m_closestHitFraction
        Assert(rayResult.m_hitFraction <= m_closestHitFraction);

        m_closestHitFraction = rayResult.m_hitFraction;
        m_collisionObject = rayResult.m_collisionObject;
        if (rayResult.m_localShapeInfo)
        {
            m_faceId = rayResult.m_localShapeInfo->m_triangleIndex;
        }
        else
        {
            m_faceId = -1;
        }
        if (normalInWorldSpace)
        {
            m_hitNormalWorld = rayResult.m_hitNormalLocal;
        }
        else
        {
            ///need to transform normal into worldspace
            m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal;
        }
        m_hitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);
        return rayResult.m_hitFraction;
    }
};

bool PhysicsServerCommandProcessor::pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld)
{
    if (m_data->m_dynamicsWorld == 0)
        return false;

    //CollisionWorld::ClosestRayResultCallback rayCallback(rayFromWorld, rayToWorld);
    MyResultCallback rayCallback(rayFromWorld, rayToWorld);
    rayCallback.m_flags |= TriangleRaycastCallback::kF_UseGjkConvexCastRaytest;
    m_data->m_dynamicsWorld->rayTest(rayFromWorld, rayToWorld, rayCallback);
    if (rayCallback.hasHit())
    {
        Vec3 pickPos = rayCallback.m_hitPointWorld;

        RigidBody* body = (RigidBody*)RigidBody::upcast(rayCallback.m_collisionObject);
        if (body)
        {
            //other exclusions?
            if (!(body->isStaticObject() || body->isKinematicObject()))
            {
                m_data->m_pickedBody = body;
                m_data->m_savedActivationState = body->getActivationState();
                if (m_data->m_savedActivationState==ISLAND_SLEEPING)
                {
                    m_data->m_savedActivationState = ACTIVE_TAG;
                }
                m_data->m_pickedBody->setActivationState(DISABLE_DEACTIVATION);
                m_data->m_pickedBody->setDeactivationTime(0);
                //printf("pickPos=%f,%f,%f\n",pickPos.getX(),pickPos.getY(),pickPos.getZ());
                Vec3 localPivot = body->getCenterOfMassTransform().inverse() * pickPos;
                Point2PointConstraint* p2p = new Point2PointConstraint(*body, localPivot);
                m_data->m_dynamicsWorld->addConstraint(p2p, true);
                m_data->m_pickedConstraint = p2p;
                Scalar mousePickClamping = 30.f;
                p2p->m_setting.m_impulseClamp = mousePickClamping;
                //very weak constraint for picking
                p2p->m_setting.m_tau = 0.001f;
            }
        }
        else
        {
            MultiBodyLinkCollider* multiCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(rayCallback.m_collisionObject);
            if (multiCol && multiCol->m_multiBody)
            {
                m_data->m_prevCanSleep = multiCol->m_multiBody->getCanSleep();
                multiCol->m_multiBody->setCanSleep(false);

                Vec3 pivotInA = multiCol->m_multiBody->worldPosToLocal(multiCol->m_link, pickPos);

                MultiBodyPoint2Point* p2p = new MultiBodyPoint2Point(multiCol->m_multiBody, multiCol->m_link, 0, pivotInA, pickPos);
                //if you add too much energy to the system, causing high angular velocities, simulation 'explodes'
                //see also http://www.bulletphysics.org/drx3D/phpBB3/viewtopic.php?f=4&t=949
                //so we try to avoid it by clamping the maximum impulse (force) that the mouse pick can apply
                //it is not satisfying, hopefully we find a better solution (higher order integrator, using joint friction using a zero-velocity target motor with limited force etc?)
                Scalar scaling = 10;
                p2p->setMaxAppliedImpulse(2 * scaling);

                MultiBodyDynamicsWorld* world = (MultiBodyDynamicsWorld*)m_data->m_dynamicsWorld;
                world->addMultiBodyConstraint(p2p);
                m_data->m_pickingMultiBodyPoint2Point = p2p;
            }
            else
            {
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
                //deformable/soft body?
                SoftBody* psb = (SoftBody*)SoftBody::upcast(rayCallback.m_collisionObject);
                if (psb)
                {
                    DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
                    if (deformWorld)
                    {
                        i32 face_id = rayCallback.m_faceId;
                        if (face_id >= 0 && face_id < psb->m_faces.size())
                        {
                            m_data->m_pickedSoftBody = psb;
                            psb->setActivationState(DISABLE_DEACTIVATION);
                            const SoftBody::Face& f = psb->m_faces[face_id];
                            DeformableMousePickingForce* mouse_force = new DeformableMousePickingForce(100, 0, f, pickPos, m_data->m_maxPickingForce);
                            m_data->m_mouseForce = mouse_force;

                            deformWorld->addForce(psb, mouse_force);
                        }
                    }
                }
#endif
            }
        }

        //                  pickObject(pickPos, rayCallback.m_collisionObject);
        m_data->m_oldPickingPos = rayToWorld;
        m_data->m_hitPos = pickPos;
        m_data->m_oldPickingDist = (pickPos - rayFromWorld).length();
        //                  printf("hit !\n");
        //add p2p
    }
    return false;
}

bool PhysicsServerCommandProcessor::movePickedBody(const Vec3& rayFromWorld, const Vec3& rayToWorld)
{
    if (m_data->m_pickedBody && m_data->m_pickedConstraint)
    {
        Point2PointConstraint* pickCon = static_cast<Point2PointConstraint*>(m_data->m_pickedConstraint);
        if (pickCon)
        {
            //keep it at the same picking distance

            Vec3 dir = rayToWorld - rayFromWorld;
            dir.normalize();
            dir *= m_data->m_oldPickingDist;

            Vec3 newPivotB = rayFromWorld + dir;
            pickCon->setPivotB(newPivotB);
        }
    }

    if (m_data->m_pickingMultiBodyPoint2Point)
    {
        //keep it at the same picking distance

        Vec3 dir = rayToWorld - rayFromWorld;
        dir.normalize();
        dir *= m_data->m_oldPickingDist;

        Vec3 newPivotB = rayFromWorld + dir;

        m_data->m_pickingMultiBodyPoint2Point->setPivotInB(newPivotB);
    }

#ifndef SKIP_DEFORMABLE_BODY
    if (m_data->m_pickedSoftBody)
    {
        if (m_data->m_pickedSoftBody && m_data->m_mouseForce)
        {
            Vec3 newPivot;
            Vec3 dir = rayToWorld - rayFromWorld;
            dir.normalize();
            dir *= m_data->m_oldPickingDist;
            newPivot = rayFromWorld + dir;
            m_data->m_mouseForce->setMousePos(newPivot);
        }
    }
#endif

    return false;
}

void PhysicsServerCommandProcessor::removePickingConstraint()
{
    if (m_data->m_pickedConstraint)
    {
        m_data->m_dynamicsWorld->removeConstraint(m_data->m_pickedConstraint);
        delete m_data->m_pickedConstraint;
        m_data->m_pickedConstraint = 0;
        m_data->m_pickedBody->forceActivationState(m_data->m_savedActivationState);
        m_data->m_pickedBody = 0;
    }
    if (m_data->m_pickingMultiBodyPoint2Point)
    {
        m_data->m_pickingMultiBodyPoint2Point->getMultiBodyA()->setCanSleep(m_data->m_prevCanSleep);
        MultiBodyDynamicsWorld* world = (MultiBodyDynamicsWorld*)m_data->m_dynamicsWorld;
        world->removeMultiBodyConstraint(m_data->m_pickingMultiBodyPoint2Point);
        delete m_data->m_pickingMultiBodyPoint2Point;
        m_data->m_pickingMultiBodyPoint2Point = 0;
    }

#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    //deformable/soft body?
    DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
    if (deformWorld && m_data->m_mouseForce)
    {
        deformWorld->removeForce(m_data->m_pickedSoftBody, m_data->m_mouseForce);
        delete m_data->m_mouseForce;
        m_data->m_mouseForce = 0;
        m_data->m_pickedSoftBody = 0;
    }
#endif
}

void PhysicsServerCommandProcessor::enableCommandLogging(bool enable, tukk fileName)
{
    if (enable)
    {
        if (0 == m_data->m_commandLogger)
        {
            m_data->m_commandLogger = new CommandLogger(fileName);
        }
    }
    else
    {
        if (0 != m_data->m_commandLogger)
        {
            delete m_data->m_commandLogger;
            m_data->m_commandLogger = 0;
        }
    }
}

void PhysicsServerCommandProcessor::replayFromLogFile(tukk fileName)
{
    CommandLogPlayback* pb = new CommandLogPlayback(fileName);
    m_data->m_logPlayback = pb;
}

i32 gDroppedSimulationSteps = 0;
i32 gNumSteps = 0;
double gDtInSec = 0.f;
double gSubStep = 0.f;

void PhysicsServerCommandProcessor::enableRealTimeSimulation(bool enableRealTimeSim)
{
    m_data->m_useRealTimeSimulation = enableRealTimeSim;
}

bool PhysicsServerCommandProcessor::isRealTimeSimulationEnabled() const
{
    return m_data->m_useRealTimeSimulation;
}

void PhysicsServerCommandProcessor::stepSimulationRealTime(double dtInSec, const struct b3VRControllerEvent* vrControllerEvents, i32 numVRControllerEvents, const struct b3KeyboardEvent* keyEvents, i32 numKeyEvents, const struct b3MouseEvent* mouseEvents, i32 numMouseEvents)
{
    m_data->m_vrControllerEvents.addNewVREvents(vrControllerEvents, numVRControllerEvents);
    m_data->m_pluginManager.addEvents(vrControllerEvents, numVRControllerEvents, keyEvents, numKeyEvents, mouseEvents, numMouseEvents);

    for (i32 i = 0; i < m_data->m_stateLoggers.size(); i++)
    {
        if (m_data->m_stateLoggers[i]->m_loggingType == STATE_LOGGING_VR_CONTROLLERS)
        {
            VRControllerStateLogger* vrLogger = (VRControllerStateLogger*)m_data->m_stateLoggers[i];
            vrLogger->m_vrEvents.addNewVREvents(vrControllerEvents, numVRControllerEvents);
        }
    }

    for (i32 ii = 0; ii < numMouseEvents; ii++)
    {
        const b3MouseEvent& event = mouseEvents[ii];
        bool found = false;
        //search a matching one first, otherwise add new event
        for (i32 e = 0; e < m_data->m_mouseEvents.size(); e++)
        {
            if (event.m_eventType == m_data->m_mouseEvents[e].m_eventType)
            {
                if (event.m_eventType == MOUSE_MOVE_EVENT)
                {
                    m_data->m_mouseEvents[e].m_mousePosX = event.m_mousePosX;
                    m_data->m_mouseEvents[e].m_mousePosY = event.m_mousePosY;
                    found = true;
                }
                else if ((event.m_eventType == MOUSE_BUTTON_EVENT) && event.m_buttonIndex == m_data->m_mouseEvents[e].m_buttonIndex)
                {
                    m_data->m_mouseEvents[e].m_buttonState |= event.m_buttonState;
                    if (event.m_buttonState & eButtonIsDown)
                    {
                        m_data->m_mouseEvents[e].m_buttonState |= eButtonIsDown;
                    }
                    else
                    {
                        m_data->m_mouseEvents[e].m_buttonState &= ~eButtonIsDown;
                    }
                    found = true;
                }
            }
        }
        if (!found)
        {
            m_data->m_mouseEvents.push_back(event);
        }
    }

    for (i32 i = 0; i < numKeyEvents; i++)
    {
        const b3KeyboardEvent& event = keyEvents[i];
        bool found = false;
        //search a matching one first, otherwise add new event
        for (i32 e = 0; e < m_data->m_keyboardEvents.size(); e++)
        {
            if (event.m_keyCode == m_data->m_keyboardEvents[e].m_keyCode)
            {
                m_data->m_keyboardEvents[e].m_keyState |= event.m_keyState;
                if (event.m_keyState & eButtonIsDown)
                {
                    m_data->m_keyboardEvents[e].m_keyState |= eButtonIsDown;
                }
                else
                {
                    m_data->m_keyboardEvents[e].m_keyState &= ~eButtonIsDown;
                }
                found = true;
            }
        }
        if (!found)
        {
            m_data->m_keyboardEvents.push_back(event);
        }
    }
    if (gResetSimulation)
    {
        resetSimulation();
        gResetSimulation = false;
    }

    if (gVRTrackingObjectUniqueId >= 0)
    {
        InternalBodyHandle* bodyHandle = m_data->m_bodyHandles.getHandle(gVRTrackingObjectUniqueId);
        if (bodyHandle && bodyHandle->m_multiBody)
        {
            //          gVRTrackingObjectTr  = bodyHandle->m_multiBody->getBaseWorldTransform();

            if (gVRTrackingObjectUniqueId >= 0)
            {
                gVRTrackingObjectTr.setOrigin(bodyHandle->m_multiBody->getBaseWorldTransform().getOrigin());
                gVRTeleportPos1 = gVRTrackingObjectTr.getOrigin();
            }
            if (gVRTrackingObjectFlag & VR_CAMERA_TRACK_OBJECT_ORIENTATION)
            {
                gVRTrackingObjectTr.setBasis(bodyHandle->m_multiBody->getBaseWorldTransform().getBasis());
                gVRTeleportOrn = gVRTrackingObjectTr.getRotation();
            }
        }
    }

    if ((m_data->m_useRealTimeSimulation) && m_data->m_guiHelper)
    {
        i32 maxSteps = m_data->m_numSimulationSubSteps + 3;
        if (m_data->m_numSimulationSubSteps)
        {
            gSubStep = m_data->m_physicsDeltaTime / m_data->m_numSimulationSubSteps;
        }
        else
        {
            gSubStep = m_data->m_physicsDeltaTime;
        }

        Scalar deltaTimeScaled = dtInSec * simTimeScalingFactor;
        i32 numSteps = m_data->m_dynamicsWorld->stepSimulation(deltaTimeScaled, maxSteps, gSubStep);
        m_data->m_simulationTimestamp += deltaTimeScaled;
        gDroppedSimulationSteps += numSteps > maxSteps ? numSteps - maxSteps : 0;

        if (numSteps)
        {
            gNumSteps = numSteps;
            gDtInSec = dtInSec;

            addBodyChangedNotifications();
        }
    }
}

b3Notification createTransformChangedNotification(i32 bodyUniqueId, i32 linkIndex, const CollisionObject2* colObj)
{
    b3Notification notification;
    notification.m_notificationType = TRANSFORM_CHANGED;
    notification.m_transformChangeArgs.m_bodyUniqueId = bodyUniqueId;
    notification.m_transformChangeArgs.m_linkIndex = linkIndex;

    const Transform2& tr = colObj->getWorldTransform();
    notification.m_transformChangeArgs.m_worldPosition[0] = tr.getOrigin()[0];
    notification.m_transformChangeArgs.m_worldPosition[1] = tr.getOrigin()[1];
    notification.m_transformChangeArgs.m_worldPosition[2] = tr.getOrigin()[2];

    notification.m_transformChangeArgs.m_worldRotation[0] = tr.getRotation()[0];
    notification.m_transformChangeArgs.m_worldRotation[1] = tr.getRotation()[1];
    notification.m_transformChangeArgs.m_worldRotation[2] = tr.getRotation()[2];
    notification.m_transformChangeArgs.m_worldRotation[3] = tr.getRotation()[3];

    const Vec3& scaling = colObj->getCollisionShape()->getLocalScaling();
    notification.m_transformChangeArgs.m_localScaling[0] = scaling[0];
    notification.m_transformChangeArgs.m_localScaling[1] = scaling[1];
    notification.m_transformChangeArgs.m_localScaling[2] = scaling[2];
    return notification;
}

#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
b3Notification createSoftBodyChangedNotification(i32 bodyUniqueId, i32 linkIndex)
{
    b3Notification notification;
    notification.m_notificationType = SOFTBODY_CHANGED;
    notification.m_softBodyChangeArgs.m_bodyUniqueId = bodyUniqueId;
    notification.m_softBodyChangeArgs.m_linkIndex = linkIndex;
    return notification;
}
#endif

void PhysicsServerCommandProcessor::addBodyChangedNotifications()
{
    b3Notification notification;
    notification.m_notificationType = SIMULATION_STEPPED;
    m_data->m_pluginManager.addNotification(notification);

    b3AlignedObjectArray<i32> usedHandles;
    m_data->m_bodyHandles.getUsedHandles(usedHandles);
    for (i32 i = 0; i < usedHandles.size(); i++)
    {
        i32 bodyUniqueId = usedHandles[i];
        InternalBodyData* bodyData = m_data->m_bodyHandles.getHandle(bodyUniqueId);
        if (!bodyData)
        {
            continue;
        }
        if (bodyData->m_multiBody)
        {
            MultiBody* mb = bodyData->m_multiBody;
            if (mb->getBaseCollider()->isActive())
            {
                m_data->m_pluginManager.addNotification(createTransformChangedNotification(bodyUniqueId, -1, mb->getBaseCollider()));
            }
            for (i32 linkIndex = 0; linkIndex < mb->getNumLinks(); linkIndex++)
            {
                if (mb->getLinkCollider(linkIndex)->isActive())
                {
                    m_data->m_pluginManager.addNotification(createTransformChangedNotification(bodyUniqueId, linkIndex, mb->getLinkCollider(linkIndex)));
                }
            }
        }
        else if (bodyData->m_rigidBody && bodyData->m_rigidBody->isActive())
        {
            m_data->m_pluginManager.addNotification(createTransformChangedNotification(bodyUniqueId, -1, bodyData->m_rigidBody));
        }
#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
        else if (bodyData->m_softBody)
        {
            i32 linkIndex = -1;
            m_data->m_pluginManager.addNotification(createSoftBodyChangedNotification(bodyUniqueId, linkIndex));
        }
#endif
    }
}

void PhysicsServerCommandProcessor::resetSimulation(i32 flags)
{
    //clean up all data
    m_data->m_remoteSyncTransformTime = m_data->m_remoteSyncTransformInterval;

    m_data->m_simulationTimestamp = 0;
    m_data->m_cachedVUrdfisualShapes.clear();

#ifndef SKIP_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD
    if (m_data && m_data->m_dynamicsWorld)
    {
        {
            DeformableMultiBodyDynamicsWorld* deformWorld = getDeformableWorld();
            if (deformWorld)
            {
                deformWorld->getWorldInfo().m_sparsesdf.Reset();
            }
        }
        {
            SoftMultiBodyDynamicsWorld* softWorld = getSoftWorld();
            if (softWorld)
            {
                softWorld->getWorldInfo().m_sparsesdf.Reset();
            }
        }
    }
#endif
    if (m_data && m_data->m_guiHelper)
    {
        m_data->m_guiHelper->removeAllGraphicsInstances();
        m_data->m_guiHelper->removeAllUserDebugItems();
    }
    if (m_data)
    {
        if (m_data->m_pluginManager.getRenderInterface())
        {
            m_data->m_pluginManager.getRenderInterface()->resetAll();
        }

        if (m_data->m_pluginManager.getCollisionInterface())
        {
            m_data->m_pluginManager.getCollisionInterface()->resetAll();
        }

        for (i32 i = 0; i < m_data->m_savedStates.size(); i++)
        {
            delete m_data->m_savedStates[i].m_bulletFile;
            delete m_data->m_savedStates[i].m_serializer;
        }
        m_data->m_savedStates.clear();
    }

    removePickingConstraint();

    deleteDynamicsWorld();
    createEmptyDynamicsWorld(flags);

    m_data->m_bodyHandles.exitHandles();
    m_data->m_bodyHandles.initHandles();

    m_data->m_userVisualShapeHandles.exitHandles();
    m_data->m_userVisualShapeHandles.initHandles();

    m_data->m_userCollisionShapeHandles.exitHandles();
    m_data->m_userCollisionShapeHandles.initHandles();

    m_data->m_userDataHandles.exitHandles();
    m_data->m_userDataHandles.initHandles();
    m_data->m_userDataHandleLookup.clear();

    b3Notification notification;
    notification.m_notificationType = SIMULATION_RESET;
    m_data->m_pluginManager.addNotification(notification);

    syncPhysicsToGraphics2();
}

void PhysicsServerCommandProcessor::setTimeOut(double /*timeOutInSeconds*/)
{
}

const Vec3& PhysicsServerCommandProcessor::getVRTeleportPosition() const
{
    return gVRTeleportPos1;
}
void PhysicsServerCommandProcessor::setVRTeleportPosition(const Vec3& vrTeleportPos)
{
    gVRTeleportPos1 = vrTeleportPos;
}
const Quat& PhysicsServerCommandProcessor::getVRTeleportOrientation() const
{
    return gVRTeleportOrn;
}
void PhysicsServerCommandProcessor::setVRTeleportOrientation(const Quat& vrTeleportOrn)
{
    gVRTeleportOrn = vrTeleportOrn;
}
