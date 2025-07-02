#ifndef D3_CONTACT4_H
#define D3_CONTACT4_H

#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Contact4Data.h>

D3_ATTRIBUTE_ALIGNED16(struct)
b3Contact4 : public b3Contact4Data
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	i32 getBodyA() const { return abs(m_bodyAPtrAndSignBit); }
	i32 getBodyB() const { return abs(m_bodyBPtrAndSignBit); }
	bool isBodyAFixed() const { return m_bodyAPtrAndSignBit < 0; }
	bool isBodyBFixed() const { return m_bodyBPtrAndSignBit < 0; }
	//	todo. make it safer
	i32& getBatchIdx() { return m_batchIdx; }
	i32k& getBatchIdx() const { return m_batchIdx; }
	float getRestituitionCoeff() const { return ((float)m_restituitionCoeffCmp / (float)0xffff); }
	void setRestituitionCoeff(float c)
	{
		drx3DAssert(c >= 0.f && c <= 1.f);
		m_restituitionCoeffCmp = (unsigned short)(c * 0xffff);
	}
	float getFrictionCoeff() const { return ((float)m_frictionCoeffCmp / (float)0xffff); }
	void setFrictionCoeff(float c)
	{
		drx3DAssert(c >= 0.f && c <= 1.f);
		m_frictionCoeffCmp = (unsigned short)(c * 0xffff);
	}

	//float& getNPoints() { return m_worldNormal[3]; }
	i32 getNPoints() const { return (i32)m_worldNormalOnB.w; }

	float getPenetration(i32 idx) const { return m_worldPosB[idx].w; }

	bool isInvalid() const { return (getBodyA() == 0 || getBodyB() == 0); }
};

#endif  //D3_CONTACT4_H
