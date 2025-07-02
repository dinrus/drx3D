///These spatial algebra classes are used for MultiBody,
///see BulletDynamics/MultiBody

#ifndef DRX3D_SPATIAL_ALGEBRA_H
#define DRX3D_SPATIAL_ALGEBRA_H

#include <drx3D/Maths/Linear/Matrix3x3.h>

struct SpatialForceVector
{
	Vec3 m_topVec, m_bottomVec;
	//
	SpatialForceVector() { setZero(); }
	SpatialForceVector(const Vec3 &angular, const Vec3 &linear) : m_topVec(linear), m_bottomVec(angular) {}
	SpatialForceVector(const Scalar &ax, const Scalar &ay, const Scalar &az, const Scalar &lx, const Scalar &ly, const Scalar &lz)
	{
		setVal(ax, ay, az, lx, ly, lz);
	}
	//
	void setVector(const Vec3 &angular, const Vec3 &linear)
	{
		m_topVec = linear;
		m_bottomVec = angular;
	}
	void setVal(const Scalar &ax, const Scalar &ay, const Scalar &az, const Scalar &lx, const Scalar &ly, const Scalar &lz)
	{
		m_bottomVec.setVal(ax, ay, az);
		m_topVec.setVal(lx, ly, lz);
	}
	//
	void addVector(const Vec3 &angular, const Vec3 &linear)
	{
		m_topVec += linear;
		m_bottomVec += angular;
	}
	void addValue(const Scalar &ax, const Scalar &ay, const Scalar &az, const Scalar &lx, const Scalar &ly, const Scalar &lz)
	{
		m_bottomVec[0] += ax;
		m_bottomVec[1] += ay;
		m_bottomVec[2] += az;
		m_topVec[0] += lx;
		m_topVec[1] += ly;
		m_topVec[2] += lz;
	}
	//
	const Vec3 &getLinear() const { return m_topVec; }
	const Vec3 &getAngular() const { return m_bottomVec; }
	//
	void setLinear(const Vec3 &linear) { m_topVec = linear; }
	void setAngular(const Vec3 &angular) { m_bottomVec = angular; }
	//
	void addAngular(const Vec3 &angular) { m_bottomVec += angular; }
	void addLinear(const Vec3 &linear) { m_topVec += linear; }
	//
	void setZero()
	{
		m_topVec.setZero();
		m_bottomVec.setZero();
	}
	//
	SpatialForceVector &operator+=(const SpatialForceVector &vec)
	{
		m_topVec += vec.m_topVec;
		m_bottomVec += vec.m_bottomVec;
		return *this;
	}
	SpatialForceVector &operator-=(const SpatialForceVector &vec)
	{
		m_topVec -= vec.m_topVec;
		m_bottomVec -= vec.m_bottomVec;
		return *this;
	}
	SpatialForceVector operator-(const SpatialForceVector &vec) const { return SpatialForceVector(m_bottomVec - vec.m_bottomVec, m_topVec - vec.m_topVec); }
	SpatialForceVector operator+(const SpatialForceVector &vec) const { return SpatialForceVector(m_bottomVec + vec.m_bottomVec, m_topVec + vec.m_topVec); }
	SpatialForceVector operator-() const { return SpatialForceVector(-m_bottomVec, -m_topVec); }
	SpatialForceVector operator*(const Scalar &s) const { return SpatialForceVector(s * m_bottomVec, s * m_topVec); }
	//SpatialForceVector & operator = (const SpatialForceVector &vec) { m_topVec = vec.m_topVec; m_bottomVec = vec.m_bottomVec; return *this; }
};

