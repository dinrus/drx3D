// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Platform/platform.h>

#include "FrameProfiler_Shared.h"

////////////////////////////////////////////////////////////////////////////
// forward declaration for per-platform implementation functions
namespace DrxProfile
{
void PushProfilingMarker(const EProfileDescription desc, tukk pName, ...);
void PopProfilingMarker(const EProfileDescription desc, tukk pName);

namespace detail
{
void        SetThreadName(tukk pName);
void        SetProfilingEvent(const BYTE colorId, tukk pName);
void        PushProfilingMarker(const BYTE colorId, tukk pName);
void        PopProfilingMarker(const BYTE colorId, tukk pName);
void        ProfilerFrameStart(i32 nFrameId);
void        ProfilerFrameEnd(i32 nFrameId);
void        ProfilerPause();
void        ProfilerResume();
}
inline void ProfilerFrameStart(i32 nFrameId) { detail::ProfilerFrameStart(nFrameId); }
inline void ProfilerFrameEnd(i32 nFrameId)   { detail::ProfilerFrameEnd(nFrameId); }
inline void ProfilerPause()                  { detail::ProfilerPause(); }
inline void ProfilerResume()                 { detail::ProfilerResume(); }
}

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
// Profiling is enabled in every configuration except Release

// Platform profilers such as Pix, Razor or GPA
	#if ALLOW_PLATFORM_PROFILER

		#define PLATFORM_PROFILER_THREADNAME(szName) do { ::DrxProfile::detail::SetThreadName(szName); } while (0)
		#define PLATFORM_PROFILER_FRAMESTART(szName) /*not implemented*/

		#if defined(PERFORMANCE_BUILD)
			#define PLATFORM_PROFILER_REGION(szName)           /*not implemented*/
			#define PLATFORM_PROFILER_REGION_WAITING(szName)   /*not implemented*/
			#define PLATFORM_PROFILER_FUNCTION(szName)         /*not implemented*/
			#define PLATFORM_PROFILER_FUNCTION_WAITING(szName) /*not implemented*/
			#define PLATFORM_PROFILER_SECTION(szName)          /*not implemented*/
			#define PLATFORM_PROFILER_SECTION_WAITING(szName)  /*not implemented*/
			#define PLATFORM_PROFILER_MARKER(szLabel)          do { ::DrxProfile::detail::SetProfilingEvent(0, szLabel); } while (0)
			#define PLATFORM_PROFILER_PUSH(szLabel)            do { ::DrxProfile::detail::PushProfilingMarker(0, szLabel); } while (0)
			#define PLATFORM_PROFILER_POP(szLabel)             do { ::DrxProfile::detail::PopProfilingMarker(0,szLabel); } while (0)
		#else                                                // profile builds take another path to allow runtime switch
			#define PLATFORM_PROFILER_REGION(szName)           /*do nothing*/
			#define PLATFORM_PROFILER_REGION_WAITING(szName)   /*do nothing*/
			#define PLATFORM_PROFILER_FUNCTION(szName)         /*do nothing*/
			#define PLATFORM_PROFILER_FUNCTION_WAITING(szName) /*do nothing*/
			#define PLATFORM_PROFILER_SECTION(szName)          /*do nothing*/
			#define PLATFORM_PROFILER_SECTION_WAITING(szName)  /*do nothing*/
			#define PLATFORM_PROFILER_MARKER(szName)           /*do nothing*/
			#define PLATFORM_PROFILER_PUSH(szLabel)            /*do nothing*/
			#define PLATFORM_PROFILER_POP(szLabel)             /*do nothing*/
		#endif

	#else

		#define PLATFORM_PROFILER_THREADNAME(szName)       /*do nothing*/
		#define PLATFORM_PROFILER_FRAMESTART(szName)       /*do nothing*/
		#define PLATFORM_PROFILER_REGION(szName)           /*do nothing*/
		#define PLATFORM_PROFILER_REGION_WAITING(szName)   /*do nothing*/
		#define PLATFORM_PROFILER_FUNCTION(szName)         /*do nothing*/
		#define PLATFORM_PROFILER_FUNCTION_WAITING(szName) /*do nothing*/
		#define PLATFORM_PROFILER_SECTION(szName)          /*do nothing*/
		#define PLATFORM_PROFILER_SECTION_WAITING(szName)  /*do nothing*/
		#define PLATFORM_PROFILER_MARKER(szLabel)          /*do nothing*/
		#define PLATFORM_PROFILER_PUSH(szLabel)            /*do nothing*/
		#define PLATFORM_PROFILER_POP(szLabel)             /*do nothing*/

	#endif

//! Deprecated Interface - do not use
namespace DrxProfilePlatform
{
//////////////////////////////////////////////////////////////////////////
// class to define a profile scope, to represent time events in profile tools
class CScopedPlatformProfileMarker
{
	tukk m_name = nullptr;
public:
	inline CScopedPlatformProfileMarker(const EProfileDescription desc, tukk pName, ...) : m_name(pName)
	{
		va_list args;
		va_start(args, pName);

		// Format event name
		char markerName[256];
		i32k cNumCharsNeeded = vsnprintf(markerName, DRX_ARRAY_COUNT(markerName), pName, args);
		if (cNumCharsNeeded > DRX_ARRAY_COUNT(markerName) - 1 || cNumCharsNeeded < 0)
		{
			markerName[DRX_ARRAY_COUNT(markerName) - 1] = '\0'; // The WinApi only null terminates if strLen < bufSize
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "ProfileEvent: Marker name \"%s\" has been truncated. Max characters allowed: %i. ", pName, DRX_ARRAY_COUNT(markerName) - 1);
		}

		// Set marker
		::DrxProfile::detail::PushProfilingMarker(0, markerName);
		va_end(args);
	}
	inline ~CScopedPlatformProfileMarker() { ::DrxProfile::detail::PopProfilingMarker(0,m_name); }
};

} // namespace DrxProfile

// Util Macro to create scoped profiling markers
	#define DRXPROFILE_CONCAT_(a, b)              a ## b
	#define DRXPROFILE_CONCAT(a, b)               DRXPROFILE_CONCAT_(a, b)
	#define DRXPROFILE_SCOPE_PLATFORM_MARKER(...) DrxProfilePlatform::CScopedPlatformProfileMarker DRXPROFILE_CONCAT(__scopedProfileMarker, __LINE__)(EProfileDescription::SECTION, __VA_ARGS__);

#else

	#define PLATFORM_PROFILER_THREADNAME(szName)       /*do nothing*/
	#define PLATFORM_PROFILER_FRAMESTART(szName)       /*do nothing*/
	#define PLATFORM_PROFILER_REGION(szName)           /*do nothing*/
	#define PLATFORM_PROFILER_REGION_WAITING(szName)   /*do nothing*/
	#define PLATFORM_PROFILER_FUNCTION(szName)         /*do nothing*/
	#define PLATFORM_PROFILER_FUNCTION_WAITING(szName) /*do nothing*/
	#define PLATFORM_PROFILER_SECTION(szName)          /*do nothing*/
	#define PLATFORM_PROFILER_SECTION_WAITING(szName)  /*do nothing*/
	#define PLATFORM_PROFILER_MARKER(szLabel)          /*do nothing*/
	#define PLATFORM_PROFILER_PUSH(szLabel)            /*do nothing*/
	#define PLATFORM_PROFILER_POP(szLabel)             /*do nothing*/

	#define DRXPROFILE_SCOPE_PLATFORM_MARKER(...)

#endif
