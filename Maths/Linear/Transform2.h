#ifndef DRX3D_TRANSFORM_H
#define DRX3D_TRANSFORM_H

#include <drx3D/Maths/Linear/Matrix3x3.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define Transform2Data Transform2DoubleData
#else
#define Transform2Data Transform2FloatData
#endif

/**@brief The Transform2 class supports rigid transforms with only translation and rotation and no scaling/shear.
 *It can be used in combination with Vec3, Quat and Matrix3x3 linear algebra classes. */
ATTRIBUTE_ALIGNED16(class)
Transform2
{
	///Storage for the rotation
	Matrix3x3 m_basis;
	///Storage for the translation
	Vec3 m_origin;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();
	/**@brief No initialization constructor */
	Transform2() {}
	/**@brief Constructor from Quat (optional Vec3 )
   * @param q Rotation from quaternion
   * @param c Translation from Vector (default 0,0,0) */
	explicit SIMD_FORCE_INLINE Transform2(const Quat& q,
										   const Vec3& c = Vec3(Scalar(0), Scalar(0), Scalar(0)))
		: m_basis(q),
		  m_origin(c)
	{
	}

	/**@brief Constructor from Matrix3x3 (optional Vec3)
   * @param b Rotation from Matrix
   * @param c Translation from Vector default (0,0,0)*/
	explicit SIMD_FORCE_INLINE Transform2(const Matrix3x3& b,
										   const Vec3& c = Vec3(Scalar(0), Scalar(0), Scalar(0)))
		: m_basis(b),
		  m_origin(c)
	{
	}
	/**@brief Copy constructor */
	SIMD_FORCE_INLINE Transform2(const Transform2& other)
		: m_basis(other.m_basis),
		  m_origin(other.m_origin)
	{
	}
	/**@brief Assignment Operator */
	SIMD_FORCE_INLINE Transform2& operator=(const Transform2& other)
	{
		m_basis = other.m_basis;
		m_origin = other.m_origin;
		return *this;
	}

	/**@brief Set the current transform as the value of the product of two transforms
   * @param t1 Transform2 1
   * @param t2 Transform2 2
   * This = Transform21 * Transform22 */
	SIMD_FORCE_INLINE void mult(const Transform2& t1, const Transform2& t2)
	{
		m_basis = t1.m_basis * t2.m_basis;
		m_origin = t1(t2.m_origin);
	}

	/*		void multInverseLeft(const Transform2& t1, const Transform2& t2) {
			Vec3 v = t2.m_origin - t1.m_origin;
			m_basis = MultTransposeLeft(t1.m_basis, t2.m_basis);
			m_origin = v * t1.m_basis;
		}
		*/

	/**@brief Return the transform of the vector */
	SIMD_FORCE_INLINE Vec3 operator()(const Vec3& x) const
	{
		return x.dot3(m_basis[0], m_basis[1], m_basis[2]) + m_origin;
	}

	/**@brief Return the transform of the vector */
	SIMD_FORCE_INLINE Vec3 operator*(const Vec3& x) const
	{
		return (*this)(x);
	}

	/**@brief Return the transform of the Quat */
	SIMD_FORCE_INLINE Quat operator*(const Quat& q) const
	{
		return getRotation() * q;
	}

	/**@brief Return the basis matrix for the rotation */
	SIMD_FORCE_INLINE Matrix3x3& getBasis() { return m_basis; }
	/**@brief Return the basis matrix for the rotation */
	SIMD_FORCE_INLINE const Matrix3x3& getBasis() const { return m_basis; }

	/**@brief Return the origin vector translation */
	SIMD_FORCE_INLINE Vec3& getOrigin() { return m_origin; }
	/**@brief Return the origin vector translation */
	SIMD_FORCE_INLINE const Vec3& getOrigin() const { return m_origin; }

	/**@brief Return a quaternion representing the rotation */
	Quat getRotation() const
	{
		Quat q;
		m_basis.getRotation(q);
		return q;
	}

	/**@brief Set from an array
   * @param m A pointer to a 16 element array (12 rotation(row major padded on the right by 1), and 3 translation */
	void setFromOpenGLMatrix(const Scalar* m)
	{
		m_basis.setFromOpenGLSubMatrix(m);
		m_origin.setVal(m[12], m[13], m[14]);
	}

	/**@brief Fill an array representation
   * @param m A pointer to a 16 element array (12 rotation(row major padded on the right by 1), and 3 translation */
	void getOpenGLMatrix(Scalar * m) const
	{
		m_basis.getOpenGLSubMatrix(m);
		m[12] = m_origin.x();
		m[13] = m_origin.y();
		m[14] = m_origin.z();
		m[15] = Scalar(1.0);
	}

	/**@brief Set the translational element
   * @param origin The vector to set the translation to */
	SIMD_FORCE_INLINE void setOrigin(const Vec3& origin)
	{
		m_origin = origin;
	}

	SIMD_FORCE_INLINE Vec3 invXform(const Vec3& inVec) const;

	/**@brief Set the rotational element by Matrix3x3 */
	SIMD_FORCE_INLINE void setBasis(const Matrix3x3& basis)
	{
		m_basis = basis;
	}

	/**@brief Set the rotational element by Quat */
	SIMD_FORCE_INLINE void setRotation(const Quat& q)
	{
		m_basis.setRotation(q);
	}

	/**@brief Set this transformation to the identity */
	void setIdentity()
	{
		m_basis.setIdentity();
		m_origin.setVal(Scalar(0.0), Scalar(0.0), Scalar(0.0));
	}

	/**@brief Multiply this Transform2 by another(this = this * another)
   * @param t The other transform */
	Transform2& operator*=(const Transform2& t)
	{
		m_origin += m_basis * t.m_origin;
		m_basis *= t.m_basis;
		return *this;
	}

	/**@brief Return the inverse of this transform */
	Transform2 inverse() const
	{
		Matrix3x3 inv = m_basis.transpose();
		return Transform2(inv, inv * -m_origin);
	}

	/**@brief Return the inverse of this transform times the other transform
   * @param t The other transform
   * return this.inverse() * the other */
	Transform2 inverseTimes(const Transform2& t) const;

	/**@brief Return the product of this transform and the other */
	Transform2 operator*(const Transform2& t) const;

	/**@brief Return an identity transform */
	static const Transform2& getIdentity()
	{
		static const Transform2 identityTransform2(Matrix3x3::getIdentity());
		return identityTransform2;
	}

	void serialize(struct Transform2Data & dataOut) const;

	void serializeFloat(struct Transform2FloatData & dataOut) const;

	void deSerialize(const struct Transform2Data& dataIn);

	void deSerializeDouble(const struct Transform2DoubleData& dataIn);

	void deSerializeFloat(const struct Transform2FloatData& dataIn);
};