struct SpatialMotionVector
{
	Vec3 m_topVec, m_bottomVec;
	//
	SpatialMotionVector() { setZero(); }
	SpatialMotionVector(const Vec3 &angular, const Vec3 &linear) : m_topVec(angular), m_bottomVec(linear) {}
	//
	void setVector(const Vec3 &angular, const Vec3 &linear)
	{
		m_topVec = angular;
		m_bottomVec = linear;
	}
	void setVal(const Scalar &ax, const Scalar &ay, const Scalar &az, const Scalar &lx, const Scalar &ly, const Scalar &lz)
	{
		m_topVec.setVal(ax, ay, az);
		m_bottomVec.setVal(lx, ly, lz);
	}
	//
	void addVector(const Vec3 &angular, const Vec3 &linear)
	{
		m_topVec += linear;
		m_bottomVec += angular;
	}
	void addValue(const Scalar &ax, const Scalar &ay, const Scalar &az, const Scalar &lx, const Scalar &ly, const Scalar &lz)
	{
		m_topVec[0] += ax;
		m_topVec[1] += ay;
		m_topVec[2] += az;
		m_bottomVec[0] += lx;
		m_bottomVec[1] += ly;
		m_bottomVec[2] += lz;
	}
	//
	const Vec3 &getAngular() const { return m_topVec; }
	const Vec3 &getLinear() const { return m_bottomVec; }
	//
	void setAngular(const Vec3 &angular) { m_topVec = angular; }
	void setLinear(const Vec3 &linear) { m_bottomVec = linear; }
	//
	void addAngular(const Vec3 &angular) { m_topVec += angular; }
	void addLinear(const Vec3 &linear) { m_bottomVec += linear; }
	//
	void setZero()
	{
		m_topVec.setZero();
		m_bottomVec.setZero();
	}
	//
	Scalar dot(const SpatialForceVector &b) const
	{
		return m_bottomVec.dot(b.m_topVec) + m_topVec.dot(b.m_bottomVec);
	}
	//
	template <typename SpatialVectorType>
	void cross(const SpatialVectorType &b, SpatialVectorType &out) const
	{
		out.m_topVec = m_topVec.cross(b.m_topVec);
		out.m_bottomVec = m_bottomVec.cross(b.m_topVec) + m_topVec.cross(b.m_bottomVec);
	}
	template <typename SpatialVectorType>
	SpatialVectorType cross(const SpatialVectorType &b) const
	{
		SpatialVectorType out;
		out.m_topVec = m_topVec.cross(b.m_topVec);
		out.m_bottomVec = m_bottomVec.cross(b.m_topVec) + m_topVec.cross(b.m_bottomVec);
		return out;
	}
	//
	SpatialMotionVector &operator+=(const SpatialMotionVector &vec)
	{
		m_topVec += vec.m_topVec;
		m_bottomVec += vec.m_bottomVec;
		return *this;
	}
	SpatialMotionVector &operator-=(const SpatialMotionVector &vec)
	{
		m_topVec -= vec.m_topVec;
		m_bottomVec -= vec.m_bottomVec;
		return *this;
	}
	SpatialMotionVector &operator*=(const Scalar &s)
	{
		m_topVec *= s;
		m_bottomVec *= s;
		return *this;
	}
	SpatialMotionVector operator-(const SpatialMotionVector &vec) const { return SpatialMotionVector(m_topVec - vec.m_topVec, m_bottomVec - vec.m_bottomVec); }
	SpatialMotionVector operator+(const SpatialMotionVector &vec) const { return SpatialMotionVector(m_topVec + vec.m_topVec, m_bottomVec + vec.m_bottomVec); }
	SpatialMotionVector operator-() const { return SpatialMotionVector(-m_topVec, -m_bottomVec); }
	SpatialMotionVector operator*(const Scalar &s) const { return SpatialMotionVector(s * m_topVec, s * m_bottomVec); }
};

struct SymmetricSpatialDyad
{
	Matrix3x3 m_topLeftMat, m_topRightMat, m_bottomLeftMat;
	//
	SymmetricSpatialDyad() { setIdentity(); }
	SymmetricSpatialDyad(const Matrix3x3 &topLeftMat, const Matrix3x3 &topRightMat, const Matrix3x3 &bottomLeftMat) { setMatrix(topLeftMat, topRightMat, bottomLeftMat); }
	//
	void setMatrix(const Matrix3x3 &topLeftMat, const Matrix3x3 &topRightMat, const Matrix3x3 &bottomLeftMat)
	{
		m_topLeftMat = topLeftMat;
		m_topRightMat = topRightMat;
		m_bottomLeftMat = bottomLeftMat;
	}
	//
	void addMatrix(const Matrix3x3 &topLeftMat, const Matrix3x3 &topRightMat, const Matrix3x3 &bottomLeftMat)
	{
		m_topLeftMat += topLeftMat;
		m_topRightMat += topRightMat;
		m_bottomLeftMat += bottomLeftMat;
	}
	//
	void setIdentity()
	{
		m_topLeftMat.setIdentity();
		m_topRightMat.setIdentity();
		m_bottomLeftMat.setIdentity();
	}
	//
	SymmetricSpatialDyad &operator-=(const SymmetricSpatialDyad &mat)
	{
		m_topLeftMat -= mat.m_topLeftMat;
		m_topRightMat -= mat.m_topRightMat;
		m_bottomLeftMat -= mat.m_bottomLeftMat;
		return *this;
	}
	//
	SpatialForceVector operator*(const SpatialMotionVector &vec)
	{
		return SpatialForceVector(m_bottomLeftMat * vec.m_topVec + m_topLeftMat.transpose() * vec.m_bottomVec, m_topLeftMat * vec.m_topVec + m_topRightMat * vec.m_bottomVec);
	}
};

