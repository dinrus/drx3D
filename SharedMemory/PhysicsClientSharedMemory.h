#ifndef DRX3D_PHYSICS_CLIENT_SHARED_MEMORY_API_H
#define DRX3D_PHYSICS_CLIENT_SHARED_MEMORY_API_H

#include <drx3D/SharedMemory/PhysicsClient.h>
#include <drx3D/Maths/Linear/Vec3.h>

class PhysicsClientSharedMemory : public PhysicsClient
{
	

protected:
	struct PhysicsClientSharedMemoryInternalData* m_data;
	virtual void setSharedMemoryInterface(class SharedMemoryInterface* sharedMem);
	void processBodyJointInfo(i32 bodyUniqueId, const struct SharedMemoryStatus& serverCmd);
	void resetData();
	void removeCachedBody(i32 bodyUniqueId);
	void clearCachedBodies();
	virtual void renderSceneInternal(){};

public:
	PhysicsClientSharedMemory();
	virtual ~PhysicsClientSharedMemory();

	// return true if connection succesfull, can also check 'isConnected'
	virtual bool connect();

	virtual void disconnectSharedMemory();

	virtual bool isConnected() const;

	// return non-null if there is a status, nullptr otherwise
	virtual const struct SharedMemoryStatus* processServerStatus();

	virtual struct SharedMemoryCommand* getAvailableSharedMemoryCommand();

	virtual bool canSubmitCommand() const;

	virtual bool submitClientCommand(const struct SharedMemoryCommand& command);

	virtual i32 getNumBodies() const;

	virtual i32 getBodyUniqueId(i32 serialIndex) const;

	virtual bool getBodyInfo(i32 bodyUniqueId, struct b3BodyInfo& info) const;

	virtual i32 getNumJoints(i32 bodyUniqueId) const;

	virtual i32 getNumDofs(i32 bodyUniqueId) const;

	virtual bool getJointInfo(i32 bodyUniqueId, i32 jointIndex, struct b3JointInfo& info) const;

	virtual i32 getNumUserConstraints() const;

	virtual i32 getUserConstraintInfo(i32 constraintUniqueId, struct b3UserConstraint& info) const;

	virtual i32 getUserConstraintId(i32 serialIndex) const;

	virtual void setSharedMemoryKey(i32 key);

	virtual void uploadBulletFileToSharedMemory(tukk data, i32 len);

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

	virtual bool getCachedReturnData(b3UserDataValue* returnData);

	virtual void setTimeOut(double timeOutInSeconds);
	virtual double getTimeOut() const;

	virtual bool getCachedUserData(i32 userDataId, struct b3UserDataValue& valueOut) const;
	virtual i32 getCachedUserDataId(i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key) const;
	virtual i32 getNumUserData(i32 bodyUniqueId) const;
	virtual void getUserDataInfo(i32 bodyUniqueId, i32 userDataIndex, tukk* keyOut, i32* userDataIdOut, i32* linkIndexOut, i32* visualShapeIndexOut) const;

	virtual void pushProfileTiming(tukk timingName);
	virtual void popProfileTiming();
};

#endif  // DRX3D_PHYSICS_CLIENT_API_H
