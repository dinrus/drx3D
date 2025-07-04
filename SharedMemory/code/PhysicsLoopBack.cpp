#include <drx3D/SharedMemory/PhysicsLoopBack.h>
#include <drx3D/SharedMemory/PhysicsServerSharedMemory.h>
#include <drx3D/SharedMemory/PhysicsClientSharedMemory.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/SharedMemory/PhysicsServerCommandProcessor.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>

struct PhysicsLoopBackInternalData
{
	CommandProcessorInterface* m_commandProcessor;
	PhysicsClientSharedMemory* m_physicsClient;
	PhysicsServerSharedMemory* m_physicsServer;
	DummyGUIHelper m_noGfx;

	PhysicsLoopBackInternalData()
		: m_commandProcessor(0),
		  m_physicsClient(0),
		  m_physicsServer(0)
	{
	}
};

struct Bullet2CommandProcessorCreation2 : public CommandProcessorCreationInterface
{
	virtual class CommandProcessorInterface* createCommandProcessor()
	{
		PhysicsServerCommandProcessor* proc = new PhysicsServerCommandProcessor;
		return proc;
	}

	virtual void deleteCommandProcessor(CommandProcessorInterface* proc)
	{
		delete proc;
	}
};

static Bullet2CommandProcessorCreation2 sB2Proc;

PhysicsLoopBack::PhysicsLoopBack()
{
	m_data = new PhysicsLoopBackInternalData;
	m_data->m_physicsServer = new PhysicsServerSharedMemory(&sB2Proc, 0, 0);
	m_data->m_physicsClient = new PhysicsClientSharedMemory();
}

PhysicsLoopBack::~PhysicsLoopBack()
{
	delete m_data->m_physicsClient;
	delete m_data->m_physicsServer;
	delete m_data->m_commandProcessor;
	delete m_data;
}

// return true if connection succesfull, can also check 'isConnected'
bool PhysicsLoopBack::connect()
{
	m_data->m_physicsServer->connectSharedMemory(&m_data->m_noGfx);
	m_data->m_physicsClient->connect();
	return m_data->m_physicsClient->isConnected();
}

////todo: rename to 'disconnect'
void PhysicsLoopBack::disconnectSharedMemory()
{
	m_data->m_physicsClient->disconnectSharedMemory();
	m_data->m_physicsServer->disconnectSharedMemory(true);
}

bool PhysicsLoopBack::isConnected() const
{
	return m_data->m_physicsClient->isConnected();
}

// return non-null if there is a status, nullptr otherwise
const SharedMemoryStatus* PhysicsLoopBack::processServerStatus()
{
	m_data->m_physicsServer->processClientCommands();
	return m_data->m_physicsClient->processServerStatus();
}

SharedMemoryCommand* PhysicsLoopBack::getAvailableSharedMemoryCommand()
{
	return m_data->m_physicsClient->getAvailableSharedMemoryCommand();
}

bool PhysicsLoopBack::canSubmitCommand() const
{
	return m_data->m_physicsClient->canSubmitCommand();
}

bool PhysicsLoopBack::submitClientCommand(const struct SharedMemoryCommand& command)
{
	return m_data->m_physicsClient->submitClientCommand(command);
}

i32 PhysicsLoopBack::getNumBodies() const
{
	return m_data->m_physicsClient->getNumBodies();
}

i32 PhysicsLoopBack::getBodyUniqueId(i32 serialIndex) const
{
	return m_data->m_physicsClient->getBodyUniqueId(serialIndex);
}

bool PhysicsLoopBack::getBodyInfo(i32 bodyUniqueId, struct b3BodyInfo& info) const
{
	return m_data->m_physicsClient->getBodyInfo(bodyUniqueId, info);
}

i32 PhysicsLoopBack::getNumJoints(i32 bodyUniqueId) const
{
	return m_data->m_physicsClient->getNumJoints(bodyUniqueId);
}

i32 PhysicsLoopBack::getNumDofs(i32 bodyUniqueId) const
{
        return m_data->m_physicsClient->getNumDofs(bodyUniqueId);
}

bool PhysicsLoopBack::getJointInfo(i32 bodyIndex, i32 jointIndex, struct b3JointInfo& info) const
{
	return m_data->m_physicsClient->getJointInfo(bodyIndex, jointIndex, info);
}

i32 PhysicsLoopBack::getNumUserConstraints() const
{
	return m_data->m_physicsClient->getNumUserConstraints();
}
i32 PhysicsLoopBack::getUserConstraintInfo(i32 constraintUniqueId, struct b3UserConstraint& info) const
{
	return m_data->m_physicsClient->getUserConstraintInfo(constraintUniqueId, info);
}

i32 PhysicsLoopBack::getUserConstraintId(i32 serialIndex) const
{
	return m_data->m_physicsClient->getUserConstraintId(serialIndex);
}

///todo: move this out of the interface
void PhysicsLoopBack::setSharedMemoryKey(i32 key)
{
	m_data->m_physicsServer->setSharedMemoryKey(key);
	m_data->m_physicsClient->setSharedMemoryKey(key);
}

