// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct IScriptObject;

//////////////////////////////////////////////////////////////////////////
// This class handles assignment of entity script properties from XML nodes
// to the script tables.
//////////////////////////////////////////////////////////////////////////
class CScriptProperties
{
public:
	bool SetProperties(XmlNodeRef& entityNode, IScriptTable* pEntityTable);
	void Assign(XmlNodeRef& propsNode, IScriptTable* pPropsTable);
};
