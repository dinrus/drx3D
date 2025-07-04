// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   Описание:
   A multiplatform base class for handling errors and collecting call stacks

   -------------------------------------------------------------------------
   История:
   - 12:10:2009	: Created by Alex McCarthy
*************************************************************************/

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/IDebugCallStack.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>
#include <drx3D/Sys/System.h>

//#if !(DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID)

#include <drx3D/Sys/ISystem.h>

tukk const IDebugCallStack::s_szFatalErrorCode = "FATAL_ERROR";

IDebugCallStack::IDebugCallStack() : m_bIsFatalError(false), m_postBackupProcess(0)
{

}

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
IDebugCallStack* IDebugCallStack::instance()
{
	static IDebugCallStack sInstance;
	return &sInstance;
}
#endif

void IDebugCallStack::FileCreationCallback(void (* postBackupProcess)())
{
	m_postBackupProcess = postBackupProcess;
}
//////////////////////////////////////////////////////////////////////////
void IDebugCallStack::LogCallstack()
{
	CollectCurrentCallStack();    // is updating m_functions

	WriteLineToLog("=============================================================================");
	i32 len = (i32)m_functions.size();
	for (i32 i = 0; i < len; i++)
	{
		tukk str = m_functions[i].c_str();
		WriteLineToLog("%2d) %s", len - i, str);
	}
	WriteLineToLog("=============================================================================");
}

tukk IDebugCallStack::TranslateExceptionCode(DWORD dwExcept)
{
	switch (dwExcept)
	{
#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_APPLE && !DRX_PLATFORM_ORBIS
	case EXCEPTION_ACCESS_VIOLATION:
		return "EXCEPTION_ACCESS_VIOLATION";
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		return "EXCEPTION_DATATYPE_MISALIGNMENT";
		break;
	case EXCEPTION_BREAKPOINT:
		return "EXCEPTION_BREAKPOINT";
		break;
	case EXCEPTION_SINGLE_STEP:
		return "EXCEPTION_SINGLE_STEP";
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		return "EXCEPTION_FLT_DENORMAL_OPERAND";
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		return "EXCEPTION_FLT_INEXACT_RESULT";
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		return "EXCEPTION_FLT_INVALID_OPERATION";
		break;
	case EXCEPTION_FLT_OVERFLOW:
		return "EXCEPTION_FLT_OVERFLOW";
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		return "EXCEPTION_FLT_STACK_CHECK";
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		return "EXCEPTION_FLT_UNDERFLOW";
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		return "EXCEPTION_INT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_INT_OVERFLOW:
		return "EXCEPTION_INT_OVERFLOW";
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		return "EXCEPTION_PRIV_INSTRUCTION";
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		return "EXCEPTION_IN_PAGE_ERROR";
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		return "EXCEPTION_ILLEGAL_INSTRUCTION";
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
		break;
	case EXCEPTION_STACK_OVERFLOW:
		return "EXCEPTION_STACK_OVERFLOW";
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		return "EXCEPTION_INVALID_DISPOSITION";
		break;
	case EXCEPTION_GUARD_PAGE:
		return "EXCEPTION_GUARD_PAGE";
		break;
	case EXCEPTION_INVALID_HANDLE:
		return "EXCEPTION_INVALID_HANDLE";
		break;
	//case EXCEPTION_POSSIBLE_DEADLOCK:	return "EXCEPTION_POSSIBLE_DEADLOCK";	break ;

	case STATUS_FLOAT_MULTIPLE_FAULTS:
		return "STATUS_FLOAT_MULTIPLE_FAULTS";
		break;
	case STATUS_FLOAT_MULTIPLE_TRAPS:
		return "STATUS_FLOAT_MULTIPLE_TRAPS";
		break;

#endif
	default:
		return "Unknown";
		break;
	}
}

