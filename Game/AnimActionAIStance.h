// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ANIM_ACTION_AI_STANCE__H__
#define __ANIM_ACTION_AI_STANCE__H__

#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/AI/IAgent.h>

class CPlayer;

class CAnimActionAIStance
	: public TAction< SAnimationContext >
{
public:
	typedef TAction< SAnimationContext > TBase;

	DEFINE_ACTION( "AIStance" );

	CAnimActionAIStance( i32 priority, CPlayer* pPlayer, const EStance targetStance );

	virtual void Enter() override;
	virtual void Exit() override;
	virtual EStatus Update( float elapsedSeconds ) override;
	virtual EStatus UpdatePending( float timePassed ) override;


	virtual void OnInitialise() override;

	EStance GetTargetStance() const { return m_targetStance; }
	bool IsPlayerAnimationStanceSet() const { return m_isPlayerAnimationStanceSet; }

	ActionScopes FindStanceActionScopeMask( const SAnimationContext& context ) const;
	bool FragmentExistsInDatabase( const SAnimationContext& context, const IAnimationDatabase& database ) const;
private:
	void SetMovementParameters();
	void RestoreMovementParameters();

	void SetPlayerAnimationStanceOnce();
	EStance GetPlayerAnimationStance() const;

	bool FindFragmentInfo( const SAnimationContext& context, FragmentID& fragmentIdOut, SFragTagState& fragTagStateOut ) const;

private:
	CPlayer* const m_pPlayer;
	const EStance m_targetStance;
	bool m_isPlayerAnimationStanceSet;
};


#endif
