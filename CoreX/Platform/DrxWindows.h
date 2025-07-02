// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrxWindows.h
//  Version:     v1.00
//  Created:     02/05/2012 by James Chilvers.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Specific header to handle Windows.h include
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#if DRX_PLATFORM_WINAPI

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif

// Do not define min/max in windows.h
	#define NOMINMAX

// Prevents <Windows.h> from #including <Winsock.h>
// Manually define your <Winsock2.h> inclusion point elsewhere instead.
	#ifndef _WINSOCKAPI_
		#define _WINSOCKAPI_
	#endif

//	#if defined(_WINDOWS_) && !defined(DRX_INCLUDE_WINDOWS_VIA_MFC_OR_ATL_INCLUDES)
//		#error "<windows.h> has been included by other means than DrxWindows.h"
//	#endif

	#include <windows.h>

	#if !defined(DRX_SUPPRESS_DRXENGINE_WINDOWS_FUNCTION_RENAMING)
		#undef min
		#undef max
		#undef GetCommandLine
		#undef GetObject
		#undef PlaySound
		#undef GetClassName
		#undef DrawText
		#undef GetCharWidth
		#undef GetUserName
		#undef LoadLibrary
#if !defined(RESOURCE_COMPILER)
		#undef MessageBox // Disable usage of MessageBox, we want DrxMessageBox to be used instead
#endif
	#endif

	#ifdef DRX_PLATFORM_DURANGO
		#include <drx3D/CoreX/Platform/Durango_Win32Legacy.h>
	#endif

// In RELEASE disable OutputDebugString
	#if defined(_RELEASE) && !DRX_PLATFORM_DESKTOP && !defined(RELEASE_LOGGING)
		#undef OutputDebugString
		#define OutputDebugString(...) (void) 0
	#endif

#endif
