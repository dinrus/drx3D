#ifndef DRX3D_VECTOR3_H
#define DRX3D_VECTOR3_H

//#include <stdint.h>
#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define Vec3Data Vec3DoubleData
#define Vec3DataName "Vec3DoubleData"
#else
#define Vec3Data Vec3FloatData
#define Vec3DataName "Vec3FloatData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

#if defined DRX3D_USE_SSE

//typedef  uint32_t __m128i __attribute__ ((vector_size(16)));

#ifdef _MSC_VER
#pragma warning(disable : 4556)  // value of intrinsic immediate argument '4294967239' is out of range '0 - 255'
#endif

#define DRX3D_SHUFFLE(x, y, z, w) (((w) << 6 | (z) << 4 | (y) << 2 | (x)) & 0xff)
//#define drx3d_pshufd_ps( _a, _mask ) (__m128) _mm_shuffle_epi32((__m128i)(_a), (_mask) )
#define drx3d_pshufd_ps(_a, _mask) _mm_shuffle_ps((_a), (_a), (_mask))
#define drx3d_splat3_ps(_a, _i) drx3d_pshufd_ps((_a), DRX3D_SHUFFLE(_i, _i, _i, 3))
#define drx3d_splat_ps(_a, _i) drx3d_pshufd_ps((_a), DRX3D_SHUFFLE(_i, _i, _i, _i))

#define drxAbsiMask (_mm_set_epi32(0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF))
#define drx3dAbsMask (_mm_set_epi32(0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF))
#define drx3dFFF0Mask (_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))
#define drxAbsfMask CastiTo128f(v3AbsiMask)
#define drx3dFFF0fMask CastiTo128f(vFFF0Mask)
#define drx3dxyzMaskf drx3dFFF0fMask
#define drx3dAbsfMask CastiTo128f(vAbsMask)

//there is an issue with XCode 3.2 (LCx errors)
#define drx3dMzeroMask (_mm_set_ps(-0.0f, -0.0f, -0.0f, -0.0f))
#define v1110 (_mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f))
#define vHalf (_mm_set_ps(0.5f, 0.5f, 0.5f, 0.5f))
#define v1_5 (_mm_set_ps(1.5f, 1.5f, 1.5f, 1.5f))

#endif

#ifdef DRX3D_USE_NEON

const float32x4_t ATTRIBUTE_ALIGNED16(vMzeroMask) = (float32x4_t){-0.0f, -0.0f, -0.0f, -0.0f};
const int32x4_t ATTRIBUTE_ALIGNED16(vFFF0Mask) = (int32x4_t){static_cast<int32_t>(0xFFFFFFFF),
															static_cast<int32_t>(0xFFFFFFFF), static_cast<int32_t>(0xFFFFFFFF), 0x0};
const int32x4_t ATTRIBUTE_ALIGNED16(vAbsMask) = (int32x4_t){0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
const int32x4_t ATTRIBUTE_ALIGNED16(v3AbsMask) = (int32x4_t){0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x0};

#endif
class Vec3;
class Vec4;
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
//#if defined (DRX3D_USE_SSE) || defined (DRX3D_USE_NEON)
inline Vec3 MakeVector3(SimdFloat4 v);
inline Vec4 MakeVector4(SimdFloat4 vec);
#endif

inline Vec3 MakeVector3(Scalar x, Scalar y, Scalar z);
inline Vec3 MakeVector3(Scalar x, Scalar y, Scalar z, Scalar w);
inline Vec4 MakeVector4(Scalar x, Scalar y, Scalar z, Scalar w);

/**@brief Vec3 can be used to represent 3D points and vectors.
* It has an un-used w component to suit 16-byte alignment when Vec3 is stored in containers. This extra component can be used by derived classes (Quat?) or by user
* Ideally, this class should be replaced by a platform optimized SIMD version that keeps the data in registers
*/
ATTRIBUTE_ALIGNED16(class)
Vec3
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

#if defined(__SPU__) && defined(__CELLOS_LV2__)
	Scalar m_floats[4];

public:
	SIMD_FORCE_INLINE const vec_float4& get128() const
	{
		return *((const vec_float4*)&m_floats[0]);
	}

public:
#else                                            //__CELLOS_LV2__ __SPU__
#if defined(DRX3D_USE_SSE) || defined(DRX3D_USE_NEON)  // _WIN32 || ARM
	union {
		SimdFloat4 mVec128;
		Scalar m_floats[4];
	};
	SIMD_FORCE_INLINE SimdFloat4 get128() const
	{
		return mVec128;
	}
	SIMD_FORCE_INLINE void set128(SimdFloat4 v128)
	{
		mVec128 = v128;
	}
#else
	Scalar m_floats[4];
#endif
#endif  //__CELLOS_LV2__ __SPU__

public:
	/**@brief No initialization constructor */
	SIMD_FORCE_INLINE Vec3()
	{
	}

	/**@brief Constructor from scalars
* @param x X value
* @param y Y value
* @param z Z value
*/
	SIMD_FORCE_INLINE Vec3(const Scalar& _x, const Scalar& _y, const Scalar& _z)
	{
		m_floats[0] = _x;
		m_floats[1] = _y;
		m_floats[2] = _z;
		m_floats[3] = Scalar(0.f);
	}

#if (defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)) || defined(DRX3D_USE_NEON)
	// Set Vector
	SIMD_FORCE_INLINE Vec3(SimdFloat4 v)
	{
		mVec128 = v;
	}

	// Copy constructor
	SIMD_FORCE_INLINE Vec3(const Vec3& rhs)
	{
		mVec128 = rhs.mVec128;
	}

	// Assignment Operator
	SIMD_FORCE_INLINE Vec3&
	operator=(const Vec3& v)
	{
		mVec128 = v.mVec128;

		return *this;
	}
#endif  // #if defined (DRX3D_USE_SSE_IN_API) || defined (DRX3D_USE_NEON)

	/**@brief Add a vector to this one
* @param The vector to add to this one */
	SIMD_FORCE_INLINE Vec3& operator+=(const Vec3& v)
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		mVec128 = _mm_add_ps(mVec128, v.mVec128);
#elif defined(DRX3D_USE_NEON)
		mVec128 = vaddq_f32(mVec128, v.mVec128);
#else
		m_floats[0] += v.m_floats[0];
		m_floats[1] += v.m_floats[1];
		m_floats[2] += v.m_floats[2];
#endif
		return *this;
	}

	/**@brief Subtract a vector from this one
* @param The vector to subtract */
	SIMD_FORCE_INLINE Vec3& operator-=(const Vec3& v)
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		mVec128 = _mm_sub_ps(mVec128, v.mVec128);
#elif defined(DRX3D_USE_NEON)
		mVec128 = vsubq_f32(mVec128, v.mVec128);
