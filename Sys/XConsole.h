// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_XCONSOLE_H__BA902011_5C47_4954_8E09_68598456912D__INCLUDED_)
#define AFX_XCONSOLE_H__BA902011_5C47_4954_8E09_68598456912D__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/Sys/IConsole.h>
#include <drx3D/Input/IInput.h>
#include <drx3D/CoreX/DrxCrc32.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <drx3D/Sys/Timer.h>

//forward declaration
struct IIpnut;
struct INetwork;
class CSystem;

#define MAX_HISTORY_ENTRIES 50
#define LINE_BORDER         10

enum ScrollDir
{
	sdDOWN,
	sdUP,
	sdNONE
};

//////////////////////////////////////////////////////////////////////////
// Console command holds information about commands registered to console.
//////////////////////////////////////////////////////////////////////////
struct CConsoleCommand
{
	string             m_sName;    // Console command name
	string             m_sCommand; // lua code that is executed when this command is invoked
	string             m_sHelp;    // optional help string - can be shown in the console with "<commandname> ?"
	i32                m_nFlags;   // bitmask consist of flag starting with VF_ e.g. VF_CHEAT
	ConsoleCommandFunc m_func;     // Pointer to console command.
	bool               m_isManagedExternally;// true if console command is added from C# and the notification of console commands will be through C# class method invocation via mono

	//////////////////////////////////////////////////////////////////////////
	CConsoleCommand()
		: m_func(0)
		, m_nFlags(0)
		, m_isManagedExternally(false)
	{}
	size_t sizeofThis() const { return sizeof(*this) + m_sName.capacity() + 1 + m_sCommand.capacity() + 1; }
	void   GetMemoryUsage(class IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_sName);
		pSizer->AddObject(m_sCommand);
		pSizer->AddObject(m_sHelp);
	}
};

//////////////////////////////////////////////////////////////////////////
// Implements IConsoleCmdArgs.
//////////////////////////////////////////////////////////////////////////
struct CConsoleCommandArgs : public IConsoleCmdArgs
{
	CConsoleCommandArgs(string& line, std::vector<string>& args) : m_line(line), m_args(args) {};
	virtual i32         GetArgCount() const { return m_args.size(); };
	// Get argument by index, nIndex must be in 0 <= nIndex < GetArgCount()
	virtual tukk GetArg(i32 nIndex) const
	{
		assert(nIndex >= 0 && nIndex < GetArgCount());
		if (!(nIndex >= 0 && nIndex < GetArgCount()))
			return NULL;
		return m_args[nIndex].c_str();
	}
	virtual tukk GetCommandLine() const
	{
		return m_line.c_str();
	}

private:
	std::vector<string>& m_args;
	string&              m_line;
};

struct string_nocase_lt
{
	bool operator()(tukk s1, tukk s2) const
	{
		return stricmp(s1, s2) < 0;
	}
};

/* - very dangerous to use with STL containers
   struct string_nocase_lt
   {
   bool operator()( tukk s1,tukk s2 ) const
   {
    return stricmp(s1,s2) < 0;
   }
   bool operator()( const string &s1,const string &s2 ) const
   {
    return stricmp(s1.c_str(),s2.c_str()) < 0;
   }
   };
 */

//forward declarations
class ITexture;
struct IRenderer;

/*! engine console implementation
   @see IConsole
 */
class CXConsole : public IConsole, public IInputEventListener, public IRemoteConsoleListener
{
public:
	typedef std::deque<string>              ConsoleBuffer;
	typedef ConsoleBuffer::iterator         ConsoleBufferItor;
	typedef ConsoleBuffer::reverse_iterator ConsoleBufferRItor;

	// constructor
	CXConsole();
	// destructor
	virtual ~CXConsole();

	//
	void Init(CSystem* pSystem);
	//
	void SetStatus(bool bActive) { m_bConsoleActive = bActive; }
	bool GetStatus() const       { return m_bConsoleActive; }
	//
	void FreeRenderResources();
	//
	void Copy();
	//
	void Paste();

