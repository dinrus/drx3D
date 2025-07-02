#ifndef MUJOCO_PHYSICS_CLIENT_H
#define MUJOCO_PHYSICS_CLIENT_H

#include "../PhysicsClient.h"

///PhysicsDirect executes the commands directly, without transporting them or having a separate server executing commands
class MuJoCoPhysicsClient : public PhysicsClient
{
protected:
	struct MuJoCoPhysicsDirectInternalData* m_data;

	bool processDebugLines(const struct SharedMemoryCommand& orgCommand);

	bool processCamera(const struct SharedMemoryCommand& orgCommand);

	bool processContactPointData(const struct SharedMemoryCommand& orgCommand);

	bool processOverlappingObjects(const struct SharedMemoryCommand& orgCommand);

	bool processVisualShapeData(const struct SharedMemoryCommand& orgCommand);

	void processBodyJointInfo(i32 bodyUniqueId, const struct SharedMemoryStatus& serverCmd);

	void processAddUserData(const struct SharedMemoryStatus& serverCmd);

	void postProcessStatus(const struct SharedMemoryStatus& serverCmd);

	void resetData();

	void removeCachedBody(i32 bodyUniqueId);

public:
	MuJoCoPhysicsClient(class PhysicsCommandProcessorInterface* physSdk, bool passSdkOwnership);

	virtual ~MuJoCoPhysicsClient();

	// return true if connection succesfull, can also check 'isConnected'
	//it is OK to pass a null pointer for the gui helper
	virtual bool connect();

	////todo: rename to 'disconnect'
	virtual void disconnectSharedMemory();

	virtual bool isConnected() const;

	// return non-null if there is a status, nullptr otherwise
	virtual const SharedMemoryStatus* processServerStatus();

	virtual SharedMemoryCommand* getAvailableSharedMemoryCommand();

	virtual bool canSubmitCommand() const;

	virtual bool submitClientCommand(const struct SharedMemoryCommand& command);

	virtual i32 getNumBodies() const;

	virtual i32 getBodyUniqueId(i32 serialIndex) const;

	virtual bool getBodyInfo(i32 bodyUniqueId, struct b3BodyInfo& info) const;

	virtual i32 getNumJoints(i32 bodyIndex) const;

	virtual bool getJointInfo(i32 bodyIndex, i32 jointIndex, struct b3JointInfo& info) const;

	virtual i32 getNumUserConstraints() const;

	virtual i32 getUserConstraintInfo(i32 constraintUniqueId, struct b3UserConstraint& info) const;

	virtual i32 getUserConstraintId(i32 serialIndex) const;

	///todo: move this out of the
	virtual void setSharedMemoryKey(i32 key);

	void uploadBulletFileToSharedMemory(tukk data, i32 len);

	virtual void uploadRaysToSharedMemory(struct SharedMemoryCommand& command, const double* rayFromWorldArray, const double* rayToWorldArray, i32 numRays);

	virtual i32 getNumDebugLines() const;

	virtual const float* getDebugLinesFrom() const;
	virtual const float* getDebugLinesTo() const;
	virtual const float* getDebugLinesColor() const;

	virtual void getCachedCameraImage(b3CameraImageData* cameraData);

	virtual void getCachedContactPointInformation(struct b3ContactInformation* contactPointData);

	virtual void getCachedOverlappingObjects(struct b3AABBOverlapData* overlappingObjects);

	virtual void getCachedVisualShapeInformation(struct b3VisualShapeInformation* visualShapesInfo);

	virtual void getCachedCollisionShapeInformation(struct b3CollisionShapeInformation* collisionShapesInfo);

	virtual void getCachedVREvents(struct b3VREventsData* vrEventsData);

	virtual void getCachedKeyboardEvents(struct b3KeyboardEventsData* keyboardEventsData);

	virtual void getCachedMouseEvents(struct b3MouseEventsData* mouseEventsData);

	virtual void getCachedRaycastHits(struct b3RaycastInformation* raycastHits);

	virtual void getCachedMassMatrix(i32 dofCountCheck, double* massMatrix);

	//the following APIs are for internal use for visualization:
	virtual bool connect(struct GUIHelperInterface* guiHelper);
	virtual void renderScene();
	virtual void debugDraw(i32 debugDrawMode);

	virtual void setTimeOut(double timeOutInSeconds);
	virtual double getTimeOut() const;

	virtual bool getCachedUserData(i32 bodyUniqueId, i32 linkIndex, i32 userDataId, struct b3UserDataValue& valueOut) const;
	virtual i32 getCachedUserDataId(i32 bodyUniqueId, i32 linkIndex, tukk key) const;
	virtual i32 getNumUserData(i32 bodyUniqueId, i32 linkIndex) const;
	virtual void getUserDataInfo(i32 bodyUniqueId, i32 linkIndex, i32 userDataIndex, tukk* keyOut, i32* userDataIdOut) const;

	virtual void pushProfileTiming(tukk timingName);
	virtual void popProfileTiming();
};

#endif  //MUJOCO_PHYSICS_CLIENT_H
