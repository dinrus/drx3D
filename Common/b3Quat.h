#ifndef D3_SIMD__QUATERNION_H_
#define D3_SIMD__QUATERNION_H_

#include <drx3D/Common/b3Vec3.h>
#include  <drx3D/Common/b3QuadWord.h>

#ifdef D3_USE_SSE

const __m128 D3_ATTRIBUTE_ALIGNED16(b3vOnes) = {1.0f, 1.0f, 1.0f, 1.0f};

#endif

#if defined(D3_USE_SSE) || defined(D3_USE_NEON)

const b3SimdFloat4 D3_ATTRIBUTE_ALIGNED16(b3vQInv) = {-0.0f, -0.0f, -0.0f, +0.0f};
const b3SimdFloat4 D3_ATTRIBUTE_ALIGNED16(b3vPPPM) = {+0.0f, +0.0f, +0.0f, -0.0f};

#endif

/**@brief The b3Quat implements quaternion to perform linear algebra rotations in combination with b3Matrix3x3, b3Vec3 and b3Transform. */
class b3Quat : public b3QuadWord
{
public:
	/**@brief No initialization constructor */
	b3Quat() {}

#if (defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)) || defined(D3_USE_NEON)
	// Set Vector
	D3_FORCE_INLINE b3Quat(const b3SimdFloat4 vec)
	{
		mVec128 = vec;
	}

	// Copy constructor
	D3_FORCE_INLINE b3Quat(const b3Quat& rhs)
	{
		mVec128 = rhs.mVec128;
	}

	// Assignment Operator
	D3_FORCE_INLINE b3Quat&
	operator=(const b3Quat& v)
	{
		mVec128 = v.mVec128;

		return *this;
	}

#endif

	//		template <typename b3Scalar>
	//		explicit Quaternion(const b3Scalar *v) : Tuple4<b3Scalar>(v) {}
	/**@brief Constructor from scalars */
	b3Quat(const b3Scalar& _x, const b3Scalar& _y, const b3Scalar& _z, const b3Scalar& _w)
		: b3QuadWord(_x, _y, _z, _w)
	{
		//drx3DAssert(!((_x==1.f) && (_y==0.f) && (_z==0.f) && (_w==0.f)));
	}
	/**@brief Axis angle Constructor
   * @param axis The axis which the rotation is around
   * @param angle The magnitude of the rotation around the angle (Radians) */
	b3Quat(const b3Vec3& _axis, const b3Scalar& _angle)
	{
		setRotation(_axis, _angle);
	}
	/**@brief Constructor from Euler angles
   * @param yaw Angle around Y unless D3_EULER_DEFAULT_ZYX defined then Z
   * @param pitch Angle around X unless D3_EULER_DEFAULT_ZYX defined then Y
   * @param roll Angle around Z unless D3_EULER_DEFAULT_ZYX defined then X */
	b3Quat(const b3Scalar& yaw, const b3Scalar& pitch, const b3Scalar& roll)
	{
#ifndef D3_EULER_DEFAULT_ZYX
		setEuler(yaw, pitch, roll);
#else
		setEulerZYX(yaw, pitch, roll);
#endif
	}
	/**@brief Set the rotation using axis angle notation
   * @param axis The axis around which to rotate
   * @param angle The magnitude of the rotation in Radians */
	void setRotation(const b3Vec3& axis1, const b3Scalar& _angle)
	{
		b3Vec3 axis = axis1;
		axis.safeNormalize();

		b3Scalar d = axis.length();
		drx3DAssert(d != b3Scalar(0.0));
		if (d < D3_EPSILON)
		{
			setVal(0, 0, 0, 1);
		}
		else
		{
			b3Scalar s = b3Sin(_angle * b3Scalar(0.5)) / d;
			setVal(axis.getX() * s, axis.getY() * s, axis.getZ() * s,
				b3Cos(_angle * b3Scalar(0.5)));
		}
	}
	/**@brief Set the quaternion using Euler angles
   * @param yaw Angle around Y
   * @param pitch Angle around X
   * @param roll Angle around Z */
	void setEuler(const b3Scalar& yaw, const b3Scalar& pitch, const b3Scalar& roll)
	{
		b3Scalar halfYaw = b3Scalar(yaw) * b3Scalar(0.5);
		b3Scalar halfPitch = b3Scalar(pitch) * b3Scalar(0.5);
		b3Scalar halfRoll = b3Scalar(roll) * b3Scalar(0.5);
		b3Scalar cosYaw = b3Cos(halfYaw);
		b3Scalar sinYaw = b3Sin(halfYaw);
		b3Scalar cosPitch = b3Cos(halfPitch);
		b3Scalar sinPitch = b3Sin(halfPitch);
		b3Scalar cosRoll = b3Cos(halfRoll);
		b3Scalar sinRoll = b3Sin(halfRoll);
		setVal(cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
				 cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
				 sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw,
				 cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw);
	}

