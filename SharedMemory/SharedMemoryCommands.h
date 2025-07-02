#ifndef SHARED_MEMORY_COMMANDS_H
#define SHARED_MEMORY_COMMANDS_H

//this is a very experimental draft of commands. We will iterate on this API (commands, arguments etc)

#include <drx3D/SharedMemory/SharedMemoryPublic.h>

#ifdef __GNUC__
#include <stdint.h>
typedef int32_t smInt32_t;
typedef int64_t smInt64_t;
typedef uint32_t smUint32_t;
typedef uint64_t smUint64_t;
#elif defined(_MSC_VER)
typedef __int32 smInt32_t;
typedef __int64 smInt64_t;
typedef unsigned __int32 smUint32_t;
typedef unsigned __int64 smUint64_t;
#else
typedef i32 smInt32_t;
typedef z64 smInt64_t;
typedef u32 smUint32_t;
typedef zu64 smUint64_t;
#endif

#ifdef __APPLE__
#define SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE (1024 * 1024)
#else
#define SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE (8 * 1024 * 1024)
#endif

#define SHARED_MEMORY_SERVER_TEST_C
#define MAX_DEGREE_OF_FREEDOM 128
#define MAX_NUM_SENSORS 256
#define MAX_URDF_FILENAME_LENGTH 1024
#define MAX_SDF_FILENAME_LENGTH 1024
#define MAX_FILENAME_LENGTH MAX_URDF_FILENAME_LENGTH
#define MAX_NUM_LINKS MAX_DEGREE_OF_FREEDOM


struct TmpFloat3
{
	float m_x;
	float m_y;
	float m_z;
};

#ifdef _WIN32
__inline
#else
inline
#endif
	TmpFloat3
	CreateTmpFloat3(float x, float y, float z)
{
	TmpFloat3 tmp;
	tmp.m_x = x;
	tmp.m_y = y;
	tmp.m_z = z;
	return tmp;
}

enum EnumSdfArgsUpdateFlags
{
	SDF_ARGS_FILE_NAME = 1,
};

struct SdfArgs
{
	char m_sdfFileName[MAX_URDF_FILENAME_LENGTH];
	i32 m_useMultiBody;
	double m_globalScaling;
};

struct FileArgs
{
	char m_fileName[MAX_URDF_FILENAME_LENGTH];
	i32 m_stateId;
};

enum EnumLoadStateArgsUpdateFlags
{
	CMD_LOAD_STATE_HAS_STATEID = 1,
	CMD_LOAD_STATE_HAS_FILENAME = 2,
};

enum EnumUrdfArgsUpdateFlags
{
	URDF_ARGS_FILE_NAME = 1,
	URDF_ARGS_INITIAL_POSITION = 2,
	URDF_ARGS_INITIAL_ORIENTATION = 4,
	URDF_ARGS_USE_MULTIBODY = 8,
	URDF_ARGS_USE_FIXED_BASE = 16,
	URDF_ARGS_HAS_CUSTOM_URDF_FLAGS = 32,
	URDF_ARGS_USE_GLOBAL_SCALING = 64,
};

struct UrdfArgs
{
	char m_urdfFileName[MAX_URDF_FILENAME_LENGTH];
	double m_initialPosition[3];
	double m_initialOrientation[4];
	i32 m_useMultiBody;
	i32 m_useFixedBase;
	i32 m_urdfFlags;
	double m_globalScaling;
};

struct MjcfArgs
{
	char m_mjcfFileName[MAX_URDF_FILENAME_LENGTH];
	i32 m_useMultiBody;
	i32 m_flags;
};

struct b3SearchPathfArgs
{
	char m_path[MAX_FILENAME_LENGTH];
};

enum CustomCommandEnum
{
	CMD_CUSTOM_COMMAND_LOAD_PLUGIN = 1,
	CMD_CUSTOM_COMMAND_UNLOAD_PLUGIN = 2,
	CMD_CUSTOM_COMMAND_EXECUTE_PLUGIN_COMMAND = 4,
	CMD_CUSTOM_COMMAND_LOAD_PLUGIN_POSTFIX = 8,
};

struct b3CustomCommand
{
	i32 m_pluginUniqueId;
	b3PluginArguments m_arguments;
	char m_pluginPath[MAX_FILENAME_LENGTH];
	char m_postFix[MAX_FILENAME_LENGTH];
	i32 m_startingReturnBytes;
};

struct b3CustomCommandResultArgs
{
	i32 m_pluginUniqueId;
	i32 m_executeCommandResult;
	i32 m_returnDataType;
	i32 m_returnDataSizeInBytes;
	i32 m_returnDataStart;
};

struct BulletDataStreamArgs
{
	char m_bulletFileName[MAX_FILENAME_LENGTH];
	i32 m_bodyUniqueId;
	char m_bodyName[MAX_FILENAME_LENGTH];
};

enum EnumChangeDynamicsInfoFlags
{
	CHANGE_DYNAMICS_INFO_SET_MASS = 1,
	CHANGE_DYNAMICS_INFO_SET_COM = 2,
	CHANGE_DYNAMICS_INFO_SET_LATERAL_FRICTION = 4,
	CHANGE_DYNAMICS_INFO_SET_SPINNING_FRICTION = 8,
	CHANGE_DYNAMICS_INFO_SET_ROLLING_FRICTION = 16,
	CHANGE_DYNAMICS_INFO_SET_RESTITUTION = 32,
	CHANGE_DYNAMICS_INFO_SET_LINEAR_DAMPING = 64,
	CHANGE_DYNAMICS_INFO_SET_ANGULAR_DAMPING = 128,
	CHANGE_DYNAMICS_INFO_SET_CONTACT_STIFFNESS_AND_DAMPING = 256,
	CHANGE_DYNAMICS_INFO_SET_FRICTION_ANCHOR = 512,
	CHANGE_DYNAMICS_INFO_SET_LOCAL_INERTIA_DIAGONAL = 1024,
	CHANGE_DYNAMICS_INFO_SET_CCD_SWEPT_SPHERE_RADIUS = 2048,
	CHANGE_DYNAMICS_INFO_SET_CONTACT_PROCESSING_THRESHOLD = 4096,
	CHANGE_DYNAMICS_INFO_SET_ACTIVATION_STATE = 8192,
	CHANGE_DYNAMICS_INFO_SET_JOINT_DAMPING = 16384,
	CHANGE_DYNAMICS_INFO_SET_ANISOTROPIC_FRICTION = 32768,
	CHANGE_DYNAMICS_INFO_SET_MAX_JOINT_VELOCITY = 1<<16,	
	CHANGE_DYNAMICS_INFO_SET_COLLISION_MARGIN = 1 << 17,
	CHANGE_DYNAMICS_INFO_SET_JOINT_LIMITS = 1 << 18,
	CHANGE_DYNAMICS_INFO_SET_JOINT_LIMIT_MAX_FORCE = 1 << 19,
	CHANGE_DYNAMICS_INFO_SET_DYNAMIC_TYPE = 1 << 20,
	CHANGE_DYNAMICS_INFO_SET_SLEEP_THRESHOLD = 1 << 21,
};

