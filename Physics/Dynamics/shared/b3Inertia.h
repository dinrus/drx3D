

#ifndef D3_INERTIA_H
#define D3_INERTIA_H

#include <drx3D/Common/shared/b3Mat3x3.h>

struct b3Inertia
{
	b3Mat3x3 m_invInertiaWorld;
	b3Mat3x3 m_initInvInertia;
};

#endif  //D3_INERTIA_H