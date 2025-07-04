// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "GraphItem.h"

#include "AbstractObjectModel.h"

#include "ScriptBrowserUtils.h"
#include "ObjectModel.h"
#include "GraphViewModel.h"

#include <DrxSchematyc/SerializationUtils/ISerializationContext.h>

#include <DrxSchematyc/Script/Elements/IScriptFunction.h>
#include <DrxSchematyc/Script/Elements/IScriptConstructor.h>

#include <DrxSchematyc/Script/IScriptGraph.h>
#include <DrxSchematyc/Script/IScriptExtension.h>

#include <DrxIcon.h>
#include <QtUtil.h>

namespace DrxSchematycEditor 
{
CGraphItem::CGraphItem(Schematyc::IScriptFunction& scriptFunction, CAbstractObjectStructureModel& model)
	: CAbstractObjectStructureModelItem(model)
	, m_pScriptFunction(&scriptFunction)
	, m_pParentItem(nullptr)
	, m_pGraphModel(nullptr)
{
	m_graphType = eGraphType_Function;
	LoadFromScriptElement();
}

CGraphItem::CGraphItem(Schematyc::IScriptConstructor& scriptConstructor, CAbstractObjectStructureModel& model)
	: CAbstractObjectStructureModelItem(model)
	, m_pScriptConstructor(&scriptConstructor)
	, m_pParentItem(nullptr)
	, m_pGraphModel(nullptr)
{
	m_graphType = eGraphType_Construction;
	LoadFromScriptElement();
}

CGraphItem::CGraphItem(Schematyc::IScriptSignalReceiver& scriptSignalReceiver, CAbstractObjectStructureModel& model)
	: CAbstractObjectStructureModelItem(model)
	, m_pScriptSignalReceiver(&scriptSignalReceiver)
	, m_pParentItem(nullptr)
	, m_pGraphModel(nullptr)
{
	m_graphType = eGraphType_SignalReceiver;
	LoadFromScriptElement();
}

CGraphItem::~CGraphItem()
{

}

void CGraphItem::SetName(QString name)
{
	if (AllowsRenaming())
	{
		Schematyc::CStackString uniqueName = QtUtil::ToString(name).c_str();
		Schematyc::ScriptBrowserUtils::MakeScriptElementNameUnique(uniqueName, m_model.GetScriptElement());

		m_pScriptElement->SetName(uniqueName);
		m_name = m_pScriptElement->GetName();
	}
}

const DrxIcon* CGraphItem::GetIcon() const
{
	switch (m_graphType)
	{
	case eGraphType_Construction:
	{
		static std::unique_ptr<DrxIcon> pIconConstructor;
		if (pIconConstructor.get() == nullptr)
		{
			pIconConstructor = stl::make_unique<DrxIcon>("icons:schematyc/script_constructor.png");
		}
		return pIconConstructor.get();
	}
	case eGraphType_Destruction:
	{
		static std::unique_ptr<DrxIcon> pIconDestructor;
		if (pIconDestructor.get() == nullptr)
		{
			pIconDestructor = stl::make_unique<DrxIcon>("icons:schematyc/script_destructor.png");
		}
		return pIconDestructor.get();
	}
	case eGraphType_Function:
	{
		static std::unique_ptr<DrxIcon> pIconFunction;
		if (pIconFunction.get() == nullptr)
		{
			pIconFunction = stl::make_unique<DrxIcon>("icons:schematyc/script_function.png");
		}
		return pIconFunction.get();
	}
	case eGraphType_SignalReceiver:
	{
		static std::unique_ptr<DrxIcon> pIconSignalReceivers;
		if (pIconSignalReceivers.get() == nullptr)
		{
			pIconSignalReceivers = stl::make_unique<DrxIcon>("icons:schematyc/script_signal_receiver.png");
		}
		return pIconSignalReceivers.get();
	}
	default:
		break;
	}

	return nullptr;
}

CAbstractObjectStructureModelItem* CGraphItem::GetParentItem() const
{
	return static_cast<CAbstractObjectStructureModelItem*>(m_pParentItem);
}

u32 CGraphItem::GetIndex() const
{
	if (m_pParentItem)
	{
		return m_pParentItem->GetChildItemIndex(*this);
	}
	else
	{
		return GetModel().GetChildItemIndex(*this);
	}

	return 0xffffffff;
}

void CGraphItem::Serialize(Serialization::IArchive& archive)
{
	// TODO: This will only work for serialization to properties in inspector!
	Schematyc::SSerializationContextParams serParams(archive, Schematyc::ESerializationPass::Edit);
	Schematyc::ISerializationContextPtr pSerializationContext = gEnv->pSchematyc->CreateSerializationContext(serParams);
	// ~TODO

	m_pScriptElement->Serialize(archive);
}

bool CGraphItem::AllowsRenaming() const
{
	const bool allowsRenaming = !m_pScriptElement->GetFlags().Check(Schematyc::EScriptElementFlags::FixedName);
	return allowsRenaming;
}

DrxGUID CGraphItem::GetGUID() const
{
	return m_pScriptElement->GetGUID();
}

void CGraphItem::LoadFromScriptElement()
{
	m_name = m_pScriptElement->GetName();

	if (m_pGraphModel)
	{
		// TODO: Signal!
		delete m_pGraphModel;
	}

	Schematyc::IScriptExtension* pGraphExtension = m_pScriptElement->GetExtensions().QueryExtension(Schematyc::EScriptExtensionType::Graph);
	if (pGraphExtension)
	{
		Schematyc::IScriptGraph* pScriptGraph = static_cast<Schematyc::IScriptGraph*>(pGraphExtension);
		m_pGraphModel = new CNodeGraphViewModel(*pScriptGraph);
		// TODO: Signal!
	}
}

}

