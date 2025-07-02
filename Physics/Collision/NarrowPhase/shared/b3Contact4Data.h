#ifndef D3_CONTACT4DATA_H
#define D3_CONTACT4DATA_H

#include <drx3D/Common/shared/b3Float4.h>

typedef struct b3Contact4Data b3Contact4Data_t;

struct b3Contact4Data
{
	b3Float4 m_worldPosB[4];
	//	b3Float4	m_localPosA[4];
	//	b3Float4	m_localPosB[4];
	b3Float4 m_worldNormalOnB;  //	w: m_nPoints
	unsigned short m_restituitionCoeffCmp;
	unsigned short m_frictionCoeffCmp;
	i32 m_batchIdx;
	i32 m_bodyAPtrAndSignBit;  //x:m_bodyAPtr, y:m_bodyBPtr
	i32 m_bodyBPtrAndSignBit;

	i32 m_childIndexA;
	i32 m_childIndexB;
	i32 m_unused1;
	i32 m_unused2;
};

inline i32 b3Contact4Data_getNumPoints(const struct b3Contact4Data* contact)
{
	return (i32)contact->m_worldNormalOnB.w;
};

inline void b3Contact4Data_setNumPoints(struct b3Contact4Data* contact, i32 numPoints)
{
	contact->m_worldNormalOnB.w = (float)numPoints;
};

#endif  //D3_CONTACT4DATA_H