// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Linux32Specific.h
//  Version:     v1.00
//  Created:     05/03/2004 by MarcoK.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64, GCC 3.2
//  Описание: Specific to Linux declarations, inline functions etc.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#pragma once

#define __debugbreak() raise(SIGTRAP)
#define RC_EXECUTABLE "rc"
#define USE_CRT       1
#define SIZEOF_PTR    4
#define TARGET_DEFAULT_ALIGN (0x4U)

//////////////////////////////////////////////////////////////////////////
// Standard includes.
//////////////////////////////////////////////////////////////////////////
#include <malloc.h>
//#include <winbase.h>
#include <stdint.h>
#include <sys/dir.h>
#include <sys/io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Define platform independent types.
//////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/BaseTypes.h>

typedef z64              INT64;

typedef double                        real;

typedef u64                 DWORD;
typedef u64*                LPDWORD;
typedef DWORD                         DWORD_PTR;
typedef i32 INT_PTR, *                PINT_PTR;
typedef u32 UINT_PTR, *      PUINT_PTR;
typedef tuk LPSTR, *                PSTR;

typedef long LONG_PTR, * PLONG_PTR, * PLONG;
typedef u64 ULONG_PTR, *    PULONG_PTR;

typedef u8                 BYTE;
typedef unsigned short                WORD;
typedef uk                         HWND;
typedef UINT_PTR                      WPARAM;
typedef LONG_PTR                      LPARAM;
typedef LONG_PTR                      LRESULT;
#define PLARGE_INTEGER LARGE_INTEGER *
typedef tukk LPCSTR, *         PCSTR;
typedef long long                     LONGLONG;
typedef ULONG_PTR                     SIZE_T;
typedef u8                 byte;

#define __int64   long long


#include "LinuxSpecific.h"

