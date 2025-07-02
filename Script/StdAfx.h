// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_STDAFX_H__844E5BAB_B810_40FC_8939_167146C07AED__INCLUDED_)
#define AFX_STDAFX_H__844E5BAB_B810_40FC_8939_167146C07AED__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule   eDrxM_ScriptSystem
#define RWI_NAME_TAG "RayWorldIntersection(Script)"
#define PWI_NAME_TAG "PrimitiveWorldIntersection(Script)"

#define DRXSCRIPTSYSTEM_EXPORTS

#include <drx3D/CoreX/Platform/platform.h>

#include <vector>
#include <map>

//#define DEBUG_LUA_STATE

#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/Memory/PoolAllocator.h>

#include <drx3D/Sys/FrameProfiler_JobSystem.h>  // to be removed

//////////////////////////////////////////////////////////////////////////
//! Reports a Game Warning to validator with WARNING severity.
#ifdef _RELEASE
	#define ScriptWarning(...) ((void)0)
#else
void        ScriptWarning(tukk , ...) PRINTF_PARAMS(1, 2);
inline void ScriptWarning(tukk format, ...)
{
	IF (!format, 0)
		return;

	char buffer[MAX_WARNING_LENGTH];
	va_list args;
	va_start(args, format);
	drx_vsprintf(buffer, format, args);
	va_end(args);
	DrxWarning(VALIDATOR_MODULE_SCRIPTSYSTEM, VALIDATOR_WARNING, "%s", buffer);
}
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__844E5BAB_B810_40FC_8939_167146C07AED__INCLUDED_)
