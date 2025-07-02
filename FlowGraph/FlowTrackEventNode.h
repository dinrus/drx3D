// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   FlowTrackEventNode.h
   Описание: Dynamic node for Track Event logic
   ---------------------------------------------------------------------
   История:
   - 10:04:2008 : Created by Kevin Kirst
   - Apr 11 2017  Modified by Fei Teng

 *********************************************************************/

#pragma once

#include <drx3D/FlowGraph/IFlowBaseNode.h>
#include <drx3D/Movie/IMovieSystem.h>

class CFlowTrackEventNode : public CFlowBaseNode<eNCT_Instanced>, public ITrackEventListener
{
public:
	CFlowTrackEventNode(SActivationInfo*);
	CFlowTrackEventNode(CFlowTrackEventNode const& obj) = default;
	CFlowTrackEventNode& operator=(CFlowTrackEventNode const& obj) = default;
	CFlowTrackEventNode(CFlowTrackEventNode&& obj) = default;
	CFlowTrackEventNode& operator=(CFlowTrackEventNode&& obj) = default;
	virtual ~CFlowTrackEventNode();

	// IFlowNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo) override;
	virtual void         GetConfiguration(SFlowNodeConfig& config) override;
	virtual void         ProcessEvent(EFlowEvent event, SActivationInfo*) override;
	virtual bool         SerializeXML(SActivationInfo* pActInfo, const XmlNodeRef& root, bool reading) override;
	virtual void         Serialize(SActivationInfo*, TSerialize ser) override;
	virtual void         GetMemoryUsage(class IDrxSizer* pSizer) const override;

	// ITrackEventListener
	virtual void OnTrackEvent(IAnimSequence* pSequence, i32 reason, tukk event, uk pUserData) override;
protected:
	// Add to sequence listener
	void AddListener(SActivationInfo* pActInfo);

private:
	std::vector<string> m_outputStrings;
	std::vector<SOutputPortConfig> m_outputs;

	SActivationInfo    m_actInfo;
	IAnimSequence*     m_pSequence = nullptr;
};