	/**@brief Set the quaternion using euler angles
   * @param yaw Angle around Z
   * @param pitch Angle around Y
   * @param roll Angle around X */
	void setEulerZYX(const b3Scalar& yawZ, const b3Scalar& pitchY, const b3Scalar& rollX)
	{
		b3Scalar halfYaw = b3Scalar(yawZ) * b3Scalar(0.5);
		b3Scalar halfPitch = b3Scalar(pitchY) * b3Scalar(0.5);
		b3Scalar halfRoll = b3Scalar(rollX) * b3Scalar(0.5);
		b3Scalar cosYaw = b3Cos(halfYaw);
		b3Scalar sinYaw = b3Sin(halfYaw);
		b3Scalar cosPitch = b3Cos(halfPitch);
		b3Scalar sinPitch = b3Sin(halfPitch);
		b3Scalar cosRoll = b3Cos(halfRoll);
		b3Scalar sinRoll = b3Sin(halfRoll);
		setVal(sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw,   //x
				 cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,   //y
				 cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,   //z
				 cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw);  //formerly yzx
		normalize();
	}

	/**@brief Get the euler angles from this quaternion
	   * @param yaw Angle around Z
	   * @param pitch Angle around Y
	   * @param roll Angle around X */
	void getEulerZYX(b3Scalar& yawZ, b3Scalar& pitchY, b3Scalar& rollX) const
	{
		b3Scalar squ;
		b3Scalar sqx;
		b3Scalar sqy;
		b3Scalar sqz;
		b3Scalar sarg;
		sqx = m_floats[0] * m_floats[0];
		sqy = m_floats[1] * m_floats[1];
		sqz = m_floats[2] * m_floats[2];
		squ = m_floats[3] * m_floats[3];
		rollX = b3Atan2(2 * (m_floats[1] * m_floats[2] + m_floats[3] * m_floats[0]), squ - sqx - sqy + sqz);
		sarg = b3Scalar(-2.) * (m_floats[0] * m_floats[2] - m_floats[3] * m_floats[1]);
		pitchY = sarg <= b3Scalar(-1.0) ? b3Scalar(-0.5) * D3_PI : (sarg >= b3Scalar(1.0) ? b3Scalar(0.5) * D3_PI : b3Asin(sarg));
		yawZ = b3Atan2(2 * (m_floats[0] * m_floats[1] + m_floats[3] * m_floats[2]), squ + sqx - sqy - sqz);
	}

	/**@brief Add two quaternions
   * @param q The quaternion to add to this one */
	D3_FORCE_INLINE b3Quat& operator+=(const b3Quat& q)
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		mVec128 = _mm_add_ps(mVec128, q.mVec128);
#elif defined(D3_USE_NEON)
		mVec128 = vaddq_f32(mVec128, q.mVec128);
#else
		m_floats[0] += q.getX();
		m_floats[1] += q.getY();
		m_floats[2] += q.getZ();
		m_floats[3] += q.m_floats[3];
#endif
		return *this;
	}

	/**@brief Subtract out a quaternion
   * @param q The quaternion to subtract from this one */
	b3Quat& operator-=(const b3Quat& q)
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		mVec128 = _mm_sub_ps(mVec128, q.mVec128);
#elif defined(D3_USE_NEON)
		mVec128 = vsubq_f32(mVec128, q.mVec128);
#else
		m_floats[0] -= q.getX();
		m_floats[1] -= q.getY();
		m_floats[2] -= q.getZ();
		m_floats[3] -= q.m_floats[3];
#endif
		return *this;
	}

	/**@brief Scale this quaternion
   * @param s The scalar to scale by */
	b3Quat& operator*=(const b3Scalar& s)
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		__m128 vs = _mm_load_ss(&s);  //	(S 0 0 0)
		vs = b3_pshufd_ps(vs, 0);     //	(S S S S)
		mVec128 = _mm_mul_ps(mVec128, vs);
#elif defined(D3_USE_NEON)
		mVec128 = vmulq_n_f32(mVec128, s);
#else
		m_floats[0] *= s;
		m_floats[1] *= s;
		m_floats[2] *= s;
		m_floats[3] *= s;
#endif
		return *this;
	}

	/**@brief Multiply this quaternion by q on the right
   * @param q The other quaternion
   * Equivilant to this = this * q */
	b3Quat& operator*=(const b3Quat& q)
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		__m128 vQ2 = q.get128();

		__m128 A1 = b3_pshufd_ps(mVec128, D3_SHUFFLE(0, 1, 2, 0));
		__m128 B1 = b3_pshufd_ps(vQ2, D3_SHUFFLE(3, 3, 3, 0));

		A1 = A1 * B1;

		__m128 A2 = b3_pshufd_ps(mVec128, D3_SHUFFLE(1, 2, 0, 1));
		__m128 B2 = b3_pshufd_ps(vQ2, D3_SHUFFLE(2, 0, 1, 1));

		A2 = A2 * B2;

		B1 = b3_pshufd_ps(mVec128, D3_SHUFFLE(2, 0, 1, 2));
		B2 = b3_pshufd_ps(vQ2, D3_SHUFFLE(1, 2, 0, 2));

		B1 = B1 * B2;  //	A3 *= B3

		mVec128 = b3_splat_ps(mVec128, 3);  //	A0
		mVec128 = mVec128 * vQ2;            //	A0 * B0

		A1 = A1 + A2;                  //	AB12
		mVec128 = mVec128 - B1;        //	AB03 = AB0 - AB3
		A1 = _mm_xor_ps(A1, b3vPPPM);  //	change sign of the last element
		mVec128 = mVec128 + A1;        //	AB03 + AB12

