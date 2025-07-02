// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FLOWDATA_H__
#define __FLOWDATA_H__

#pragma once

#include <drx3D/FlowGraph/IFlowSystem.h>

class CFlowData : public IFlowNodeData
{
public:
	static i32k TYPE_BITS = 12;
	static i32k TYPE_MAX_COUNT = 1 << TYPE_BITS;

	CFlowData(IFlowNodePtr pImpl, const string& name, TFlowNodeTypeId type);
	CFlowData();
	~CFlowData();
	CFlowData(const CFlowData&);
	CFlowData(CFlowData&&) = default;
	void       Swap(CFlowData&);
	CFlowData& operator=(const CFlowData& rhs);
	CFlowData& operator=(CFlowData&&) = default;

	ILINE i32  GetNumOutputs() const                    { return m_nOutputs; }
	ILINE void SetOutputFirstEdge(i32 output, i32 edge) { m_pOutputFirstEdge[output] = edge; }
	ILINE i32  GetOutputFirstEdge(i32 output) const     { return m_pOutputFirstEdge[output]; }

	// activation of ports with data
	template<class T>
	ILINE bool ActivateInputPort(TFlowPortId port, const T& value)
	{
		TFlowInputData* pPort = &m_pInputData[port];
		pPort->SetUserFlag(true);
		return pPort->SetValueWithConversion(value);
	}

	ILINE bool SetInputPort(TFlowPortId port, const TFlowInputData& value)
	{
		TFlowInputData* pPort = &m_pInputData[port];
		return pPort->SetValueWithConversion(value);
	}
	TFlowInputData* GetInputPort(TFlowPortId port)
	{
		TFlowInputData* pPort = &m_pInputData[port];
		return pPort;
	}

	void FlagInputPorts();

	// perform a single activation round
	void Activated(IFlowNode::SActivationInfo*, IFlowNode::EFlowEvent);
	// perform a single update round (called if we have called SetRegularlyUpdated(id,true);
	void Update(IFlowNode::SActivationInfo*);
	// sends Suspended/Resumed events
	void SetSuspended(IFlowNode::SActivationInfo*, bool suspended);
	// resolve a port identifier
	bool ResolvePort(tukk name, TFlowPortId& port, bool isOutput);
	bool SerializeXML(IFlowNode::SActivationInfo*, const XmlNodeRef& node, bool reading);
	void Serialize(IFlowNode::SActivationInfo*, TSerialize ser);
	void PostSerialize(IFlowNode::SActivationInfo*);

	void CloneImpl(IFlowNode::SActivationInfo* pActInfo)
	{
		m_pImpl = m_pImpl->Clone(pActInfo);
	}

	bool IsValid() const
	{
		return m_pImpl != NULL;
	}

	bool ValidatePort(TFlowPortId id, bool isOutput) const
	{
		if (isOutput)
		{
			return id < m_nOutputs;
		}
		else
		{
			return id < m_nInputs;
		}
	}

	const IFlowNodePtr& GetImpl() const
	{
		return m_pImpl;
	}

	TFlowNodeTypeId GetTypeId() const           { return m_typeId; }
	const string& GetName() const             { return m_name; }
	void          SetName(const string& name) { m_name = name; }

	tukk   GetPortName(TFlowPortId port, bool output) const
	{
		SFlowNodeConfig config;
		DoGetConfiguration(config);
		return output ? config.pOutputPorts[port].name : config.pInputPorts[port].name;
	}

