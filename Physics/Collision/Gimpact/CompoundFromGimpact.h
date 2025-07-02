#ifndef DRX3D_COMPOUND_FROM_GIMPACT
#define DRX3D_COMPOUND_FROM_GIMPACT

#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/Gimpact/GImpactShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/RaycastCallback.h>

ATTRIBUTE_ALIGNED16(class)
CompoundFromGimpactShape : public CompoundShape
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	virtual ~CompoundFromGimpactShape()
	{
		/*delete all the BU_Simplex1to4 ChildShapes*/
		for (i32 i = 0; i < m_children.size(); i++)
		{
			delete m_children[i].m_childShape;
		}
	}
};

struct MyCallback : public TriangleRaycastCallback
{
	i32 m_ignorePart;
	i32 m_ignoreTriangleIndex;

	MyCallback(const Vec3& from, const Vec3& to, i32 ignorePart, i32 ignoreTriangleIndex)
		: TriangleRaycastCallback(from, to),
		  m_ignorePart(ignorePart),
		  m_ignoreTriangleIndex(ignoreTriangleIndex)
	{
	}
	virtual Scalar reportHit(const Vec3& hitNormalLocal, Scalar hitFraction, i32 partId, i32 triangleIndex)
	{
		if (partId != m_ignorePart || triangleIndex != m_ignoreTriangleIndex)
		{
			if (hitFraction < m_hitFraction)
				return hitFraction;
		}

		return m_hitFraction;
	}
};
struct MyInternalTriangleIndexCallback : public InternalTriangleIndexCallback
{
	const GImpactMeshShape* m_gimpactShape;
	CompoundShape* m_colShape;
	Scalar m_depth;

	MyInternalTriangleIndexCallback(CompoundShape* colShape, const GImpactMeshShape* meshShape, Scalar depth)
		: m_colShape(colShape),
		  m_gimpactShape(meshShape),
		  m_depth(depth)
	{
	}

	virtual void internalProcessTriangleIndex(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		Vec3 scale = m_gimpactShape->getLocalScaling();
		Vec3 v0 = triangle[0] * scale;
		Vec3 v1 = triangle[1] * scale;
		Vec3 v2 = triangle[2] * scale;

		Vec3 centroid = (v0 + v1 + v2) / 3;
		Vec3 normal = (v1 - v0).cross(v2 - v0);
		normal.normalize();
		Vec3 rayFrom = centroid;
		Vec3 rayTo = centroid - normal * m_depth;

		MyCallback cb(rayFrom, rayTo, partId, triangleIndex);

		m_gimpactShape->processAllTrianglesRay(&cb, rayFrom, rayTo);
		if (cb.m_hitFraction < 1)
		{
			rayTo.setInterpolate3(cb.m_from, cb.m_to, cb.m_hitFraction);
			//rayTo = cb.m_from;
			//rayTo = rayTo.lerp(cb.m_to,cb.m_hitFraction);
			//gDebugDraw.drawLine(tr(centroid),tr(centroid+normal),Vec3(1,0,0));
		}

		BU_Simplex1to4* tet = new BU_Simplex1to4(v0, v1, v2, rayTo);
		Transform2 ident;
		ident.setIdentity();
		m_colShape->addChildShape(ident, tet);
	}
};

CompoundShape* CreateCompoundFromGimpactShape(const GImpactMeshShape* gimpactMesh, Scalar depth)
{
	CompoundShape* colShape = new CompoundFromGimpactShape();

	Transform2 tr;
	tr.setIdentity();

	MyInternalTriangleIndexCallback cb(colShape, gimpactMesh, depth);
	Vec3 aabbMin, aabbMax;
	gimpactMesh->getAabb(tr, aabbMin, aabbMax);
	gimpactMesh->getMeshInterface()->InternalProcessAllTriangles(&cb, aabbMin, aabbMax);

	return colShape;
}

#endif  //DRX3D_COMPOUND_FROM_GIMPACT
