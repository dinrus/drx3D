// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SandboxAPI.h"
#include <drx3D/Sys/ILog.h>
#include <drx3D/Sys/IConsole.h>

//struct IConsole;
//struct ICVar;

//////////////////////////////////////////////////////////////////////////
// Global log functions.
//////////////////////////////////////////////////////////////////////////
//! Displays error message.
extern void Error(tukk format, ...);
//! Log to console and file.
extern void Log(tukk format, ...);
//! Display Warning dialog.
extern void Warning(tukk format, ...);

/*!
 *	CLogFile implements ILog interface.
 */
class SANDBOX_API CLogFile : public ILogCallback
{
public:
	static tukk GetLogFileName();
	static void        AttachListBox(HWND hWndListBox) { m_hWndListBox = hWndListBox; };
	static void        AttachEditBox(HWND hWndEditBox) { m_hWndEditBox = hWndEditBox; };

	//! Write to log spanpshot of current process memory usage.
	static string GetMemUsage();

	//DEPRECATED, use DrxLog directly, do not use this file outside of Sandbox project
	static void    WriteString(tukk pszString);
	static void    WriteLine(tukk pszLine);
	static void    FormatLine(PSTR pszMessage, ...);

	//////////////////////////////////////////////////////////////////////////
	// ILogCallback
	//////////////////////////////////////////////////////////////////////////
	virtual void OnWriteToConsole(tukk sText, bool bNewLine);
	virtual void OnWriteToFile(tukk sText, bool bNewLine);
	//////////////////////////////////////////////////////////////////////////

	// logs some useful information
	// should be called after DrxLog() is available
	static void AboutSystem();

private:
	static void OpenFile();

	// Attached control(s)
	static HWND m_hWndListBox;
	static HWND m_hWndEditBox;
	static bool m_bShowMemUsage;
};


