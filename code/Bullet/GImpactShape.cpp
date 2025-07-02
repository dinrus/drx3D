#include <drx3D/Physics/Collision/Gimpact/GImpactShape.h>
#include <drx3D/Physics/Collision/Gimpact/GImpactMassUtil.h>

GImpactMeshShapePart::GImpactMeshShapePart(StridingMeshInterface* meshInterface, i32 part)
{
	// moved from .h to .cpp because of conditional compilation
	// (The setting of DRX3D_THREADSAFE may differ between various cpp files, so it is best to
	// avoid using it in h files)
	m_primitive_manager.m_meshInterface = meshInterface;
	m_primitive_manager.m_part = part;
	m_box_set.setPrimitiveManager(&m_primitive_manager);
#if DRX3D_THREADSAFE
	// If threadsafe is requested, this object uses a different lock/unlock
	//  model with the StridingMeshInterface -- lock once when the object is constructed
	//  and unlock once in the destructor.
	// The other way of locking and unlocking for each collision check in the narrowphase
	// is not threadsafe.  Note these are not thread-locks, they are calls to the meshInterface's
	// getLockedReadOnlyVertexIndexBase virtual function, which by default just returns a couple of
	// pointers.  In theory a client could override the lock function to do all sorts of
	// things like reading data from GPU memory, or decompressing data on the fly, but such things
	// do not seem all that likely or useful, given the performance cost.
	m_primitive_manager.lock();
#endif
}

GImpactMeshShapePart::~GImpactMeshShapePart()
{
	// moved from .h to .cpp because of conditional compilation
#if DRX3D_THREADSAFE
	m_primitive_manager.unlock();
#endif
}

void GImpactMeshShapePart::lockChildShapes() const
{
	// moved from .h to .cpp because of conditional compilation
#if !DRX3D_THREADSAFE
	// called in the narrowphase -- not threadsafe!
	uk dummy = (uk )(m_box_set.getPrimitiveManager());
	TrimeshPrimitiveManager* dummymanager = static_cast<TrimeshPrimitiveManager*>(dummy);
	dummymanager->lock();
#endif
}

void GImpactMeshShapePart::unlockChildShapes() const
{
	// moved from .h to .cpp because of conditional compilation
#if !DRX3D_THREADSAFE
	// called in the narrowphase -- not threadsafe!
	uk dummy = (uk )(m_box_set.getPrimitiveManager());
	TrimeshPrimitiveManager* dummymanager = static_cast<TrimeshPrimitiveManager*>(dummy);
	dummymanager->unlock();
#endif
}

#define CALC_EXACT_INERTIA 1

void GImpactCompoundShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	lockChildShapes();
#ifdef CALC_EXACT_INERTIA
	inertia.setVal(0.f, 0.f, 0.f);

	i32 i = this->getNumChildShapes();
	Scalar shapemass = mass / Scalar(i);

	while (i--)
	{
		Vec3 temp_inertia;
		m_childShapes[i]->calculateLocalInertia(shapemass, temp_inertia);
		if (childrenHasTransform2())
		{
			inertia = gim_inertia_add_transformed(inertia, temp_inertia, m_childTransforms[i]);
		}
		else
		{
			inertia = gim_inertia_add_transformed(inertia, temp_inertia, Transform2::getIdentity());
		}
	}

#else

	// Calc box inertia

	Scalar lx = m_localAABB.m_max[0] - m_localAABB.m_min[0];
	Scalar ly = m_localAABB.m_max[1] - m_localAABB.m_min[1];
	Scalar lz = m_localAABB.m_max[2] - m_localAABB.m_min[2];
	const Scalar x2 = lx * lx;
	const Scalar y2 = ly * ly;
	const Scalar z2 = lz * lz;
	const Scalar scaledmass = mass * Scalar(0.08333333);

	inertia = scaledmass * (Vec3(y2 + z2, x2 + z2, x2 + y2));

#endif
	unlockChildShapes();
}

void GImpactMeshShapePart::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	lockChildShapes();

#ifdef CALC_EXACT_INERTIA
	inertia.setVal(0.f, 0.f, 0.f);

	i32 i = this->getVertexCount();
	Scalar pointmass = mass / Scalar(i);

	while (i--)
	{
		Vec3 pointintertia;
		this->getVertex(i, pointintertia);
		pointintertia = gim_get_point_inertia(pointintertia, pointmass);
		inertia += pointintertia;
	}

