#ifndef D3_AABB_UTIL2
#define D3_AABB_UTIL2

#include <drx3D/Common/b3Transform.h>
#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Maths/Linear/MinMax.h>

D3_FORCE_INLINE void b3AabbExpand(b3Vec3& aabbMin,
								  b3Vec3& aabbMax,
								  const b3Vec3& expansionMin,
								  const b3Vec3& expansionMax)
{
	aabbMin = aabbMin + expansionMin;
	aabbMax = aabbMax + expansionMax;
}

/// conservative test for overlap between two aabbs
D3_FORCE_INLINE bool b3TestPointAgainstAabb2(const b3Vec3& aabbMin1, const b3Vec3& aabbMax1,
											 const b3Vec3& point)
{
	bool overlap = true;
	overlap = (aabbMin1.getX() > point.getX() || aabbMax1.getX() < point.getX()) ? false : overlap;
	overlap = (aabbMin1.getZ() > point.getZ() || aabbMax1.getZ() < point.getZ()) ? false : overlap;
	overlap = (aabbMin1.getY() > point.getY() || aabbMax1.getY() < point.getY()) ? false : overlap;
	return overlap;
}

/// conservative test for overlap between two aabbs
D3_FORCE_INLINE bool b3TestAabbAgainstAabb2(const b3Vec3& aabbMin1, const b3Vec3& aabbMax1,
											const b3Vec3& aabbMin2, const b3Vec3& aabbMax2)
{
	bool overlap = true;
	overlap = (aabbMin1.getX() > aabbMax2.getX() || aabbMax1.getX() < aabbMin2.getX()) ? false : overlap;
	overlap = (aabbMin1.getZ() > aabbMax2.getZ() || aabbMax1.getZ() < aabbMin2.getZ()) ? false : overlap;
	overlap = (aabbMin1.getY() > aabbMax2.getY() || aabbMax1.getY() < aabbMin2.getY()) ? false : overlap;
	return overlap;
}

/// conservative test for overlap between triangle and aabb
D3_FORCE_INLINE bool b3TestTriangleAgainstAabb2(const b3Vec3* vertices,
												const b3Vec3& aabbMin, const b3Vec3& aabbMax)
{
	const b3Vec3& p1 = vertices[0];
	const b3Vec3& p2 = vertices[1];
	const b3Vec3& p3 = vertices[2];

	if (d3Min(d3Min(p1[0], p2[0]), p3[0]) > aabbMax[0]) return false;
	if (d3Max(d3Max(p1[0], p2[0]), p3[0]) < aabbMin[0]) return false;

	if (d3Min(d3Min(p1[2], p2[2]), p3[2]) > aabbMax[2]) return false;
	if (d3Max(d3Max(p1[2], p2[2]), p3[2]) < aabbMin[2]) return false;

	if (d3Min(d3Min(p1[1], p2[1]), p3[1]) > aabbMax[1]) return false;
	if (d3Max(d3Max(p1[1], p2[1]), p3[1]) < aabbMin[1]) return false;
	return true;
}

D3_FORCE_INLINE i32 b3Outcode(const b3Vec3& p, const b3Vec3& halfExtent)
{
	return (p.getX() < -halfExtent.getX() ? 0x01 : 0x0) |
		   (p.getX() > halfExtent.getX() ? 0x08 : 0x0) |
		   (p.getY() < -halfExtent.getY() ? 0x02 : 0x0) |
		   (p.getY() > halfExtent.getY() ? 0x10 : 0x0) |
		   (p.getZ() < -halfExtent.getZ() ? 0x4 : 0x0) |
		   (p.getZ() > halfExtent.getZ() ? 0x20 : 0x0);
}

