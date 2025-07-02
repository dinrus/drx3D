#ifndef DRX3D_GIMPACT_QUANTIZATION_H_INCLUDED
#define DRX3D_GIMPACT_QUANTIZATION_H_INCLUDED

#include <drx3D/Maths/Linear/Transform2.h>

SIMD_FORCE_INLINE void drx3d_calc_quantization_parameters(
	Vec3& outMinBound,
	Vec3& outMaxBound,
	Vec3& bvhQuantization,
	const Vec3& srcMinBound, const Vec3& srcMaxBound,
	Scalar quantizationMargin)
{
	//enlarge the AABB to avoid division by zero when initializing the quantization values
	Vec3 clampValue(quantizationMargin, quantizationMargin, quantizationMargin);
	outMinBound = srcMinBound - clampValue;
	outMaxBound = srcMaxBound + clampValue;
	Vec3 aabbSize = outMaxBound - outMinBound;
	bvhQuantization = Vec3(Scalar(65535.0),
								Scalar(65535.0),
								Scalar(65535.0)) /
					  aabbSize;
}

SIMD_FORCE_INLINE void drx3d_quantize_clamp(
	unsigned short* out,
	const Vec3& point,
	const Vec3& min_bound,
	const Vec3& max_bound,
	const Vec3& bvhQuantization)
{
	Vec3 clampedPoint(point);
	clampedPoint.setMax(min_bound);
	clampedPoint.setMin(max_bound);

	Vec3 v = (clampedPoint - min_bound) * bvhQuantization;
	out[0] = (unsigned short)(v.getX() + 0.5f);
	out[1] = (unsigned short)(v.getY() + 0.5f);
	out[2] = (unsigned short)(v.getZ() + 0.5f);
}

SIMD_FORCE_INLINE Vec3 drx3d_unquantize(
	const unsigned short* vecIn,
	const Vec3& offset,
	const Vec3& bvhQuantization)
{
	Vec3 vecOut;
	vecOut.setVal(
		(Scalar)(vecIn[0]) / (bvhQuantization.getX()),
		(Scalar)(vecIn[1]) / (bvhQuantization.getY()),
		(Scalar)(vecIn[2]) / (bvhQuantization.getZ()));
	vecOut += offset;
	return vecOut;
}

#endif  // DRX3D_GIMPACT_QUANTIZATION_H_INCLUDED