#else

	// Calc box inertia

	Scalar lx = m_localAABB.m_max[0] - m_localAABB.m_min[0];
	Scalar ly = m_localAABB.m_max[1] - m_localAABB.m_min[1];
	Scalar lz = m_localAABB.m_max[2] - m_localAABB.m_min[2];
	const Scalar x2 = lx * lx;
	const Scalar y2 = ly * ly;
	const Scalar z2 = lz * lz;
	const Scalar scaledmass = mass * Scalar(0.08333333);

	inertia = scaledmass * (Vec3(y2 + z2, x2 + z2, x2 + y2));

#endif

	unlockChildShapes();
}

void GImpactMeshShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
#ifdef CALC_EXACT_INERTIA
	inertia.setVal(0.f, 0.f, 0.f);

	i32 i = this->getMeshPartCount();
	Scalar partmass = mass / Scalar(i);

	while (i--)
	{
		Vec3 partinertia;
		getMeshPart(i)->calculateLocalInertia(partmass, partinertia);
		inertia += partinertia;
	}

#else

	// Calc box inertia

	Scalar lx = m_localAABB.m_max[0] - m_localAABB.m_min[0];
	Scalar ly = m_localAABB.m_max[1] - m_localAABB.m_min[1];
	Scalar lz = m_localAABB.m_max[2] - m_localAABB.m_min[2];
	const Scalar x2 = lx * lx;
	const Scalar y2 = ly * ly;
	const Scalar z2 = lz * lz;
	const Scalar scaledmass = mass * Scalar(0.08333333);

	inertia = scaledmass * (Vec3(y2 + z2, x2 + z2, x2 + y2));

#endif
}

void GImpactMeshShape::rayTest(const Vec3& rayFrom, const Vec3& rayTo, CollisionWorld::RayResultCallback& resultCallback) const
{
}

void GImpactMeshShapePart::processAllTrianglesRay(TriangleCallback* callback, const Vec3& rayFrom, const Vec3& rayTo) const
{
	lockChildShapes();

	AlignedObjectArray<i32> collided;
	Vec3 rayDir(rayTo - rayFrom);
	rayDir.normalize();
	m_box_set.rayQuery(rayDir, rayFrom, collided);

	if (collided.size() == 0)
	{
		unlockChildShapes();
		return;
	}

	i32 part = (i32)getPart();
	PrimitiveTriangle triangle;
	i32 i = collided.size();
	while (i--)
	{
		getPrimitiveTriangle(collided[i], triangle);
		callback->processTriangle(triangle.m_vertices, part, collided[i]);
	}
	unlockChildShapes();
}

void GImpactMeshShapePart::processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
{
	lockChildShapes();
	AABB box;
	box.m_min = aabbMin;
	box.m_max = aabbMax;

	AlignedObjectArray<i32> collided;
	m_box_set.boxQuery(box, collided);

	if (collided.size() == 0)
	{
		unlockChildShapes();
		return;
	}

	i32 part = (i32)getPart();
	PrimitiveTriangle triangle;
	i32 i = collided.size();
	while (i--)
	{
		this->getPrimitiveTriangle(collided[i], triangle);
		callback->processTriangle(triangle.m_vertices, part, collided[i]);
	}
	unlockChildShapes();
}

void GImpactMeshShape::processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
{
	i32 i = m_mesh_parts.size();
	while (i--)
	{
		m_mesh_parts[i]->processAllTriangles(callback, aabbMin, aabbMax);
	}
}

void GImpactMeshShape::processAllTrianglesRay(TriangleCallback* callback, const Vec3& rayFrom, const Vec3& rayTo) const
{
	i32 i = m_mesh_parts.size();
	while (i--)
	{
		m_mesh_parts[i]->processAllTrianglesRay(callback, rayFrom, rayTo);
	}
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk GImpactMeshShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	GImpactMeshShapeData* trimeshData = (GImpactMeshShapeData*)dataBuffer;

	CollisionShape::serialize(&trimeshData->m_collisionShapeData, serializer);

	m_meshInterface->serialize(&trimeshData->m_meshInterface, serializer);

	trimeshData->m_collisionMargin = float(m_collisionMargin);

	localScaling.serializeFloat(trimeshData->m_localScaling);

	trimeshData->m_gimpactSubType = i32(getGImpactShapeType());

	return "GImpactMeshShapeData";
}
