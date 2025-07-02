// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERIALIZEXMLWRITER_H__
#define __SERIALIZEXMLWRITER_H__

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/Sys/IValidator.h>
#include <drx3D/Network/SimpleSerialize.h>

class CSerializeXMLWriterImpl : public CSimpleSerializeImpl<false, eST_SaveGame>
{
public:
	CSerializeXMLWriterImpl(const XmlNodeRef& nodeRef);
	~CSerializeXMLWriterImpl();

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

	void BeginGroup(tukk szName);
	bool BeginOptionalGroup(tukk szName, bool condition);
	void EndGroup();

	void GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	//////////////////////////////////////////////////////////////////////////
	// Vars.
	//////////////////////////////////////////////////////////////////////////
	CTimeValue                 m_curTime;

	std::vector<XmlNodeRef>    m_nodeStack;
	//bool m_bCheckEntityOnScript;
	std::vector<IScriptTable*> m_savedTables;
	std::vector<tukk >   m_luaSaveStack;
	//////////////////////////////////////////////////////////////////////////

	ILINE const XmlNodeRef& CurNode()
	{
		assert(!m_nodeStack.empty());
		if (m_nodeStack.empty())
		{
			static XmlNodeRef temp = GetISystem()->CreateXmlNode("Error");
			return temp;
		}
		return m_nodeStack.back();
	}

	XmlNodeRef CreateNodeNamed(tukk name);

	template<class T>
	void AddValue(tukk name, const T& value)
	{
		if (strchr(name, ' ') != 0)
		{
			assert(0 && "Spaces in Value name not supported");
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Spaces in Value name not supported: %s in Group %s", name, GetStackInfo());
			return;
		}
		if (GetISystem()->IsDevMode() && CurNode())
		{
			// Check if this attribute already added.
			if (CurNode()->haveAttr(name))
			{
				assert(0);
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "!Duplicate tag Value( \"%s\" ) in Group %s", name, GetStackInfo());
			}
		}

		if (!IsDefaultValue(value))
		{
			CurNode()->setAttr(name, value);
		}
	}
	void AddValue(tukk name, const SSerializeString& value)
	{
		AddValue(name, value.c_str());
	}
	void AddValue(tukk name, const SNetObjectID& value)
	{
		assert(false);
	}
	template<class T>
	void AddTypedValue(tukk name, const T& value, tukk type)
	{
		if (!IsDefaultValue(value))
		{
			XmlNodeRef newNode = CreateNodeNamed(name);
			newNode->setAttr("v", value);
			newNode->setAttr("t", type);
		}
	}

	void        WriteTable(XmlNodeRef addTo, SmartScriptTable tbl, bool bCheckEntityOnScript);
	void        ScriptValue(XmlNodeRef addTo, tukk tag, tukk name, const ScriptAnyValue& value, bool bCheckEntityOnScript);
	// Used for printing currebnt stack info for warnings.
	tukk GetStackInfo() const;
	tukk GetLuaStackInfo() const;

	static bool ShouldSkipValue(tukk name, const ScriptAnyValue& value);
	static bool IsVector(SmartScriptTable tbl);
	bool        IsEntity(SmartScriptTable tbl, EntityId& entityId);

	//////////////////////////////////////////////////////////////////////////
	// Check For Defaults.
	//////////////////////////////////////////////////////////////////////////
	bool IsDefaultValue(bool v) const                      { return v == false; };
	bool IsDefaultValue(float v) const                     { return v == 0; };
	bool IsDefaultValue(int8 v) const                      { return v == 0; };
	bool IsDefaultValue(u8 v) const                     { return v == 0; };
	bool IsDefaultValue(i16 v) const                     { return v == 0; };
	bool IsDefaultValue(u16 v) const                    { return v == 0; };
	bool IsDefaultValue(i32 v) const                     { return v == 0; };
	bool IsDefaultValue(u32 v) const                    { return v == 0; };
	bool IsDefaultValue(int64 v) const                     { return v == 0; };
	bool IsDefaultValue(uint64 v) const                    { return v == 0; };
	bool IsDefaultValue(const Vec2& v) const               { return v.x == 0 && v.y == 0; };
	bool IsDefaultValue(const Vec3& v) const               { return v.x == 0 && v.y == 0 && v.z == 0; };
	bool IsDefaultValue(const Ang3& v) const               { return v.x == 0 && v.y == 0 && v.z == 0; };
	bool IsDefaultValue(const Quat& v) const               { return v.w == 1.0f && v.v.x == 0 && v.v.y == 0 && v.v.z == 0; };
	bool IsDefaultValue(const ScriptAnyValue& v) const     { return false; };
	bool IsDefaultValue(const CTimeValue& v) const         { return v.GetValue() == 0; };
	bool IsDefaultValue(tukk str) const             { return !str || !*str; };
	bool IsDefaultValue(const string& str) const           { return str.empty(); };
	bool IsDefaultValue(const SSerializeString& str) const { return str.empty(); };
	//////////////////////////////////////////////////////////////////////////

	/*

	   template <class T>
	   bool IsDefaultValue( const T& v ) const { return false; };
	 */
};

#endif
