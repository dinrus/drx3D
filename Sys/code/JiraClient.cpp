// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: A jira client used to submit bug reports.

   -------------------------------------------------------------------------
   История:
   - 06:03:2009 : Created By Alex McCarthy

*************************************************************************/

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/JiraClient.h>

#if DRX_PLATFORM_WINDOWS && !defined(_RELEASE)

#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

namespace {
bool FileExists(tukk szFileName)
{
	FILE* pFile = fopen(szFileName, "r");
	if (pFile)
	{
		fclose(pFile);
		return true;
	}
	else
	{
		return false;
	}
}
};

bool CJiraClient::ReportBug()
{
	const ICVar* pCVarCrashHandler = gEnv->pConsole->GetCVar("sys_enable_crash_handler");
	if (pCVarCrashHandler && pCVarCrashHandler->GetIVal() == 0)
	{
		return true;
	}

	string crashHandlerPath = PathUtil::Make(PathUtil::GetEnginePath(), "Tools/CrashHandler/CrashHandler.exe");

	if (!FileExists(crashHandlerPath.c_str()))
	{
		static tukk szErrorMessage = "Couldn't find the crash handler! (looking for Tools/CrashHandler/CrashHandler.exe)";
		DrxMessageBox(szErrorMessage, "Couldn't find the crash handler", eMB_Error);
		return false;
	}

	string commandLine;
	char workingDirectory[MAX_PATH];
	GetCurrentDirectory(sizeof(workingDirectory) - 1, workingDirectory);
	commandLine.Format("\"%s\" -buildFolder=\"%s\" -logFileName=\"%s\"",
	                   crashHandlerPath.c_str(), workingDirectory,
	                   gEnv->pSystem->GetILog()->GetFileName());

	// how to create a process: http://msdn.microsoft.com/en-us/library/ms682512(VS.85).aspx

	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInformation;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	ZeroMemory(&processInformation, sizeof(processInformation));

	i32k iBufferSize = commandLine.length() + 1;
	tuk args = new char[iBufferSize];
	drx_strcpy(args, iBufferSize, commandLine.c_str());

	//When there's a memory allocation crash this call might fail (processInformation would be left as zero)
	CreateProcess(crashHandlerPath.c_str(),
	              args, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInformation);

	// Wait until child process exits.
	WaitForSingleObject(processInformation.hProcess, INFINITE);
	DWORD exitCode = -1;
	GetExitCodeProcess(processInformation.hProcess, &exitCode);

	// Close process and thread handles.
	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);

	delete[] args;

	return exitCode == 0;

}

#else  // DRX_PLATFORM_WINDOWS && !defined(_RELEASE)

bool CJiraClient::ReportBug()
{
	return true;
}

#endif // DRX_PLATFORM_WINDOWS && !defined(_RELEASE)
