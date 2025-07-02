// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxSchematyc/Script/IScriptView.h>
#include <DrxSchematyc/Script/IScriptRegistry.h>
#include <DrxSchematyc/Script/Elements/IScriptComponentInstance.h>

#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include <drx3D/CoreX/Serialization/Forward.h>
#include <DrxIcon.h>

#include <QVariant>
#include <QString>

class CAbstractDictionary;

namespace DrxSchematycEditor {

class CAbstractComponentsModel;

class CComponentItem
{
public:
	CComponentItem(Schematyc::IScriptComponentInstance& componentInstance, CAbstractComponentsModel& model);
	~CComponentItem();

	void                                 SetName(QString name);
	QString                              GetName() const { return m_name; }
	QString                              GetDescription() const;
	DrxIcon                              GetIcon() const;

	CAbstractComponentsModel&            GetModel() const    { return m_model; }

	Schematyc::IScriptComponentInstance& GetInstance() const { return m_componentInstance; }

	void                                 Serialize(Serialization::IArchive& archive);

public:
	CDrxSignal<void(CComponentItem&)> SignalComponentChanged;

private:
	CAbstractComponentsModel&            m_model;
	Schematyc::IScriptComponentInstance& m_componentInstance;
	QString                              m_name;
};

class CAbstractComponentsModel
{
public:
	virtual u32                     GetComponentItemCount() const = 0;
	virtual CComponentItem*            GetComponentItemByIndex(u32 index) const = 0;
	virtual CComponentItem*            CreateComponent(DrxGUID typeId, tukk szName) = 0;
	virtual bool                       RemoveComponent(CComponentItem& component) = 0;

	virtual CAbstractDictionary*       GetAvailableComponentsDictionary() = 0;

	virtual Schematyc::IScriptElement* GetScriptElement() const = 0;

public:
	CDrxSignal<void(CComponentItem&)> SignalAddedComponentItem;
	CDrxSignal<void(CComponentItem&)> SignalRemovedComponentItem;
};

}

