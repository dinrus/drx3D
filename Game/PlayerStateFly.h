// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Implements the Player fly state

-------------------------------------------------------------------------
История:
- 27.10.10: Created by Stephen M. North

*************************************************************************/
#ifndef __PlayerStateFly_h__
#define __PlayerStateFly_h__

#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>

class CPlayer;
struct SActorFrameMovementParams;
class CPlayerStateFly
{
public:
	CPlayerStateFly();

	void OnEnter( CPlayer& player );
	bool OnPrePhysicsUpdate( CPlayer& player, const SActorFrameMovementParams &movement, float frameTime );
	void OnExit( CPlayer& player );

private:

	u8 m_flyMode;
	float m_flyModeDisplayTime;
	void ProcessFlyMode( CPlayer& player, const SActorFrameMovementParams &movement );

};

class CPlayerStateSpectate : public CPlayerStateFly, public IHUDEventListener
{
private:
	#define STAY_FADED_TIME 0.5f

	typedef CPlayerStateFly inherited;

	float m_fFadeOutAmount;
	float m_fFadeForTime;

public:
	CPlayerStateSpectate();
	void ResetFadeParameters() { m_fFadeOutAmount = 1.0f; m_fFadeForTime = STAY_FADED_TIME; }

	virtual void OnHUDEvent(const SHUDEvent& event);

	void OnEnter( CPlayer& player );
	void UpdateFade( float frameTime );
	void OnExit( CPlayer& player );

protected:
	void DrawSpectatorFade();
};

#endif // __PlayerStateFly_h__