struct ChangeDynamicsInfoArgs
{
	i32 m_bodyUniqueId;
	i32 m_linkIndex;
	double m_mass;
	double m_COM[3];
	double m_lateralFriction;
	double m_spinningFriction;
	double m_rollingFriction;
	double m_restitution;
	double m_linearDamping;
	double m_angularDamping;
	double m_contactStiffness;
	double m_contactDamping;
	double m_localInertiaDiagonal[3];
	i32 m_frictionAnchor;
	double m_ccdSweptSphereRadius;
	double m_contactProcessingThreshold;
	i32 m_activationState;
	double m_jointDamping;
	double m_anisotropicFriction[3];
	double m_maxJointVelocity;
	double m_collisionMargin;
	
	double m_jointLowerLimit;
	double m_jointUpperLimit;
	double m_jointLimitForce;
	
	i32 m_dynamicType;

	double m_sleepThreshold;

};

struct GetDynamicsInfoArgs
{
	i32 m_bodyUniqueId;
	i32 m_linkIndex;
};

struct SetJointFeedbackArgs
{
	i32 m_bodyUniqueId;
	i32 m_linkId;
	i32 m_isEnabled;
};

enum EnumInitPoseFlags
{
	INIT_POSE_HAS_INITIAL_POSITION = 1,
	INIT_POSE_HAS_INITIAL_ORIENTATION = 2,
	INIT_POSE_HAS_JOINT_STATE = 4,
	INIT_POSE_HAS_BASE_LINEAR_VELOCITY = 8,
	INIT_POSE_HAS_BASE_ANGULAR_VELOCITY = 16,
	INIT_POSE_HAS_JOINT_VELOCITY = 32,
	INIT_POSE_HAS_SCALING=64,
};

///InitPoseArgs is mainly to initialize (teleport) the robot in a particular position
///No motors or controls are needed to initialize the pose. It is similar to
///moving a robot to a starting place, while it is switched off. It is only called
///at the start of a robot control session. All velocities and control forces are cleared to zero.
struct InitPoseArgs
{
	i32 m_bodyUniqueId;
	i32 m_hasInitialStateQ[MAX_DEGREE_OF_FREEDOM];
	double m_initialStateQ[MAX_DEGREE_OF_FREEDOM];
	i32 m_hasInitialStateQdot[MAX_DEGREE_OF_FREEDOM];
	double m_initialStateQdot[MAX_DEGREE_OF_FREEDOM];
	double m_scaling[3];
};

struct RequestDebugLinesArgs
{
	i32 m_debugMode;
	i32 m_startingLineIndex;
};

struct RequestPixelDataArgs
{
	float m_viewMatrix[16];
	float m_projectionMatrix[16];
	i32 m_startPixelIndex;
	i32 m_pixelWidth;
	i32 m_pixelHeight;
	float m_lightDirection[3];
	float m_lightColor[3];
	float m_lightDistance;
	float m_lightAmbientCoeff;
	float m_lightDiffuseCoeff;
	float m_lightSpecularCoeff;
	i32 m_hasShadow;
	i32 m_flags;
	float m_projectiveTextureViewMatrix[16];
	float m_projectiveTextureProjectionMatrix[16];
};

enum EnumRequestPixelDataUpdateFlags
{
	REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES = 1,
	REQUEST_PIXEL_ARGS_SET_PIXEL_WIDTH_HEIGHT = 2,
	REQUEST_PIXEL_ARGS_SET_LIGHT_DIRECTION = 4,
	REQUEST_PIXEL_ARGS_SET_LIGHT_COLOR = 8,
	REQUEST_PIXEL_ARGS_SET_LIGHT_DISTANCE = 16,
	REQUEST_PIXEL_ARGS_SET_SHADOW = 32,
	REQUEST_PIXEL_ARGS_SET_AMBIENT_COEFF = 64,
	REQUEST_PIXEL_ARGS_SET_DIFFUSE_COEFF = 128,
	REQUEST_PIXEL_ARGS_SET_SPECULAR_COEFF = 256,
	REQUEST_PIXEL_ARGS_HAS_FLAGS = 512,
	REQUEST_PIXEL_ARGS_HAS_PROJECTIVE_TEXTURE_MATRICES = 1024,

	//don't exceed (1<<15), because this enum is shared with EnumRenderer in SharedMemoryPublic.h

};

enum EnumRequestContactDataUpdateFlags
{
	CMD_REQUEST_CONTACT_POINT_HAS_QUERY_MODE = 1,
	CMD_REQUEST_CONTACT_POINT_HAS_CLOSEST_DISTANCE_THRESHOLD = 2,
	CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_A_FILTER = 4,
	CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_B_FILTER = 8,

	CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_A = 16,
	CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_B = 32,
	CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_POSITION_A = 64,
	CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_POSITION_B = 128,
	CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_ORIENTATION_A = 256,
	CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_ORIENTATION_B = 512,

};

struct RequestRaycastIntersections
{
	// The number of threads that drx3D may use to perform the ray casts.
	// 0: Let drx3D decide
	// 1: Use a single thread (i.e. no multi-threading)
	// 2 or more: Number of threads to use.
	i32 m_numThreads;
	i32 m_numCommandRays;
	//m_numCommandRays command rays are stored in m_fromToRays
	b3RayData m_fromToRays[MAX_RAY_INTERSECTION_BATCH_SIZE];

	i32 m_numStreamingRays;
	//optional m_parentObjectUniqueId (-1 for unused)
	i32 m_parentObjectUniqueId;
	i32 m_parentLinkIndex;
	i32 m_reportHitNumber;
	i32 m_collisionFilterMask;
	double m_fractionEpsilon;
	//streaming ray data stored in shared memory streaming part. (size m_numStreamingRays )
};

struct SendRaycastHits
{
	i32 m_numRaycastHits;
	// Actual ray result data stored in shared memory streaming part.
};

struct RequestContactDataArgs
{
	i32 m_startingContactPointIndex;
	i32 m_objectAIndexFilter;
	i32 m_objectBIndexFilter;
	i32 m_linkIndexAIndexFilter;
	i32 m_linkIndexBIndexFilter;
	double m_closestDistanceThreshold;

