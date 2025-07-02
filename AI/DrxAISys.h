// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DRXAISYS_H_
#define __DRXAISYS_H_

#if _MSC_VER > 1000
	#pragma once
#endif

#ifdef DRXAISYS_EXPORTS
	#define DRXAIAPI DLL_EXPORT
#else
	#define DRXAIAPI DLL_IMPORT
#endif

struct IAISystem;
struct ISystem;

extern "C"
{
	DRXAIAPI IAISystem* CreateAISystem(ISystem* pSystem);
}

#endif //__DRXAISYS_H_
