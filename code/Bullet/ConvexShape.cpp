#if defined(_WIN32) || defined(__i386__)
#define DRX3D_USE_SSE_IN_API
#endif

#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Shapes/CylinderShape.h>
#include <drx3D/Physics/Collision/Shapes/ConeShape.h>
#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexHullShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexPointCloudShape.h>

///not supported on IBM SDK, until we fix the alignment of Vec3
#if defined(__CELLOS_LV2__) && defined(__SPU__)
#include <spu_intrinsics.h>
static inline vec_float4 vec_dot3(vec_float4 vec0, vec_float4 vec1)
{
	vec_float4 result;
	result = spu_mul(vec0, vec1);
	result = spu_madd(spu_rlqwbyte(vec0, 4), spu_rlqwbyte(vec1, 4), result);
	return spu_madd(spu_rlqwbyte(vec0, 8), spu_rlqwbyte(vec1, 8), result);
}
#endif  //__SPU__

ConvexShape::ConvexShape()
{
}

ConvexShape::~ConvexShape()
{
}

void ConvexShape::project(const Transform2& trans, const Vec3& dir, Scalar& min, Scalar& max, Vec3& witnesPtMin, Vec3& witnesPtMax) const
{
	Vec3 localAxis = dir * trans.getBasis();
	Vec3 vtx1 = trans(localGetSupportingVertex(localAxis));
	Vec3 vtx2 = trans(localGetSupportingVertex(-localAxis));

	min = vtx1.dot(dir);
	max = vtx2.dot(dir);
	witnesPtMax = vtx2;
	witnesPtMin = vtx1;

	if (min > max)
	{
		Scalar tmp = min;
		min = max;
		max = tmp;
		witnesPtMax = vtx1;
		witnesPtMin = vtx2;
	}
}

static Vec3 convexHullSupport(const Vec3& localDirOrg, const Vec3* points, i32 numPoints, const Vec3& localScaling)
{
	Vec3 vec = localDirOrg * localScaling;

#if defined(__CELLOS_LV2__) && defined(__SPU__)

	Vec3 localDir = vec;

	vec_float4 v_distMax = {-FLT_MAX, 0, 0, 0};
	vec_int4 v_idxMax = {-999, 0, 0, 0};
	i32 v = 0;
	i32 numverts = numPoints;

	for (; v < (i32)numverts - 4; v += 4)
	{
		vec_float4 p0 = vec_dot3(points[v].get128(), localDir.get128());
		vec_float4 p1 = vec_dot3(points[v + 1].get128(), localDir.get128());
		vec_float4 p2 = vec_dot3(points[v + 2].get128(), localDir.get128());
		vec_float4 p3 = vec_dot3(points[v + 3].get128(), localDir.get128());
		const vec_int4 i0 = {v, 0, 0, 0};
		const vec_int4 i1 = {v + 1, 0, 0, 0};
		const vec_int4 i2 = {v + 2, 0, 0, 0};
		const vec_int4 i3 = {v + 3, 0, 0, 0};
		vec_uint4 retGt01 = spu_cmpgt(p0, p1);
		vec_float4 pmax01 = spu_sel(p1, p0, retGt01);
		vec_int4 imax01 = spu_sel(i1, i0, retGt01);
		vec_uint4 retGt23 = spu_cmpgt(p2, p3);
		vec_float4 pmax23 = spu_sel(p3, p2, retGt23);
		vec_int4 imax23 = spu_sel(i3, i2, retGt23);
		vec_uint4 retGt0123 = spu_cmpgt(pmax01, pmax23);
		vec_float4 pmax0123 = spu_sel(pmax23, pmax01, retGt0123);
		vec_int4 imax0123 = spu_sel(imax23, imax01, retGt0123);
		vec_uint4 retGtMax = spu_cmpgt(v_distMax, pmax0123);
		v_distMax = spu_sel(pmax0123, v_distMax, retGtMax);
		v_idxMax = spu_sel(imax0123, v_idxMax, retGtMax);
	}
	for (; v < (i32)numverts; v++)
	{
		vec_float4 p = vec_dot3(points[v].get128(), localDir.get128());
		const vec_int4 i = {v, 0, 0, 0};
		vec_uint4 retGtMax = spu_cmpgt(v_distMax, p);
		v_distMax = spu_sel(p, v_distMax, retGtMax);
		v_idxMax = spu_sel(i, v_idxMax, retGtMax);
	}
	i32 ptIndex = spu_extract(v_idxMax, 0);
	const Vec3& supVec = points[ptIndex] * localScaling;
	return supVec;
#else

	Scalar maxDot;
	long ptIndex = vec.maxDot(points, numPoints, maxDot);
	Assert(ptIndex >= 0);
	if (ptIndex < 0)
	{
		ptIndex = 0;
	}
	Vec3 supVec = points[ptIndex] * localScaling;
	return supVec;
#endif  //__SPU__
}

