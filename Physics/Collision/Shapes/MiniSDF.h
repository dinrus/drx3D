#ifndef MINISDF_H
#define MINISDF_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

struct MultiIndex
{
	u32 ijk[3];
};

struct AlignedBox3d
{
	Vec3 m_min;
	Vec3 m_max;

	const Vec3& min() const
	{
		return m_min;
	}

	const Vec3& max() const
	{
		return m_max;
	}

	bool contains(const Vec3& x) const
	{
		return TestPointAgainstAabb2(m_min, m_max, x);
	}

	AlignedBox3d(const Vec3& mn, const Vec3& mx)
		: m_min(mn),
		  m_max(mx)
	{
	}

	AlignedBox3d()
	{
	}
};

struct ShapeMatrix
{
	double m_vec[32];

	inline double& operator[](i32 i)
	{
		return m_vec[i];
	}

	inline const double& operator[](i32 i) const
	{
		return m_vec[i];
	}
};

struct ShapeGradients
{
	Vec3 m_vec[32];

	void topRowsDivide(i32 row, double denom)
	{
		for (i32 i = 0; i < row; i++)
		{
			m_vec[i] /= denom;
		}
	}

	void bottomRowsMul(i32 row, double val)
	{
		for (i32 i = 32 - row; i < 32; i++)
		{
			m_vec[i] *= val;
		}
	}

	inline Scalar& operator()(i32 i, i32 j)
	{
		return m_vec[i][j];
	}
};

struct Cell32
{
	u32 m_cells[32];
};

struct MiniSDF
{
	AlignedBox3d m_domain;
	u32 m_resolution[3];
	Vec3 m_cell_size;
	Vec3 m_inv_cell_size;
	std::size_t m_n_cells;
	std::size_t m_n_fields;
	bool m_isValid;

	AlignedObjectArray<AlignedObjectArray<double> > m_nodes;
	AlignedObjectArray<AlignedObjectArray<Cell32> > m_cells;
	AlignedObjectArray<AlignedObjectArray<u32> > m_cell_map;

	MiniSDF()
		: m_isValid(false)
	{
	}
	bool load(tukk data, i32 size);
	bool isValid() const
	{
		return m_isValid;
	}
	u32 multiToSingleIndex(MultiIndex const& ijk) const;

	AlignedBox3d subdomain(MultiIndex const& ijk) const;

	MultiIndex singleToMultiIndex(u32 l) const;

	AlignedBox3d subdomain(u32 l) const;

	ShapeMatrix
	shape_function_(Vec3 const& xi, ShapeGradients* gradient = 0) const;

	bool interpolate(u32 field_id, double& dist, Vec3 const& x, Vec3* gradient) const;
};

#endif  //MINISDF_H
