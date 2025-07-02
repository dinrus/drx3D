// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ANIM_ACTION_AI_COVER_ACTION__H__
#define __ANIM_ACTION_AI_COVER_ACTION__H__

#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/AI/IAgent.h>

class CPlayer;

#define COVER_ACTION_STORE_NAME() 1


class CAnimActionAIChangeCoverBodyDirection
	: public TAction< SAnimationContext >
{
public:
	typedef TAction< SAnimationContext > TBase;

	DEFINE_ACTION( "AIChangeCoverBodyDirection" );

	CAnimActionAIChangeCoverBodyDirection( i32 priority, CPlayer* pPlayer, const ECoverBodyDirection targetCoverBodyDirection );

	virtual EPriorityComparison ComparePriority( const IAction& actionCurrent ) const override { return Equal; }

	virtual void OnInitialise() override;

	virtual void Enter() override;
	virtual void Exit() override;

	ECoverBodyDirection GetTargetCoverBodyDirection() const { return m_targetCoverBodyDirection; }
	bool IsTargetCoverBodyDirectionSet() const { return m_isTargetCoverBodyDirectionSet; }

private:
	void SetPlayerAnimationInProgressCoverBodyDirection( const ECoverBodyDirection coverBodyDirection );

	// After calling this method once, IsTargetCoverBodyDirectionSet() will return
	// true and further calls won't do anything anymore
	void SetPlayerAnimationCoverBodyDirectionOnce();

	void SetAnimationControlledMovementParameters();
	void SetEntityControlledMovementParameters();

private:
	CPlayer* const m_pPlayer;
	ECoverBodyDirection m_targetCoverBodyDirection;
	bool m_isTargetCoverBodyDirectionSet;
};





class CAnimActionAICoverAction
	: public TAction< SAnimationContext >
{
public:
	enum EState
	{
		eTransitionIn,
		eTransitionOut,
		eAction,
		eStatesCount,
		eNone = eStatesCount,
	};

public:
	typedef TAction< SAnimationContext > TBase;

	DEFINE_ACTION( "AICoverAction" );

	CAnimActionAICoverAction( i32 priority, CPlayer* pPlayer, tukk actionName );

	virtual EPriorityComparison ComparePriority( const IAction& actionCurrent ) const override { return Equal; }

	virtual void OnInitialise() override;

	virtual void Enter() override;
	virtual void Exit() override;

	virtual void OnSequenceFinished( i32 layer, u32 scopeID ) override;

	void CancelAction();

	EState GetState() const { return m_state; }

	bool IsTargetActionName( tukk actionName ) const;

	ActionScopes FindCoverActionScopeMask( const SAnimationContext& context ) const;

private:
	void SetAnimationControlledMovementParameters();
	void SetEntityControlledMovementParameters();

	void SetCurrentState( const EState state );

	void RequestIdleMotionDetail();
	void RequestNothingMotionDetail();

private:
	CPlayer* const m_pPlayer;

	typedef SCRCRef< COVER_ACTION_STORE_NAME() > TCoverActionName;
	const TCoverActionName m_action;
	u32 m_toActionCrc;
	u32 m_fromActionCrc;

	struct SStateInfo
	{
		enum EType
		{
			eType_None,
			eType_Normal,
			eType_Transition,
		};

		FragmentID m_fragmentId;
		TagState m_fragmentTags;
		EType m_stateType;

		SStateInfo()
			: m_fragmentId( FRAGMENT_ID_INVALID )
			, m_fragmentTags( TAG_STATE_EMPTY )
			, m_stateType( eType_Normal )
		{
		}

		bool Init( const SAnimationContext& context, u32k stateFragmentNameCrc, u32k actionNameCrc, const EType stateType )
		{
			m_fragmentId = context.controllerDef.m_fragmentIDs.Find( stateFragmentNameCrc );
			if ( m_fragmentId == FRAGMENT_ID_INVALID )
			{
				return false;
			}

			const CTagDefinition* pFragmentTagDefinition = context.controllerDef.GetFragmentTagDef( m_fragmentId );
			m_fragmentTags = TAG_STATE_EMPTY;

			if ( ! pFragmentTagDefinition )
			{
				return false;
			}

			const TagID actionTagId = pFragmentTagDefinition->Find( actionNameCrc );
			if ( actionTagId == TAG_ID_INVALID )
			{
				return false;
			}

			pFragmentTagDefinition->Set( m_fragmentTags, actionTagId, true );

			m_stateType = stateType;
			return true;
		}
	};

	SStateInfo m_actionStates[ eStatesCount ];

	EState m_state;
	bool m_canceled;
};


#endif
