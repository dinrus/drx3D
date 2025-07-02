// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

namespace DrxSchematycEditor {

class CAbstractVariablesModel;

class CAbstractVariablesModelItem
{
public:
	CAbstractVariablesModelItem(CAbstractVariablesModel& model)
		: m_model(model)
	{}
	~CAbstractVariablesModelItem() {}

	CAbstractVariablesModel& GetModel() const { return m_model; }

	virtual QString          GetName() const = 0;
	virtual void             SetName(QString name) = 0;

	virtual u32           GetIndex() const;

	virtual void             Serialize(Serialization::IArchive& archive) {}

public:
	CDrxSignal<void(CAbstractVariablesModelItem&)> SignalNameChanged;

private:
	CAbstractVariablesModel& m_model;
};

class CAbstractVariablesModel
{
public:
	CAbstractVariablesModel() {}
	virtual ~CAbstractVariablesModel() {}

	virtual u32                       GetNumVariables() const                                                     { return 0; }
	virtual CAbstractVariablesModelItem* GetVariableItemByIndex(u32 index)                                        { return nullptr; }
	virtual u32                       GetVariableItemIndex(const CAbstractVariablesModelItem& variableItem) const { return 0xffffffff; }
	virtual CAbstractVariablesModelItem* CreateVariable()                                                            { return nullptr; }
	virtual bool                         RemoveVariable(CAbstractVariablesModelItem& variableItem)                   { return false; }

public:
	CDrxSignal<void(CAbstractVariablesModelItem&)> SignalVariableAdded;
	CDrxSignal<void(CAbstractVariablesModelItem&)> SignalVariableRemoved;
	CDrxSignal<void(CAbstractVariablesModelItem&)> SignalVariableInvalidated;
};

}

