// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Platform/platform.h>

#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Thread/IJobUpr.h>

#include "FrameProfiler_Shared.h"

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
// Profiling is enabled in every configuration except Release

//! Public Interface for use of Profile Marker.
namespace DrxProfile
{
//////////////////////////////////////////////////////////////////////////
// class to define a profile scope, to represent time events in profile tools
class CScopedProfileMarker
{
	EProfileDescription m_desc;
	tukk m_name;
public:
	inline CScopedProfileMarker(const EProfileDescription desc, tukk pName, ...) : m_desc(desc), m_name(pName)
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
		DrxProfile::PushProfilingMarker(desc, markerName);
		va_end(args);
	}
	inline ~CScopedProfileMarker() { DrxProfile::PopProfilingMarker(m_desc,m_name); }
};

} // namespace DrxProfile

// Util Macro to create scoped profiling markers
	#define DRXPROFILE_CONCAT_(a, b)             a ## b
	#define DRXPROFILE_CONCAT(a, b)              DRXPROFILE_CONCAT_(a, b)
	#define DRXPROFILE_SCOPE_PROFILE_MARKER(...) DrxProfile::CScopedProfileMarker DRXPROFILE_CONCAT(__scopedProfileMarker, __LINE__)(EProfileDescription::SECTION, __VA_ARGS__);

#else

	#define DRXPROFILE_SCOPE_PROFILE_MARKER(...)

#endif
