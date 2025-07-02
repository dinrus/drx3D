#ifndef DRX3D_PHYSICS_CLIENT_API_H
#define DRX3D_PHYSICS_CLIENT_API_H

//#include "SharedMemoryCommands.h"
#include <drx3D/Maths/Linear/Vec3.h>

class PhysicsClient
{
public:
	virtual ~PhysicsClient();

	// return true if connection succesfull, can also check 'isConnected'
	virtual bool connect() = 0;

	virtual void disconnectSharedMemory() = 0;

	virtual bool isConnected() const = 0;

	// return non-null if there is a status, nullptr otherwise
	virtual const struct SharedMemoryStatus* processServerStatus() = 0;

	virtual struct SharedMemoryCommand* getAvailableSharedMemoryCommand() = 0;

	virtual bool canSubmitCommand() const = 0;

	virtual bool submitClientCommand(const struct SharedMemoryCommand& command) = 0;

	virtual i32 getNumBodies() const = 0;

	virtual i32 getBodyUniqueId(i32 serialIndex) const = 0;

	virtual bool getBodyInfo(i32 bodyUniqueId, struct b3BodyInfo& info) const = 0;

	virtual i32 getNumJoints(i32 bodyUniqueId) const = 0;

	virtual i32 getNumDofs(i32 bodyUniqueId) const = 0;

	virtual bool getJointInfo(i32 bodyUniqueId, i32 jointIndex, struct b3JointInfo& info) const = 0;

	virtual i32 getNumUserConstraints() const = 0;

	virtual i32 getUserConstraintInfo(i32 constraintUniqueId, struct b3UserConstraint& info) const = 0;

	virtual i32 getUserConstraintId(i32 serialIndex) const = 0;

	virtual void setSharedMemoryKey(i32 key) = 0;

	virtual void uploadBulletFileToSharedMemory(tukk data, i32 len) = 0;

	virtual void uploadRaysToSharedMemory(struct SharedMemoryCommand& command, const double* rayFromWorldArray, const double* rayToWorldArray, i32 numRays) = 0;

	virtual i32 getNumDebugLines() const = 0;

	virtual const float* getDebugLinesFrom() const = 0;
	virtual const float* getDebugLinesTo() const = 0;
	virtual const float* getDebugLinesColor() const = 0;

	virtual void getCachedCameraImage(struct b3CameraImageData* cameraData) = 0;

	virtual void getCachedContactPointInformation(struct b3ContactInformation* contactPointData) = 0;

	virtual void getCachedOverlappingObjects(struct b3AABBOverlapData* overlappingObjects) = 0;

	virtual void getCachedVisualShapeInformation(struct b3VisualShapeInformation* visualShapesInfo) = 0;

	virtual void getCachedCollisionShapeInformation(struct b3CollisionShapeInformation* collisionShapesInfo) = 0;

	virtual void getCachedMeshData(struct b3MeshData* meshData) = 0;

	virtual void getCachedVREvents(struct b3VREventsData* vrEventsData) = 0;

	virtual void getCachedKeyboardEvents(struct b3KeyboardEventsData* keyboardEventsData) = 0;

	virtual void getCachedMouseEvents(struct b3MouseEventsData* mouseEventsData) = 0;

	virtual void getCachedRaycastHits(struct b3RaycastInformation* raycastHits) = 0;

	virtual void getCachedMassMatrix(i32 dofCountCheck, double* massMatrix) = 0;

	virtual bool getCachedReturnData(struct b3UserDataValue* returnData) = 0;

	virtual void setTimeOut(double timeOutInSeconds) = 0;
	virtual double getTimeOut() const = 0;

	virtual bool getCachedUserData(i32 userDataId, struct b3UserDataValue& valueOut) const = 0;
	virtual i32 getCachedUserDataId(i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key) const = 0;
	virtual i32 getNumUserData(i32 bodyUniqueId) const = 0;
	virtual void getUserDataInfo(i32 bodyUniqueId, i32 userDataIndex, tukk* keyOut, i32* userDataIdOut, i32* linkIndexOut, i32* visualShapeIndexOut) const = 0;

	virtual void pushProfileTiming(tukk timingName) = 0;
	virtual void popProfileTiming() = 0;
};

#endif  // DRX3D_PHYSICS_CLIENT_API_H