#elif defined(D3_USE_NEON)

		float32x4_t vQ1 = mVec128;
		float32x4_t vQ2 = q.get128();
		float32x4_t A0, A1, B1, A2, B2, A3, B3;
		float32x2_t vQ1zx, vQ2wx, vQ1yz, vQ2zx, vQ2yz, vQ2xz;

		{
			float32x2x2_t tmp;
			tmp = vtrn_f32(vget_high_f32(vQ1), vget_low_f32(vQ1));  // {z x}, {w y}
			vQ1zx = tmp.val[0];

			tmp = vtrn_f32(vget_high_f32(vQ2), vget_low_f32(vQ2));  // {z x}, {w y}
			vQ2zx = tmp.val[0];
		}
		vQ2wx = vext_f32(vget_high_f32(vQ2), vget_low_f32(vQ2), 1);

		vQ1yz = vext_f32(vget_low_f32(vQ1), vget_high_f32(vQ1), 1);

		vQ2yz = vext_f32(vget_low_f32(vQ2), vget_high_f32(vQ2), 1);
		vQ2xz = vext_f32(vQ2zx, vQ2zx, 1);

		A1 = vcombine_f32(vget_low_f32(vQ1), vQ1zx);                     // X Y  z x
		B1 = vcombine_f32(vdup_lane_f32(vget_high_f32(vQ2), 1), vQ2wx);  // W W  W X

		A2 = vcombine_f32(vQ1yz, vget_low_f32(vQ1));
		B2 = vcombine_f32(vQ2zx, vdup_lane_f32(vget_low_f32(vQ2), 1));

		A3 = vcombine_f32(vQ1zx, vQ1yz);  // Z X Y Z
		B3 = vcombine_f32(vQ2yz, vQ2xz);  // Y Z x z

		A1 = vmulq_f32(A1, B1);
		A2 = vmulq_f32(A2, B2);
		A3 = vmulq_f32(A3, B3);                           //	A3 *= B3
		A0 = vmulq_lane_f32(vQ2, vget_high_f32(vQ1), 1);  //	A0 * B0

		A1 = vaddq_f32(A1, A2);  //	AB12 = AB1 + AB2
		A0 = vsubq_f32(A0, A3);  //	AB03 = AB0 - AB3

		//	change the sign of the last element
		A1 = (b3SimdFloat4)veorq_s32((int32x4_t)A1, (int32x4_t)b3vPPPM);
		A0 = vaddq_f32(A0, A1);  //	AB03 + AB12

		mVec128 = A0;
#else
		setVal(
			m_floats[3] * q.getX() + m_floats[0] * q.m_floats[3] + m_floats[1] * q.getZ() - m_floats[2] * q.getY(),
			m_floats[3] * q.getY() + m_floats[1] * q.m_floats[3] + m_floats[2] * q.getX() - m_floats[0] * q.getZ(),
			m_floats[3] * q.getZ() + m_floats[2] * q.m_floats[3] + m_floats[0] * q.getY() - m_floats[1] * q.getX(),
			m_floats[3] * q.m_floats[3] - m_floats[0] * q.getX() - m_floats[1] * q.getY() - m_floats[2] * q.getZ());
#endif
		return *this;
	}
	/**@brief Return the dot product between this quaternion and another
   * @param q The other quaternion */
	b3Scalar dot(const b3Quat& q) const
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		__m128 vd;

		vd = _mm_mul_ps(mVec128, q.mVec128);

		__m128 t = _mm_movehl_ps(vd, vd);
		vd = _mm_add_ps(vd, t);
		t = _mm_shuffle_ps(vd, vd, 0x55);
		vd = _mm_add_ss(vd, t);

		return _mm_cvtss_f32(vd);
#elif defined(D3_USE_NEON)
		float32x4_t vd = vmulq_f32(mVec128, q.mVec128);
		float32x2_t x = vpadd_f32(vget_low_f32(vd), vget_high_f32(vd));
		x = vpadd_f32(x, x);
		return vget_lane_f32(x, 0);
#else
		return m_floats[0] * q.getX() +
			   m_floats[1] * q.getY() +
			   m_floats[2] * q.getZ() +
			   m_floats[3] * q.m_floats[3];
