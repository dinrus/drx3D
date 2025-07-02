// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "VariablesModel.h"

#include <QString>

namespace Schematyc {

struct IScriptVariable;

}

namespace DrxSchematycEditor {

class CStateItem;
class CObjectModel;

class CVariableItem : public CAbstractVariablesModelItem
{
	enum EOwner
	{
		State,
		Object,
	};

public:
	//CVariableItem(Schematyc::IScriptVariable& scriptVariable, CStateItem& state);
	CVariableItem(Schematyc::IScriptVariable& scriptVariable, CObjectModel& object);
	~CVariableItem();

	// CAbstractVariableItem
	virtual QString GetName() const override;
	virtual void    SetName(QString name) override;

	virtual void    Serialize(Serialization::IArchive& archive) override;
	// ~CAbstractVariableItem

	DrxGUID GetGUID() const;

private:
	Schematyc::IScriptVariable& m_scriptVariable;

	EOwner                      m_owner;
};

}

