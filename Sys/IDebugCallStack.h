// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   Описание:
   A multiplatform base class for handling errors and collecting call stacks

   -------------------------------------------------------------------------
   История:
   - 12:10:2009	: Created by Alex McCarthy
*************************************************************************/

#ifndef __I_DEBUG_CALLSTACK_H__
#define __I_DEBUG_CALLSTACK_H__

#include <drx3D/CoreX/Platform/DrxWindows.h>

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE || DRX_PLATFORM_ORBIS
struct EXCEPTION_POINTERS;
#endif
//! Limits the maximal number of functions in call stack.
enum {MAX_DEBUG_STACK_ENTRIES = 80};

class IDebugCallStack
{
public:
	// Returns single instance of DebugStack
	static IDebugCallStack* instance();

	virtual i32             handleException(EXCEPTION_POINTERS* exception_pointer)                 { return 0; }
	virtual void            CollectCurrentCallStack(i32 maxStackEntries = MAX_DEBUG_STACK_ENTRIES) {}
	// Collects the low level callstack frames.
	// Returns number of collected stack frames.
	virtual i32 CollectCallStackFrames(uk * pCallstack, i32 maxStackEntries) { return 0; }

	// collects low level callstack for given thread handle
	virtual i32 CollectCallStack(HANDLE thread, uk * pCallstack, i32 maxStackEntries) { return 0; }

	// returns the module name of a given address
	virtual string GetModuleNameForAddr(uk addr) { return "[unknown]"; }

	// returns the function name of a given address together with source file and line number (if available) of a given address
	virtual bool GetProcNameForAddr(uk addr, string& procName, uk & baseAddr, string& filename, i32& line)
	{
		filename = "[unknown]";
		line = 0;
		baseAddr = addr;
#if DRX_PLATFORM_64BIT
		procName.Format("[%016llX]", (uint64) addr);
#else
		procName.Format("[%08X]", (u32) addr);
#endif
		return false;
	}

	// returns current filename
	virtual string GetCurrentFilename() { return "[unknown]"; }

	// initialize symbols
	virtual void InitSymbols() {}
	virtual void DoneSymbols() {}

	//! Dumps Current Call Stack to log.
	virtual void LogCallstack();
	//triggers a fatal error, so the DebugCallstack can create the error.log and terminate the application
	virtual void FatalError(tukk);

	//Reports a bug and continues execution
	virtual void ReportBug(tukk) {}

	virtual void FileCreationCallback(void (* postBackupProcess)());

	//! Get current call stack information.
	void getCallStack(std::vector<string>& functions) { functions = m_functions; }
	
	
	static void        Screenshot(tukk szFileName);

protected:
	IDebugCallStack();
	virtual ~IDebugCallStack() {}

	static tukk TranslateExceptionCode(DWORD dwExcept);
	static void        PutVersion(tuk str);

	static void        WriteLineToLog(tukk format, ...);

	bool                     m_bIsFatalError;
	static tukk const s_szFatalErrorCode;

	std::vector<string>      m_functions;
	void                     (* m_postBackupProcess)();
};

#endif // __I_DEBUG_CALLSTACK_H__