	// interface IConsole ---------------------------------------------------------

	virtual void                   Release();

	virtual void                   UnregisterVariable(tukk sVarName, bool bDelete = false);
	virtual void                   SetScrollMax(i32 value);
	virtual void                   AddOutputPrintSink(IOutputPrintSink* inpSink);
	virtual void                   RemoveOutputPrintSink(IOutputPrintSink* inpSink);
	virtual void                   ShowConsole(bool show, i32k iRequestScrollMax = -1);
	virtual void                   DumpCVars(ICVarDumpSink* pCallback, u32 nFlagsFilter = 0);
	virtual void                   DumpKeyBinds(IKeyBindDumpSink* pCallback);
	virtual void                   CreateKeyBind(tukk sCmd, tukk sRes);
	virtual tukk            FindKeyBind(tukk sCmd) const;
	virtual void                   SetImage(ITexture* pImage, bool bDeleteCurrent);
	virtual inline ITexture*       GetImage()                     { return m_pImage; }
	virtual void                   StaticBackground(bool bStatic) { m_bStaticBackground = bStatic; }
	virtual bool                   GetLineNo(i32k indwLineNo, tuk outszBuffer, i32k indwBufferSize) const;
	virtual i32                    GetLineCount() const;
	virtual ICVar*                 GetCVar(tukk name);
	virtual tuk                  GetVariable(tukk szVarName, tukk szFileName, tukk def_val);
	virtual float                  GetVariable(tukk szVarName, tukk szFileName, float def_val);
	virtual void                   PrintLine(tukk s);
	virtual void                   PrintLinePlus(tukk s);
	virtual bool                   GetStatus();
	virtual void                   Clear();
	virtual void                   Update();
	virtual void                   Draw();
	virtual void                   RegisterListener(IManagedConsoleCommandListener* pListener, tukk name);
	virtual void                   UnregisterListener(IManagedConsoleCommandListener* pListener);
	virtual void                   RemoveCommand(tukk sName);
	virtual void                   ExecuteString(tukk command, const bool bSilentMode, const bool bDeferExecution = false);
	virtual void                   Exit(tukk command, ...) PRINTF_PARAMS(2, 3);
	virtual bool                   IsOpened();
	virtual size_t                 GetNumVars(bool bIncludeCommands = false) const;
	virtual size_t                 GetSortedVars(tukk* pszArray, size_t numItems, tukk szPrefix = 0, i32 nListTypes = 0) const;
	virtual i32                    GetNumCheatVars();
	virtual void                   SetCheatVarHashRange(size_t firstVar, size_t lastVar);
	virtual void                   CalcCheatVarHash();
	virtual bool                   IsHashCalculated();
	virtual uint64                 GetCheatVarHash();
	virtual void                   FindVar(tukk substr);
	virtual tukk            AutoComplete(tukk substr);
	virtual tukk            AutoCompletePrev(tukk substr);
	virtual tukk            ProcessCompletion(tukk szInputBuffer);
	virtual void                   RegisterAutoComplete(tukk sVarOrCommand, IConsoleArgumentAutoComplete* pArgAutoComplete);
	virtual void                   UnRegisterAutoComplete(tukk sVarOrCommand);
	virtual void                   ResetAutoCompletion();
	virtual void                   GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual void                   ResetProgressBar(i32 nProgressRange);
	virtual void                   TickProgressBar();
	virtual void                   SetLoadingImage(tukk szFilename);
	virtual void                   AddConsoleVarSink(IConsoleVarSink* pSink);
	virtual void                   RemoveConsoleVarSink(IConsoleVarSink* pSink);
	virtual tukk            GetHistoryElement(const bool bUpOrDown);
	virtual void                   AddCommandToHistory(tukk szCommand);
	virtual void                   SetInputLine(tukk szLine);
	virtual void                   LoadConfigVar(tukk sVariable, tukk sValue);
	virtual void                   LoadConfigCommand(tukk szCommand, tukk szArguments = nullptr);
	virtual ELoadConfigurationType SetCurrentConfigType(ELoadConfigurationType configType);
	virtual void                   EnableActivationKey(bool bEnable);
#if defined(DEDICATED_SERVER)
	virtual void                   SetClientDataProbeString(tukk pName, tukk pValue);
#endif
	virtual void                   SaveInternalState(struct IDataWriteStream& writer) const;
	virtual void                   LoadInternalState(struct IDataReadStream& reader);

