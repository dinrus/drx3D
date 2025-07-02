#ifndef DRX3D_AABB_UTIL2
#define DRX3D_AABB_UTIL2

#include <drxtypes.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/MinMax.h>

SIMD_FORCE_INLINE void AabbExpand(Vec3& aabbMin,
								  Vec3& aabbMax,
								  const Vec3& expansionMin,
								  const Vec3& expansionMax)
{
	aabbMin = aabbMin + expansionMin;
	aabbMax = aabbMax + expansionMax;
}

/// conservative test for overlap between two aabbs
SIMD_FORCE_INLINE bool TestPointAgainstAabb2(const Vec3& aabbMin1, const Vec3& aabbMax1,
											 const Vec3& point)
{
	bool overlap = true;
	overlap = (aabbMin1.getX() > point.getX() || aabbMax1.getX() < point.getX()) ? false : overlap;
	overlap = (aabbMin1.getZ() > point.getZ() || aabbMax1.getZ() < point.getZ()) ? false : overlap;
	overlap = (aabbMin1.getY() > point.getY() || aabbMax1.getY() < point.getY()) ? false : overlap;
	return overlap;
}

/// conservative test for overlap between two aabbs
SIMD_FORCE_INLINE bool TestAabbAgainstAabb2(const Vec3& aabbMin1, const Vec3& aabbMax1,
											const Vec3& aabbMin2, const Vec3& aabbMax2)
{
	bool overlap = true;
	overlap = (aabbMin1.getX() > aabbMax2.getX() || aabbMax1.getX() < aabbMin2.getX()) ? false : overlap;
	overlap = (aabbMin1.getZ() > aabbMax2.getZ() || aabbMax1.getZ() < aabbMin2.getZ()) ? false : overlap;
	overlap = (aabbMin1.getY() > aabbMax2.getY() || aabbMax1.getY() < aabbMin2.getY()) ? false : overlap;
	return overlap;
}

/// conservative test for overlap between triangle and aabb
SIMD_FORCE_INLINE bool TestTriangleAgainstAabb2(const Vec3* vertices,
												const Vec3& aabbMin, const Vec3& aabbMax)
{
	const Vec3& p1 = vertices[0];
	const Vec3& p2 = vertices[1];
	const Vec3& p3 = vertices[2];

	if (d3Min(d3Min(p1[0], p2[0]), p3[0]) > aabbMax[0]) return false;
	if (d3Max(d3Max(p1[0], p2[0]), p3[0]) < aabbMin[0]) return false;

	if (d3Min(d3Min(p1[2], p2[2]), p3[2]) > aabbMax[2]) return false;
	if (d3Max(d3Max(p1[2], p2[2]), p3[2]) < aabbMin[2]) return false;

	if (d3Min(d3Min(p1[1], p2[1]), p3[1]) > aabbMax[1]) return false;
	if (d3Max(d3Max(p1[1], p2[1]), p3[1]) < aabbMin[1]) return false;
	return true;
}

SIMD_FORCE_INLINE i32 Outcode(const Vec3& p, const Vec3& halfExtent)
{
	return (p.getX() < -halfExtent.getX() ? 0x01 : 0x0) |
		   (p.getX() > halfExtent.getX() ? 0x08 : 0x0) |
		   (p.getY() < -halfExtent.getY() ? 0x02 : 0x0) |
		   (p.getY() > halfExtent.getY() ? 0x10 : 0x0) |
		   (p.getZ() < -halfExtent.getZ() ? 0x4 : 0x0) |
		   (p.getZ() > halfExtent.getZ() ? 0x20 : 0x0);
}

SIMD_FORCE_INLINE bool RayAabb2(const Vec3& rayFrom,
								  const Vec3& rayInvDirection,
								  u32k raySign[3],
								  const Vec3 bounds[2],
								  Scalar& tmin,
								  Scalar lambda_min,
								  Scalar lambda_max)
{
	Scalar tmax, tymin, tymax, tzmin, tzmax;
	tmin = (bounds[raySign[0]].getX() - rayFrom.getX()) * rayInvDirection.getX();
	tmax = (bounds[1 - raySign[0]].getX() - rayFrom.getX()) * rayInvDirection.getX();
	tymin = (bounds[raySign[1]].getY() - rayFrom.getY()) * rayInvDirection.getY();
	tymax = (bounds[1 - raySign[1]].getY() - rayFrom.getY()) * rayInvDirection.getY();

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	tzmin = (bounds[raySign[2]].getZ() - rayFrom.getZ()) * rayInvDirection.getZ();
	tzmax = (bounds[1 - raySign[2]].getZ() - rayFrom.getZ()) * rayInvDirection.getZ();

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
	return ((tmin < lambda_max) && (tmax > lambda_min));
}

