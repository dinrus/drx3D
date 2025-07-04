// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 12:05:2009   Created by Federico Rebora
*************************************************************************/

#pragma once

#ifndef COLOR_GRADIENT_NODE_H
#define COLOR_GRADIENT_NODE_H

#include <drx3D/Game/G2FlowBaseNode.h>

//namespace EngineFacade
//{
//	struct IGameEnvironment;
//}

class CFlowNode_ColorGradient : public CFlowBaseNode<eNCT_Instanced>
{
public:
	static const SInputPortConfig inputPorts[];

	CFlowNode_ColorGradient( SActivationInfo* activationInformation);
	~CFlowNode_ColorGradient();

	virtual void GetConfiguration(SFlowNodeConfig& config);
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* activationInformation);
	virtual void GetMemoryUsage(IDrxSizer* sizer) const;

	virtual IFlowNodePtr Clone( SActivationInfo *pActInfo ) { return new CFlowNode_ColorGradient(pActInfo); }

	enum EInputPorts
	{
		eInputPorts_Trigger,
		eInputPorts_TexturePath,
		eInputPorts_TransitionTime,
		eInputPorts_Count,
	};

private:
//	IGameEnvironment& m_environment;
	ITexture *m_pTexture;
};

#endif 