#endif
	}

	/**@brief Return the length squared of the quaternion */
	b3Scalar length2() const
	{
		return dot(*this);
	}

	/**@brief Return the length of the quaternion */
	b3Scalar length() const
	{
		return b3Sqrt(length2());
	}

	/**@brief Normalize the quaternion
   * Such that x^2 + y^2 + z^2 +w^2 = 1 */
	b3Quat& normalize()
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		__m128 vd;

		vd = _mm_mul_ps(mVec128, mVec128);

		__m128 t = _mm_movehl_ps(vd, vd);
		vd = _mm_add_ps(vd, t);
		t = _mm_shuffle_ps(vd, vd, 0x55);
		vd = _mm_add_ss(vd, t);

		vd = _mm_sqrt_ss(vd);
		vd = _mm_div_ss(b3vOnes, vd);
		vd = b3_pshufd_ps(vd, 0);  // splat
		mVec128 = _mm_mul_ps(mVec128, vd);

		return *this;
#else
		return *this /= length();
#endif
	}

	/**@brief Return a scaled version of this quaternion
   * @param s The scale factor */
	D3_FORCE_INLINE b3Quat
	operator*(const b3Scalar& s) const
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		__m128 vs = _mm_load_ss(&s);  //	(S 0 0 0)
		vs = b3_pshufd_ps(vs, 0x00);  //	(S S S S)

		return b3Quat(_mm_mul_ps(mVec128, vs));
#elif defined(D3_USE_NEON)
		return b3Quat(vmulq_n_f32(mVec128, s));
#else
		return b3Quat(getX() * s, getY() * s, getZ() * s, m_floats[3] * s);
#endif
	}

	/**@brief Return an inversely scaled versionof this quaternion
   * @param s The inverse scale factor */
	b3Quat operator/(const b3Scalar& s) const
	{
		drx3DAssert(s != b3Scalar(0.0));
		return *this * (b3Scalar(1.0) / s);
	}

	/**@brief Inversely scale this quaternion
   * @param s The scale factor */
	b3Quat& operator/=(const b3Scalar& s)
	{
		drx3DAssert(s != b3Scalar(0.0));
		return *this *= b3Scalar(1.0) / s;
	}

	/**@brief Return a normalized version of this quaternion */
	b3Quat normalized() const
	{
		return *this / length();
	}
	/**@brief Return the angle between this quaternion and the other
   * @param q The other quaternion */
	b3Scalar angle(const b3Quat& q) const
	{
		b3Scalar s = b3Sqrt(length2() * q.length2());
		drx3DAssert(s != b3Scalar(0.0));
		return b3Acos(dot(q) / s);
	}
	/**@brief Return the angle of rotation represented by this quaternion */
	b3Scalar getAngle() const
	{
		b3Scalar s = b3Scalar(2.) * b3Acos(m_floats[3]);
		return s;
	}

	/**@brief Return the axis of the rotation represented by this quaternion */
	b3Vec3 getAxis() const
	{
		b3Scalar s_squared = 1.f - m_floats[3] * m_floats[3];

		if (s_squared < b3Scalar(10.) * D3_EPSILON)  //Check for divide by zero
			return b3MakeVector3(1.0, 0.0, 0.0);     // Arbitrary
		b3Scalar s = 1.f / b3Sqrt(s_squared);
		return b3MakeVector3(m_floats[0] * s, m_floats[1] * s, m_floats[2] * s);
	}

	/**@brief Return the inverse of this quaternion */
	b3Quat inverse() const
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		return b3Quat(_mm_xor_ps(mVec128, b3vQInv));
#elif defined(D3_USE_NEON)
		return b3Quat((b3SimdFloat4)veorq_s32((int32x4_t)mVec128, (int32x4_t)b3vQInv));
#else
		return b3Quat(-m_floats[0], -m_floats[1], -m_floats[2], m_floats[3]);
#endif
	}

	/**@brief Return the sum of this quaternion and the other
   * @param q2 The other quaternion */
	D3_FORCE_INLINE b3Quat
	operator+(const b3Quat& q2) const
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		return b3Quat(_mm_add_ps(mVec128, q2.mVec128));
#elif defined(D3_USE_NEON)
		return b3Quat(vaddq_f32(mVec128, q2.mVec128));
#else
		const b3Quat& q1 = *this;
		return b3Quat(q1.getX() + q2.getX(), q1.getY() + q2.getY(), q1.getZ() + q2.getZ(), q1.m_floats[3] + q2.m_floats[3]);
#endif
	}

	/**@brief Return the difference between this quaternion and the other
   * @param q2 The other quaternion */
	D3_FORCE_INLINE b3Quat
	operator-(const b3Quat& q2) const
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		return b3Quat(_mm_sub_ps(mVec128, q2.mVec128));
#elif defined(D3_USE_NEON)
		return b3Quat(vsubq_f32(mVec128, q2.mVec128));
