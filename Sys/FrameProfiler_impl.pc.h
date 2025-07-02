// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if defined(DRX_PROFILE_MARKERS_USE_GPA)
	#include <ittnotify.h>

	#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
LINK_THIRD_PARTY_LIBRARY("SDKs/GPA/lib64/libittnotify.lib")
LINK_THIRD_PARTY_LIBRARY("SDKs/GPA/lib64/jitprofiling.lib")
	#elif DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT
LINK_THIRD_PARTY_LIBRARY("SDKs/GPA/lib32/libittnotify.lib")
LINK_THIRD_PARTY_LIBRARY("SDKs/GPA/lib32/jitprofiling.lib")
	#else
		#error Unknown Platform
	#endif
#endif

#include <drx3D/CoreX/Containers/VectorMap.h>

////////////////////////////////////////////////////////////////////////////
namespace DrxProfile {
namespace detail {

#if defined(DRX_PROFILE_MARKERS_USE_GPA)
////////////////////////////////////////////////////////////////////////////
//! Utility function to create a unique domain for GPA in DinrusX.
//! Moved out of function to global scope.
//! Static function variables are not guaranteed to be thread-safe.
static __itt_domain* domain = __itt_domain_create("DinrusX");

__itt_domain* GetDomain()
{
	return domain;
}

////////////////////////////////////////////////////////////////////////////
//! Utility function to create a unique string handle for GPA.
//! Declared in global scope to ensure thread safety.
static  i32 _lock = 0;
static VectorMap<string, __itt_string_handle*> _handle_lookup;

__itt_string_handle* GetStringHandle(tukk pName)
{
	__itt_string_handle* pHandle = NULL;

	{
		// first try a simple read lock to prevent thread contention
		DrxReadLock(&_lock);
		VectorMap<string, __itt_string_handle*>::iterator it = _handle_lookup.find(CONST_TEMP_STRING(pName));
		if (it != _handle_lookup.end())
		{
			pHandle = it->second;
			DrxReleaseReadLock(&_lock);
			return pHandle;
		}
		DrxReleaseReadLock(&_lock);
	}

	// Nothing found, use write lock to add a new element safely.
	{
		DrxWriteLock(&_lock);
		// check again to make sure not two thread want to add the same handle
		VectorMap<string, __itt_string_handle*>::iterator it = _handle_lookup.find(CONST_TEMP_STRING(pName));
		if (it != _handle_lookup.end())
			pHandle = it->second;
		else
		{
			pHandle = __itt_string_handle_create(pName);
			_handle_lookup.insert(std::make_pair(string(pName), pHandle));
		}
		DrxReleaseWriteLock(&_lock);
		return pHandle;
	}
}

////////////////////////////////////////////////////////////////////////////
//! Utility function to create a unique event handle for GPA.
//! Declared in global scope to ensure thread safety.
static  i32 _event_lock = 0;
static VectorMap<string, __itt_event> _event_handle_lookup;

__itt_event& GetEventHandle(tukk pName)
{
	__itt_event* pHandle = NULL;

	{
		// first try a simple read lock to prevent thread contention
		DrxReadLock(&_event_lock);
		VectorMap<string, __itt_event>::iterator it = _event_handle_lookup.find(CONST_TEMP_STRING(pName));
		if (it != _event_handle_lookup.end())
		{
			pHandle = &it->second;
			DrxReleaseReadLock(&_event_lock);
			return *pHandle;
		}
		DrxReleaseReadLock(&_event_lock);
	}

	// nothing found, use write lock to add a new element safely
	{
		DrxWriteLock(&_event_lock);
		// check again to make sure not two thread want to add the same handle
		VectorMap<string, __itt_event>::iterator it = _event_handle_lookup.find(CONST_TEMP_STRING(pName));
		if (it != _event_handle_lookup.end())
			pHandle = &it->second;
		else
		{
			__itt_event handle = __itt_event_create(pName, strlen(pName));
			pHandle = &_event_handle_lookup.insert(std::make_pair(string(pName), handle)).first->second;
		}
		DrxReleaseWriteLock(&_event_lock);
		return *pHandle;
	}
}
#endif // #if defined(DRX_PROFILE_MARKERS_USE_GPA)

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//! Set the Thread Name.
void SetThreadName(tukk pName)
{
#if defined(DRX_PROFILE_MARKERS_USE_GPA)
	__itt_thread_set_name(pName);
#endif
}

////////////////////////////////////////////////////////////////////////////
//! Set a one Profiling Event marker.
void SetProfilingEvent(const BYTE colorId, tukk pName)
{
#if defined(DRX_PROFILE_MARKERS_USE_GPA)
	__itt_event_start(GetEventHandle(pName));
#endif
}

////////////////////////////////////////////////////////////////////////////
//! Set the beginning of a profile marker.
void PushProfilingMarker(const BYTE colorId, tukk pName)
{
#if defined(DRX_PROFILE_MARKERS_USE_GPA)
	__itt_task_begin(GetDomain(), __itt_null, __itt_null, GetStringHandle(pName));
#endif
}

////////////////////////////////////////////////////////////////////////////
//! Set the end of a profiling marker.
void PopProfilingMarker(const BYTE colorId, tukk pName)
{
#if defined(DRX_PROFILE_MARKERS_USE_GPA)
	__itt_task_end(GetDomain());
#endif
}

void ProfilerPause()
{
#if defined(DRX_PROFILE_MARKERS_USE_GPA)
	__itt_pause();
#endif
}

void ProfilerResume()
{
#if defined(DRX_PROFILE_MARKERS_USE_GPA)
	__itt_resume();
#endif
}

void ProfilerFrameStart(i32 nFrameId)
{
#if defined(DRX_PROFILE_MARKERS_USE_GPA)
	__itt_frame_begin_v3(GetDomain(), nullptr);
#endif
}

void ProfilerFrameEnd(i32 nFrameId)
{
#if defined(DRX_PROFILE_MARKERS_USE_GPA)
	__itt_frame_end_v3(GetDomain(), nullptr);
#endif
}

} // namespace detail
} // namespace DrxProfile