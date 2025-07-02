// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   WindowsErrorReporting.cpp
//  Created:     16/11/2006 by Timur.
//  Описание: Support for Windows Error Reporting (WER)
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>

#if DRX_PLATFORM_WINDOWS

	#include <drx3D/Sys/System.h>
	#include <tchar.h>
	#include "errorrep.h"
	#include <drx3D/Sys/ISystem.h>
	#include "dbghelp.h"

static WCHAR szPath[MAX_PATH + 1];
static WCHAR szFR[] = L"\\System32\\FaultRep.dll";

WCHAR* GetFullPathToFaultrepDll(void)
{
	CHAR* lpRet = NULL;
	UINT rc;

	rc = GetSystemWindowsDirectoryW(szPath, ARRAYSIZE(szPath));
	if (rc == 0 || rc > ARRAYSIZE(szPath) - ARRAYSIZE(szFR) - 1)
		return NULL;

	wcscat(szPath, szFR);
	return szPath;
}

typedef BOOL (WINAPI * MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                          CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                          CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                          CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
                                          );

//////////////////////////////////////////////////////////////////////////
LONG WINAPI DinrusXExceptionFilterMiniDump(struct _EXCEPTION_POINTERS* pExceptionPointers, tukk szDumpPath, MINIDUMP_TYPE mdumpValue)
{
	LONG lRet = EXCEPTION_CONTINUE_SEARCH;
	HWND hParent = NULL;
	HMODULE hDll = NULL;
	//	char szDbgHelpPath[_MAX_PATH];

	if (hDll == NULL)
	{
		// load any version we can
		hDll = ::LoadLibraryA("DBGHELP.DLL");
	}

	const TCHAR* szResult = NULL;
	char szLogMessage[_MAX_PATH + 1024] = { 0 };// extra data for prefix

	//TCHAR * m_szAppName = _T("CE2Dump");
	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
		if (pDump)
		{
			// ask the user if they want to save a dump file
			if (true /*::MessageBox( NULL, "Something bad happened in your program, would you like to save a diagnostic file?", m_szAppName, MB_YESNO )==IDYES*/)
			{
				// create the file
				HANDLE hFile = ::CreateFile(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				                            FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile != INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionPointers;
					ExInfo.ClientPointers = NULL;

					BOOL bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdumpValue, &ExInfo, NULL, NULL);
					if (bOK)
					{
						drx_sprintf(szLogMessage, "Saved dump file to '%s'", szDumpPath);
						szResult = szLogMessage;
						lRet = EXCEPTION_EXECUTE_HANDLER;
					}
					else
					{
						drx_sprintf(szLogMessage, "Failed to save dump file to '%s' (error %d)", szDumpPath, GetLastError());
						szResult = szLogMessage;
					}
					::CloseHandle(hFile);
				}
				else
				{
					drx_sprintf(szLogMessage, "Failed to create dump file '%s' (error %d)", szDumpPath, GetLastError());
					szResult = szLogMessage;
				}
			}
		}
		else
		{
			szResult = "DBGHELP.DLL too old";
		}
	}
	else
	{
		szResult = "DBGHELP.DLL not found";
	}
	DrxLogAlways("%s", szResult);

	return lRet;
}

/*
   struct AutoSetDinrusXExceptionFilter
   {
   AutoSetDinrusXExceptionFilter()
   {
    WCHAR * psz = GetFullPathToFaultrepDll();
    SetUnhandledExceptionFilter(DinrusXExceptionFilterWER);
   }
   };
   AutoSetDinrusXExceptionFilter g_AutoSetDinrusXExceptionFilter;
 */

//////////////////////////////////////////////////////////////////////////
LONG WINAPI DinrusXExceptionFilterWER(struct _EXCEPTION_POINTERS* pExceptionPointers)
{

	if (g_cvars.sys_WER > 1)
	{
		char szScratch[_MAX_PATH];
		tukk szDumpPath = gEnv->pDrxPak->AdjustFileName("%USER%/CE2Dump.dmp", szScratch, 0);

		MINIDUMP_TYPE mdumpValue = (MINIDUMP_TYPE)(MiniDumpNormal);
		if (g_cvars.sys_WER > 1)
			mdumpValue = (MINIDUMP_TYPE)(g_cvars.sys_WER - 2);

		return DinrusXExceptionFilterMiniDump(pExceptionPointers, szDumpPath, mdumpValue);
	}

	LONG lRet = EXCEPTION_CONTINUE_SEARCH;
	WCHAR* psz = GetFullPathToFaultrepDll();
	if (psz)
	{
		HMODULE hFaultRepDll = LoadLibraryW(psz);
		if (hFaultRepDll)
		{
			pfn_REPORTFAULT pfn = (pfn_REPORTFAULT)GetProcAddress(hFaultRepDll, "ReportFault");
			if (pfn)
			{
				EFaultRepRetVal rc = pfn(pExceptionPointers, 0);
				lRet = EXCEPTION_EXECUTE_HANDLER;
			}
			FreeLibrary(hFaultRepDll);
		}
	}
	return lRet;
}

#endif // DRX_PLATFORM_WINDOWS
