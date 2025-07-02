// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: TODO macro messages

-------------------------------------------------------------------------
История:
- 31:03:2009: Created by Filipe Amim

*************************************************************************/
#ifndef __DRX_MACROS_H__
#define __DRX_MACROS_H__

// Disabled by Will with Filipe's consent to keep the compile output more readable...
// #define DRX_SHOW_COMPILE_MESSAGES
// #define DRX_COMPILE_MESSAGES_AS_WARNINGS


// messaging
# define DRX_PP_STRINGIZE(L)					#L
# define DRX_PP_APPLY(function, target)			function(target)
# define DRX_PP_LINE							DRX_PP_APPLY(DRX_PP_STRINGIZE, __LINE__)
# define DRX_PP_FORMAT_DATE(day, month, year)	#day "/" #month "/" #year
# define DRX_PP_FORMAT_FILE_LINE				__FILE__ "(" DRX_PP_LINE ")"
#
# ifdef DRX_COMPILE_MESSAGES_AS_WARNINGS
#	define DRX_PP_FORMAT_MESSAGE(day, month, year, reason, message)		DRX_PP_FORMAT_FILE_LINE " : Drx message(" #reason "): " DRX_PP_FORMAT_DATE(day, month, year) ": " message
# else
#	define DRX_PP_FORMAT_MESSAGE(day, month, year, reason, message)		DRX_PP_FORMAT_FILE_LINE " : cry " #reason ": " DRX_PP_FORMAT_DATE(day, month, year) ": " message
# endif
#
#
# if defined(_MSC_VER)
#	define DRX_PP_PRINT(msg)					__pragma(message(msg))
# else
#	define DRX_PP_PRINT(msg)
# endif
#
#
# if defined(DRX_SHOW_COMPILE_MESSAGES)
#	define DRX_PRINT(msg)								DRX_PP_PRINT(msg)
#	define DRX_MESSAGE(msg)								DRX_PP_PRINT(DRX_PP_FORMAT_FILE_LINE " : " msg)
#	define DRX_TODO(day, month, year, message)			DRX_PP_PRINT(DRX_PP_FORMAT_MESSAGE(day, month, year, TODO, message))
#	define DRX_HACK(day, month, year, message)			DRX_PP_PRINT(DRX_PP_FORMAT_MESSAGE(day, month, year, HACK, message))
#	define DRX_FIXME(day, month, year, message)			DRX_PP_PRINT(DRX_PP_FORMAT_MESSAGE(day, month, year, FIXME, message))
# else
#	define DRX_PRINT(msg)
#	define DRX_MESSAGE(msg)
#	define DRX_TODO(day, month, year, message)
#	define DRX_HACK(day, month, year, message)
#	define DRX_FIXME(day, month, year, message)
# endif

// TODO: improve these macros to behave like the DrxUnitAsserts 
#define ASSERT_ARE_EQUAL(expected, actual)									DRX_ASSERT( expected == actual )
#define ASSERT_ARE_NOT_EQUAL(expected, actual)							DRX_ASSERT( expected != actual )
#define ASSERT_IS_TRUE(cond)																DRX_ASSERT(cond )
#define ASSERT_IS_FALSE(cond)																DRX_ASSERT( !cond )
#define ASSERT_IS_NULL(ptr)																	DRX_ASSERT( ptr == NULL )
#define ASSERT_IS_NOT_NULL(ptr)															DRX_ASSERT( ptr != NULL )
#define ASSERT_FLOAT_ARE_EQUAL(expected, actual, epsilon)		DRX_ASSERT( fabs( (expected) - (actual) ) <= (epsilon) )

#endif
