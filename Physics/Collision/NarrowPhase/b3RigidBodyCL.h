#ifndef D3_RIGID_BODY_CL
#define D3_RIGID_BODY_CL

#include <drx3D/Common/b3Scalar.h>
#include <drx3D/Common/b3Matrix3x3.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>

inline float b3GetInvMass(const b3RigidBodyData& body)
{
	return body.m_invMass;
}

#endif  //D3_RIGID_BODY_CL
