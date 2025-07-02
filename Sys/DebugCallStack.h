// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DebugCallStack.h
//  Version:     v1.00
//  Created:     3/12/2001 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual C++ 6.0
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DebugCallStack_h__
#define __DebugCallStack_h__

#include <drx3D/Sys/IDebugCallStack.h>

#if DRX_PLATFORM_WINDOWS

//! Limits the maximal number of functions in call stack.
i32k MAX_DEBUG_STACK_ENTRIES_FILE_DUMP = 12;

struct ISystem;

//!============================================================================
//!
//! DebugCallStack class, capture call stack information from symbol files.
//!
//!============================================================================
class DebugCallStack : public IDebugCallStack
{
	friend class CCaptureCrashScreenShot;
public:
	DebugCallStack();
	virtual ~DebugCallStack();

	ISystem*       GetSystem() { return m_pSystem; };
	//! Dumps Current Call Stack to log.
	void           LogMemCallstackFile(i32 memSize);
	void           SetMemLogFile(bool open, tukk filename);

	virtual void   CollectCurrentCallStack(i32 maxStackEntries = MAX_DEBUG_STACK_ENTRIES);
	virtual i32    CollectCallStackFrames(uk * pCallstack, i32 maxStackEntries);
	virtual i32    CollectCallStack(HANDLE thread, uk * pCallstack, i32 maxStackEntries);
	virtual string GetModuleNameForAddr(uk addr);
	virtual bool   GetProcNameForAddr(uk addr, string& procName, uk & baseAddr, string& filename, i32& line);
	virtual string GetCurrentFilename();
	virtual void   InitSymbols() { initSymbols(); }
	virtual void   DoneSymbols() { doneSymbols(); }
	virtual void   FatalError(tukk message);

	void           installErrorHandler(ISystem* pSystem);
	void           uninstallErrorHandler();
	virtual i32    handleException(EXCEPTION_POINTERS* exception_pointer);

	virtual void   ReportBug(tukk);

	void           dumpCallStack(std::vector<string>& functions);

	void           SetUserDialogEnable(const bool bUserDialogEnable);

	void           PrintThreadCallstack(const threadID nThreadId, FILE* f);

	// Simulates generation of the crash report.
	void           GenerateCrashReport();

	// Creates a minimal necessary crash reporting, without UI.
	void           MinimalExceptionReport(EXCEPTION_POINTERS* exception_pointer);

	// Register CVars needed for debug call stack.
	void           RegisterCVars();

	typedef std::map<uk , string> TModules;
protected:
	bool                    initSymbols();
	void                    doneSymbols();

	static void             RemoveOldFiles();
	static void             MoveFile(tukk szFileNameOld, tukk szFileNameNew);
	static void             RemoveFile(tukk szFileName);

	NO_INLINE void          FillStackTrace(i32 maxStackEntries = MAX_DEBUG_STACK_ENTRIES, i32 skipNumFunctions = 0, HANDLE hThread = GetCurrentThread());

	static i32              PrintException(EXCEPTION_POINTERS* exception_pointer);
	static INT_PTR CALLBACK ExceptionDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

	i32                     updateCallStack(EXCEPTION_POINTERS* exception_pointer);
	void                    LogExceptionInfo(EXCEPTION_POINTERS* exception_pointer);

	void                    WriteErrorLog( tukk filename,tukk writeString );
	void                    CaptureScreenshot();

	bool                    BackupCurrentLevel();
	i32                     SubmitBug(EXCEPTION_POINTERS* exception_pointer);
	void                    ResetFPU(EXCEPTION_POINTERS* pex);

	static string           LookupFunctionName(uk address, bool fileInfo);
	static bool             LookupFunctionName(uk address, bool fileInfo, string& proc, string& file, i32& line, uk & baseAddr);

	static i32k s_iCallStackSize = 32768;

	char             m_excLine[256];
	char             m_excModule[128];

	char             m_excDesc[MAX_WARNING_LENGTH];
	char             m_excCode[MAX_WARNING_LENGTH];
	char             m_excAddr[80];
	char             m_excCallstack[s_iCallStackSize];

	bool             m_symbols;
	bool             m_bCrash;
	tukk      m_szBugMessage;

	ISystem*         m_pSystem;

	CONTEXT          m_context;

	TModules         m_modules;

	LPTOP_LEVEL_EXCEPTION_FILTER m_previousHandler;

	string           m_outputPath;
};

#ifdef DRX_USE_CRASHRPT

#include "CrashRpt.h"
class CCrashRpt
{
public:
	static void RegisterCVars();

	static bool InstallHandler();

	static void UninstallHandler();

	static i32 CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo);

	static void CmdGenerateCrashReport(IConsoleCmdArgs*);

	static void FatalError();

	static void ReInstallCrashRptHandler(ICVar*);

	static SFileVersion GetSystemVersionInfo();

};
#endif // DRX_USE_CRASHRPT

#endif // DRX_PLATFORM_WINDOWS

#endif // __DebugCallStack_h__