#else
		const b3Quat& q1 = *this;
		return b3Quat(q1.getX() - q2.getX(), q1.getY() - q2.getY(), q1.getZ() - q2.getZ(), q1.m_floats[3] - q2.m_floats[3]);
#endif
	}

	/**@brief Return the negative of this quaternion
   * This simply negates each element */
	D3_FORCE_INLINE b3Quat operator-() const
	{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
		return b3Quat(_mm_xor_ps(mVec128, b3vMzeroMask));
#elif defined(D3_USE_NEON)
		return b3Quat((b3SimdFloat4)veorq_s32((int32x4_t)mVec128, (int32x4_t)b3vMzeroMask));
#else
		const b3Quat& q2 = *this;
		return b3Quat(-q2.getX(), -q2.getY(), -q2.getZ(), -q2.m_floats[3]);
#endif
	}
	/**@todo document this and it's use */
	D3_FORCE_INLINE b3Quat farthest(const b3Quat& qd) const
	{
		b3Quat diff, sum;
		diff = *this - qd;
		sum = *this + qd;
		if (diff.dot(diff) > sum.dot(sum))
			return qd;
		return (-qd);
	}

	/**@todo document this and it's use */
	D3_FORCE_INLINE b3Quat nearest(const b3Quat& qd) const
	{
		b3Quat diff, sum;
		diff = *this - qd;
		sum = *this + qd;
		if (diff.dot(diff) < sum.dot(sum))
			return qd;
		return (-qd);
	}

	/**@brief Return the quaternion which is the result of Spherical Linear Interpolation between this and the other quaternion
   * @param q The other quaternion to interpolate with
   * @param t The ratio between this and q to interpolate.  If t = 0 the result is this, if t=1 the result is q.
   * Slerp interpolates assuming constant velocity.  */
	b3Quat slerp(const b3Quat& q, const b3Scalar& t) const
	{
		b3Scalar magnitude = b3Sqrt(length2() * q.length2());
		drx3DAssert(magnitude > b3Scalar(0));

		b3Scalar product = dot(q) / magnitude;
		if (b3Fabs(product) < b3Scalar(1))
		{
			// Take care of long angle case see http://en.wikipedia.org/wiki/Slerp
			const b3Scalar sign = (product < 0) ? b3Scalar(-1) : b3Scalar(1);

			const b3Scalar theta = b3Acos(sign * product);
			const b3Scalar s1 = b3Sin(sign * t * theta);
			const b3Scalar d = b3Scalar(1.0) / b3Sin(theta);
			const b3Scalar s0 = b3Sin((b3Scalar(1.0) - t) * theta);

			return b3Quat(
				(m_floats[0] * s0 + q.getX() * s1) * d,
				(m_floats[1] * s0 + q.getY() * s1) * d,
				(m_floats[2] * s0 + q.getZ() * s1) * d,
				(m_floats[3] * s0 + q.m_floats[3] * s1) * d);
		}
		else
		{
			return *this;
		}
	}

	static const b3Quat& getIdentity()
	{
		static const b3Quat identityQuaternion(b3Scalar(0.), b3Scalar(0.), b3Scalar(0.), b3Scalar(1.));
		return identityQuaternion;
	}

	D3_FORCE_INLINE const b3Scalar& getW() const { return m_floats[3]; }
};

