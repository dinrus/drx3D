// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/FlowGraph/StdAfx.h>
#include <drx3D/FlowGraph/IFlowBaseNode.h>

class CFlowNode_AudioSwitch final : public CFlowBaseNode<eNCT_Instanced>
{
public:

	explicit CFlowNode_AudioSwitch(SActivationInfo* pActInfo)
		: m_currentState(0)
		, m_switchId(DrxAudio::InvalidControlId)
	{
		//sanity checks
		DRX_ASSERT((eIn_SwitchStateNameLast - eIn_SwitchStateNameFirst) == (NUM_STATES - 1));
		DRX_ASSERT((eIn_SetStateLast - eIn_SetStateFirst) == (NUM_STATES - 1));

		for (i32 i = 0; i < NUM_STATES; ++i)
		{
			m_switchStates[i] = DrxAudio::InvalidSwitchStateId;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	virtual ~CFlowNode_AudioSwitch() override = default;

	CFlowNode_AudioSwitch(CFlowNode_AudioSwitch const&) = delete;
	CFlowNode_AudioSwitch(CFlowNode_AudioSwitch&&) = delete;
	CFlowNode_AudioSwitch& operator=(CFlowNode_AudioSwitch const&) = delete;
	CFlowNode_AudioSwitch& operator=(CFlowNode_AudioSwitch&&) = delete;

	//////////////////////////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo) override
	{
		return new CFlowNode_AudioSwitch(pActInfo);
	}

	//////////////////////////////////////////////////////////////////////////
	enum INPUTS
	{
		eIn_SwitchName,

		eIn_SwitchStateNameFirst,
		eIn_SwitchStateName2,
		eIn_SwitchStateName3,
		eIn_SwitchStateNameLast,

		eIn_SetStateFirst,
		eIn_SetState2,
		eIn_SetState3,
		eIn_SetStateLast,
	};

	enum OUTPUTS
	{
	};

	virtual void GetConfiguration(SFlowNodeConfig& config) override
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig<string>("audioSwitch_SwitchName",            _HELP("name of the Audio Switch"),                   "Switch"),
			InputPortConfig<string>("audioSwitchState_SwitchStateName1", _HELP("name of a Switch State"),                     "State1"),
			InputPortConfig<string>("audioSwitchState_SwitchStateName2", _HELP("name of a Switch State"),                     "State2"),
			InputPortConfig<string>("audioSwitchState_SwitchStateName3", _HELP("name of a Switch State"),                     "State3"),
			InputPortConfig<string>("audioSwitchState_SwitchStateName4", _HELP("name of a Switch State"),                     "State4"),

			InputPortConfig_Void("audioSwitchState_SetState1",           _HELP("Sets the switch to the corresponding state"), "SetState1"),
			InputPortConfig_Void("audioSwitchState_SetState2",           _HELP("Sets the switch to the corresponding state"), "SetState2"),
			InputPortConfig_Void("audioSwitchState_SetState3",           _HELP("Sets the switch to the corresponding state"), "SetState3"),
			InputPortConfig_Void("audioSwitchState_SetState4",           _HELP("Sets the switch to the corresponding state"), "SetState4"),
			{ 0 }
		};

		static const SOutputPortConfig outputs[] =
		{
			{ 0 }
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("This node allows one to set Audio Switches.");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser) override
	{
		i32 currentState = m_currentState;
		ser.BeginGroup("FlowAudioSwitchNode");
		ser.Value("current_state", currentState);
		ser.EndGroup();

		if (ser.IsReading())
		{
			m_currentState = 0;
			Init(pActInfo, currentState);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo) override
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				Init(pActInfo, 0);

				break;
			}
		case eFE_Activate:
			{
				if (IsPortActive(pActInfo, eIn_SwitchName))
				{
					GetSwitchId(pActInfo);
				}

				for (i32 stateIndex = eIn_SwitchStateNameFirst; stateIndex <= eIn_SwitchStateNameLast; ++stateIndex)
				{
					if (IsPortActive(pActInfo, stateIndex))
					{
						GetSwitchStateId(pActInfo, stateIndex);
					}
				}

				for (i32 statePort = eIn_SetStateFirst; statePort <= eIn_SetStateLast; ++statePort)
				{
					if (IsPortActive(pActInfo, statePort))
					{
						SetState(pActInfo->pEntity, statePort - eIn_SetStateFirst + 1);
						break;// short-circuit behavior: set the first state activated and stop
					}
				}

				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void GetMemoryUsage(IDrxSizer* s) const override
	{
		s->Add(*this);
	}

private:

	static i32k NUM_STATES = 4;

	//////////////////////////////////////////////////////////////////////////
	void GetSwitchId(SActivationInfo* const pActInfo)
	{
		string const& switchName = GetPortString(pActInfo, eIn_SwitchName);

		if (!switchName.empty())
		{
			m_switchId = DrxAudio::StringToId(switchName.c_str());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void GetSwitchStateId(SActivationInfo* const pActInfo, i32 const stateIndex)
	{
		string const& stateName = GetPortString(pActInfo, stateIndex);

		if (!stateName.empty() && (m_switchId != DrxAudio::InvalidControlId))
		{
			m_switchStates[stateIndex - eIn_SwitchStateNameFirst] = DrxAudio::StringToId(stateName.c_str());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Init(SActivationInfo* const pActInfo, i32 const currentState)
	{
		if (gEnv->pAudioSystem != nullptr)
		{
			GetSwitchId(pActInfo);

			if (m_switchId != DrxAudio::InvalidControlId)
			{
				for (i32 stateIndex = eIn_SwitchStateNameFirst; stateIndex <= eIn_SwitchStateNameLast; ++stateIndex)
				{
					GetSwitchStateId(pActInfo, stateIndex);
				}
			}

			m_currentState = 0;

			if (currentState != 0)
			{
				SetState(pActInfo->pEntity, currentState);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void SetStateOnProxy(IEntity* const pIEntity, i32 const stateIndex)
	{
		IEntityAudioComponent* const pIEntityAudioComponent = pIEntity->GetOrCreateComponent<IEntityAudioComponent>();

		if (pIEntityAudioComponent != nullptr)
		{
			pIEntityAudioComponent->SetSwitchState(m_switchId, m_switchStates[stateIndex]);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void SetStateOnGlobalObject(i32 const stateIndex)
	{
		gEnv->pAudioSystem->SetSwitchState(m_switchId, m_switchStates[stateIndex]);
	}

	//////////////////////////////////////////////////////////////////////////
	void SetState(IEntity* const pIEntity, i32 const newState)
	{
		DRX_ASSERT((0 < newState) && (newState <= NUM_STATES));

		// Cannot check for m_currentState != newState, because there can be several flowgraph nodes
		// setting the states on the same switch. This particular node might need to set the same state again,
		// if another node has set a different one in between the calls to set the state on this node.
		m_currentState = newState;

		if (pIEntity != nullptr)
		{
			SetStateOnProxy(pIEntity, m_currentState - 1);
		}
		else
		{
			SetStateOnGlobalObject(m_currentState - 1);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	i32                     m_currentState;
	DrxAudio::ControlId     m_switchId;
	DrxAudio::SwitchStateId m_switchStates[NUM_STATES];
};

REGISTER_FLOW_NODE("Audio:Switch", CFlowNode_AudioSwitch);