D3_FORCE_INLINE bool b3RayAabb2(const b3Vec3& rayFrom,
								const b3Vec3& rayInvDirection,
								u32k raySign[3],
								const b3Vec3 bounds[2],
								b3Scalar& tmin,
								b3Scalar lambda_min,
								b3Scalar lambda_max)
{
	b3Scalar tmax, tymin, tymax, tzmin, tzmax;
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

D3_FORCE_INLINE bool b3RayAabb(const b3Vec3& rayFrom,
							   const b3Vec3& rayTo,
							   const b3Vec3& aabbMin,
							   const b3Vec3& aabbMax,
							   b3Scalar& param, b3Vec3& normal)
{
	b3Vec3 aabbHalfExtent = (aabbMax - aabbMin) * b3Scalar(0.5);
	b3Vec3 aabbCenter = (aabbMax + aabbMin) * b3Scalar(0.5);
	b3Vec3 source = rayFrom - aabbCenter;
	b3Vec3 target = rayTo - aabbCenter;
	i32 sourceOutcode = b3Outcode(source, aabbHalfExtent);
	i32 targetOutcode = b3Outcode(target, aabbHalfExtent);
	if ((sourceOutcode & targetOutcode) == 0x0)
	{
		b3Scalar lambda_enter = b3Scalar(0.0);
		b3Scalar lambda_exit = param;
		b3Vec3 r = target - source;
		i32 i;
		b3Scalar normSign = 1;
		b3Vec3 hitNormal = b3MakeVector3(0, 0, 0);
		i32 bit = 1;

		for (i32 j = 0; j < 2; j++)
		{
			for (i = 0; i != 3; ++i)
			{
				if (sourceOutcode & bit)
				{
					b3Scalar lambda = (-source[i] - aabbHalfExtent[i] * normSign) / r[i];
					if (lambda_enter <= lambda)
					{
						lambda_enter = lambda;
						hitNormal.setVal(0, 0, 0);
						hitNormal[i] = normSign;
					}
				}
				else if (targetOutcode & bit)
				{
					b3Scalar lambda = (-source[i] - aabbHalfExtent[i] * normSign) / r[i];
					SetMin(lambda_exit, lambda);
				}
				bit <<= 1;
			}
			normSign = b3Scalar(-1.);
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

D3_FORCE_INLINE void b3TransformAabb(const b3Vec3& halfExtents, b3Scalar margin, const b3Transform& t, b3Vec3& aabbMinOut, b3Vec3& aabbMaxOut)
{
	b3Vec3 halfExtentsWithMargin = halfExtents + b3MakeVector3(margin, margin, margin);
	b3Matrix3x3 abs_b = t.getBasis().absolute();
	b3Vec3 center = t.getOrigin();
	b3Vec3 extent = halfExtentsWithMargin.dot3(abs_b[0], abs_b[1], abs_b[2]);
	aabbMinOut = center - extent;
	aabbMaxOut = center + extent;
}

D3_FORCE_INLINE void b3TransformAabb(const b3Vec3& localAabbMin, const b3Vec3& localAabbMax, b3Scalar margin, const b3Transform& trans, b3Vec3& aabbMinOut, b3Vec3& aabbMaxOut)
{
	//drx3DAssert(localAabbMin.getX() <= localAabbMax.getX());
	//drx3DAssert(localAabbMin.getY() <= localAabbMax.getY());
	//drx3DAssert(localAabbMin.getZ() <= localAabbMax.getZ());
	b3Vec3 localHalfExtents = b3Scalar(0.5) * (localAabbMax - localAabbMin);
	localHalfExtents += b3MakeVector3(margin, margin, margin);

	b3Vec3 localCenter = b3Scalar(0.5) * (localAabbMax + localAabbMin);
	b3Matrix3x3 abs_b = trans.getBasis().absolute();
	b3Vec3 center = trans(localCenter);
	b3Vec3 extent = localHalfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);
	aabbMinOut = center - extent;
	aabbMaxOut = center + extent;
}

#define D3_USE_BANCHLESS 1
#ifdef D3_USE_BANCHLESS
//This block replaces the block below and uses no branches, and replaces the 8 bit return with a 32 bit return for improved performance (~3x on XBox 360)
D3_FORCE_INLINE unsigned b3TestQuantizedAabbAgainstQuantizedAabb(u16k* aabbMin1, u16k* aabbMax1, u16k* aabbMin2, u16k* aabbMax2)
{
	return static_cast<u32>(b3Select((unsigned)((aabbMin1[0] <= aabbMax2[0]) & (aabbMax1[0] >= aabbMin2[0]) & (aabbMin1[2] <= aabbMax2[2]) & (aabbMax1[2] >= aabbMin2[2]) & (aabbMin1[1] <= aabbMax2[1]) & (aabbMax1[1] >= aabbMin2[1])),
											  1, 0));
}
#else
D3_FORCE_INLINE bool b3TestQuantizedAabbAgainstQuantizedAabb(u16k* aabbMin1, u16k* aabbMax1, u16k* aabbMin2, u16k* aabbMax2)
{
	bool overlap = true;
	overlap = (aabbMin1[0] > aabbMax2[0] || aabbMax1[0] < aabbMin2[0]) ? false : overlap;
	overlap = (aabbMin1[2] > aabbMax2[2] || aabbMax1[2] < aabbMin2[2]) ? false : overlap;
	overlap = (aabbMin1[1] > aabbMax2[1] || aabbMax1[1] < aabbMin2[1]) ? false : overlap;
	return overlap;
}
#endif  //D3_USE_BANCHLESS

#endif  //D3_AABB_UTIL2