/**@brief Return the product of two quaternions */
D3_FORCE_INLINE b3Quat
operator*(const b3Quat& q1, const b3Quat& q2)
{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
	__m128 vQ1 = q1.get128();
	__m128 vQ2 = q2.get128();
	__m128 A0, A1, B1, A2, B2;

	A1 = b3_pshufd_ps(vQ1, D3_SHUFFLE(0, 1, 2, 0));  // X Y  z x     //      vtrn
	B1 = b3_pshufd_ps(vQ2, D3_SHUFFLE(3, 3, 3, 0));  // W W  W X     // vdup vext

	A1 = A1 * B1;

	A2 = b3_pshufd_ps(vQ1, D3_SHUFFLE(1, 2, 0, 1));  // Y Z  X Y     // vext
	B2 = b3_pshufd_ps(vQ2, D3_SHUFFLE(2, 0, 1, 1));  // z x  Y Y     // vtrn vdup

	A2 = A2 * B2;

	B1 = b3_pshufd_ps(vQ1, D3_SHUFFLE(2, 0, 1, 2));  // z x Y Z      // vtrn vext
	B2 = b3_pshufd_ps(vQ2, D3_SHUFFLE(1, 2, 0, 2));  // Y Z x z      // vext vtrn

	B1 = B1 * B2;  //	A3 *= B3

	A0 = b3_splat_ps(vQ1, 3);  //	A0
	A0 = A0 * vQ2;             //	A0 * B0

	A1 = A1 + A2;  //	AB12
	A0 = A0 - B1;  //	AB03 = AB0 - AB3

	A1 = _mm_xor_ps(A1, b3vPPPM);  //	change sign of the last element
	A0 = A0 + A1;                  //	AB03 + AB12

	return b3Quat(A0);

#elif defined(D3_USE_NEON)

	float32x4_t vQ1 = q1.get128();
	float32x4_t vQ2 = q2.get128();
	float32x4_t A0, A1, B1, A2, B2, A3, B3;
	float32x2_t vQ1zx, vQ2wx, vQ1yz, vQ2zx, vQ2yz, vQ2xz;

	{
		float32x2x2_t tmp;
		tmp = vtrn_f32(vget_high_f32(vQ1), vget_low_f32(vQ1));  // {z x}, {w y}
		vQ1zx = tmp.val[0];

		tmp = vtrn_f32(vget_high_f32(vQ2), vget_low_f32(vQ2));  // {z x}, {w y}
		vQ2zx = tmp.val[0];
	}
	vQ2wx = vext_f32(vget_high_f32(vQ2), vget_low_f32(vQ2), 1);

	vQ1yz = vext_f32(vget_low_f32(vQ1), vget_high_f32(vQ1), 1);

	vQ2yz = vext_f32(vget_low_f32(vQ2), vget_high_f32(vQ2), 1);
	vQ2xz = vext_f32(vQ2zx, vQ2zx, 1);

	A1 = vcombine_f32(vget_low_f32(vQ1), vQ1zx);                     // X Y  z x
	B1 = vcombine_f32(vdup_lane_f32(vget_high_f32(vQ2), 1), vQ2wx);  // W W  W X

	A2 = vcombine_f32(vQ1yz, vget_low_f32(vQ1));
	B2 = vcombine_f32(vQ2zx, vdup_lane_f32(vget_low_f32(vQ2), 1));

	A3 = vcombine_f32(vQ1zx, vQ1yz);  // Z X Y Z
	B3 = vcombine_f32(vQ2yz, vQ2xz);  // Y Z x z

	A1 = vmulq_f32(A1, B1);
	A2 = vmulq_f32(A2, B2);
	A3 = vmulq_f32(A3, B3);                           //	A3 *= B3
	A0 = vmulq_lane_f32(vQ2, vget_high_f32(vQ1), 1);  //	A0 * B0

	A1 = vaddq_f32(A1, A2);  //	AB12 = AB1 + AB2
	A0 = vsubq_f32(A0, A3);  //	AB03 = AB0 - AB3

	//	change the sign of the last element
	A1 = (b3SimdFloat4)veorq_s32((int32x4_t)A1, (int32x4_t)b3vPPPM);
	A0 = vaddq_f32(A0, A1);  //	AB03 + AB12

	return b3Quat(A0);

#else
	return b3Quat(
		q1.getW() * q2.getX() + q1.getX() * q2.getW() + q1.getY() * q2.getZ() - q1.getZ() * q2.getY(),
		q1.getW() * q2.getY() + q1.getY() * q2.getW() + q1.getZ() * q2.getX() - q1.getX() * q2.getZ(),
		q1.getW() * q2.getZ() + q1.getZ() * q2.getW() + q1.getX() * q2.getY() - q1.getY() * q2.getX(),
		q1.getW() * q2.getW() - q1.getX() * q2.getX() - q1.getY() * q2.getY() - q1.getZ() * q2.getZ());
#endif
}

