// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/HitDeathReactionsDefs.h>
#include <drx3D/Game/HitDeathReactionsSystem.h>
#include <drx3D/Game/HitDeathReactions.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/CoreX/String/NameCRCHelper.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
SRandomGeneratorFunct::SRandomGeneratorFunct(CMTRand_int32& pseudoRandomGenerator) : m_pseudoRandomGenerator(pseudoRandomGenerator) {} 

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::GetMemoryUsage(IDrxSizer* s) const
{
	s->AddObject(this, sizeof(*this));

	s->AddObject(validationParams);

	s->AddObject(sCustomExecutionFunc);
	s->AddObject(sCustomExecutionEndFunc);
	s->AddObject(sCustomAISignal);

	//agReaction size
	s->AddObject(&agReaction, sizeof(agReaction));
	s->AddObject(agReaction.sAGInputValue);
	s->AddObject(agReaction.variations);

	// reactionAnim size
	s->AddObject(reactionAnim.get(), sizeof(reactionAnim));
	s->AddObject(reactionAnim->animCRCs);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
SReactionParams::SReactionParams() : orientationSnapAngle(0), reactionOnCollision(NO_COLLISION_REACTION), 
flags(0), bPauseAI(true), endVelocity(ZERO)
{

}

///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::Reset()
{
	validationParams.clear();

	// Execution params
	sCustomExecutionFunc.clear();

	sCustomAISignal.clear();
	orientationSnapAngle = 0;
	reactionOnCollision = NO_COLLISION_REACTION;
	flags = 0;
	bPauseAI = true;
	agReaction.Reset();
	mannequinData.Reset();
	if (reactionAnim != NULL)
		reactionAnim->Reset();
	reactionScriptTable = ScriptTablePtr();
	endVelocity.zero();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
SReactionParams::SValidationParams::SValidationParams() : fMinimumSpeedAllowed(0.0f), fMaximumSpeedAllowed(FLT_MAX), 
fMinimumDamageAllowed(0.0f), fMaximumDamageAllowed(FLT_MAX), fProbability(1.0f), 
shotOrigin(eCD_Invalid), movementDir(eCD_Invalid), bAllowOnlyWhenUsingMountedItems(false), airState(eAS_Unset), destructibleEvent(0),
fMinimumDistance(0.0f), fMaximumDistance(0.0f) 
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SValidationParams::Reset()
{
	sCustomValidationFunc.clear();
	fMinimumSpeedAllowed = 0.0f;
	fMaximumSpeedAllowed = std::numeric_limits<float>::max();
	fMinimumDamageAllowed = 0.0f;
	fMaximumDamageAllowed = std::numeric_limits<float>::max();
	healthThresholds.clear();
	fMinimumDistance = 0.0f;
	fMaximumDistance = 0.0f;
	allowedPartIds.clear();
	shotOrigin = eCD_Invalid; 
	movementDir = eCD_Invalid;
	fProbability = 1.0f;
	allowedStances.clear();
	allowedHitTypes.clear();
	allowedProjectiles.clear();
	allowedWeapons.clear();
	bAllowOnlyWhenUsingMountedItems = false;
	airState = eAS_Unset;
	destructibleEvent = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SValidationParams::GetMemoryUsage(IDrxSizer * s) const
{
	s->AddObject(sCustomValidationFunc);
	s->AddObject(allowedPartIds);
	s->AddObject(allowedStances);
	s->AddObject(allowedHitTypes);
	s->AddObject(allowedProjectiles);
	s->AddObject(allowedWeapons);
	s->AddObject(healthThresholds);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
SReactionParams::SReactionAnim::SReactionAnim()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
SReactionParams::SReactionAnim::~SReactionAnim()
{
	// Release reference to current and requested assets on removal
	ReleaseRequestedAnims();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SReactionAnim::Reset()
{
	animCRCs.clear();
	iLayer = 0;
	animFlags = DEFAULT_REACTION_ANIM_FLAGS;
	bAdditive = false;
	bNoAnimCamera = false;
	fOverrideTransTimeToAG = -1.0f;

	m_iNextAnimIndex = -1;
	m_nextAnimCRC = 0;
	m_iTimerHandle = 0;
	m_requestedAnimCRC = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
i32	SReactionParams::SReactionAnim::GetNextReactionAnimIndex() const
{
	// Lazily check if the requested asset has already been loaded
	UpdateRequestedAnimStatus();

	return m_iNextAnimIndex; // This will be -1 if the first asset hasn't been loaded yet
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
i32	SReactionParams::SReactionAnim::GetNextReactionAnimId(const IAnimationSet* pAnimSet) const
{
	DRX_ASSERT(pAnimSet);

	i32k iNextAnimIndex = GetNextReactionAnimIndex();

#ifndef _RELEASE
	// Paranoid check to catch logic errors (or DrxAnimation releasing assets we have locked)
	if (iNextAnimIndex >= 0)
	{
		const bool bIsAnimLoaded = gEnv->pCharacterUpr->CAF_IsLoaded(m_nextAnimCRC);
		DRX_ASSERT_MESSAGE(bIsAnimLoaded, "This anim was expected to have finished streaming!!");
		if (!bIsAnimLoaded)
			CHitDeathReactionsSystem::Warning("%s was expected to have finished streaming!!", pAnimSet->GetNameByAnimID(pAnimSet->GetAnimIDByCRC(animCRCs[iNextAnimIndex])));
	}
#endif

	return (iNextAnimIndex >= 0) ? pAnimSet->GetAnimIDByCRC(animCRCs[iNextAnimIndex]) : -1; 
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SReactionAnim::OnAnimLoaded() const
{
	DRX_ASSERT((m_nextAnimCRC != 0) || (m_iNextAnimIndex == -1) || !g_pGame->GetHitDeathReactionsSystem().IsStreamingEnabled());

	// Release previous one
	if (m_nextAnimCRC != 0)
	{
		gEnv->pCharacterUpr->CAF_Release(m_nextAnimCRC);
	}

	// Requested animation has been loaded. Update index
	++m_iNextAnimIndex %= animCRCs.size();
	m_nextAnimCRC = m_requestedAnimCRC;

	// We are no longer waiting for any request
	m_requestedAnimCRC = 0;

	// Remove timer if any
	if (m_iTimerHandle)
	{
		gEnv->pGame->GetIGameFramework()->RemoveTimer(m_iTimerHandle);
		m_iTimerHandle = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SReactionAnim::RequestNextAnim(const IAnimationSet* pAnimSet) const
{
	DRX_ASSERT(pAnimSet);

	// If we are already waiting for a requested anim to load there's no need to request a new one
	if (m_requestedAnimCRC != 0)
		return;

	// Request loading of the next index
	i32k iAnimCRCsSize = animCRCs.size();
	if (iAnimCRCsSize > 0)
	{
		i32 iIndexToRequest = 0;
		if (iAnimCRCsSize > 1)
		{
			if ((m_iNextAnimIndex + 1) >= iAnimCRCsSize)
			{
				// Randomly reshuffle animIDs vector
				// [*DavidR | 22/Sep/2010] Note: We are reusing the seed previously set on 
				// the random generator. Should be deterministic across the network.
				// This shuffling avoids playing the same animation twice in sequence
				SRandomGeneratorFunct randomFunctor(g_pGame->GetHitDeathReactionsSystem().GetRandomGenerator());
				std::random_shuffle(animCRCs.begin(), animCRCs.end() - 1, randomFunctor);
				std::iter_swap(animCRCs.begin(), animCRCs.end() - 1);

				m_iNextAnimIndex = 0;
			}

			iIndexToRequest = m_iNextAnimIndex + 1;
		}

		// Request reference
		if (g_pGame->GetHitDeathReactionsSystem().IsStreamingEnabled())
		{
			i32k requestedAnimModelID = pAnimSet->GetAnimIDByCRC(animCRCs[iIndexToRequest]);
			u32k requestedAnimCRC = pAnimSet->GetFilePathCRCByAnimID(requestedAnimModelID);
			if (requestedAnimCRC != m_nextAnimCRC)
			{
				m_requestedAnimCRC = requestedAnimCRC;
				gEnv->pCharacterUpr->CAF_AddRef(m_requestedAnimCRC);

				// Create a timer to poll when the asset ends streaming
				m_iTimerHandle = gEnv->pGame->GetIGameFramework()->AddTimer(CTimeValue(0.5f), false, functor(*this, &SReactionAnim::OnTimer), NULL);
			}
			else if (requestedAnimCRC != 0)
			{
				// Requested anim is the one we already have loaded. Immediate success :)
				m_iNextAnimIndex = iIndexToRequest;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SReactionAnim::ReleaseRequestedAnims()
{
	if (m_requestedAnimCRC != 0)
	{
		gEnv->pCharacterUpr->CAF_Release(m_requestedAnimCRC);
		m_requestedAnimCRC = 0;
	}

	if (m_nextAnimCRC != 0)
	{
		gEnv->pCharacterUpr->CAF_Release(m_nextAnimCRC);
		m_nextAnimCRC = 0;
	}

	// Remove the timer, since there's no need to poll for the end of the requested assets streaming
	if (m_iTimerHandle)
	{
		gEnv->pGame->GetIGameFramework()->RemoveTimer(m_iTimerHandle);
		m_iTimerHandle = 0;
	}

	m_iNextAnimIndex = -1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SReactionAnim::OnTimer(uk pUserData, IGameFramework::TimerID handler) const
{
	DRX_ASSERT(g_pGame->GetHitDeathReactionsSystem().IsStreamingEnabled());
	DRX_ASSERT(m_requestedAnimCRC != 0);

	m_iTimerHandle = 0;

	UpdateRequestedAnimStatus();

	// If still is not loaded, wait 0.5 seconds for the next
	if (m_requestedAnimCRC != 0)
	{
		m_iTimerHandle = gEnv->pGame->GetIGameFramework()->AddTimer(CTimeValue(0.5f), false, functor(*this, &SReactionAnim::OnTimer), NULL);	
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SReactionAnim::UpdateRequestedAnimStatus() const
{	
	if (!g_pGame->GetHitDeathReactionsSystem().IsStreamingEnabled() || 
		((m_requestedAnimCRC != 0) && gEnv->pCharacterUpr->CAF_IsLoaded(m_requestedAnimCRC)))
	{
		OnAnimLoaded();
	}
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SAnimGraphReaction::Reset()
{
	sAGInputValue.clear();
	variations.clear();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool SReactionParams::SMannequinData::IsCurrentFragmentLoaded() const
{
	return( (m_pCurrentFragment!=NULL) ? m_pCurrentFragment->IsLoaded() : true );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SMannequinData::Reset()
{
	m_pRequestedFragment.reset();
	m_iTimerHandle = 0;
	tagState = TAG_STATE_EMPTY;
	m_animIndexMP = 0;
	m_iNextOptionIndex = m_numOptions = OPTION_IDX_INVALID;
	actionType = EActionType_Invalid;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SMannequinData::Initialize( const IActionController* piActionController )
{
	DRX_ASSERT( piActionController );

	const TagState globalTags = piActionController->GetContext().state.GetMask();
	const FragmentID fragID = CHitDeathReactions::GetHitDeathFragmentID( piActionController );
	const SFragTagState fragTagState( TAG_STATE_FULL, tagState );
	SFragmentQuery query( fragID, fragTagState, 0 );

	m_pCurrentFragment.reset( new CFragmentCache( query ) );
	m_pCurrentFragment->PrecacheAnimsFromAllDatabases( piActionController );
	m_numOptions = m_pCurrentFragment->GetNumOptions();


/*	if( m_numOptions == OPTION_IDX_INVALID )
	{
		const IScope* piScope = piActionController->GetScope(0);
		if( piScope->HasDatabase() )
		{
			const IAnimationDatabase& animationDB = piScope->GetDatabase();
			const CTagDefinition* pTagDef = animationDB.GetFragmentDefs().GetSubTagDefinition( fragID );
			if( pTagDef )
			{
				char tmp[1024] = {0};
				pTagDef->FlagsToTagList( tagState, tmp, 1024 );
				DrxLogAlways( "***** InitData: %s", tmp );
			}
		}
	}*/

	if( (m_numOptions != OPTION_IDX_INVALID) && (m_numOptions > 0) )
	{
		m_iNextOptionIndex = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SMannequinData::AddDB( const IActionController* piActionController, const IAnimationDatabase* piAnimationDB ) const
{
	DRX_ASSERT( m_pCurrentFragment.get() );

	if( m_pCurrentFragment != NULL )
	{
		m_pCurrentFragment->PrecacheAnimsFromDatabase( piAnimationDB, piActionController );
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
i32	SReactionParams::SMannequinData::GetNextReactionAnimIndex() const
{
	u32 iNextOptionIndex = OPTION_IDX_RANDOM;

	// paranoid guard - could be a data setup problem
	if( (m_pCurrentFragment != NULL) && (m_pCurrentFragment->GetNumOptions() != OPTION_IDX_INVALID) && (m_pCurrentFragment->GetNumOptions() > 0) )
	{
		DRX_ASSERT( m_pCurrentFragment->IsLoaded() );

		if( m_iNextOptionIndex == OPTION_IDX_INVALID )
		{
			m_iNextOptionIndex = 0;
		}

		// need to return the current option as this may have been set at some point (e.g. net deserialize).
		iNextOptionIndex = m_iNextOptionIndex;

		m_iNextOptionIndex = g_pGame->GetHitDeathReactionsSystem().GetRandomGenerator().GenerateUint32() % m_pCurrentFragment->GetNumOptions();
	}
	else if( gEnv->bMultiplayer || (g_pGameCVars->g_hitDeathReactions_usePrecaching == 0) || (actionType == EActionType_Coop) )
	{
		return OPTION_IDX_RANDOM;
	}
	else
	{
		return OPTION_IDX_INVALID;
	}

	// Lazily check if the requested asset has already been loaded
	UpdateRequestedAnimStatus();

	return i32( iNextOptionIndex );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
i32	SReactionParams::SMannequinData::GetCurrentReactionAnimIndex() const
{
	// paranoid guard - could be a data setup problem
	if( (m_pCurrentFragment != NULL) && (m_pCurrentFragment->GetNumOptions() != OPTION_IDX_INVALID) && (m_pCurrentFragment->GetNumOptions() > 0) )
	{
		return m_iNextOptionIndex;
	}
	else if( gEnv->bMultiplayer || (g_pGameCVars->g_hitDeathReactions_usePrecaching == 0) || (actionType == EActionType_Coop) )
	{
		return OPTION_IDX_RANDOM;
	}
	else
	{
		return OPTION_IDX_INVALID;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SMannequinData::OnAnimLoaded() const
{
	DRX_ASSERT( g_pGame->GetHitDeathReactionsSystem().IsStreamingEnabled() );

	// Requested animation has been loaded.
	m_pCurrentFragment = m_pRequestedFragment;
	m_pRequestedFragment.reset();

	// Remove timer if any
	if (m_iTimerHandle)
	{
		gEnv->pGame->GetIGameFramework()->RemoveTimer(m_iTimerHandle);
		m_iTimerHandle = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SMannequinData::RequestNextAnim( const IActionController* piActionController ) const
{
	IF_UNLIKELY( (m_numOptions == OPTION_IDX_INVALID) || (m_iNextOptionIndex == OPTION_IDX_INVALID) || (m_numOptions == 0) ) 
	{
		return;
	}

	// If we are already waiting for a requested anim to load there's no need to request a new one
	if (m_pRequestedFragment != NULL)
		return;

	// Request reference
	if ( (m_pCurrentFragment != NULL) && g_pGame->GetHitDeathReactionsSystem().IsStreamingEnabled())
	{
		u32k optionIdx = m_pCurrentFragment->GetCurrentOption();
		if( (optionIdx != OPTION_IDX_RANDOM) && (optionIdx != m_iNextOptionIndex) )
		{
			ICharacterInstance* piCharacterInstance = piActionController->GetEntity().GetCharacter(0);
			const IAnimationSet* piAnimationSet = piCharacterInstance->GetIAnimationSet();

			const TagState globalTags = piActionController->GetContext().state.GetMask();
			const FragmentID fragID = CHitDeathReactions::GetHitDeathFragmentID( piActionController );
			m_pRequestedFragment.reset( new CFragmentCache( *m_pCurrentFragment, piActionController, m_iNextOptionIndex ) );

			// Create a timer to poll when the asset ends streaming
			m_iTimerHandle = gEnv->pGame->GetIGameFramework()->AddTimer(CTimeValue(0.5f), false, functor(*this, &SMannequinData::OnTimer), NULL);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SMannequinData::ReleaseRequestedAnims()
{
	m_pRequestedFragment.reset();
	m_pCurrentFragment.reset();

	// Remove the timer, since there's no need to poll for the end of the requested assets streaming
	if (m_iTimerHandle)
	{
		gEnv->pGame->GetIGameFramework()->RemoveTimer(m_iTimerHandle);
		m_iTimerHandle = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SMannequinData::OnTimer(uk pUserData, IGameFramework::TimerID handler) const
{
	DRX_ASSERT(g_pGame->GetHitDeathReactionsSystem().IsStreamingEnabled());
	DRX_ASSERT(m_pRequestedFragment != 0);

	m_iTimerHandle = 0;

	UpdateRequestedAnimStatus();

	// If still is not loaded, wait 0.5 seconds for the next
	if (m_pRequestedFragment != NULL)
	{
		m_iTimerHandle = gEnv->pGame->GetIGameFramework()->AddTimer(CTimeValue(0.5f), false, functor(*this, &SMannequinData::OnTimer), NULL);	
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SReactionParams::SMannequinData::UpdateRequestedAnimStatus() const
{	
	if (!g_pGame->GetHitDeathReactionsSystem().IsStreamingEnabled() || 
		((m_pRequestedFragment != NULL) && m_pRequestedFragment->IsLoaded()))
	{
		OnAnimLoaded();
	}
}
