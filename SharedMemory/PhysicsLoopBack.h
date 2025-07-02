#ifndef PHYSICS_LOOP_BACK_H
#define PHYSICS_LOOP_BACK_H

//#include "SharedMemoryCommands.h"

#include <drx3D/SharedMemory/PhysicsClient.h>
#include <drx3D/Maths/Linear/Vec3.h>

///todo: the PhysicsClient API was designed with shared memory in mind,
///now it become more general we need to move out the shared memory specifics away
///for example naming [disconnectSharedMemory -> disconnect] [ move setSharedMemoryKey to shared memory specific subclass ]

class PhysicsLoopBack : public PhysicsClient
{
	struct PhysicsLoopBackInternalData* m_data;

public:
	PhysicsLoopBack();

	virtual ~PhysicsLoopBack();

	// return true if connection succesfull, can also check 'isConnected'
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

	virtual i32 getNumJoints(i32 bodyUniqueId) const;

	virtual i32 getNumDofs(i32 bodyUniqueId) const;

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
	virtual void getCachedCameraImage(struct b3CameraImageData* cameraData);

	virtual void getCachedContactPointInformation(struct b3ContactInformation* contactPointData);

	virtual void getCachedOverlappingObjects(struct b3AABBOverlapData* overlappingObjects);

	virtual void getCachedVisualShapeInformation(struct b3VisualShapeInformation* visualShapesInfo);

	virtual void getCachedCollisionShapeInformation(struct b3CollisionShapeInformation* collisionShapesInfo);

	virtual void getCachedMeshData(struct b3MeshData* meshData);

	virtual void getCachedVREvents(struct b3VREventsData* vrEventsData);

	virtual void getCachedKeyboardEvents(struct b3KeyboardEventsData* keyboardEventsData);

	virtual void getCachedMouseEvents(struct b3MouseEventsData* mouseEventsData);

	virtual void getCachedRaycastHits(struct b3RaycastInformation* raycastHits);

	virtual void getCachedMassMatrix(i32 dofCountCheck, double* massMatrix);

	virtual bool getCachedReturnData(struct b3UserDataValue* returnData);

	virtual void setTimeOut(double timeOutInSeconds);
	virtual double getTimeOut() const;

	virtual bool getCachedUserData(i32 userDataId, struct b3UserDataValue& valueOut) const;
	virtual i32 getCachedUserDataId(i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key) const;
	virtual i32 getNumUserData(i32 bodyUniqueId) const;
	virtual void getUserDataInfo(i32 bodyUniqueId, i32 userDataIndex, tukk* keyOut, i32* userDataIdOut, i32* linkIndexOut, i32* visualShapeIndexOut) const;

	virtual void pushProfileTiming(tukk timingName);
	virtual void popProfileTiming();
};

#endif  //PHYSICS_LOOP_BACK_H
