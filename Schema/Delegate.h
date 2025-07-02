// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
//#include <bits/std_function.h>
#include <functional>

#define SXEMA_DELEGATE(functionPtr) (functionPtr)
#define SXEMA_MEMBER_DELEGATE(functionPtr, object) sxema::MemberDelegate<decltype(functionPtr)>::Make(&object,functionPtr)

namespace sxema
{
template<typename T>
struct MemberDelegate {};

template<typename OBJECT_TYPE, typename RETURN_TYPE, typename ... PARAM_TYPES>
struct MemberDelegate<RETURN_TYPE(OBJECT_TYPE::*)(PARAM_TYPES ...)>
{
	static std::function<RETURN_TYPE(PARAM_TYPES ...)> Make(OBJECT_TYPE* t, RETURN_TYPE(OBJECT_TYPE::*f)(PARAM_TYPES...))
	{
		return [=](PARAM_TYPES... v) { return (t->*f)(std::forward<decltype(v)>(v)...); };
	}
};
} // sxema
