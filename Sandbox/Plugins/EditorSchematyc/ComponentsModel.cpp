// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ComponentsModel.h"

#include "ScriptBrowserUtils.h"
#include "DetailWidget.h"

#include <DrxSchematyc/Utils/Any.h>
#include <DrxSchematyc/SerializationUtils/ISerializationContext.h>

#include <DrxIcon.h>

#include "QtUtil.h"

namespace DrxSchematycEditor {

CComponentItem::CComponentItem(Schematyc::IScriptComponentInstance& componentInstance, CAbstractComponentsModel& model)
	: m_componentInstance(componentInstance)
	, m_model(model)
{
	m_name = m_componentInstance.GetName();
}

CComponentItem::~CComponentItem()
{

}

void CComponentItem::SetName(QString name)
{
	Schematyc::CStackString uniqueName = QtUtil::ToString(name).c_str();
	Schematyc::ScriptBrowserUtils::MakeScriptElementNameUnique(uniqueName, m_model.GetScriptElement());

	m_componentInstance.SetName(uniqueName.c_str());
	m_name = m_componentInstance.GetName();
}

QString CComponentItem::GetDescription() const
{
	return "";
}

DrxIcon CComponentItem::GetIcon() const
{
	return DrxIcon(Schematyc::ScriptBrowserUtils::GetScriptElementIcon(m_componentInstance));
}

void CComponentItem::Serialize(Serialization::IArchive& archive)
{
	Schematyc::SSerializationContextParams serializationParams(archive, Schematyc::ESerializationPass::Edit);
	Schematyc::ISerializationContextPtr pSerializationContext = gEnv->pSchematyc->CreateSerializationContext(serializationParams);
	m_componentInstance.Serialize(archive);
	if (archive.isInput())
	{
		gEnv->pSchematyc->GetScriptRegistry().ElementModified(m_componentInstance);
	}
}

}

