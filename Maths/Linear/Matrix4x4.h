#ifndef MATRIX4x4_H
#define MATRIX4x4_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Quat.h>

/// This 4x4 matrix class is extremely limited, just created for the purpose of
/// accumulating transform matrices in COLLADA .dae files
ATTRIBUTE_ALIGNED16(class)
Matrix4x4
{
	Vec4 m_el[4];

public:
	Matrix4x4()
	{
	}
	Matrix4x4(const Scalar& xx, const Scalar& xy, const Scalar& xz, const Scalar& xw,
				const Scalar& yx, const Scalar& yy, const Scalar& yz, const Scalar& yw,
				const Scalar& zx, const Scalar& zy, const Scalar& zz, const Scalar& zw,
				const Scalar& wx, const Scalar& wy, const Scalar& wz, const Scalar& ww)
	{
		setVal(xx, xy, xz, xw,
				 yx, yy, yz, yw,
				 zx, zy, zz, zw,
				 wx, wy, wz, ww);
	}

	~Matrix4x4()
	{
	}

	inline void setVal(const Scalar& xx, const Scalar& xy, const Scalar& xz, const Scalar& xw,
						 const Scalar& yx, const Scalar& yy, const Scalar& yz, const Scalar& yw,
						 const Scalar& zx, const Scalar& zy, const Scalar& zz, const Scalar& zw,
						 const Scalar& wx, const Scalar& wy, const Scalar& wz, const Scalar& ww)
	{
		m_el[0].setVal(xx, xy, xz, xw);
		m_el[1].setVal(yx, yy, yz, yw);
		m_el[2].setVal(zx, zy, zz, zw);
		m_el[3].setVal(wx, wy, wz, ww);
	}

	inline void setIdentity()
	{
		m_el[0].setVal(1, 0, 0, 0);
		m_el[1].setVal(0, 1, 0, 0);
		m_el[2].setVal(0, 0, 1, 0);
		m_el[3].setVal(0, 0, 0, 1);
	}
	inline void setPureRotation(const Quat& orn)
	{
		setIdentity();

		Matrix3x3 m3(orn);
		for (i32 i = 0; i < 3; i++)
		{
			for (i32 j = 0; j < 3; j++)
			{
				m_el[i][j] = m3[i][j];
			}
		}
	}

	inline void setPureScaling(const Vec3& scale)
	{
		m_el[0].setVal(scale[0], 0, 0, 0);
		m_el[1].setVal(0, scale[1], 0, 0);
		m_el[2].setVal(0, 0, scale[2], 0);
		m_el[3].setVal(0, 0, 0, 1);
	}

	inline void setPureTranslation(const Vec3& pos)
	{
		m_el[0].setVal(1, 0, 0, pos[0]);
		m_el[1].setVal(0, 1, 0, pos[1]);
		m_el[2].setVal(0, 0, 1, pos[2]);
		m_el[3].setVal(0, 0, 0, 1);
	}
	SIMD_FORCE_INLINE const Vec4& operator[](i32 i) const
	{
		FullAssert(0 <= i && i < 3);
		return m_el[i];
	}

	SIMD_FORCE_INLINE Scalar tdotx(const Vec4& v) const
	{
		return m_el[0].x() * v.x() + m_el[1].x() * v.y() + m_el[2].x() * v.z() + m_el[3].x() * v.w();
	}
	SIMD_FORCE_INLINE Scalar tdoty(const Vec4& v) const
	{
		return m_el[0].y() * v.x() + m_el[1].y() * v.y() + m_el[2].y() * v.z() + m_el[3].y() * v.w();
	}
	SIMD_FORCE_INLINE Scalar tdotz(const Vec4& v) const
	{
		return m_el[0].z() * v.x() + m_el[1].z() * v.y() + m_el[2].z() * v.z() + m_el[3].z() * v.w();
	}
	SIMD_FORCE_INLINE Scalar tdotw(const Vec4& v) const
	{
		return m_el[0].w() * v.x() + m_el[1].w() * v.y() + m_el[2].w() * v.z() + m_el[3].w() * v.w();
	}

	SIMD_FORCE_INLINE Matrix4x4&
	operator*=(const Matrix4x4& m)
	{
		setVal(
			m.tdotx(m_el[0]), m.tdoty(m_el[0]), m.tdotz(m_el[0]), m.tdotw(m_el[0]),
			m.tdotx(m_el[1]), m.tdoty(m_el[1]), m.tdotz(m_el[1]), m.tdotw(m_el[1]),
			m.tdotx(m_el[2]), m.tdoty(m_el[2]), m.tdotz(m_el[2]), m.tdotw(m_el[2]),
			m.tdotx(m_el[3]), m.tdoty(m_el[3]), m.tdotz(m_el[3]), m.tdotw(m_el[3]));
		return *this;
	}
};

inline Scalar Dot4(const Vec4& v0, const Vec4& v1)
{
	return v0.x() * v1.x() + v0.y() * v1.y() + v0.z() * v1.z() + v0.w() * v1.w();
}
SIMD_FORCE_INLINE Vec3
operator*(const Matrix4x4& m, const Vec3& v1)
{
	Vec4 v(v1[0], v1[1], v1[2], 1);
	return Vec3(Dot4(m[0], v), Dot4(m[1], v), Dot4(m[2], v));
}

SIMD_FORCE_INLINE Matrix4x4
operator*(const Matrix4x4& m1, Matrix4x4& m2)
{
	return Matrix4x4(
		m2.tdotx(m1[0]), m2.tdoty(m1[0]), m2.tdotz(m1[0]), m2.tdotw(m1[0]),
		m2.tdotx(m1[1]), m2.tdoty(m1[1]), m2.tdotz(m1[1]), m2.tdotw(m1[1]),
		m2.tdotx(m1[2]), m2.tdoty(m1[2]), m2.tdotz(m1[2]), m2.tdotw(m1[2]),
		m2.tdotx(m1[3]), m2.tdoty(m1[3]), m2.tdotz(m1[3]), m2.tdotw(m1[3]));
}

#endif  //MATRIX4x4_H
