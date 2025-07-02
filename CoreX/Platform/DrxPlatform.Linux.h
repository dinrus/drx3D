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
#ifndef _DRX_PLATFORM_LINUX_H_
#define _DRX_PLATFORM_LINUX_H_

////////////////////////////////////////////////////////////////////////////
// check that we are allowed to be included
#if !defined(DRXPLATFROM_ALLOW_DETAIL_INCLUDES)
	#error Please include DrxPlatfrom.h instead of this private implementation header
#endif

////////////////////////////////////////////////////////////////////////////
// util macros for __DETAIL__LINK_THIRD_PARTY_LIBRARY and __DETAIL__LINK_SYSTEM_PARTY_LIBRARY
// SNC is a little strange with macros and pragmas
// the lib pragma requieres a string literal containing a escaped string literal
// eg _Pragma ("comment (lib, \"<lib>\")")
#define __DETAIL__CREATE_PRAGMA(x) _Pragma(DRX_CREATE_STRING(x))

////////////////////////////////////////////////////////////////////////////
// Create a string from an Preprocessor macro or a literal text
#define DRX_DETAIL_CREATE_STRING(string) # string
#define DRX_CREATE_STRING(string)        DRX_DETAIL_CREATE_STRING(string)

////////////////////////////////////////////////////////////////////////////
#define __DETAIL__LINK_THIRD_PARTY_LIBRARY(name) \
  __DETAIL__CREATE_PRAGMA("comment(lib, \"" DRX_CREATE_STRING(CODE_BASE_FOLDER) name "\")")

////////////////////////////////////////////////////////////////////////////
#define __DETAIL__LINK_SYSTEM_PARTY_LIBRARY(name) \
  __DETAIL__CREATE_PRAGMA("comment(lib, \"" name "\")")

#endif // _DRX_PLATFORM_LINUX_H_