D3_FORCE_INLINE b3Quat
operator*(const b3Quat& q, const b3Vec3& w)
{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
	__m128 vQ1 = q.get128();
	__m128 vQ2 = w.get128();
	__m128 A1, B1, A2, B2, A3, B3;

	A1 = b3_pshufd_ps(vQ1, D3_SHUFFLE(3, 3, 3, 0));
	B1 = b3_pshufd_ps(vQ2, D3_SHUFFLE(0, 1, 2, 0));

	A1 = A1 * B1;

	A2 = b3_pshufd_ps(vQ1, D3_SHUFFLE(1, 2, 0, 1));
	B2 = b3_pshufd_ps(vQ2, D3_SHUFFLE(2, 0, 1, 1));

	A2 = A2 * B2;

	A3 = b3_pshufd_ps(vQ1, D3_SHUFFLE(2, 0, 1, 2));
	B3 = b3_pshufd_ps(vQ2, D3_SHUFFLE(1, 2, 0, 2));

	A3 = A3 * B3;  //	A3 *= B3

	A1 = A1 + A2;                  //	AB12
	A1 = _mm_xor_ps(A1, b3vPPPM);  //	change sign of the last element
	A1 = A1 - A3;                  //	AB123 = AB12 - AB3

	return b3Quat(A1);

#elif defined(D3_USE_NEON)

	float32x4_t vQ1 = q.get128();
	float32x4_t vQ2 = w.get128();
	float32x4_t A1, B1, A2, B2, A3, B3;
	float32x2_t vQ1wx, vQ2zx, vQ1yz, vQ2yz, vQ1zx, vQ2xz;

	vQ1wx = vext_f32(vget_high_f32(vQ1), vget_low_f32(vQ1), 1);
	{
		float32x2x2_t tmp;

		tmp = vtrn_f32(vget_high_f32(vQ2), vget_low_f32(vQ2));  // {z x}, {w y}
		vQ2zx = tmp.val[0];

		tmp = vtrn_f32(vget_high_f32(vQ1), vget_low_f32(vQ1));  // {z x}, {w y}
		vQ1zx = tmp.val[0];
	}

	vQ1yz = vext_f32(vget_low_f32(vQ1), vget_high_f32(vQ1), 1);

	vQ2yz = vext_f32(vget_low_f32(vQ2), vget_high_f32(vQ2), 1);
	vQ2xz = vext_f32(vQ2zx, vQ2zx, 1);

	A1 = vcombine_f32(vdup_lane_f32(vget_high_f32(vQ1), 1), vQ1wx);  // W W  W X
	B1 = vcombine_f32(vget_low_f32(vQ2), vQ2zx);                     // X Y  z x

	A2 = vcombine_f32(vQ1yz, vget_low_f32(vQ1));
	B2 = vcombine_f32(vQ2zx, vdup_lane_f32(vget_low_f32(vQ2), 1));

	A3 = vcombine_f32(vQ1zx, vQ1yz);  // Z X Y Z
	B3 = vcombine_f32(vQ2yz, vQ2xz);  // Y Z x z

	A1 = vmulq_f32(A1, B1);
	A2 = vmulq_f32(A2, B2);
	A3 = vmulq_f32(A3, B3);  //	A3 *= B3

	A1 = vaddq_f32(A1, A2);  //	AB12 = AB1 + AB2

	//	change the sign of the last element
	A1 = (b3SimdFloat4)veorq_s32((int32x4_t)A1, (int32x4_t)b3vPPPM);

	A1 = vsubq_f32(A1, A3);  //	AB123 = AB12 - AB3

	return b3Quat(A1);

#else
	return b3Quat(
		q.getW() * w.getX() + q.getY() * w.getZ() - q.getZ() * w.getY(),
		q.getW() * w.getY() + q.getZ() * w.getX() - q.getX() * w.getZ(),
		q.getW() * w.getZ() + q.getX() * w.getY() - q.getY() * w.getX(),
		-q.getX() * w.getX() - q.getY() * w.getY() - q.getZ() * w.getZ());
#endif
}

D3_FORCE_INLINE b3Quat
operator*(const b3Vec3& w, const b3Quat& q)
{
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
	__m128 vQ1 = w.get128();
	__m128 vQ2 = q.get128();
	__m128 A1, B1, A2, B2, A3, B3;

	A1 = b3_pshufd_ps(vQ1, D3_SHUFFLE(0, 1, 2, 0));  // X Y  z x
	B1 = b3_pshufd_ps(vQ2, D3_SHUFFLE(3, 3, 3, 0));  // W W  W X

	A1 = A1 * B1;

	A2 = b3_pshufd_ps(vQ1, D3_SHUFFLE(1, 2, 0, 1));
	B2 = b3_pshufd_ps(vQ2, D3_SHUFFLE(2, 0, 1, 1));

	A2 = A2 * B2;

	A3 = b3_pshufd_ps(vQ1, D3_SHUFFLE(2, 0, 1, 2));
	B3 = b3_pshufd_ps(vQ2, D3_SHUFFLE(1, 2, 0, 2));

	A3 = A3 * B3;  //	A3 *= B3

	A1 = A1 + A2;                  //	AB12
	A1 = _mm_xor_ps(A1, b3vPPPM);  //	change sign of the last element
	A1 = A1 - A3;                  //	AB123 = AB12 - AB3

	return b3Quat(A1);

#elif defined(D3_USE_NEON)

	float32x4_t vQ1 = w.get128();
	float32x4_t vQ2 = q.get128();
	float32x4_t A1, B1, A2, B2, A3, B3;
	float32x2_t vQ1zx, vQ2wx, vQ1yz, vQ2zx, vQ2yz, vQ2xz;

	{
		float32x2x2_t tmp;

		tmp = vtrn_f32(vget_high_f32(vQ1), vget_low_f32(vQ1));  // {z x}, {w y}
		vQ1zx = tmp.val[0];

		tmp = vtrn_f32(vget_high_f32(vQ2), vget_low_f32(vQ2));  // {z x}, {w y}
		vQ2zx = tmp.val[0];
	}
	vQ2wx = vext_f32(vget_high_f32(vQ2), vget_low_f32(vQ2), 1);

	vQ1yz = vext_f32(vget_low_f32(vQ1), vget_high_f32(vQ1), 1);

	vQ2yz = vext_f32(vget_low_f32(vQ2), vget_high_f32(vQ2), 1);
	vQ2xz = vext_f32(vQ2zx, vQ2zx, 1);

	A1 = vcombine_f32(vget_low_f32(vQ1), vQ1zx);                     // X Y  z x
	B1 = vcombine_f32(vdup_lane_f32(vget_high_f32(vQ2), 1), vQ2wx);  // W W  W X

	A2 = vcombine_f32(vQ1yz, vget_low_f32(vQ1));
	B2 = vcombine_f32(vQ2zx, vdup_lane_f32(vget_low_f32(vQ2), 1));

	A3 = vcombine_f32(vQ1zx, vQ1yz);  // Z X Y Z
	B3 = vcombine_f32(vQ2yz, vQ2xz);  // Y Z x z

	A1 = vmulq_f32(A1, B1);
	A2 = vmulq_f32(A2, B2);
	A3 = vmulq_f32(A3, B3);  //	A3 *= B3

	A1 = vaddq_f32(A1, A2);  //	AB12 = AB1 + AB2

	//	change the sign of the last element
	A1 = (b3SimdFloat4)veorq_s32((int32x4_t)A1, (int32x4_t)b3vPPPM);

	A1 = vsubq_f32(A1, A3);  //	AB123 = AB12 - AB3

	return b3Quat(A1);

#else
	return b3Quat(
		+w.getX() * q.getW() + w.getY() * q.getZ() - w.getZ() * q.getY(),
		+w.getY() * q.getW() + w.getZ() * q.getX() - w.getX() * q.getZ(),
		+w.getZ() * q.getW() + w.getX() * q.getY() - w.getY() * q.getX(),
		-w.getX() * q.getX() - w.getY() * q.getY() - w.getZ() * q.getZ());
#endif
}

