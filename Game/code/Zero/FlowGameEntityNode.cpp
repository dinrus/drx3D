// Разработка 2018-2023 DinrusPro / Dinrus Group. All rights reserved.

#include "StdAfx.h"
#include "FlowGameEntityNode.h"

CGameEntityNodeFactory::CGameEntityNodeFactory()
{
	m_nRefCount = 0;
	m_activateCallback = 0;
}

CGameEntityNodeFactory::~CGameEntityNodeFactory()
{
	Reset();
}

void CGameEntityNodeFactory::Reset()
{
}

void CGameEntityNodeFactory::MakeHumanName(SInputPortConfig& config)
{
	tuk name = strchr((tuk)config.name, '_');
	if (name != 0)
		config.humanName = name + 1;
}

void CGameEntityNodeFactory::AddInputs(const std::vector<SInputPortConfig>& inputs, FlowNodeOnActivateFunction callback)
{
	DRX_ASSERT_MESSAGE(m_inputs.empty(), "Supplying entity node inputs multiple times: not supported");
	DRX_ASSERT_MESSAGE(m_callbacks.empty(), "Using global callback with individually specified callbacks: not supported");

	m_inputs.clear();
	m_callbacks.clear();
	m_inputs = inputs;
	m_activateCallback = callback;
}

void CGameEntityNodeFactory::AddInput(IEntityClass::EventValueType type, tukk name, FlowNodeInputFunction callback)
{
	switch (type)
	{
	case IEntityClass::EVT_BOOL:
		m_inputs.push_back(InputPortConfig<bool>(name));
		MakeHumanName(m_inputs.back());
		break;
	case IEntityClass::EVT_INT:
		m_inputs.push_back(InputPortConfig<i32>(name));
		MakeHumanName(m_inputs.back());
		break;
	case IEntityClass::EVT_FLOAT:
		m_inputs.push_back(InputPortConfig<float>(name));
		MakeHumanName(m_inputs.back());
		break;
	case IEntityClass::EVT_VECTOR:
		m_inputs.push_back(InputPortConfig<Vec3>(name));
		MakeHumanName(m_inputs.back());
		break;
	case IEntityClass::EVT_ENTITY:
		m_inputs.push_back(InputPortConfig<EntityId>(name));
		MakeHumanName(m_inputs.back());
		break;
	case IEntityClass::EVT_STRING:
		m_inputs.push_back(InputPortConfig<string>(name));
		MakeHumanName(m_inputs.back());
		break;
	default:
		assert(false);
		break;
	}

	m_callbacks.push_back(callback);
}

void CGameEntityNodeFactory::AddOutputs(const std::vector<SOutputPortConfig>& outputs)
{
	m_outputs = outputs;
}

i32 CGameEntityNodeFactory::AddOutput(IEntityClass::EventValueType type, tukk name, tukk description)
{
	switch (type)
	{
	case IEntityClass::EVT_BOOL:
		m_outputs.push_back(OutputPortConfig<bool>(name, description));
		break;
	case IEntityClass::EVT_INT:
		m_outputs.push_back(OutputPortConfig<i32>(name, description));
		break;
	case IEntityClass::EVT_FLOAT:
		m_outputs.push_back(OutputPortConfig<float>(name, description));
		break;
	case IEntityClass::EVT_VECTOR:
		m_outputs.push_back(OutputPortConfig<Vec3>(name, description));
		break;
	case IEntityClass::EVT_ENTITY:
		m_outputs.push_back(OutputPortConfig<EntityId>(name, description));
		break;
	case IEntityClass::EVT_STRING:
		m_outputs.push_back(OutputPortConfig<string>(name, description));
		break;
	default:
		assert(false);
		return -1;
	}

	return (m_outputs.size() - 1);
}

void CGameEntityNodeFactory::Close()
{
	// add null entries at end
	m_outputs.push_back(OutputPortConfig<bool>(0, 0));
	m_inputs.push_back(InputPortConfig<bool>(0, false));
}

void CGameEntityNodeFactory::GetConfiguration(SFlowNodeConfig& config)
{
	static const SInputPortConfig in_config[] =
	{
		{ 0 }
	};
	static const SOutputPortConfig out_config[] =
	{
		{ 0 }
	};

	config.nFlags |= EFLN_TARGET_ENTITY | EFLN_HIDE_UI;
	config.SetCategory(EFLN_APPROVED);

	config.pInputPorts = in_config;
	config.pOutputPorts = out_config;

	if (!m_inputs.empty())
	{
		assert((m_activateCallback != nullptr) || (m_inputs.size() == m_callbacks.size() + 1));  // one extra input is a null one as a terminator
		config.pInputPorts = &m_inputs[0];
	}

	if (!m_outputs.empty())
	{
		config.pOutputPorts = &m_outputs[0];
	}
}

