// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DRXLIVECREATE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// DRXLIVECREATE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef DRXLIVECREATE_EXPORTS
	#define DRXLIVECREATE_API DLL_EXPORT
#else
	#define DRXLIVECREATE_API DLL_IMPORT
#endif

struct ISystem;

namespace LiveCreate
{
struct IUpr;
}

extern "C"
{
	DRXLIVECREATE_API LiveCreate::IUpr* CreateLiveCreate(ISystem* pSystem);
	DRXLIVECREATE_API void                  DeleteLiveCreate(LiveCreate::IUpr* pLC);
}