#else
		m_floats[0] -= v.m_floats[0];
		m_floats[1] -= v.m_floats[1];
		m_floats[2] -= v.m_floats[2];
#endif
		return *this;
	}

	/**@brief Scale the vector
* @param s Scale factor */
	SIMD_FORCE_INLINE Vec3& operator*=(const Scalar& s)
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		__m128 vs = _mm_load_ss(&s);  //	(S 0 0 0)
		vs = drx3d_pshufd_ps(vs, 0x80);  //	(S S S 0.0)
		mVec128 = _mm_mul_ps(mVec128, vs);
#elif defined(DRX3D_USE_NEON)
		mVec128 = vmulq_n_f32(mVec128, s);
#else
		m_floats[0] *= s;
		m_floats[1] *= s;
		m_floats[2] *= s;
#endif
		return *this;
	}

	/**@brief Inversely scale the vector
* @param s Scale factor to divide by */
	SIMD_FORCE_INLINE Vec3& operator/=(const Scalar& s)
	{
		FullAssert(s != Scalar(0.0));

#if 0  //defined(DRX3D_USE_SSE_IN_API)
// this code is not faster !
		__m128 vs = _mm_load_ss(&s);
		vs = _mm_div_ss(v1110, vs);
		vs = drx3d_pshufd_ps(vs, 0x00);	//	(S S S S)

		mVec128 = _mm_mul_ps(mVec128, vs);

		return *this;
#else
		return *this *= Scalar(1.0) / s;
#endif
	}

	/**@brief Return the dot product
* @param v The other vector in the dot product */
	SIMD_FORCE_INLINE Scalar dot(const Vec3& v) const
	{
#if defined DRX3D_USE_SIMD_VECTOR3 && defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		__m128 vd = _mm_mul_ps(mVec128, v.mVec128);
		__m128 z = _mm_movehl_ps(vd, vd);
		__m128 y = _mm_shuffle_ps(vd, vd, 0x55);
		vd = _mm_add_ss(vd, y);
		vd = _mm_add_ss(vd, z);
		return _mm_cvtss_f32(vd);
#elif defined(DRX3D_USE_NEON)
		float32x4_t vd = vmulq_f32(mVec128, v.mVec128);
		float32x2_t x = vpadd_f32(vget_low_f32(vd), vget_low_f32(vd));
		x = vadd_f32(x, vget_high_f32(vd));
		return vget_lane_f32(x, 0);
#else
		return m_floats[0] * v.m_floats[0] +
			m_floats[1] * v.m_floats[1] +
			m_floats[2] * v.m_floats[2];
#endif
	}

	/**@brief Return the length of the vector squared */
	SIMD_FORCE_INLINE Scalar length2() const
	{
		return dot(*this);
	}

	/**@brief Return the length of the vector */
	SIMD_FORCE_INLINE Scalar length() const
	{
		return Sqrt(length2());
	}

	/**@brief Return the norm (length) of the vector */
	SIMD_FORCE_INLINE Scalar norm() const
	{
		return length();
	}

	/**@brief Return the norm (length) of the vector */
	SIMD_FORCE_INLINE Scalar safeNorm() const
	{
		Scalar d = length2();
		//workaround for some clang/gcc issue of sqrtf(tiny number) = -INF
		if (d > SIMD_EPSILON)
			return Sqrt(d);
		return Scalar(0);
	}

	/**@brief Return the distance squared between the ends of this and another vector
* This is symantically treating the vector like a point */
	SIMD_FORCE_INLINE Scalar distance2(const Vec3& v) const;

	/**@brief Return the distance between the ends of this and another vector
* This is symantically treating the vector like a point */
	SIMD_FORCE_INLINE Scalar distance(const Vec3& v) const;

	SIMD_FORCE_INLINE Vec3& safeNormalize()
	{
		Scalar l2 = length2();
		//triNormal.normalize();
		if (l2 >= SIMD_EPSILON * SIMD_EPSILON)
		{
			(*this) /= Sqrt(l2);
		}
		else
		{
			setVal(1, 0, 0);
		}
		return *this;
	}

	/**@brief Normalize this vector
* x^2 + y^2 + z^2 = 1 */
	SIMD_FORCE_INLINE Vec3& normalize()
	{
		Assert(!fuzzyZero());

#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		// dot product first
		__m128 vd = _mm_mul_ps(mVec128, mVec128);
		__m128 z = _mm_movehl_ps(vd, vd);
		__m128 y = _mm_shuffle_ps(vd, vd, 0x55);
		vd = _mm_add_ss(vd, y);
		vd = _mm_add_ss(vd, z);

#if 0
		vd = _mm_sqrt_ss(vd);
		vd = _mm_div_ss(v1110, vd);
		vd = drx3d_splat_ps(vd, 0x80);
		mVec128 = _mm_mul_ps(mVec128, vd);
#else

		// NR step 1/sqrt(x) - vd is x, y is output
		y = _mm_rsqrt_ss(vd);  // estimate

		//  one step NR
		z = v1_5;
		vd = _mm_mul_ss(vd, vHalf);  // vd * 0.5
		//x2 = vd;
		vd = _mm_mul_ss(vd, y);  // vd * 0.5 * y0
		vd = _mm_mul_ss(vd, y);  // vd * 0.5 * y0 * y0
		z = _mm_sub_ss(z, vd);   // 1.5 - vd * 0.5 * y0 * y0

		y = _mm_mul_ss(y, z);  // y0 * (1.5 - vd * 0.5 * y0 * y0)

		y = drx3d_splat_ps(y, 0x80);
		mVec128 = _mm_mul_ps(mVec128, y);

#endif

		return *this;
#else
		return *this /= length();
#endif
	}

	/**@brief Return a normalized version of this vector */
	SIMD_FORCE_INLINE Vec3 normalized() const;

	/**@brief Return a rotated version of this vector
* @param wAxis The axis to rotate about
* @param angle The angle to rotate by */
	SIMD_FORCE_INLINE Vec3 rotate(const Vec3& wAxis, const Scalar angle) const;

	/**@brief Return the angle between this and another vector
* @param v The other vector */
	SIMD_FORCE_INLINE Scalar angle(const Vec3& v) const
	{
		Scalar s = Sqrt(length2() * v.length2());
		FullAssert(s != Scalar(0.0));
		return Acos(dot(v) / s);
	}

	/**@brief Return a vector with the absolute values of each element */
	SIMD_FORCE_INLINE Vec3 absolute() const
	{
#if defined DRX3D_USE_SIMD_VECTOR3 && defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		return Vec3(_mm_and_ps(mVec128, drxAbsfMask));
#elif defined(DRX3D_USE_NEON)
		return Vec3(vabsq_f32(mVec128));
#else
		return Vec3(
			Fabs(m_floats[0]),
			Fabs(m_floats[1]),
			Fabs(m_floats[2]));
#endif
	}

	/**@brief Return the cross product between this and another vector
* @param v The other vector */
	SIMD_FORCE_INLINE Vec3 cross(const Vec3& v) const
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		__m128 T, V;

		T = drx3d_pshufd_ps(mVec128, DRX3D_SHUFFLE(1, 2, 0, 3));    //	(Y Z X 0)
		V = drx3d_pshufd_ps(v.mVec128, DRX3D_SHUFFLE(1, 2, 0, 3));  //	(Y Z X 0)

		V = _mm_mul_ps(V, mVec128);
		T = _mm_mul_ps(T, v.mVec128);
		V = _mm_sub_ps(V, T);

		V = drx3d_pshufd_ps(V, DRX3D_SHUFFLE(1, 2, 0, 3));
		return Vec3(V);
