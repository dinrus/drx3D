// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/Sys/IMiniLog.h> // <> required for Interfuscator

// enable this define to support log scopes to provide more context information for log lines
// this code is disable by default due it's runtime cost
//#define SUPPORT_LOG_IDENTER

enum ELogMode
{
	eLogMode_Normal   = BIT(0),
	eLogMode_AppCrash = BIT(1), //!< Log straight to file. Do not write to console.
};

// forward declarations
class IDrxSizer;

//! \cond INTERNAL
//! Callback interface to the ILog.
struct ILogCallback
{
	// <interfuscator:shuffle>
	virtual ~ILogCallback(){}
	virtual void OnWriteToConsole(tukk sText, bool bNewLine) = 0;
	virtual void OnWriteToFile(tukk sText, bool bNewLine) = 0;
	// </interfuscator:shuffle>
};
//! \endcond

//! Interface for logging operations based on IMiniLog.
//! Logging in DinrusX should be done using the following global functions:
//! DrxLog (eMessage)
//! DrxLogAlways (eAlways)
//! DrxError (eError)
//! DrxWarning (eWarning)
//! DrxComment (eComment)
//! ILog gives you more control on logging operations.
struct ILog : public IMiniLog
{
	// <interfuscator:shuffle>
	virtual void Release() = 0;

	//! Set the file used to log to disk.
	virtual bool SetFileName(tukk command = NULL) = 0;

	//! Get the filename used to log to disk.
	virtual tukk GetFileName() const = 0;

	//! Get the filename where the current log backup was copied to on disk.
	virtual tukk GetBackupFileName() const = 0;

	//! All the following functions will be removed are here just to be able to compile the project.

	//! Logs the text both to file and console.
	virtual void Log(tukk szCommand, ...) PRINTF_PARAMS(2, 3) = 0;

	virtual void LogAlways(tukk szCommand, ...) PRINTF_PARAMS(2, 3) = 0;

	virtual void LogWarning(tukk szCommand, ...) PRINTF_PARAMS(2, 3) = 0;

	virtual void LogError(tukk szCommand, ...) PRINTF_PARAMS(2, 3) = 0;

	//! Logs the text both to the end of file and console.
	virtual void LogPlus(tukk command, ...) PRINTF_PARAMS(2, 3) = 0;

	//! Logs to the file specified in SetFileName.
	virtual void LogToFile(tukk command, ...) PRINTF_PARAMS(2, 3) = 0;

	virtual void LogToFilePlus(tukk command, ...) PRINTF_PARAMS(2, 3) = 0;

	//! Logs to console only.
	virtual void LogToConsole(tukk command, ...) PRINTF_PARAMS(2, 3) = 0;

	virtual void LogToConsolePlus(tukk command, ...) PRINTF_PARAMS(2, 3) = 0;

	virtual void UpdateLoadingScreen(tukk command, ...) PRINTF_PARAMS(2, 3) = 0;

	virtual void RegisterConsoleVariables()   {}

	virtual void UnregisterConsoleVariables() {}

	//! Set log verbosity.
	//! Full logging (to console and file) can be enabled with verbosity 4.
	//! In the console 'log_Verbosity 4' command can be used.
	virtual void     SetVerbosity(i32 verbosity) = 0;

	virtual i32      GetVerbosityLevel() const = 0;

	virtual void     AddCallback(ILogCallback* pCallback) = 0;
	virtual void     RemoveCallback(ILogCallback* pCallback) = 0;

	virtual void     SetLogMode(ELogMode eLogMode) = 0;
	virtual ELogMode GetLogMode() const = 0;

	//! Wait for all other threads to finish writing. Then only allow calling thread to write to log.
	virtual void ThreadExclusiveLogAccess(bool state) = 0;

	//! \note This is called on every frame by the system.
	virtual void        Update() = 0;

	virtual tukk GetModuleFilter() = 0;