Vec3 ConvexShape::localGetSupportVertexWithoutMarginNonVirtual(const Vec3& localDir) const
{
	switch (m_shapeType)
	{
		case SPHERE_SHAPE_PROXYTYPE:
		{
			return Vec3(0, 0, 0);
		}
		case BOX_SHAPE_PROXYTYPE:
		{
			BoxShape* convexShape = (BoxShape*)this;
			const Vec3& halfExtents = convexShape->getImplicitShapeDimensions();

#if defined(__APPLE__) && (defined(DRX3D_USE_SSE) || defined(DRX3D_USE_NEON))
#if defined(DRX3D_USE_SSE)
			return Vec3(_mm_xor_ps(_mm_and_ps(localDir.mVec128, (__m128){-0.0f, -0.0f, -0.0f, -0.0f}), halfExtents.mVec128));
#elif defined(DRX3D_USE_NEON)
			return Vec3((float32x4_t)(((uint32x4_t)localDir.mVec128 & (uint32x4_t){0x80000000, 0x80000000, 0x80000000, 0x80000000}) ^ (uint32x4_t)halfExtents.mVec128));
#else
#error unknown vector arch
#endif
#else
			return Vec3(Fsels(localDir.x(), halfExtents.x(), -halfExtents.x()),
							 Fsels(localDir.y(), halfExtents.y(), -halfExtents.y()),
							 Fsels(localDir.z(), halfExtents.z(), -halfExtents.z()));
#endif
		}
		case TRIANGLE_SHAPE_PROXYTYPE:
		{
			TriangleShape* triangleShape = (TriangleShape*)this;
			Vec3 dir(localDir.getX(), localDir.getY(), localDir.getZ());
			Vec3* vertices = &triangleShape->m_vertices1[0];
			Vec3 dots = dir.dot3(vertices[0], vertices[1], vertices[2]);
			Vec3 sup = vertices[dots.maxAxis()];
			return Vec3(sup.getX(), sup.getY(), sup.getZ());
		}
		case CYLINDER_SHAPE_PROXYTYPE:
		{
			CylinderShape* cylShape = (CylinderShape*)this;
			//mapping of halfextents/dimension onto radius/height depends on how cylinder local orientation is (upAxis)

			Vec3 halfExtents = cylShape->getImplicitShapeDimensions();
			Vec3 v(localDir.getX(), localDir.getY(), localDir.getZ());
			i32 cylinderUpAxis = cylShape->getUpAxis();
			i32 XX(1), YY(0), ZZ(2);

			switch (cylinderUpAxis)
			{
				case 0:
				{
					XX = 1;
					YY = 0;
					ZZ = 2;
				}
				break;
				case 1:
				{
					XX = 0;
					YY = 1;
					ZZ = 2;
				}
				break;
				case 2:
				{
					XX = 0;
					YY = 2;
					ZZ = 1;
				}
				break;
				default:
					Assert(0);
					break;
			};

			Scalar radius = halfExtents[XX];
			Scalar halfHeight = halfExtents[cylinderUpAxis];

			Vec3 tmp;
			Scalar d;

			Scalar s = Sqrt(v[XX] * v[XX] + v[ZZ] * v[ZZ]);
			if (s != Scalar(0.0))
			{
				d = radius / s;
				tmp[XX] = v[XX] * d;
				tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
				tmp[ZZ] = v[ZZ] * d;
				return Vec3(tmp.getX(), tmp.getY(), tmp.getZ());
			}
			else
			{
				tmp[XX] = radius;
				tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
				tmp[ZZ] = Scalar(0.0);
				return Vec3(tmp.getX(), tmp.getY(), tmp.getZ());
			}
		}
		case CAPSULE_SHAPE_PROXYTYPE:
		{
			Vec3 vec0(localDir.getX(), localDir.getY(), localDir.getZ());

			CapsuleShape* capsuleShape = (CapsuleShape*)this;
			Scalar halfHeight = capsuleShape->getHalfHeight();
			i32 capsuleUpAxis = capsuleShape->getUpAxis();

			Vec3 supVec(0, 0, 0);

			Scalar maxDot(Scalar(-DRX3D_LARGE_FLOAT));

			Vec3 vec = vec0;
			Scalar lenSqr = vec.length2();
			if (lenSqr < SIMD_EPSILON * SIMD_EPSILON)
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
			{
				Vec3 pos(0, 0, 0);
				pos[capsuleUpAxis] = halfHeight;

				vtx = pos;
				newDot = vec.dot(vtx);

				if (newDot > maxDot)
				{
					maxDot = newDot;
					supVec = vtx;
				}
			}
			{
				Vec3 pos(0, 0, 0);
				pos[capsuleUpAxis] = -halfHeight;

				vtx = pos;
				newDot = vec.dot(vtx);
				if (newDot > maxDot)
				{
					maxDot = newDot;
					supVec = vtx;
				}
			}
			return Vec3(supVec.getX(), supVec.getY(), supVec.getZ());
		}
		case CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE:
		{
			ConvexPointCloudShape* convexPointCloudShape = (ConvexPointCloudShape*)this;
			Vec3* points = convexPointCloudShape->getUnscaledPoints();
			i32 numPoints = convexPointCloudShape->getNumPoints();
			return convexHullSupport(localDir, points, numPoints, convexPointCloudShape->getLocalScalingNV());
		}
		case CONVEX_HULL_SHAPE_PROXYTYPE:
		{
			ConvexHullShape* convexHullShape = (ConvexHullShape*)this;
			Vec3* points = convexHullShape->getUnscaledPoints();
			i32 numPoints = convexHullShape->getNumPoints();
			return convexHullSupport(localDir, points, numPoints, convexHullShape->getLocalScalingNV());
		}
		default:
#ifndef __SPU__
			return this->localGetSupportingVertexWithoutMargin(localDir);
#else
			Assert(0);
#endif
	}

	// should never reach here
	Assert(0);
	return Vec3(Scalar(0.0f), Scalar(0.0f), Scalar(0.0f));
}

