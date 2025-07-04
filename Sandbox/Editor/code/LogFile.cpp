// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "LogFile.h"
#include "DrxEdit.h"

#include "ProcessInfo.h"

#include <Preferences/GeneralPreferences.h>

#define EDITOR_LOG_FILE "Editor.log"

// Static member variables
HWND CLogFile::m_hWndListBox = 0;
HWND CLogFile::m_hWndEditBox = 0;
bool CLogFile::m_bShowMemUsage = false;

#define MAX_LOGBUFFER_SIZE 16384

//////////////////////////////////////////////////////////////////////////
void Error(tukk format, ...)
{
	va_list args;
	char szBuffer[MAX_LOGBUFFER_SIZE];

	va_start(args, format);
	drx_vsprintf(szBuffer, format, args);
	va_end(args);

	string str = "####-ERROR-####: ";
	str += szBuffer;

	//CLogFile::WriteLine( str );
	DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, str);

	if (!((CDrxEditApp*)AfxGetApp())->IsInTestMode() && !((CDrxEditApp*)AfxGetApp())->IsInExportMode() && !((CDrxEditApp*)AfxGetApp())->IsInLevelLoadTestMode())
	{
		CQuestionDialog::SCritical("Error", szBuffer);
	}
}

//////////////////////////////////////////////////////////////////////////
void Warning(tukk format, ...)
{
	va_list args;
	char szBuffer[MAX_LOGBUFFER_SIZE];

	va_start(args, format);
	drx_vsprintf(szBuffer, format, args);
	va_end(args);

	DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, szBuffer);

	bool bNoUI = false;
	ICVar* pCVar = gEnv->pConsole->GetCVar("sys_no_crash_dialog");
	if (pCVar->GetIVal() != 0)
	{
		bNoUI = true;
	}

	if (!((CDrxEditApp*)AfxGetApp())->IsInTestMode() && !((CDrxEditApp*)AfxGetApp())->IsInExportMode() && !bNoUI)
	{
		CQuestionDialog::SWarning("Warning", szBuffer);
	}
}

//////////////////////////////////////////////////////////////////////////
void Log(tukk format, ...)
{
	va_list args;
	char szBuffer[MAX_LOGBUFFER_SIZE];

	va_start(args, format);
	drx_vsprintf(szBuffer, format, args);
	va_end(args);

	CLogFile::WriteLine(szBuffer);
}

//////////////////////////////////////////////////////////////////////////
tukk CLogFile::GetLogFileName()
{
	// Return the path
	return gEnv->pLog->GetFileName();
}

void CLogFile::FormatLine(PSTR format, ...)
{
	va_list args;
	char szBuffer[MAX_LOGBUFFER_SIZE];

	va_start(args, format);
	drx_vsprintf(szBuffer, format, args);
	va_end(args);

	DrxLog(szBuffer);
}

