
#include <stdio.h>
#include <drx3D/Maths/Linear/ReducedVector.h>
#include <cmath>

// returns the projection of this onto other
ReducedVector ReducedVector::proj(const ReducedVector& other) const
{
    ReducedVector ret(m_sz);
    Scalar other_length2 = other.length2();
    if (other_length2 < SIMD_EPSILON)
    {
        return ret;
    }
    return other*(this->dot(other))/other_length2;
}

void ReducedVector::normalize()
{
    if (this->length2() < SIMD_EPSILON)
    {
        m_indices.clear();
        m_vecs.clear();
        return;
    }
    *this /= std::sqrt(this->length2());
}

bool ReducedVector::testAdd() const
{
    i32 sz = 5;
    AlignedObjectArray<i32> id1;
    id1.push_back(1);
    id1.push_back(3);
    AlignedObjectArray<Vec3> v1;
    v1.push_back(Vec3(1,0,1));
    v1.push_back(Vec3(3,1,5));
    AlignedObjectArray<i32> id2;
    id2.push_back(2);
    id2.push_back(3);
    id2.push_back(5);
    AlignedObjectArray<Vec3> v2;
    v2.push_back(Vec3(2,3,1));
    v2.push_back(Vec3(3,4,9));
    v2.push_back(Vec3(0,4,0));
    AlignedObjectArray<i32> id3;
    id3.push_back(1);
    id3.push_back(2);
    id3.push_back(3);
    id3.push_back(5);
    AlignedObjectArray<Vec3> v3;
    v3.push_back(Vec3(1,0,1));
    v3.push_back(Vec3(2,3,1));
    v3.push_back(Vec3(6,5,14));
    v3.push_back(Vec3(0,4,0));
    ReducedVector rv1(sz, id1, v1);
    ReducedVector rv2(sz, id2, v2);
    ReducedVector ans(sz, id3, v3);
    bool ret = ((ans == rv1+rv2) && (ans == rv2+rv1));
    if (!ret)
        printf("ReducedVector testAdd failed\n");
    return ret;
}

bool ReducedVector::testMinus() const
{
    i32 sz = 5;
    AlignedObjectArray<i32> id1;
    id1.push_back(1);
    id1.push_back(3);
    AlignedObjectArray<Vec3> v1;
    v1.push_back(Vec3(1,0,1));
    v1.push_back(Vec3(3,1,5));
    AlignedObjectArray<i32> id2;
    id2.push_back(2);
    id2.push_back(3);
    id2.push_back(5);
    AlignedObjectArray<Vec3> v2;
    v2.push_back(Vec3(2,3,1));
    v2.push_back(Vec3(3,4,9));
    v2.push_back(Vec3(0,4,0));
    AlignedObjectArray<i32> id3;
    id3.push_back(1);
    id3.push_back(2);
    id3.push_back(3);
    id3.push_back(5);
    AlignedObjectArray<Vec3> v3;
    v3.push_back(Vec3(-1,-0,-1));
    v3.push_back(Vec3(2,3,1));
    v3.push_back(Vec3(0,3,4));
    v3.push_back(Vec3(0,4,0));
    ReducedVector rv1(sz, id1, v1);
    ReducedVector rv2(sz, id2, v2);
    ReducedVector ans(sz, id3, v3);
    bool ret = (ans == rv2-rv1);
    if (!ret)
        printf("ReducedVector testMinus failed\n");
    return ret;
}

bool ReducedVector::testDot() const
{
    i32 sz = 5;
    AlignedObjectArray<i32> id1;
    id1.push_back(1);
    id1.push_back(3);
    AlignedObjectArray<Vec3> v1;
    v1.push_back(Vec3(1,0,1));
    v1.push_back(Vec3(3,1,5));
    AlignedObjectArray<i32> id2;
    id2.push_back(2);
    id2.push_back(3);
    id2.push_back(5);
    AlignedObjectArray<Vec3> v2;
    v2.push_back(Vec3(2,3,1));
    v2.push_back(Vec3(3,4,9));
    v2.push_back(Vec3(0,4,0));
    ReducedVector rv1(sz, id1, v1);
    ReducedVector rv2(sz, id2, v2);
    Scalar ans = 58;
    bool ret = (ans == rv2.dot(rv1) && ans == rv1.dot(rv2));
    ans = 14+16+9+16+81;
    ret &= (ans==rv2.dot(rv2));
    
    if (!ret)
        printf("ReducedVector testDot failed\n");
    return ret;
}

bool ReducedVector::testMultiply() const
{
    i32 sz = 5;
    AlignedObjectArray<i32> id1;
    id1.push_back(1);
    id1.push_back(3);
    AlignedObjectArray<Vec3> v1;
    v1.push_back(Vec3(1,0,1));
    v1.push_back(Vec3(3,1,5));
    Scalar s = 2;
    ReducedVector rv1(sz, id1, v1);
    AlignedObjectArray<i32> id2;
    id2.push_back(1);
    id2.push_back(3);
    AlignedObjectArray<Vec3> v2;
    v2.push_back(Vec3(2,0,2));
    v2.push_back(Vec3(6,2,10));
    ReducedVector ans(sz, id2, v2);
    bool ret = (ans == rv1*s);
    if (!ret)
        printf("ReducedVector testMultiply failed\n");
    return ret;
}

void ReducedVector::test() const
{
    bool ans = testAdd() && testMinus() && testDot() && testMultiply();
    if (ans)
    {
        printf("All tests passed\n");
    }
    else
    {
        printf("Tests failed\n");
    }
}
