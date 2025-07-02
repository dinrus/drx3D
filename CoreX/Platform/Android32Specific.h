// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#define SIZEOF_PTR    4
//#define TARGET_DEFAULT_ALIGN (0x4U)
#define TARGET_DEFAULT_ALIGN (16U)

// Standard includes.
#include <malloc.h>
#include <stdint.h>
#include <fcntl.h>
#include <float.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <sys/socket.h>

// Define platform independent types.
#include <drx3D/CoreX/BaseTypes.h>


typedef z64              INT64;

typedef double                        real;

typedef u32                        DWORD;
typedef DWORD*                        LPDWORD;
typedef DWORD                         DWORD_PTR;
typedef i32 INT_PTR, *PINT_PTR;
typedef u32 UINT_PTR, *PUINT_PTR;
typedef tuk LPSTR, *PSTR;
typedef u32                        __uint32;
typedef i32                         INT32;
typedef u32                        UINT32;
typedef uint64                        __uint64;
typedef int64                         INT64;
typedef uint64                        UINT64;

typedef long LONG_PTR, *PLONG_PTR, *PLONG;
typedef u64 ULONG_PTR, *PULONG_PTR;

typedef u8                 BYTE;
typedef unsigned short                WORD;
typedef i32                           INT;
typedef u32                  UINT;
typedef float                         FLOAT;
typedef uk                         HWND;
typedef UINT_PTR                      WPARAM;
typedef LONG_PTR                      LPARAM;
typedef LONG_PTR                      LRESULT;
#define PLARGE_INTEGER LARGE_INTEGER *
typedef tukk LPCSTR, *PCSTR;
typedef long long                     LONGLONG;
typedef ULONG_PTR                     SIZE_T;
typedef u8                 byte;

typedef ukk LPCVOID;

#include "AndroidSpecific.h"