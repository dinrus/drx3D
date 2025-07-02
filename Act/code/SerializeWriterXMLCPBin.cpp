// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/SerializeWriterXMLCPBin.h>
#include <drx3D/Entity/IEntitySystem.h>

static const size_t MAX_NODE_STACK_DEP = 40;

#define TAG_SCRIPT_VALUE "v"
#define TAG_SCRIPT_TYPE  "t"
#define TAG_SCRIPT_NAME  "n"

CSerializeWriterXMLCPBin::CSerializeWriterXMLCPBin(const XMLCPB::CNodeLiveWriterRef& nodeRef, XMLCPB::CWriterInterface& binWriter)
	: m_binWriter(binWriter)
{
	assert(nodeRef.IsValid());
	m_curTime = gEnv->pTimer->GetFrameStartTime();
	m_nodeStack.reserve(MAX_NODE_STACK_DEP);
	m_nodeStack.push_back(nodeRef);
	m_luaSaveStack.reserve(10);
}

//////////////////////////////////////////////////////////////////////////
CSerializeWriterXMLCPBin::~CSerializeWriterXMLCPBin()
{
	if (m_nodeStack.size() != 1)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!BeginGroup/EndGroup mismatch in SaveGame");
	}
}

//////////////////////////////////////////////////////////////////////////
bool CSerializeWriterXMLCPBin::Value(tukk name, CTimeValue value)
{
	if (value == CTimeValue(0.0f))
		AddValue(name, "zero");
	else
		AddValue(name, (value - m_curTime).GetSeconds());
	return true;
}

bool CSerializeWriterXMLCPBin::Value(tukk name, ScriptAnyValue& value)
{
	m_luaSaveStack.resize(0);
	ScriptValue(CurNode(), NULL, name, value, true);
	return true;
}

void CSerializeWriterXMLCPBin::RecursiveAddXmlNodeRef(XMLCPB::CNodeLiveWriterRef BNode, XmlNodeRef xmlNode)
{
	XMLCPB::CNodeLiveWriterRef BChild = BNode->AddChildNode(xmlNode->getTag());
	for (i32 i = 0; i < xmlNode->getNumAttributes(); ++i)
	{
		tukk pKey = NULL;
		tukk pVal = NULL;
		bool ok = xmlNode->getAttributeByIndex(i, &pKey, &pVal);
		assert(ok);
		BChild->AddAttr(pKey, pVal);
	}

	for (i32 i = 0; i < xmlNode->getChildCount(); ++i)
	{
		RecursiveAddXmlNodeRef(BChild, xmlNode->getChild(i));
	}
}

bool CSerializeWriterXMLCPBin::Value(tukk name, XmlNodeRef& value)
{
	if (!value)
		return false;

	if (value->getNumAttributes() == 0 && value->getChildCount() == 0)
		return false;

	XMLCPB::CNodeLiveWriterRef BNode = CurNode()->AddChildNode(name);
	RecursiveAddXmlNodeRef(BNode, value);
	return true;
}

