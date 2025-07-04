// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef UTILS_H
#define UTILS_H

#include <drx3D/Sys/ISystem.h>

typedef Vec2d Vector2d;

// needed to use Vector2d in a map
template<class F>
bool operator<(const Vec2_tpl<F>& op1, const Vec2_tpl<F>& op2)
{
	if (op1.x < op2.x)
		return true;
	else if (op1.x > op2.x)
		return false;
	if (op1.y < op2.y)
		return true;
	else
		return false;
}

#endif