#elif defined(DRX3D_USE_NEON)
		float32x4_t T, V;
		// form (Y, Z, X, _) of mVec128 and v.mVec128
		float32x2_t Tlow = vget_low_f32(mVec128);
		float32x2_t Vlow = vget_low_f32(v.mVec128);
		T = vcombine_f32(vext_f32(Tlow, vget_high_f32(mVec128), 1), Tlow);
		V = vcombine_f32(vext_f32(Vlow, vget_high_f32(v.mVec128), 1), Vlow);

		V = vmulq_f32(V, mVec128);
		T = vmulq_f32(T, v.mVec128);
		V = vsubq_f32(V, T);
		Vlow = vget_low_f32(V);
		// form (Y, Z, X, _);
		V = vcombine_f32(vext_f32(Vlow, vget_high_f32(V), 1), Vlow);
		V = (float32x4_t)vandq_s32((int32x4_t)V, drx3dFFF0Mask);

		return Vec3(V);
#else
		return Vec3(
			m_floats[1] * v.m_floats[2] - m_floats[2] * v.m_floats[1],
			m_floats[2] * v.m_floats[0] - m_floats[0] * v.m_floats[2],
			m_floats[0] * v.m_floats[1] - m_floats[1] * v.m_floats[0]);
#endif
	}

	SIMD_FORCE_INLINE Scalar triple(const Vec3& v1, const Vec3& v2) const
	{
#if defined DRX3D_USE_SIMD_VECTOR3 && defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		// cross:
		__m128 T = _mm_shuffle_ps(v1.mVec128, v1.mVec128, DRX3D_SHUFFLE(1, 2, 0, 3));  //	(Y Z X 0)
		__m128 V = _mm_shuffle_ps(v2.mVec128, v2.mVec128, DRX3D_SHUFFLE(1, 2, 0, 3));  //	(Y Z X 0)

		V = _mm_mul_ps(V, v1.mVec128);
		T = _mm_mul_ps(T, v2.mVec128);
		V = _mm_sub_ps(V, T);

		V = _mm_shuffle_ps(V, V, DRX3D_SHUFFLE(1, 2, 0, 3));

		// dot:
		V = _mm_mul_ps(V, mVec128);
		__m128 z = _mm_movehl_ps(V, V);
		__m128 y = _mm_shuffle_ps(V, V, 0x55);
		V = _mm_add_ss(V, y);
		V = _mm_add_ss(V, z);
		return _mm_cvtss_f32(V);

#elif defined(DRX3D_USE_NEON)
		// cross:
		float32x4_t T, V;
		// form (Y, Z, X, _) of mVec128 and v.mVec128
		float32x2_t Tlow = vget_low_f32(v1.mVec128);
		float32x2_t Vlow = vget_low_f32(v2.mVec128);
		T = vcombine_f32(vext_f32(Tlow, vget_high_f32(v1.mVec128), 1), Tlow);
		V = vcombine_f32(vext_f32(Vlow, vget_high_f32(v2.mVec128), 1), Vlow);

		V = vmulq_f32(V, v1.mVec128);
		T = vmulq_f32(T, v2.mVec128);
		V = vsubq_f32(V, T);
		Vlow = vget_low_f32(V);
		// form (Y, Z, X, _);
		V = vcombine_f32(vext_f32(Vlow, vget_high_f32(V), 1), Vlow);

		// dot:
		V = vmulq_f32(mVec128, V);
		float32x2_t x = vpadd_f32(vget_low_f32(V), vget_low_f32(V));
		x = vadd_f32(x, vget_high_f32(V));
		return vget_lane_f32(x, 0);
#else
		return m_floats[0] * (v1.m_floats[1] * v2.m_floats[2] - v1.m_floats[2] * v2.m_floats[1]) +
			m_floats[1] * (v1.m_floats[2] * v2.m_floats[0] - v1.m_floats[0] * v2.m_floats[2]) +
			m_floats[2] * (v1.m_floats[0] * v2.m_floats[1] - v1.m_floats[1] * v2.m_floats[0]);
#endif
	}

	/**@brief Return the axis with the smallest value
* Note return values are 0,1,2 for x, y, or z */
	SIMD_FORCE_INLINE i32 minAxis() const
	{
		return m_floats[0] < m_floats[1] ? (m_floats[0] < m_floats[2] ? 0 : 2) : (m_floats[1] < m_floats[2] ? 1 : 2);
	}

	/**@brief Return the axis with the largest value
* Note return values are 0,1,2 for x, y, or z */
	SIMD_FORCE_INLINE i32 maxAxis() const
	{
		return m_floats[0] < m_floats[1] ? (m_floats[1] < m_floats[2] ? 2 : 1) : (m_floats[0] < m_floats[2] ? 2 : 0);
	}

	SIMD_FORCE_INLINE i32 furthestAxis() const
	{
		return absolute().minAxis();
	}

	SIMD_FORCE_INLINE i32 closestAxis() const
	{
		return absolute().maxAxis();
	}

	SIMD_FORCE_INLINE void setInterpolate3(const Vec3& v0, const Vec3& v1, Scalar rt)
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		__m128 vrt = _mm_load_ss(&rt);  //	(rt 0 0 0)
		Scalar s = Scalar(1.0) - rt;
		__m128 vs = _mm_load_ss(&s);  //	(S 0 0 0)
		vs = drx3d_pshufd_ps(vs, 0x80);  //	(S S S 0.0)
		__m128 r0 = _mm_mul_ps(v0.mVec128, vs);
		vrt = drx3d_pshufd_ps(vrt, 0x80);  //	(rt rt rt 0.0)
		__m128 r1 = _mm_mul_ps(v1.mVec128, vrt);
		__m128 tmp3 = _mm_add_ps(r0, r1);
		mVec128 = tmp3;
