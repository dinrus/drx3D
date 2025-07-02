// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERIALIZESCRIPTTABLEWRITER_H__
#define __SERIALIZESCRIPTTABLEWRITER_H__

#pragma once

#include <drx3D/Network/SimpleSerialize.h>
#include <stack>

class CSerializeScriptTableWriterImpl : public CSimpleSerializeImpl<false, eST_Script>
{
public:
	CSerializeScriptTableWriterImpl(SmartScriptTable tbl = SmartScriptTable());

	template<class T>
	void Value(tukk name, const T& value)
	{
		IScriptTable* pTbl = CurTable();
		pTbl->SetValue(name, value);
	}

	void Value(tukk name, SNetObjectID value)
	{
		DRX_ASSERT(false);
	}

	void Value(tukk name, XmlNodeRef ref)
	{
		DRX_ASSERT(false);
	}

	void Value(tukk name, Vec3 value);
	void Value(tukk name, Vec2 value);
	void Value(tukk name, const SSerializeString& value);
	void Value(tukk name, int64 value);
	void Value(tukk name, Quat value);
	void Value(tukk name, uint64 value);
	void Value(tukk name, CTimeValue value);

	template<class T, class P>
	void Value(tukk name, T& value, const P& p)
	{
		Value(name, value);
	}

	bool BeginGroup(tukk szName);
	bool BeginOptionalGroup(tukk szName, bool cond) { return false; }
	void EndGroup();

private:
	std::stack<SmartScriptTable> m_tables;
	IScriptTable*    CurTable() { return m_tables.top().GetPtr(); }
	SmartScriptTable ReuseTable(tukk name);

	IScriptSystem* m_pSS;
};

#endif
