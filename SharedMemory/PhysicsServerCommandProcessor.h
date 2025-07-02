#ifndef PHYSICS_SERVER_COMMAND_PROCESSOR_H
#define PHYSICS_SERVER_COMMAND_PROCESSOR_H

#include <drx3D/Maths/Linear/HashMap.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/SharedMemory/PhysicsCommandProcessorInterface.h>
#include <drx3D/Importers/URDF/UrdfParser.h>

struct SharedMemLines
{
	Vec3 m_from;
	Vec3 m_to;
	Vec3 m_color;
};

///todo: naming. Perhaps PhysicsSdkCommandprocessor?
class PhysicsServerCommandProcessor : public CommandProcessorInterface
{
	struct PhysicsServerCommandProcessorInternalData* m_data;

	void resetSimulation(i32 flags = 0);
	void createThreadPool();

	class DeformableMultiBodyDynamicsWorld* getDeformableWorld();
	class SoftMultiBodyDynamicsWorld* getSoftWorld();

protected:
	bool processStateLoggingCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestCameraImageCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSaveWorldCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCreateCollisionShapeCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCreateVisualShapeCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestMeshDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processResetMeshDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCustomCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processUserDebugDrawCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSetVRCameraStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestVREventsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestMouseEventsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestKeyboardEventsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestRaycastIntersectionsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestDebugLinesCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSyncBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSendDesiredStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestActualStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestContactpointInformationCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestDeformableContactpointHelper(const struct SharedMemoryCommand& clientCmd);
    bool processRequestDeformableDeformableContactpointHelper(const struct SharedMemoryCommand& clientCmd);
	bool processRequestBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processLoadSDFCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCreateMultiBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCreateMultiBodyCommandSingle(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	bool processLoadURDFCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processLoadSoftBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCreateSensorCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processProfileTimingCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestCollisionInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processForwardDynamicsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool performCollisionDetectionCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestInternalDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processChangeDynamicsInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSetAdditionalSearchPathCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processGetDynamicsInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestPhysicsSimulationParametersCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSendPhysicsParametersCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processInitPoseCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processResetSimulationCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCreateRigidBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processPickBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processMovePickedBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRemovePickingConstraintCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestAabbOverlapCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestOpenGLVisualizeCameraCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processConfigureOpenGLVisualizerCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processdrx3d_inverseCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCalculateJacobianCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCalculateMassMatrixCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processApplyExternalForceCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRemoveBodyCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCreateUserConstraintCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCalculateInverseKinematicsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCalculateInverseKinematicsCommand2(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestVisualShapeInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestCollisionShapeInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processUpdateVisualShapeCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processChangeTextureCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processLoadTextureCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processLoadBulletCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSaveBulletCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processLoadMJCFCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRestoreStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSaveStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRemoveStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSyncUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processAddUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRemoveUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCollisionFilterCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	i32 extractCollisionShapes(const class CollisionShape* colShape, const class Transform2& transform, struct b3CollisionShapeData* collisionShapeBuffer, i32 maxCollisionShapes);

	bool loadSdf(tukk fileName, tuk bufferServerToClient, i32 bufferSizeInBytes, bool useMultiBody, i32 flags, Scalar globalScaling);

	bool loadUrdf(tukk fileName, const class Vec3& pos, const class Quat& orn,
				  bool useMultiBody, bool useFixedBase, i32* bodyUniqueIdPtr, tuk bufferServerToClient, i32 bufferSizeInBytes, i32 flags, Scalar globalScaling);

	bool loadMjcf(tukk fileName, tuk bufferServerToClient, i32 bufferSizeInBytes, bool useMultiBody, i32 flags);

	bool processImportedObjects(tukk fileName, tuk bufferServerToClient, i32 bufferSizeInBytes, bool useMultiBody, i32 flags, class URDFImporterInterface& u2b);
	bool processDeformable(const UrdfDeformable& deformable, const Vec3& pos, const Quat& orn, i32* bodyUniqueId, tuk bufferServerToClient, i32 bufferSizeInBytes, Scalar scale, bool useSelfCollision);
	bool processReducedDeformable(const UrdfReducedDeformable& deformable, const Vec3& pos, const Quat& orn, i32* bodyUniqueId, tuk bufferServerToClient, i32 bufferSizeInBytes, Scalar scale, bool useSelfCollision);

	bool supportsJointMotor(class MultiBody* body, i32 linkIndex);

	i32 createBodyInfoStream(i32 bodyUniqueId, tuk bufferServerToClient, i32 bufferSizeInBytes);
	void deleteCacheddrx3d_inverseBodies();
	void deleteCachedInverseKinematicsBodies();
	void deleteStateLoggers();

public:
	PhysicsServerCommandProcessor();
	virtual ~PhysicsServerCommandProcessor();

	void createJointMotors(class MultiBody* body);

	virtual void createEmptyDynamicsWorld(i32 flags = 0);
	virtual void deleteDynamicsWorld();

	virtual bool connect()
	{
		return true;
	};

	virtual void disconnect() {}

	virtual bool isConnected() const
	{
		return true;
	}

	virtual bool processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	virtual bool receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
	{
		return false;
	};

	virtual void renderScene(i32 renderFlags);
	virtual void physicsDebugDraw(i32 debugDrawFlags);
	virtual void setGuiHelper(struct GUIHelperInterface* guiHelper);
	virtual void syncPhysicsToGraphics();
	virtual void syncPhysicsToGraphics2();

	//@todo(erwincoumans) Should we have shared memory commands for picking objects?
	///The pickBody method will try to pick the first body along a ray, return true if succeeds, false otherwise
	virtual bool pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld);
	virtual bool movePickedBody(const Vec3& rayFromWorld, const Vec3& rayToWorld);
	virtual void removePickingConstraint();

	//logging /playback the shared memory commands
	virtual void enableCommandLogging(bool enable, tukk fileName);
	virtual void replayFromLogFile(tukk fileName);
	virtual void replayLogCommand(tuk bufferServerToClient, i32 bufferSizeInBytes);

	//logging of object states (position etc)
	virtual void reportNotifications();
	virtual void processClientCommands();
	void tickPlugins(Scalar timeStep, bool isPreTick);
	void logObjectStates(Scalar timeStep);
	void processCollisionForces(Scalar timeStep);

	virtual void stepSimulationRealTime(double dtInSec, const struct b3VRControllerEvent* vrControllerEvents, i32 numVRControllerEvents, const struct b3KeyboardEvent* keyEvents, i32 numKeyEvents, const struct b3MouseEvent* mouseEvents, i32 numMouseEvents);

	virtual void enableRealTimeSimulation(bool enableRealTimeSim);
	virtual bool isRealTimeSimulationEnabled() const;

	void applyJointDamping(i32 bodyUniqueId);

	virtual void setTimeOut(double timeOutInSeconds);

	virtual const Vec3& getVRTeleportPosition() const;
	virtual void setVRTeleportPosition(const Vec3& vrTeleportPos);

	virtual const Quat& getVRTeleportOrientation() const;
	virtual void setVRTeleportOrientation(const Quat& vrTeleportOrn);

private:
	void addBodyChangedNotifications();
	i32 addUserData(i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key, tukk valueBytes, i32 valueLength, i32 valueType);
	void addUserData(const HashMap<HashString, STxt>& user_data_entries, i32 bodyUniqueId, i32 linkIndex = -1, i32 visualShapeIndex = -1);
};

#endif  //PHYSICS_SERVER_COMMAND_PROCESSOR_H
