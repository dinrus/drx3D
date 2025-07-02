// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#define SIZEOF_PTR    8
//#define TARGET_DEFAULT_ALIGN (0x8U)
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
#include <dlfcn.h>

// Define platform independent types.
#include <drx3D/CoreX/BaseTypes.h>
typedef double                        real;

typedef u32                        DWORD;
typedef DWORD*                        LPDWORD;
typedef uint64                        DWORD_PTR;
typedef intptr_t INT_PTR, *PINT_PTR;
typedef uintptr_t UINT_PTR, *PUINT_PTR;
typedef tuk LPSTR, *PSTR;
typedef uint64                        __uint64;
#if !defined(__clang__)
typedef int64                         __int64;
#endif
typedef int64                         INT64;
typedef uint64                        UINT64;

typedef long LONG_PTR, *PLONG_PTR, *PLONG;
typedef u64 ULONG_PTR, *PULONG_PTR;

typedef u8                         BYTE;
typedef u16                        WORD;
typedef uk                         HWND;
typedef UINT_PTR                      WPARAM;
typedef LONG_PTR                      LPARAM;
typedef LONG_PTR                      LRESULT;
#define PLARGE_INTEGER LARGE_INTEGER *
typedef tukk LPCSTR, *PCSTR;
typedef long long                     LONGLONG;
typedef ULONG_PTR                     SIZE_T;
typedef u8                         byte;

typedef ukk LPCVOID;

#include "AndroidSpecific.h"

