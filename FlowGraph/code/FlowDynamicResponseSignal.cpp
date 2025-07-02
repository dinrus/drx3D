// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/FlowGraph/StdAfx.h>

#include <drx3D/FlowGraph/IFlowBaseNode.h>
#include <drx3D/Entity/IEntityComponent.h>
#include <drx3D/DynRespSys/IDynamicResponseSystem.h>

class CFlowNode_SendDynamicResponseSignal final : public CFlowBaseNode<eNCT_Instanced>, public DRS::IResponseUpr::IListener
{
public:

	explicit CFlowNode_SendDynamicResponseSignal(SActivationInfo* pActInfo)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	~CFlowNode_SendDynamicResponseSignal()
	{
		gEnv->pDynamicResponseSystem->GetResponseUpr()->RemoveListener(this);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo) override
	{
		return new CFlowNode_SendDynamicResponseSignal(pActInfo);
	}

	//////////////////////////////////////////////////////////////////////////
	enum EInputs
	{
		eIn_Send,
		eIn_SignalName,
		eIn_StringContextVariableName,
		eIn_StringContextVariableValue,
		eIn_IntContextVariableName,
		eIn_IntContextVariableValue
	};

	enum EOutputs
	{
		eOut_SignalSent,
		eOut_Done
	};

	virtual void GetConfiguration(SFlowNodeConfig& config) override
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Send",                          _HELP("send dynamic response signal name")),
			InputPortConfig<string>("SignalName",                 _HELP("dynamic response signal name")),
			InputPortConfig<string>("StringContextVariableName",  _HELP("the name of a string variable that is send along with this signal and can therefore be used in the responses")),
			InputPortConfig<string>("StringContextVariableValue", _HELP("a string value to send along with this signal")),
			InputPortConfig<string>("IntContextVariableName",     _HELP("the name of a numeric variable that is send along with this signal and can therefore be used in the responses")),
			InputPortConfig<i32>("IntContextVariableValue",       _HELP("a numeric value to send along with this signal")),
			{ 0 }
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<i32>("Sent", _HELP("Will be triggered when the signal has been sent")),
			OutputPortConfig<i32>("Done", _HELP("Will be triggered when the processing of the signal has finished. This means the response has finished or there was none.")),
			{ 0 }
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("This node sends a signal to the Dynamic Response System.");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser) override
	{
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo) override
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				break;
			}
		case eFE_Activate:
			{
				if (IsPortActive(pActInfo, eIn_Send))
				{
					if (pActInfo->pEntity)
					{
						const string& signalName = GetPortString(pActInfo, eIn_SignalName);
						IEntityDynamicResponseComponent* const pIEntityDrsProxy = drxcomponent_cast<IEntityDynamicResponseComponent*>(pActInfo->pEntity->CreateProxy(ENTITY_PROXY_DYNAMICRESPONSE));

						SET_DRS_USER_SCOPED("SendDrsSignal FlowGraph");

						const string& stringVariableName = GetPortString(pActInfo, eIn_StringContextVariableName);
						const string& intVariableName = GetPortString(pActInfo, eIn_IntContextVariableName);

						DRS::IVariableCollectionSharedPtr pContextVariableCollection = nullptr;
						if (!stringVariableName.empty())
						{
							pContextVariableCollection = gEnv->pDynamicResponseSystem->CreateContextCollection();
							const string& stringVariableValue = GetPortString(pActInfo, eIn_StringContextVariableValue);
							pContextVariableCollection->SetVariableValue(stringVariableName, CHashedString(stringVariableValue));
						}
						if (!intVariableName.empty())
						{
							if (!pContextVariableCollection)
							{
								pContextVariableCollection = gEnv->pDynamicResponseSystem->CreateContextCollection();
							}
							i32k intVariableValue = GetPortInt(pActInfo, eIn_IntContextVariableValue);
							pContextVariableCollection->SetVariableValue(intVariableName.c_str(), intVariableValue);
						}

						DRS::SignalInstanceId signalID;
						if (IsOutputConnected(pActInfo, eOut_Done)) //we only need to register ourself as a listener to the signal, if the done port is used
						{
							m_actInfo = *pActInfo;
							signalID = pIEntityDrsProxy->GetResponseActor()->QueueSignal(signalName, pContextVariableCollection, this);
						}
						else
						{
							signalID = pIEntityDrsProxy->GetResponseActor()->QueueSignal(signalName, pContextVariableCollection);
						}

						ActivateOutput(pActInfo, eOut_SignalSent, static_cast<i32>(signalID));
					}
					else
					{
						DrxWarning(VALIDATOR_MODULE_FLOWGRAPH, VALIDATOR_WARNING, "CAction:DRS_SendSignal: Cannot send a signal without an specified EntityID.");
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

	//////////////////////////////////////////////////////////
	// DRS::IResponseUpr::IListener implementation
	virtual void OnSignalProcessingFinished(DRS::IResponseUpr::IListener::SSignalInfos& signal, DRS::IResponseInstance* pFinishedResponse, DRS::IResponseUpr::IListener::eProcessingResult outcome) override
	{
		ActivateOutput(&m_actInfo, eOut_Done, static_cast<i32>(signal.id));
	}
	//////////////////////////////////////////////////////////////////////////

private:
	SActivationInfo m_actInfo;
	//////////////////////////////////////////////////////////////////////////
};

class CFlowNode_CancelDynamicResponseSignal final : public CFlowBaseNode<eNCT_Instanced>
{
public:
	explicit CFlowNode_CancelDynamicResponseSignal(SActivationInfo* pActInfo)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo) override
	{
		return new CFlowNode_CancelDynamicResponseSignal(pActInfo);
	}

	//////////////////////////////////////////////////////////////////////////
	enum EInputs
	{
		eIn_Cancel,
		eIn_SignalName,
	};

	enum EOutputs
	{
		eOut_CancelRequestSent,
	};

	virtual void GetConfiguration(SFlowNodeConfig& config) override
	{
		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Cancel",        _HELP("cancel the processing of the dynamic response signal")),
			InputPortConfig<string>("SignalName", _HELP("the name of the dynamic response signal name to cancel"),"Name"),
			{ 0 }
		};

		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<string>("Canceled", _HELP("Will be triggered when the cancel-request for the signal has been sent")),
			{ 0 }
		};

		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("This node sends a signal to the Dynamic Response System.");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.SetCategory(EFLN_APPROVED);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser) override
	{
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo) override
	{
		switch (event)
		{
		case eFE_Initialize:
			{
				break;
			}
		case eFE_Activate:
			{
				if (IsPortActive(pActInfo, eIn_Cancel))
				{
					const string& signalName = GetPortString(pActInfo, eIn_SignalName);
					if (pActInfo->pEntity)
					{
						IEntityDynamicResponseComponent* const pIEntityDrsProxy = drxcomponent_cast<IEntityDynamicResponseComponent*>(pActInfo->pEntity->CreateProxy(ENTITY_PROXY_DYNAMICRESPONSE));
						gEnv->pDynamicResponseSystem->CancelSignalProcessing(signalName, pIEntityDrsProxy->GetResponseActor());
					}
					else
					{
						gEnv->pDynamicResponseSystem->CancelSignalProcessing(signalName);
					}
					ActivateOutput(pActInfo, eOut_CancelRequestSent, true);
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
	//////////////////////////////////////////////////////////////////////////
};

REGISTER_FLOW_NODE("DynamicResponse:SendSignal", CFlowNode_SendDynamicResponseSignal);
REGISTER_FLOW_NODE("DynamicResponse:CancelSignalProcessing", CFlowNode_CancelDynamicResponseSignal);
