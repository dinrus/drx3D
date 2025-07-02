#ifndef ReducedVectors_h
#define ReducedVectors_h
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
struct TwoInts
{
    i32 a,b;
};
inline bool operator<(const TwoInts& A, const TwoInts& B)
{
    return A.b < B.b;
}


// A helper vector type used for CG projections
class ReducedVector
{
public:
    AlignedObjectArray<i32> m_indices;
    AlignedObjectArray<Vec3> m_vecs;
    i32 m_sz; // all m_indices value < m_sz
public:
	ReducedVector():m_sz(0)
	{
		m_indices.resize(0);
		m_vecs.resize(0);
        m_indices.clear();
        m_vecs.clear();
	}
	
    ReducedVector(i32 sz): m_sz(sz)
    {
        m_indices.resize(0);
        m_vecs.resize(0);
        m_indices.clear();
        m_vecs.clear();
    }
    
    ReducedVector(i32 sz, const AlignedObjectArray<i32>& indices, const AlignedObjectArray<Vec3>& vecs): m_sz(sz), m_indices(indices), m_vecs(vecs)
    {
    }
    
    void simplify()
    {
        AlignedObjectArray<i32> old_indices(m_indices);
        AlignedObjectArray<Vec3> old_vecs(m_vecs);
        m_indices.resize(0);
        m_vecs.resize(0);
        m_indices.clear();
        m_vecs.clear();
        for (i32 i = 0; i < old_indices.size(); ++i)
        {
            if (old_vecs[i].length2() > SIMD_EPSILON)
            {
                m_indices.push_back(old_indices[i]);
                m_vecs.push_back(old_vecs[i]);
            }
        }
    }
    
    ReducedVector operator+(const ReducedVector& other)
    {
		ReducedVector ret(m_sz);
		i32 i=0, j=0;
		while (i < m_indices.size() && j < other.m_indices.size())
		{
			if (m_indices[i] < other.m_indices[j])
			{
				ret.m_indices.push_back(m_indices[i]);
				ret.m_vecs.push_back(m_vecs[i]);
				++i;
			}
			else if (m_indices[i] > other.m_indices[j])
			{
				ret.m_indices.push_back(other.m_indices[j]);
				ret.m_vecs.push_back(other.m_vecs[j]);
				++j;
			}
			else
			{
				ret.m_indices.push_back(other.m_indices[j]);
				ret.m_vecs.push_back(m_vecs[i] + other.m_vecs[j]);
				++i; ++j;
			}
		}
		while (i < m_indices.size())
		{
			ret.m_indices.push_back(m_indices[i]);
			ret.m_vecs.push_back(m_vecs[i]);
			++i;
		}
		while (j < other.m_indices.size())
		{
			ret.m_indices.push_back(other.m_indices[j]);
			ret.m_vecs.push_back(other.m_vecs[j]);
			++j;
		}
        ret.simplify();
        return ret;
    }

    ReducedVector operator-()
    {
        ReducedVector ret(m_sz);
        for (i32 i = 0; i < m_indices.size(); ++i)
        {
            ret.m_indices.push_back(m_indices[i]);
            ret.m_vecs.push_back(-m_vecs[i]);
        }
        ret.simplify();
        return ret;
    }
    
    ReducedVector operator-(const ReducedVector& other)
    {
		ReducedVector ret(m_sz);
		i32 i=0, j=0;
		while (i < m_indices.size() && j < other.m_indices.size())
		{
			if (m_indices[i] < other.m_indices[j])
			{
				ret.m_indices.push_back(m_indices[i]);
				ret.m_vecs.push_back(m_vecs[i]);
				++i;
			}
			else if (m_indices[i] > other.m_indices[j])
			{
				ret.m_indices.push_back(other.m_indices[j]);
				ret.m_vecs.push_back(-other.m_vecs[j]);
				++j;
			}
			else
			{
				ret.m_indices.push_back(other.m_indices[j]);
				ret.m_vecs.push_back(m_vecs[i] - other.m_vecs[j]);
				++i; ++j;
			}
		}
		while (i < m_indices.size())
		{
			ret.m_indices.push_back(m_indices[i]);
			ret.m_vecs.push_back(m_vecs[i]);
			++i;
		}
		while (j < other.m_indices.size())
		{
			ret.m_indices.push_back(other.m_indices[j]);
			ret.m_vecs.push_back(-other.m_vecs[j]);
			++j;
		}
        ret.simplify();
		return ret;
    }
    
