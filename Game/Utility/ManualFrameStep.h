// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Player ability manager.
-------------------------------------------------------------------------
История:
- 14:04:2014: Created by Jean Geffroy
*************************************************************************/

#ifndef __MANUAL_FRAME_STEP_H__
#define __MANUAL_FRAME_STEP_H__

#include <drx3D/Game/GameRulesModules/IGameRulesModuleRMIListener.h>

class CManualFrameStepUpr
	: public IInputEventListener
	, public IGameRulesModuleRMIListener
{
public:
	CManualFrameStepUpr();
	virtual ~CManualFrameStepUpr();

	bool Update();
	
	bool IsEnabled() const;
	void Enable(bool enable);
	void Toggle();
	void RequestFrames(u8 numFrames);

	// IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& inputEvent);
	// ~IInputEventListener

	// IGameRulesModuleRMIListener
	virtual void OnSingleEntityRMI(CGameRules::SModuleRMIEntityParams params);
	virtual void OnDoubleEntityRMI(CGameRules::SModuleRMITwoEntityParams params){}
	virtual void OnEntityWithTimeRMI(CGameRules::SModuleRMIEntityTimeParams params){}
	virtual void OnSvClientActionRMI(CGameRules::SModuleRMISvClientActionParams params, EntityId fromEid){}
	// ~IGameRulesModuleRMIListener

private:
	i32         m_rmiListenerIdx;
	i32       m_framesLeft;
	float       m_previousFixedStep;
};

#endif
