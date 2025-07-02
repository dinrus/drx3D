#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Collision/BroadPhase/Dbvt.h>
#include <drx3D/Maths/Linear/Serializer.h>

CompoundShape::CompoundShape(bool enableDynamicAabbTree, i32k initialChildCapacity)
	: m_localAabbMin(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT)),
	  m_localAabbMax(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT)),
	  m_dynamicAabbTree(0),
	  m_updateRevision(1),
	  m_collisionMargin(Scalar(0.)),
	  m_localScaling(Scalar(1.), Scalar(1.), Scalar(1.))
{
	m_shapeType = COMPOUND_SHAPE_PROXYTYPE;

	if (enableDynamicAabbTree)
	{
		uk mem = AlignedAlloc(sizeof(Dbvt), 16);
		m_dynamicAabbTree = new (mem) Dbvt();
		Assert(mem == m_dynamicAabbTree);
	}

	m_children.reserve(initialChildCapacity);
}

CompoundShape::~CompoundShape()
{
	if (m_dynamicAabbTree)
	{
		m_dynamicAabbTree->~Dbvt();
		AlignedFree(m_dynamicAabbTree);
	}
}

void CompoundShape::addChildShape(const Transform2& localTransform, CollisionShape* shape)
{
	m_updateRevision++;
	//m_childTransforms.push_back(localTransform);
	//m_childShapes.push_back(shape);
	CompoundShapeChild child;
	child.m_node = 0;
	child.m_transform = localTransform;
	child.m_childShape = shape;
	child.m_childShapeType = shape->getShapeType();
	child.m_childMargin = shape->getMargin();

	//extend the local aabbMin/aabbMax
	Vec3 localAabbMin, localAabbMax;
	shape->getAabb(localTransform, localAabbMin, localAabbMax);
	for (i32 i = 0; i < 3; i++)
	{
		if (m_localAabbMin[i] > localAabbMin[i])
		{
			m_localAabbMin[i] = localAabbMin[i];
		}
		if (m_localAabbMax[i] < localAabbMax[i])
		{
			m_localAabbMax[i] = localAabbMax[i];
		}
	}
	if (m_dynamicAabbTree)
	{
		const DbvtVolume bounds = DbvtVolume::FromMM(localAabbMin, localAabbMax);
		size_t index = m_children.size();
		child.m_node = m_dynamicAabbTree->insert(bounds, reinterpret_cast<uk>(index));
	}

	m_children.push_back(child);
}

void CompoundShape::updateChildTransform(i32 childIndex, const Transform2& newChildTransform, bool shouldRecalculateLocalAabb)
{
	m_children[childIndex].m_transform = newChildTransform;

	if (m_dynamicAabbTree)
	{
		///update the dynamic aabb tree
		Vec3 localAabbMin, localAabbMax;
		m_children[childIndex].m_childShape->getAabb(newChildTransform, localAabbMin, localAabbMax);
		ATTRIBUTE_ALIGNED16(DbvtVolume)
		bounds = DbvtVolume::FromMM(localAabbMin, localAabbMax);
		//i32 index = m_children.size()-1;
		m_dynamicAabbTree->update(m_children[childIndex].m_node, bounds);
	}

	if (shouldRecalculateLocalAabb)
	{
		recalculateLocalAabb();
	}
}

void CompoundShape::removeChildShapeByIndex(i32 childShapeIndex)
{
	m_updateRevision++;
	Assert(childShapeIndex >= 0 && childShapeIndex < m_children.size());
	if (m_dynamicAabbTree)
	{
		m_dynamicAabbTree->remove(m_children[childShapeIndex].m_node);
	}
	m_children.swap(childShapeIndex, m_children.size() - 1);
	if (m_dynamicAabbTree)
		m_children[childShapeIndex].m_node->dataAsInt = childShapeIndex;
	m_children.pop_back();
}

void CompoundShape::removeChildShape(CollisionShape* shape)
{
	m_updateRevision++;
	// Find the children containing the shape specified, and remove those children.
	//note: there might be multiple children using the same shape!
	for (i32 i = m_children.size() - 1; i >= 0; i--)
	{
		if (m_children[i].m_childShape == shape)
		{
			removeChildShapeByIndex(i);
		}
	}

	recalculateLocalAabb();
}

