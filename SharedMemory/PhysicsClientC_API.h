#ifndef PHYSICS_CLIENT_C_API_H
#define PHYSICS_CLIENT_C_API_H

//#include "SharedMemoryBlock.h"
#include <drx3D/SharedMemory/SharedMemoryPublic.h>

#define D3_DECLARE_HANDLE(name) \
	typedef struct name##__     \
	{                           \
		i32 unused;             \
	} * name

D3_DECLARE_HANDLE(b3PhysicsClientHandle);
D3_DECLARE_HANDLE(b3SharedMemoryCommandHandle);
D3_DECLARE_HANDLE(b3SharedMemoryStatusHandle);

#ifdef _WIN32
#define DRX3D_SHARED_API __declspec(dllexport)
#elif defined(__GNUC__)
#define DRX3D_SHARED_API __attribute__((visibility("default")))
#else
#define DRX3D_SHARED_API
#endif

///There are several connection methods, see following header files:
#include "PhysicsClientSharedMemory_C_API.h"
#include "PhysicsClientSharedMemory2_C_API.h"
#include "PhysicsDirectC_API.h"

//#ifdef DRX3D_ENABLE_ENET
#include "PhysicsClientUDP_C_API.h"
//#endif

//#ifdef DRX3D_ENABLE_CLSOCKET
#include "PhysicsClientTCP_C_API.h"
//#endif

