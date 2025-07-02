#ifndef ModifiedGramSchmidt_h
#define ModifiedGramSchmidt_h

#include <drx3D/Maths/Linear/ReducedVector.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <iostream>
#include <cmath>
template<class TV>
class ModifiedGramSchmidt
{
public:
    AlignedObjectArray<TV> m_in;
    AlignedObjectArray<TV> m_out;
    
    ModifiedGramSchmidt(const AlignedObjectArray<TV>& vecs): m_in(vecs)
    {
        m_out.resize(0);
    }
    
    void solve()
    {
        m_out.resize(m_in.size());
        for (i32 i = 0; i < m_in.size(); ++i)
        {
//            printf("========= starting %d ==========\n", i);
            TV v(m_in[i]);
//            v.print();
            for (i32 j = 0; j < i; ++j)
            {
                v = v - v.proj(m_out[j]);
//                v.print();
            }
            v.normalize();
            m_out[i] = v;
//            v.print();
        }
    }
    
    void test()
    {
        std::cout << SIMD_EPSILON << std::endl;
        printf("=======inputs=========\n");
        for (i32 i = 0; i < m_out.size(); ++i)
        {
            m_in[i].print();
        }
        printf("=======output=========\n");
        for (i32 i = 0; i < m_out.size(); ++i)
        {
            m_out[i].print();
        }
        Scalar eps = SIMD_EPSILON;
        for (i32 i = 0; i < m_out.size(); ++i)
        {
            for (i32 j = 0; j < m_out.size(); ++j)
            {
                if (i == j)
                {
                    if (std::abs(1.0-m_out[i].dot(m_out[j])) > eps)// && std::abs(m_out[i].dot(m_out[j])) > eps)
                    {
                        printf("vec[%d] is not unit, norm squared = %f\n", i,m_out[i].dot(m_out[j]));
                    }
                }
                else
                {
                    if (std::abs(m_out[i].dot(m_out[j])) > eps)
                    {
                        printf("vec[%d] and vec[%d] is not orthogonal, dot product = %f\n", i, j, m_out[i].dot(m_out[j]));
                    }
                }
            }
        }
    }
};
template class ModifiedGramSchmidt<ReducedVector>;
#endif /* ModifiedGramSchmidt_h */
