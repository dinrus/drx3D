// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/DrxCustomTypes.h>

#ifndef SXEMA_NOP
#define SXEMA_NOP ((void)0)
#endif

#ifndef SXEMA_DEBUG_BREAK
#ifdef _RELEASE
#define SXEMA_DEBUG_BREAK
#else
#define SXEMA_DEBUG_BREAK DrxDebugBreak();
#endif
#endif

#ifndef SXEMA_FILE_NAME
#define SXEMA_FILE_NAME __FILE__
#endif

#ifndef SXEMA_LINE_NUMBER
#define SXEMA_LINE_NUMBER __LINE__
#endif

#ifndef SXEMA_FUNCTION_NAME
#ifdef _MSC_VER
#define SXEMA_FUNCTION_NAME __FUNCSIG__
#else
#define SXEMA_FUNCTION_NAME __PRETTY_FUNCTION__
#endif
#endif

#define SXEMA_PP_EMPTY
#define SXEMA_PP_COMMA                        ,
#define SXEMA_PP_LEFT_BRACKET                 (
#define SXEMA_PP_RIGHT_BRACKET                )
#define SXEMA_PP_SEMI_COLON                   ;
#define SXEMA_PP_COLON                        :

#define SXEMA_PP_NUMBER(x)                    x

#define SXEMA_PP_TO_STRING_(x)                #x
#define SXEMA_PP_TO_STRING(x)                 SXEMA_PP_TO_STRING_(x)
#define SXEMA_PP_JOIN_XY_(x, y)               x##y
#define SXEMA_PP_JOIN_XY(x, y)                SXEMA_PP_JOIN_XY_(x, y)
#define SXEMA_PP_JOIN_XYZ_(x, y, z)           x##y##z
#define SXEMA_PP_JOIN_XYZ(x, y, z)            SXEMA_PP_JOIN_XYZ_(x, y, z)
