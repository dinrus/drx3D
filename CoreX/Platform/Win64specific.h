// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Win32specific.h
//  Version:     v1.00
//  Created:     31/03/2003 by Sergiy.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Specific to Win32 declarations, inline functions etc.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

// Ensure WINAPI version is consistent everywhere
//#define _WIN32_WINNT  0x0600
#define NTDDI_VERSION 0x06000000
#define WINVER        0x0600

#define RC_EXECUTABLE "rc.exe"
#define SIZEOF_PTR    8

#pragma warning( disable : 4267 ) //warning C4267: 'initializing' : conversion from 'size_t' to 'u32', possible loss of data

//////////////////////////////////////////////////////////////////////////
// Standard includes.
//////////////////////////////////////////////////////////////////////////
#include <malloc.h>
#include <io.h>
#include <new.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <float.h>
//////////////////////////////////////////////////////////////////////////

// Special intrinsics
#include <math.h> // Should be included before intrin.h
#include <intrin.h>
#include <process.h>

//////////////////////////////////////////////////////////////////////////
// Define platform independent types.
//////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/BaseTypes.h>

#define THREADID_NULL 0
typedef long                          LONG;
typedef u8                 BYTE;
typedef u64                 threadID;
typedef u64                 DWORD;
typedef double                        real; //!< Biggest float-type on this machine.

typedef uk                         THREAD_HANDLE;
typedef uk                         EVENT_HANDLE;

typedef __int64 INT_PTR, *            PINT_PTR;
typedef unsigned __int64 UINT_PTR, *  PUINT_PTR;

typedef __int64 LONG_PTR, *           PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, * PULONG_PTR;

typedef ULONG_PTR DWORD_PTR, *        PDWORD_PTR;

#define SIZEOF_PTR 8

#ifndef FILE_ATTRIBUTE_NORMAL
	#define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif

#define TARGET_DEFAULT_ALIGN (0x8U)
