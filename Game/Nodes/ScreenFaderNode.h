// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 3:06:2009   Created by Benito G.R.
*************************************************************************/

#pragma once

#ifndef SCREEN_FADER_NODE_H
#define SCREEN_FADER_NODE_H

#include <drx3D/Game/GameFlowBaseNode.h>

namespace EngineFacade
{
	struct IGameEnvironment;
}

class CFlowNode_ScreenFader : public CGameFlowBaseNode
{
public:
	static const SInputPortConfig inputPorts[];

	CFlowNode_ScreenFader(IGameEnvironment& environment, SActivationInfo* activationInformation);

	virtual void GetConfiguration(SFlowNodeConfig& config);
	virtual void ProcessActivateEvent(SActivationInfo* activationInformation);
	virtual void GetMemoryStatistics(IDrxSizer* sizer);

	enum EInputPorts
	{
		eInputPorts_FadeIn,
		eInputPorts_FadeOut,
		eInputPorts_FadeInTime,
		eInputPorts_FadeOutTime,
		eInputPorts_FadeColor,
		eInputPorts_UseCurrentColor,
		eInputPorts_Texture,
		eInputPorts_Count,
	};

private:
	IGameEnvironment& m_environment;
};

#endif 