#elif defined(DRX3D_USE_NEON)
		float32x4_t vl = vsubq_f32(v1.mVec128, v0.mVec128);
		vl = vmulq_n_f32(vl, rt);
		mVec128 = vaddq_f32(vl, v0.mVec128);
#else
		Scalar s = Scalar(1.0) - rt;
		m_floats[0] = s * v0.m_floats[0] + rt * v1.m_floats[0];
		m_floats[1] = s * v0.m_floats[1] + rt * v1.m_floats[1];
		m_floats[2] = s * v0.m_floats[2] + rt * v1.m_floats[2];
		//don't do the unused w component
		//		m_co[3] = s * v0[3] + rt * v1[3];
#endif
	}

	/**@brief Return the linear interpolation between this and another vector
* @param v The other vector
* @param t The ration of this to v (t = 0 => return this, t=1 => return other) */
	SIMD_FORCE_INLINE Vec3 lerp(const Vec3& v, const Scalar& t) const
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		__m128 vt = _mm_load_ss(&t);  //	(t 0 0 0)
		vt = drx3d_pshufd_ps(vt, 0x80);  //	(rt rt rt 0.0)
		__m128 vl = _mm_sub_ps(v.mVec128, mVec128);
		vl = _mm_mul_ps(vl, vt);
		vl = _mm_add_ps(vl, mVec128);

		return Vec3(vl);
#elif defined(DRX3D_USE_NEON)
		float32x4_t vl = vsubq_f32(v.mVec128, mVec128);
		vl = vmulq_n_f32(vl, t);
		vl = vaddq_f32(vl, mVec128);

		return Vec3(vl);
#else
		return Vec3(m_floats[0] + (v.m_floats[0] - m_floats[0]) * t,
						m_floats[1] + (v.m_floats[1] - m_floats[1]) * t,
						m_floats[2] + (v.m_floats[2] - m_floats[2]) * t);
#endif
	}

	/**@brief Elementwise multiply this vector by the other
* @param v The other vector */
	SIMD_FORCE_INLINE Vec3& operator*=(const Vec3& v)
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		mVec128 = _mm_mul_ps(mVec128, v.mVec128);
#elif defined(DRX3D_USE_NEON)
		mVec128 = vmulq_f32(mVec128, v.mVec128);
#else
		m_floats[0] *= v.m_floats[0];
		m_floats[1] *= v.m_floats[1];
		m_floats[2] *= v.m_floats[2];
#endif
		return *this;
	}

	/**@brief Return the x value */
	SIMD_FORCE_INLINE const Scalar& getX() const { return m_floats[0]; }
	/**@brief Return the y value */
	SIMD_FORCE_INLINE const Scalar& getY() const { return m_floats[1]; }
	/**@brief Return the z value */
	SIMD_FORCE_INLINE const Scalar& getZ() const { return m_floats[2]; }
	/**@brief Set the x value */
	SIMD_FORCE_INLINE void setX(Scalar _x) { m_floats[0] = _x; };
	/**@brief Set the y value */
	SIMD_FORCE_INLINE void setY(Scalar _y) { m_floats[1] = _y; };
	/**@brief Set the z value */
	SIMD_FORCE_INLINE void setZ(Scalar _z) { m_floats[2] = _z; };
	/**@brief Set the w value */
	SIMD_FORCE_INLINE void setW(Scalar _w) { m_floats[3] = _w; };
	/**@brief Return the x value */
	SIMD_FORCE_INLINE const Scalar& x() const { return m_floats[0]; }
	/**@brief Return the y value */
	SIMD_FORCE_INLINE const Scalar& y() const { return m_floats[1]; }
	/**@brief Return the z value */
	SIMD_FORCE_INLINE const Scalar& z() const { return m_floats[2]; }
	/**@brief Return the w value */
	SIMD_FORCE_INLINE const Scalar& w() const { return m_floats[3]; }

	//SIMD_FORCE_INLINE Scalar&       operator[](i32 i)       { return (&m_floats[0])[i];	}
	//SIMD_FORCE_INLINE const Scalar& operator[](i32 i) const { return (&m_floats[0])[i]; }
	///operator Scalar*() replaces operator[], using implicit conversion. We added operator != and operator == to avoid pointer comparisons.
	SIMD_FORCE_INLINE operator Scalar*() { return &m_floats[0]; }
	SIMD_FORCE_INLINE operator const Scalar*() const { return &m_floats[0]; }

	SIMD_FORCE_INLINE bool operator==(const Vec3& other) const
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		return (0xf == _mm_movemask_ps((__m128)_mm_cmpeq_ps(mVec128, other.mVec128)));
#else
		return ((m_floats[3] == other.m_floats[3]) &&
				(m_floats[2] == other.m_floats[2]) &&
				(m_floats[1] == other.m_floats[1]) &&
				(m_floats[0] == other.m_floats[0]));
#endif
	}

	SIMD_FORCE_INLINE bool operator!=(const Vec3& other) const
	{
		return !(*this == other);
	}

	/**@brief Set each element to the max of the current values and the values of another Vec3
* @param other The other Vec3 to compare with
*/
	SIMD_FORCE_INLINE void setMax(const Vec3& other)
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		mVec128 = _mm_max_ps(mVec128, other.mVec128);
#elif defined(DRX3D_USE_NEON)
		mVec128 = vmaxq_f32(mVec128, other.mVec128);
#else
		SetMax(m_floats[0], other.m_floats[0]);
		SetMax(m_floats[1], other.m_floats[1]);
		SetMax(m_floats[2], other.m_floats[2]);
		SetMax(m_floats[3], other.w());
#endif
	}

	/**@brief Set each element to the min of the current values and the values of another Vec3
* @param other The other Vec3 to compare with
*/
	SIMD_FORCE_INLINE void setMin(const Vec3& other)
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		mVec128 = _mm_min_ps(mVec128, other.mVec128);
#elif defined(DRX3D_USE_NEON)
		mVec128 = vminq_f32(mVec128, other.mVec128);
#else
		SetMin(m_floats[0], other.m_floats[0]);
		SetMin(m_floats[1], other.m_floats[1]);
		SetMin(m_floats[2], other.m_floats[2]);
		SetMin(m_floats[3], other.w());