	//////////////////////////////////////////////////////////////////////////
	// IFlowNodeData
	//////////////////////////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		DoGetConfiguration(config);
	}
	virtual TFlowNodeTypeId GetNodeTypeId() { return m_typeId; }
	virtual tukk     GetNodeName()   { return m_name.c_str(); }
	virtual IFlowNode*      GetNode()
	{
		return &*GetImpl();
	}
	virtual i32      GetNumInputPorts() const           { return m_nInputs; }
	virtual i32      GetNumOutputPorts() const          { return m_nOutputs; }
	virtual EntityId GetCurrentForwardingEntity() const { return m_forwardingEntityID; }
	virtual TFlowInputData* GetInputData() const        { return m_pInputData.get(); };
	//////////////////////////////////////////////////////////////////////////
	// ~IFlowNodeData
	//////////////////////////////////////////////////////////////////////////

	virtual bool SetEntityId(const EntityId id);

	EntityId     GetEntityId()
	{
		if (m_hasEntity)
		{
			EntityId id = INVALID_ENTITYID;
			if (m_pInputData[0].GetValueWithConversion(id))
			{
				return id;
			}
		}
		return INVALID_ENTITYID;
	}

	void CompleteActivationInfo(IFlowNode::SActivationInfo*);

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject((tuk)m_pInputData.get() - 4, (sizeof(*m_pInputData.get()) * m_nInputs) + 4);

		for (i32 i = 0; i < m_nInputs; i++)
		{
			pSizer->AddObject(m_pInputData[i]);
		}
	}
	bool DoForwardingIfNeed(IFlowNode::SActivationInfo*);

	ILINE bool HasEntity() const { return m_hasEntity; }

private:
	// REMEMBER: When adding members update CFlowData::Swap(CFlowData&)
	// should be well packed
	std::unique_ptr<TFlowInputData[]> m_pInputData;
	std::unique_ptr<i32[]> m_pOutputFirstEdge;
	IFlowNodePtr m_pImpl;
	string m_name;
	HSCRIPTFUNCTION m_getFlowgraphForwardingEntity;
	EntityId m_forwardingEntityID;
	u8  m_nInputs;
	u8  m_nOutputs;
	u16 m_typeId : TYPE_BITS;
	u16 m_hasEntity : 1; // note: subsequent bitfields need to have the same variable type to be packed together in msvc (it's implementation defined ch.9.6)
	u16 m_failedGettingFlowgraphForwardingEntity : 1;
	DrxGUID m_entityGuid;

	void              DoGetConfiguration(SFlowNodeConfig& config) const;
	bool              ForwardingActivated(IFlowNode::SActivationInfo*, IFlowNode::EFlowEvent);
	bool              NoForwarding(IFlowNode::SActivationInfo*);
	static ILINE bool HasForwarding(IFlowNode::SActivationInfo*) { return true; }
	void              ClearInputActivations();
};

ILINE void CFlowData::ClearInputActivations()
{
	for (i32 i = 0; i < m_nInputs; i++)
		m_pInputData[i].ClearUserFlag();
}

ILINE void CFlowData::CompleteActivationInfo(IFlowNode::SActivationInfo* pActInfo)
{
	pActInfo->pInputPorts = m_pInputData.get() + m_hasEntity;
}

ILINE void CFlowData::Activated(IFlowNode::SActivationInfo* pActInfo, IFlowNode::EFlowEvent event)
{
	//	DRX_PROFILE_REGION(PROFILE_ACTION, m_type.c_str());
	if (m_hasEntity && (m_getFlowgraphForwardingEntity || m_failedGettingFlowgraphForwardingEntity))
	{
		if (ForwardingActivated(pActInfo, event))
		{
			ClearInputActivations();
			return;
		}
	}

	CompleteActivationInfo(pActInfo);
	if (m_hasEntity && m_pInputData[0].IsUserFlagSet())
		m_pImpl->ProcessEvent(IFlowNode::eFE_SetEntityId, pActInfo);

	m_pImpl->ProcessEvent(event, pActInfo);

	// now clear any ports
	ClearInputActivations();
}

ILINE void CFlowData::Update(IFlowNode::SActivationInfo* pActInfo)
{
	//	DRX_PROFILE_REGION(PROFILE_ACTION, m_type.c_str());
	CompleteActivationInfo(pActInfo);
	if (m_hasEntity && (m_getFlowgraphForwardingEntity || m_failedGettingFlowgraphForwardingEntity))
		if (ForwardingActivated(pActInfo, IFlowNode::eFE_Update))
			return;
	m_pImpl->ProcessEvent(IFlowNode::eFE_Update, pActInfo);
}

ILINE void CFlowData::SetSuspended(IFlowNode::SActivationInfo* pActInfo, bool suspended)
{
	CompleteActivationInfo(pActInfo);
	m_pImpl->ProcessEvent(suspended ? IFlowNode::eFE_Suspend : IFlowNode::eFE_Resume, pActInfo);
}

#endif