#ifdef __cplusplus
extern "C"
{
#endif

	///b3DisconnectSharedMemory will disconnect the client from the server and cleanup memory.
	DRX3D_SHARED_API void b3DisconnectSharedMemory(b3PhysicsClientHandle physClient);

	///There can only be 1 outstanding command. Check if a command can be send.
	DRX3D_SHARED_API i32 b3CanSubmitCommand(b3PhysicsClientHandle physClient);

	///blocking submit command and wait for status
	DRX3D_SHARED_API b3SharedMemoryStatusHandle b3SubmitClientCommandAndWaitStatus(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle);

	///In general it is better to use b3SubmitClientCommandAndWaitStatus. b3SubmitClientCommand is a non-blocking submit
	///command, which requires checking for the status manually, using b3ProcessServerStatus. Also, before sending the
	///next command, make sure to check if you can send a command using 'b3CanSubmitCommand'.
	DRX3D_SHARED_API i32 b3SubmitClientCommand(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle);

	///non-blocking check status
	DRX3D_SHARED_API b3SharedMemoryStatusHandle b3ProcessServerStatus(b3PhysicsClientHandle physClient);

	/// Get the physics server return status type. See EnumSharedMemoryServerStatus in SharedMemoryPublic.h for error codes.
	DRX3D_SHARED_API i32 b3GetStatusType(b3SharedMemoryStatusHandle statusHandle);

	///Plugin system, load and unload a plugin, execute a command
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateCustomCommand(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API void b3CustomCommandLoadPlugin(b3SharedMemoryCommandHandle commandHandle, tukk pluginPath);
	DRX3D_SHARED_API void b3CustomCommandLoadPluginSetPostFix(b3SharedMemoryCommandHandle commandHandle, tukk postFix);
	DRX3D_SHARED_API i32 b3GetStatusPluginUniqueId(b3SharedMemoryStatusHandle statusHandle);
	DRX3D_SHARED_API i32 b3GetStatusPluginCommandResult(b3SharedMemoryStatusHandle statusHandle);
	DRX3D_SHARED_API i32 b3GetStatusPluginCommandReturnData(b3PhysicsClientHandle physClient, struct b3UserDataValue* valueOut);
	
	DRX3D_SHARED_API void b3CustomCommandUnloadPlugin(b3SharedMemoryCommandHandle commandHandle, i32 pluginUniqueId);
	DRX3D_SHARED_API void b3CustomCommandExecutePluginCommand(b3SharedMemoryCommandHandle commandHandle, i32 pluginUniqueId, tukk textArguments);
	DRX3D_SHARED_API void b3CustomCommandExecuteAddIntArgument(b3SharedMemoryCommandHandle commandHandle, i32 intVal);
	DRX3D_SHARED_API void b3CustomCommandExecuteAddFloatArgument(b3SharedMemoryCommandHandle commandHandle, float floatVal);

	DRX3D_SHARED_API i32 b3GetStatusBodyIndices(b3SharedMemoryStatusHandle statusHandle, i32* bodyIndicesOut, i32 bodyIndicesCapacity);

	DRX3D_SHARED_API i32 b3GetStatusBodyIndex(b3SharedMemoryStatusHandle statusHandle);

	DRX3D_SHARED_API i32 b3GetStatusActualState(b3SharedMemoryStatusHandle statusHandle,
											 i32* bodyUniqueId,
											 i32* numDegreeOfFreedomQ,
											 i32* numDegreeOfFreedomU,
											 const double* rootLocalInertialFrame[],
											 const double* actualStateQ[],
											 const double* actualStateQdot[],
											 const double* jointReactionForces[]);

	DRX3D_SHARED_API i32 b3GetStatusActualState2(b3SharedMemoryStatusHandle statusHandle,
											  i32* bodyUniqueId,
											  i32* numLinks,
											  i32* numDegreeOfFreedomQ,
											  i32* numDegreeOfFreedomU,
											  const double* rootLocalInertialFrame[],
											  const double* actualStateQ[],
											  const double* actualStateQdot[],
											  const double* jointReactionForces[],
											  const double* linkLocalInertialFrames[],
											  const double* jointMotorForces[],
											  const double* linkStates[],
											  const double* linkWorldVelocities[]);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestCollisionInfoCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId);
	DRX3D_SHARED_API i32 b3GetStatusAABB(b3SharedMemoryStatusHandle statusHandle, i32 linkIndex, double aabbMin[/*3*/], double aabbMax[/*3*/]);

	///If you re-connected to an existing server, or server changed otherwise, sync the body info and user constraints etc.
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitSyncBodyInfoCommand(b3PhysicsClientHandle physClient);

	// Sync the body info of a single body. Useful when a new body has been added by a different client (e,g, when detecting through a body added notification).
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestBodyInfoCommand(b3PhysicsClientHandle physClient, i32 bodyUniqueId);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveBodyCommand(b3PhysicsClientHandle physClient, i32 bodyUniqueId);

	///return the total number of bodies in the simulation
	DRX3D_SHARED_API i32 b3GetNumBodies(b3PhysicsClientHandle physClient);

	/// return the body unique id, given the index in range [0 , b3GetNumBodies() )
	DRX3D_SHARED_API i32 b3GetBodyUniqueId(b3PhysicsClientHandle physClient, i32 serialIndex);

	///given a body unique id, return the body information. See b3BodyInfo in SharedMemoryPublic.h
	DRX3D_SHARED_API i32 b3GetBodyInfo(b3PhysicsClientHandle physClient, i32 bodyUniqueId, struct b3BodyInfo* info);

	///give a unique body index (after loading the body) return the number of joints.
	DRX3D_SHARED_API i32 b3GetNumJoints(b3PhysicsClientHandle physClient, i32 bodyUniqueId);
	  ///give a unique body index (after loading the body) return the number of degrees of freedom (DoF).
        DRX3D_SHARED_API i32 b3GetNumDofs(b3PhysicsClientHandle physClient, i32 bodyUniqueId);
	
	///compute the number of degrees of freedom for this body.
	///Return -1 for unsupported spherical joint, -2 for unsupported planar joint.
	DRX3D_SHARED_API i32 b3ComputeDofCount(b3PhysicsClientHandle physClient, i32 bodyUniqueId);

	///given a body and joint index, return the joint information. See b3JointInfo in SharedMemoryPublic.h
	DRX3D_SHARED_API i32 b3GetJointInfo(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 jointIndex, struct b3JointInfo* info);

	///user data handling
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitSyncUserDataCommand(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API void b3AddBodyToSyncUserDataRequest(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitAddUserDataCommand(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key, enum UserDataValueType valueType, i32 valueLength, ukk valueData);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveUserDataCommand(b3PhysicsClientHandle physClient, i32 userDataId);

	DRX3D_SHARED_API i32 b3GetUserData(b3PhysicsClientHandle physClient, i32 userDataId, struct b3UserDataValue* valueOut);
	DRX3D_SHARED_API i32 b3GetUserDataId(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key);
	DRX3D_SHARED_API i32 b3GetUserDataIdFromStatus(b3SharedMemoryStatusHandle statusHandle);
	DRX3D_SHARED_API i32 b3GetNumUserData(b3PhysicsClientHandle physClient, i32 bodyUniqueId);
	DRX3D_SHARED_API void b3GetUserDataInfo(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 userDataIndex, tukk* keyOut, i32* userDataIdOut, i32* linkIndexOut, i32* visualShapeIndexOut);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3GetDynamicsInfoCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3GetDynamicsInfoCommandInit2(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex);

	///given a body unique id and link index, return the dynamics information. See b3DynamicsInfo in SharedMemoryPublic.h
	DRX3D_SHARED_API i32 b3GetDynamicsInfo(b3SharedMemoryStatusHandle statusHandle, struct b3DynamicsInfo* info);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitChangeDynamicsInfo(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitChangeDynamicsInfo2(b3SharedMemoryCommandHandle commandHandle);

	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetMass(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double mass);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetLocalInertiaDiagonal(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, const double localInertiaDiagonal[]);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetAnisotropicFriction(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, const double anisotropicFriction[]);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetJointLimit(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double jointLowerLimit, double jointUpperLimit);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetJointLimitForce(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double jointLimitForce);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetDynamicType(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, i32 dynamicType);

	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetSleepThreshold(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double sleepThreshold);
	
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetLateralFriction(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double lateralFriction);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetSpinningFriction(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double friction);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetRollingFriction(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double friction);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetRestitution(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double restitution);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetLinearDamping(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double linearDamping);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetAngularDamping(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double angularDamping);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetJointDamping(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double jointDamping);
	
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetContactStiffnessAndDamping(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double contactStiffness, double contactDamping);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetFrictionAnchor(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, i32 frictionAnchor);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetCcdSweptSphereRadius(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double ccdSweptSphereRadius);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetContactProcessingThreshold(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double contactProcessingThreshold);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetActivationState(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 activationState);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetMaxJointVelocity(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double maxJointVelocity);
	DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetCollisionMargin(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double collisionMargin);
	
	
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitCreateUserConstraintCommand(b3PhysicsClientHandle physClient, i32 parentBodyUniqueId, i32 parentJointIndex, i32 childBodyUniqueId, i32 childJointIndex, struct b3JointInfo* info);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitCreateUserConstraintCommand2(b3SharedMemoryCommandHandle commandHandle, i32 parentBodyUniqueId, i32 parentJointIndex, i32 childBodyUniqueId, i32 childJointIndex, struct b3JointInfo* info);

	///return a unique id for the user constraint, after successful creation, or -1 for an invalid constraint id
	DRX3D_SHARED_API i32 b3GetStatusUserConstraintUniqueId(b3SharedMemoryStatusHandle statusHandle);

	///change parameters of an existing user constraint
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitChangeUserConstraintCommand(b3PhysicsClientHandle physClient, i32 userConstraintUniqueId);
	DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetPivotInB(b3SharedMemoryCommandHandle commandHandle, const double jointChildPivot[/*3*/]);
	DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetFrameInB(b3SharedMemoryCommandHandle commandHandle, const double jointChildFrameOrn[/*4*/]);
	DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetMaxForce(b3SharedMemoryCommandHandle commandHandle, double maxAppliedForce);
	DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetGearRatio(b3SharedMemoryCommandHandle commandHandle, double gearRatio);
	DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetGearAuxLink(b3SharedMemoryCommandHandle commandHandle, i32 gearAuxLink);
	DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetRelativePositionTarget(b3SharedMemoryCommandHandle commandHandle, double relativePositionTarget);
	DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetERP(b3SharedMemoryCommandHandle commandHandle, double erp);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveUserConstraintCommand(b3PhysicsClientHandle physClient, i32 userConstraintUniqueId);

	DRX3D_SHARED_API i32 b3GetNumUserConstraints(b3PhysicsClientHandle physClient);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitGetUserConstraintStateCommand(b3PhysicsClientHandle physClient, i32 constraintUniqueId);
	DRX3D_SHARED_API i32 b3GetStatusUserConstraintState(b3SharedMemoryStatusHandle statusHandle, struct b3UserConstraintState* constraintState);

	DRX3D_SHARED_API i32 b3GetUserConstraintInfo(b3PhysicsClientHandle physClient, i32 constraintUniqueId, struct b3UserConstraint* info);
	/// return the user constraint id, given the index in range [0 , b3GetNumUserConstraints() )
	DRX3D_SHARED_API i32 b3GetUserConstraintId(b3PhysicsClientHandle physClient, i32 serialIndex);

	///Request physics debug lines for debug visualization. The flags in debugMode are the same as used in drx3D
	///See btIDebugDraw::DebugDrawModes in drx3D/src/LinearMath/IDebugDraw.h
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestDebugLinesCommand(b3PhysicsClientHandle physClient, i32 debugMode);

	///Get the pointers to the physics debug line information, after b3InitRequestDebugLinesCommand returns
	///status CMD_DEBUG_LINES_COMPLETED
	DRX3D_SHARED_API void b3GetDebugLines(b3PhysicsClientHandle physClient, struct b3DebugLines* lines);

	///configure the 3D OpenGL debug visualizer (enable/disable GUI widgets, shadows, position camera etc)
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitConfigureOpenGLVisualizer(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitConfigureOpenGLVisualizer2(b3SharedMemoryCommandHandle commandHandle);
	DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetVisualizationFlags(b3SharedMemoryCommandHandle commandHandle, i32 flag, i32 enabled);
	DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetLightPosition(b3SharedMemoryCommandHandle commandHandle, const float lightPosition[3]);
	DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetShadowMapResolution(b3SharedMemoryCommandHandle commandHandle, i32 shadowMapResolution);
	DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetShadowMapIntensity(b3SharedMemoryCommandHandle commandHandle, double shadowMapIntensity);
	DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetLightRgbBackground(b3SharedMemoryCommandHandle commandHandle, const float rgbBackground[3]);
	

	DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetShadowMapWorldSize(b3SharedMemoryCommandHandle commandHandle, i32 shadowMapWorldSize);
	DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetRemoteSyncTransformInterval(b3SharedMemoryCommandHandle commandHandle, double remoteSyncTransformInterval);
	
	DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetViewMatrix(b3SharedMemoryCommandHandle commandHandle, float cameraDistance, float cameraPitch, float cameraYaw, const float cameraTargetPosition[/*3*/]);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestOpenGLVisualizerCameraCommand(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API i32 b3GetStatusOpenGLVisualizerCamera(b3SharedMemoryStatusHandle statusHandle, struct b3OpenGLVisualizerCameraInfo* camera);

	/// Add/remove user-specific debug lines and debug text messages
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawAddLine3D(b3PhysicsClientHandle physClient, const double fromXYZ[/*3*/], const double toXYZ[/*3*/], const double colorRGB[/*3*/], double lineWidth, double lifeTime);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawAddPoints3D(b3PhysicsClientHandle physClient, const double positionsXYZ[/*3n*/], const double colorsRGB[/*3*/], double pointSize, double lifeTime, i32 pointNum);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawAddText3D(b3PhysicsClientHandle physClient, tukk txt, const double positionXYZ[/*3*/], const double colorRGB[/*3*/], double textSize, double lifeTime);
	DRX3D_SHARED_API void b3UserDebugTextSetOptionFlags(b3SharedMemoryCommandHandle commandHandle, i32 optionFlags);
	DRX3D_SHARED_API void b3UserDebugTextSetOrientation(b3SharedMemoryCommandHandle commandHandle, const double orientation[/*4*/]);
	DRX3D_SHARED_API void b3UserDebugItemSetReplaceItemUniqueId(b3SharedMemoryCommandHandle commandHandle, i32 replaceItem);

	DRX3D_SHARED_API void b3UserDebugItemSetParentObject(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId, i32 linkIndex);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugAddParameter(b3PhysicsClientHandle physClient, tukk txt, double rangeMin, double rangeMax, double startValue);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugReadParameter(b3PhysicsClientHandle physClient, i32 debugItemUniqueId);
	DRX3D_SHARED_API i32 b3GetStatusDebugParameterValue(b3SharedMemoryStatusHandle statusHandle, double* paramValue);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawRemove(b3PhysicsClientHandle physClient, i32 debugItemUniqueId);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawRemoveAll(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserRemoveAllParameters(b3PhysicsClientHandle physClient);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitDebugDrawingCommand(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API void b3SetDebugObjectColor(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId, i32 linkIndex, const double objectColorRGB[/*3*/]);
	DRX3D_SHARED_API void b3RemoveDebugObjectColor(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId, i32 linkIndex);

	///All debug items unique Ids are positive: a negative unique Id means failure.
	DRX3D_SHARED_API i32 b3GetDebugItemUniqueId(b3SharedMemoryStatusHandle statusHandle);

	///request an image from a simulated camera, using a software renderer.
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestCameraImage(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestCameraImage2(b3SharedMemoryCommandHandle commandHandle);
	DRX3D_SHARED_API void b3RequestCameraImageSetCameraMatrices(b3SharedMemoryCommandHandle commandHandle, float viewMatrix[/*16*/], float projectionMatrix[/*16*/]);
	DRX3D_SHARED_API void b3RequestCameraImageSetPixelResolution(b3SharedMemoryCommandHandle commandHandle, i32 width, i32 height);
	DRX3D_SHARED_API void b3RequestCameraImageSetLightDirection(b3SharedMemoryCommandHandle commandHandle, const float lightDirection[/*3*/]);
	DRX3D_SHARED_API void b3RequestCameraImageSetLightColor(b3SharedMemoryCommandHandle commandHandle, const float lightColor[/*3*/]);
	DRX3D_SHARED_API void b3RequestCameraImageSetLightDistance(b3SharedMemoryCommandHandle commandHandle, float lightDistance);
	DRX3D_SHARED_API void b3RequestCameraImageSetLightAmbientCoeff(b3SharedMemoryCommandHandle commandHandle, float lightAmbientCoeff);
	DRX3D_SHARED_API void b3RequestCameraImageSetLightDiffuseCoeff(b3SharedMemoryCommandHandle commandHandle, float lightDiffuseCoeff);
	DRX3D_SHARED_API void b3RequestCameraImageSetLightSpecularCoeff(b3SharedMemoryCommandHandle commandHandle, float lightSpecularCoeff);
	DRX3D_SHARED_API void b3RequestCameraImageSetShadow(b3SharedMemoryCommandHandle commandHandle, i32 hasShadow);
	DRX3D_SHARED_API void b3RequestCameraImageSelectRenderer(b3SharedMemoryCommandHandle commandHandle, i32 renderer);
	DRX3D_SHARED_API void b3RequestCameraImageSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);

	DRX3D_SHARED_API void b3GetCameraImageData(b3PhysicsClientHandle physClient, struct b3CameraImageData* imageData);

	///set projective texture camera matrices.
	DRX3D_SHARED_API void b3RequestCameraImageSetProjectiveTextureMatrices(b3SharedMemoryCommandHandle commandHandle, float viewMatrix[/*16*/], float projectionMatrix[/*16*/]);

	///compute a view matrix, helper function for b3RequestCameraImageSetCameraMatrices
	DRX3D_SHARED_API void b3ComputeViewMatrixFromPositions(const float cameraPosition[/*3*/], const float cameraTargetPosition[/*3*/], const float cameraUp[/*3*/], float viewMatrix[/*16*/]);
	DRX3D_SHARED_API void b3ComputeViewMatrixFromYawPitchRoll(const float cameraTargetPosition[/*3*/], float distance, float yaw, float pitch, float roll, i32 upAxis, float viewMatrix[/*16*/]);
	DRX3D_SHARED_API void b3ComputePositionFromViewMatrix(const float viewMatrix[/*16*/], float cameraPosition[/*3*/], float cameraTargetPosition[/*3*/], float cameraUp[/*3*/]);

	///compute a projection matrix, helper function for b3RequestCameraImageSetCameraMatrices
	DRX3D_SHARED_API void b3ComputeProjectionMatrix(float left, float right, float bottom, float top, float nearVal, float farVal, float projectionMatrix[/*16*/]);
	DRX3D_SHARED_API void b3ComputeProjectionMatrixFOV(float fov, float aspect, float nearVal, float farVal, float projectionMatrix[/*16*/]);

	/* obsolete, please use b3ComputeViewProjectionMatrices */
	DRX3D_SHARED_API void b3RequestCameraImageSetViewMatrix(b3SharedMemoryCommandHandle commandHandle, const float cameraPosition[/*3*/], const float cameraTargetPosition[/*3*/], const float cameraUp[/*3*/]);
	/* obsolete, please use b3ComputeViewProjectionMatrices */
	DRX3D_SHARED_API void b3RequestCameraImageSetViewMatrix2(b3SharedMemoryCommandHandle commandHandle, const float cameraTargetPosition[/*3*/], float distance, float yaw, float pitch, float roll, i32 upAxis);
	/* obsolete, please use b3ComputeViewProjectionMatrices */
	DRX3D_SHARED_API void b3RequestCameraImageSetProjectionMatrix(b3SharedMemoryCommandHandle commandHandle, float left, float right, float bottom, float top, float nearVal, float farVal);
	/* obsolete, please use b3ComputeViewProjectionMatrices */
	DRX3D_SHARED_API void b3RequestCameraImageSetFOVProjectionMatrix(b3SharedMemoryCommandHandle commandHandle, float fov, float aspect, float nearVal, float farVal);

	///request an contact point information
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestContactPointInformation(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API void b3SetContactFilterBodyA(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdA);
	DRX3D_SHARED_API void b3SetContactFilterBodyB(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdB);
	DRX3D_SHARED_API void b3SetContactFilterLinkA(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexA);
	DRX3D_SHARED_API void b3SetContactFilterLinkB(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexB);
	DRX3D_SHARED_API void b3GetContactPointInformation(b3PhysicsClientHandle physClient, struct b3ContactInformation* contactPointData);

	///compute the closest points between two bodies
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitClosestDistanceQuery(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API void b3SetClosestDistanceFilterBodyA(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdA);
	DRX3D_SHARED_API void b3SetClosestDistanceFilterLinkA(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexA);
	DRX3D_SHARED_API void b3SetClosestDistanceFilterBodyB(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdB);
	DRX3D_SHARED_API void b3SetClosestDistanceFilterLinkB(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexB);
	DRX3D_SHARED_API void b3SetClosestDistanceThreshold(b3SharedMemoryCommandHandle commandHandle, double distance);

	DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapeA(b3SharedMemoryCommandHandle commandHandle, i32 collisionShapeA);
	DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapeB(b3SharedMemoryCommandHandle commandHandle, i32 collisionShapeB);
	DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapePositionA(b3SharedMemoryCommandHandle commandHandle, const double collisionShapePositionA[/*3*/]);
	DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapePositionB(b3SharedMemoryCommandHandle commandHandle, const double collisionShapePositionB[/*3*/]);
	DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapeOrientationA(b3SharedMemoryCommandHandle commandHandle, const double collisionShapeOrientationA[/*4*/]);
	DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapeOrientationB(b3SharedMemoryCommandHandle commandHandle, const double collisionShapeOrientationB[/*4*/]);

	DRX3D_SHARED_API void b3GetClosestPointInformation(b3PhysicsClientHandle physClient, struct b3ContactInformation* contactPointInfo);

	///get all the bodies that touch a given axis aligned bounding box specified in world space (min and max coordinates)
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitAABBOverlapQuery(b3PhysicsClientHandle physClient, const double aabbMin[/*3*/], const double aabbMax[/*3*/]);
	DRX3D_SHARED_API void b3GetAABBOverlapResults(b3PhysicsClientHandle physClient, struct b3AABBOverlapData* data);

	//request visual shape information
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestVisualShapeInformation(b3PhysicsClientHandle physClient, i32 bodyUniqueIdA);
	DRX3D_SHARED_API void b3GetVisualShapeInformation(b3PhysicsClientHandle physClient, struct b3VisualShapeInformation* visualShapeInfo);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestCollisionShapeInformation(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex);
	DRX3D_SHARED_API void b3GetCollisionShapeInformation(b3PhysicsClientHandle physClient, struct b3CollisionShapeInformation* collisionShapeInfo);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitLoadTexture(b3PhysicsClientHandle physClient, tukk filename);
	DRX3D_SHARED_API i32 b3GetStatusTextureUniqueId(b3SharedMemoryStatusHandle statusHandle);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateChangeTextureCommandInit(b3PhysicsClientHandle physClient, i32 textureUniqueId, i32 width, i32 height, tukk rgbPixels);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUpdateVisualShape(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 jointIndex, i32 shapeIndex, i32 textureUniqueId);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUpdateVisualShape2(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 jointIndex, i32 shapeIndex);
	DRX3D_SHARED_API void b3UpdateVisualShapeTexture(b3SharedMemoryCommandHandle commandHandle, i32 textureUniqueId);
	DRX3D_SHARED_API void b3UpdateVisualShapeRGBAColor(b3SharedMemoryCommandHandle commandHandle, const double rgbaColor[/*4*/]);
	DRX3D_SHARED_API void b3UpdateVisualShapeFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);
	
	DRX3D_SHARED_API void b3UpdateVisualShapeSpecularColor(b3SharedMemoryCommandHandle commandHandle, const double specularColor[/*3*/]);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitPhysicsParamCommand(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitPhysicsParamCommand2(b3SharedMemoryCommandHandle commandHandle);
	DRX3D_SHARED_API i32 b3PhysicsParamSetGravity(b3SharedMemoryCommandHandle commandHandle, double gravx, double gravy, double gravz);
	DRX3D_SHARED_API i32 b3PhysicsParamSetTimeStep(b3SharedMemoryCommandHandle commandHandle, double timeStep);
	DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultContactERP(b3SharedMemoryCommandHandle commandHandle, double defaultContactERP);
	DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultNonContactERP(b3SharedMemoryCommandHandle commandHandle, double defaultNonContactERP);
	DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultFrictionERP(b3SharedMemoryCommandHandle commandHandle, double frictionERP);
	DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultGlobalCFM(b3SharedMemoryCommandHandle commandHandle, double defaultGlobalCFM);
	DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultFrictionCFM(b3SharedMemoryCommandHandle commandHandle, double frictionCFM);
	DRX3D_SHARED_API i32 b3PhysicsParamSetNumSubSteps(b3SharedMemoryCommandHandle commandHandle, i32 numSubSteps);
	DRX3D_SHARED_API i32 b3PhysicsParamSetRealTimeSimulation(b3SharedMemoryCommandHandle commandHandle, i32 enableRealTimeSimulation);
	DRX3D_SHARED_API i32 b3PhysicsParamSetNumSolverIterations(b3SharedMemoryCommandHandle commandHandle, i32 numSolverIterations);
	DRX3D_SHARED_API i32 b3PhysicsParamSetNumNonContactInnerIterations(b3SharedMemoryCommandHandle commandHandle, i32 numMotorIterations);
	DRX3D_SHARED_API i32 b3PhysicsParamSetWarmStartingFactor(b3SharedMemoryCommandHandle commandHandle, double warmStartingFactor);
	DRX3D_SHARED_API i32 b3PhysicsParamSetArticulatedWarmStartingFactor(b3SharedMemoryCommandHandle commandHandle, double warmStartingFactor);
	DRX3D_SHARED_API i32 b3PhysicsParamSetCollisionFilterMode(b3SharedMemoryCommandHandle commandHandle, i32 filterMode);
	DRX3D_SHARED_API i32 b3PhysicsParamSetUseSplitImpulse(b3SharedMemoryCommandHandle commandHandle, i32 useSplitImpulse);
	DRX3D_SHARED_API i32 b3PhysicsParamSetSplitImpulsePenetrationThreshold(b3SharedMemoryCommandHandle commandHandle, double splitImpulsePenetrationThreshold);
	DRX3D_SHARED_API i32 b3PhysicsParamSetContactBreakingThreshold(b3SharedMemoryCommandHandle commandHandle, double contactBreakingThreshold);
	DRX3D_SHARED_API i32 b3PhysicsParamSetMaxNumCommandsPer1ms(b3SharedMemoryCommandHandle commandHandle, i32 maxNumCmdPer1ms);
	DRX3D_SHARED_API i32 b3PhysicsParamSetEnableFileCaching(b3SharedMemoryCommandHandle commandHandle, i32 enableFileCaching);
	DRX3D_SHARED_API i32 b3PhysicsParamSetRestitutionVelocityThreshold(b3SharedMemoryCommandHandle commandHandle, double restitutionVelocityThreshold);
	DRX3D_SHARED_API i32 b3PhysicsParamSetEnableConeFriction(b3SharedMemoryCommandHandle commandHandle, i32 enableConeFriction);
	DRX3D_SHARED_API i32 b3PhysicsParameterSetDeterministicOverlappingPairs(b3SharedMemoryCommandHandle commandHandle, i32 deterministicOverlappingPairs);
	DRX3D_SHARED_API i32 b3PhysicsParameterSetAllowedCcdPenetration(b3SharedMemoryCommandHandle commandHandle, double allowedCcdPenetration);
	DRX3D_SHARED_API i32 b3PhysicsParameterSetJointFeedbackMode(b3SharedMemoryCommandHandle commandHandle, i32 jointFeedbackMode);
	DRX3D_SHARED_API i32 b3PhysicsParamSetSolverResidualThreshold(b3SharedMemoryCommandHandle commandHandle, double solverResidualThreshold);
	DRX3D_SHARED_API i32 b3PhysicsParamSetContactSlop(b3SharedMemoryCommandHandle commandHandle, double contactSlop);
	DRX3D_SHARED_API i32 b3PhysicsParameterSetEnableSAT(b3SharedMemoryCommandHandle commandHandle, i32 enableSAT);
	DRX3D_SHARED_API i32 b3PhysicsParameterSetConstraintSolverType(b3SharedMemoryCommandHandle commandHandle, i32 constraintSolverType);
	DRX3D_SHARED_API i32 b3PhysicsParameterSetMinimumSolverIslandSize(b3SharedMemoryCommandHandle commandHandle, i32 minimumSolverIslandSize);
	DRX3D_SHARED_API i32 b3PhysicsParamSetSolverAnalytics(b3SharedMemoryCommandHandle commandHandle, i32 reportSolverAnalytics);
	DRX3D_SHARED_API i32 b3PhysicsParameterSetSparseSdfVoxelSize(b3SharedMemoryCommandHandle commandHandle, double sparseSdfVoxelSize);
	                  
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestPhysicsParamCommand(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API i32 b3GetStatusPhysicsSimulationParameters(b3SharedMemoryStatusHandle statusHandle, struct b3PhysicsSimulationParameters* params);

	//b3PhysicsParamSetInternalSimFlags is for internal/temporary/easter-egg/experimental demo purposes
	//Use at own risk: magic things may or my not happen when calling this API
	DRX3D_SHARED_API i32 b3PhysicsParamSetInternalSimFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitStepSimulationCommand(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitStepSimulationCommand2(b3SharedMemoryCommandHandle commandHandle);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitPerformCollisionDetectionCommand(b3PhysicsClientHandle physClient);

	DRX3D_SHARED_API i32 b3GetStatusForwardDynamicsAnalyticsData(b3SharedMemoryStatusHandle statusHandle, struct b3ForwardDynamicsAnalyticsArgs* analyticsData);


	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitResetSimulationCommand(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitResetSimulationCommand2(b3SharedMemoryCommandHandle commandHandle);
	DRX3D_SHARED_API i32 b3InitResetSimulationSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);
	///Load a robot from a URDF file. Status type will CMD_URDF_LOADING_COMPLETED.
	///Access the robot from the unique body index, through b3GetStatusBodyIndex(statusHandle);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadUrdfCommandInit(b3PhysicsClientHandle physClient, tukk urdfFileName);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadUrdfCommandInit2(b3SharedMemoryCommandHandle commandHandle, tukk urdfFileName);
	DRX3D_SHARED_API i32 b3LoadUrdfCommandSetStartPosition(b3SharedMemoryCommandHandle commandHandle, double startPosX, double startPosY, double startPosZ);
	DRX3D_SHARED_API i32 b3LoadUrdfCommandSetStartOrientation(b3SharedMemoryCommandHandle commandHandle, double startOrnX, double startOrnY, double startOrnZ, double startOrnW);
	DRX3D_SHARED_API i32 b3LoadUrdfCommandSetUseMultiBody(b3SharedMemoryCommandHandle commandHandle, i32 useMultiBody);
	DRX3D_SHARED_API i32 b3LoadUrdfCommandSetUseFixedBase(b3SharedMemoryCommandHandle commandHandle, i32 useFixedBase);
	DRX3D_SHARED_API i32 b3LoadUrdfCommandSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);
	DRX3D_SHARED_API i32 b3LoadUrdfCommandSetGlobalScaling(b3SharedMemoryCommandHandle commandHandle, double globalScaling);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SaveStateCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveStateCommand(b3PhysicsClientHandle physClient, i32 stateId);
	DRX3D_SHARED_API i32 b3GetStatusGetStateId(b3SharedMemoryStatusHandle statusHandle);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadStateCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API i32 b3LoadStateSetStateId(b3SharedMemoryCommandHandle commandHandle, i32 stateId);
	DRX3D_SHARED_API i32 b3LoadStateSetFileName(b3SharedMemoryCommandHandle commandHandle, tukk fileName);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadBulletCommandInit(b3PhysicsClientHandle physClient, tukk fileName);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SaveBulletCommandInit(b3PhysicsClientHandle physClient, tukk fileName);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadMJCFCommandInit(b3PhysicsClientHandle physClient, tukk fileName);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadMJCFCommandInit2(b3SharedMemoryCommandHandle commandHandle, tukk fileName);
	DRX3D_SHARED_API void b3LoadMJCFCommandSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);
	DRX3D_SHARED_API void b3LoadMJCFCommandSetUseMultiBody(b3SharedMemoryCommandHandle commandHandle, i32 useMultiBody);
	

	///compute the forces to achieve an acceleration, given a state q and qdot using inverse dynamics
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3Calculatedrx3d_inverseCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId,
																					const double* jointPositionsQ, const double* jointVelocitiesQdot, const double* jointAccelerations);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3Calculatedrx3d_inverseCommandInit2(b3PhysicsClientHandle physClient, i32 bodyUniqueId,
		const double* jointPositionsQ, i32 dofCountQ, const double* jointVelocitiesQdot, const double* jointAccelerations, i32 dofCountQdot);
	DRX3D_SHARED_API void b3Calculatedrx3d_inverseSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);

	DRX3D_SHARED_API i32 b3GetStatusdrx3d_inverseJointForces(b3SharedMemoryStatusHandle statusHandle,
															i32* bodyUniqueId,
															i32* dofCount,
															double* jointForces);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CalculateJacobianCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex, const double* localPosition, const double* jointPositionsQ, const double* jointVelocitiesQdot, const double* jointAccelerations);
	DRX3D_SHARED_API i32 b3GetStatusJacobian(b3SharedMemoryStatusHandle statusHandle,
										  i32* dofCount,
										  double* linearJacobian,
										  double* angularJacobian);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CalculateMassMatrixCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, const double* jointPositionsQ, i32 dofCountQ);
	DRX3D_SHARED_API void b3CalculateMassMatrixSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);
	///the mass matrix is stored in column-major layout of size dofCount*dofCount
	DRX3D_SHARED_API i32 b3GetStatusMassMatrix(b3PhysicsClientHandle physClient, b3SharedMemoryStatusHandle statusHandle, i32* dofCount, double* massMatrix);

	///compute the joint positions to move the end effector to a desired target using inverse kinematics
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CalculateInverseKinematicsCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId);
	DRX3D_SHARED_API void b3CalculateInverseKinematicsAddTargetPurePosition(b3SharedMemoryCommandHandle commandHandle, i32 endEffectorLinkIndex, const double targetPosition[/*3*/]);
	DRX3D_SHARED_API void b3CalculateInverseKinematicsAddTargetsPurePosition(b3SharedMemoryCommandHandle commandHandle, i32 numEndEffectorLinkIndices, i32k* endEffectorIndices, const double* targetPositions);
	DRX3D_SHARED_API void b3CalculateInverseKinematicsAddTargetPositionWithOrientation(b3SharedMemoryCommandHandle commandHandle, i32 endEffectorLinkIndex, const double targetPosition[/*3*/], const double targetOrientation[/*4*/]);
	DRX3D_SHARED_API void b3CalculateInverseKinematicsPosWithNullSpaceVel(b3SharedMemoryCommandHandle commandHandle, i32 numDof, i32 endEffectorLinkIndex, const double targetPosition[/*3*/], const double* lowerLimit, const double* upperLimit, const double* jointRange, const double* restPose);
	DRX3D_SHARED_API void b3CalculateInverseKinematicsPosOrnWithNullSpaceVel(b3SharedMemoryCommandHandle commandHandle, i32 numDof, i32 endEffectorLinkIndex, const double targetPosition[/*3*/], const double targetOrientation[/*4*/], const double* lowerLimit, const double* upperLimit, const double* jointRange, const double* restPose);
	DRX3D_SHARED_API void b3CalculateInverseKinematicsSetJointDamping(b3SharedMemoryCommandHandle commandHandle, i32 numDof, const double* jointDampingCoeff);
	DRX3D_SHARED_API void b3CalculateInverseKinematicsSelectSolver(b3SharedMemoryCommandHandle commandHandle, i32 solver);
	DRX3D_SHARED_API i32 b3GetStatusInverseKinematicsJointPositions(b3SharedMemoryStatusHandle statusHandle,
																 i32* bodyUniqueId,
																 i32* dofCount,
																 double* jointPositions);

	DRX3D_SHARED_API void b3CalculateInverseKinematicsSetCurrentPositions(b3SharedMemoryCommandHandle commandHandle, i32 numDof, const double* currentJointPositions);
	DRX3D_SHARED_API void b3CalculateInverseKinematicsSetMaxNumIterations(b3SharedMemoryCommandHandle commandHandle, i32 maxNumIterations);
	DRX3D_SHARED_API void b3CalculateInverseKinematicsSetResidualThreshold(b3SharedMemoryCommandHandle commandHandle, double residualThreshold);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CollisionFilterCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API void b3SetCollisionFilterPair(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdA,
												i32 bodyUniqueIdB, i32 linkIndexA, i32 linkIndexB, i32 enableCollision);
	DRX3D_SHARED_API void b3SetCollisionFilterGroupMask(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdA,
													 i32 linkIndexA, i32 collisionFilterGroup, i32 collisionFilterMask);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadSdfCommandInit(b3PhysicsClientHandle physClient, tukk sdfFileName);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadSdfCommandInit2(b3SharedMemoryCommandHandle commandHandle, tukk sdfFileName);

	DRX3D_SHARED_API i32 b3LoadSdfCommandSetUseMultiBody(b3SharedMemoryCommandHandle commandHandle, i32 useMultiBody);
	DRX3D_SHARED_API i32 b3LoadSdfCommandSetUseGlobalScaling(b3SharedMemoryCommandHandle commandHandle, double globalScaling);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SaveWorldCommandInit(b3PhysicsClientHandle physClient, tukk sdfFileName);

	///The b3JointControlCommandInit method is obsolete, use b3JointControlCommandInit2 instead
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3JointControlCommandInit(b3PhysicsClientHandle physClient, i32 controlMode);

	///Set joint motor control variables such as desired position/angle, desired velocity,
	///applied joint forces, dependent on the control mode (CONTROL_MODE_VELOCITY or CONTROL_MODE_TORQUE)
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3JointControlCommandInit2(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 controlMode);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3JointControlCommandInit2Internal(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 controlMode);

	///Only use when controlMode is CONTROL_MODE_POSITION_VELOCITY_PD
	DRX3D_SHARED_API i32 b3JointControlSetDesiredPosition(b3SharedMemoryCommandHandle commandHandle, i32 qIndex, double value);
	DRX3D_SHARED_API i32 b3JointControlSetDesiredPositionMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 qIndex, const double* position, i32 dofCount);

	DRX3D_SHARED_API i32 b3JointControlSetKp(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value);
	DRX3D_SHARED_API i32 b3JointControlSetKpMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double* kps, i32 dofCount);
	DRX3D_SHARED_API i32 b3JointControlSetKd(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value);
	DRX3D_SHARED_API i32 b3JointControlSetKdMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double* kds, i32 dofCount);
	DRX3D_SHARED_API i32 b3JointControlSetMaximumVelocity(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double maximumVelocity);

	///Only use when controlMode is CONTROL_MODE_VELOCITY
	DRX3D_SHARED_API i32 b3JointControlSetDesiredVelocity(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value); /* find a better name for dof/q/u indices, point to b3JointInfo */
	DRX3D_SHARED_API i32 b3JointControlSetDesiredVelocityMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, const double* velocity, i32 dofCount);
	DRX3D_SHARED_API i32 b3JointControlSetDesiredVelocityMultiDof2(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, const double* velocity, i32 dofCount);

	DRX3D_SHARED_API i32 b3JointControlSetMaximumForce(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value);
	DRX3D_SHARED_API i32 b3JointControlSetDesiredForceTorqueMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double* forces, i32 dofCount);
	DRX3D_SHARED_API i32 b3JointControlSetDamping(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value);
	DRX3D_SHARED_API i32 b3JointControlSetDampingMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double* damping, i32 dofCount);
	
	///Only use if when controlMode is CONTROL_MODE_TORQUE,
	DRX3D_SHARED_API i32 b3JointControlSetDesiredForceTorque(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value);

	///the creation of collision shapes and rigid bodies etc is likely going to change,
	///but good to have a b3CreateBoxShapeCommandInit for now

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateCollisionShapeCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddSphere(b3SharedMemoryCommandHandle commandHandle, double radius);
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddBox(b3SharedMemoryCommandHandle commandHandle, const double halfExtents[/*3*/]);
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddCapsule(b3SharedMemoryCommandHandle commandHandle, double radius, double height);
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddCylinder(b3SharedMemoryCommandHandle commandHandle, double radius, double height);
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddHeightfield(b3SharedMemoryCommandHandle commandHandle, tukk fileName, const double meshScale[/*3*/], double textureScaling);
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddHeightfield2(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double meshScale[/*3*/], double textureScaling, float* heightfieldData, i32 numHeightfieldRows, i32 numHeightfieldColumns, i32 replaceHeightfieldIndex);
	
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddPlane(b3SharedMemoryCommandHandle commandHandle, const double planeNormal[/*3*/], double planeConstant);
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddMesh(b3SharedMemoryCommandHandle commandHandle, tukk fileName, const double meshScale[/*3*/]);
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddConvexMesh(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double meshScale[/*3*/], const double* vertices, i32 numVertices);
	DRX3D_SHARED_API i32 b3CreateCollisionShapeAddConcaveMesh(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double meshScale[/*3*/], const double* vertices, i32 numVertices, i32k* indices, i32 numIndices);
	DRX3D_SHARED_API void b3CreateCollisionSetFlag(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, i32 flags);
	DRX3D_SHARED_API void b3CreateCollisionShapeSetChildTransform(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, const double childPosition[/*3*/], const double childOrientation[/*4*/]);
	DRX3D_SHARED_API i32 b3GetStatusCollisionShapeUniqueId(b3SharedMemoryStatusHandle statusHandle);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveCollisionShapeCommand(b3PhysicsClientHandle physClient, i32 collisionShapeId);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3GetMeshDataCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex);
	DRX3D_SHARED_API void b3GetMeshDataSimulationMesh(b3SharedMemoryCommandHandle commandHandle);
	DRX3D_SHARED_API void b3MeshDataSimulationMeshVelocity(b3SharedMemoryCommandHandle commandHandle);
	DRX3D_SHARED_API void b3GetMeshDataSetCollisionShapeIndex(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex);
	DRX3D_SHARED_API void b3GetMeshDataSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);
	
	DRX3D_SHARED_API void b3GetMeshData(b3PhysicsClientHandle physClient, struct b3MeshData* meshData);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3ResetMeshDataCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 num_vertices, const double* vertices);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateVisualShapeCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API i32 b3CreateVisualShapeAddSphere(b3SharedMemoryCommandHandle commandHandle, double radius);
	DRX3D_SHARED_API i32 b3CreateVisualShapeAddBox(b3SharedMemoryCommandHandle commandHandle, const double halfExtents[/*3*/]);
	DRX3D_SHARED_API i32 b3CreateVisualShapeAddCapsule(b3SharedMemoryCommandHandle commandHandle, double radius, double height);
	DRX3D_SHARED_API i32 b3CreateVisualShapeAddCylinder(b3SharedMemoryCommandHandle commandHandle, double radius, double height);
	DRX3D_SHARED_API i32 b3CreateVisualShapeAddPlane(b3SharedMemoryCommandHandle commandHandle, const double planeNormal[/*3*/], double planeConstant);
	DRX3D_SHARED_API i32 b3CreateVisualShapeAddMesh(b3SharedMemoryCommandHandle commandHandle, tukk fileName, const double meshScale[/*3*/]);
	DRX3D_SHARED_API i32 b3CreateVisualShapeAddMesh2(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double meshScale[/*3*/], const double* vertices, i32 numVertices, i32k* indices, i32 numIndices, const double* normals, i32 numNormals, const double* uvs, i32 numUVs);


	DRX3D_SHARED_API void b3CreateVisualSetFlag(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, i32 flags);
	DRX3D_SHARED_API void b3CreateVisualShapeSetChildTransform(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, const double childPosition[/*3*/], const double childOrientation[/*4*/]);
	DRX3D_SHARED_API void b3CreateVisualShapeSetSpecularColor(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, const double specularColor[/*3*/]);
	DRX3D_SHARED_API void b3CreateVisualShapeSetRGBAColor(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, const double rgbaColor[/*4*/]);

	DRX3D_SHARED_API i32 b3GetStatusVisualShapeUniqueId(b3SharedMemoryStatusHandle statusHandle);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateMultiBodyCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API i32 b3CreateMultiBodyBase(b3SharedMemoryCommandHandle commandHandle, double mass, i32 collisionShapeUnique, i32 visualShapeUniqueId, const double basePosition[/*3*/], const double baseOrientation[/*4*/], const double baseInertialFramePosition[/*3*/], const double baseInertialFrameOrientation[/*4*/]);

	DRX3D_SHARED_API i32 b3CreateMultiBodyLink(b3SharedMemoryCommandHandle commandHandle, double linkMass, double linkCollisionShapeIndex,
											double linkVisualShapeIndex,
											const double linkPosition[/*3*/],
											const double linkOrientation[/*4*/],
											const double linkInertialFramePosition[/*3*/],
											const double linkInertialFrameOrientation[/*4*/],
											i32 linkParentIndex,
											i32 linkJointType,
											const double linkJointAxis[/*3*/]);

	//batch creation is an performance feature to create a large number of multi bodies in one command
	DRX3D_SHARED_API i32 b3CreateMultiBodySetBatchPositions(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, double* batchPositions, i32 numBatchObjects);

	//useMaximalCoordinates are disabled by default, enabling them is experimental and not fully supported yet
	DRX3D_SHARED_API void b3CreateMultiBodyUseMaximalCoordinates(b3SharedMemoryCommandHandle commandHandle);
	DRX3D_SHARED_API void b3CreateMultiBodySetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags);

	//i32 b3CreateMultiBodyAddLink(b3SharedMemoryCommandHandle commandHandle, i32 jointType, i32 parentLinkIndex, double linkMass, i32 linkCollisionShapeUnique, i32 linkVisualShapeUniqueId);

	///create a box of size (1,1,1) at world origin (0,0,0) at orientation quat (0,0,0,1)
	///after that, you can optionally adjust the initial position, orientation and size
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateBoxShapeCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API i32 b3CreateBoxCommandSetStartPosition(b3SharedMemoryCommandHandle commandHandle, double startPosX, double startPosY, double startPosZ);
	DRX3D_SHARED_API i32 b3CreateBoxCommandSetStartOrientation(b3SharedMemoryCommandHandle commandHandle, double startOrnX, double startOrnY, double startOrnZ, double startOrnW);
	DRX3D_SHARED_API i32 b3CreateBoxCommandSetHalfExtents(b3SharedMemoryCommandHandle commandHandle, double halfExtentsX, double halfExtentsY, double halfExtentsZ);
	DRX3D_SHARED_API i32 b3CreateBoxCommandSetMass(b3SharedMemoryCommandHandle commandHandle, double mass);
	DRX3D_SHARED_API i32 b3CreateBoxCommandSetCollisionShapeType(b3SharedMemoryCommandHandle commandHandle, i32 collisionShapeType);
	DRX3D_SHARED_API i32 b3CreateBoxCommandSetColorRGBA(b3SharedMemoryCommandHandle commandHandle, double red, double green, double blue, double alpha);

	///b3CreatePoseCommandInit will initialize (teleport) the pose of a body/robot. You can individually set the base position,
	///base orientation and joint angles. This will set all velocities of base and joints to zero.
	///This is not a robot control command using actuators/joint motors, but manual repositioning the robot.
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreatePoseCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreatePoseCommandInit2(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId);

	DRX3D_SHARED_API i32 b3CreatePoseCommandSetBasePosition(b3SharedMemoryCommandHandle commandHandle, double startPosX, double startPosY, double startPosZ);
	DRX3D_SHARED_API i32 b3CreatePoseCommandSetBaseOrientation(b3SharedMemoryCommandHandle commandHandle, double startOrnX, double startOrnY, double startOrnZ, double startOrnW);
	DRX3D_SHARED_API i32 b3CreatePoseCommandSetBaseLinearVelocity(b3SharedMemoryCommandHandle commandHandle, const double linVel[/*3*/]);
	DRX3D_SHARED_API i32 b3CreatePoseCommandSetBaseAngularVelocity(b3SharedMemoryCommandHandle commandHandle, const double angVel[/*3*/]);
	DRX3D_SHARED_API i32 b3CreatePoseCommandSetBaseScaling(b3SharedMemoryCommandHandle commandHandle, double scaling[/* 3*/]);


	DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointPositions(b3SharedMemoryCommandHandle commandHandle, i32 numJointPositions, const double* jointPositions);
	DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointPosition(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, double jointPosition);
	DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointPositionMultiDof(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, const double* jointPosition, i32 posSize);

	DRX3D_SHARED_API i32 b3CreatePoseCommandSetQ(b3SharedMemoryCommandHandle commandHandle, i32 numJointPositions, const double* q, i32k* hasQ);
	DRX3D_SHARED_API i32 b3CreatePoseCommandSetQdots(b3SharedMemoryCommandHandle commandHandle, i32 numJointVelocities, const double* qDots, i32k* hasQdots);

	DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointVelocities(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 numJointVelocities, const double* jointVelocities);
	DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointVelocity(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, double jointVelocity);
	DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointVelocityMultiDof(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, const double* jointVelocity, i32 velSize);
	
	///We are currently not reading the sensor information from the URDF file, and programmatically assign sensors.
	///This is rather inconsistent, to mix programmatical creation with loading from file.
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateSensorCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId);
	DRX3D_SHARED_API i32 b3CreateSensorEnable6DofJointForceTorqueSensor(b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, i32 enable);
	///b3CreateSensorEnableIMUForLink is not implemented yet.
	///For now, if the IMU is located in the root link, use the root world transform to mimic an IMU.
	DRX3D_SHARED_API i32 b3CreateSensorEnableIMUForLink(b3SharedMemoryCommandHandle commandHandle, i32 linkIndex, i32 enable);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestActualStateCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestActualStateCommandInit2(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId);

	DRX3D_SHARED_API i32 b3RequestActualStateCommandComputeLinkVelocity(b3SharedMemoryCommandHandle commandHandle, i32 computeLinkVelocity);
	DRX3D_SHARED_API i32 b3RequestActualStateCommandComputeForwardKinematics(b3SharedMemoryCommandHandle commandHandle, i32 computeForwardKinematics);

	DRX3D_SHARED_API i32 b3GetJointState(b3PhysicsClientHandle physClient, b3SharedMemoryStatusHandle statusHandle, i32 jointIndex, struct b3JointSensorState* state);
	DRX3D_SHARED_API i32 b3GetJointStateMultiDof(b3PhysicsClientHandle physClient, b3SharedMemoryStatusHandle statusHandle, i32 jointIndex, struct b3JointSensorState2* state);
	DRX3D_SHARED_API i32 b3GetLinkState(b3PhysicsClientHandle physClient, b3SharedMemoryStatusHandle statusHandle, i32 linkIndex, struct b3LinkState* state);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3PickBody(b3PhysicsClientHandle physClient, double rayFromWorldX,
														 double rayFromWorldY, double rayFromWorldZ,
														 double rayToWorldX, double rayToWorldY, double rayToWorldZ);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3MovePickedBody(b3PhysicsClientHandle physClient, double rayFromWorldX,
															   double rayFromWorldY, double rayFromWorldZ,
															   double rayToWorldX, double rayToWorldY,
															   double rayToWorldZ);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RemovePickingConstraint(b3PhysicsClientHandle physClient);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateRaycastCommandInit(b3PhysicsClientHandle physClient, double rayFromWorldX,
																		 double rayFromWorldY, double rayFromWorldZ,
																		 double rayToWorldX, double rayToWorldY, double rayToWorldZ);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateRaycastBatchCommandInit(b3PhysicsClientHandle physClient);
	// Sets the number of threads to use to compute the ray intersections for the batch. Specify 0 to let drx3D decide, 1 (default) for single core execution, 2 or more to select the number of threads to use.
	DRX3D_SHARED_API void b3RaycastBatchSetNumThreads(b3SharedMemoryCommandHandle commandHandle, i32 numThreads);
	//max num rays for b3RaycastBatchAddRay is MAX_RAY_INTERSECTION_BATCH_SIZE
	DRX3D_SHARED_API void b3RaycastBatchAddRay(b3SharedMemoryCommandHandle commandHandle, const double rayFromWorld[/*3*/], const double rayToWorld[/*3*/]);
	//max num rays for b3RaycastBatchAddRays is MAX_RAY_INTERSECTION_BATCH_SIZE_STREAMING
	DRX3D_SHARED_API void b3RaycastBatchAddRays(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double* rayFromWorld, const double* rayToWorld, i32 numRays);
	DRX3D_SHARED_API void b3RaycastBatchSetParentObject(b3SharedMemoryCommandHandle commandHandle, i32 parentObjectUniqueId, i32 parentLinkIndex);
	DRX3D_SHARED_API void b3RaycastBatchSetReportHitNumber(b3SharedMemoryCommandHandle commandHandle, i32 reportHitNumber);
	DRX3D_SHARED_API void b3RaycastBatchSetCollisionFilterMask(b3SharedMemoryCommandHandle commandHandle, i32 collisionFilterMask);
	DRX3D_SHARED_API void b3RaycastBatchSetFractionEpsilon(b3SharedMemoryCommandHandle commandHandle, double fractionEpsilon);
	
	DRX3D_SHARED_API void b3GetRaycastInformation(b3PhysicsClientHandle physClient, struct b3RaycastInformation* raycastInfo);

	/// Apply external force at the body (or link) center of mass, in world space/Cartesian coordinates.
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3ApplyExternalForceCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API void b3ApplyExternalForce(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkId, const double force[/*3*/], const double position[/*3*/], i32 flag);
	DRX3D_SHARED_API void b3ApplyExternalTorque(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkId, const double torque[/*3*/], i32 flag);

	///experiments of robots interacting with non-rigid objects (such as SoftBody)
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadSoftBodyCommandInit(b3PhysicsClientHandle physClient, tukk fileName);
	DRX3D_SHARED_API i32 b3LoadSoftBodySetScale(b3SharedMemoryCommandHandle commandHandle, double scale);
	DRX3D_SHARED_API i32 b3LoadSoftBodySetMass(b3SharedMemoryCommandHandle commandHandle, double mass);
	DRX3D_SHARED_API i32 b3LoadSoftBodySetCollisionMargin(b3SharedMemoryCommandHandle commandHandle, double collisionMargin);
	DRX3D_SHARED_API i32 b3LoadSoftBodySetStartPosition(b3SharedMemoryCommandHandle commandHandle, double startPosX, double startPosY, double startPosZ);
	DRX3D_SHARED_API i32 b3LoadSoftBodySetStartOrientation(b3SharedMemoryCommandHandle commandHandle, double startOrnX, double startOrnY, double startOrnZ, double startOrnW);
    DRX3D_SHARED_API i32 b3LoadSoftBodyUpdateSimMesh(b3SharedMemoryCommandHandle commandHandle, tukk filename);
	DRX3D_SHARED_API i32 b3LoadSoftBodyAddCorotatedForce(b3SharedMemoryCommandHandle commandHandle, double corotatedMu, double corotatedLambda);
    DRX3D_SHARED_API i32 b3LoadSoftBodyAddCorotatedForce(b3SharedMemoryCommandHandle commandHandle, double corotatedMu, double corotatedLambda);
	DRX3D_SHARED_API i32 b3LoadSoftBodyAddNeoHookeanForce(b3SharedMemoryCommandHandle commandHandle, double NeoHookeanMu, double NeoHookeanLambda, double NeoHookeanDamping);
	DRX3D_SHARED_API i32 b3LoadSoftBodyAddMassSpringForce(b3SharedMemoryCommandHandle commandHandle, double springElasticStiffness , double springDampingStiffness);
	DRX3D_SHARED_API i32 b3LoadSoftBodyAddGravityForce(b3SharedMemoryCommandHandle commandHandle, double gravityX, double gravityY, double gravityZ);
	DRX3D_SHARED_API i32 b3LoadSoftBodySetCollisionHardness(b3SharedMemoryCommandHandle commandHandle, double collisionHardness);
	DRX3D_SHARED_API i32 b3LoadSoftBodySetSelfCollision(b3SharedMemoryCommandHandle commandHandle, i32 useSelfCollision);
	DRX3D_SHARED_API i32 b3LoadSoftBodySetRepulsionStiffness(b3SharedMemoryCommandHandle commandHandle, double stiffness);
	DRX3D_SHARED_API i32 b3LoadSoftBodyUseFaceContact(b3SharedMemoryCommandHandle commandHandle, i32 useFaceContact);
	DRX3D_SHARED_API i32 b3LoadSoftBodySetFrictionCoefficient(b3SharedMemoryCommandHandle commandHandle, double frictionCoefficient);
	DRX3D_SHARED_API i32 b3LoadSoftBodyUseBendingSprings(b3SharedMemoryCommandHandle commandHandle, i32 useBendingSprings, double bendingStiffness);
    DRX3D_SHARED_API i32 b3LoadSoftBodyUseAllDirectionDampingSprings(b3SharedMemoryCommandHandle commandHandle, i32 useAllDirectionDamping);
	
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitCreateSoftBodyAnchorConstraintCommand(b3PhysicsClientHandle physClient, i32 softBodyUniqueId, i32 nodeIndex, i32 bodyUniqueId, i32 linkIndex, const double bodyFramePosition[3]);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestVREventsCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API void b3VREventsSetDeviceTypeFilter(b3SharedMemoryCommandHandle commandHandle, i32 deviceTypeFilter);

	DRX3D_SHARED_API void b3GetVREventsData(b3PhysicsClientHandle physClient, struct b3VREventsData* vrEventsData);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SetVRCameraStateCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API i32 b3SetVRCameraRootPosition(b3SharedMemoryCommandHandle commandHandle, const double rootPos[/*3*/]);
	DRX3D_SHARED_API i32 b3SetVRCameraRootOrientation(b3SharedMemoryCommandHandle commandHandle, const double rootOrn[/*4*/]);
	DRX3D_SHARED_API i32 b3SetVRCameraTrackingObject(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId);
	DRX3D_SHARED_API i32 b3SetVRCameraTrackingObjectFlag(b3SharedMemoryCommandHandle commandHandle, i32 flag);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestKeyboardEventsCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestKeyboardEventsCommandInit2(b3SharedMemoryCommandHandle commandHandle);
	DRX3D_SHARED_API void b3GetKeyboardEventsData(b3PhysicsClientHandle physClient, struct b3KeyboardEventsData* keyboardEventsData);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestMouseEventsCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API void b3GetMouseEventsData(b3PhysicsClientHandle physClient, struct b3MouseEventsData* mouseEventsData);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3StateLoggingCommandInit(b3PhysicsClientHandle physClient);
	DRX3D_SHARED_API i32 b3StateLoggingStart(b3SharedMemoryCommandHandle commandHandle, i32 loggingType, tukk fileName);
	DRX3D_SHARED_API i32 b3StateLoggingAddLoggingObjectUniqueId(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId);
	DRX3D_SHARED_API i32 b3StateLoggingSetMaxLogDof(b3SharedMemoryCommandHandle commandHandle, i32 maxLogDof);
	DRX3D_SHARED_API i32 b3StateLoggingSetLinkIndexA(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexA);
	DRX3D_SHARED_API i32 b3StateLoggingSetLinkIndexB(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexB);
	DRX3D_SHARED_API i32 b3StateLoggingSetBodyAUniqueId(b3SharedMemoryCommandHandle commandHandle, i32 bodyAUniqueId);
	DRX3D_SHARED_API i32 b3StateLoggingSetBodyBUniqueId(b3SharedMemoryCommandHandle commandHandle, i32 bodyBUniqueId);
	DRX3D_SHARED_API i32 b3StateLoggingSetDeviceTypeFilter(b3SharedMemoryCommandHandle commandHandle, i32 deviceTypeFilter);
	DRX3D_SHARED_API i32 b3StateLoggingSetLogFlags(b3SharedMemoryCommandHandle commandHandle, i32 logFlags);

	DRX3D_SHARED_API i32 b3GetStatusLoggingUniqueId(b3SharedMemoryStatusHandle statusHandle);
	DRX3D_SHARED_API i32 b3StateLoggingStop(b3SharedMemoryCommandHandle commandHandle, i32 loggingUid);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3ProfileTimingCommandInit(b3PhysicsClientHandle physClient, tukk name);
	DRX3D_SHARED_API void b3SetProfileTimingDuractionInMicroSeconds(b3SharedMemoryCommandHandle commandHandle, i32 duration);
	DRX3D_SHARED_API void b3SetProfileTimingType(b3SharedMemoryCommandHandle commandHandle, i32 type);

	DRX3D_SHARED_API void b3PushProfileTiming(b3PhysicsClientHandle physClient, tukk timingName);
	DRX3D_SHARED_API void b3PopProfileTiming(b3PhysicsClientHandle physClient);

	DRX3D_SHARED_API void b3SetTimeOut(b3PhysicsClientHandle physClient, double timeOutInSeconds);
	DRX3D_SHARED_API double b3GetTimeOut(b3PhysicsClientHandle physClient);

	DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SetAdditionalSearchPath(b3PhysicsClientHandle physClient, tukk path);

	DRX3D_SHARED_API void b3MultiplyTransforms(const double posA[/*3*/], const double ornA[/*4*/], const double posB[/*3*/], const double ornB[/*4*/], double outPos[/*3*/], double outOrn[/*4*/]);
	DRX3D_SHARED_API void b3InvertTransform(const double pos[/*3*/], const double orn[/*4*/], double outPos[/*3*/], double outOrn[/*4*/]);
	DRX3D_SHARED_API void b3QuatSlerp(const double startQuat[/*4*/], const double endQuat[/*4*/], double interpolationFraction, double outOrn[/*4*/]);
	DRX3D_SHARED_API void b3GetQuaternionFromAxisAngle(const double axis[/*3*/], double angle, double outQuat[/*4*/]);
	DRX3D_SHARED_API void b3GetAxisAngleFromQuaternion(const double quat[/*4*/], double axis[/*3*/], double* angle);
	DRX3D_SHARED_API void b3GetQuaternionDifference(const double startQuat[/*4*/], const double endQuat[/*4*/], double outOrn[/*4*/]);
	DRX3D_SHARED_API void b3GetAxisDifferenceQuaternion(const double startQuat[/*4*/], const double endQuat[/*4*/], double axisOut[/*3*/]);
	DRX3D_SHARED_API void b3CalculateVelocityQuaternion(const double startQuat[/*4*/], const double endQuat[/*4*/], double deltaTime, double angVelOut[/*3*/]);
	DRX3D_SHARED_API void b3RotateVector(const double quat[/*4*/], const double vec[/*3*/], double vecOut[/*3*/]);

#ifdef DRX3D_ENABLE_VHACD
	DRX3D_SHARED_API void b3VHACD(tukk fileNameInput, tukk fileNameOutput, tukk fileNameLogging,
		double concavity, double alpha, double beta, double gamma, double minVolumePerCH, i32 resolution,
		i32 maxNumVerticesPerCH, i32 depth, i32 planeDownsampling, i32 convexhullDownsampling,
		i32 pca, i32 mode, i32 convexhullApproximation);


#endif

#ifdef __cplusplus
}
#endif

#endif  //PHYSICS_CLIENT_C_API_H