#endif
	}

	SIMD_FORCE_INLINE void setVal(const Scalar& _x, const Scalar& _y, const Scalar& _z)
	{
		m_floats[0] = _x;
		m_floats[1] = _y;
		m_floats[2] = _z;
		m_floats[3] = Scalar(0.f);
	}

	void getSkewSymmetricMatrix(Vec3 * v0, Vec3 * v1, Vec3 * v2) const
	{
#if defined DRX3D_USE_SIMD_VECTOR3 && defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)

		__m128 V = _mm_and_ps(mVec128, drx3dFFF0fMask);
		__m128 V0 = _mm_xor_ps(vMzeroMask, V);
		__m128 V2 = _mm_movelh_ps(V0, V);

		__m128 V1 = _mm_shuffle_ps(V, V0, 0xCE);

		V0 = _mm_shuffle_ps(V0, V, 0xDB);
		V2 = _mm_shuffle_ps(V2, V, 0xF9);

		v0->mVec128 = V0;
		v1->mVec128 = V1;
		v2->mVec128 = V2;
#else
		v0->setVal(0., -z(), y());
		v1->setVal(z(), 0., -x());
		v2->setVal(-y(), x(), 0.);
#endif
	}

	void setZero()
	{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		mVec128 = (__m128)_mm_xor_ps(mVec128, mVec128);
#elif defined(DRX3D_USE_NEON)
		int32x4_t vi = vdupq_n_s32(0);
		mVec128 = vreinterpretq_f32_s32(vi);
#else
		setVal(Scalar(0.), Scalar(0.), Scalar(0.));
#endif
	}

	SIMD_FORCE_INLINE bool isZero() const
	{
		return m_floats[0] == Scalar(0) && m_floats[1] == Scalar(0) && m_floats[2] == Scalar(0);
	}

	SIMD_FORCE_INLINE bool fuzzyZero() const
	{
		return length2() < SIMD_EPSILON * SIMD_EPSILON;
	}

	SIMD_FORCE_INLINE void serialize(struct Vec3Data & dataOut) const;

	SIMD_FORCE_INLINE void deSerialize(const struct Vec3DoubleData& dataIn);

	SIMD_FORCE_INLINE void deSerialize(const struct Vec3FloatData& dataIn);

	SIMD_FORCE_INLINE void serializeFloat(struct Vec3FloatData & dataOut) const;

	SIMD_FORCE_INLINE void deSerializeFloat(const struct Vec3FloatData& dataIn);

	SIMD_FORCE_INLINE void serializeDouble(struct Vec3DoubleData & dataOut) const;

	SIMD_FORCE_INLINE void deSerializeDouble(const struct Vec3DoubleData& dataIn);

	/**@brief returns index of maximum dot product between this and vectors in array[]
		* @param array The other vectors
		* @param array_count The number of other vectors
		* @param dotOut The maximum dot product */
	SIMD_FORCE_INLINE long maxDot(const Vec3* array, long array_count, Scalar& dotOut) const;

	/**@brief returns index of minimum dot product between this and vectors in array[]
		* @param array The other vectors
		* @param array_count The number of other vectors
		* @param dotOut The minimum dot product */
	SIMD_FORCE_INLINE long minDot(const Vec3* array, long array_count, Scalar& dotOut) const;

	/* create a vector as  Vec3( this->dot( Vec3 v0 ), this->dot( Vec3 v1), this->dot( Vec3 v2 ))  */
	SIMD_FORCE_INLINE Vec3 dot3(const Vec3& v0, const Vec3& v1, const Vec3& v2) const
	{
#if defined DRX3D_USE_SIMD_VECTOR3 && defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)

		__m128 a0 = _mm_mul_ps(v0.mVec128, this->mVec128);
		__m128 a1 = _mm_mul_ps(v1.mVec128, this->mVec128);
		__m128 a2 = _mm_mul_ps(v2.mVec128, this->mVec128);
		__m128 b0 = _mm_unpacklo_ps(a0, a1);
		__m128 b1 = _mm_unpackhi_ps(a0, a1);
		__m128 b2 = _mm_unpacklo_ps(a2, _mm_setzero_ps());
		__m128 r = _mm_movelh_ps(b0, b2);
		r = _mm_add_ps(r, _mm_movehl_ps(b2, b0));
		a2 = _mm_and_ps(a2, drx3dxyzMaskf);
		r = _mm_add_ps(r, CastdTo128f(_mm_move_sd(CastfTo128d(a2), CastfTo128d(b1))));
		return Vec3(r);

#elif defined(DRX3D_USE_NEON)
		static const uint32x4_t xyzMask = (const uint32x4_t){static_cast<uint32_t>(-1), static_cast<uint32_t>(-1), static_cast<uint32_t>(-1), 0};
		float32x4_t a0 = vmulq_f32(v0.mVec128, this->mVec128);
		float32x4_t a1 = vmulq_f32(v1.mVec128, this->mVec128);
		float32x4_t a2 = vmulq_f32(v2.mVec128, this->mVec128);
		float32x2x2_t zLo = vtrn_f32(vget_high_f32(a0), vget_high_f32(a1));
		a2 = (float32x4_t)vandq_u32((uint32x4_t)a2, xyzMask);
		float32x2_t b0 = vadd_f32(vpadd_f32(vget_low_f32(a0), vget_low_f32(a1)), zLo.val[0]);
		float32x2_t b1 = vpadd_f32(vpadd_f32(vget_low_f32(a2), vget_high_f32(a2)), vdup_n_f32(0.0f));
		return Vec3(vcombine_f32(b0, b1));
#else
		return Vec3(dot(v0), dot(v1), dot(v2));
#endif
	}
};

/**@brief Return the sum of two vectors (Point symantics)*/
SIMD_FORCE_INLINE Vec3
operator+(const Vec3& v1, const Vec3& v2)
{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
	return Vec3(_mm_add_ps(v1.mVec128, v2.mVec128));
#elif defined(DRX3D_USE_NEON)
	return Vec3(vaddq_f32(v1.mVec128, v2.mVec128));
#else
	return Vec3(
		v1.m_floats[0] + v2.m_floats[0],
		v1.m_floats[1] + v2.m_floats[1],
		v1.m_floats[2] + v2.m_floats[2]);
#endif
}

/**@brief Return the elementwise product of two vectors */
SIMD_FORCE_INLINE Vec3
operator*(const Vec3& v1, const Vec3& v2)
{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
	return Vec3(_mm_mul_ps(v1.mVec128, v2.mVec128));
#elif defined(DRX3D_USE_NEON)
	return Vec3(vmulq_f32(v1.mVec128, v2.mVec128));
#else
	return Vec3(
		v1.m_floats[0] * v2.m_floats[0],
		v1.m_floats[1] * v2.m_floats[1],
		v1.m_floats[2] * v2.m_floats[2]);
#endif
}

