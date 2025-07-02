// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Note: Can't use #pragma once here, since (like assert.h) this file CAN be included more than once.
// Each time it's included, it will re-define assert to match DRX_ASSERT.
// This behavior can be used to "fix" 3rdParty headers that #include <assert.h> (by including DrxAssert.h after the 3rdParty header).
// In the case where assert's definition gets trampled, just including DrxAssert.h should fix that problem for the remainder of the file.
#ifndef DRX_ASSERT_H_INCLUDED
	#define DRX_ASSERT_H_INCLUDED

// FORCE_STANDARD_ASSERT is set by some tools (like RC)
	#if !defined(_RELEASE) && !defined(FORCE_STANDARD_ASSERT)
		#define USE_DRX_ASSERT
	#else
		#undef USE_DRX_ASSERT
	#endif

//-----------------------------------------------------------------------------------------------------
// Use like this:
// DRX_ASSERT(expression);
// DRX_ASSERT_MESSAGE(expression,"Useful message");
// DRX_ASSERT_TRACE(expression,("This should never happen because parameter %d named %s is %f",iParameter,szParam,fValue));
//-----------------------------------------------------------------------------------------------------

	#if defined(USE_DRX_ASSERT)

enum class EDrxAssertLevel
{
	Disabled,
	Enabled,
	FatalErrorOnAssert,
	DebugBreakOnAssert
};

bool DrxAssertIsEnabled();
void DrxAssertTrace(tukk , ...);
void DrxLogAssert(tukk , tukk , u32, bool*);
bool DrxAssert(tukk , tukk , u32, bool*);
void DrxDebugBreak();

namespace Detail
{
struct SAssertData
{
	char const* const szExpression;
	char const* const szFunction;
	char const* const szFile;
	long const        line;
};

struct SAssertCond
{
	bool bIgnoreAssert;
	bool bLogAssert;
};

bool DrxAssertHandler(SAssertData const&, SAssertCond&);
bool  DrxAssertHandler(SAssertData const& data, SAssertCond& cond, char const* const szMessage);

template<typename ... TraceArgs>
bool DrxAssertHandler(SAssertData const& data, SAssertCond& cond, char const* const szFormattedMessage, TraceArgs ... traceArgs)
{
	DrxAssertTrace(szFormattedMessage, traceArgs ...);
	return DrxAssertHandler(data, cond);
}
} //endns Detail

//! The code to insert when assert is used.
		#define DRX_AUX_VA_ARGS(...)    __VA_ARGS__
		#define DRX_AUX_STRIP_PARENS(X) X

		#define DRX_ASSERT_MESSAGE(condition, ...)                                 \
		  do                                                                       \
		  {                                                                        \
		    IF_UNLIKELY (!(condition))                                             \
		    {                                                                      \
		      ::Detail::SAssertData const assertData =                             \
		      {                                                                    \
		        # condition,                                                       \
		        __func__,                                                          \
		        __FILE__,                                                          \
		        __LINE__                                                           \
		      };                                                                   \
		      static ::Detail::SAssertCond assertCond =                            \
		      {                                                                    \
		        false, true                                                        \
		      };                                                                   \
		      if (::Detail::DrxAssertHandler(assertData, assertCond, __VA_ARGS__)) \
		        DrxDebugBreak();                                                   \
		    }                                                                      \
		    PREFAST_ASSUME(condition);                                         \
		  } while (false);

		#define DRX_ASSERT_TRACE(condition, parenthese_message) \
		  DRX_ASSERT_MESSAGE(condition, DRX_AUX_STRIP_PARENS(DRX_AUX_VA_ARGS parenthese_message))

		#define DRX_ASSERT(condition) DRX_ASSERT_MESSAGE(condition, nullptr)

	#else

//! Use the platform's default assert.
		#include <assert.h>
		#define DRX_ASSERT_TRACE(condition, parenthese_message) assert(condition)
		#define DRX_ASSERT_MESSAGE(condition, ... )             assert(condition)
		#define DRX_ASSERT(condition)                           assert(condition)

	#endif

	#ifdef IS_EDITOR_BUILD
		#undef  Q_ASSERT
		#undef  Q_ASSERT_X
		#define Q_ASSERT(cond)                DRX_ASSERT_MESSAGE(cond, "Q_ASSERT")
		#define Q_ASSERT_X(cond, where, what) DRX_ASSERT_MESSAGE(cond, "Q_ASSERT_X" where what)
	#endif

	template<typename T>
	inline T const& DrxVerify(T const& expr, tukk szMessage)
	{
		DRX_ASSERT_MESSAGE(expr, szMessage);
		return expr;
	}

	#define DRX_VERIFY(expr) DrxVerify(expr, #expr)

//! This forces boost to use DRX_ASSERT, regardless of what it is defined as.
	#define BOOST_ENABLE_ASSERT_HANDLER
namespace boost
{
inline void assertion_failed_msg(char const* const szExpr, char const* const szMsg, char const* const szFunction, char const* const szFile, long const line)
{
	#ifdef USE_DRX_ASSERT
		::Detail::SAssertData const assertData =
		{
			szExpr, szFunction, szFile, line
		};
		static ::Detail::SAssertCond assertCond =
		{
			false, true
		};
		if (::Detail::DrxAssertHandler(assertData, assertCond, szMsg))
			DrxDebugBreak();
	#else
		DRX_ASSERT_TRACE(false, ("An assertion failed in boost: expr=%s, msg=%s, function=%s, file=%s, line=%d", szExpr, szMsg, szFunction, szFile, (i32)line));
	#endif // USE_DRX_ASSERT
}

inline void assertion_failed(char const* const szExpr, char const* const szFunction, char const* const szFile, long const line)
{
	assertion_failed_msg(szExpr, "BOOST_ASSERT", szFunction, szFile, line);
}
} //endns boost

// Remember the value of USE_DRX_ASSERT
	#if defined(USE_DRX_ASSERT)
		#define DRX_WAS_USE_ASSERT_SET 1
	#else
		#define DRX_WAS_USE_ASSERT_SET 0
	#endif

#endif

// Note: This ends the "once per compilation unit" part of this file, from here on, the code is included every time
// See the top of this file on the reasoning behind this.
#if defined(USE_DRX_ASSERT)
	#if DRX_WAS_USE_ASSERT_SET != 1
		#error USE_DRX_ASSERT value changed between includes of DrxAssert.h (was undefined, now defined)
	#endif
	#undef assert
	#define assert DRX_ASSERT
#else
	#if DRX_WAS_USE_ASSERT_SET != 0
		#error USE_DRX_ASSERT value changed between includes of DrxAssert.h (was defined, now undefined)
	#endif
#endif