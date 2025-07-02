// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "VariableItem.h"

#include "ObjectModel.h"
#include "StateItem.h"

#include <DrxSchematyc/Script/Elements/IScriptVariable.h>
#include <DrxSchematyc/SerializationUtils/ISerializationContext.h>

#include <QtUtil.h>
#include <QObject>

namespace DrxSchematycEditor {

/*
   CVariableItem::CVariableItem(Schematyc::IScriptVariable& scriptVariable, CStateItem& state)
   : CAbstractVariablesModelItem(static_cast<CAbstractVariablesModelItem&>(state))
   , m_scriptVariable(scriptVariable)
   , m_owner(EOwner::State)
   {

   }*/

CVariableItem::CVariableItem(Schematyc::IScriptVariable& scriptVariable, CObjectModel& object)
	: CAbstractVariablesModelItem(static_cast<CAbstractVariablesModel&>(object))
	, m_scriptVariable(scriptVariable)
	, m_owner(EOwner::Object)
{

}

CVariableItem::~CVariableItem()
{

}

QString CVariableItem::GetName() const
{
	QString name = m_scriptVariable.GetName();
	return name;
}

void CVariableItem::SetName(QString name)
{
	Schematyc::CStackString uniqueName = QtUtil::ToString(name).c_str();
	// TODO: Unique in which scope?
	//Schematyc::ScriptBrowserUtils::MakeScriptElementNameUnique(uniqueName, m_model.GetScriptElement());
	// ~TODO

	m_scriptVariable.SetName(uniqueName.c_str());
	GetModel().SignalVariableInvalidated(*this);
}

void CVariableItem::Serialize(Serialization::IArchive& archive)
{
	// TODO: This will only work for serialization to properties in inspector!
	Schematyc::SSerializationContextParams serParams(archive, Schematyc::ESerializationPass::Edit);
	Schematyc::ISerializationContextPtr pSerializationContext = gEnv->pSchematyc->CreateSerializationContext(serParams);
	// ~TODO

	m_scriptVariable.Serialize(archive);
}

DrxGUID CVariableItem::GetGUID() const
{
	return m_scriptVariable.GetGUID();
}

}

