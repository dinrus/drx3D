#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Maths/Linear/Serializer.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>

CollisionObject2::CollisionObject2()
	: m_interpolationLinearVelocity(0.f, 0.f, 0.f),
	  m_interpolationAngularVelocity(0.f, 0.f, 0.f),
	  m_anisotropicFriction(1.f, 1.f, 1.f),
	  m_hasAnisotropicFriction(false),
	  m_contactProcessingThreshold(DRX3D_LARGE_FLOAT),
	  m_broadphaseHandle(0),
	  m_collisionShape(0),
	  m_extensionPointer(0),
	  m_rootCollisionShape(0),
	  m_collisionFlags(CollisionObject2::CF_STATIC_OBJECT),
	  m_islandTag1(-1),
	  m_companionId(-1),
	  m_worldArrayIndex(-1),
	  m_activationState1(1),
	  m_deactivationTime(Scalar(0.)),
	  m_friction(Scalar(0.5)),
	  m_restitution(Scalar(0.)),
	  m_rollingFriction(0.0f),
	  m_spinningFriction(0.f),
	  m_contactDamping(.1),
	  m_contactStiffness(DRX3D_LARGE_FLOAT),
	  m_internalType(CO_COLLISION_OBJECT),
	  m_userObjectPointer(0),
	  m_userIndex2(-1),
	  m_userIndex(-1),
	  m_userIndex3(-1),
	  m_hitFraction(Scalar(1.)),
	  m_ccdSweptSphereRadius(Scalar(0.)),
	  m_ccdMotionThreshold(Scalar(0.)),
	  m_checkCollideWith(false),
	  m_updateRevision(0)
{
	m_worldTransform.setIdentity();
	m_interpolationWorldTransform.setIdentity();
}

CollisionObject2::~CollisionObject2()
{
}

void CollisionObject2::setActivationState(i32 newState) const
{
	if ((m_activationState1 != DISABLE_DEACTIVATION) && (m_activationState1 != DISABLE_SIMULATION))
		m_activationState1 = newState;
}

void CollisionObject2::forceActivationState(i32 newState) const
{
	m_activationState1 = newState;
}

void CollisionObject2::activate(bool forceActivation) const
{
	if (forceActivation || !(m_collisionFlags & (CF_STATIC_OBJECT | CF_KINEMATIC_OBJECT)))
	{
		setActivationState(ACTIVE_TAG);
		m_deactivationTime = Scalar(0.);
	}
}

tukk CollisionObject2::serialize(uk dataBuffer, Serializer* serializer) const
{
	CollisionObject2Data* dataOut = (CollisionObject2Data*)dataBuffer;

	m_worldTransform.serialize(dataOut->m_worldTransform);
	m_interpolationWorldTransform.serialize(dataOut->m_interpolationWorldTransform);
	m_interpolationLinearVelocity.serialize(dataOut->m_interpolationLinearVelocity);
	m_interpolationAngularVelocity.serialize(dataOut->m_interpolationAngularVelocity);
	m_anisotropicFriction.serialize(dataOut->m_anisotropicFriction);
	dataOut->m_hasAnisotropicFriction = m_hasAnisotropicFriction;
	dataOut->m_contactProcessingThreshold = m_contactProcessingThreshold;
	dataOut->m_broadphaseHandle = 0;
	dataOut->m_collisionShape = serializer->getUniquePointer(m_collisionShape);
	dataOut->m_rootCollisionShape = 0;  //@todo
	dataOut->m_collisionFlags = m_collisionFlags;
	dataOut->m_islandTag1 = m_islandTag1;
	dataOut->m_companionId = m_companionId;
	dataOut->m_activationState1 = m_activationState1;
	dataOut->m_deactivationTime = m_deactivationTime;
	dataOut->m_friction = m_friction;
	dataOut->m_rollingFriction = m_rollingFriction;
	dataOut->m_contactDamping = m_contactDamping;
	dataOut->m_contactStiffness = m_contactStiffness;
	dataOut->m_restitution = m_restitution;
	dataOut->m_internalType = m_internalType;

	tuk name = (tuk)serializer->findNameForPointer(this);
	dataOut->m_name = (tuk)serializer->getUniquePointer(name);
	if (dataOut->m_name)
	{
		serializer->serializeName(name);
	}
	dataOut->m_hitFraction = m_hitFraction;
	dataOut->m_ccdSweptSphereRadius = m_ccdSweptSphereRadius;
	dataOut->m_ccdMotionThreshold = m_ccdMotionThreshold;
	dataOut->m_checkCollideWith = m_checkCollideWith;
	if (m_broadphaseHandle)
	{
		dataOut->m_collisionFilterGroup = m_broadphaseHandle->m_collisionFilterGroup;
		dataOut->m_collisionFilterMask = m_broadphaseHandle->m_collisionFilterMask;
		dataOut->m_uniqueId = m_broadphaseHandle->m_uniqueId;
	}
	else
	{
		dataOut->m_collisionFilterGroup = 0;
		dataOut->m_collisionFilterMask = 0;
		dataOut->m_uniqueId = -1;
	}
	return CollisionObject2DataName;
}

void CollisionObject2::serializeSingleObject(class Serializer* serializer) const
{
	i32 len = calculateSerializeBufferSize();
	Chunk* chunk = serializer->allocate(len, 1);
	tukk structType = serialize(chunk->m_oldPtr, serializer);
	serializer->finalizeChunk(chunk, structType, DRX3D_COLLISIONOBJECT_CODE, (uk )this);
}