/**@brief Calculate the dot product between two quaternions */
D3_FORCE_INLINE b3Scalar
b3Dot(const b3Quat& q1, const b3Quat& q2)
{
	return q1.dot(q2);
}

/**@brief Return the length of a quaternion */
D3_FORCE_INLINE b3Scalar
b3Length(const b3Quat& q)
{
	return q.length();
}

/**@brief Return the angle between two quaternions*/
D3_FORCE_INLINE b3Scalar
b3Angle(const b3Quat& q1, const b3Quat& q2)
{
	return q1.angle(q2);
}

/**@brief Return the inverse of a quaternion*/
D3_FORCE_INLINE b3Quat
b3Inverse(const b3Quat& q)
{
	return q.inverse();
}

/**@brief Return the result of spherical linear interpolation betwen two quaternions
 * @param q1 The first quaternion
 * @param q2 The second quaternion
 * @param t The ration between q1 and q2.  t = 0 return q1, t=1 returns q2
 * Slerp assumes constant velocity between positions. */
D3_FORCE_INLINE b3Quat
b3Slerp(const b3Quat& q1, const b3Quat& q2, const b3Scalar& t)
{
	return q1.slerp(q2, t);
}

D3_FORCE_INLINE b3Quat
b3QuatMul(const b3Quat& rot0, const b3Quat& rot1)
{
	return rot0 * rot1;
}

D3_FORCE_INLINE b3Quat
b3QuatNormalized(const b3Quat& orn)
{
	return orn.normalized();
}

D3_FORCE_INLINE b3Vec3
b3QuatRotate(const b3Quat& rotation, const b3Vec3& v)
{
	b3Quat q = rotation * v;
	q *= rotation.inverse();
#if defined(D3_USE_SSE_IN_API) && defined(D3_USE_SSE)
	return b3MakeVector3(_mm_and_ps(q.get128(), b3vFFF0fMask));
#elif defined(D3_USE_NEON)
	return b3MakeVector3((float32x4_t)vandq_s32((int32x4_t)q.get128(), b3vFFF0Mask));
#else
	return b3MakeVector3(q.getX(), q.getY(), q.getZ());
#endif
}

D3_FORCE_INLINE b3Quat
b3ShortestArcQuaternion(const b3Vec3& v0, const b3Vec3& v1)  // Game Programming Gems 2.10. make sure v0,v1 are normalized
{
	b3Vec3 c = v0.cross(v1);
	b3Scalar d = v0.dot(v1);

	if (d < -1.0 + D3_EPSILON)
	{
		b3Vec3 n, unused;
		b3PlaneSpace1(v0, n, unused);
		return b3Quat(n.getX(), n.getY(), n.getZ(), 0.0f);  // just pick any vector that is orthogonal to v0
	}

	b3Scalar s = b3Sqrt((1.0f + d) * 2.0f);
	b3Scalar rs = 1.0f / s;

	return b3Quat(c.getX() * rs, c.getY() * rs, c.getZ() * rs, s * 0.5f);
}

D3_FORCE_INLINE b3Quat
b3ShortestArcQuaternionNormalize2(b3Vec3& v0, b3Vec3& v1)
{
	v0.normalize();
	v1.normalize();
	return b3ShortestArcQuaternion(v0, v1);
}

#endif  //D3_SIMD__QUATERNION_H_
