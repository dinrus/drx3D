// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlowPrefabNodes.cpp
//  Version:     v1.00
//  Created:     11/6/2012 by Dean Claassen
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2010
//  Описание: Prefab flownode functionality
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/FlowGraph/StdAfx.h>

#include <drx3D/FlowGraph/FlowPrefabNodes.h>

#include <drx3D/Act/ICustomEvents.h>

//////////////////////////////////////////////////////////////////////////
// Prefab:EventSource node
// This node is placed inside a prefab flowgraph to create / handle a custom event
//////////////////////////////////////////////////////////////////////////
CFlowNode_PrefabEventSource::~CFlowNode_PrefabEventSource()
{
	if (m_eventId != CUSTOMEVENTID_INVALID)
	{
		ICustomEventUpr* pCustomEventUpr = gEnv->pGameFramework->GetICustomEventUpr();
		DRX_ASSERT(pCustomEventUpr != NULL);
		pCustomEventUpr->UnregisterEventListener(this, m_eventId);
	}
}

void CFlowNode_PrefabEventSource::GetConfiguration(SFlowNodeConfig& config)
{
	static const SInputPortConfig in_ports[] =
	{
		InputPortConfig<string>("PrefabName",   _HELP("Prefab name")),
		InputPortConfig<string>("InstanceName", _HELP("Prefab instance name")),
		InputPortConfig<string>("EventName",    _HELP("Name of event associated to this prefab")),
		InputPortConfig<i32>("EventType",       ePrefabEventType_Out,                             _HELP("Name of event associated to this prefab"), nullptr, _UICONFIG("enum_int:InOut=0,In=1,Out=2") ),
		InputPortConfig_AnyType("FireEvent",    _HELP("Fires associated event")),
		InputPortConfig<i32>("EventId",         0,                                                _HELP("Event id storage")),
		InputPortConfig<i32>("EventIndex",      -1,                                               _HELP("Event id storage")),
		{ 0 }
	};

	static const SOutputPortConfig out_ports[] =
	{
		OutputPortConfig_AnyType("EventFired", _HELP("Triggered when associated event is fired")),
		{ 0 }
	};

	config.pInputPorts = in_ports;
	config.pOutputPorts = out_ports;
	config.sDescription = _HELP("Event node that must be placed inside a prefab for it to be handled in an instance");
	config.SetCategory(EFLN_APPROVED);
}

void CFlowNode_PrefabEventSource::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
{
	ICustomEventUpr* pCustomEventUpr = gEnv->pGameFramework->GetICustomEventUpr();
	DRX_ASSERT(pCustomEventUpr != NULL);

	if (event == eFE_Activate)
	{
		if (IsPortActive(pActInfo, EIP_FireEvent))
		{
			if (m_eventId != CUSTOMEVENTID_INVALID)
			{
				const TFlowInputData& customData = GetPortAny(pActInfo, EIP_FireEvent);
				pCustomEventUpr->FireEvent(m_eventId, customData);
			}

			// Output port is activated from event
		}
	}
	else if (event == eFE_Initialize)
	{
		m_actInfo = *pActInfo;

		const TCustomEventId eventId = (TCustomEventId)GetPortInt(pActInfo, EIP_EventId);

		if (eventId != m_eventId)
		{
			if (m_eventId != CUSTOMEVENTID_INVALID)
			{
				pCustomEventUpr->UnregisterEventListener(this, m_eventId);
			}
			m_eventId = eventId;
			if (eventId != CUSTOMEVENTID_INVALID)
			{
				pCustomEventUpr->RegisterEventListener(this, eventId);
			}
		}
	}
}

void CFlowNode_PrefabEventSource::OnCustomEvent(const TCustomEventId eventId, const TFlowInputData& customData)
{
	if (m_eventId == eventId)
	{
		ActivateOutput(&m_actInfo, EOP_EventFired, customData);
	}
}

//////////////////////////////////////////////////////////////////////////
// Prefab:Instance node
// This node represents a prefab instance (Currently used to handle events specific to a prefab instance)
//////////////////////////////////////////////////////////////////////////
CFlowNode_PrefabInstance::CFlowNode_PrefabInstance(SActivationInfo* pActInfo)
{
	for (i32 i = 0; i < CUSTOMEVENTS_PREFABS_MAXNPERINSTANCE; ++i)
	{
		m_eventIds.push_back(CUSTOMEVENTID_INVALID);
	}
}

CFlowNode_PrefabInstance::~CFlowNode_PrefabInstance()
{
	ICustomEventUpr* pCustomEventUpr = gEnv->pGameFramework->GetICustomEventUpr();
	DRX_ASSERT(pCustomEventUpr != NULL);

	for (i32 i = 0; i < CUSTOMEVENTS_PREFABS_MAXNPERINSTANCE; ++i)
	{
		TCustomEventId& eventId = m_eventIds[i];
		if (eventId != CUSTOMEVENTID_INVALID)
		{
			pCustomEventUpr->UnregisterEventListener(this, eventId);
			eventId = CUSTOMEVENTID_INVALID;
		}
	}
}