void CompoundShape::recalculateLocalAabb()
{
	// Recalculate the local aabb
	// Brute force, it iterates over all the shapes left.

	m_localAabbMin = Vec3(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
	m_localAabbMax = Vec3(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT));

	//extend the local aabbMin/aabbMax
	for (i32 j = 0; j < m_children.size(); j++)
	{
		Vec3 localAabbMin, localAabbMax;
		m_children[j].m_childShape->getAabb(m_children[j].m_transform, localAabbMin, localAabbMax);
		for (i32 i = 0; i < 3; i++)
		{
			if (m_localAabbMin[i] > localAabbMin[i])
				m_localAabbMin[i] = localAabbMin[i];
			if (m_localAabbMax[i] < localAabbMax[i])
				m_localAabbMax[i] = localAabbMax[i];
		}
	}
}

///getAabb's default implementation is brute force, expected derived classes to implement a fast dedicated version
void CompoundShape::getAabb(const Transform2& trans, Vec3& aabbMin, Vec3& aabbMax) const
{
	Vec3 localHalfExtents = Scalar(0.5) * (m_localAabbMax - m_localAabbMin);
	Vec3 localCenter = Scalar(0.5) * (m_localAabbMax + m_localAabbMin);

	//avoid an illegal AABB when there are no children
	if (!m_children.size())
	{
		localHalfExtents.setVal(0, 0, 0);
		localCenter.setVal(0, 0, 0);
	}
	localHalfExtents += Vec3(getMargin(), getMargin(), getMargin());

	Matrix3x3 abs_b = trans.getBasis().absolute();

	Vec3 center = trans(localCenter);

	Vec3 extent = localHalfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);
	aabbMin = center - extent;
	aabbMax = center + extent;
}

void CompoundShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	//approximation: take the inertia from the aabb for now
	Transform2 ident;
	ident.setIdentity();
	Vec3 aabbMin, aabbMax;
	getAabb(ident, aabbMin, aabbMax);

	Vec3 halfExtents = (aabbMax - aabbMin) * Scalar(0.5);

	Scalar lx = Scalar(2.) * (halfExtents.x());
	Scalar ly = Scalar(2.) * (halfExtents.y());
	Scalar lz = Scalar(2.) * (halfExtents.z());

	inertia[0] = mass / (Scalar(12.0)) * (ly * ly + lz * lz);
	inertia[1] = mass / (Scalar(12.0)) * (lx * lx + lz * lz);
	inertia[2] = mass / (Scalar(12.0)) * (lx * lx + ly * ly);
}

void CompoundShape::calculatePrincipalAxisTransform(const Scalar* masses, Transform2& principal, Vec3& inertia) const
{
	i32 n = m_children.size();

	Scalar totalMass = 0;
	Vec3 center(0, 0, 0);
	i32 k;

	for (k = 0; k < n; k++)
	{
		Assert(masses[k] > 0);
		center += m_children[k].m_transform.getOrigin() * masses[k];
		totalMass += masses[k];
	}

	Assert(totalMass > 0);

	center /= totalMass;
	principal.setOrigin(center);

	Matrix3x3 tensor(0, 0, 0, 0, 0, 0, 0, 0, 0);
	for (k = 0; k < n; k++)
	{
		Vec3 i;
		m_children[k].m_childShape->calculateLocalInertia(masses[k], i);

		const Transform2& t = m_children[k].m_transform;
		Vec3 o = t.getOrigin() - center;

		//compute inertia tensor in coordinate system of compound shape
		Matrix3x3 j = t.getBasis().transpose();
		j[0] *= i[0];
		j[1] *= i[1];
		j[2] *= i[2];
		j = t.getBasis() * j;

		//add inertia tensor
		tensor[0] += j[0];
		tensor[1] += j[1];
		tensor[2] += j[2];

		//compute inertia tensor of pointmass at o
		Scalar o2 = o.length2();
		j[0].setVal(o2, 0, 0);
		j[1].setVal(0, o2, 0);
		j[2].setVal(0, 0, o2);
		j[0] += o * -o.x();
		j[1] += o * -o.y();
		j[2] += o * -o.z();

		//add inertia tensor of pointmass
		tensor[0] += masses[k] * j[0];
		tensor[1] += masses[k] * j[1];
		tensor[2] += masses[k] * j[2];
	}

	tensor.diagonalize(principal.getBasis(), Scalar(0.00001), 20);
	inertia.setVal(tensor[0][0], tensor[1][1], tensor[2][2]);
}

