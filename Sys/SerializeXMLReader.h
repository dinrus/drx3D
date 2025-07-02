// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERIALIZEXMLREADER_H__
#define __SERIALIZEXMLREADER_H__

#pragma once

#include <drx3D/Network/SimpleSerialize.h>
#include <stack>
#include <drx3D/Sys/IXml.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/IValidator.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/xml.h>

class CSerializeXMLReaderImpl : public CSimpleSerializeImpl<true, eST_SaveGame>
{
public:
	CSerializeXMLReaderImpl(const XmlNodeRef& nodeRef);

	template<class T_Value>
	ILINE bool GetAttr(const XmlNodeRef& node, tukk name, T_Value& value)
	{
		XmlStrCmpFunc pPrevCmpFunc = g_pXmlStrCmp;
		g_pXmlStrCmp = &strcmp; // Do case-sensitive compare
		bool bReturn = node->getAttr(name, value);
		g_pXmlStrCmp = pPrevCmpFunc;
		return bReturn;
	}
	ILINE bool GetAttr(const XmlNodeRef& node, tukk name, SSerializeString& value)
	{
		XmlStrCmpFunc pPrevCmpFunc = g_pXmlStrCmp;
		g_pXmlStrCmp = &strcmp; // Do case-sensitive compare
		bool bReturn = node->haveAttr(name);
		if (bReturn)
		{
			value = node->getAttr(name);
		}
		g_pXmlStrCmp = pPrevCmpFunc;
		return bReturn;
	}
	ILINE bool GetAttr(XmlNodeRef& node, tukk name, const string& value)
	{
		return false;
	}
	ILINE bool GetAttr(const XmlNodeRef& node, tukk name, SNetObjectID& value)
	{
		return false;
	}

	template<class T_Value>
	bool Value(tukk name, T_Value& value)
	{
		DefaultValue(value); // Set input value to default.
		if (m_nErrors)
			return false;

		if (!GetAttr(CurNode(), name, value))
		{
			//DrxWarning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,"Unable to read attribute %s (invalid type?)", name);
			//Failed();
			return false;
		}
		return true;
	}

	bool Value(tukk name, ScriptAnyValue& value);
	bool Value(tukk name, int8& value);
	bool Value(tukk name, string& value);
	bool Value(tukk name, CTimeValue& value);
	bool Value(tukk name, XmlNodeRef& value);

	template<class T_Value, class T_Policy>
	bool Value(tukk name, T_Value& value, const T_Policy& policy)
	{
		return Value(name, value);
	}

	void        BeginGroup(tukk szName);
	bool        BeginOptionalGroup(tukk szName, bool condition);
	void        EndGroup();
	tukk GetStackInfo() const;

	void        GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	//CTimeValue m_curTime;
	XmlNodeRef CurNode() { return m_nodeStack.back().m_node; }
	XmlNodeRef NextOf(tukk name)
	{
		XmlStrCmpFunc pPrevCmpFunc = g_pXmlStrCmp;
		g_pXmlStrCmp = &strcmp; // Do case-sensitive compare
		assert(!m_nodeStack.empty());
		CParseState& ps = m_nodeStack.back();
		XmlNodeRef node = ps.GetNext(name);
		g_pXmlStrCmp = pPrevCmpFunc;
		return node;
	}

	class CParseState
	{
	public:
		CParseState() : m_nCurrent(0) {}
		void Init(const XmlNodeRef& node)
		{
			m_node = node;
			m_nCurrent = 0;
		}

		XmlNodeRef GetNext(tukk name)
		{
			i32 i;
			i32 num = m_node->getChildCount();
			for (i = m_nCurrent; i < num; i++)
			{
				XmlNodeRef child = m_node->getChild(i);
				if (strcmp(child->getTag(), name) == 0)
				{
					m_nCurrent = i + 1;
					return child;
				}
			}
			i32 ncount = min(m_nCurrent, num);
			// Try searching from begining.
			for (i = 0; i < ncount; i++)
			{
				XmlNodeRef child = m_node->getChild(i);
				if (strcmp(child->getTag(), name) == 0)
				{
					m_nCurrent = i + 1;
					return child;
				}
			}
			return XmlNodeRef();
		}

	public:
		// TODO: make this much more efficient
		i32        m_nCurrent;
		XmlNodeRef m_node;
	};

	i32                      m_nErrors;
	std::vector<CParseState> m_nodeStack;

	bool ReadScript(XmlNodeRef node, ScriptAnyValue& value);

	//////////////////////////////////////////////////////////////////////////
	// Set Defaults.
	//////////////////////////////////////////////////////////////////////////
	void DefaultValue(bool& v) const               { v = false; }
	void DefaultValue(float& v) const              { v = 0; }
	void DefaultValue(int8& v) const               { v = 0; }
	void DefaultValue(u8& v) const              { v = 0; }
	void DefaultValue(i16& v) const              { v = 0; }
	void DefaultValue(u16& v) const             { v = 0; }
	void DefaultValue(i32& v) const              { v = 0; }
	void DefaultValue(u32& v) const             { v = 0; }
	void DefaultValue(int64& v) const              { v = 0; }
	void DefaultValue(uint64& v) const             { v = 0; }
	void DefaultValue(Vec2& v) const               { v.x = 0; v.y = 0; }
	void DefaultValue(Vec3& v) const               { v.x = 0; v.y = 0; v.z = 0; }
	void DefaultValue(Ang3& v) const               { v.x = 0; v.y = 0; v.z = 0; }
	void DefaultValue(Quat& v) const               { v.w = 1.0f; v.v.x = 0; v.v.y = 0; v.v.z = 0; }
	void DefaultValue(ScriptAnyValue& v) const     {}
	void DefaultValue(CTimeValue& v) const         { v.SetValue(0); }
	//void DefaultValue( char *str ) const { if (str) str[0] = 0; }
	void DefaultValue(string& str) const           { str = ""; }
	void DefaultValue(const string& str) const     {}
	void DefaultValue(SNetObjectID& id) const      {}
	void DefaultValue(SSerializeString& str) const {}
	void DefaultValue(XmlNodeRef& ref) const       { ref = NULL; }
	//////////////////////////////////////////////////////////////////////////
};

#endif