#define PREFAB_INSTANCE_CREATE_INPUT_PORT(ID)                                     \
  InputPortConfig_AnyType("Event" STRINGIFY(ID), _HELP("Event trigger port")),    \
  InputPortConfig<i32>("Event" STRINGIFY(ID) "Id", 0, _HELP("Event id storage")), \
  InputPortConfig<string>("Event" STRINGIFY(ID) "Name", _HELP("Event name storage")),

#define PREFAB_INSTANCE_CREATE_OUTPUT_PORT(ID) \
  OutputPortConfig_AnyType("Event" STRINGIFY(ID), _HELP("Triggered when associated event is fired")),

void CFlowNode_PrefabInstance::GetConfiguration(SFlowNodeConfig& config)
{
	static const SInputPortConfig in_ports[] =
	{
		InputPortConfig<string>("PrefabName",   _HELP("Prefab name")),
		InputPortConfig<string>("InstanceName", _HELP("Prefab instance name")),
		PREFAB_INSTANCE_CREATE_INPUT_PORT(1)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(2)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(3)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(4)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(5)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(6)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(7)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(8)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(9)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(10)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(11)
		PREFAB_INSTANCE_CREATE_INPUT_PORT(12)
		{
			0
		}
	};

	static const SOutputPortConfig out_ports[] =
	{
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(1)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(2)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(3)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(4)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(5)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(6)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(7)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(8)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(9)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(10)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(11)
		PREFAB_INSTANCE_CREATE_OUTPUT_PORT(12)
		{
			0
		}
	};

	config.pInputPorts = in_ports;
	config.pOutputPorts = out_ports;
	config.sDescription = _HELP("Prefab instance node to handle events specific to an instance");
	config.SetCategory(EFLN_APPROVED | EFLN_HIDE_UI);
}

#undef PREFAB_INSTANCE_CREATE_INPUT_PORT
#undef PREFAB_INSTANCE_CREATE_OUTPUT_PORT

void CFlowNode_PrefabInstance::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
{
	ICustomEventUpr* pCustomEventUpr = gEnv->pGameFramework->GetICustomEventUpr();
	DRX_ASSERT(pCustomEventUpr != NULL);

	if (event == eFE_Activate)
	{
		i32 iCurInputPort = EIP_Event1;
		for (i32 i = 0; i < CUSTOMEVENTS_PREFABS_MAXNPERINSTANCE; ++i)
		{
			if (IsPortActive(pActInfo, iCurInputPort))
			{
				const TCustomEventId eventId = m_eventIds[i];
				if (m_eventIds[i] != CUSTOMEVENTID_INVALID)
				{
					const TFlowInputData& customData = GetPortAny(pActInfo, iCurInputPort);
					pCustomEventUpr->FireEvent(eventId, customData);
				}

				// Output port is activated from event
			}

			iCurInputPort += static_cast<i32>(ePrefabInstance_NumInputPortsPerEvent);
		}

	}
	else if (event == eFE_Initialize)
	{
		m_actInfo = *pActInfo;

		i32 iCurPort = EIP_Event1Id;
		for (i32 i = 0; i < CUSTOMEVENTS_PREFABS_MAXNPERINSTANCE; ++i)
		{
			const TCustomEventId eventId = (TCustomEventId)GetPortInt(pActInfo, iCurPort);

			if (eventId != m_eventIds[i])
			{
				if (m_eventIds[i] != CUSTOMEVENTID_INVALID)
				{
					pCustomEventUpr->UnregisterEventListener(this, m_eventIds[i]);
				}
				m_eventIds[i] = eventId;
				if (eventId != CUSTOMEVENTID_INVALID)
				{
					pCustomEventUpr->RegisterEventListener(this, eventId);
				}
			}

			iCurPort += static_cast<i32>(ePrefabInstance_NumInputPortsPerEvent);
		}
	}
}

void CFlowNode_PrefabInstance::OnCustomEvent(const TCustomEventId eventId, const TFlowInputData& customData)
{
	// Need to determine which output port corresponds to event id
	i32 iOutputPort = EOP_Event1;
	for (i32 i = 0; i < CUSTOMEVENTS_PREFABS_MAXNPERINSTANCE; ++i)
	{
		const TCustomEventId curEventId = m_eventIds[i];
		if (curEventId == eventId)
		{
			ActivateOutput(&m_actInfo, iOutputPort, customData);
			break;
		}

		iOutputPort++;
	}
}

REGISTER_FLOW_NODE("Prefab:EventSource", CFlowNode_PrefabEventSource)
REGISTER_FLOW_NODE("Prefab:Instance", CFlowNode_PrefabInstance)