void CLogFile::AboutSystem()
{
	//////////////////////////////////////////////////////////////////////
	// Write the system informations to the log
	//////////////////////////////////////////////////////////////////////

	char szBuffer[MAX_LOGBUFFER_SIZE];
	char szProfileBuffer[128];
	char szLanguageBuffer[64];
	//char szCPUModel[64];
	tuk pChar = 0;
	MEMORYSTATUS MemoryStatus;
	DEVMODE DisplayConfig;
	RTL_OSVERSIONINFOEXW OSVerInfo = { 0 };
	OSVerInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

	//////////////////////////////////////////////////////////////////////
	// Display editor and Windows version
	//////////////////////////////////////////////////////////////////////

	// Get system language
	GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SENGLANGUAGE,
	              szLanguageBuffer, sizeof(szLanguageBuffer));

	// Format and send OS information line
	drx_sprintf(szBuffer, "Current Language: %s ", szLanguageBuffer);
	DrxLog(szBuffer);

	// Format and send OS version line
	string str = "Windows ";
	//RtlGetVersion is required to ensure correct information without manifest on Windows 8.1 and later
	typedef LONG (WINAPI * fnRtlGetVersion)(PRTL_OSVERSIONINFOEXW lpVersionInformation);
	static auto RtlGetVersion = (fnRtlGetVersion)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlGetVersion");
	if (RtlGetVersion && RtlGetVersion(&OSVerInfo) == 0)
	{
		if (OSVerInfo.dwMajorVersion == 4)
		{
			if (OSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			{
				if (OSVerInfo.dwMinorVersion > 0)
					// Windows 98
					str += "98";
				else
					// Windows 95
					str += "95";
			}
			else
				// Windows NT
				str += "NT";
		}
		else if (OSVerInfo.dwMajorVersion == 5)
		{
			if (OSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
				// Windows Millennium
				str += "ME";
			else
			{
				if (OSVerInfo.dwMinorVersion > 0)
					// Windows XP
					str += "XP";
				else
					// Windows 2000
					str += "2000";
			}
		}
		else if (OSVerInfo.dwMajorVersion == 6)
		{
			if (OSVerInfo.dwMinorVersion == 0)
			{
				// Windows Vista
				str += "Vista";
			}
			else if (OSVerInfo.dwMinorVersion == 1)
			{
				// Windows 7
				str += "7";
			}
			else if (OSVerInfo.dwMinorVersion == 2)
			{
				// Windows 8
				str += "8";
			}
			else if (OSVerInfo.dwMinorVersion == 3)
			{
				// Windows 8.1
				str += "8.1";
			}
		}
		else if (OSVerInfo.dwMajorVersion == 10)
		{
			if (OSVerInfo.dwMinorVersion == 0)
			{
				//Windows 10
				str += "10";
			}
			else
			{
				// Windows unknown (newer than Win10)
				str += "Version Unknown";
			}
		}
		else
		{
			// Windows unknown (newer than Win10)
			str += "Version Unknown";
		}
		drx_sprintf(szBuffer, " %d.%d", OSVerInfo.dwMajorVersion, OSVerInfo.dwMinorVersion);
		str += szBuffer;
	}
	else
	{
		//RtlGetVersion failed
		str += "Version Unknown";
	}

	//////////////////////////////////////////////////////////////////////
	// Show Windows directory
	//////////////////////////////////////////////////////////////////////

	str += " (";
	GetWindowsDirectory(szBuffer, sizeof(szBuffer));
	str += szBuffer;
	str += ")";
	DrxLog(str);

	//////////////////////////////////////////////////////////////////////
	// Send system time & date
	//////////////////////////////////////////////////////////////////////

	str = "Local time is ";
	_strtime(szBuffer);
	str += szBuffer;
	str += " ";
	_strdate(szBuffer);
	str += szBuffer;
	drx_sprintf(szBuffer, ", system running for %d minutes", GetTickCount() / 60000);
	str += szBuffer;
	DrxLog(str);

	//////////////////////////////////////////////////////////////////////
	// Send system memory status
	//////////////////////////////////////////////////////////////////////

	GlobalMemoryStatus(&MemoryStatus);
	drx_sprintf(szBuffer, "%" PRISIZE_T "MB phys. memory installed, %" PRISIZE_T "MB paging available",
	            MemoryStatus.dwTotalPhys / 1048576 + 1,
	            MemoryStatus.dwAvailPageFile / 1048576);
	DrxLog(szBuffer);

	//////////////////////////////////////////////////////////////////////
	// Send display settings
	//////////////////////////////////////////////////////////////////////

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DisplayConfig);
	GetPrivateProfileString("boot.description", "display.drv",
	                        "(Unknown graphics card)", szProfileBuffer, sizeof(szProfileBuffer),
	                        "system.ini");
	drx_sprintf(szBuffer, "Current display mode is %dx%dx%d, %s",
	            DisplayConfig.dmPelsWidth, DisplayConfig.dmPelsHeight,
	            DisplayConfig.dmBitsPerPel, szProfileBuffer);
	DrxLog(szBuffer);

	//////////////////////////////////////////////////////////////////////
	// Send input device configuration
	//////////////////////////////////////////////////////////////////////

	str = "";
	// Detect the keyboard type
	switch (GetKeyboardType(0))
	{
	case 1:
		str = "IBM PC/XT (83-key)";
		break;
	case 2:
		str = "ICO (102-key)";
		break;
	case 3:
		str = "IBM PC/AT (84-key)";
		break;
	case 4:
		str = "IBM enhanced (101/102-key)";
		break;
	case 5:
		str = "Nokia 1050";
		break;
	case 6:
		str = "Nokia 9140";
		break;
	case 7:
		str = "Japanese";
		break;
	default:
		str = "Unknown";
		break;
	}

	// Any mouse attached ?
	if (!GetSystemMetrics(SM_MOUSEPRESENT))
		DrxLog(str + " keyboard and no mouse installed");
	else
	{
		drx_sprintf(szBuffer, " keyboard and %i+ button mouse installed",
		            GetSystemMetrics(SM_CMOUSEBUTTONS));
		DrxLog(str + szBuffer);
	}

	DrxLog("--------------------------------------------------------------------------------");
}

//////////////////////////////////////////////////////////////////////////
string CLogFile::GetMemUsage()
{
	ProcessMemInfo mi;
	CProcessInfo::QueryMemInfo(mi);
	i32 MB = 1024 * 1024;

	string str;
	str.Format("Memory=%dMb, Pagefile=%dMb", mi.WorkingSet / MB, mi.PagefileUsage / MB);
	//FormatLine( "PeakWorkingSet=%dMb, PeakPagefileUsage=%dMb",pc.PeakWorkingSetSize/MB,pc.PeakPagefileUsage/MB );
	//FormatLine( "PagedPoolUsage=%d",pc.QuotaPagedPoolUsage/MB );
	//FormatLine( "NonPagedPoolUsage=%d",pc.QuotaNonPagedPoolUsage/MB );

	return str;
}

