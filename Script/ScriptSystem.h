// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/IConsole.h>
#include <drx3D/Script/IScriptSystem.h>
//#pragma warning(push) // Because lua.h touches warning C4996
extern "C" {
#include <lua.h>
#include <drx/plugin/lua/lauxlib.h>
}
//#pragma warning(pop)

#include "StackGuard.h"
#include "ScriptBinding.h"
#include "ScriptTimerMgr.h"

class CLUADbg;

struct SLuaStackEntry
{
	i32    line;
	string source;
	string description;
};

// Returns literal representation of the type value
inline tukk ScriptAnyTypeToString(EScriptAnyType type)
{
	switch (type)
	{
	case EScriptAnyType::Any:
		return "Any";
	case EScriptAnyType::Nil:
		return "Null";
	case EScriptAnyType::Boolean:
		return "Boolean";
	case EScriptAnyType::String:
		return "String";
	case EScriptAnyType::Number:
		return "Number";
	case EScriptAnyType::Function:
		return "Function";
	case EScriptAnyType::Table:
		return "Table";
	case EScriptAnyType::UserData:
		return "UserData";
	case EScriptAnyType::Handle:
		return "Pointer";
	case EScriptAnyType::Vector:
		return "Vec3";
	default:
		return "Unknown";
	}
}

// Returns literal representation of the type value
inline tukk ScriptVarTypeAsCStr(ScriptVarType t)
{
	switch (t)
	{
	case svtNull:
		return "Null";
	case svtBool:
		return "Boolean";
	case svtString:
		return "String";
	case svtNumber:
		return "Number";
	case svtFunction:
		return "Function";
	case svtObject:
		return "Table";
	case svtUserData:
		return "UserData";
	case svtPointer:
		return "Pointer";
	default:
		return "Unknown";
	}
}

typedef std::set<string, stl::less_stricmp<string>> ScriptFileList;
typedef ScriptFileList::iterator                    ScriptFileListItor;

//////////////////////////////////////////////////////////////////////////
// forwarde declarations.
class CScriptSystem;
class CScriptTable;

#define SCRIPT_OBJECT_POOL_SIZE 15000

/*! IScriptSystem implementation
   @see IScriptSystem
 */
class CScriptSystem : public IScriptSystem, public ISystemEventListener
{
public:
	//! constructor
	CScriptSystem();
	//! destructor
	virtual ~CScriptSystem();
	//!
	bool          Init(ISystem* pSystem, bool bStdLibs, i32 nStackSize);

	void          Update();
	void          SetGCFrequency(const float fRate);

	void          SetEnvironment(HSCRIPTFUNCTION scriptFunction, IScriptTable* pEnv);
	IScriptTable* GetEnvironment(HSCRIPTFUNCTION scriptFunction);

	//!
	void RegisterErrorHandler(void);

	//!
	bool _ExecuteFile(tukk sFileName, bool bRaiseError, IScriptTable* pEnv = 0);
	//!

	// interface IScriptSystem -----------------------------------------------------------

	virtual bool          ExecuteFile(tukk sFileName, bool bRaiseError, bool bForceReload, IScriptTable* pEnv = 0);
	virtual bool          ExecuteBuffer(tukk sBuffer, size_t nSize, tukk sBufferDescription, IScriptTable* pEnv = 0);
	virtual void          UnloadScript(tukk sFileName);
	virtual void          UnloadScripts();
	virtual bool          ReloadScript(tukk sFileName, bool bRaiseError);
	virtual bool          ReloadScripts();
	virtual void          DumpLoadedScripts();

	virtual IScriptTable* CreateTable(bool bEmpty = false);

	//////////////////////////////////////////////////////////////////////////
	// Begin Call.
	//////////////////////////////////////////////////////////////////////////
	virtual i32 BeginCall(HSCRIPTFUNCTION hFunc);
	virtual i32 BeginCall(tukk sFuncName);
	virtual i32 BeginCall(tukk sTableName, tukk sFuncName);
	virtual i32 BeginCall(IScriptTable* pTable, tukk sFuncName);

	//////////////////////////////////////////////////////////////////////////
	// End Call.
	//////////////////////////////////////////////////////////////////////////
	virtual bool EndCall();
	virtual bool EndCallAny(ScriptAnyValue& any);
	virtual bool EndCallAnyN(i32 n, ScriptAnyValue* anys);

	//////////////////////////////////////////////////////////////////////////
	// Get function pointer.
	//////////////////////////////////////////////////////////////////////////
	virtual HSCRIPTFUNCTION GetFunctionPtr(tukk sFuncName);
	virtual HSCRIPTFUNCTION GetFunctionPtr(tukk sTableName, tukk sFuncName);
	virtual void            ReleaseFunc(HSCRIPTFUNCTION f);
	virtual HSCRIPTFUNCTION AddFuncRef(HSCRIPTFUNCTION f);
	virtual bool            CompareFuncRef(HSCRIPTFUNCTION f1, HSCRIPTFUNCTION f2);

	virtual ScriptAnyValue  CloneAny(const ScriptAnyValue& any);
	virtual void            ReleaseAny(const ScriptAnyValue& any);

	//////////////////////////////////////////////////////////////////////////
	// Push function param.
	//////////////////////////////////////////////////////////////////////////
	virtual void PushFuncParamAny(const ScriptAnyValue& any);

	//////////////////////////////////////////////////////////////////////////
	// Set global value.
	//////////////////////////////////////////////////////////////////////////
	virtual void          SetGlobalAny(tukk sKey, const ScriptAnyValue& any);
	virtual bool          GetGlobalAny(tukk sKey, ScriptAnyValue& any);

