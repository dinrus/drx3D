// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_SCRIPTOBJECT_H__6EA3E6D6_4FF9_4709_BD62_D5A97C40DB68__INCLUDED_)
#define AFX_SCRIPTOBJECT_H__6EA3E6D6_4FF9_4709_BD62_D5A97C40DB68__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/Script/IScriptSystem.h>
//#pragma warning(push) // Because lua.h touches warning C4996
extern "C" {
#include <lua.h>
}
//#pragma warning(pop)
class CScriptSystem;

enum
{
	DELETED_REF = -1,
	NULL_REF    = 0,
};

#ifdef DEBUG_LUA_STATE
extern std::set<class CScriptTable*> gAllScriptTables;
#endif

/*! IScriptTable implementation
   @see IScriptTable
 */
class CScriptTable : public IScriptTable
{
public:
	//! constructor
	CScriptTable() { m_nRef = NULL_REF; m_nRefCount = 0; }

	// interface IScriptTable ----------------------------------------------------------------
	virtual void           AddRef()  { m_nRefCount++; }
	virtual void           Release() { if (--m_nRefCount <= 0) DeleteThis(); };

	virtual IScriptSystem* GetScriptSystem() const;
	virtual void           Delegate(IScriptTable* pMetatable);

	virtual uk          GetUserDataValue();

	//////////////////////////////////////////////////////////////////////////
	// Set/Get chain.
	//////////////////////////////////////////////////////////////////////////
	virtual bool BeginSetGetChain();
	virtual void EndSetGetChain();

	//////////////////////////////////////////////////////////////////////////
	virtual void SetValueAny(tukk sKey, const ScriptAnyValue& any, bool bChain = false);
	virtual bool GetValueAny(tukk sKey, ScriptAnyValue& any, bool bChain = false);

	//////////////////////////////////////////////////////////////////////////
	virtual void          SetAtAny(i32 nIndex, const ScriptAnyValue& any);
	virtual bool          GetAtAny(i32 nIndex, ScriptAnyValue& any);

	virtual ScriptVarType GetValueType(tukk sKey);
	virtual ScriptVarType GetAtType(i32 nIdx);

	//////////////////////////////////////////////////////////////////////////
	// Iteration.
	//////////////////////////////////////////////////////////////////////////
	virtual IScriptTable::Iterator BeginIteration(bool resolvePrototypeTableAsWell = false);
	virtual bool                   MoveNext(Iterator& iter);
	virtual void                   EndIteration(const Iterator& iter);
	//////////////////////////////////////////////////////////////////////////

	virtual void Clear();
	virtual i32  Count();
	virtual bool Clone(IScriptTable* pSrcTable, bool bDeepCopy = false, bool bCopyByReference = false);
	virtual void Dump(IScriptTableDumpSink* p);

	virtual bool AddFunction(const SUserFunctionDesc& fd);

	// --------------------------------------------------------------------------
	void CreateNew();

	i32  GetRef();
	void Attach();
	void AttachToObject(IScriptTable* so);
	void DeleteThis();

	// Create object from pool.
	void Recreate() { m_nRef = NULL_REF; m_nRefCount = 1; };
	// Assign a metatable to a table.
	void SetMetatable(IScriptTable* pMetatable);
	// Push reference of this object to the stack.
	void PushRef();
	// Push reference to specified script table to the stack.
	void PushRef(IScriptTable* pObj);

	//////////////////////////////////////////////////////////////////////////
	// Custom new/delete.
	//////////////////////////////////////////////////////////////////////////
	uk operator new(size_t nSize);
	void  operator delete(uk ptr);

public:
	// Lua state, set by CScriptSystem::Init
	static lua_State*     L;
	// Pointer to ScriptSystem, set by CScriptSystem::Init
	static CScriptSystem* m_pSS;

private:
	static i32  StdCFunction(lua_State* L);
	static i32  StdCUserDataFunction(lua_State* L);

	static void CloneTable(i32 srcTable, i32 trgTable);
	static void CloneTable_r(i32 srcTable, i32 trgTable);
	static void ReferenceTable_r(i32 scrTable, i32 trgTable);

private:
	i32 m_nRefCount;
	i32 m_nRef;
};

#endif // !defined(AFX_SCRIPTOBJECT_H__6EA3E6D6_4FF9_4709_BD62_D5A97C40DB68__INCLUDED_)