Vec3 ConvexShape::localGetSupportVertexNonVirtual(const Vec3& localDir) const
{
	Vec3 localDirNorm = localDir;
	if (localDirNorm.length2() < (SIMD_EPSILON * SIMD_EPSILON))
	{
		localDirNorm.setVal(Scalar(-1.), Scalar(-1.), Scalar(-1.));
	}
	localDirNorm.normalize();

	return localGetSupportVertexWithoutMarginNonVirtual(localDirNorm) + getMarginNonVirtual() * localDirNorm;
}

/* TODO: This should be bumped up to CollisionShape () */
Scalar ConvexShape::getMarginNonVirtual() const
{
	switch (m_shapeType)
	{
		case SPHERE_SHAPE_PROXYTYPE:
		{
			SphereShape* sphereShape = (SphereShape*)this;
			return sphereShape->getRadius();
		}
		case BOX_SHAPE_PROXYTYPE:
		{
			BoxShape* convexShape = (BoxShape*)this;
			return convexShape->getMarginNV();
		}
		case TRIANGLE_SHAPE_PROXYTYPE:
		{
			TriangleShape* triangleShape = (TriangleShape*)this;
			return triangleShape->getMarginNV();
		}
		case CYLINDER_SHAPE_PROXYTYPE:
		{
			CylinderShape* cylShape = (CylinderShape*)this;
			return cylShape->getMarginNV();
		}
		case CONE_SHAPE_PROXYTYPE:
		{
			ConeShape* conShape = (ConeShape*)this;
			return conShape->getMarginNV();
		}
		case CAPSULE_SHAPE_PROXYTYPE:
		{
			CapsuleShape* capsuleShape = (CapsuleShape*)this;
			return capsuleShape->getMarginNV();
		}
		case CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE:
		/* fall through */
		case CONVEX_HULL_SHAPE_PROXYTYPE:
		{
			PolyhedralConvexShape* convexHullShape = (PolyhedralConvexShape*)this;
			return convexHullShape->getMarginNV();
		}
		default:
#ifndef __SPU__
			return this->getMargin();
#else
			Assert(0);
#endif
	}

	// should never reach here
	Assert(0);
	return Scalar(0.0f);
}
#ifndef __SPU__
void ConvexShape::getAabbNonVirtual(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	switch (m_shapeType)
	{
		case SPHERE_SHAPE_PROXYTYPE:
		{
			SphereShape* sphereShape = (SphereShape*)this;
			Scalar radius = sphereShape->getImplicitShapeDimensions().getX();  // * convexShape->getLocalScaling().getX();
			Scalar margin = radius + sphereShape->getMarginNonVirtual();
			const Vec3& center = t.getOrigin();
			Vec3 extent(margin, margin, margin);
			aabbMin = center - extent;
			aabbMax = center + extent;
		}
		break;
		case CYLINDER_SHAPE_PROXYTYPE:
		/* fall through */
		case BOX_SHAPE_PROXYTYPE:
		{
			BoxShape* convexShape = (BoxShape*)this;
			Scalar margin = convexShape->getMarginNonVirtual();
			Vec3 halfExtents = convexShape->getImplicitShapeDimensions();
			halfExtents += Vec3(margin, margin, margin);
			Matrix3x3 abs_b = t.getBasis().absolute();
			Vec3 center = t.getOrigin();
			Vec3 extent = halfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);

			aabbMin = center - extent;
			aabbMax = center + extent;
			break;
		}
		case TRIANGLE_SHAPE_PROXYTYPE:
		{
			TriangleShape* triangleShape = (TriangleShape*)this;
			Scalar margin = triangleShape->getMarginNonVirtual();
			for (i32 i = 0; i < 3; i++)
			{
				Vec3 vec(Scalar(0.), Scalar(0.), Scalar(0.));
				vec[i] = Scalar(1.);

				Vec3 sv = localGetSupportVertexWithoutMarginNonVirtual(vec * t.getBasis());

				Vec3 tmp = t(sv);
				aabbMax[i] = tmp[i] + margin;
				vec[i] = Scalar(-1.);
				tmp = t(localGetSupportVertexWithoutMarginNonVirtual(vec * t.getBasis()));
				aabbMin[i] = tmp[i] - margin;
			}
		}
		break;
		case CAPSULE_SHAPE_PROXYTYPE:
		{
			CapsuleShape* capsuleShape = (CapsuleShape*)this;
			Vec3 halfExtents(capsuleShape->getRadius(), capsuleShape->getRadius(), capsuleShape->getRadius());
			i32 m_upAxis = capsuleShape->getUpAxis();
			halfExtents[m_upAxis] = capsuleShape->getRadius() + capsuleShape->getHalfHeight();
			Matrix3x3 abs_b = t.getBasis().absolute();
			Vec3 center = t.getOrigin();
			Vec3 extent = halfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);
			aabbMin = center - extent;
			aabbMax = center + extent;
		}
		break;
		case CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE:
		case CONVEX_HULL_SHAPE_PROXYTYPE:
		{
			PolyhedralConvexAabbCachingShape* convexHullShape = (PolyhedralConvexAabbCachingShape*)this;
			Scalar margin = convexHullShape->getMarginNonVirtual();
			convexHullShape->getNonvirtualAabb(t, aabbMin, aabbMax, margin);
		}
		break;
		default:
#ifndef __SPU__
			this->getAabb(t, aabbMin, aabbMax);
#else
			Assert(0);
#endif
			break;
	}

	// should never reach here
	Assert(0);
}

#endif  //__SPU__