	i32 m_collisionShapeA;
	i32 m_collisionShapeB;
	double m_collisionShapePositionA[3];
	double m_collisionShapePositionB[3];
	double m_collisionShapeOrientationA[4];
	double m_collisionShapeOrientationB[4];

	i32 m_mode;
};

struct RequestOverlappingObjectsArgs
{
	i32 m_startingOverlappingObjectIndex;
	double m_aabbQueryMin[3];
	double m_aabbQueryMax[3];
};

struct RequestVisualShapeDataArgs
{
	i32 m_bodyUniqueId;
	i32 m_startingVisualShapeIndex;
};

struct RequestCollisionShapeDataArgs
{
	i32 m_bodyUniqueId;
	i32 m_linkIndex;
};

enum EnumUpdateVisualShapeData
{
	CMD_UPDATE_VISUAL_SHAPE_TEXTURE = 1,
	CMD_UPDATE_VISUAL_SHAPE_RGBA_COLOR = 2,
	CMD_UPDATE_VISUAL_SHAPE_SPECULAR_COLOR = 4,
	CMD_UPDATE_VISUAL_SHAPE_FLAGS = 8,
};



struct UpdateVisualShapeDataArgs
{
	i32 m_bodyUniqueId;
	i32 m_jointIndex;
	i32 m_shapeIndex;
	i32 m_textureUniqueId;
	double m_rgbaColor[4];
	double m_specularColor[3];
	i32 m_flags;
};

struct LoadTextureArgs
{
	char m_textureFileName[MAX_FILENAME_LENGTH];
};

struct b3LoadTextureResultArgs
{
	i32 m_textureUniqueId;
};

struct SendVisualShapeDataArgs
{
	i32 m_bodyUniqueId;
	i32 m_startingVisualShapeIndex;
	i32 m_numVisualShapesCopied;
	i32 m_numRemainingVisualShapes;
};

struct SendCollisionShapeDataArgs
{
	i32 m_bodyUniqueId;
	i32 m_linkIndex;
	i32 m_numCollisionShapes;
};

struct SendDebugLinesArgs
{
	i32 m_startingLineIndex;
	i32 m_numDebugLines;
	i32 m_numRemainingDebugLines;
};

struct SendPixelDataArgs
{
	i32 m_imageWidth;
	i32 m_imageHeight;

	i32 m_startingPixelIndex;
	i32 m_numPixelsCopied;
	i32 m_numRemainingPixels;
};

struct PickBodyArgs
{
	double m_rayFromWorld[3];
	double m_rayToWorld[3];
};

///Controlling a robot involves sending the desired state to its joint motor controllers.
///The control mode determines the state variables used for motor control.
struct SendDesiredStateArgs
{
	i32 m_bodyUniqueId;
	i32 m_controlMode;

	//PD parameters in case m_controlMode == CONTROL_MODE_POSITION_VELOCITY_PD
	double m_Kp[MAX_DEGREE_OF_FREEDOM];  //indexed by degree of freedom, 6 for base, and then the dofs for each link
	double m_Kd[MAX_DEGREE_OF_FREEDOM];  //indexed by degree of freedom, 6 for base, and then the dofs for each link
	double m_rhsClamp[MAX_DEGREE_OF_FREEDOM];

	i32 m_hasDesiredStateFlags[MAX_DEGREE_OF_FREEDOM];

	//desired state is only written by the client, read-only access by server is expected

	//m_desiredStateQ is indexed by position variables,
	//starting with 3 base position variables, 4 base orientation variables (quaternion), then link position variables
	double m_desiredStateQ[MAX_DEGREE_OF_FREEDOM];

	//m_desiredStateQdot is index by velocity degrees of freedom, 3 linear and 3 angular variables for the base and then link velocity variables
	double m_desiredStateQdot[MAX_DEGREE_OF_FREEDOM];

	//m_desiredStateForceTorque is either the actual applied force/torque (in CONTROL_MODE_TORQUE) or
	//or the maximum applied force/torque for the PD/motor/constraint to reach the desired velocity in CONTROL_MODE_VELOCITY and CONTROL_MODE_POSITION_VELOCITY_PD mode
	//indexed by degree of freedom, 6 dof base, and then dofs for each link
	double m_desiredStateForceTorque[MAX_DEGREE_OF_FREEDOM];

	double m_damping[MAX_DEGREE_OF_FREEDOM];
};

enum EnumSimDesiredStateUpdateFlags
{
	SIM_DESIRED_STATE_HAS_Q = 1,
	SIM_DESIRED_STATE_HAS_QDOT = 2,
	SIM_DESIRED_STATE_HAS_KD = 4,
	SIM_DESIRED_STATE_HAS_KP = 8,
	SIM_DESIRED_STATE_HAS_MAX_FORCE = 16,
	SIM_DESIRED_STATE_HAS_RHS_CLAMP = 32,
	SIM_DESIRED_STATE_HAS_DAMPING = 64,
};