    bool operator==(const ReducedVector& other) const
    {
        if (m_sz != other.m_sz)
            return false;
        if (m_indices.size() != other.m_indices.size())
            return false;
        for (i32 i = 0; i < m_indices.size(); ++i)
        {
            if (m_indices[i] != other.m_indices[i] || m_vecs[i] != other.m_vecs[i])
            {
                return false;
            }
        }
        return true;
    }
    
    bool operator!=(const ReducedVector& other) const
    {
        return !(*this == other);
    }
	
	ReducedVector& operator=(const ReducedVector& other)
	{
		if (this == &other)
		{
			return *this;
		}
        m_sz = other.m_sz;
		m_indices.copyFromArray(other.m_indices);
		m_vecs.copyFromArray(other.m_vecs);
		return *this;
	}
    
    Scalar dot(const ReducedVector& other) const
    {
        Scalar ret = 0;
        i32 j = 0;
        for (i32 i = 0; i < m_indices.size(); ++i)
        {
            while (j < other.m_indices.size() && other.m_indices[j] < m_indices[i])
            {
                ++j;
            }
            if (j < other.m_indices.size() && other.m_indices[j] == m_indices[i])
            {
                ret += m_vecs[i].dot(other.m_vecs[j]);
//                ++j;
            }
        }
        return ret;
    }
    
    Scalar dot(const AlignedObjectArray<Vec3>& other) const
    {
        Scalar ret = 0;
        for (i32 i = 0; i < m_indices.size(); ++i)
        {
            ret += m_vecs[i].dot(other[m_indices[i]]);
        }
        return ret;
    }
    
    Scalar length2() const
    {
        return this->dot(*this);
    }
	
	void normalize();
    
    // returns the projection of this onto other
    ReducedVector proj(const ReducedVector& other) const;
    
    bool testAdd() const;
    
    bool testMinus() const;
    
    bool testDot() const;
    
    bool testMultiply() const;
    
    void test() const;
    
    void print() const
    {
        for (i32 i = 0; i < m_indices.size(); ++i)
        {
            printf("%d: (%f, %f, %f)/", m_indices[i], m_vecs[i][0],m_vecs[i][1],m_vecs[i][2]);
        }
        printf("\n");
    }
    
    
    void sort()
    {
        std::vector<TwoInts> tuples;
        for (i32 i = 0; i < m_indices.size(); ++i)
        {
            TwoInts ti;
            ti.a = i;
            ti.b = m_indices[i];
            tuples.push_back(ti);
        }
        std::sort(tuples.begin(), tuples.end());
        AlignedObjectArray<i32> new_indices;
        AlignedObjectArray<Vec3> new_vecs;
        for (size_t i = 0; i < tuples.size(); ++i)
        {
            new_indices.push_back(tuples[i].b);
            new_vecs.push_back(m_vecs[tuples[i].a]);
        }
        m_indices = new_indices;
        m_vecs = new_vecs;
    }
};

SIMD_FORCE_INLINE ReducedVector operator*(const ReducedVector& v, Scalar s)
{
    ReducedVector ret(v.m_sz);
    for (i32 i = 0; i < v.m_indices.size(); ++i)
    {
        ret.m_indices.push_back(v.m_indices[i]);
        ret.m_vecs.push_back(s*v.m_vecs[i]);
    }
    ret.simplify();
    return ret;
}

SIMD_FORCE_INLINE ReducedVector operator*(Scalar s, const ReducedVector& v)
{
    return v*s;
}

SIMD_FORCE_INLINE ReducedVector operator/(const ReducedVector& v, Scalar s)
{
	return v * (1.0/s);
}

SIMD_FORCE_INLINE ReducedVector& operator/=(ReducedVector& v, Scalar s)
{
	v = v/s;
	return v;
}

SIMD_FORCE_INLINE ReducedVector& operator+=(ReducedVector& v1, const ReducedVector& v2)
{
	v1 = v1+v2;
	return v1;
}

SIMD_FORCE_INLINE ReducedVector& operator-=(ReducedVector& v1, const ReducedVector& v2)
{
	v1 = v1-v2;
	return v1;
}

#endif /* ReducedVectors_h */
