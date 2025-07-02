// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
DesignerWarning.h
	- basic message box in windows to allow designers to be told something is wrong with their setup

-	[10/11/2009] : Created by James Bamford

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/DesignerWarning.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/Testing/FeatureTester.h>
#include <drx3D/Game/Testing/AutoTester.h>

#if DESIGNER_WARNING_ENABLED

static i32 s_numDesignerWarningsHit=0;

i32 DesignerWarningFunc(tukk  message)
{
	if (g_pGameCVars->designer_warning_enabled && (!gEnv->IsDedicated()))
	{
		GameWarning("!DESIGNER WARNING\n%s", message);
	}

	// kept because autotests gather all designer warnings out of logs with this form
	DrxLogAlways("---DESIGNER_WARNING: %s", message);
	DrxLogAlways("----------------------------------------");

#if ENABLE_FEATURE_TESTER
	// If feature testing is in progress, write each designer warning out as a failed feature test

	CFeatureTester * featureTester = CFeatureTester::GetInstance();
	if (featureTester)
	{
		CAutoTester * autoTestResultWriter = featureTester->GetAutoTesterIfActive();
		if (autoTestResultWriter)
		{
			DrxFixedStringT<32> warningName;
			warningName.Format("DesignerWarning%04u", s_numDesignerWarningsHit);
			autoTestResultWriter->AddSimpleTestCase("DesignerWarnings", warningName.c_str(), 0.1f, message);
		}
	}
#endif

	s_numDesignerWarningsHit++;
	return 0;
}

i32 GetNumDesignerWarningsHit()
{
	return s_numDesignerWarningsHit;
}

#endif // DESIGNER_WARNING_ENABLED