IFlowNodePtr CGameEntityNodeFactory::Create(IFlowNode::SActivationInfo* pActInfo)
{
	return new CFlowGameEntityNode(this, pActInfo);
}

CFlowGameEntityNode::CFlowGameEntityNode(CGameEntityNodeFactory* pClass, SActivationInfo* pActInfo)
{
	m_entityId = 0;
	m_pClass = pClass;
	m_nodeID = pActInfo->myID;
	m_pGraph = pActInfo->pGraph;
	m_lastInitializeFrameId = -1;
}

IFlowNodePtr CFlowGameEntityNode::Clone(SActivationInfo* pActInfo)
{
	IFlowNode* pNode = new CFlowGameEntityNode(m_pClass, pActInfo);
	return pNode;
};

void CFlowGameEntityNode::GetConfiguration(SFlowNodeConfig& config)
{
	m_pClass->GetConfiguration(config);
}

void CFlowGameEntityNode::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_ACTION);

	switch (event)
	{
	case eFE_SetEntityId:
		{
			EntityId newEntityId = GetEntityId(pActInfo);

			if (m_entityId)
			{
				UnregisterEvent();
			}

			m_entityId = newEntityId;

			if (newEntityId)
			{
				RegisterEvent();
			}
		}
		break;
	case eFE_Activate:
		{
			FlowNodeOnActivateFunction activateFunc = m_pClass->m_activateCallback;
			if (activateFunc)
			{
				activateFunc(m_entityId, pActInfo, this);
			}
			else
			{
				for (size_t i = 0; i < m_pClass->m_inputs.size() - 1; i++)
				{
					if (IsPortActive(pActInfo, i))
					{
						FlowNodeInputFunction func = m_pClass->m_callbacks[i];
						assert(func);
						if (func)
						{
							func(m_entityId, pActInfo->pInputPorts[i]);
						}
					}
				}
			}
		}
		break;
	case eFE_Initialize:
		{
			if (gEnv->IsEditor() && m_entityId)
			{
				UnregisterEvent();
				RegisterEvent();
			}

			i32k frameId = gEnv->pRenderer->GetFrameID(false);
			if (frameId != m_lastInitializeFrameId)
			{
				m_lastInitializeFrameId = frameId;
				for (size_t i = 0; i < m_pClass->GetNumOutputs(); i++)
				{
					ActivateOutput(pActInfo, i, SFlowSystemVoid());
				}
			}
		}
		break;
	}
}

void CFlowGameEntityNode::OnEntityEvent(IEntity* pEntity, SEntityEvent& event)
{
	if (!m_pGraph->IsEnabled() || m_pGraph->IsSuspended() || !m_pGraph->IsActive())
		return;

	switch (event.event)
	{
	case ENTITY_EVENT_DONE:
		{
			// The entity being destroyed is not always the entity stored in the input of the node (which is what actually is Get/Set by those functions ).
			// In the case of AIWaves, when the node is forwarded to newly spawned entities, this event will be called when those spawned entities are destroyed. But
			//  but the input will still be the original entityId. And should not be zeroed, or the node will stop working after a reset (this only can happen on editor mode)
			if (m_pGraph->GetEntityId(m_nodeID) == pEntity->GetId())
				m_pGraph->SetEntityId(m_nodeID, 0);
		}
		break;

	case ENTITY_EVENT_ACTIVATE_FLOW_NODE_OUTPUT:
		{
			i32 port = (i32)event.nParam[0];

			// Find output port.
			if (port >= (i32)m_pClass->m_outputs.size() || !m_pClass->m_outputs[port].name)
				break;

			SFlowAddress addr(m_nodeID, port, true);

			TFlowInputData data = *(TFlowInputData*)(event.nParam[1]);
			m_pGraph->ActivatePort(addr, data);
		}
		break;

	default:
		break;
	}
}

bool CFlowGameEntityNode::SerializeXML(SActivationInfo* pActivatioNInfo, const XmlNodeRef& root, bool reading)
{
	return true;
}

void CFlowGameEntityNode::Serialize(SActivationInfo*, TSerialize ser)
{
	if (ser.IsReading())
		m_lastInitializeFrameId = -1;
}