enum EnumSimParamUpdateFlags
{
	SIM_PARAM_UPDATE_DELTA_TIME = 1,
	SIM_PARAM_UPDATE_GRAVITY = 1<<1,
	SIM_PARAM_UPDATE_NUM_SOLVER_ITERATIONS = 1<<2,
	SIM_PARAM_UPDATE_NUM_SIMULATION_SUB_STEPS = 1<<3,
	SIM_PARAM_UPDATE_REAL_TIME_SIMULATION = 1<<4,
	SIM_PARAM_UPDATE_DEFAULT_CONTACT_ERP = 1<<5,
	SIM_PARAM_UPDATE_INTERNAL_SIMULATION_FLAGS = 1<<6,
	SIM_PARAM_UPDATE_USE_SPLIT_IMPULSE = 1<<7,
	SIM_PARAM_UPDATE_SPLIT_IMPULSE_PENETRATION_THRESHOLD = 1<<8,
	SIM_PARAM_UPDATE_COLLISION_FILTER_MODE = 1 << 9,
	SIM_PARAM_UPDATE_CONTACT_BREAKING_THRESHOLD = 1 << 10,
	SIM_PARAM_ENABLE_CONE_FRICTION = 1 << 11,
	SIM_PARAM_ENABLE_FILE_CACHING = 1 << 12,
	SIM_PARAM_UPDATE_RESTITUTION_VELOCITY_THRESHOLD = 1 << 13,
	SIM_PARAM_UPDATE_DEFAULT_NON_CONTACT_ERP = 1 << 14,
	SIM_PARAM_UPDATE_DEFAULT_FRICTION_ERP = 1 << 15,
	SIM_PARAM_UPDATE_DETERMINISTIC_OVERLAPPING_PAIRS = 1 << 16,
	SIM_PARAM_UPDATE_CCD_ALLOWED_PENETRATION = 1 << 17,
	SIM_PARAM_UPDATE_JOINT_FEEDBACK_MODE = 1 << 18,
	SIM_PARAM_UPDATE_DEFAULT_GLOBAL_CFM = 1 << 19,
	SIM_PARAM_UPDATE_DEFAULT_FRICTION_CFM = 1 << 20,
	SIM_PARAM_UPDATE_SOLVER_RESIDULAL_THRESHOLD = 1 << 21,
	SIM_PARAM_UPDATE_CONTACT_SLOP = 1 << 22,
	SIM_PARAM_ENABLE_SAT = 1 << 23,
	SIM_PARAM_CONSTRAINT_SOLVER_TYPE = 1 << 24,
	SIM_PARAM_CONSTRAINT_MIN_SOLVER_ISLAND_SIZE = 1 << 25,
	SIM_PARAM_REPORT_CONSTRAINT_SOLVER_ANALYTICS = 1 << 26,
	SIM_PARAM_UPDATE_WARM_STARTING_FACTOR = 1 << 27,
	SIM_PARAM_UPDATE_ARTICULATED_WARM_STARTING_FACTOR = 1 << 28,
	SIM_PARAM_UPDATE_SPARSE_SDF = 1 << 29,
	SIM_PARAM_UPDATE_NUM_NONCONTACT_INNER_ITERATIONS = 1 << 30,
};

enum EnumLoadSoftBodyUpdateFlags
{
	LOAD_SOFT_BODY_FILE_NAME = 1,
	LOAD_SOFT_BODY_UPDATE_SCALE = 1<<1,
	LOAD_SOFT_BODY_UPDATE_MASS = 1<<2,
	LOAD_SOFT_BODY_UPDATE_COLLISION_MARGIN = 1<<3,
	LOAD_SOFT_BODY_INITIAL_POSITION = 1<<4,
	LOAD_SOFT_BODY_INITIAL_ORIENTATION = 1<<5,
	LOAD_SOFT_BODY_ADD_COROTATED_FORCE = 1<<6,
	LOAD_SOFT_BODY_ADD_MASS_SPRING_FORCE = 1<<7,
	LOAD_SOFT_BODY_ADD_GRAVITY_FORCE = 1<<8,
	LOAD_SOFT_BODY_SET_COLLISION_HARDNESS = 1<<9,
	LOAD_SOFT_BODY_SET_FRICTION_COEFFICIENT = 1<<10,
	LOAD_SOFT_BODY_ADD_BENDING_SPRINGS = 1<<11,
	LOAD_SOFT_BODY_ADD_NEOHOOKEAN_FORCE = 1<<12,
	LOAD_SOFT_BODY_USE_SELF_COLLISION = 1<<13,
	LOAD_SOFT_BODY_USE_FACE_CONTACT = 1<<14,
	LOAD_SOFT_BODY_SIM_MESH = 1<<15,
	LOAD_SOFT_BODY_SET_REPULSION_STIFFNESS = 1<<16,
	LOAD_SOFT_BODY_SET_DAMPING_SPRING_MODE = 1<<17,
	LOAD_SOFT_BODY_SET_GRAVITY_FACTOR = 1<<18,
};

enum EnumSimParamInternalSimFlags
{
	SIM_PARAM_INTERNAL_CREATE_ROBOT_ASSETS = 1,
};

///Controlling a robot involves sending the desired state to its joint motor controllers.
///The control mode determines the state variables used for motor control.

struct LoadSoftBodyArgs
{
	char m_fileName[MAX_FILENAME_LENGTH];
	double m_scale;
	double m_mass;
	double m_collisionMargin;
	double m_initialPosition[3];
    double m_initialOrientation[4];
    double m_springElasticStiffness;
    double m_springDampingStiffness;
	i32 m_dampAllDirections;
    double m_springBendingStiffness;
    double m_corotatedMu;
    double m_corotatedLambda;
    i32 m_useBendingSprings;
    double m_collisionHardness;
    i32 m_useSelfCollision;
    double m_frictionCoeff;
    double m_NeoHookeanMu;
    double m_NeoHookeanLambda;
    double m_NeoHookeanDamping;
    i32 m_useFaceContact;
    char m_simFileName[MAX_FILENAME_LENGTH];
	double m_repulsionStiffness;
	double m_gravFactor;
};

struct b3LoadSoftBodyResultArgs
{
	i32 m_objectUniqueId;
};

struct RequestActualStateArgs
{
	i32 m_bodyUniqueId;
};

struct SendActualStateArgs
{
	i32 m_bodyUniqueId;
	i32 m_numLinks;
	i32 m_numDegreeOfFreedomQ;
	i32 m_numDegreeOfFreedomU;

	double m_rootLocalInertialFrame[7];
	struct SendActualStateSharedMemoryStorage* m_stateDetails;

};

struct SendActualStateSharedMemoryStorage
{
	//actual state is only written by the server, read-only access by client is expected
	double m_actualStateQ[MAX_DEGREE_OF_FREEDOM];
	double m_actualStateQdot[MAX_DEGREE_OF_FREEDOM];

	//measured 6DOF force/torque sensors: force[x,y,z] and torque[x,y,z]
	double m_jointReactionForces[6 * MAX_DEGREE_OF_FREEDOM];

	double m_jointMotorForce[MAX_DEGREE_OF_FREEDOM];
	double m_jointMotorForceMultiDof[MAX_DEGREE_OF_FREEDOM];

	double m_linkState[7 * MAX_NUM_LINKS];
	double m_linkWorldVelocities[6 * MAX_NUM_LINKS];  //linear velocity and angular velocity in world space (x/y/z each).
	double m_linkLocalInertialFrames[7 * MAX_NUM_LINKS];
};

struct b3SendCollisionInfoArgs
{
	i32 m_numLinks;
	double m_rootWorldAABBMin[3];
	double m_rootWorldAABBMax[3];

	double m_linkWorldAABBsMin[3 * MAX_NUM_LINKS];
	double m_linkWorldAABBsMax[3 * MAX_NUM_LINKS];
};

struct b3RequestCollisionInfoArgs
{
	i32 m_bodyUniqueId;
};

enum EnumSensorTypes
{
	SENSOR_FORCE_TORQUE = 1,
	SENSOR_IMU = 2,
};