	// interface IInputEventListener ------------------------------------------------------------------

	virtual bool OnInputEvent(const SInputEvent& event);
	virtual bool OnInputEventUI(const SUnicodeEvent& event);

	virtual void SetReadOnly(bool readonly) { m_readOnly = readonly; }
	virtual bool IsReadOnly()               { return m_readOnly; }

	// interface IRemoteConsoleListener ------------------------------------------------------------------

	virtual void OnConsoleCommand(tukk cmd);

	// interface IConsoleVarSink ----------------------------------------------------------------------

	virtual bool OnBeforeVarChange(ICVar* pVar, tukk sNewValue);
	virtual void OnAfterVarChange(ICVar* pVar);

	//////////////////////////////////////////////////////////////////////////

	// Returns
	//   0 if the operation failed
	ICVar*        RegisterCVarGroup(tukk sName, tukk szFileName);

	virtual void  PrintCheatVars(bool bUseLastHashRange);
	virtual tuk GetCheatVarAt(u32 nOffset);

	void          SetProcessingGroup(bool isGroup) { m_bIsProcessingGroup = isGroup; }
	bool          GetIsProcessingGroup(void) const { return m_bIsProcessingGroup; }

protected: // ----------------------------------------------------------------------------------------
	void DrawBuffer(i32 nScrollPos, tukk szEffect);

	void RegisterVar(ICVar* pCVar, ConsoleVarFunc pChangeFunc = 0);

	bool ProcessInput(const SInputEvent& event);
	void AddLine(tukk inputStr);
	void AddLinePlus(tukk inputStr);
	void AddInputChar(u32k c);
	void RemoveInputChar(bool bBackSpace);
	void ExecuteInputBuffer();
	void ExecuteCommand(CConsoleCommand& cmd, string& params, bool bIgnoreDevMode = false);

	void ScrollConsole();

#if ALLOW_AUDIT_CVARS
	void AuditCVars(IConsoleCmdArgs* pArg);
#endif // ALLOW_AUDIT_CVARS

#ifndef _RELEASE
	// will be removed once the HTML version is good enough
	void DumpCommandsVarsTxt(tukk prefix);
	void DumpVarsTxt(const bool includeCheat);
#endif

	void ConsoleLogInputResponse(tukk szFormat, ...) PRINTF_PARAMS(2, 3);
	void ConsoleLogInput(tukk szFormat, ...) PRINTF_PARAMS(2, 3);
	void ConsoleWarning(tukk szFormat, ...) PRINTF_PARAMS(2, 3);

	void DisplayHelp(tukk help, tukk name);
	void DisplayVarValue(ICVar* pVar);

	// Arguments:
	//   bFromConsole - true=from console, false=from outside
	void               SplitCommands(tukk line, std::list<string>& split);
	void               ExecuteStringInternal(tukk command, const bool bFromConsole, const bool bSilentMode = false);
	void               ExecuteDeferredCommands();

	static tukk GetFlagsString(u32k dwFlags);

	static void        CmdDumpAllAnticheatVars(IConsoleCmdArgs* pArgs);
	static void        CmdDumpLastHashedAnticheatVars(IConsoleCmdArgs* pArgs);

private: // ----------------------------------------------------------

	typedef std::map<tukk , ICVar*, string_nocase_lt> ConsoleVariablesMap;    // key points into string stored in ICVar or in .exe/.dll
	typedef ConsoleVariablesMap::iterator                   ConsoleVariablesMapItor;