void PhysicsLoopBack::uploadBulletFileToSharedMemory(tukk data, i32 len)
{
	m_data->m_physicsClient->uploadBulletFileToSharedMemory(data, len);
}

void PhysicsLoopBack::uploadRaysToSharedMemory(struct SharedMemoryCommand& command, const double* rayFromWorldArray, const double* rayToWorldArray, i32 numRays)
{
	m_data->m_physicsClient->uploadRaysToSharedMemory(command, rayFromWorldArray, rayToWorldArray, numRays);
}

i32 PhysicsLoopBack::getNumDebugLines() const
{
	return m_data->m_physicsClient->getNumDebugLines();
}

const float* PhysicsLoopBack::getDebugLinesFrom() const
{
	return m_data->m_physicsClient->getDebugLinesFrom();
}

const float* PhysicsLoopBack::getDebugLinesTo() const
{
	return m_data->m_physicsClient->getDebugLinesTo();
}

const float* PhysicsLoopBack::getDebugLinesColor() const
{
	return m_data->m_physicsClient->getDebugLinesColor();
}

void PhysicsLoopBack::getCachedCameraImage(struct b3CameraImageData* cameraData)
{
	return m_data->m_physicsClient->getCachedCameraImage(cameraData);
}

void PhysicsLoopBack::getCachedMeshData(struct b3MeshData* meshData)
{
	return m_data->m_physicsClient->getCachedMeshData(meshData);
}

void PhysicsLoopBack::getCachedContactPointInformation(struct b3ContactInformation* contactPointData)
{
	return m_data->m_physicsClient->getCachedContactPointInformation(contactPointData);
}

void PhysicsLoopBack::getCachedVisualShapeInformation(struct b3VisualShapeInformation* visualShapesInfo)
{
	return m_data->m_physicsClient->getCachedVisualShapeInformation(visualShapesInfo);
}

void PhysicsLoopBack::getCachedCollisionShapeInformation(struct b3CollisionShapeInformation* collisionShapesInfo)
{
	return m_data->m_physicsClient->getCachedCollisionShapeInformation(collisionShapesInfo);
}

void PhysicsLoopBack::getCachedVREvents(struct b3VREventsData* vrEventsData)
{
	return m_data->m_physicsClient->getCachedVREvents(vrEventsData);
}

void PhysicsLoopBack::getCachedKeyboardEvents(struct b3KeyboardEventsData* keyboardEventsData)
{
	return m_data->m_physicsClient->getCachedKeyboardEvents(keyboardEventsData);
}

void PhysicsLoopBack::getCachedMouseEvents(struct b3MouseEventsData* mouseEventsData)
{
	return m_data->m_physicsClient->getCachedMouseEvents(mouseEventsData);
}

void PhysicsLoopBack::getCachedOverlappingObjects(struct b3AABBOverlapData* overlappingObjects)
{
	return m_data->m_physicsClient->getCachedOverlappingObjects(overlappingObjects);
}

void PhysicsLoopBack::getCachedRaycastHits(struct b3RaycastInformation* raycastHits)
{
	return m_data->m_physicsClient->getCachedRaycastHits(raycastHits);
}

void PhysicsLoopBack::getCachedMassMatrix(i32 dofCountCheck, double* massMatrix)
{
	m_data->m_physicsClient->getCachedMassMatrix(dofCountCheck, massMatrix);
}
bool PhysicsLoopBack::getCachedReturnData(struct b3UserDataValue* returnData)
{
	return m_data->m_physicsClient->getCachedReturnData(returnData);
}

void PhysicsLoopBack::setTimeOut(double timeOutInSeconds)
{
	m_data->m_physicsClient->setTimeOut(timeOutInSeconds);
}
double PhysicsLoopBack::getTimeOut() const
{
	return m_data->m_physicsClient->getTimeOut();
}

bool PhysicsLoopBack::getCachedUserData(i32 userDataId, struct b3UserDataValue& valueOut) const
{
	return m_data->m_physicsClient->getCachedUserData(userDataId, valueOut);
}

i32 PhysicsLoopBack::getCachedUserDataId(i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key) const
{
	return m_data->m_physicsClient->getCachedUserDataId(bodyUniqueId, linkIndex, visualShapeIndex, key);
}

i32 PhysicsLoopBack::getNumUserData(i32 bodyUniqueId) const
{
	return m_data->m_physicsClient->getNumUserData(bodyUniqueId);
}

void PhysicsLoopBack::getUserDataInfo(i32 bodyUniqueId, i32 userDataIndex, tukk* keyOut, i32* userDataIdOut, i32* linkIndexOut, i32* visualShapeIndexOut) const
{
	m_data->m_physicsClient->getUserDataInfo(bodyUniqueId, userDataIndex, keyOut, userDataIdOut, linkIndexOut, visualShapeIndexOut);
}

void PhysicsLoopBack::pushProfileTiming(tukk timingName)
{
	m_data->m_physicsClient->pushProfileTiming(timingName);
}
void PhysicsLoopBack::popProfileTiming()
{
	m_data->m_physicsClient->popProfileTiming();
}