struct CreateSensorArgs
{
	i32 m_bodyUniqueId;
	i32 m_numJointSensorChanges;
	i32 m_sensorType[MAX_DEGREE_OF_FREEDOM];

	///todo: clean up the duplication, make sure no-one else is using those members directly (use C-API header instead)
	i32 m_jointIndex[MAX_DEGREE_OF_FREEDOM];
	i32 m_enableJointForceSensor[MAX_DEGREE_OF_FREEDOM];

	i32 m_linkIndex[MAX_DEGREE_OF_FREEDOM];
	i32 m_enableSensor[MAX_DEGREE_OF_FREEDOM];
};

typedef struct SharedMemoryCommand SharedMemoryCommand_t;

enum EnumBoxShapeFlags
{
	BOX_SHAPE_HAS_INITIAL_POSITION = 1,
	BOX_SHAPE_HAS_INITIAL_ORIENTATION = 2,
	BOX_SHAPE_HAS_HALF_EXTENTS = 4,
	BOX_SHAPE_HAS_MASS = 8,
	BOX_SHAPE_HAS_COLLISION_SHAPE_TYPE = 16,
	BOX_SHAPE_HAS_COLOR = 32,
};
///This command will be replaced to allow arbitrary collision shape types
struct CreateBoxShapeArgs
{
	double m_halfExtentsX;
	double m_halfExtentsY;
	double m_halfExtentsZ;

	double m_mass;
	i32 m_collisionShapeType;  //see SharedMemoryPublic.h

	double m_initialPosition[3];
	double m_initialOrientation[4];
	double m_colorRGBA[4];
};

struct b3ObjectArgs
{
	i32 m_numBodies;
	i32 m_bodyUniqueIds[MAX_SDF_BODIES];
	i32 m_numUserConstraints;
	i32 m_userConstraintUniqueIds[MAX_SDF_BODIES];
	i32 m_numUserCollisionShapes;
	i32 m_userCollisionShapes[MAX_SDF_BODIES];
};

struct b3Profile
{
	char m_name[MAX_FILENAME_LENGTH];
	i32 m_durationInMicroSeconds;
	i32 m_type;
};

struct SdfLoadedArgs
{
	i32 m_numBodies;
	i32 m_bodyUniqueIds[MAX_SDF_BODIES];
	i32 m_numUserConstraints;
	i32 m_userConstraintUniqueIds[MAX_SDF_BODIES];

	///@todo(erwincoumans) load cameras, lights etc
	//i32 m_numCameras;
	//i32 m_numLights;
};

struct SdfRequestInfoArgs
{
	i32 m_bodyUniqueId;
};

///flags for b3ApplyExternalTorque and b3ApplyExternalForce
enum EnumExternalForcePrivateFlags
{
	//    EF_LINK_FRAME=1,
	//    EF_WORLD_FRAME=2,
	EF_TORQUE = 4,
	EF_FORCE = 8,
};

struct ExternalForceArgs
{
	i32 m_numForcesAndTorques;
	i32 m_bodyUniqueIds[MAX_SDF_BODIES];
	i32 m_linkIds[MAX_SDF_BODIES];
	double m_forcesAndTorques[3 * MAX_SDF_BODIES];
	double m_positions[3 * MAX_SDF_BODIES];
	i32 m_forceFlags[MAX_SDF_BODIES];
};

enum EnumSdfRequestInfoFlags
{
	SDF_REQUEST_INFO_BODY = 1,
	//SDF_REQUEST_INFO_CAMERA=2,
};

struct Calculatedrx3d_inverseArgs
{
	i32 m_bodyUniqueId;
	i32 m_dofCountQ;
	i32 m_dofCountQdot;
	double m_jointPositionsQ[MAX_DEGREE_OF_FREEDOM];
	double m_jointVelocitiesQdot[MAX_DEGREE_OF_FREEDOM];
	double m_jointAccelerations[MAX_DEGREE_OF_FREEDOM];
	i32 m_flags;
};

struct Calculatedrx3d_inverseResultArgs
{
	i32 m_bodyUniqueId;
	i32 m_dofCount;
	double m_jointForces[MAX_DEGREE_OF_FREEDOM];
};

struct CalculateJacobianArgs
{
	i32 m_bodyUniqueId;
	i32 m_linkIndex;
	double m_localPosition[3];
	double m_jointPositionsQ[MAX_DEGREE_OF_FREEDOM];
	double m_jointVelocitiesQdot[MAX_DEGREE_OF_FREEDOM];
	double m_jointAccelerations[MAX_DEGREE_OF_FREEDOM];
};

struct CalculateJacobianResultArgs
{
	i32 m_dofCount;
	double m_linearJacobian[3 * MAX_DEGREE_OF_FREEDOM];
	double m_angularJacobian[3 * MAX_DEGREE_OF_FREEDOM];
};

struct CalculateMassMatrixArgs
{
	i32 m_bodyUniqueId;
	double m_jointPositionsQ[MAX_DEGREE_OF_FREEDOM];
	i32 m_dofCountQ;
	i32 m_flags;
};

struct CalculateMassMatrixResultArgs
{
	i32 m_dofCount;
};

enum b3EnumCollisionFilterFlags
{
	D3_COLLISION_FILTER_PAIR = 1,
	D3_COLLISION_FILTER_GROUP_MASK = 2,
};

struct b3CollisionFilterArgs
{
	i32 m_bodyUniqueIdA;
	i32 m_bodyUniqueIdB;
	i32 m_linkIndexA;
	i32 m_linkIndexB;
	i32 m_enableCollision;
	i32 m_collisionFilterGroup;
	i32 m_collisionFilterMask;
};

struct CalculateInverseKinematicsArgs
{
	i32 m_bodyUniqueId;
	//	double m_jointPositionsQ[MAX_DEGREE_OF_FREEDOM];
	double m_targetPositions[MAX_DEGREE_OF_FREEDOM*3];
	i32 m_numEndEffectorLinkIndices;
	double m_targetOrientation[MAX_DEGREE_OF_FREEDOM*4];  //orientation represented as quaternion, x,y,z,w
	i32 m_endEffectorLinkIndices[MAX_DEGREE_OF_FREEDOM];
	double m_lowerLimit[MAX_DEGREE_OF_FREEDOM];
	double m_upperLimit[MAX_DEGREE_OF_FREEDOM];
	double m_jointRange[MAX_DEGREE_OF_FREEDOM];
	double m_restPose[MAX_DEGREE_OF_FREEDOM];
	double m_jointDamping[MAX_DEGREE_OF_FREEDOM];
	double m_currentPositions[MAX_DEGREE_OF_FREEDOM];
	i32 m_maxNumIterations;
	double m_residualThreshold;
};

