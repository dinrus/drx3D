// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/SerializeXMLWriter.h>
#include <drx3D/Entity/IEntitySystem.h>

static const size_t MAX_NODE_STACK_DEPTH = 40;

#define TAG_SCRIPT_VALUE "v"
#define TAG_SCRIPT_TYPE  "t"
#define TAG_SCRIPT_NAME  "n"

CSerializeXMLWriterImpl::CSerializeXMLWriterImpl(const XmlNodeRef& nodeRef)
{
	m_curTime = gEnv->pTimer->GetFrameStartTime();
	//	m_bCheckEntityOnScript = false;
	assert(!!nodeRef);
	m_nodeStack.push_back(nodeRef);

	m_luaSaveStack.reserve(10);
}

//////////////////////////////////////////////////////////////////////////
CSerializeXMLWriterImpl::~CSerializeXMLWriterImpl()
{
	if (m_nodeStack.size() != 1)
	{
		// Node stack is incorrect.
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!BeginGroup/EndGroup mismatch in SaveGame");
	}
}

//////////////////////////////////////////////////////////////////////////
bool CSerializeXMLWriterImpl::Value(tukk name, CTimeValue value)
{
	if (value == CTimeValue(0.0f))
		AddValue(name, "zero");
	else
		AddValue(name, (value - m_curTime).GetSeconds());
	return true;
}

bool CSerializeXMLWriterImpl::Value(tukk name, ScriptAnyValue& value)
{
	m_luaSaveStack.resize(0);
	ScriptValue(CurNode(), NULL, name, value, true);
	return true;
}

bool CSerializeXMLWriterImpl::Value(tukk name, XmlNodeRef& value)
{
	if (BeginOptionalGroup(name, value != NULL))
	{
		CurNode()->addChild(value);
		EndGroup();
	}
	return true;
}

void CSerializeXMLWriterImpl::ScriptValue(XmlNodeRef addTo, tukk tag, tukk name, const ScriptAnyValue& value, bool bCheckEntityOnScript)
{
	CHECK_SCRIPT_STACK;

	//bool bCheckEntityOnScript = m_bCheckEntityOnScript;
	//m_bCheckEntityOnScript = true;
	bool bShouldAdd = true;

	m_luaSaveStack.push_back(name);

	assert(tag || name);
	XmlNodeRef node = addTo->createNode(tag ? tag : name);
	if (tag && name)
		node->setAttr(TAG_SCRIPT_NAME, name);

	switch (value.GetType())
	{
	case EScriptAnyType::Nil:
		node->setAttr(TAG_SCRIPT_TYPE, "nil");
		break;
	case EScriptAnyType::Boolean:
		node->setAttr(TAG_SCRIPT_VALUE, value.GetBool());
		node->setAttr(TAG_SCRIPT_TYPE, "b");
		break;
	case EScriptAnyType::Handle:
		{
			// We are always writing a uint64 for handles. In x86, this is a bit inefficient. But when dealing
			//	with writing in x86 and reading in x64 (or vice versa), this ensures we don't have to guess which DWORD
			//	to cast from when reading (especially for Endian).
			node->setAttr(TAG_SCRIPT_VALUE, (uint64)(UINT_PTR)value.GetScriptHandle().ptr);
			node->setAttr(TAG_SCRIPT_TYPE, "h");
		}
		break;
	case EScriptAnyType::Number:
		node->setAttr(TAG_SCRIPT_VALUE, value.GetNumber());
		node->setAttr(TAG_SCRIPT_TYPE, "n");
		break;
	case EScriptAnyType::String:
		node->setAttr(TAG_SCRIPT_VALUE, value.GetString());
		node->setAttr(TAG_SCRIPT_TYPE, "s");
		break;
	case EScriptAnyType::Vector:
		node->setAttr(TAG_SCRIPT_VALUE, value.GetVector());
		node->setAttr(TAG_SCRIPT_TYPE, "v");
		break;
	case EScriptAnyType::Table:
		if (!value.GetScriptTable())
			node->setAttr(TAG_SCRIPT_TYPE, "nil");
		else
		{
			// Check for recursion.
			if (std::find(m_savedTables.begin(), m_savedTables.end(), value.GetScriptTable()) == m_savedTables.end())
			{
				m_savedTables.push_back(value.GetScriptTable());
				WriteTable(node, value.GetScriptTable(), true);
				m_savedTables.pop_back();
			}
			else
			{
				// Trying to write table recursively.
				assert(0 && "Writing script table recursively");
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Writing script table recursively: %s", GetStackInfo());
				bShouldAdd = false;
			}
		}
		break;
	default:
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Unhandled script-any-type %s of type:%d in Group %s", name, value.GetType(), GetStackInfo());
		Failed();
		bShouldAdd = false;
	}

	if (bShouldAdd)
		addTo->addChild(node);

	m_luaSaveStack.pop_back();

	//	m_bCheckEntityOnScript = bCheckEntityOnScript;
}

