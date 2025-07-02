// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERIALIZEWRITERXMLCPBIN_H__
#define __SERIALIZEWRITERXMLCPBIN_H__

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/IValidator.h>
#include <drx3D/Network/SimpleSerialize.h>
#include "XMLCPB_WriterInterface.h"

class CSerializeWriterXMLCPBin : public CSimpleSerializeImpl<false, eST_SaveGame>
{
public:
	CSerializeWriterXMLCPBin(const XMLCPB::CNodeLiveWriterRef& nodeRef, XMLCPB::CWriterInterface& binWriter);
	~CSerializeWriterXMLCPBin();

	template<class T_Value>
	bool Value(tukk name, T_Value& value)
	{
		AddValue(name, value);
		return true;
	}

	template<class T_Value, class T_Policy>
	bool Value(tukk name, T_Value& value, const T_Policy& policy)
	{
		return Value(name, value);
	}

	bool Value(tukk name, CTimeValue value);
	bool Value(tukk name, ScriptAnyValue& value);
	bool Value(tukk name, XmlNodeRef& value);
	bool ValueByteArray(tukk name, u8k* data, u32 len);

	void BeginGroup(tukk szName);
	bool BeginOptionalGroup(tukk szName, bool condition);
	void EndGroup();

private:
	CTimeValue                              m_curTime;
	std::vector<XMLCPB::CNodeLiveWriterRef> m_nodeStack;  // TODO: look to get rid of this. it should be useless, because can access all necesary data from the XMLCPBin object
	std::vector<IScriptTable*>              m_savedTables;
	std::vector<tukk >                m_luaSaveStack;
	XMLCPB::CWriterInterface&               m_binWriter;

	void RecursiveAddXmlNodeRef(XMLCPB::CNodeLiveWriterRef BNode, XmlNodeRef xmlNode);

	//////////////////////////////////////////////////////////////////////////
	ILINE XMLCPB::CNodeLiveWriterRef& CurNode()
	{
		assert(!m_nodeStack.empty());
		if (m_nodeStack.empty())
		{
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "CSerializeWriterXMLCPBin: !Trying to access a node from the nodeStack, but the stack is empty. Savegame will be corrupted");
			static XMLCPB::CNodeLiveWriterRef temp = m_binWriter.GetRoot()->AddChildNode("Error");
			return temp;
		}
		return m_nodeStack.back();
	}

	//////////////////////////////////////////////////////////////////////////
	template<class T>
	void AddValue(tukk name, const T& value)
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

		if (!IsDefaultValue(value))
		{
			curNode->AddAttr(name, value);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void AddValue(tukk name, const SSerializeString& value)
	{
		AddValue(name, value.c_str());
	}

	void AddValue(tukk name, const SNetObjectID& value)
	{
		assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	template<class T>
	void AddTypedValue(tukk name, const T& value, tukk type)
	{
		assert(false);    // not needed for savegames, apparently
		//		if (!IsDefaultValue(value))
		//		{
		//			XMLCPB::CNodeLiveWriterRef newNode = CreateNodeNamed(name);
		//			newNode->AddAttr("v",value);
		//			newNode->AddAttr("t",type);
		//		}
	}

	void WriteTable(XMLCPB::CNodeLiveWriterRef addTo, SmartScriptTable tbl, bool bCheckEntityOnScript);
	void ScriptValue(XMLCPB::CNodeLiveWriterRef addTo, tukk tag, tukk name, const ScriptAnyValue& value, bool bCheckEntityOnScript);

	// Used for printing current stack info for warnings.
	tukk GetStackInfo() const;
	tukk GetLuaStackInfo() const;

	static bool ShouldSkipValue(tukk name, const ScriptAnyValue& value);
	static bool IsVector(SmartScriptTable tbl);
	bool        IsEntity(SmartScriptTable tbl, EntityId& entityId);

	//////////////////////////////////////////////////////////////////////////
	// Check For Defaults.
	//////////////////////////////////////////////////////////////////////////
	bool IsDefaultValue(bool v) const                        { return v == false; };
	bool IsDefaultValue(float v) const                       { return v == 0; };
	bool IsDefaultValue(int8 v) const                        { return v == 0; };
	bool IsDefaultValue(u8 v) const                       { return v == 0; };
	bool IsDefaultValue(i16 v) const                       { return v == 0; };
	bool IsDefaultValue(u16 v) const                      { return v == 0; };
	bool IsDefaultValue(i32 v) const                       { return v == 0; };
	bool IsDefaultValue(u32 v) const                      { return v == 0; };
	bool IsDefaultValue(int64 v) const                       { return v == 0; };
	bool IsDefaultValue(uint64 v) const                      { return v == 0; };
	bool IsDefaultValue(const Vec2& v) const                 { return v.x == 0 && v.y == 0; };
	bool IsDefaultValue(const Vec3& v) const                 { return v.x == 0 && v.y == 0 && v.z == 0; };
	bool IsDefaultValue(const Ang3& v) const                 { return v.x == 0 && v.y == 0 && v.z == 0; };
	bool IsDefaultValue(const Quat& v) const                 { return v.w == 1.0f && v.v.x == 0 && v.v.y == 0 && v.v.z == 0; };
	bool IsDefaultValue(const ScriptAnyValue& v) const       { return false; };
	bool IsDefaultValue(const CTimeValue& v) const           { return v.GetValue() == 0; };
	bool IsDefaultValue(tukk str) const               { return !str || !*str; };
	bool IsDefaultValue(const string& str) const             { return str.empty(); };
	bool IsDefaultValue(const SSerializeString& str) const   { return str.empty(); };
	bool IsDefaultValue(u8k* data, u32 len) const { return (len <= 0); };
	//////////////////////////////////////////////////////////////////////////

};

#endif
