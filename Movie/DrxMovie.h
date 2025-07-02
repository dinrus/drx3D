// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DRXMOVIE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// DRXMOVIE_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef DRXMOVIE_EXPORTS
	#define DRXMOVIE_API DLL_EXPORT
#else
	#define DRXMOVIE_API DLL_IMPORT
#endif

struct ISystem;
struct IMovieSystem;

extern "C"
{
	DRXMOVIE_API IMovieSystem* CreateMovieSystem(ISystem* pSystem);
	DRXMOVIE_API void          DeleteMovieSystem(IMovieSystem* pMM);
}