void CSerializeXMLWriterImpl::BeginGroup(tukk szName)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "XML");
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "BeginGroup");
	if (strchr(szName, ' ') != 0)
	{
		assert(0 && "Spaces in group name not supported");
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Spaces in group name not supported: %s/%s", GetStackInfo(), szName);
	}
	XmlNodeRef node = CreateNodeNamed(szName);
	CurNode()->addChild(node);
	m_nodeStack.push_back(node);
	if (m_nodeStack.size() > MAX_NODE_STACK_DEPTH)
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Too Deep Node Stack:\r\n%s", GetStackInfo());
}

bool CSerializeXMLWriterImpl::BeginOptionalGroup(tukk szName, bool condition)
{
	if (condition)
	{
		BeginGroup(szName);
		return true;
	}

	return condition;
}

XmlNodeRef CSerializeXMLWriterImpl::CreateNodeNamed(tukk name)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "XML");
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "CreateNodeNamed");
	XmlNodeRef newNode = CurNode()->createNode(name);
	return newNode;
}

void CSerializeXMLWriterImpl::EndGroup()
{
	if (m_nodeStack.size() == 1)
	{
		//
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Misplaced EndGroup() for BeginGroup(%s)", CurNode()->getTag());
	}
	assert(!m_nodeStack.empty());
	m_nodeStack.pop_back();
	assert(!m_nodeStack.empty());
}

void CSerializeXMLWriterImpl::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
	pSizer->AddObject(m_nodeStack);
	pSizer->AddContainer(m_savedTables); //TODO: can we track memory used by the tables?
	pSizer->AddContainer(m_luaSaveStack);
}

bool CSerializeXMLWriterImpl::ShouldSkipValue(tukk name, const ScriptAnyValue& value)
{
	CHECK_SCRIPT_STACK;

	if (!name)
		return true;

#if 0
	static tukk skipNames[] = {
	};
	for (i32 i = 0; i < sizeof(skipNames) / sizeof(*skipNames); ++i)
		if (0 == strcmp(skipNames[i], name))
			return true;
#endif
	if (strlen(name) >= 2)
		if (name[0] == '_' && name[1] == '_')
			return true;
	if (!strlen(name))
		return true;

	switch (value.GetType())
	{
	case EScriptAnyType::Boolean:
	case EScriptAnyType::Handle:
	case EScriptAnyType::Nil:
	case EScriptAnyType::Number:
	case EScriptAnyType::String:
	case EScriptAnyType::Vector:
		return false;
	case EScriptAnyType::Table:
		{
			ScriptAnyValue temp;
			value.GetScriptTable()->GetValueAny("__nopersist", temp);
			return temp.GetType() != EScriptAnyType::Nil;
		}
	default:
		return true;
	}
}