void CompoundShape::setLocalScaling(const Vec3& scaling)
{
	for (i32 i = 0; i < m_children.size(); i++)
	{
		Transform2 childTrans = getChildTransform(i);
		Vec3 childScale = m_children[i].m_childShape->getLocalScaling();
		//		childScale = childScale * (childTrans.getBasis() * scaling);
		childScale = childScale * scaling / m_localScaling;
		m_children[i].m_childShape->setLocalScaling(childScale);
		childTrans.setOrigin((childTrans.getOrigin()) * scaling / m_localScaling);
		updateChildTransform(i, childTrans, false);
	}

	m_localScaling = scaling;
	recalculateLocalAabb();
}

void CompoundShape::createAabbTreeFromChildren()
{
	if (!m_dynamicAabbTree)
	{
		uk mem = AlignedAlloc(sizeof(Dbvt), 16);
		m_dynamicAabbTree = new (mem) Dbvt();
		Assert(mem == m_dynamicAabbTree);

		for (i32 index = 0; index < m_children.size(); index++)
		{
			CompoundShapeChild& child = m_children[index];

			//extend the local aabbMin/aabbMax
			Vec3 localAabbMin, localAabbMax;
			child.m_childShape->getAabb(child.m_transform, localAabbMin, localAabbMax);

			const DbvtVolume bounds = DbvtVolume::FromMM(localAabbMin, localAabbMax);
			size_t index2 = index;
			child.m_node = m_dynamicAabbTree->insert(bounds, reinterpret_cast<uk>(index2));
		}
	}
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk CompoundShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	CompoundShapeData* shapeData = (CompoundShapeData*)dataBuffer;
	CollisionShape::serialize(&shapeData->m_collisionShapeData, serializer);

	shapeData->m_collisionMargin = float(m_collisionMargin);
	shapeData->m_numChildShapes = m_children.size();
	shapeData->m_childShapePtr = 0;
	if (shapeData->m_numChildShapes)
	{
		Chunk* chunk = serializer->allocate(sizeof(CompoundShapeChildData), shapeData->m_numChildShapes);
		CompoundShapeChildData* memPtr = (CompoundShapeChildData*)chunk->m_oldPtr;
		shapeData->m_childShapePtr = (CompoundShapeChildData*)serializer->getUniquePointer(memPtr);

		for (i32 i = 0; i < shapeData->m_numChildShapes; i++, memPtr++)
		{
			memPtr->m_childMargin = float(m_children[i].m_childMargin);
			memPtr->m_childShape = (CollisionShapeData*)serializer->getUniquePointer(m_children[i].m_childShape);
			//don't serialize shapes that already have been serialized
			if (!serializer->findPointer(m_children[i].m_childShape))
			{
				Chunk* chunk = serializer->allocate(m_children[i].m_childShape->calculateSerializeBufferSize(), 1);
				tukk structType = m_children[i].m_childShape->serialize(chunk->m_oldPtr, serializer);
				serializer->finalizeChunk(chunk, structType, DRX3D_SHAPE_CODE, m_children[i].m_childShape);
			}

			memPtr->m_childShapeType = m_children[i].m_childShapeType;
			m_children[i].m_transform.serializeFloat(memPtr->m_transform);
		}
		serializer->finalizeChunk(chunk, "CompoundShapeChildData", DRX3D_ARRAY_CODE, chunk->m_oldPtr);
	}
	return "CompoundShapeData";
}
