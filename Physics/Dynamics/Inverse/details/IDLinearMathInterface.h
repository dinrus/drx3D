#ifndef IDLINEARMATHINTERFACE_H_
#define IDLINEARMATHINTERFACE_H_

#include <cstdlib>
#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/MatrixX.h>

#define DRX3D_ID_HAVE_MAT3X

namespace drx3d_inverse
{
class vec3;
class vecx;
class mat33;
typedef MatrixX<idScalar> matxx;

class vec3 : public Vec3
{
public:
	vec3() : Vec3() {}
	vec3(const Vec3& btv) { *this = btv; }
	idScalar& operator()(i32 i) { return (*this)[i]; }
	const idScalar& operator()(i32 i) const { return (*this)[i]; }
	i32 size() const { return 3; }
	const vec3& operator=(const Vec3& rhs)
	{
		*static_cast<Vec3*>(this) = rhs;
		return *this;
	}
};

class mat33 : public Matrix3x3
{
public:
	mat33() : Matrix3x3() {}
	mat33(const Matrix3x3& btm) { *this = btm; }
	idScalar& operator()(i32 i, i32 j) { return (*this)[i][j]; }
	const idScalar& operator()(i32 i, i32 j) const { return (*this)[i][j]; }
	const mat33& operator=(const Matrix3x3& rhs)
	{
		*static_cast<Matrix3x3*>(this) = rhs;
		return *this;
	}
	friend mat33 operator*(const idScalar& s, const mat33& a);
	friend mat33 operator/(const mat33& a, const idScalar& s);
};

inline mat33 operator/(const mat33& a, const idScalar& s) { return a * (1.0 / s); }

inline mat33 operator*(const idScalar& s, const mat33& a) { return a * s; }

class vecx : public VectorX<idScalar>
{
public:
	vecx(i32 size) : VectorX<idScalar>(size) {}
	const vecx& operator=(const VectorX<idScalar>& rhs)
	{
		*static_cast<VectorX<idScalar>*>(this) = rhs;
		return *this;
	}

	idScalar& operator()(i32 i) { return (*this)[i]; }
	const idScalar& operator()(i32 i) const { return (*this)[i]; }

	friend vecx operator*(const vecx& a, const idScalar& s);
	friend vecx operator*(const idScalar& s, const vecx& a);

	friend vecx operator+(const vecx& a, const vecx& b);
	friend vecx operator-(const vecx& a, const vecx& b);
	friend vecx operator/(const vecx& a, const idScalar& s);
};

inline vecx operator*(const vecx& a, const idScalar& s)
{
	vecx result(a.size());
	for (i32 i = 0; i < result.size(); i++)
	{
		result(i) = a(i) * s;
	}
	return result;
}
inline vecx operator*(const idScalar& s, const vecx& a) { return a * s; }
inline vecx operator+(const vecx& a, const vecx& b)
{
	vecx result(a.size());
	// TODO: error handling for a.size() != b.size()??
	if (a.size() != b.size())
	{
		drx3d_id_error_message("size missmatch. a.size()= %d, b.size()= %d\n", a.size(), b.size());
		abort();
	}
	for (i32 i = 0; i < a.size(); i++)
	{
		result(i) = a(i) + b(i);
	}

	return result;
}

inline vecx operator-(const vecx& a, const vecx& b)
{
	vecx result(a.size());
	// TODO: error handling for a.size() != b.size()??
	if (a.size() != b.size())
	{
		drx3d_id_error_message("size missmatch. a.size()= %d, b.size()= %d\n", a.size(), b.size());
		abort();
	}
	for (i32 i = 0; i < a.size(); i++)
	{
		result(i) = a(i) - b(i);
	}
	return result;
}
inline vecx operator/(const vecx& a, const idScalar& s)
{
	vecx result(a.size());
	for (i32 i = 0; i < result.size(); i++)
	{
		result(i) = a(i) / s;
	}

	return result;
}

// use MatrixX to implement 3xX matrix
class mat3x : public matxx
{
public:
	mat3x() {}
	mat3x(const mat3x& rhs)
	{
		matxx::resize(rhs.rows(), rhs.cols());
		*this = rhs;
	}
	mat3x(i32 rows, i32 cols) : matxx(3, cols)
	{
	}
	void operator=(const mat3x& rhs)
	{
		if (m_cols != rhs.m_cols)
		{
			drx3d_id_error_message("size missmatch, cols= %d but rhs.cols= %d\n", cols(), rhs.cols());
			abort();
		}
		for (i32 i = 0; i < rows(); i++)
		{
			for (i32 k = 0; k < cols(); k++)
			{
				setElem(i, k, rhs(i, k));
			}
		}
	}
	void setZero()
	{
		matxx::setZero();
	}
};

inline vec3 operator*(const mat3x& a, const vecx& b)
{
	vec3 result;
	if (a.cols() != b.size())
	{
		drx3d_id_error_message("size missmatch. a.cols()= %d, b.size()= %d\n", a.cols(), b.size());
		abort();
	}
	result(0) = 0.0;
	result(1) = 0.0;
	result(2) = 0.0;
	for (i32 i = 0; i < b.size(); i++)
	{
		for (i32 k = 0; k < 3; k++)
		{
			result(k) += a(k, i) * b(i);
		}
	}
	return result;
}

inline void resize(mat3x& m, idArrayIdx size)
{
	m.resize(3, size);
	m.setZero();
}

inline void setMatxxElem(const idArrayIdx row, const idArrayIdx col, const idScalar val, matxx* m)
{
	m->setElem(row, col, val);
}

inline void setMat3xElem(const idArrayIdx row, const idArrayIdx col, const idScalar val, mat3x* m)
{
	m->setElem(row, col, val);
}

}  // namespace drx3d_inverse

#endif  // IDLINEARMATHINTERFACE_H_