SIMD_FORCE_INLINE bool RayAabb(const Vec3& rayFrom,
								 const Vec3& rayTo,
								 const Vec3& aabbMin,
								 const Vec3& aabbMax,
								 Scalar& param, Vec3& normal)
{
	Vec3 aabbHalfExtent = (aabbMax - aabbMin) * Scalar(0.5);
	Vec3 aabbCenter = (aabbMax + aabbMin) * Scalar(0.5);
	Vec3 source = rayFrom - aabbCenter;
	Vec3 target = rayTo - aabbCenter;
	i32 sourceOutcode = Outcode(source, aabbHalfExtent);
	i32 targetOutcode = Outcode(target, aabbHalfExtent);
	if ((sourceOutcode & targetOutcode) == 0x0)
	{
		Scalar lambda_enter = Scalar(0.0);
		Scalar lambda_exit = param;
		Vec3 r = target - source;
		i32 i;
		Scalar normSign = 1;
		Vec3 hitNormal(0, 0, 0);
		i32 bit = 1;

		for (i32 j = 0; j < 2; j++)
		{
			for (i = 0; i != 3; ++i)
			{
				if (sourceOutcode & bit)
				{
					Scalar lambda = (-source[i] - aabbHalfExtent[i] * normSign) / r[i];
					if (lambda_enter <= lambda)
					{
						lambda_enter = lambda;
						hitNormal.setVal(0, 0, 0);
						hitNormal[i] = normSign;
					}
				}
				else if (targetOutcode & bit)
				{
					Scalar lambda = (-source[i] - aabbHalfExtent[i] * normSign) / r[i];
					SetMin(lambda_exit, lambda);
				}
				bit <<= 1;
			}
			normSign = Scalar(-1.);
		}
		if (lambda_enter <= lambda_exit)
		{
			param = lambda_enter;
			normal = hitNormal;
			return true;
		}
	}
	return false;
}

SIMD_FORCE_INLINE void Transform2Aabb(const Vec3& halfExtents, Scalar margin, const Transform2& t, Vec3& aabbMinOut, Vec3& aabbMaxOut)
{
	Vec3 halfExtentsWithMargin = halfExtents + Vec3(margin, margin, margin);
	Matrix3x3 abs_b = t.getBasis().absolute();
	Vec3 center = t.getOrigin();
	Vec3 extent = halfExtentsWithMargin.dot3(abs_b[0], abs_b[1], abs_b[2]);
	aabbMinOut = center - extent;
	aabbMaxOut = center + extent;
}

SIMD_FORCE_INLINE void Transform2Aabb(const Vec3& localAabbMin, const Vec3& localAabbMax, Scalar margin, const Transform2& trans, Vec3& aabbMinOut, Vec3& aabbMaxOut)
{
	Assert(localAabbMin.getX() <= localAabbMax.getX());
	Assert(localAabbMin.getY() <= localAabbMax.getY());
	Assert(localAabbMin.getZ() <= localAabbMax.getZ());
	Vec3 localHalfExtents = Scalar(0.5) * (localAabbMax - localAabbMin);
	localHalfExtents += Vec3(margin, margin, margin);

	Vec3 localCenter = Scalar(0.5) * (localAabbMax + localAabbMin);
	Matrix3x3 abs_b = trans.getBasis().absolute();
	Vec3 center = trans(localCenter);
	Vec3 extent = localHalfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);
	aabbMinOut = center - extent;
	aabbMaxOut = center + extent;
}

#define USE_BANCHLESS 1
#ifdef USE_BANCHLESS
//This block replaces the block below and uses no branches, and replaces the 8 bit return with a 32 bit return for improved performance (~3x on XBox 360)
SIMD_FORCE_INLINE unsigned testQuantizedAabbAgainstQuantizedAabb(u16k* aabbMin1, u16k* aabbMax1, u16k* aabbMin2, u16k* aabbMax2)
{
	return static_cast<u32>(Select((unsigned)((aabbMin1[0] <= aabbMax2[0]) & (aabbMax1[0] >= aabbMin2[0]) & (aabbMin1[2] <= aabbMax2[2]) & (aabbMax1[2] >= aabbMin2[2]) & (aabbMin1[1] <= aabbMax2[1]) & (aabbMax1[1] >= aabbMin2[1])),
											  1, 0));
}
#else
SIMD_FORCE_INLINE bool testQuantizedAabbAgainstQuantizedAabb(u16k* aabbMin1, u16k* aabbMax1, u16k* aabbMin2, u16k* aabbMax2)
{
	bool overlap = true;
	overlap = (aabbMin1[0] > aabbMax2[0] || aabbMax1[0] < aabbMin2[0]) ? false : overlap;
	overlap = (aabbMin1[2] > aabbMax2[2] || aabbMax1[2] < aabbMin2[2]) ? false : overlap;
	overlap = (aabbMin1[1] > aabbMax2[1] || aabbMax1[1] < aabbMin2[1]) ? false : overlap;
	return overlap;
}
#endif  //USE_BANCHLESS

#endif  //DRX3D_AABB_UTIL2
