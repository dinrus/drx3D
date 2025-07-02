
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Maths/Linear/Serializer.h>

/*
  Make sure this dummy function never changes so that it
  can be used by probes that are checking whether the
  library is actually installed.
*/
extern "C"
{
	void BulletCollisionProbe();

	void BulletCollisionProbe() {}
}

void CollisionShape::getBoundingSphere(Vec3& center, Scalar& radius) const
{
	Transform2 tr;
	tr.setIdentity();
	Vec3 aabbMin, aabbMax;

	getAabb(tr, aabbMin, aabbMax);

	radius = (aabbMax - aabbMin).length() * Scalar(0.5);
	center = (aabbMin + aabbMax) * Scalar(0.5);
}

Scalar CollisionShape::getContactBreakingThreshold(Scalar defaultContactThreshold) const
{
	return getAngularMotionDisc() * defaultContactThreshold;
}

Scalar CollisionShape::getAngularMotionDisc() const
{
	///@todo cache this value, to improve performance
	Vec3 center;
	Scalar disc;
	getBoundingSphere(center, disc);
	disc += (center).length();
	return disc;
}

void CollisionShape::calculateTemporalAabb(const Transform2& curTrans, const Vec3& linvel, const Vec3& angvel, Scalar timeStep, Vec3& temporalAabbMin, Vec3& temporalAabbMax) const
{
	//start with static aabb
	getAabb(curTrans, temporalAabbMin, temporalAabbMax);

	Scalar temporalAabbMaxx = temporalAabbMax.getX();
	Scalar temporalAabbMaxy = temporalAabbMax.getY();
	Scalar temporalAabbMaxz = temporalAabbMax.getZ();
	Scalar temporalAabbMinx = temporalAabbMin.getX();
	Scalar temporalAabbMiny = temporalAabbMin.getY();
	Scalar temporalAabbMinz = temporalAabbMin.getZ();

	// add linear motion
	Vec3 linMotion = linvel * timeStep;
	///@todo: simd would have a vector max/min operation, instead of per-element access
	if (linMotion.x() > Scalar(0.))
		temporalAabbMaxx += linMotion.x();
	else
		temporalAabbMinx += linMotion.x();
	if (linMotion.y() > Scalar(0.))
		temporalAabbMaxy += linMotion.y();
	else
		temporalAabbMiny += linMotion.y();
	if (linMotion.z() > Scalar(0.))
		temporalAabbMaxz += linMotion.z();
	else
		temporalAabbMinz += linMotion.z();

	//add conservative angular motion
	Scalar angularMotion = angvel.length() * getAngularMotionDisc() * timeStep;
	Vec3 angularMotion3d(angularMotion, angularMotion, angularMotion);
	temporalAabbMin = Vec3(temporalAabbMinx, temporalAabbMiny, temporalAabbMinz);
	temporalAabbMax = Vec3(temporalAabbMaxx, temporalAabbMaxy, temporalAabbMaxz);

	temporalAabbMin -= angularMotion3d;
	temporalAabbMax += angularMotion3d;
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk CollisionShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	CollisionShapeData* shapeData = (CollisionShapeData*)dataBuffer;
	tuk name = (tuk)serializer->findNameForPointer(this);
	shapeData->m_name = (tuk)serializer->getUniquePointer(name);
	if (shapeData->m_name)
	{
		serializer->serializeName(name);
	}
	shapeData->m_shapeType = m_shapeType;

	// Fill padding with zeros to appease msan.
	memset(shapeData->m_padding, 0, sizeof(shapeData->m_padding));

	return "CollisionShapeData";
}

void CollisionShape::serializeSingleShape(Serializer* serializer) const
{
	i32 len = calculateSerializeBufferSize();
	Chunk* chunk = serializer->allocate(len, 1);
	tukk structType = serialize(chunk->m_oldPtr, serializer);
	serializer->finalizeChunk(chunk, structType, DRX3D_SHAPE_CODE, (uk )this);
}