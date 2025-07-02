// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrxPlatform.h
//  Version:     v1.00
//  Created:     31/01/2013 by Christopher Bolte.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef _DRX_PLATFORM_WIN32_H_
#define _DRX_PLATFORM_WIN32_H_

////////////////////////////////////////////////////////////////////////////
// check that we are allowed to be included
#if !defined(DRXPLATFROM_ALLOW_DETAIL_INCLUDES)
	#error Please include DrxPlatfrom.h instead of this private implementation header
#endif

////////////////////////////////////////////////////////////////////////////
// Create a string from an Preprocessor macro or a literal text
#define DRX_DETAIL_CREATE_STRING(string) # string
#define DRX_CREATE_STRING(string)        DRX_DETAIL_CREATE_STRING(string)
#define RESOLVE_MACRO(x)                 x

////////////////////////////////////////////////////////////////////////////
#define __DETAIL__LINK_THIRD_PARTY_LIBRARY(name)                                                \
  __pragma(message(__FILE__ "(" DRX_CREATE_STRING(__LINE__) "): Including SDK Library: " name)) \
  __pragma(comment(lib, RESOLVE_MACRO(CODE_BASE_FOLDER) name))

////////////////////////////////////////////////////////////////////////////
#define __DETAIL__LINK_SYSTEM_PARTY_LIBRARY(name)                                                  \
  __pragma(message(__FILE__ "(" DRX_CREATE_STRING(__LINE__) "): Including System Library: " name)) \
  __pragma(comment(lib, name))

#endif // _DRX_PLATFORM_WIN32_H_