struct SpatialTransform2ationMatrix
{
	Matrix3x3 m_rotMat;  //Matrix3x3 m_trnCrossMat;
	Vec3 m_trnVec;
	//
	enum eOutputOperation
	{
		None = 0,
		Add = 1,
		Subtract = 2
	};
	//
	template <typename SpatialVectorType>
	void transform(const SpatialVectorType &inVec,
				   SpatialVectorType &outVec,
				   eOutputOperation outOp = None)
	{
		if (outOp == None)
		{
			outVec.m_topVec = m_rotMat * inVec.m_topVec;
			outVec.m_bottomVec = -m_trnVec.cross(outVec.m_topVec) + m_rotMat * inVec.m_bottomVec;
		}
		else if (outOp == Add)
		{
			outVec.m_topVec += m_rotMat * inVec.m_topVec;
			outVec.m_bottomVec += -m_trnVec.cross(outVec.m_topVec) + m_rotMat * inVec.m_bottomVec;
		}
		else if (outOp == Subtract)
		{
			outVec.m_topVec -= m_rotMat * inVec.m_topVec;
			outVec.m_bottomVec -= -m_trnVec.cross(outVec.m_topVec) + m_rotMat * inVec.m_bottomVec;
		}
	}

	template <typename SpatialVectorType>
	void transformRotationOnly(const SpatialVectorType &inVec,
							   SpatialVectorType &outVec,
							   eOutputOperation outOp = None)
	{
		if (outOp == None)
		{
			outVec.m_topVec = m_rotMat * inVec.m_topVec;
			outVec.m_bottomVec = m_rotMat * inVec.m_bottomVec;
		}
		else if (outOp == Add)
		{
			outVec.m_topVec += m_rotMat * inVec.m_topVec;
			outVec.m_bottomVec += m_rotMat * inVec.m_bottomVec;
		}
		else if (outOp == Subtract)
		{
			outVec.m_topVec -= m_rotMat * inVec.m_topVec;
			outVec.m_bottomVec -= m_rotMat * inVec.m_bottomVec;
		}
	}

	template <typename SpatialVectorType>
	void transformInverse(const SpatialVectorType &inVec,
						  SpatialVectorType &outVec,
						  eOutputOperation outOp = None)
	{
		if (outOp == None)
		{
			outVec.m_topVec = m_rotMat.transpose() * inVec.m_topVec;
			outVec.m_bottomVec = m_rotMat.transpose() * (inVec.m_bottomVec + m_trnVec.cross(inVec.m_topVec));
		}
		else if (outOp == Add)
		{
			outVec.m_topVec += m_rotMat.transpose() * inVec.m_topVec;
			outVec.m_bottomVec += m_rotMat.transpose() * (inVec.m_bottomVec + m_trnVec.cross(inVec.m_topVec));
		}
		else if (outOp == Subtract)
		{
			outVec.m_topVec -= m_rotMat.transpose() * inVec.m_topVec;
			outVec.m_bottomVec -= m_rotMat.transpose() * (inVec.m_bottomVec + m_trnVec.cross(inVec.m_topVec));
		}
	}

