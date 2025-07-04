// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/ScriptTable.h>
#include <drx3D/Script/ScriptSystem.h>
#include <drx3D/Script/StackGuard.h>
#include <drx3D/Script/FunctionHandler.h>

#include <drx3D/Sys/ISystem.h>

lua_State* CScriptTable::L = NULL;
CScriptSystem* CScriptTable::m_pSS = NULL;

#ifdef DEBUG_LUA_STATE
std::set<class CScriptTable*> gAllScriptTables;
#endif

//////////////////////////////////////////////////////////////////////////
IScriptSystem* CScriptTable::GetScriptSystem() const
{
	return m_pSS;
};

//////////////////////////////////////////////////////////////////////////
i32 CScriptTable::GetRef()
{
	return m_nRef;
}

//////////////////////////////////////////////////////////////////////////
uk CScriptTable::GetUserDataValue()
{
	PushRef();
	uk ptr = lua_touserdata(L, -1);
	lua_pop(L, 1);
	return ptr;
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::PushRef()
{
	if (m_nRef != DELETED_REF && m_nRef != NULL_REF)
		luaL_ref(L, m_nRef);
	else
	{
		lua_pushnil(L);
		if (m_nRef == DELETED_REF)
		{
			assert(0 && "Access to deleted script object");
			DrxFatalError("Access to deleted script object %x", (u32)(UINT_PTR)this);
		}
		else
		{
			assert(0 && "Pushing Nil table reference");
			ScriptWarning("Pushing Nil table reference");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::PushRef(IScriptTable* pObj)
{
	i32 nRef = ((CScriptTable*)pObj)->m_nRef;
	if (nRef != DELETED_REF)
		luaL_ref(L, nRef);
	else
		DrxFatalError("Access to deleted script object");
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::Attach()
{
	if (m_nRef != NULL_REF)
		luaL_unref(L, 0, m_nRef);
	m_nRef = luaL_ref(L, 1);

#ifdef DEBUG_LUA_STATE
	gAllScriptTables.insert(this);
#endif
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::AttachToObject(IScriptTable* so)
{
	PushRef(so);
	Attach();
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::DeleteThis()
{
	if (m_nRef == DELETED_REF)
	{
		assert(0);
		DrxFatalError("Attempt to Release already released script table.");
	}

#ifdef DEBUG_LUA_STATE
	gAllScriptTables.erase(this);
#endif

	if (m_nRef != NULL_REF)
		luaL_unref(L, 0, m_nRef);

	m_nRef = DELETED_REF;
	delete this;
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::CreateNew()
{
	lua_newtable(L);
	Attach();
}

//////////////////////////////////////////////////////////////////////////
bool CScriptTable::BeginSetGetChain()
{
	PushRef();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::EndSetGetChain()
{
	if (lua_istable(L, -1))
		lua_pop(L, 1);
	else
	{
		assert(0 && "Mismatch in Set/Get Chain");
	}
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::SetValueAny(tukk sKey, const ScriptAnyValue& any, bool bChain)
{
	CHECK_STACK(L);
	i32 top = lua_gettop(L);

	ScriptAnyValue oldValue;
	if (top && lua_getmetatable(L, -1))     // if there is no metatable nothing is pushed
	{
		lua_pop(L, 1);    // pop the metatable - we only care that it exists, not about the value
		if (GetValueAny(sKey, oldValue, bChain) && oldValue == any)
			return;
	}

	if (!bChain)
		PushRef();

	assert(sKey);
	size_t len = strlen(sKey);

	if (any.GetType() == EScriptAnyType::Vector)
	{
		// Check if we can reuse Vec3 value already in the table.
		lua_pushlstring(L, sKey, len);
		lua_gettable(L, -2);
		i32 luatype = lua_type(L, -1);
		if (luatype == LUA_TTABLE)
		{
			lua_pushlstring(L, "x", 1);
			lua_gettable(L, -2);
			bool bXIsNumber = lua_isnumber(L, -1) != 0;
			lua_pop(L, 1); // pop x value.
			if (bXIsNumber)
			{
				// Assume its a vector, just fill it with new vector values.
				lua_pushlstring(L, "x", 1);
				lua_pushnumber(L, any.GetVector().x);
				lua_settable(L, -3);
				lua_pushlstring(L, "y", 1);
				lua_pushnumber(L, any.GetVector().y);
				lua_settable(L, -3);
				lua_pushlstring(L, "z", 1);
				lua_pushnumber(L, any.GetVector().z);
				lua_settable(L, -3);

				lua_settop(L, top);
				return;
			}
		}
		lua_pop(L, 1); // pop key value.
	}
	lua_pushlstring(L, sKey, len);
	m_pSS->PushAny(any);
	lua_settable(L, -3);
	lua_settop(L, top);
}

//////////////////////////////////////////////////////////////////////////
bool CScriptTable::GetValueAny(tukk sKey, ScriptAnyValue& any, bool bChain)
{
	CHECK_STACK(L);
	i32 top = lua_gettop(L);
	if (!bChain)
		PushRef();
	lua_pushstring(L, sKey);
	lua_gettable(L, -2);
	bool res = m_pSS->PopAny(any);
	lua_settop(L, top);
	return res;
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::SetAtAny(i32 nIndex, const ScriptAnyValue& any)
{
	CHECK_STACK(L);
	PushRef();
	m_pSS->PushAny(any);
	lua_rawseti(L, -2, nIndex);
	lua_pop(L, 1); // Pop table.
}

//////////////////////////////////////////////////////////////////////////
bool CScriptTable::GetAtAny(i32 nIndex, ScriptAnyValue& any)
{
	CHECK_STACK(L);
	PushRef();
	lua_rawgeti(L, -1, nIndex);
	bool res = m_pSS->PopAny(any);
	lua_pop(L, 1); // Pop table.

	return res;
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptTable::Count()
{
	CHECK_STACK(L);

	PushRef();
	i32 count = /*luaL_getn*/luaL_len(L, -1);
	lua_pop(L, 1);

	return count;
}

void CScriptTable::CloneTable_r(i32 srcTable, i32 trgTable)
{
	CHECK_STACK(L);
	i32 top = lua_gettop(L);
	lua_pushnil(L);  // first key
	while (lua_next(L, srcTable) != 0)
	{
		if (lua_type(L, -1) == LUA_TTABLE)
		{
			i32 srct = lua_gettop(L);

			lua_pushvalue(L, -2); // Push again index.
			lua_newtable(L);      // Make value.
			i32 trgt = lua_gettop(L);
			CloneTable_r(srct, trgt);
			lua_rawset(L, trgTable); // Set new table to trgtable.
		}
		else
		{
			// `key' is at index -2 and `value' at index -1
			lua_pushvalue(L, -2); // Push again index.
			lua_pushvalue(L, -2); // Push value.
			lua_rawset(L, trgTable);
		}
		lua_pop(L, 1); // pop value, leave index.
	}
	lua_settop(L, top); // Restore stack.
}

void CScriptTable::ReferenceTable_r(i32 srcTable, i32 trgTable)
{
	CHECK_STACK(L);
	i32 top = lua_gettop(L);

	lua_newtable(L);                                  // push new meta table
	lua_pushlstring(L, "__index", strlen("__index")); // push __index
	lua_pushvalue(L, srcTable);                       // push src table
	lua_rawset(L, -3);                                // meta.__index==src table
	lua_setmetatable(L, trgTable);                    // set meta table

	lua_pushnil(L);  // first key
	while (lua_next(L, srcTable) != 0)
	{
		if (lua_type(L, -1) == LUA_TTABLE)
		{
			i32 srct = lua_gettop(L);
			lua_pushvalue(L, -2); // Push again index.
			lua_newtable(L);      // Make value.
			i32 trgt = lua_gettop(L);
			ReferenceTable_r(srct, trgt);
			lua_rawset(L, trgTable); // Set new table to trgtable.
		}
		lua_pop(L, 1); // pop value, leave index.
	}
	lua_settop(L, top); // Restore stack.
}

void CScriptTable::CloneTable(i32 srcTable, i32 trgTable)
{
	CHECK_STACK(L);
	i32 top = lua_gettop(L);
	lua_pushnil(L);  // first key
	while (lua_next(L, srcTable) != 0)
	{
		// `key' is at index -2 and `value' at index -1
		lua_pushvalue(L, -2); // Push again index.
		lua_pushvalue(L, -2); // Push value.
		lua_rawset(L, trgTable);
		lua_pop(L, 1); // pop value, leave index.
	}
	lua_settop(L, top); // Restore stack.
}

//////////////////////////////////////////////////////////////////////////
bool CScriptTable::Clone(IScriptTable* pSrcTable, bool bDeepCopy, bool bCopyByReference)
{
	CHECK_STACK(L);
	i32 top = lua_gettop(L);

	PushRef(pSrcTable);
	PushRef();

	i32 srcTable = top + 1;
	i32 trgTable = top + 2;

	if (bDeepCopy)
	{
		if (bCopyByReference)
		{
			ReferenceTable_r(srcTable, trgTable);
		}
		else
		{
			CloneTable_r(srcTable, trgTable);
		}
	}
	else
	{
		CloneTable(srcTable, trgTable);
	}
	lua_settop(L, top); // Restore stack.

	return true;
}

static void IterTable(lua_State* L, IScriptTableDumpSink* p)
{
	lua_pushnil(L); // first key
	while (lua_next(L, -2))
	{
		i32 keyType = lua_type(L, -2);
		i32 valType = lua_type(L, -1);

		if (keyType == LUA_TSTRING)
		{
			tukk key = lua_tostring(L, -2);
			switch (valType)
			{
			case LUA_TNIL:
				p->OnElementFound(key, svtNull);
				break;
			case LUA_TBOOLEAN:
				p->OnElementFound(key, svtBool);
				break;
			case LUA_TLIGHTUSERDATA:
				p->OnElementFound(key, svtPointer);
				break;
			case LUA_TNUMBER:
				p->OnElementFound(key, svtNumber);
				break;
			case LUA_TSTRING:
				p->OnElementFound(key, svtString);
				break;
			case LUA_TTABLE:
				if (strcmp(key, "__index") != 0)
					p->OnElementFound(key, svtObject);
				break;
			case LUA_TFUNCTION:
				p->OnElementFound(key, svtFunction);
				break;
			case LUA_TUSERDATA:
				p->OnElementFound(key, svtUserData);
				break;
			}
		}
		else
		{
			i32 idx = (i32)lua_tonumber(L, -2);
			switch (valType)
			{
			case LUA_TNIL:
				p->OnElementFound(idx, svtNull);
				break;
			case LUA_TBOOLEAN:
				p->OnElementFound(idx, svtBool);
				break;
			case LUA_TLIGHTUSERDATA:
				p->OnElementFound(idx, svtPointer);
				break;
			case LUA_TNUMBER:
				p->OnElementFound(idx, svtNumber);
				break;
			case LUA_TSTRING:
				p->OnElementFound(idx, svtString);
				break;
			case LUA_TTABLE:
				p->OnElementFound(idx, svtObject);
				break;
			case LUA_TFUNCTION:
				p->OnElementFound(idx, svtFunction);
				break;
			case LUA_TUSERDATA:
				p->OnElementFound(idx, svtUserData);
				break;
			}
		}

		lua_pop(L, 1);  // pop value, keep key
	}
}

void CScriptTable::Dump(IScriptTableDumpSink* p)
{
	if (!p) return;
	CHECK_STACK(L);
	i32 top = lua_gettop(L);

	PushRef();

	i32 trgTable = top + 1;

	lua_pushnil(L);  // first key
	i32 reftop = lua_gettop(L);
	while (lua_next(L, trgTable) != 0)
	{
		// `key' is at index -2 and `value' at index -1
		if (lua_type(L, -2) == LUA_TSTRING)
		{
			tukk sName = lua_tostring(L, -2); // again index
			switch (lua_type(L, -1))
			{
			case LUA_TNIL:
				p->OnElementFound(sName, svtNull);
				break;
			case LUA_TBOOLEAN:
				p->OnElementFound(sName, svtBool);
				break;
			case LUA_TLIGHTUSERDATA:
				p->OnElementFound(sName, svtPointer);
				break;
			case LUA_TNUMBER:
				p->OnElementFound(sName, svtNumber);
				break;
			case LUA_TSTRING:
				p->OnElementFound(sName, svtString);
				break;
			case LUA_TTABLE:
				if (strcmp(sName, "__index") != 0)
					p->OnElementFound(sName, svtObject);
				break;
			case LUA_TFUNCTION:
				p->OnElementFound(sName, svtFunction);
				break;
			case LUA_TUSERDATA:
				p->OnElementFound(sName, svtUserData);
				break;
			}
			;
		}
		else
		{
			i32 nIdx = (i32)lua_tonumber(L, -2);  // again index
			switch (lua_type(L, -1))
			{
			case LUA_TNIL:
				p->OnElementFound(nIdx, svtNull);
				break;
			case LUA_TBOOLEAN:
				p->OnElementFound(nIdx, svtBool);
				break;
			case LUA_TLIGHTUSERDATA:
				p->OnElementFound(nIdx, svtPointer);
				break;
			case LUA_TNUMBER:
				p->OnElementFound(nIdx, svtNumber);
				break;
			case LUA_TSTRING:
				p->OnElementFound(nIdx, svtString);
				break;
			case LUA_TTABLE:
				p->OnElementFound(nIdx, svtObject);
				break;
			case LUA_TFUNCTION:
				p->OnElementFound(nIdx, svtFunction);
				break;
			case LUA_TUSERDATA:
				p->OnElementFound(nIdx, svtUserData);
				break;
			}
			;
		}
		lua_settop(L, reftop); // pop value, leave index.
	}

	if (lua_getmetatable(L, -1))
	{
		lua_pushstring(L, "__index");
		lua_rawget(L, -2);
		if (lua_type(L, -1) == LUA_TTABLE)
			IterTable(L, p);
	}
	lua_settop(L, trgTable);

	lua_pop(L, 1); // pop table ref
}

ScriptVarType CScriptTable::GetValueType(tukk sKey)
{
	CHECK_STACK(L);
	ScriptVarType type = svtNull;

	PushRef();
	lua_pushstring(L, sKey);
	lua_gettable(L, -2);
	i32 luatype = lua_type(L, -1);
	switch (luatype)
	{
	case LUA_TNIL:
		type = svtNull;
		break;
	case LUA_TBOOLEAN:
		type = svtBool;
		break;
	case LUA_TNUMBER:
		type = svtNumber;
		break;
	case LUA_TSTRING:
		type = svtString;
		break;
	case LUA_TFUNCTION:
		type = svtFunction;
		break;
	case LUA_TLIGHTUSERDATA:
		type = svtPointer;
		break;
	case LUA_TTABLE:
		type = svtObject;
		break;
	}
	lua_pop(L, 2); // Pop value and table.
	return type;
}

ScriptVarType CScriptTable::GetAtType(i32 nIdx)
{
	CHECK_STACK(L);
	ScriptVarType svtRetVal = svtNull;
	PushRef();

	if (/*luaL_getn*/luaL_len(L, -1) < nIdx)
	{
		lua_pop(L, 1);
		return svtNull;
	}

	lua_rawgeti(L, -1, nIdx);

	switch (lua_type(L, -1))
	{
	case LUA_TNIL:
		svtRetVal = svtNull;
		break;
	case LUA_TBOOLEAN:
		svtRetVal = svtBool;
		break;
	case LUA_TNUMBER:
		svtRetVal = svtNumber;
		break;
	case LUA_TSTRING:
		svtRetVal = svtString;
		break;
	case LUA_TTABLE:
		svtRetVal = svtObject;
		break;
	case LUA_TFUNCTION:
		svtRetVal = svtFunction;
		break;
	}

	lua_pop(L, 2);
	return svtRetVal;
}

//////////////////////////////////////////////////////////////////////////
IScriptTable::Iterator CScriptTable::BeginIteration(bool resolvePrototypeTableAsWell)
{
	Iterator iter;
	iter.nKey = -1;
	iter.sKey = NULL;
	iter.internal.resolvePrototypeTableAsWell = resolvePrototypeTableAsWell;
	iter.internal.nStackMarker1 = lua_gettop(L) + 1;
	iter.internal.nStackMarker2 = 0;

	PushRef();
	lua_pushnil(L);
	return iter;
}

//////////////////////////////////////////////////////////////////////////
bool CScriptTable::MoveNext(Iterator& iter)
{
	if (!iter.internal.nStackMarker1)
		return false;

	i32 nTop;
	if (iter.internal.nStackMarker2)
		nTop = iter.internal.nStackMarker2 - 1; // already traversing the prototype table
	else
		nTop = iter.internal.nStackMarker1 - 1; // still traversing our own table

	//leave only the index into the stack
	while ((lua_gettop(L) - (nTop + 1)) > 1)
	{
		lua_pop(L, 1);
	}
	bool bResult = lua_next(L, nTop + 1) != 0;
	if (bResult)
	{
		iter.value.Clear();
		bResult = m_pSS->PopAny(iter.value);
		// Get current key.
		m_pSS->ToAny(iter.key, -1);
		if (lua_type(L, -1) == LUA_TSTRING)
		{
			// String key.
			iter.sKey = (tukk)lua_tostring(L, -1);
			iter.nKey = -1;
		}
		else if (lua_type(L, -1) == LUA_TNUMBER)
		{
			// Number key.
			iter.sKey = NULL;
			iter.nKey = (i32)lua_tonumber(L, -1);
		}
		else
		{
			iter.sKey = 0;
			iter.nKey = -1;
		}
	}
	if (!bResult)
	{
		if (iter.internal.nStackMarker1 && !iter.internal.nStackMarker2)
		{
			// just finished traversing our own table
			// => now see if we have a prototype table attached by inspecting our potential metatable
			// => if we don't have a metatable, or have a metatable but no prototype table attached, finish the whole iteration

			if (iter.internal.resolvePrototypeTableAsWell)
			{
				if (lua_getmetatable(L, -1))
				{
					// yep, we have a metatable
					lua_pushstring(L, "__index");
					lua_rawget(L, -2);
					if (lua_type(L, -1) == LUA_TTABLE)
					{
						// yep, the metatable provides us with the prototype table
						iter.internal.nStackMarker2 = lua_gettop(L);
						lua_pushnil(L);
						return MoveNext(iter);
					}
				}
			}
		}

		EndIteration(iter);
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::EndIteration(const Iterator& iterr)
{
	Iterator& iter = const_cast<Iterator&>(iterr);

	if (iter.internal.nStackMarker1)
	{
		lua_settop(L, iter.internal.nStackMarker1 - 1);
		iter.internal.nStackMarker1 = 0;
		iter.internal.nStackMarker2 = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::Clear()
{
	CHECK_STACK(L);

	PushRef();
	i32 trgTable = lua_gettop(L);

	lua_pushnil(L);  // first key
	while (lua_next(L, trgTable) != 0)
	{
		lua_pop(L, 1);        // pop value, leave index.
		lua_pushvalue(L, -1); // Push again index.
		lua_pushnil(L);
		lua_rawset(L, trgTable);
	}
	assert(lua_istable(L, -1));
	lua_pop(L, 1);
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::SetMetatable(IScriptTable* pMetatable)
{
	CHECK_STACK(L);
	//////////////////////////////////////////////////////////////////////////
	// Set metatable for this script object.
	//////////////////////////////////////////////////////////////////////////
	PushRef();           // -2
	PushRef(pMetatable); // -1
	lua_setmetatable(L, -2);
	lua_pop(L, 1); // pop table
}

//////////////////////////////////////////////////////////////////////////
void CScriptTable::Delegate(IScriptTable* pMetatable)
{
	if (!pMetatable)
		return;

	CHECK_STACK(L);

	PushRef(pMetatable);
	lua_pushstring(L, "__index"); // push key.
	PushRef(pMetatable);
	lua_rawset(L, -3); // sets metatable.__index = metatable
	lua_pop(L, 1);     // pop metatable from stack.

	SetMetatable(pMetatable);
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptTable::StdCFunction(lua_State* L)
{
	u8* pBuffer = (u8*)lua_touserdata(L, lua_upvalueindex(1));
	// Not very safe cast, but we sure what is in the upvalue 1 should be.
	FunctionFunctor* pFunction = (FunctionFunctor*)pBuffer;
	int8 nParamIdOffset = *(int8*)(pBuffer + sizeof(FunctionFunctor));
	tukk sFuncName = (tukk)(pBuffer + sizeof(FunctionFunctor) + 1);
	CFunctionHandler fh(m_pSS, L, sFuncName, nParamIdOffset);
	// Call functor.
	i32 nRet = (*pFunction)(&fh);   // i32 CallbackFunction( IFunctionHandler *pHandler )
	return nRet;
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptTable::StdCUserDataFunction(lua_State* L)
{
	u8* pBuffer = (u8*)lua_touserdata(L, lua_upvalueindex(1));
	// Not very safe cast, but we sure what is in the upvalue 1 should be.
	UserDataFunction* pFunction = (UserDataFunction*)pBuffer;
	pBuffer += sizeof(UserDataFunction);
	i32 nSize = *((i32*)pBuffer); // first 4 bytes are size of user data.
	pBuffer += sizeof(i32);
	int8 nParamIdOffset = *(int8*)(pBuffer + nSize);
	tukk sFuncName = (tukk)(pBuffer + nSize + 1);

	INDENT_LOG_DURING_SCOPE(true, "While calling function '%s'...", sFuncName);

	CFunctionHandler fh(m_pSS, L, sFuncName, nParamIdOffset);
	// Call functor.
	i32 nRet = (*pFunction)(&fh, pBuffer, nSize); // i32 CallbackFunction( IFunctionHandler *pHandler,pBuffer,nSize )
	return nRet;
}

//////////////////////////////////////////////////////////////////////////
bool CScriptTable::AddFunction(const SUserFunctionDesc& fd)
{
	CHECK_STACK(L);

	assert(fd.pFunctor || fd.pUserDataFunc != 0);

	// Make function signature.
	char sFuncSignature[256];
	if (fd.sGlobalName[0] != 0)
		drx_sprintf(sFuncSignature, "%s.%s(%s)", fd.sGlobalName, fd.sFunctionName, fd.sFunctionParams);
	else
		drx_sprintf(sFuncSignature, "%s(%s)", fd.sFunctionName, fd.sFunctionParams);

	PushRef();
	lua_pushstring(L, fd.sFunctionName);

	int8 nParamIdOffset = fd.nParamIdOffset;
	if (fd.pFunctor)
	{
		i32 nDataSize = sizeof(fd.pFunctor) + strlen(sFuncSignature) + 1 + 1;

		// Store functor in first upvalue.
		u8* pBuffer = (u8*)lua_newuserdata(L, nDataSize);
		memcpy(pBuffer, &fd.pFunctor, sizeof(fd.pFunctor));
		memcpy(pBuffer + sizeof(fd.pFunctor), &nParamIdOffset, 1);
		memcpy(pBuffer + sizeof(fd.pFunctor) + 1, sFuncSignature, strlen(sFuncSignature) + 1);
		lua_pushcclosure(L, StdCFunction, 1);
	}
	else
	{
		assert(fd.pDataBuffer != NULL && fd.nDataSize > 0);
		UserDataFunction function = fd.pUserDataFunc;
		i32 nSize = fd.nDataSize;
		i32 nTotalSize = sizeof(function) + sizeof(i32) + nSize + strlen(sFuncSignature) + 1 + 1;
		// Store functor in first upvalue.
		u8* pBuffer = (u8*)lua_newuserdata(L, nTotalSize);
		memcpy(pBuffer, &function, sizeof(function));
		memcpy(pBuffer + sizeof(function), &nSize, sizeof(nSize));
		memcpy(pBuffer + sizeof(function) + sizeof(nSize), fd.pDataBuffer, nSize);
		memcpy(pBuffer + sizeof(function) + sizeof(nSize) + nSize, &nParamIdOffset, 1);
		memcpy(pBuffer + sizeof(function) + sizeof(nSize) + nSize + 1, sFuncSignature, strlen(sFuncSignature) + 1);
		lua_pushcclosure(L, StdCUserDataFunction, 1);
	}

	lua_rawset(L, -3);
	lua_pop(L, 1); // pop table.
	return true;
}

//////////////////////////////////////////////////////////////////////////
uk CScriptTable::operator new(size_t nSize)
{
	uk ptr = m_pSS->Allocate(nSize);
	if (ptr)
	{
		memset(ptr, 0, nSize); // Clear objects memory.
	}
	return ptr;
}
//////////////////////////////////////////////////////////////////////////
void CScriptTable::operator delete(uk ptr)
{
	if (ptr)
	{
		m_pSS->Deallocate(ptr);
	}
}