struct CalculateInverseKinematicsResultArgs
{
	i32 m_bodyUniqueId;
	i32 m_dofCount;
	double m_jointPositions[MAX_DEGREE_OF_FREEDOM];
};


enum EnumBodyChangeFlags
{
	BODY_DELETE_FLAG = 1,
};

enum EnumUserDebugDrawFlags
{
	USER_DEBUG_HAS_LINE = 1,
	USER_DEBUG_HAS_TEXT = 2,
	USER_DEBUG_REMOVE_ONE_ITEM = 4,
	USER_DEBUG_REMOVE_ALL = 8,
	USER_DEBUG_SET_CUSTOM_OBJECT_COLOR = 16,
	USER_DEBUG_REMOVE_CUSTOM_OBJECT_COLOR = 32,
	USER_DEBUG_ADD_PARAMETER = 64,
	USER_DEBUG_READ_PARAMETER = 128,
	USER_DEBUG_HAS_OPTION_FLAGS = 256,
	USER_DEBUG_HAS_TEXT_ORIENTATION = 512,
	USER_DEBUG_HAS_PARENT_OBJECT = 1024,
	USER_DEBUG_HAS_REPLACE_ITEM_UNIQUE_ID = 2048,
	USER_DEBUG_REMOVE_ALL_PARAMETERS = 4096,
	USER_DEBUG_HAS_POINTS = 8192,
};

struct UserDebugDrawArgs
{
	double m_debugLineFromXYZ[3];
	double m_debugLineToXYZ[3];
	double m_debugLineColorRGB[3];
	double m_lineWidth;

	double m_lifeTime;
	i32 m_itemUniqueId;

	char m_text[MAX_FILENAME_LENGTH];
	double m_textPositionXYZ[3];
	double m_textOrientation[4];
	i32 m_parentObjectUniqueId;
	i32 m_parentLinkIndex;
	double m_textColorRGB[3];
	double m_textSize;
	i32 m_optionFlags;
	i32 m_replaceItemUniqueId;

	double m_rangeMin;
	double m_rangeMax;
	double m_startValue;

	double m_objectDebugColorRGB[3];
	i32 m_objectUniqueId;
	i32 m_linkIndex;

	i32 m_debugPointNum;
	double m_pointSize;
};

struct UserDebugDrawResultArgs
{
	i32 m_debugItemUniqueId;
	double m_parameterValue;
};

struct SendVREvents
{
	i32 m_numVRControllerEvents;
	b3VRControllerEvent m_controllerEvents[MAX_VR_CONTROLLERS];
};

struct SendKeyboardEvents
{
	i32 m_numKeyboardEvents;
	b3KeyboardEvent m_keyboardEvents[MAX_KEYBOARD_EVENTS];
};

struct SendMouseEvents
{
	i32 m_numMouseEvents;
	b3MouseEvent m_mouseEvents[MAX_MOUSE_EVENTS];
};

enum eVRCameraEnums
{
	VR_CAMERA_ROOT_POSITION = 1,
	VR_CAMERA_ROOT_ORIENTATION = 2,
	VR_CAMERA_ROOT_TRACKING_OBJECT = 4,
	VR_CAMERA_FLAG = 8,
};

enum eStateLoggingEnums
{
	STATE_LOGGING_START_LOG = 1,
	STATE_LOGGING_STOP_LOG = 2,
	STATE_LOGGING_FILTER_OBJECT_UNIQUE_ID = 4,
	STATE_LOGGING_MAX_LOG_DOF = 8,
	STATE_LOGGING_FILTER_LINK_INDEX_A = 16,
	STATE_LOGGING_FILTER_LINK_INDEX_B = 32,
	STATE_LOGGING_FILTER_BODY_UNIQUE_ID_A = 64,
	STATE_LOGGING_FILTER_BODY_UNIQUE_ID_B = 128,
	STATE_LOGGING_FILTER_DEVICE_TYPE = 256,
	STATE_LOGGING_LOG_FLAGS = 512
};

struct VRCameraState
{
	double m_rootPosition[3];
	double m_rootOrientation[4];
	i32 m_trackingObjectUniqueId;
	i32 m_trackingObjectFlag;
};

struct StateLoggingRequest
{
	char m_fileName[MAX_FILENAME_LENGTH];
	i32 m_logType;           //Minitaur, generic robot, VR states, contact points
	i32 m_numBodyUniqueIds;  ////only if STATE_LOGGING_FILTER_OBJECT_UNIQUE_ID flag is set
	i32 m_bodyUniqueIds[MAX_SDF_BODIES];
	i32 m_loggingUniqueId;
	i32 m_maxLogDof;
	i32 m_linkIndexA;        // only if STATE_LOGGING_FILTER_LINK_INDEX_A flag is set
	i32 m_linkIndexB;        // only if STATE_LOGGING_FILTER_LINK_INDEX_B flag is set
	i32 m_bodyUniqueIdA;     // only if STATE_LOGGING_FILTER_BODY_UNIQUE_ID_A flag is set
	i32 m_bodyUniqueIdB;     // only if STATE_LOGGING_FILTER_BODY_UNIQUE_ID_B flag is set
	i32 m_deviceFilterType;  //user to select (filter) which VR devices to log
	i32 m_logFlags;
};

struct StateLoggingResultArgs
{
	i32 m_loggingUniqueId;
};

enum InternalOpenGLVisualizerUpdateFlags
{
	COV_SET_CAMERA_VIEW_MATRIX = 1,
	COV_SET_FLAGS = 2,
	COV_SET_LIGHT_POSITION = 4,
	COV_SET_SHADOWMAP_RESOLUTION = 8,
	COV_SET_SHADOWMAP_WORLD_SIZE = 16,
	COV_SET_REMOTE_SYNC_TRANSFORM_INTERVAL = 32,
	COV_SET_SHADOWMAP_INTENSITY = 64,
	COV_SET_RGB_BACKGROUND = 128,
};

struct ConfigureOpenGLVisualizerRequest
{
	double m_cameraDistance;
	double m_cameraPitch;
	double m_cameraYaw;
	double m_cameraTargetPosition[3];
	double m_lightPosition[3];
	i32 m_shadowMapResolution;
	i32 m_shadowMapWorldSize;
	double m_remoteSyncTransformInterval;
	i32 m_setFlag;
	i32 m_setEnabled;
	double m_shadowMapIntensity;
	double m_rgbBackground[3];
};

enum
{
	URDF_GEOM_HAS_RADIUS = 1,
};

struct b3CreateUserShapeData
{
	i32 m_type;  //see UrdfGeomTypes