void CSerializeXMLWriterImpl::WriteTable(XmlNodeRef node, SmartScriptTable tbl, bool bCheckEntityOnScript)
{
	CHECK_SCRIPT_STACK;

	EntityId entityId;
	if (bCheckEntityOnScript && IsEntity(tbl, entityId))
	{
		node->setAttr(TAG_SCRIPT_TYPE, "entityId");
		node->setAttr(TAG_SCRIPT_VALUE, entityId);
	}
	else if (IsVector(tbl))
	{
		Vec3 value;
		tbl->GetValue("x", value.x);
		tbl->GetValue("y", value.y);
		tbl->GetValue("z", value.z);
		node->setAttr(TAG_SCRIPT_TYPE, "vec");
		node->setAttr(TAG_SCRIPT_VALUE, value);
	}
	else
	{
		node->setAttr(TAG_SCRIPT_TYPE, "table");
		CHECK_SCRIPT_STACK;
		i32 arrayCount = tbl->Count();
		if (arrayCount)
		{
			node->setAttr("count", arrayCount);
			CHECK_SCRIPT_STACK;
			for (i32 i = 1; i <= arrayCount; i++)
			{
				ScriptAnyValue tempValue;
				tbl->GetAtAny(i, tempValue);
				if (tempValue.GetType() != EScriptAnyType::Function && tempValue.GetType() != EScriptAnyType::UserData && tempValue.GetType() != EScriptAnyType::Any)
					ScriptValue(node, "i", NULL, tempValue, true);
			}
		}
		else
		{
			CHECK_SCRIPT_STACK;
			IScriptTable::Iterator iter = tbl->BeginIteration();
			while (tbl->MoveNext(iter))
			{
				if (iter.sKey)
				{
					ScriptAnyValue tempValue;
					tbl->GetValueAny(iter.sKey, tempValue);
					if (ShouldSkipValue(iter.sKey, tempValue))
						continue;
					if (tempValue.GetType() != EScriptAnyType::Function && tempValue.GetType() != EScriptAnyType::UserData && tempValue.GetType() != EScriptAnyType::Any)
						ScriptValue(node, "te", iter.sKey, tempValue, true);
				}
			}
			tbl->EndIteration(iter);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CSerializeXMLWriterImpl::IsEntity(SmartScriptTable tbl, EntityId& entityId)
{
	CHECK_SCRIPT_STACK;

	bool bEntityTable = false;
	ScriptHandle hdlId, hdlThis;
	if (tbl->GetValue("id", hdlId))
	{
		if (hdlId.n && tbl->GetValue("__this", hdlThis))
		{
			bEntityTable = true;
			if ((EntityId)(TRUNCATE_PTR)hdlThis.ptr == (EntityId)(TRUNCATE_PTR)gEnv->pEntitySystem->GetEntity((EntityId)hdlId.n))
			{
				entityId = (EntityId)hdlId.n;
			}
			/* kirill - no warning needed here
			      else
			      {
			        DrxWarning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,"!Trying to save deleted entity reference: %s\r\nLua: %s", GetStackInfo(),GetLuaStackInfo() );
			      }
			 */
		}
	}
	return bEntityTable;
}

//////////////////////////////////////////////////////////////////////////
bool CSerializeXMLWriterImpl::IsVector(SmartScriptTable tbl)
{
	CHECK_SCRIPT_STACK;

	if (tbl->Count())
		return false;

	bool have[3] = { false, false, false };

	IScriptTable::Iterator iter = tbl->BeginIteration();
	while (tbl->MoveNext(iter))
	{
		if (iter.sKey)
		{
			if (strlen(iter.sKey) > 1)
			{
				tbl->EndIteration(iter);
				return false;
			}
			char elem = iter.sKey[0];
			if (elem < 'x' || elem > 'z')
			{
				tbl->EndIteration(iter);
				return false;
			}
			have[elem - 'x'] = true;
		}
	}
	tbl->EndIteration(iter);
	return have[0] && have[1] && have[2];
}

//////////////////////////////////////////////////////////////////////////
tukk CSerializeXMLWriterImpl::GetStackInfo() const
{
	static string str;
	str.assign("");
	for (i32 i = 0; i < (i32)m_nodeStack.size(); i++)
	{
		tukk name = m_nodeStack[i]->getAttr(TAG_SCRIPT_NAME);
		if (name && name[0])
			str += name;
		else
			str += m_nodeStack[i]->getTag();
		if (i != m_nodeStack.size() - 1)
			str += "/";
	}
	return str.c_str();
}

//////////////////////////////////////////////////////////////////////////
tukk CSerializeXMLWriterImpl::GetLuaStackInfo() const
{
	static string str;
	str.assign("");
	for (i32 i = 0; i < (i32)m_luaSaveStack.size(); i++)
	{
		tukk name = m_luaSaveStack[i];
		str += name;
		if (i != m_luaSaveStack.size() - 1)
			str += ".";
	}
	return str.c_str();
}
