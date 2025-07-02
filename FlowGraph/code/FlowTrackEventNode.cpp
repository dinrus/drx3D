// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   FlowTrackEventNode.cpp
   Описание: Dynamic node for Track Event logic
   ---------------------------------------------------------------------
   История:
   - 10:04:2008 : Created by Kevin Kirst

 *********************************************************************/

#include <drx3D/FlowGraph/StdAfx.h>
#include <drx3D/FlowGraph/FlowTrackEventNode.h>

CFlowTrackEventNode::CFlowTrackEventNode(SActivationInfo* pActInfo)
	: m_outputs(1)//one empty
	, m_actInfo(*pActInfo)
{
}

CFlowTrackEventNode::~CFlowTrackEventNode()
{
	if (m_pSequence != nullptr && !gEnv->IsEditor())
	{
		m_pSequence->RemoveTrackEventListener(this);
		m_pSequence = nullptr;
	}
}

IFlowNodePtr CFlowTrackEventNode::Clone(SActivationInfo* pActInfo)
{
	CFlowTrackEventNode* pClone = new CFlowTrackEventNode(*this);
	return pClone;
}

void CFlowTrackEventNode::GetConfiguration(SFlowNodeConfig& config)
{
	static const SInputPortConfig inputs[] = {
		// Note: Must be first!
		InputPortConfig<string>("seq_Sequence",  "", _HELP("Working animation sequence"), _HELP("Sequence"),   0),
		InputPortConfig<i32>("seqid_SequenceId", 0,  _HELP("Working animation sequence"), _HELP("SequenceId"), 0),
		{ 0 }
	};

	config.pInputPorts = inputs;
	config.pOutputPorts = &m_outputs[0];
	config.SetCategory(EFLN_APPROVED);
	config.nFlags |= EFLN_DYNAMIC_OUTPUT;
	config.nFlags |= EFLN_HIDE_UI;
}

void CFlowTrackEventNode::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
{
	if (event == eFE_Initialize && !gEnv->IsEditor())
	{
		AddListener(pActInfo);
	}
}

bool CFlowTrackEventNode::SerializeXML(SActivationInfo* pActInfo, const XmlNodeRef& root, bool reading)
{
	if (reading)
	{
		i32 count = root->getChildCount();

		m_outputs.resize(count + 1); //the last one is kept empty value

		for (i32 i = 0; i < count; ++i)
		{
			XmlNodeRef child = root->getChild(i);
			m_outputStrings.push_back(child->getAttr("Name"));
			m_outputs[i] = OutputPortConfig<string>(m_outputStrings[i]);
		}
	}
	return true;
}

void CFlowTrackEventNode::Serialize(SActivationInfo* pActInfo, TSerialize ser)
{
	if (ser.IsReading() && !gEnv->IsEditor())
	{
		AddListener(pActInfo);
	}
}

void CFlowTrackEventNode::GetMemoryUsage(class IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_outputStrings);
	pSizer->AddObject(m_outputs);
}

void CFlowTrackEventNode::AddListener(SActivationInfo* pActInfo)
{
	DRX_ASSERT(pActInfo != nullptr);
	m_actInfo = *pActInfo;

	// Remove from old
	if (m_pSequence != nullptr)
	{
		m_pSequence->RemoveTrackEventListener(this);
		m_pSequence = nullptr;
	}

	// Look up sequence
	i32k kSequenceName = 0;
	i32k kSequenceId = 1;
	m_pSequence = gEnv->pMovieSystem->FindSequenceById((u32)GetPortInt(pActInfo, kSequenceId));
	if (m_pSequence == nullptr)
	{
		string name = GetPortString(pActInfo, kSequenceName);
		m_pSequence = gEnv->pMovieSystem->FindSequence(name.c_str());
	}
	if (m_pSequence != nullptr)
		m_pSequence->AddTrackEventListener(this);
}

void CFlowTrackEventNode::OnTrackEvent(IAnimSequence* pSequence, i32 reason, tukk event, uk pUserData)
{
	if (reason != ITrackEventListener::eTrackEventReason_Triggered)
		return;

	// Find output port and call it
	for (i32 i = 0, nOutput = m_outputs.size(); i < nOutput; ++i)
	{
		if (m_outputs[i].name != nullptr && strcmp(m_outputs[i].name, event) == 0)
		{
			// Call it
			TFlowInputData value;
			tukk param = (tukk)pUserData;
			value.Set(string(param));
			ActivateOutput(&m_actInfo, i, value);
			return;
		}
	}
}
