// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/ILog.h>
#include <drx3D/CoreX/Thread/DrxAtomics.h>
#include <drx3D/CoreX/Thread/MultiThread_Containers.h>

//////////////////////////////////////////////////////////////////////

#define MAX_FILENAME_SIZE 256

#if defined(DEDICATED_SERVER) || defined (DRX_PLATFORM_WINDOWS)
	#define KEEP_LOG_FILE_OPEN 1
#else
	#define KEEP_LOG_FILE_OPEN 0
#endif

struct SExclusiveThreadAccessLock
{
	SExclusiveThreadAccessLock() : counter(0), writerThreadId(0) {}
	 u32   counter;
	 threadID writerThreadId;
};

//////////////////////////////////////////////////////////////////////
class CLog : public ILog
{
public:
	typedef std::list<ILogCallback*>   Callbacks;
	typedef DrxStackStringT<char, 256> LogStringType;

	// constructor
	CLog(ISystem* pSystem);
	// destructor
	~CLog();

	// interface ILog, IMiniLog -------------------------------------------------

	virtual void        Release() { delete this; };
	virtual bool        SetFileName(tukk filename);
	virtual tukk GetFileName() const;
	virtual tukk GetBackupFileName() const;
#if !defined(EXCLUDE_NORMAL_LOG)
	virtual void        Log(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual void        LogAlways(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual void        LogWarning(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual void        LogError(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual void        LogPlus(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual void        LogToFile(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual void        LogToFilePlus(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual void        LogToConsole(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual void        LogToConsolePlus(tukk command, ...) PRINTF_PARAMS(2, 3);
#else
	virtual void        Log(tukk command, ...) PRINTF_PARAMS(2, 3)              {};
	virtual void        LogAlways(tukk command, ...) PRINTF_PARAMS(2, 3)        {};
	virtual void        LogWarning(tukk command, ...) PRINTF_PARAMS(2, 3)       {};
	virtual void        LogError(tukk command, ...) PRINTF_PARAMS(2, 3)         {};
	virtual void        LogPlus(tukk command, ...) PRINTF_PARAMS(2, 3)          {};
	virtual void        LogToFile(tukk command, ...) PRINTF_PARAMS(2, 3)        {};
	virtual void        LogToFilePlus(tukk command, ...) PRINTF_PARAMS(2, 3)    {};
	virtual void        LogToConsole(tukk command, ...) PRINTF_PARAMS(2, 3)     {};
	virtual void        LogToConsolePlus(tukk command, ...) PRINTF_PARAMS(2, 3) {};
#endif // !defined(EXCLUDE_NORMAL_LOG)
	virtual void        UpdateLoadingScreen(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual void        SetVerbosity(i32 verbosity);
	virtual i32         GetVerbosityLevel() const;
	virtual void        RegisterConsoleVariables();
	virtual void        UnregisterConsoleVariables();
	virtual void        AddCallback(ILogCallback* pCallback);
	virtual void        RemoveCallback(ILogCallback* pCallback);
	virtual void        LogV(const ELogType ineType, i32 flags, tukk szFormat, va_list args);
	virtual void        LogV(const ELogType ineType, tukk szFormat, va_list args);
	virtual void        Update();
	virtual tukk GetModuleFilter();
	virtual void        Flush();
	virtual void        FlushAndClose();

	virtual void        SetLogMode(ELogMode eLogMode);
	virtual ELogMode    GetLogMode() const;

	virtual void        ThreadExclusiveLogAccess(bool state);

private: // -------------------------------------------------------------------

#if !defined(EXCLUDE_NORMAL_LOG)
	void LogStringToFile(tukk szString, bool bAdd, bool bError = false);
	void LogStringToConsole(tukk szString, bool bAdd = false);
#else
	void LogStringToFile(tukk szString, bool bAdd, bool bError = false) {};
	void LogStringToConsole(tukk szString, bool bAdd = false)           {};
#endif // !defined(EXCLUDE_NORMAL_LOG)

	FILE* OpenLogFile(tukk filename, tukk mode);
	void  CloseLogFile(bool force = false);

	// will format the message into m_szTemp
	void FormatMessage(tukk szCommand, ...) PRINTF_PARAMS(2, 3);

#if defined(SUPPORT_LOG_IDENTER)
	void                Indent(CLogIndenter* indenter);
	void                Unindent(CLogIndenter* indenter);
	void                BuildIndentString();
	virtual void        PushAssetScopeName(tukk sAssetType, tukk sName);
	virtual void        PopAssetScopeName();
	virtual tukk GetAssetScopeString();
#endif

	ISystem*                  m_pSystem;                  //
	float                     m_fLastLoadingUpdateTime;   // for non-frequent streamingEngine update
	//char						m_szTemp[MAX_TEMP_LENGTH_SIZE];				//
	char                      m_szFilename[MAX_FILENAME_SIZE];      // can be with path
	mutable char              m_sBackupFilename[MAX_FILENAME_SIZE]; // can be with path
	FILE*                     m_pLogFile;
	DrxStackStringT<char, 32> m_LogMode;                            //mode m_pLogFile has been opened with
	FILE*                     m_pErrFile;
	i32                       m_nErrCount;

#if defined(SUPPORT_LOG_IDENTER)
	u8               m_indentation;
	LogStringType       m_indentWithString;
	class CLogIndenter* m_topIndenter;

	struct SAssetScopeInfo
	{
		string sType;
		string sName;
	};

	std::vector<SAssetScopeInfo> m_assetScopeQueue;
	DrxCriticalSection           m_assetScopeQueueLock;
	string                       m_assetScopeString;
#endif

	ICVar*             m_pLogIncludeTime;                 //

	IConsole*          m_pConsole;                        //

	DrxCriticalSection m_logCriticalSection;

	struct SLogHistoryItem
	{
		LogStringType str;
		tukk   ptr;
		ELogType      type;
		float         time;

		SLogHistoryItem()
			: ptr(nullptr)
			, type(eMessage)
			, time(0.0f) {}
	};
	SLogHistoryItem m_history[16];
	i32             m_iLastHistoryItem;

#if KEEP_LOG_FILE_OPEN
	static void LogFlushFile(IConsoleCmdArgs* pArgs);

	bool m_bFirstLine;
	char m_logBuffer[0x200000];
#endif

public: // -------------------------------------------------------------------

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pLogVerbosity);
		pSizer->AddObject(m_pLogWriteToFile);
		pSizer->AddObject(m_pLogWriteToFileVerbosity);
		pSizer->AddObject(m_pLogVerbosityOverridesWriteToFile);
		pSizer->AddObject(m_pLogSpamDelay);
		pSizer->AddObject(m_threadSafeMsgQueue);
	}
	// checks the verbosity of the message and returns NULL if the message must NOT be
	// logged, or the pointer to the part of the message that should be logged
	tukk CheckAgainstVerbosity(tukk pText, bool& logtofile, bool& logtoconsole, u8k DefaultVerbosity = 2);

	// create backup of log file, useful behavior - only on development platform
	void CreateBackupFile() const;

	ICVar*    m_pLogVerbosity;                            //
	ICVar*    m_pLogWriteToFile;                          //
	ICVar*    m_pLogWriteToFileVerbosity;                 //
	ICVar*    m_pLogVerbosityOverridesWriteToFile;        //
	ICVar*    m_pLogSpamDelay;                            //
	ICVar*    m_pLogModule;                               // Module filter for log
	Callbacks m_callbacks;                                //

	threadID  m_nMainThreadId;

	struct SLogMsg
	{
		LogStringType msg;
		bool          bError;
		bool          bAdd;
		bool          bConsole;
		void          GetMemoryUsage(IDrxSizer* pSizer) const {}
	};
	DrxMT::queue<SLogMsg>              m_threadSafeMsgQueue;

	ELogMode                           m_eLogMode;
	mutable SExclusiveThreadAccessLock m_exclusiveLogFileThreadAccessLock;
};