void IDebugCallStack::PutVersion(tuk str)
{
	if (!gEnv || !gEnv->pSystem)
		return;

	char sFileVersion[128];
	gEnv->pSystem->GetFileVersion().ToString(sFileVersion);

	char sProductVersion[128];
	gEnv->pSystem->GetProductVersion().ToString(sProductVersion);

	//! Get time.
	time_t ltime;
	time(&ltime);
	tm* today = localtime(&ltime);

	char s[1024];
	//! Use strftime to build a customized time string.
	strftime(s, 128, "Logged at %#c\n", today);
	strcat(str, s);
	drx_sprintf(s, "FileVersion: %s\n", sFileVersion);
	strcat(str, s);
	drx_sprintf(s, "ProductVersion: %s\n", sProductVersion);
	strcat(str, s);

	if (gEnv->pLog)
	{
		tukk logfile = gEnv->pLog->GetFileName();
		if (logfile)
		{
			drx_sprintf(s, "LogFile: %s\n", logfile);
			strcat(str, s);
		}
	}

	if (gEnv->pConsole)
	{
		if (ICVar* pCVarGameDir = gEnv->pConsole->GetCVar("sys_game_folder"))
		{
			drx_sprintf(s, "GameDir: %s\n", pCVarGameDir->GetString());
			strcat(str, s);
		}
	}

#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_APPLE && !DRX_PLATFORM_DURANGO && !DRX_PLATFORM_ORBIS
	GetModuleFileNameA(NULL, s, sizeof(s));
	strcat(str, "Executable: ");
	strcat(str, s);
	strcat(str, "\n");
#endif
}

//Crash the application, in this way the debug callstack routine will be called and it will create all the necessary files (error.log, dump, and eventually screenshot)
void IDebugCallStack::FatalError(tukk description)
{
	m_bIsFatalError = true;
	WriteLineToLog(description);

#if !defined(_RELEASE) && !defined(DRX_PLATFORM_WINDOWS)
	bool bShowDebugScreen = g_cvars.sys_no_crash_dialog == 0;
	// showing the debug screen is not safe when not called from mainthread
	// it normally leads to a infinity recursion followed by a stackoverflow, preventing
	// useful callstacks, thus they are disabled
	bShowDebugScreen = bShowDebugScreen && gEnv->mMainThreadId == DrxGetCurrentThreadId();
	if (bShowDebugScreen)
	{
		DrxMessageBox(description, "DinrusX Fatal Error", eMB_Error);
	}
#endif

#if DRX_PLATFORM_WINDOWS || !defined(_RELEASE)
	__debugbreak(); // We're intentionally stopping execution and crashing here.
#endif
}

void IDebugCallStack::WriteLineToLog(tukk format, ...)
{
	SCOPED_ALLOW_FILE_ACCESS_FROM_THIS_THREAD();

	if (gEnv && gEnv->pLog)
	{
		tukk logFilename = gEnv->pLog->GetFileName();

		va_list ArgList;
		char szBuffer[MAX_WARNING_LENGTH];
		va_start(ArgList, format);
		drx_vsprintf(szBuffer, format, ArgList);
		drx_strcat(szBuffer, "\n");
		va_end(ArgList);

		FILE* f = fxopen(logFilename, "a+t");
		if (f)
		{
			fputs(szBuffer, f);
			fflush(f);
			fclose(f);
		}
	}
}

void IDebugCallStack::Screenshot(tukk szFileName)
{
#if !defined(DEDICATED_SERVER)
	WriteLineToLog("Attempting to create error screenshot \"%s\"", szFileName);

	static i32 g_numScreenshots = 0;
	if (gEnv && gEnv->pRenderer && !g_numScreenshots++)
	{
		const bool result = gEnv->pRenderer->ScreenShot(szFileName);
		WriteLineToLog(result ?
			"Successfully created screenshot." :
			"Error creating screenshot.");
	}
	else
	{
		WriteLineToLog("Ignoring multiple calls to Screenshot");
	}
#endif //!defined(DEDICATED_SERVER)
}

//#endif