/**@brief Return the difference between two vectors */
SIMD_FORCE_INLINE Vec3
operator-(const Vec3& v1, const Vec3& v2)
{
#if defined DRX3D_USE_SIMD_VECTOR3 && (defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE))

	//	without _mm_and_ps this code causes slowdown in Concave moving
	__m128 r = _mm_sub_ps(v1.mVec128, v2.mVec128);
	return Vec3(_mm_and_ps(r, drx3dFFF0fMask));
#elif defined(DRX3D_USE_NEON)
	float32x4_t r = vsubq_f32(v1.mVec128, v2.mVec128);
	return Vec3((float32x4_t)vandq_s32((int32x4_t)r, drx3dFFF0Mask));
#else
	return Vec3(
		v1.m_floats[0] - v2.m_floats[0],
		v1.m_floats[1] - v2.m_floats[1],
		v1.m_floats[2] - v2.m_floats[2]);
#endif
}

/**@brief Return the negative of the vector */
SIMD_FORCE_INLINE Vec3
operator-(const Vec3& v)
{
#if defined DRX3D_USE_SIMD_VECTOR3 && (defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE))
	__m128 r = _mm_xor_ps(v.mVec128, drx3dMzeroMask);
	return Vec3(_mm_and_ps(r, drx3dFFF0fMask));
#elif defined(DRX3D_USE_NEON)
	return Vec3((SimdFloat4)veorq_s32((int32x4_t)v.mVec128, (int32x4_t)drx3dMzeroMask));
#else
	return Vec3(-v.m_floats[0], -v.m_floats[1], -v.m_floats[2]);
#endif
}

/**@brief Return the vector scaled by s */
SIMD_FORCE_INLINE Vec3
operator*(const Vec3& v, const Scalar& s)
{
#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
	__m128 vs = _mm_load_ss(&s);  //	(S 0 0 0)
	vs = drx3d_pshufd_ps(vs, 0x80);  //	(S S S 0.0)
	return Vec3(_mm_mul_ps(v.mVec128, vs));
#elif defined(DRX3D_USE_NEON)
	float32x4_t r = vmulq_n_f32(v.mVec128, s);
	return Vec3((float32x4_t)vandq_s32((int32x4_t)r, drx3dFFF0Mask));
#else
	return Vec3(v.m_floats[0] * s, v.m_floats[1] * s, v.m_floats[2] * s);
#endif
}

/**@brief Return the vector scaled by s */
SIMD_FORCE_INLINE Vec3
operator*(const Scalar& s, const Vec3& v)
{
	return v * s;
}

/**@brief Return the vector inversely scaled by s */
SIMD_FORCE_INLINE Vec3
operator/(const Vec3& v, const Scalar& s)
{
	FullAssert(s != Scalar(0.0));
#if 0  //defined(DRX3D_USE_SSE_IN_API)
// this code is not faster !
	__m128 vs = _mm_load_ss(&s);
	vs = _mm_div_ss(v1110, vs);
	vs = drx3d_pshufd_ps(vs, 0x00);	//	(S S S S)

	return Vec3(_mm_mul_ps(v.mVec128, vs));
#else
	return v * (Scalar(1.0) / s);
#endif
}

/**@brief Return the vector inversely scaled by s */
SIMD_FORCE_INLINE Vec3
operator/(const Vec3& v1, const Vec3& v2)
{
#if defined DRX3D_USE_SIMD_VECTOR3 && (defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE))
	__m128 vec = _mm_div_ps(v1.mVec128, v2.mVec128);
	vec = _mm_and_ps(vec, drx3dFFF0fMask);
	return Vec3(vec);
#elif defined(DRX3D_USE_NEON)
	float32x4_t x, y, v, m;

	x = v1.mVec128;
	y = v2.mVec128;

	v = vrecpeq_f32(y);     // v ~ 1/y
	m = vrecpsq_f32(y, v);  // m = (2-v*y)
	v = vmulq_f32(v, m);    // vv = v*m ~~ 1/y
	m = vrecpsq_f32(y, v);  // mm = (2-vv*y)
	v = vmulq_f32(v, x);    // x*vv
	v = vmulq_f32(v, m);    // (x*vv)*(2-vv*y) = x*(vv(2-vv*y)) ~~~ x/y

	return Vec3(v);
#else
	return Vec3(
		v1.m_floats[0] / v2.m_floats[0],
		v1.m_floats[1] / v2.m_floats[1],
		v1.m_floats[2] / v2.m_floats[2]);
#endif
}

/**@brief Return the dot product between two vectors */
SIMD_FORCE_INLINE Scalar
Dot(const Vec3& v1, const Vec3& v2)
{
	return v1.dot(v2);
}

/**@brief Return the distance squared between two vectors */
SIMD_FORCE_INLINE Scalar
Distance2(const Vec3& v1, const Vec3& v2)
{
	return v1.distance2(v2);
}

/**@brief Return the distance between two vectors */
SIMD_FORCE_INLINE Scalar
Distance(const Vec3& v1, const Vec3& v2)
{
	return v1.distance(v2);
}

/**@brief Return the angle between two vectors */
SIMD_FORCE_INLINE Scalar
Angle(const Vec3& v1, const Vec3& v2)
{
	return v1.angle(v2);
}

/**@brief Return the cross product of two vectors */
SIMD_FORCE_INLINE Vec3
Cross(const Vec3& v1, const Vec3& v2)
{
	return v1.cross(v2);
}

SIMD_FORCE_INLINE Scalar
Triple(const Vec3& v1, const Vec3& v2, const Vec3& v3)
{
	return v1.triple(v2, v3);
}

/**@brief Return the linear interpolation between two vectors
* @param v1 One vector
* @param v2 The other vector
* @param t The ration of this to v (t = 0 => return v1, t=1 => return v2) */
SIMD_FORCE_INLINE Vec3
lerp(const Vec3& v1, const Vec3& v2, const Scalar& t)
{
	return v1.lerp(v2, t);
}

SIMD_FORCE_INLINE Scalar Vec3::distance2(const Vec3& v) const
{
	return (v - *this).length2();
}

SIMD_FORCE_INLINE Scalar Vec3::distance(const Vec3& v) const
{
	return (v - *this).length();
}

SIMD_FORCE_INLINE Vec3 Vec3::normalized() const
{
	Vec3 nrm = *this;

	return nrm.normalize();
}

