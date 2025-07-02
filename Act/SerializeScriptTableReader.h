// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERIALIZESCRIPTTABLEREADER_H__
#define __SERIALIZESCRIPTTABLEREADER_H__

#pragma once

#include <drx3D/Network/SimpleSerialize.h>
#include <stack>

class CSerializeScriptTableReaderImpl : public CSimpleSerializeImpl<true, eST_Script>
{
public:
	CSerializeScriptTableReaderImpl(SmartScriptTable tbl);

	template<class T>
	void Value(tukk name, T& value)
	{
		IScriptTable* pTbl = CurTable();
		if (pTbl)
			if (pTbl->HaveValue(name))
				if (!pTbl->GetValue(name, value))
				{
					Failed();
					GameWarning("Failed to read %s", name);
				}
	}

	void Value(tukk name, EntityId& value);

	void Value(tukk name, SNetObjectID& value)
	{
		DRX_ASSERT(false);
	}

	void Value(tukk name, XmlNodeRef& value)
	{
		DRX_ASSERT(false);
	}

	void Value(tukk name, Quat& value);
	void Value(tukk name, ScriptAnyValue& value);
	void Value(tukk name, SSerializeString& value);
	void Value(tukk name, u16& value);
	void Value(tukk name, uint64& value);
	void Value(tukk name, i16& value);
	void Value(tukk name, int64& value);
	void Value(tukk name, u8& value);
	void Value(tukk name, int8& value);
	void Value(tukk name, Vec2& value);
	void Value(tukk name, CTimeValue& value);

	template<class T, class P>
	void Value(tukk name, T& value, const P& p)
	{
		Value(name, value);
	}

	bool BeginGroup(tukk szName);
	bool BeginOptionalGroup(tukk szName, bool cond) { return false; }
	void EndGroup();

private:
	template<class T, class U>
	void NumValue(tukk name, U& value);

	i32                          m_nSkip;
	std::stack<SmartScriptTable> m_tables;

	IScriptTable* CurTable()
	{
		if (m_nSkip)
			return 0;
		else
			return m_tables.top();
	}
};

#endif
