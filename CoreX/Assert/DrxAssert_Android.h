// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
    Assert dialog box for android.

   -------------------------------------------------------------------------
   История:
*************************************************************************/

#if !defined(__DRXASSERT_ANDROID_H__) && defined(USE_DRX_ASSERT) && DRX_PLATFORM_ANDROID
#define __DRXASSERT_ANDROID_H__

static char gs_szMessage[MAX_PATH];

void DrxAssertTrace(tukk szFormat, ...)
{
	if (gEnv == 0)
	{
		return;
	}

	if (!gEnv->ignoreAllAsserts || gEnv->bTesting)
	{
		if (szFormat == NULL)
		{
			gs_szMessage[0] = '\0';
		}
		else
		{
			va_list args;
			va_start(args, szFormat);
			drx_vsprintf(gs_szMessage, szFormat, args);
			va_end(args);
		}
	}
}

bool DrxAssert(tukk szCondition, tukk szFile, u32 line, bool* pIgnore)
{
	IF_UNLIKELY (!gEnv || !gEnv->pSystem || !gEnv->pLog)
	{
		return false;
	}

	gEnv->pSystem->OnAssert(szCondition, gs_szMessage, szFile, line);

	gEnv->pLog->LogError("Assertion Failed! %s:%d reason:%s", szFile, line, gs_szMessage[0] ? gs_szMessage : "<empty>");

	return false;
}

#endif // __DRXASSERT_ANDROID_H__