	typedef std::vector<std::pair<tukk , ICVar*>>     ConsoleVariablesVector;

	typedef CListenerSet<IManagedConsoleCommandListener*>   TManagedConsoleCommandListener;
	TManagedConsoleCommandListener m_managedConsoleCommandListeners;

	void LogChangeMessage(tukk name, const bool isConst, const bool isCheat, const bool isReadOnly, const bool isDeprecated,
	                      tukk oldValue, tukk newValue, const bool isProcessingGroup, const bool allowChange);

	void        AddCheckedCVar(ConsoleVariablesVector& vector, const ConsoleVariablesVector::value_type& value);
	void        RemoveCheckedCVar(ConsoleVariablesVector& vector, const ConsoleVariablesVector::value_type& value);
	static void AddCVarsToHash(ConsoleVariablesVector::const_iterator begin, ConsoleVariablesVector::const_iterator end, CCrc32& runningNameCrc32, CCrc32& runningNameValueCrc32);
	static bool CVarNameLess(const std::pair<tukk , ICVar*>& lhs, const std::pair<tukk , ICVar*>& rhs);


	virtual void AddCommand(tukk sCommand, ConsoleCommandFunc func, i32 nFlags = 0, tukk sHelp = NULL, bool bIsManagedExternally = false);
	virtual void AddCommand(tukk sName, tukk sScriptFunc, i32 nFlags = 0, tukk sHelp = NULL);

	virtual ICVar* RegisterString(tukk sName, tukk sValue, i32 nFlags, tukk help = "", ConsoleVarFunc pChangeFunc = 0);
	virtual ICVar* RegisterInt(tukk sName, i32 iValue, i32 nFlags, tukk help = "", ConsoleVarFunc pChangeFunc = 0);
	virtual ICVar* RegisterInt64(tukk sName, int64 iValue, i32 nFlags, tukk help = "", ConsoleVarFunc pChangeFunc = 0);
	virtual ICVar* RegisterFloat(tukk sName, float fValue, i32 nFlags, tukk help = "", ConsoleVarFunc pChangeFunc = 0);
	virtual ICVar* Register(tukk name, float* src, float defaultvalue, i32 flags = 0, tukk help = "", ConsoleVarFunc pChangeFunc = 0, bool allowModify = true);
	virtual ICVar* Register(tukk name, i32* src, i32 defaultvalue, i32 flags = 0, tukk help = "", ConsoleVarFunc pChangeFunc = 0, bool allowModify = true);
	virtual ICVar* Register(tukk name, tukk* src, tukk defaultvalue, i32 flags = 0, tukk help = "", ConsoleVarFunc pChangeFunc = 0, bool allowModify = true);
	virtual ICVar* Register(ICVar* pVar) { RegisterVar(pVar); return pVar; }

	typedef std::map<string, CConsoleCommand, string_nocase_lt>                        ConsoleCommandsMap;
	typedef ConsoleCommandsMap::iterator                                               ConsoleCommandsMapItor;

	typedef std::map<string, string>                                                   ConsoleBindsMap;
	typedef ConsoleBindsMap::iterator                                                  ConsoleBindsMapItor;

	typedef std::map<string, IConsoleArgumentAutoComplete*, stl::less_stricmp<string>> ArgumentAutoCompleteMap;

	struct SConfigVar
	{
		string m_value;
		bool   m_partOfGroup;
		u32 nCVarOrFlags;
	};
	typedef std::map<string, SConfigVar, string_nocase_lt> ConfigVars;

	struct SDeferredCommand
	{
		string command;
		bool   silentMode;

		SDeferredCommand(const string& command, bool silentMode)
			: command(command), silentMode(silentMode)
		{}
	};
	typedef std::list<SDeferredCommand> TDeferredCommandList;

	typedef std::list<IConsoleVarSink*> ConsoleVarSinks;

	// --------------------------------------------------------------------------------

	ConsoleBuffer                  m_dqConsoleBuffer;
	ConsoleBuffer                  m_dqHistory;

