// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
   Assert dialog box for Mac OS X

   -------------------------------------------------------------------------
   История:
   - 03:09:2013: Created by Leander Beernaert

*************************************************************************/

#if defined(USE_DRX_ASSERT) && DRX_PLATFORM_IOS

static char gs_szMessage[MAX_PATH];

void DrxAssertTrace(tukk szFormat, ...)
{
	if (gEnv == 0)
	{
		return;
	}

	if (!gEnv->ignoreAllAsserts || gEnv->bTesting)
	{
		if (szFormat == NULL || szFormat[0] == '\0')
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
/*
   bool DrxAssert(tukk szCondition, tukk szFile,u32 line, bool *pIgnore)
   {
    if (!gEnv) return false;

    gEnv->pSystem->OnAssert(szCondition, gs_szMessage, szFile, line);

    if (!gEnv->bUnattendedMode && !gEnv->bIgnoreAllAsserts)
    {
        EDialogAction action = MacOSXHandleAssert(szCondition, szFile, line, gs_szMessage, gEnv->pRenderer != NULL);

        switch (action) {
            case eDAStop:
                raise(SIGABRT);
                exit(-1);
            case eDABreak:
                return true;
            case eDAIgnoreAll:
                gEnv->bIgnoreAllAsserts = true;
                break;
            case eDAIgnore:
 * pIgnore = true;
                break;
            case eDAReportAsBug:
                if ( gEnv && gEnv->pSystem)
                {
                    gEnv->pSystem->ReportBug("Assert: %s - %s", szCondition,gs_szMessage);
                }

            case eDAContinue:
            default:
                break;
        }
    }

    return false;
   }*/

bool DrxAssert(tukk szCondition, tukk szFile, u32 line, bool* pIgnore)
{
	if (!gEnv) return false;

	static i32k max_len = 4096;
	static char gs_command_str[4096];

	gEnv->pSystem->OnAssert(szCondition, gs_szMessage, szFile, line);

	size_t file_len = strlen(szFile);

	if (!gEnv->bUnattendedMode && !gEnv->ignoreAllAsserts)
	{
		DrxLogAlways("!!ASSERT!!\n\tCondition: %s\n\tMessage  : %s\n\tFile     : %s\n\tLine     : %d", szCondition, gs_szMessage, szFile, line);
	}
	return false;
}

#endif