bool CSerializeWriterXMLCPBin::ValueByteArray(tukk name, u8k* data, u32 len)
{
	XMLCPB::CNodeLiveWriterRef& curNode = CurNode();

#ifndef _RELEASE
	if (GetISystem()->IsDevMode() && curNode.IsValid())
	{
		if (curNode->HaveAttr(name))
		{
			assert(0);
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Duplicate tag Value( \"%s\" ) in Group %s", name, GetStackInfo());
		}
	}
#endif

	if (!IsDefaultValue(data, len))
	{
		curNode->AddAttr(name, data, len);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

void CSerializeWriterXMLCPBin::ScriptValue(XMLCPB::CNodeLiveWriterRef addTo, tukk tag, tukk name, const ScriptAnyValue& value, bool bCheckEntityOnScript)
{
	CHECK_SCRIPT_STACK;

	bool bShouldAdd = true;

	m_luaSaveStack.push_back(name);

	assert(tag || name);
	XMLCPB::CNodeLiveWriterRef node = addTo->AddChildNode(tag ? tag : name);
	if (tag && name)
		node->AddAttr(TAG_SCRIPT_NAME, name);

	switch (value.GetType())
	{
	case EScriptAnyType::Nil:
		node->AddAttr(TAG_SCRIPT_TYPE, "nil");
		break;
	case EScriptAnyType::Boolean:
		node->AddAttr(TAG_SCRIPT_VALUE, value.GetBool());
		node->AddAttr(TAG_SCRIPT_TYPE, "b");
		break;
	case EScriptAnyType::Handle:
		{
			// We are always writing a uint64 for handles. In x86, this is a bit inefficient. But when dealing
			//	with writing in x86 and reading in x64 (or vice versa), this ensures we don't have to guess which DWORD
			//	to cast from when reading (especially for Endian).
			uint64 v = value.GetScriptHandle().n;
			node->AddAttr(TAG_SCRIPT_VALUE, v);
			node->AddAttr(TAG_SCRIPT_TYPE, "h");
		}
		break;
	case EScriptAnyType::Number:
		node->AddAttr(TAG_SCRIPT_VALUE, value.GetNumber());
		node->AddAttr(TAG_SCRIPT_TYPE, "n");
		break;
	case EScriptAnyType::String:
		node->AddAttr(TAG_SCRIPT_VALUE, value.GetString());
		node->AddAttr(TAG_SCRIPT_TYPE, "s");
		break;
	case EScriptAnyType::Vector:
		node->AddAttr(TAG_SCRIPT_VALUE, value.GetVector());
		node->AddAttr(TAG_SCRIPT_TYPE, "v");
		break;
	case EScriptAnyType::Table:
		if (!value.GetScriptTable())
			node->AddAttr(TAG_SCRIPT_TYPE, "nil");
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

	m_luaSaveStack.pop_back();
}

//////////////////////////////////////////////////////////////////////////
void CSerializeWriterXMLCPBin::BeginGroup(tukk szName)
{
	XMLCPB::CNodeLiveWriterRef node = CurNode()->AddChildNode(szName);
	m_nodeStack.push_back(node);
	if (m_nodeStack.size() > MAX_NODE_STACK_DEP)
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Too Deep Node Stack:\r\n%s", GetStackInfo());
}

bool CSerializeWriterXMLCPBin::BeginOptionalGroup(tukk szName, bool condition)
{
	if (condition)
		BeginGroup(szName);

	return condition;
}

void CSerializeWriterXMLCPBin::EndGroup()
{
	if (m_nodeStack.size() == 1)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Misplaced EndGroup() for BeginGroup(%s)", CurNode()->GetTag());
	}
	CurNode()->Done();   // this call is actually not needed. because the XMLCPBin will already detect when the node can be closed. But it does not hurt to do it explicitely.
	m_nodeStack.pop_back();
	assert(!m_nodeStack.empty());
}

// TODO: need to re-look carefully at all this script management code. Right now it is esentially a copy paste from CSerializeXMLWriter
//////////////////////////////////////////////////////////////////////////

bool CSerializeWriterXMLCPBin::ShouldSkipValue(tukk name, const ScriptAnyValue& value)
{
	CHECK_SCRIPT_STACK;

	if (!name)
		return true;

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

void CSerializeWriterXMLCPBin::WriteTable(XMLCPB::CNodeLiveWriterRef node, SmartScriptTable tbl, bool bCheckEntityOnScript)
{
	CHECK_SCRIPT_STACK;

	EntityId entityId;
	if (bCheckEntityOnScript && IsEntity(tbl, entityId))
	{
		node->AddAttr(TAG_SCRIPT_TYPE, "entityId");
		node->AddAttr(TAG_SCRIPT_VALUE, entityId);
	}
	else if (IsVector(tbl))
	{
		Vec3 value;
		tbl->GetValue("x", value.x);
		tbl->GetValue("y", value.y);
		tbl->GetValue("z", value.z);
		node->AddAttr(TAG_SCRIPT_TYPE, "vec");
		node->AddAttr(TAG_SCRIPT_VALUE, value);
	}
	else
	{
		node->AddAttr(TAG_SCRIPT_TYPE, "table");
		CHECK_SCRIPT_STACK;
		i32 arrayCount = tbl->Count();
		if (arrayCount)
		{
			node->AddAttr("count", arrayCount);
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
bool CSerializeWriterXMLCPBin::IsEntity(SmartScriptTable tbl, EntityId& entityId)
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
		}
	}
	return bEntityTable;
}

//////////////////////////////////////////////////////////////////////////
bool CSerializeWriterXMLCPBin::IsVector(SmartScriptTable tbl)
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
tukk CSerializeWriterXMLCPBin::GetStackInfo() const
{
	static string str;
	str.assign("");
	for (i32 i = 0; i < (i32)m_nodeStack.size(); i++)
	{
		tukk name = m_nodeStack[i]->ReadAttrStr(TAG_SCRIPT_NAME);
		if (name && name[0])
			str += name;
		else
			str += m_nodeStack[i]->GetTag();
		if (i != m_nodeStack.size() - 1)
			str += "/";
	}
	return str.c_str();
}

//////////////////////////////////////////////////////////////////////////
tukk CSerializeWriterXMLCPBin::GetLuaStackInfo() const
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
