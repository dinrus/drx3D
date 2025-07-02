#ifndef DRX3D_SIMD_QUADWORD_H
#define DRX3D_SIMD_QUADWORD_H

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/MinMax.h>

#if defined(__CELLOS_LV2) && defined(__SPU__)
#include <altivec.h>
#endif

/**@brief The QuadWord class is base class for Vec3 and Quat.
 * Some issues under PS3 Linux with IBM 2.1 SDK, gcc compiler prevent from using aligned quadword.
 */
#ifndef USE_LIBSPE2
ATTRIBUTE_ALIGNED16(class)
QuadWord
#else
class QuadWord
#endif
{
protected:
#if defined(__SPU__) && defined(__CELLOS_LV2__)
	union {
		vec_float4 mVec128;
		Scalar m_floats[4];
	};

public:
	vec_float4 get128() const
	{
		return mVec128;
	}

protected:
#else  //__CELLOS_LV2__ __SPU__

#if defined(DRX3D_USE_SSE) || defined(DRX3D_USE_NEON)
	union {
		SimdFloat4 mVec128;
		Scalar m_floats[4];
	};

public:
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
#endif  // DRX3D_USE_SSE

#endif  //__CELLOS_LV2__ __SPU__

public:
#if (defined(DRX3D_USE_SSE_IN_API) && defined(DRX3D_USE_SSE)) || defined(DRX3D_USE_NEON)

	// Set Vector
	SIMD_FORCE_INLINE QuadWord(const SimdFloat4 vec)
	{
		mVec128 = vec;
	}

	// Copy constructor
	SIMD_FORCE_INLINE QuadWord(const QuadWord& rhs)
	{
		mVec128 = rhs.mVec128;
	}

	// Assignment Operator
	SIMD_FORCE_INLINE QuadWord&
	operator=(const QuadWord& v)
	{
		mVec128 = v.mVec128;

		return *this;
	}

#endif

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

	SIMD_FORCE_INLINE bool operator==(const QuadWord& other) const
	{
#ifdef DRX3D_USE_SSE
		return (0xf == _mm_movemask_ps((__m128)_mm_cmpeq_ps(mVec128, other.mVec128)));
#else
		return ((m_floats[3] == other.m_floats[3]) &&
				(m_floats[2] == other.m_floats[2]) &&
				(m_floats[1] == other.m_floats[1]) &&
				(m_floats[0] == other.m_floats[0]));
#endif
	}

	SIMD_FORCE_INLINE bool operator!=(const QuadWord& other) const
	{
		return !(*this == other);
	}

	/**@brief Set x,y,z and zero w
   * @param x Value of x
   * @param y Value of y
   * @param z Value of z
   */
	SIMD_FORCE_INLINE void setVal(const Scalar& _x, const Scalar& _y, const Scalar& _z)
	{
		m_floats[0] = _x;
		m_floats[1] = _y;
		m_floats[2] = _z;
		m_floats[3] = 0.f;
	}

	/*		void getValue(Scalar *m) const
		{
			m[0] = m_floats[0];
			m[1] = m_floats[1];
			m[2] = m_floats[2];
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
	/**@brief No initialization constructor */
	SIMD_FORCE_INLINE QuadWord()
	//	:m_floats[0](Scalar(0.)),m_floats[1](Scalar(0.)),m_floats[2](Scalar(0.)),m_floats[3](Scalar(0.))
	{
	}

	/**@brief Three argument constructor (zeros w)
   * @param x Value of x
   * @param y Value of y
   * @param z Value of z
   */
	SIMD_FORCE_INLINE QuadWord(const Scalar& _x, const Scalar& _y, const Scalar& _z)
	{
		m_floats[0] = _x, m_floats[1] = _y, m_floats[2] = _z, m_floats[3] = 0.0f;
	}

	/**@brief Initializing constructor
   * @param x Value of x
   * @param y Value of y
   * @param z Value of z
   * @param w Value of w
   */
	SIMD_FORCE_INLINE QuadWord(const Scalar& _x, const Scalar& _y, const Scalar& _z, const Scalar& _w)
	{
		m_floats[0] = _x, m_floats[1] = _y, m_floats[2] = _z, m_floats[3] = _w;
	}

	/**@brief Set each element to the max of the current values and the values of another QuadWord
   * @param other The other QuadWord to compare with
   */
	SIMD_FORCE_INLINE void setMax(const QuadWord& other)
	{
#ifdef DRX3D_USE_SSE
		mVec128 = _mm_max_ps(mVec128, other.mVec128);
#elif defined(DRX3D_USE_NEON)
		mVec128 = vmaxq_f32(mVec128, other.mVec128);
#else
		SetMax(m_floats[0], other.m_floats[0]);
		SetMax(m_floats[1], other.m_floats[1]);
		SetMax(m_floats[2], other.m_floats[2]);
		SetMax(m_floats[3], other.m_floats[3]);
#endif
	}
	/**@brief Set each element to the min of the current values and the values of another QuadWord
   * @param other The other QuadWord to compare with
   */
	SIMD_FORCE_INLINE void setMin(const QuadWord& other)
	{
#ifdef DRX3D_USE_SSE
		mVec128 = _mm_min_ps(mVec128, other.mVec128);
#elif defined(DRX3D_USE_NEON)
		mVec128 = vminq_f32(mVec128, other.mVec128);
#else
		SetMin(m_floats[0], other.m_floats[0]);
		SetMin(m_floats[1], other.m_floats[1]);
		SetMin(m_floats[2], other.m_floats[2]);
		SetMin(m_floats[3], other.m_floats[3]);
#endif
	}
};

#endif  //DRX3D_SIMD_QUADWORD_H
