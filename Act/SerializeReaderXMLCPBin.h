// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERIALIZEXMLREADER_H__
#define __SERIALIZEXMLREADER_H__

#pragma once

#include <drx3D/Network/SimpleSerialize.h>
#include <stack>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/IValidator.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Act/XMLCPB_ReaderInterface.h>

class CSerializeReaderXMLCPBin : public CSimpleSerializeImpl<true, eST_SaveGame>
{
public:
	CSerializeReaderXMLCPBin(XMLCPB::CNodeLiveReaderRef nodeRef, XMLCPB::CReaderInterface& binReader);

	template<class T_Value>
	bool Value(tukk name, T_Value& value)
	{
		DefaultValue(value); // Set input value to default.
		if (m_nErrors)
			return false;

		if (!GetAttr(CurNode(), name, value))
		{
			// the attrs are not saved if they already had the default value
			return false;
		}
		return true;
	}

	bool Value(tukk name, ScriptAnyValue& value);
	bool Value(tukk name, int8& value);
	bool Value(tukk name, string& value);
	bool Value(tukk name, CTimeValue& value);
	bool Value(tukk name, XmlNodeRef& value);
	bool ValueByteArray(tukk name, u8*& rdata, u32& outSize);

	template<class T_Value, class T_Policy>
	bool Value(tukk name, T_Value& value, const T_Policy& policy)
	{
		return Value(name, value);
	}

	void        BeginGroup(tukk szName);
	bool        BeginOptionalGroup(tukk szName, bool condition);
	void        EndGroup();
	tukk GetStackInfo() const;

private:
	template<class T_Value>
	ILINE bool GetAttr(XMLCPB::CNodeLiveReaderRef& node, tukk name, T_Value& value)
	{
		bool found = node->ReadAttr(name, value);
		return found;
	}
	ILINE bool GetAttr(XMLCPB::CNodeLiveReaderRef& node, tukk name, SSerializeString& value)
	{
		tukk pVal = NULL;
		bool found = node->ReadAttr(name, pVal);
		if (found)
			value = pVal;
		return found;
	}
	ILINE bool GetAttr(XMLCPB::CNodeLiveReaderRef& node, tukk name, const string& value)
	{
		assert(false);
		return false;
	}
	ILINE bool GetAttr(XMLCPB::CNodeLiveReaderRef& node, tukk name, SNetObjectID& value)
	{
		assert(false);
		return false;
	}

	void RecursiveReadIntoXmlNodeRef(XMLCPB::CNodeLiveReaderRef BNode, XmlNodeRef& xmlNode);

private:
	//CTimeValue m_curTime;
	XMLCPB::CNodeLiveReaderRef& CurNode() { assert(!m_nodeStack.empty()); return m_nodeStack.back(); }

	i32                                     m_nErrors;
	std::vector<XMLCPB::CNodeLiveReaderRef> m_nodeStack;
	XMLCPB::CReaderInterface&               m_binReader;

	bool ReadScript(XMLCPB::CNodeLiveReaderRef node, ScriptAnyValue& value);

	//////////////////////////////////////////////////////////////////////////
	// Set Defaults.
	//////////////////////////////////////////////////////////////////////////
	void DefaultValue(bool& v) const               { v = false; };
	void DefaultValue(float& v) const              { v = 0; };
	void DefaultValue(int8& v) const               { v = 0; };
	void DefaultValue(u8& v) const              { v = 0; };
	void DefaultValue(i16& v) const              { v = 0; };
	void DefaultValue(u16& v) const             { v = 0; };
	void DefaultValue(i32& v) const              { v = 0; };
	void DefaultValue(u32& v) const             { v = 0; };
	void DefaultValue(int64& v) const              { v = 0; };
	void DefaultValue(uint64& v) const             { v = 0; };
	void DefaultValue(Vec2& v) const               { v.x = 0; v.y = 0; };
	void DefaultValue(Vec3& v) const               { v.x = 0; v.y = 0; v.z = 0; };
	void DefaultValue(Ang3& v) const               { v.x = 0; v.y = 0; v.z = 0; };
	void DefaultValue(Quat& v) const               { v.w = 1.0f; v.v.x = 0; v.v.y = 0; v.v.z = 0; };
	void DefaultValue(ScriptAnyValue& v) const     {};
	void DefaultValue(CTimeValue& v) const         { v.SetValue(0); };
	//void DefaultValue( char *str ) const { if (str) str[0] = 0; };
	void DefaultValue(string& str) const           { str = ""; };
	void DefaultValue(const string& str) const     {};
	void DefaultValue(SNetObjectID& id) const      {};
	void DefaultValue(SSerializeString& str) const {};
	void DefaultValue(u8*& rdata, u32& outSize) const;
	//////////////////////////////////////////////////////////////////////////
};

#endif