SIMD_FORCE_INLINE Vec3 Vec3::rotate(const Vec3& wAxis, const Scalar _angle) const
{
	// wAxis must be a unit lenght vector

#if defined DRX3D_USE_SIMD_VECTOR3 && defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)

	__m128 O = _mm_mul_ps(wAxis.mVec128, mVec128);
	Scalar ssin = Sin(_angle);
	__m128 C = wAxis.cross(mVec128).mVec128;
	O = _mm_and_ps(O, drx3dFFF0fMask);
	Scalar scos = Cos(_angle);

	__m128 vsin = _mm_load_ss(&ssin);  //	(S 0 0 0)
	__m128 vcos = _mm_load_ss(&scos);  //	(S 0 0 0)

	__m128 Y = drx3d_pshufd_ps(O, 0xC9);  //	(Y Z X 0)
	__m128 Z = drx3d_pshufd_ps(O, 0xD2);  //	(Z X Y 0)
	O = _mm_add_ps(O, Y);
	vsin = drx3d_pshufd_ps(vsin, 0x80);  //	(S S S 0)
	O = _mm_add_ps(O, Z);
	vcos = drx3d_pshufd_ps(vcos, 0x80);  //	(S S S 0)

	vsin = vsin * C;
	O = O * wAxis.mVec128;
	__m128 X = mVec128 - O;

	O = O + vsin;
	vcos = vcos * X;
	O = O + vcos;

	return Vec3(O);
#else
	Vec3 o = wAxis * wAxis.dot(*this);
	Vec3 _x = *this - o;
	Vec3 _y;

	_y = wAxis.cross(*this);

	return (o + _x * Cos(_angle) + _y * Sin(_angle));
#endif
}

SIMD_FORCE_INLINE long Vec3::maxDot(const Vec3* array, long array_count, Scalar& dotOut) const
{
#if (defined DRX3D_USE_SSE && defined DRX3D_USE_SIMD_VECTOR3 && defined DRX3D_USE_SSE_IN_API) || defined(DRX3D_USE_NEON)
#if defined _WIN32 || defined(DRX3D_USE_SSE)
	const long scalar_cutoff = 10;
	long _maxdot_large(const float* array, const float* vec, u64 array_count, float* dotOut);
#elif defined DRX3D_USE_NEON
	const long scalar_cutoff = 4;
	extern long (*_maxdot_large)(const float* array, const float* vec, u64 array_count, float* dotOut);
#endif
	if (array_count < scalar_cutoff)
#endif
	{
		Scalar maxDot1 = -SIMD_INFINITY;
		i32 i = 0;
		i32 ptIndex = -1;
		for (i = 0; i < array_count; i++)
		{
			Scalar dot = array[i].dot(*this);

			if (dot > maxDot1)
			{
				maxDot1 = dot;
				ptIndex = i;
			}
		}

		dotOut = maxDot1;
		return ptIndex;
	}
#if (defined DRX3D_USE_SSE && defined DRX3D_USE_SIMD_VECTOR3 && defined DRX3D_USE_SSE_IN_API) || defined(DRX3D_USE_NEON)
	return _maxdot_large((float*)array, (float*)&m_floats[0], array_count, &dotOut);
#endif
}

SIMD_FORCE_INLINE long Vec3::minDot(const Vec3* array, long array_count, Scalar& dotOut) const
{
#if (defined DRX3D_USE_SSE && defined DRX3D_USE_SIMD_VECTOR3 && defined DRX3D_USE_SSE_IN_API) || defined(DRX3D_USE_NEON)
#if defined DRX3D_USE_SSE
	const long scalar_cutoff = 10;
	long _mindot_large(const float* array, const float* vec, u64 array_count, float* dotOut);
#elif defined DRX3D_USE_NEON
	const long scalar_cutoff = 4;
	extern long (*_mindot_large)(const float* array, const float* vec, u64 array_count, float* dotOut);
#else
#error unhandled arch!
#endif

	if (array_count < scalar_cutoff)
#endif
	{
		Scalar minDot = SIMD_INFINITY;
		i32 i = 0;
		i32 ptIndex = -1;

		for (i = 0; i < array_count; i++)
		{
			Scalar dot = array[i].dot(*this);

			if (dot < minDot)
			{
				minDot = dot;
				ptIndex = i;
			}
		}

		dotOut = minDot;

		return ptIndex;
	}
#if (defined DRX3D_USE_SSE && defined DRX3D_USE_SIMD_VECTOR3 && defined DRX3D_USE_SSE_IN_API) || defined(DRX3D_USE_NEON)
	return _mindot_large((float*)array, (float*)&m_floats[0], array_count, &dotOut);
#endif  //DRX3D_USE_SIMD_VECTOR3
}

class Vec4 : public Vec3
{
public:
	SIMD_FORCE_INLINE Vec4() {}

	SIMD_FORCE_INLINE Vec4(const Scalar& _x, const Scalar& _y, const Scalar& _z, const Scalar& _w)
		: Vec3(_x, _y, _z)
	{
		m_floats[3] = _w;
	}

#if (defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)) || defined(DRX3D_USE_NEON)
	SIMD_FORCE_INLINE Vec4(const SimdFloat4 vec)
	{
		mVec128 = vec;
	}

	SIMD_FORCE_INLINE Vec4(const Vec3& rhs)
	{
		mVec128 = rhs.mVec128;
	}

	SIMD_FORCE_INLINE Vec4&
	operator=(const Vec4& v)
	{
		mVec128 = v.mVec128;
		return *this;
	}
#endif  // #if defined (DRX3D_USE_SSE_IN_API) || defined (DRX3D_USE_NEON)

	SIMD_FORCE_INLINE Vec4 absolute4() const
	{
#if defined DRX3D_USE_SIMD_VECTOR3 && defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)
		return Vec4(_mm_and_ps(mVec128, drx3dAbsfMask));
#elif defined(DRX3D_USE_NEON)
		return Vec4(vabsq_f32(mVec128));
#else
		return Vec4(
			Fabs(m_floats[0]),
			Fabs(m_floats[1]),
			Fabs(m_floats[2]),
			Fabs(m_floats[3]));
#endif
	}

	Scalar getW() const { return m_floats[3]; }

	SIMD_FORCE_INLINE i32 maxAxis4() const
	{
		i32 maxIndex = -1;
		Scalar maxVal = Scalar(-DRX3D_LARGE_FLOAT);
		if (m_floats[0] > maxVal)
		{
			maxIndex = 0;
			maxVal = m_floats[0];
		}
		if (m_floats[1] > maxVal)
		{
			maxIndex = 1;
			maxVal = m_floats[1];
		}
		if (m_floats[2] > maxVal)
		{
			maxIndex = 2;
			maxVal = m_floats[2];
		}
		if (m_floats[3] > maxVal)
		{
			maxIndex = 3;
		}

		return maxIndex;
	}

