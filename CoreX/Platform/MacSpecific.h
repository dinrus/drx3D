// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MacSpecific.h
//  Version:     v1.00
//  Created:     Mathieu Pinard based on the LinuxSpecifc files
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64, GCC
//  Описание: Mac declarations
// -------------------------------------------------------------------------
//  История:
//  - Jul 30, 2013: Leander Beernaert - Common definitions moved to AppleSpecific.h
////////////////////////////////////////////////////////////////////////////
#ifndef __MACSPECIFIC_H__
#define __MACSPECIFIC_H__

#include "AppleSpecific.h"
#include <cstddef>
#include <cfloat>
#include <cstdlib>
#include <xmmintrin.h>

#define USE_CRT    1
#define SIZEOF_PTR 8

typedef uint64_t threadID;

// curses.h stubs for PDcurses keys
#define PADENTER KEY_MAX + 1
#define CTL_HOME KEY_MAX + 2
#define CTL_END  KEY_MAX + 3
#define CTL_PGDN KEY_MAX + 4
#define CTL_PGUP KEY_MAX + 5

// stubs for virtual keys, isn't used on Mac
#define VK_UP               0
#define VK_DOWN             0
#define VK_RIGHT            0
#define VK_LEFT             0
#define VK_CONTROL          0
#define VK_SCROLL           0

#define MAC_NOT_IMPLEMENTED assert(false);

#define MSG_NOSIGNAL        SO_NOSIGPIPE

typedef enum
{
	eDAContinue,
	eDAIgnore,
	eDAIgnoreAll,
	eDABreak,
	eDAStop,
	eDAReportAsBug
} EDialogAction;

extern EDialogAction MacOSXHandleAssert(tukk condition, tukk file, i32 line, tukk reason, bool);

#endif //__MACSPECIFIC_H__