	template <typename SpatialVectorType>
	void transformInverseRotationOnly(const SpatialVectorType &inVec,
									  SpatialVectorType &outVec,
									  eOutputOperation outOp = None)
	{
		if (outOp == None)
		{
			outVec.m_topVec = m_rotMat.transpose() * inVec.m_topVec;
			outVec.m_bottomVec = m_rotMat.transpose() * inVec.m_bottomVec;
		}
		else if (outOp == Add)
		{
			outVec.m_topVec += m_rotMat.transpose() * inVec.m_topVec;
			outVec.m_bottomVec += m_rotMat.transpose() * inVec.m_bottomVec;
		}
		else if (outOp == Subtract)
		{
			outVec.m_topVec -= m_rotMat.transpose() * inVec.m_topVec;
			outVec.m_bottomVec -= m_rotMat.transpose() * inVec.m_bottomVec;
		}
	}

	void transformInverse(const SymmetricSpatialDyad &inMat,
						  SymmetricSpatialDyad &outMat,
						  eOutputOperation outOp = None)
	{
		const Matrix3x3 r_cross(0, -m_trnVec[2], m_trnVec[1],
								  m_trnVec[2], 0, -m_trnVec[0],
								  -m_trnVec[1], m_trnVec[0], 0);

		if (outOp == None)
		{
			outMat.m_topLeftMat = m_rotMat.transpose() * (inMat.m_topLeftMat - inMat.m_topRightMat * r_cross) * m_rotMat;
			outMat.m_topRightMat = m_rotMat.transpose() * inMat.m_topRightMat * m_rotMat;
			outMat.m_bottomLeftMat = m_rotMat.transpose() * (r_cross * (inMat.m_topLeftMat - inMat.m_topRightMat * r_cross) + inMat.m_bottomLeftMat - inMat.m_topLeftMat.transpose() * r_cross) * m_rotMat;
		}
		else if (outOp == Add)
		{
			outMat.m_topLeftMat += m_rotMat.transpose() * (inMat.m_topLeftMat - inMat.m_topRightMat * r_cross) * m_rotMat;
			outMat.m_topRightMat += m_rotMat.transpose() * inMat.m_topRightMat * m_rotMat;
			outMat.m_bottomLeftMat += m_rotMat.transpose() * (r_cross * (inMat.m_topLeftMat - inMat.m_topRightMat * r_cross) + inMat.m_bottomLeftMat - inMat.m_topLeftMat.transpose() * r_cross) * m_rotMat;
		}
		else if (outOp == Subtract)
		{
			outMat.m_topLeftMat -= m_rotMat.transpose() * (inMat.m_topLeftMat - inMat.m_topRightMat * r_cross) * m_rotMat;
			outMat.m_topRightMat -= m_rotMat.transpose() * inMat.m_topRightMat * m_rotMat;
			outMat.m_bottomLeftMat -= m_rotMat.transpose() * (r_cross * (inMat.m_topLeftMat - inMat.m_topRightMat * r_cross) + inMat.m_bottomLeftMat - inMat.m_topLeftMat.transpose() * r_cross) * m_rotMat;
		}
	}

	template <typename SpatialVectorType>
	SpatialVectorType operator*(const SpatialVectorType &vec)
	{
		SpatialVectorType out;
		transform(vec, out);
		return out;
	}
};

template <typename SpatialVectorType>
void symmetricSpatialOuterProduct(const SpatialVectorType &a, const SpatialVectorType &b, SymmetricSpatialDyad &out)
{
	//output op maybe?

	out.m_topLeftMat = outerProduct(a.m_topVec, b.m_bottomVec);
	out.m_topRightMat = outerProduct(a.m_topVec, b.m_topVec);
	out.m_topLeftMat = outerProduct(a.m_bottomVec, b.m_bottomVec);
	//maybe simple a*spatTranspose(a) would be nicer?
}

template <typename SpatialVectorType>
SymmetricSpatialDyad symmetricSpatialOuterProduct(const SpatialVectorType &a, const SpatialVectorType &b)
{
	SymmetricSpatialDyad out;

	out.m_topLeftMat = outerProduct(a.m_topVec, b.m_bottomVec);
	out.m_topRightMat = outerProduct(a.m_topVec, b.m_topVec);
	out.m_bottomLeftMat = outerProduct(a.m_bottomVec, b.m_bottomVec);

	return out;
	//maybe simple a*spatTranspose(a) would be nicer?
}

#endif  //DRX3D_SPATIAL_ALGEBRA_H