	i32 m_hasChildTransform;
	double m_childPosition[3];
	double m_childOrientation[4];

	double m_sphereRadius;
	double m_boxHalfExtents[3];
	double m_capsuleRadius;
	double m_capsuleHeight;
	i32 m_hasFromTo;
	double m_capsuleFrom[3];
	double m_capsuleTo[3];
	double m_planeNormal[3];
	double m_planeConstant;

	i32 m_meshFileType;
	char m_meshFileName[VISUAL_SHAPE_MAX_PATH_LEN];
	double m_meshScale[3];
	i32 m_collisionFlags;
	i32 m_visualFlags;
	i32 m_numVertices;
	i32 m_numIndices;
	i32 m_numUVs;
	i32 m_numNormals;
	double m_heightfieldTextureScaling;
	i32 m_numHeightfieldRows;
	i32 m_numHeightfieldColumns;
	double m_rgbaColor[4];
	double m_specularColor[3];
	i32 m_replaceHeightfieldIndex;
};

#define MAX_COMPOUND_COLLISION_SHAPES 16

struct b3CreateUserShapeArgs
{
	i32 m_numUserShapes;
	b3CreateUserShapeData m_shapes[MAX_COMPOUND_COLLISION_SHAPES];
};



struct b3CreateUserShapeResultArgs
{
	i32 m_userShapeUniqueId;
};

#define MAX_CREATE_MULTI_BODY_LINKS MAX_DEGREE_OF_FREEDOM
enum eCreateMultiBodyEnum
{
	MULTI_BODY_HAS_BASE = 1,
	MULT_BODY_USE_MAXIMAL_COORDINATES = 2,
	MULT_BODY_HAS_FLAGS = 4,
};
struct b3CreateMultiBodyArgs
{
	char m_bodyName[1024];
	i32 m_baseLinkIndex;

	double m_linkPositions[3 * MAX_CREATE_MULTI_BODY_LINKS];
	double m_linkOrientations[4 * MAX_CREATE_MULTI_BODY_LINKS];

	i32 m_numLinks;
	double m_linkMasses[MAX_CREATE_MULTI_BODY_LINKS];
	double m_linkInertias[MAX_CREATE_MULTI_BODY_LINKS * 3];

	double m_linkInertialFramePositions[MAX_CREATE_MULTI_BODY_LINKS * 3];
	double m_linkInertialFrameOrientations[MAX_CREATE_MULTI_BODY_LINKS * 4];

	i32 m_linkCollisionShapeUniqueIds[MAX_CREATE_MULTI_BODY_LINKS];
	i32 m_linkVisualShapeUniqueIds[MAX_CREATE_MULTI_BODY_LINKS];
	i32 m_linkParentIndices[MAX_CREATE_MULTI_BODY_LINKS];
	i32 m_linkJointTypes[MAX_CREATE_MULTI_BODY_LINKS];
	double m_linkJointAxis[3 * MAX_CREATE_MULTI_BODY_LINKS];
	i32 m_flags;
	i32 m_numBatchObjects;

};

struct b3CreateMultiBodyResultArgs
{
	i32 m_bodyUniqueId;
};

struct b3ChangeTextureArgs
{
	i32 m_textureUniqueId;
	i32 m_width;
	i32 m_height;
};

struct b3StateSerializationArguments
{
	char m_fileName[MAX_URDF_FILENAME_LENGTH];
	i32 m_stateId;
};

struct SyncUserDataRequestArgs
{
	// The number of bodies for which we'd like to sync the user data of. When 0, all bodies are synced.
	i32 m_numRequestedBodies;
	// The body IDs for which we'd like to sync the user data of.
	i32 m_requestedBodyIds[MAX_REQUESTED_BODIES_LENGTH];
};

struct SyncUserDataArgs
{
	// User data identifiers stored in m_bulletStreamDataServerToClientRefactor
	// as as array of integers.
	i32 m_numUserDataIdentifiers;
	// Whether the client should clear its user data cache.
	bool m_clearCachedUserDataEntries;
};

struct UserDataRequestArgs
{
	i32 m_userDataId;
};

struct UserDataResponseArgs
{
	i32 m_userDataId;
	i32 m_bodyUniqueId;
	i32 m_linkIndex;
	i32 m_visualShapeIndex;
	i32 m_valueType;
	i32 m_valueLength;
	char m_key[MAX_USER_DATA_KEY_LENGTH];
	// Value data stored in m_bulletStreamDataServerToClientRefactor.
};

struct AddUserDataRequestArgs
{
	i32 m_bodyUniqueId;
	i32 m_linkIndex;
	i32 m_visualShapeIndex;
	i32 m_valueType;
	i32 m_valueLength;
	char m_key[MAX_USER_DATA_KEY_LENGTH];
	// Value data stored in m_bulletStreamDataServerToClientRefactor.
};

struct b3RequestMeshDataArgs
{
	i32 m_bodyUniqueId;
	i32 m_linkIndex;
	i32 m_startingVertex;
	i32 m_collisionShapeIndex;
	i32 m_flags;
};

struct b3ResetMeshDataArgs
{
	i32 m_bodyUniqueId;
	i32 m_numVertices;
	i32 m_flags;
};

struct b3SendMeshDataArgs
{
	i32 m_numVerticesCopied;
	i32 m_startingVertex;
	i32 m_numVerticesRemaining;
};

struct SharedMemoryCommand
{
	i32 m_type;
	smUint64_t m_timeStamp;
	i32 m_sequenceNumber;

	//m_updateFlags is a bit fields to tell which parameters need updating
	//for example m_updateFlags = SIM_PARAM_UPDATE_DELTA_TIME | SIM_PARAM_UPDATE_NUM_SOLVER_ITERATIONS;
	i32 m_updateFlags;

