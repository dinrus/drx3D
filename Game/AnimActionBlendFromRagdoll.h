// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание:
	Implements the Blend From Ragdoll AnimAction

-------------------------------------------------------------------------
История:
- 06.07.12: Created by Stephen M. North

*************************************************************************/
#pragma once

#ifndef __AnimActionBlendFromRagdoll_h__
#define __AnimActionBlendFromRagdoll_h__

#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/IAnimationPoseModifier.h>
#include <drx3D/Act/IGameRulesSystem.h>

#define USE_BLEND_FROM_RAGDOLL

class CActor;
class CAnimActionBlendFromRagdoll : public TAction< SAnimationContext >
{
	DEFINE_ACTION( "AnimActionBlendFromRagdoll" );

public:
	CAnimActionBlendFromRagdoll( i32 priority, CActor& actor, const FragmentID& fragID, const TagState fragTags = TAG_STATE_EMPTY );

protected:
	typedef std::vector<uint> TAnimationIds;
	typedef TAction< SAnimationContext > TBase;

	virtual void OnInitialise() override;
	virtual void Enter() override;
	virtual void Exit() override;
	virtual EStatus Update(float timePassed) override;

	virtual void OnFragmentStarted() override;

	void DispatchPoseModifier();
	void QueryPose();
	void GenerateAnimIDs();

private:
	CActor& m_actor;
	IAnimationPoseMatchingPtr m_pPoseMatching;
	TagState m_fragTagsTarget;
	TAnimationIds m_animIds;
	bool m_bSetAnimationFrag;
	uint m_animID;
};


class CAnimActionBlendFromRagdollSleep : public TAction< SAnimationContext >
{
	DEFINE_ACTION( "AnimActionBlendFromRagdollSleep" );

public:
	CAnimActionBlendFromRagdollSleep( i32 priority, CActor& actor, const HitInfo& hitInfo, const TagState& sleepTagState, const TagState& fragTags = TAG_STATE_EMPTY );

protected:
	typedef TAction< SAnimationContext > TBase;

	virtual void OnInitialise() override;
	virtual void Enter() override;

private:
	HitInfo m_hitInfo;
	TagState m_fragTagsTarget;

	CActor& m_actor;
};

#endif // __AnimActionBlendFromRagdoll_h__
