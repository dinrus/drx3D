#if defined(_WIN32) || defined(__i386__)
#define DRX3D_USE_SSE_IN_API
#endif

#include <drx3D/Physics/Collision/Shapes/MultiSphereShape.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>
#include <drx3D/Maths/Linear/Quat.h>
#include <drx3D/Maths/Linear/Serializer.h>

MultiSphereShape::MultiSphereShape(const Vec3* positions, const Scalar* radi, i32 numSpheres)
	: ConvexInternalAabbCachingShape()
{
	m_shapeType = MULTI_SPHERE_SHAPE_PROXYTYPE;
	//Scalar startMargin = Scalar(DRX3D_LARGE_FLOAT);

	m_localPositionArray.resize(numSpheres);
	m_radiArray.resize(numSpheres);
	for (i32 i = 0; i < numSpheres; i++)
	{
		m_localPositionArray[i] = positions[i];
		m_radiArray[i] = radi[i];
	}

	recalcLocalAabb();
}

#ifndef MIN
#define MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#endif
Vec3 MultiSphereShape::localGetSupportingVertexWithoutMargin(const Vec3& vec0) const
{
	Vec3 supVec(0, 0, 0);

	Scalar maxDot(Scalar(-DRX3D_LARGE_FLOAT));

	Vec3 vec = vec0;
	Scalar lenSqr = vec.length2();
	if (lenSqr < (SIMD_EPSILON * SIMD_EPSILON))
	{
		vec.setVal(1, 0, 0);
	}
	else
	{
		Scalar rlen = Scalar(1.) / Sqrt(lenSqr);
		vec *= rlen;
	}

	Vec3 vtx;
	Scalar newDot;

	const Vec3* pos = &m_localPositionArray[0];
	const Scalar* rad = &m_radiArray[0];
	i32 numSpheres = m_localPositionArray.size();

	for (i32 k = 0; k < numSpheres; k += 128)
	{
		Vec3 temp[128];
		i32 inner_count = MIN(numSpheres - k, 128);
		for (long i = 0; i < inner_count; i++)
		{
			temp[i] = (*pos) * m_localScaling + vec * m_localScaling * (*rad) - vec * getMargin();
			pos++;
			rad++;
		}
		long i = vec.maxDot(temp, inner_count, newDot);
		if (newDot > maxDot)
		{
			maxDot = newDot;
			supVec = temp[i];
		}
	}

	return supVec;
}

void MultiSphereShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	for (i32 j = 0; j < numVectors; j++)
	{
		Scalar maxDot(Scalar(-DRX3D_LARGE_FLOAT));

		const Vec3& vec = vectors[j];

		Vec3 vtx;
		Scalar newDot;

		const Vec3* pos = &m_localPositionArray[0];
		const Scalar* rad = &m_radiArray[0];
		i32 numSpheres = m_localPositionArray.size();

		for (i32 k = 0; k < numSpheres; k += 128)
		{
			Vec3 temp[128];
			i32 inner_count = MIN(numSpheres - k, 128);
			for (long i = 0; i < inner_count; i++)
			{
				temp[i] = (*pos) * m_localScaling + vec * m_localScaling * (*rad) - vec * getMargin();
				pos++;
				rad++;
			}
			long i = vec.maxDot(temp, inner_count, newDot);
			if (newDot > maxDot)
			{
				maxDot = newDot;
				supportVerticesOut[j] = temp[i];
			}
		}
	}
}

void MultiSphereShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	//as an approximation, take the inertia of the box that bounds the spheres

	Vec3 localAabbMin, localAabbMax;
	getCachedLocalAabb(localAabbMin, localAabbMax);
	Vec3 halfExtents = (localAabbMax - localAabbMin) * Scalar(0.5);

	Scalar lx = Scalar(2.) * (halfExtents.x());
	Scalar ly = Scalar(2.) * (halfExtents.y());
	Scalar lz = Scalar(2.) * (halfExtents.z());

	inertia.setVal(mass / (Scalar(12.0)) * (ly * ly + lz * lz),
					 mass / (Scalar(12.0)) * (lx * lx + lz * lz),
					 mass / (Scalar(12.0)) * (lx * lx + ly * ly));
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk MultiSphereShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	MultiSphereShapeData* shapeData = (MultiSphereShapeData*)dataBuffer;
	ConvexInternalShape::serialize(&shapeData->m_convexInternalShapeData, serializer);

	i32 numElem = m_localPositionArray.size();
	shapeData->m_localPositionArrayPtr = numElem ? (PositionAndRadius*)serializer->getUniquePointer((uk )&m_localPositionArray[0]) : 0;

	shapeData->m_localPositionArraySize = numElem;
	if (numElem)
	{
		Chunk* chunk = serializer->allocate(sizeof(PositionAndRadius), numElem);
		PositionAndRadius* memPtr = (PositionAndRadius*)chunk->m_oldPtr;
		for (i32 i = 0; i < numElem; i++, memPtr++)
		{
			m_localPositionArray[i].serializeFloat(memPtr->m_pos);
			memPtr->m_radius = float(m_radiArray[i]);
		}
		serializer->finalizeChunk(chunk, "PositionAndRadius", DRX3D_ARRAY_CODE, (uk )&m_localPositionArray[0]);
	}

	// Fill padding with zeros to appease msan.
	memset(shapeData->m_padding, 0, sizeof(shapeData->m_padding));

	return "MultiSphereShapeData";
}