//////////////////////////////////////////////////////////////////////////
void CLogFile::WriteLine(tukk pszString)
{
	DrxLog(pszString);
}

//////////////////////////////////////////////////////////////////////////
void CLogFile::WriteString(tukk pszString)
{
	gEnv->pLog->LogPlus(pszString);
}

static inline string CopyAndRemoveColorCode(tukk sText)
{
	CString ret = sText;    // alloc and copy

	tuk s, * d;

	s = ret.GetBuffer();
	d = s;

	// remove color code in place
	while (*s != 0)
	{
		if (*s == '$' && *(s + 1) >= '0' && *(s + 1) <= '9')
		{
			s += 2;
			continue;
		}

		*d++ = *s++;
	}

	ret.ReleaseBuffer(d - ret.GetBuffer());

	return ret.GetString();
}

//////////////////////////////////////////////////////////////////////////
void CLogFile::OnWriteToConsole(tukk sText, bool bNewLine)
{
	if (!gEnv)
		return;
	//	if (GetIEditorImpl()->IsInGameMode())
	//	return;

	// Skip any non character.
	if (*sText != 0 && ((u8) * sText) < 32)
		sText++;

	// If we have a listbox attached, also output the string to this listbox
	if (m_hWndListBox)
	{
		string str = CopyAndRemoveColorCode(sText);    // editor prinout doesn't support color coded log messages

		if (m_bShowMemUsage)
		{
			str = string("(") + GetMemUsage() + ")" + str;
		}

		// Add the string to the listbox
		SendMessage(m_hWndListBox, LB_ADDSTRING, 0, (LPARAM) (tukk)str);

		// Make sure the recently added string is visible
		SendMessage(m_hWndListBox, LB_SETTOPINDEX,
		            SendMessage(m_hWndListBox, LB_GETCOUNT, 0, 0) - 1, 0);

		static i32 nCounter = 0;
		if (nCounter++ > 500)
		{
			// Clean Edit box every 500 lines.
			nCounter = 0;
			::SetWindowText(m_hWndEditBox, "");
		}

		// remember selection and the top row
		i32 len = ::GetWindowTextLength(m_hWndEditBox);
		i32 top, from, to;
		SendMessage(m_hWndEditBox, EM_GETSEL, (WPARAM)&from, (LPARAM)&to);
		bool keepPos = false;
		bool locking = false;

		if (from != len || to != len)
		{
			keepPos = GetFocus() == m_hWndEditBox;
			if (keepPos)
			{
				top = SendMessage(m_hWndEditBox, EM_GETFIRSTVISIBLELINE, 0, 0);
				locking = bNewLine && ::IsWindowVisible(m_hWndEditBox);
				if (locking)
				{
					// ::LockWindowUpdate( m_hWndEditBox );
					//   ^ this function should not be used here!
					//
					// http://msdn.microsoft.com/en-us/library/dd145034(VS.85).aspx
					// http://blogs.msdn.com/b/oldnewthing/archive/2007/02/21/1735472.aspx
					//
					// Better way to prevent window redraw
					SendMessage(m_hWndEditBox, WM_SETREDRAW, FALSE, 0);
				}
			}
			SendMessage(m_hWndEditBox, EM_SETSEL, len, len);
		}
		if (bNewLine)
		{
			//str = string("\r\n") + str.TrimLeft();
			//str = string("\r\n") + str;
			//str = string("\r") + str;
			str.TrimRight();
			str = string("\r\n") + str;
		}
		tukk szStr = str;
		SendMessage(m_hWndEditBox, EM_REPLACESEL, FALSE, (LPARAM)szStr);

		// restore selection and the top line
		if (keepPos)
		{
			SendMessage(m_hWndEditBox, EM_SETSEL, from, to);
			top -= SendMessage(m_hWndEditBox, EM_GETFIRSTVISIBLELINE, 0, 0);
			SendMessage(m_hWndEditBox, EM_LINESCROLL, 0, (LPARAM)top);
		}

		if (locking)
		{
			SendMessage(m_hWndEditBox, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(m_hWndEditBox, 0, 0, RDW_INVALIDATE);
		}
	}

	string sOutLine(sText);
	if (gEditorGeneralPreferences.showTimeInConsole())
	{
		char sTime[128];
		time_t ltime;
		time(&ltime);
		struct tm* today = localtime(&ltime);
		strftime(sTime, sizeof(sTime), "<%H:%M:%S> ", today);
		sOutLine = sTime;
		sOutLine += sText;
	}

	//////////////////////////////////////////////////////////////////////////
	// Look for exit messages while writing to the console
	//////////////////////////////////////////////////////////////////////////
	if (gEnv && gEnv->pSystem && !gEnv->pSystem->IsQuitting())
	{
		MSG msg;
		if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				AfxGetApp()->ExitInstance();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CLogFile::OnWriteToFile(tukk sText, bool bNewLine)
{
}