	virtual IScriptTable* CreateUserData(uk ptr, size_t size);
	virtual void          ForceGarbageCollection();
	virtual i32           GetCGCount();
	virtual void          SetGCThreshhold(i32 nKb);
	virtual void          Release();
	virtual void          ShowDebugger(tukk pszSourceFile, i32 iLine, tukk pszReason);
	virtual HBREAKPOINT   AddBreakPoint(tukk sFile, i32 nLineNumber);
	virtual IScriptTable* GetLocalVariables(i32 nLevel, bool bRecursive);
	virtual IScriptTable* GetCallsStack();// { return 0; };
	virtual void          DumpCallStack();

	virtual void          DebugContinue() {}
	virtual void          DebugStepNext() {}
	virtual void          DebugStepInto() {}
	virtual void          DebugDisable()  {}

	virtual BreakState    GetBreakState() { return bsNoBreak; }
	virtual void          GetMemoryStatistics(IDrxSizer* pSizer) const;
	virtual void          GetScriptHash(tukk sPath, tukk szKey, u32& dwHash);
	virtual void          PostInit();
	virtual void          RaiseError(tukk format, ...) PRINTF_PARAMS(2, 3);
	virtual void          LoadScriptedSurfaceTypes(tukk sFolder, bool bReload);
	virtual void          SerializeTimers(ISerialize* pSer);
	virtual void          ResetTimers();

	virtual i32           GetStackSize() const;
	virtual u32        GetScriptAllocSize();

	virtual uk         Allocate(size_t sz);
	virtual size_t        Deallocate(uk ptr);

	void                  PushAny(const ScriptAnyValue& var);
	bool                  PopAny(ScriptAnyValue& var);
	// Convert top stack item to Any.
	bool                  ToAny(ScriptAnyValue& var, i32 index);
	void                  PushVec3(const Vec3& vec);
	bool                  ToVec3(Vec3& vec, i32 index);
	// Push table reference
	void                  PushTable(IScriptTable* pTable);
	// Attach reference at the top of the stack to the specified table pointer.
	void                  AttachTable(IScriptTable* pTable);
	bool                  GetRecursiveAny(IScriptTable* pTable, tukk sKey, ScriptAnyValue& any);

	lua_State*            GetLuaState() const { return L; }
	void                  TraceScriptError(tukk file, i32 line, tukk errorStr);
	void                  LogStackTrace();

	CScriptTimerMgr*      GetScriptTimerMgr() { return m_pScriptTimerMgr; };

	void                  GetCallStack(std::vector<SLuaStackEntry>& callstack);
	bool                  IsCallStackEmpty(void);
	void                  DumpStateToFile(tukk filename);

	//////////////////////////////////////////////////////////////////////////
	// Facility to pre-catch any lua buffer
	//////////////////////////////////////////////////////////////////////////
	virtual HSCRIPTFUNCTION CompileBuffer(tukk sBuffer, size_t nSize, tukk sBufferDesc);
	virtual i32             PreCacheBuffer(tukk sBuffer, size_t nSize, tukk sBufferDesc);
	virtual i32             BeginPreCachedBuffer(i32 iIndex);
	virtual void            ClearPreCachedBuffer();

	//////////////////////////////////////////////////////////////////////////
	// ISystemEventListener
	//////////////////////////////////////////////////////////////////////////
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	//////////////////////////////////////////////////////////////////////////

	// Used by Lua debugger to maintain callstack exposed to C++
	void ExposedCallstackClear();
	void ExposedCallstackPush(tukk sFunction, i32 nLine, tukk sSource);
	void ExposedCallstackPop();

private: // ---------------------------------------------------------------------
	//!
	bool       EndCallN(i32 nReturns);

	static i32 ErrorHandler(lua_State* L);

	// Create default metatables.
	void CreateMetatables();

	// Now private, doesn't need to be updated by main thread
	void        EnableDebugger(ELuaDebugMode eDebugMode);
	void        EnableCodeCoverage(bool enable);

	static void DebugModeChange(ICVar* cvar);
	static void CodeCoverageChange(ICVar* cvar);

	// Loaded file tracking helpers
	void AddFileToList(tukk sName);
	void RemoveFileFromList(const ScriptFileListItor& itor);

	// ----------------------------------------------------------------------------
private:
	static CScriptSystem* s_mpScriptSystem;
	lua_State*            L;
	ICVar*                m_cvar_script_debugger; // Stores current debugging mode
	ICVar*                m_cvar_script_coverage;
	i32                   m_nTempArg;
	i32                   m_nTempTop;

	IScriptTable*         m_pUserDataMetatable;
	IScriptTable*         m_pPreCacheBufferTable;
	std::vector<string>   m_vecPreCached;

	HSCRIPTFUNCTION       m_pErrorHandlerFunc;

	ScriptFileList        m_dqLoadedFiles;

	CScriptBindings       m_stdScriptBinds;
	ISystem*              m_pSystem;

	float                 m_fGCFreq;      //!< relative time in seconds
	float                 m_lastGCTime;   //!< absolute time in seconds
	i32                   m_nLastGCCount; //!<
	i32                   m_forceReloadCount;

	CScriptTimerMgr*      m_pScriptTimerMgr;

	// Store a simple callstack that can be inspected in C++ debugger
	const static i32 MAX_CALLDEPTH = 32;
	i32              m_nCallDepth;
	stack_string     m_sCallDescriptions[MAX_CALLDEPTH];

public: // -----------------------------------------------------------------------

	string   m_sLastBreakSource; //!
	i32      m_nLastBreakLine;   //!
	CLUADbg* m_pLuaDebugger;
};
