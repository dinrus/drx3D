#ifndef GIM_PAIR_H
#define GIM_PAIR_H


//! Overlapping pair
struct GIM_PAIR
{
        i32 m_index1;
        i32 m_index2;
        GIM_PAIR()
        {
        }

        GIM_PAIR(const GIM_PAIR& p)
        {
                m_index1 = p.m_index1;
                m_index2 = p.m_index2;
        }

        GIM_PAIR(i32 index1, i32 index2)
        {
                m_index1 = index1;
                m_index2 = index2;
        }
};

#endif //GIM_PAIR_H