	//! Collect memory statistics in DrxSizer.
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	//! Asset scope strings help to figure out asset dependencies in case of asset loading errors.
	//! Should not be used directly, only by using define DRX_DEFINE_ASSET_SCOPE.
	//! \see DRX_DEFINE_ASSET_SCOPE.
	virtual void        PushAssetScopeName(tukk sAssetType, tukk sName) {};
	virtual void        PopAssetScopeName()                                           {};
	virtual tukk GetAssetScopeString()                                         { return ""; };
	// </interfuscator:shuffle>

#if defined(SUPPORT_LOG_IDENTER)
	virtual void Indent(class CLogIndenter* indenter) = 0;
	virtual void Unindent(class CLogIndenter* indenter) = 0;
#endif

	virtual void Flush() = 0;
#if !defined(RESOURCE_COMPILER)
	virtual void FlushAndClose() = 0;
#endif
};

#if !defined(SUPPORT_LOG_IDENTER)
	#define INDENT_LOG_DURING_SCOPE(...)                   (void)(0)
	#define DRX_DEFINE_ASSET_SCOPE(sAssetType, sAssetName) (void)(0)
#else
class CLogIndenter
{
public:
	CLogIndenter(ILog* log) : m_log(log), m_enabled(false), m_nextIndenter(NULL), m_needToPrintSectionText(false)
	{
	}

	void Enable(bool enable = true, tukk sectionTextFormat = NULL, ...)
	{
		va_list args;
		va_start(args, sectionTextFormat);

		enable &= (m_log != NULL);

		if (enable != m_enabled)
		{
			if (sectionTextFormat && enable)
			{
				char buffer[1024];

				drx_vsprintf(buffer, sectionTextFormat, args);

				m_sectionText = buffer;
				m_needToPrintSectionText = true;
			}
			else
			{
				m_sectionText = "";
				m_needToPrintSectionText = m_nextIndenter ? m_nextIndenter->m_needToPrintSectionText : false;
			}

			assert(m_log);

			if (enable)
			{
				m_log->Indent(this);
			}
			else
			{
				m_log->Unindent(this);
			}
			m_enabled = enable;
		}

		va_end(args);
	}

	CLogIndenter* GetNextIndenter()
	{
		return m_nextIndenter;
	}

	void SetNextIndenter(CLogIndenter* indenter)
	{
		m_nextIndenter = indenter;
	}

	void DisplaySectionText()
	{
		if (m_needToPrintSectionText)
		{
			m_needToPrintSectionText = false;
			string sectionText = m_sectionText;
			Enable(false);

			if (m_nextIndenter)
			{
				m_nextIndenter->DisplaySectionText();
			}

			if (!sectionText.empty())
			{
				assert(m_log);
				m_log->Log("%s", sectionText.c_str());
			}
			Enable(true);
		}
	}

	~CLogIndenter()
	{
		Enable(false);
	}

private:
	bool          m_enabled;
	bool          m_needToPrintSectionText;
	ILog*         m_log;
	CLogIndenter* m_nextIndenter;
	string        m_sectionText;
};

class CLogAssetScopeName
{
	ILog* m_pLog;
public:
	CLogAssetScopeName(ILog* pLog, tukk sAssetType, tukk sAssetName) : m_pLog(pLog) { pLog->PushAssetScopeName(sAssetType, sAssetName); }
	~CLogAssetScopeName() { m_pLog->PopAssetScopeName(); }
};

	#define ILOG_CONCAT_IMPL(x, y)                         x ## y
	#define ILOG_CONCAT_MACRO(x, y)                        ILOG_CONCAT_IMPL(x, y)
	#define INDENT_LOG_DURING_SCOPE(...)                   CLogIndenter ILOG_CONCAT_MACRO(indentMe, __LINE__) ((DrxGetCurrentThreadId() == gEnv->mMainThreadId) ? gEnv->pLog : NULL); ILOG_CONCAT_MACRO(indentMe, __LINE__).Enable(__VA_ARGS__)

	#define DRX_DEFINE_ASSET_SCOPE(sAssetType, sAssetName) CLogAssetScopeName __asset_scope_name(gEnv->pLog, sAssetType, sAssetName);
#endif
