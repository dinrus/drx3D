// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FLOWLOGNODE_H__
#define __FLOWLOGNODE_H__

#pragma once

#include <drx3D/FlowGraph/IFlowSystem.h>

class CFlowLogNode : public IFlowNode
{
public:
	CFlowLogNode();

	// IFlowNode
	virtual IFlowNodePtr Clone(SActivationInfo*);
	virtual void         GetConfiguration(SFlowNodeConfig&);
	virtual void         ProcessEvent(EFlowEvent event, SActivationInfo*);
	virtual bool         SerializeXML(SActivationInfo*, const XmlNodeRef&, bool);
	virtual void         Serialize(SActivationInfo*, TSerialize ser);
	virtual void         PostSerialize(SActivationInfo*) {}

	virtual void         GetMemoryUsage(IDrxSizer* s) const
	{
		s->Add(*this);
	}
	// ~IFlowNode
};

#endif
