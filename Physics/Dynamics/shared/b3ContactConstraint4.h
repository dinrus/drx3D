#ifndef D3_CONTACT_CONSTRAINT5_H
#define D3_CONTACT_CONSTRAINT5_H

#include <drx3D/Common/shared/b3Float4.h>

typedef struct b3ContactConstraint4 b3ContactConstraint4_t;

struct b3ContactConstraint4
{
	b3Float4 m_linear;  //normal?
	b3Float4 m_worldPos[4];
	b3Float4 m_center;  //	friction
	float m_jacCoeffInv[4];
	float m_b[4];
	float m_appliedRambdaDt[4];
	float m_fJacCoeffInv[2];      //	friction
	float m_fAppliedRambdaDt[2];  //	friction

	u32 m_bodyA;
	u32 m_bodyB;
	i32 m_batchIdx;
	u32 m_paddings;
};

//inline	void setFrictionCoeff(float value) { m_linear[3] = value; }
inline float b3GetFrictionCoeff(b3ContactConstraint4_t* constraint)
{
	return constraint->m_linear.w;
}

#endif  //D3_CONTACT_CONSTRAINT5_H
