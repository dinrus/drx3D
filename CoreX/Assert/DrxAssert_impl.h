// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "DrxAssert.h"
#if defined(USE_DRX_ASSERT)

	#if !DRX_PLATFORM_WINDOWS
void DrxLogAssert(tukk _pszCondition, tukk _pszFile, u32 _uiLine, bool* _pbIgnore)
{
	// Empty on purpose
}
	#endif

	#if DRX_PLATFORM_DURANGO
		#include <drx3D/CoreX/Assert/DrxAssert_Durango.h>
	#elif DRX_PLATFORM_MAC
		#include <drx3D/CoreX/Assert/DrxAssert_Mac.h>
	#elif DRX_PLATFORM_IOS
		#include <drx3D/CoreX/Assert/DrxAssert_iOS.h>
	#elif DRX_PLATFORM_ANDROID
		#include <drx3D/CoreX/Assert/DrxAssert_Android.h>
	#elif DRX_PLATFORM_LINUX
		#include <drx3D/CoreX/Assert/DrxAssert_Linux.h>
	#elif DRX_PLATFORM_WINDOWS
		#include <drx3D/CoreX/Assert/DrxAssert_Windows.h>
	#elif DRX_PLATFORM_ORBIS
		#include <drx3D/CoreX/Assert/DrxAssert_Orbis.h>
	#else

// Pull in system assert macro
		#include <assert.h>

void DrxAssertTrace(tukk , ...)
{
	// Empty on purpose
}

bool DrxAssert(tukk , tukk , u32, bool*)
{
	assert(false && "DrxAssert не реализован");
	return true;
}

// Restore previous assert definition
		#include "DrxAssert.h"

	#endif

//! Check if assert is enabled (the same on every platform).
bool DrxAssertIsEnabled()
{
	#if defined(_DEBUG)
	static const bool defaultIfUnknown = true;
	#else
	static const bool defaultIfUnknown = false;
	#endif

	const bool suppressGlobally = gEnv ? gEnv->drxAssertLevel == EDrxAssertLevel::Disabled : !defaultIfUnknown;
	const bool suppressedByUser = gEnv ? gEnv->ignoreAllAsserts : !defaultIfUnknown;
#ifdef eDrxModule
	const bool suppressedCurrentModule = gEnv && gEnv->pSystem ? !gEnv->pSystem->AreAssertsEnabledForModule(eDrxModule) : !defaultIfUnknown;
#else
	const bool suppressedCurrentModule = false;
#endif

	return !(suppressGlobally || suppressedByUser || suppressedCurrentModule);
}

namespace Detail
{

bool DrxAssertHandler(SAssertData const& data, SAssertCond& cond, char const* const szMessage)
{
	DrxAssertTrace(szMessage);
	return DrxAssertHandler(data, cond);
}

NO_INLINE
bool DrxAssertHandler(SAssertData const& data, SAssertCond& cond)
{
	if (cond.bLogAssert) // Just log assert the first time
	{
		DrxLogAssert(data.szExpression, data.szFile, data.line, &cond.bIgnoreAssert);
		cond.bLogAssert = false;
	}

	if (!cond.bIgnoreAssert && DrxAssertIsEnabled()) // Don't show assert once it was ignored
	{
		if (DrxAssert(data.szExpression, data.szFile, data.line, &cond.bIgnoreAssert))
			return true;
	}
	return false;
}
} //endns Detail

#endif // defined(USE_DRX_ASSERT)