SIMD_FORCE_INLINE Vec3
Transform2::invXform(const Vec3& inVec) const
{
	Vec3 v = inVec - m_origin;
	return (m_basis.transpose() * v);
}

SIMD_FORCE_INLINE Transform2
Transform2::inverseTimes(const Transform2& t) const
{
	Vec3 v = t.getOrigin() - m_origin;
	return Transform2(m_basis.transposeTimes(t.m_basis),
					   v * m_basis);
}

SIMD_FORCE_INLINE Transform2
	Transform2::operator*(const Transform2& t) const
{
	return Transform2(m_basis * t.m_basis,
					   (*this)(t.m_origin));
}

/**@brief Test if two transforms have all elements equal */
SIMD_FORCE_INLINE bool operator==(const Transform2& t1, const Transform2& t2)
{
	return (t1.getBasis() == t2.getBasis() &&
			t1.getOrigin() == t2.getOrigin());
}

///for serialization
struct Transform2FloatData
{
	Matrix3x3FloatData m_basis;
	Vec3FloatData m_origin;
};

struct Transform2DoubleData
{
	Matrix3x3DoubleData m_basis;
	Vec3DoubleData m_origin;
};

SIMD_FORCE_INLINE void Transform2::serialize(Transform2Data& dataOut) const
{
	m_basis.serialize(dataOut.m_basis);
	m_origin.serialize(dataOut.m_origin);
}

SIMD_FORCE_INLINE void Transform2::serializeFloat(Transform2FloatData& dataOut) const
{
	m_basis.serializeFloat(dataOut.m_basis);
	m_origin.serializeFloat(dataOut.m_origin);
}

SIMD_FORCE_INLINE void Transform2::deSerialize(const Transform2Data& dataIn)
{
	m_basis.deSerialize(dataIn.m_basis);
	m_origin.deSerialize(dataIn.m_origin);
}

SIMD_FORCE_INLINE void Transform2::deSerializeFloat(const Transform2FloatData& dataIn)
{
	m_basis.deSerializeFloat(dataIn.m_basis);
	m_origin.deSerializeFloat(dataIn.m_origin);
}

SIMD_FORCE_INLINE void Transform2::deSerializeDouble(const Transform2DoubleData& dataIn)
{
	m_basis.deSerializeDouble(dataIn.m_basis);
	m_origin.deSerializeDouble(dataIn.m_origin);
}

#endif  //DRX3D_TRANSFORM_H
