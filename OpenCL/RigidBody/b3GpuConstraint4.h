
#ifndef D3_CONSTRAINT4_h
#define D3_CONSTRAINT4_h
#include <drx3D/Common/b3Vec3.h>

#include <drx3D/Physics/Dynamics/shared/b3ContactConstraint4.h>

D3_ATTRIBUTE_ALIGNED16(struct)
b3GpuConstraint4 : public b3ContactConstraint4
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	inline void setFrictionCoeff(float value) { m_linear[3] = value; }
	inline float getFrictionCoeff() const { return m_linear[3]; }
};

#endif  //D3_CONSTRAINT4_h
