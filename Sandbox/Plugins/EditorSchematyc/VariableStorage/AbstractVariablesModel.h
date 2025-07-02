// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AbstractVariableTypesModel.h"

class CVariableItem
{
public:
	CVariableItem();
	~CVariableItem();

	QString GetName() const { return m_name; }
	void    SetName(QString name);
	//CTypeItem& GetTypeItem() const;

	bool IsConst() const  { m_isConst; }
	bool IsLocal() const  { m_isLocal; }
	bool IsGlobal() const { m_isGlobal; }

	void Serialize(Serialization::IArchive& archive);

private:
	QString m_name;

	bool    m_isConst  : 1;
	bool    m_isLocal  : 1;
	bool    m_isGlobal : 1;
};

class CAbstractVariablesViewModel
{
public:
	CAbstractVariablesViewModel() {}
	virtual ~CAbstractVariablesViewModel() {}

	virtual u32         GetNumVariables() const              { return 0; }
	virtual CVariableItem* GetVariableItemByIndex(u32 index) { return nullptr; }
	virtual CVariableItem* CreateVariable()                     { return nullptr; }
	virtual bool           RemoveVariable()                     { return false; }
};