	union {
		struct UrdfArgs m_urdfArguments;
		struct SdfArgs m_sdfArguments;
		struct MjcfArgs m_mjcfArguments;
		struct FileArgs m_fileArguments;
		struct SdfRequestInfoArgs m_sdfRequestInfoArgs;
		struct ChangeDynamicsInfoArgs m_changeDynamicsInfoArgs;
		struct GetDynamicsInfoArgs m_getDynamicsInfoArgs;
		struct InitPoseArgs m_initPoseArgs;
		struct b3PhysicsSimulationParameters m_physSimParamArgs;
		struct BulletDataStreamArgs m_dataStreamArguments;
		struct SendDesiredStateArgs m_sendDesiredStateCommandArgument;
		struct RequestActualStateArgs m_requestActualStateInformationCommandArgument;
		struct CreateSensorArgs m_createSensorArguments;
		struct CreateBoxShapeArgs m_createBoxShapeArguments;
		struct RequestDebugLinesArgs m_requestDebugLinesArguments;
		struct RequestPixelDataArgs m_requestPixelDataArguments;
		struct PickBodyArgs m_pickBodyArguments;
		struct ExternalForceArgs m_externalForceArguments;
		struct Calculatedrx3d_inverseArgs m_calculatedrx3d_inverseArguments;
		struct CalculateJacobianArgs m_calculateJacobianArguments;
		struct CalculateMassMatrixArgs m_calculateMassMatrixArguments;
		struct b3UserConstraint m_userConstraintArguments;
		struct RequestContactDataArgs m_requestContactPointArguments;
		struct RequestOverlappingObjectsArgs m_requestOverlappingObjectsArgs;
		struct RequestVisualShapeDataArgs m_requestVisualShapeDataArguments;
		struct UpdateVisualShapeDataArgs m_updateVisualShapeDataArguments;
		struct LoadTextureArgs m_loadTextureArguments;
		struct CalculateInverseKinematicsArgs m_calculateInverseKinematicsArguments;
		struct UserDebugDrawArgs m_userDebugDrawArgs;
		struct RequestRaycastIntersections m_requestRaycastIntersections;
		struct LoadSoftBodyArgs m_loadSoftBodyArguments;
		struct VRCameraState m_vrCameraStateArguments;
		struct StateLoggingRequest m_stateLoggingArguments;
		struct ConfigureOpenGLVisualizerRequest m_configureOpenGLVisualizerArguments;
		struct b3ObjectArgs m_removeObjectArgs;
		struct b3Profile m_profile;
		struct b3CreateUserShapeArgs m_createUserShapeArgs;
		struct b3CreateMultiBodyArgs m_createMultiBodyArgs;
		struct b3RequestCollisionInfoArgs m_requestCollisionInfoArgs;
		struct b3ChangeTextureArgs m_changeTextureArgs;
		struct b3SearchPathfArgs m_searchPathArgs;
		struct b3CustomCommand m_customCommandArgs;
		struct b3StateSerializationArguments m_loadStateArguments;
		struct RequestCollisionShapeDataArgs m_requestCollisionShapeDataArguments;
		struct SyncUserDataRequestArgs m_syncUserDataRequestArgs;
		struct UserDataRequestArgs m_userDataRequestArgs;
		struct AddUserDataRequestArgs m_addUserDataRequestArgs;
		struct UserDataRequestArgs m_removeUserDataRequestArgs;
		struct b3CollisionFilterArgs m_collisionFilterArgs;
		struct b3RequestMeshDataArgs m_requestMeshDataArgs;
		struct b3ResetMeshDataArgs m_resetMeshDataArgs;

	};
};

struct RigidBodyCreateArgs
{
	i32 m_bodyUniqueId;
};

struct SendContactDataArgs
{
	i32 m_startingContactPointIndex;
	i32 m_numContactPointsCopied;
	i32 m_numRemainingContactPoints;
};

struct SendOverlappingObjectsArgs
{
	i32 m_startingOverlappingObjectIndex;
	i32 m_numOverlappingObjectsCopied;
	i32 m_numRemainingOverlappingObjects;
};

struct SharedMemoryStatus
{
	i32 m_type;

	smUint64_t m_timeStamp;
	i32 m_sequenceNumber;

	//m_streamBytes is only for internal purposes
	i32 m_numDataStreamBytes;
	tuk m_dataStream;

	//m_updateFlags is a bit fields to tell which parameters were updated,
	//m_updateFlags is ignored for most status messages
	i32 m_updateFlags;

	union {
		struct BulletDataStreamArgs m_dataStreamArguments;
		struct SdfLoadedArgs m_sdfLoadedArgs;
		struct SendActualStateArgs m_sendActualStateArgs;
		struct SendDebugLinesArgs m_sendDebugLinesArgs;
		struct SendPixelDataArgs m_sendPixelDataArguments;
		struct RigidBodyCreateArgs m_rigidBodyCreateArgs;
		struct Calculatedrx3d_inverseResultArgs m_inverseDynamicsResultArgs;
		struct CalculateJacobianResultArgs m_jacobianResultArgs;
		struct CalculateMassMatrixResultArgs m_massMatrixResultArgs;
		struct SendContactDataArgs m_sendContactPointArgs;
		struct SendOverlappingObjectsArgs m_sendOverlappingObjectsArgs;
		struct CalculateInverseKinematicsResultArgs m_inverseKinematicsResultArgs;
		struct SendVisualShapeDataArgs m_sendVisualShapeArgs;
		struct UserDebugDrawResultArgs m_userDebugDrawArgs;
		struct b3UserConstraint m_userConstraintResultArgs;
		struct b3UserConstraintState m_userConstraintStateResultArgs;
		struct SendVREvents m_sendVREvents;
		struct SendKeyboardEvents m_sendKeyboardEvents;
		struct SendRaycastHits m_raycastHits;
		struct StateLoggingResultArgs m_stateLoggingResultArgs;
		struct b3OpenGLVisualizerCameraInfo m_visualizerCameraResultArgs;
		struct b3ObjectArgs m_removeObjectArgs;
		struct b3DynamicsInfo m_dynamicsInfo;
		struct b3CreateUserShapeResultArgs m_createUserShapeResultArgs;
		struct b3CreateMultiBodyResultArgs m_createMultiBodyResultArgs;
		struct b3SendCollisionInfoArgs m_sendCollisionInfoArgs;
		struct SendMouseEvents m_sendMouseEvents;
		struct b3LoadTextureResultArgs m_loadTextureResultArguments;
		struct b3CustomCommandResultArgs m_customCommandResultArgs;
		struct b3PhysicsSimulationParameters m_simulationParameterResultArgs;
		struct b3StateSerializationArguments m_saveStateResultArgs;
		struct b3LoadSoftBodyResultArgs m_loadSoftBodyResultArguments;
		struct SendCollisionShapeDataArgs m_sendCollisionShapeArgs;
		struct SyncUserDataArgs m_syncUserDataArgs;
		struct UserDataResponseArgs m_userDataResponseArgs;
		struct UserDataRequestArgs m_removeUserDataResponseArgs;
		struct b3ForwardDynamicsAnalyticsArgs m_forwardDynamicsAnalyticsArgs;
		struct b3SendMeshDataArgs m_sendMeshDataArgs;
	};
};

typedef struct SharedMemoryStatus SharedMemoryStatus_t;

#endif  //SHARED_MEMORY_COMMANDS_H