	bool                           m_bStaticBackground;
	i32                            m_nLoadingBackTexID;
	i32                            m_nWhiteTexID;
	i32                            m_nProgress;
	i32                            m_nProgressRange;

	string                         m_sInputBuffer;
	string                         m_sReturnString;

	string                         m_sPrevTab;
	i32                            m_nTabCount;

	ConsoleCommandsMap             m_mapCommands;             //
	ConsoleBindsMap                m_mapBinds;                //
	ConsoleVariablesMap            m_mapVariables;            //
	ConsoleVariablesVector         m_randomCheckedVariables;
	ConsoleVariablesVector         m_alwaysCheckedVariables;
	std::vector<IOutputPrintSink*> m_OutputSinks;             // objects in this vector are not released

	TDeferredCommandList           m_deferredCommands;        // A fifo of deferred commands
	bool                           m_deferredExecution;       // True when deferred commands are processed
	i32                            m_waitFrames;              // A counter which is used by wait_frames command
	CTimeValue                     m_waitSeconds;             // An absolute timestamp which is used by wait_seconds command
	i32                            m_blockCounter;            // This counter is incremented whenever a blocker command (VF_BLOCKFRAME) is executed.

	ArgumentAutoCompleteMap        m_mapArgumentAutoComplete;

	ConsoleVarSinks                m_consoleVarSinks;

	ConfigVars                     m_configVars;              // temporary data of cvars that haven't been created yet
	std::multimap<string, string>  m_configCommands;  // temporary data of commands that haven't been created yet

	i32                            m_nScrollPos;
	i32                            m_nTempScrollMax;          // for currently opened console, reset to m_nScrollMax
	i32                            m_nScrollMax;              //
	i32                            m_nScrollLine;
	i32                            m_nHistoryPos;
	size_t                         m_nCursorPos;                // x position in characters
	ITexture*                      m_pImage;

	float                          m_fRepeatTimer;            // relative, next repeat even in .. decreses over time, repeats when 0, only valid if m_nRepeatEvent.keyId != eKI_Unknown
	SInputEvent                    m_nRepeatEvent;            // event that will be repeated

	float                          m_fCursorBlinkTimer;       // relative, increases over time,
	bool                           m_bDrawCursor;

	ScrollDir                      m_sdScrollDir;

	bool                           m_bConsoleActive;
	bool                           m_bActivationKeyEnable;
	bool                           m_bIsProcessingGroup;

	size_t                         m_nCheatHashRangeFirst;
	size_t                         m_nCheatHashRangeLast;
	bool                           m_bCheatHashDirty;
	uint64                         m_nCheatHash;

	CSystem*                       m_pSystem;
	IFFont*                        m_pFont;
	IRenderer*                     m_pRenderer;
	IInput*                        m_pInput;
	ITimer*                        m_pTimer;
	INetwork*                      m_pNetwork;                // EvenBalance - M. Quinn

	ICVar*                         m_pSysDeactivateConsole;

	ELoadConfigurationType         m_currentLoadConfigType;

	bool                           m_readOnly;

	static i32                     con_display_last_messages;
	static i32                     con_line_buffer_size;
	static i32                     con_showonload;
	static i32                     con_debug;
	static i32                     con_restricted;

	friend void Command_SetWaitSeconds(IConsoleCmdArgs* Cmd);
	friend void Command_SetWaitFrames(IConsoleCmdArgs* Cmd);
#if ALLOW_AUDIT_CVARS
	friend void Command_AuditCVars(IConsoleCmdArgs* pArg);
#endif // ALLOW_AUDIT_CVARS
	friend void Command_DumpCommandsVars(IConsoleCmdArgs* Cmd);
	friend void Command_DumpVars(IConsoleCmdArgs* Cmd);
	friend class CConsoleHelpGen;
};

#endif // !defined(AFX_XCONSOLE_H__BA902011_5C47_4954_8E09_68598456912D__INCLUDED_)