	SIMD_FORCE_INLINE i32 minAxis4() const
	{
		i32 minIndex = -1;
		Scalar minVal = Scalar(DRX3D_LARGE_FLOAT);
		if (m_floats[0] < minVal)
		{
			minIndex = 0;
			minVal = m_floats[0];
		}
		if (m_floats[1] < minVal)
		{
			minIndex = 1;
			minVal = m_floats[1];
		}
		if (m_floats[2] < minVal)
		{
			minIndex = 2;
			minVal = m_floats[2];
		}
		if (m_floats[3] < minVal)
		{
			minIndex = 3;
		}

		return minIndex;
	}

	SIMD_FORCE_INLINE i32 closestAxis4() const
	{
		return absolute4().maxAxis4();
	}

	/**@brief Set x,y,z and zero w
* @param x Value of x
* @param y Value of y
* @param z Value of z
*/

	/*		void getValue(Scalar *m) const
		{
			m[0] = m_floats[0];
			m[1] = m_floats[1];
			m[2] =m_floats[2];
		}
*/
	/**@brief Set the values
* @param x Value of x
* @param y Value of y
* @param z Value of z
* @param w Value of w
*/
	SIMD_FORCE_INLINE void setVal(const Scalar& _x, const Scalar& _y, const Scalar& _z, const Scalar& _w)
	{
		m_floats[0] = _x;
		m_floats[1] = _y;
		m_floats[2] = _z;
		m_floats[3] = _w;
	}
};

//SwapVector3Endian swaps vector endianness, useful for network and cross-platform serialization
SIMD_FORCE_INLINE void SwapScalarEndian(const Scalar& sourceVal, Scalar& destVal)
{
#ifdef DRX3D_USE_DOUBLE_PRECISION
	u8* dest = (u8*)&destVal;
	u8k* src = (u8k*)&sourceVal;
	dest[0] = src[7];
	dest[1] = src[6];
	dest[2] = src[5];
	dest[3] = src[4];
	dest[4] = src[3];
	dest[5] = src[2];
	dest[6] = src[1];
	dest[7] = src[0];
#else
	u8* dest = (u8*)&destVal;
	u8k* src = (u8k*)&sourceVal;
	dest[0] = src[3];
	dest[1] = src[2];
	dest[2] = src[1];
	dest[3] = src[0];
#endif  //DRX3D_USE_DOUBLE_PRECISION
}
//SwapVector3Endian swaps vector endianness, useful for network and cross-platform serialization
SIMD_FORCE_INLINE void SwapVector3Endian(const Vec3& sourceVec, Vec3& destVec)
{
	for (i32 i = 0; i < 4; i++)
	{
		SwapScalarEndian(sourceVec[i], destVec[i]);
	}
}

//UnSwapVector3Endian swaps vector endianness, useful for network and cross-platform serialization
SIMD_FORCE_INLINE void UnSwapVector3Endian(Vec3& vector)
{
	Vec3 swappedVec;
	for (i32 i = 0; i < 4; i++)
	{
		SwapScalarEndian(vector[i], swappedVec[i]);
	}
	vector = swappedVec;
}

template <class T>
SIMD_FORCE_INLINE void PlaneSpace1(const T& n, T& p, T& q)
{
	if (Fabs(n[2]) > SIMDSQRT12)
	{
		// choose p in y-z plane
		Scalar a = n[1] * n[1] + n[2] * n[2];
		Scalar k = RecipSqrt(a);
		p[0] = 0;
		p[1] = -n[2] * k;
		p[2] = n[1] * k;
		// set q = n x p
		q[0] = a * k;
		q[1] = -n[0] * p[2];
		q[2] = n[0] * p[1];
	}
	else
	{
		// choose p in x-y plane
		Scalar a = n[0] * n[0] + n[1] * n[1];
		Scalar k = RecipSqrt(a);
		p[0] = -n[1] * k;
		p[1] = n[0] * k;
		p[2] = 0;
		// set q = n x p
		q[0] = -n[2] * p[1];
		q[1] = n[2] * p[0];
		q[2] = a * k;
	}
}

struct Vec3FloatData
{
	float m_floats[4];
};

struct Vec3DoubleData
{
	double m_floats[4];
};

SIMD_FORCE_INLINE void Vec3::serializeFloat(struct Vec3FloatData& dataOut) const
{
	///could also do a memcpy, check if it is worth it
	for (i32 i = 0; i < 4; i++)
		dataOut.m_floats[i] = float(m_floats[i]);
}

SIMD_FORCE_INLINE void Vec3::deSerializeFloat(const struct Vec3FloatData& dataIn)
{
	for (i32 i = 0; i < 4; i++)
		m_floats[i] = Scalar(dataIn.m_floats[i]);
}

SIMD_FORCE_INLINE void Vec3::serializeDouble(struct Vec3DoubleData& dataOut) const
{
	///could also do a memcpy, check if it is worth it
	for (i32 i = 0; i < 4; i++)
		dataOut.m_floats[i] = double(m_floats[i]);
}

SIMD_FORCE_INLINE void Vec3::deSerializeDouble(const struct Vec3DoubleData& dataIn)
{
	for (i32 i = 0; i < 4; i++)
		m_floats[i] = Scalar(dataIn.m_floats[i]);
}

SIMD_FORCE_INLINE void Vec3::serialize(struct Vec3Data& dataOut) const
{
	///could also do a memcpy, check if it is worth it
	for (i32 i = 0; i < 4; i++)
		dataOut.m_floats[i] = m_floats[i];
}

SIMD_FORCE_INLINE void Vec3::deSerialize(const struct Vec3FloatData& dataIn)
{
	for (i32 i = 0; i < 4; i++)
		m_floats[i] = (Scalar)dataIn.m_floats[i];
}

SIMD_FORCE_INLINE void Vec3::deSerialize(const struct Vec3DoubleData& dataIn)
{
	for (i32 i = 0; i < 4; i++)
		m_floats[i] = (Scalar)dataIn.m_floats[i];
}

inline Vec3 MakeVector3(Scalar x, Scalar y, Scalar z)
{
	Vec3 tmp;
	tmp.setVal(x, y, z);
	return tmp;
}

inline Vec3 MakeVector3(Scalar x, Scalar y, Scalar z, Scalar w)
{
	Vec3 tmp;
	tmp.setVal(x, y, z);
	tmp.setW(w);
	return tmp;
}

inline Vec4 MakeVector4(Scalar x, Scalar y, Scalar z, Scalar w)
{
	Vec4 tmp;
	tmp.setVal(x, y, z, w);
	return tmp;
}

#if defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)

inline Vec3 MakeVector3(SimdFloat4 v)
{
	Vec3 tmp;
	tmp.set128(v);
	return tmp;
}

inline Vec4 MakeVector4(SimdFloat4 vec)
{
	Vec4 tmp;
	tmp.set128(vec);
	return tmp;
}

#endif

#endif  //DRX3D_VECTOR3_H
