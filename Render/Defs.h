// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#if !DRX_PLATFORM_ORBIS || (defined(__GNUC__) && __GNUC__ < 4)
	#include <signal.h>
#endif
#include <errno.h>
#include <string.h>

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef ABS
	#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#if !defined(SIGN) && !defined(DO_AMIGAOS)
	#define SIGN(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
#endif

#define EPSILON       0.001f     /* Small value */
#define SMALL_EPSILON 0.000001f  /* Very small value */
#ifndef PI
	#define PI          3.14159265358979323f /* You know this number, don't you? */
#endif
#ifndef M_PI
	#define M_PI PI
#endif
#ifndef M_PI_2
	#define M_PI_2 (PI / 2)
#endif //M_PI_2

#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_MAC
	#if defined(COMP_WCC)
		#define strcasecmp  stricmp
		#define strncasecmp strnicmp
	#endif

	#if defined(_MSC_VER)
		#define strcasecmp  _stricmp
		#define strncasecmp _strnicmp
	#endif
#endif

#if DRX_PLATFORM_X86

// This is 'stolen' from someone (I don't remember who anymore). It
// is a nice and fast way to convert a floating point number to i32
// (only works on a i386 type processor).
// It is equivalent to 'i=(i32)(f+.5)'.
	#define FIST_MAGIC ((float)((((65536.0 * 65536.0 * 16) + (65536.0 * 0.5)) * 65536.0)))
inline long QuickRound(float inval)
{
	double dtemp = FIST_MAGIC + inval;
	return ((*(long*)&dtemp) - 0x80000000);
}

inline long QuickInt(float inval)
{
	double dtemp = FIST_MAGIC + (inval - .4999f);
	return ((*(long*)&dtemp) - 0x80000000);
}

// This is my own invention derived from the previous one. This converts
// a floating point number to a 16.16 fixed point integer. It is
// equivalent to 'i=(i32)(f*65536.)'.
	#define FIST_MAGIC2 ((float)((((65536.0 * 16) + (0.5)) * 65536.0)))
inline long QuickInt16(float inval)
{
	double dtemp = FIST_MAGIC2 + inval;
	return ((*(long*)&dtemp) - 0x80000000);
}
#endif // DRX_PLATFORM_X86

#ifdef PROC_M68K

	#define FIST_MAGIC ((((65536.0 * 65536.0 * 16) + (65536.0 * 0.5)) * 65536.0))
inline long QuickRound(float inval)
{
	double dtemp = FIST_MAGIC + inval;
	return (*(((long*)&dtemp) + 1)) - 0x80000000;
}

inline long QuickInt(float inval)
{
	double dtemp = FIST_MAGIC + (inval - .4999);
	return (*(((long*)&dtemp) + 1)) - 0x80000000;
}

	#define FIST_MAGIC2 ((((65536.0 * 16) + (0.5)) * 65536.0))
inline long QuickInt16(float inval)
{
	double dtemp = FIST_MAGIC2 + inval;
	return (*(((long*)&dtemp) + 1)) - 0x80000000;
}
#endif

#if DRX_PLATFORM_X86 || defined(PROC_M68K)
	#define QRound(x) QuickRound(x)
	#define QInt(x)   QuickInt(x)
	#define QInt16(x) QuickInt16(x)
#else
	#define QRound(x) ((i32)((x) + .5))
	#define QInt(x)   ((i32)(x))
	#define QInt16(x) ((i32)((x) * 65536.))
#endif

// @@@ I don't know if there is a better way to convert
// a floating point to 8:24 fixed point (one with constants
// like the tricks above instead of the multiplication).
#define QInt24(x) (QInt16(((x) * 256.0f)))

#if STATS
	#define STAT(x) x
#else
	#define STAT(x)
#endif

//#define SMALL_Z .01
#define SMALL_Z       0.1f

#define USE_OCCLUSION 0 // Experimental feature, will not work in this version.

// Some useful macros: these should be true at least for 32-bit processors
#define LONGFROM2SHORT(s1, s2) (((short)s1) << 16 | (((short)s2) & 0xffff))
#define SHORT1FROMLONG(l)      (short)(((i32)l) >> 16)
#define SHORT2FROMLONG(l)      (short)(((i32)l) & 0xffff)

#endif /*DEF_H*/
