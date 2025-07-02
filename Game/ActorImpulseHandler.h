// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание: Class that manages the logic behind impulses on CActors
-------------------------------------------------------------------------
История:
- 20:10:2010	10:46 : Created by David Ramos
*************************************************************************/
#if _MSC_VER > 1000
	# pragma once
#endif

#ifndef __ACTOR_IMPULSE_HANDLER_H
#define __ACTOR_IMPULSE_HANDLER_H

#include <drx3D/Act/ISharedParams.h>
#include <drx3D/Animation/IDrxAnimation.h>

#include <drx3D/Game/GameRules.h>

class CActor;

//////////////////////////////////////////////////////////////////////////
struct SHitImpulse
{
	SHitImpulse( i32 partId, i32 matId, const Vec3 &pos, const Vec3 &impulse, float fPushImpulseScale = 0.0f, i32 projectileClass = -1 )
		: m_partId(partId)
		, m_matId(matId)
		, m_pos(pos)
		, m_impulse(impulse)
		, m_fPushImpulseScale(fPushImpulseScale)
		, m_projectileClass(projectileClass)
	{
	}

	i32 m_projectileClass;
	i32 m_partId;
	i32 m_matId;
	const Vec3 m_pos;
	const Vec3 m_impulse;
	float m_fPushImpulseScale;
};
//////////////////////////////////////////////////////////////////////////
class CActorImpulseHandler
{
public:
	CActorImpulseHandler(CActor& actor);

	static i32k NUM_JOINTS = 3;

	void	ReadXmlData(const IItemParamsNode* pRootNode);

	//////////////////////////////////////////////////////////////////////////
	// Applies an impulse on the associated actor, filtering it through the BodyDamage
	// system and triggering the proper events and stats. Mainly for impulses on alive actors
	// Params:
	// - fPushImpulseScale. Impulses are applied both to the alive character and the living entity, this parameter scales
	//											the impulse on the living entity
	void	AddLocalHitImpulse( const SHitImpulse& hitImpulse );

	//////////////////////////////////////////////////////////////////////////
	// Applies a set of impulses on the associated actor's ragdoll, based on the last hit info context, following
	// some parameterization on this actor's params
	void	ApplyDeathImpulse(const HitInfo& lastHit);
	void  QueueDeathImpulse(const HitInfo& hitInfo, const float delay);

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Defers an impulse to be applied immediately when the eGFE_RagdollPhysicalized event is received
	void SetOnRagdollPhysicalizedImpulse(pe_action_impulse& impulse);

	void Update(float frameTime);
	void UpdateDeath(float frameTime);

	void OnRagdollPhysicalized();

private:
	// Private types
	struct SImpulseSet
	{
		struct SImpulse
		{
			SImpulse();

			i32			iPartID;
			Vec3		vDirection;
			float		fStrength;
			bool		bUseDirection;
			bool		bUseStrength;
		};
		typedef std::vector<SImpulse> ImpulseContainer;
		typedef VectorSet<i32>				IDContainer;


		SImpulseSet();
		bool MatchesHitInfo(const HitInfo& hitInfo) const;


		IDContainer				allowedHitTypes;
		IDContainer				allowedBulletTypes;
		IDContainer				allowedPartIDs;

		float							fAngularImpulseScale;
		ImpulseContainer	impulses;
	};
	typedef std::vector<SImpulseSet> ImpulseSetsContainer;

public:
	BEGIN_SHARED_PARAMS(SSharedImpulseHandlerParams)
		SSharedImpulseHandlerParams();

		float fMaxRagdollImpulse;
		float fMass;
		ImpulseSetsContainer impulseSets;
	END_SHARED_PARAMS
private:

	struct QueuedDeathImpulse
	{
		HitInfo hitInfo;
		float   timeOut;

		QueuedDeathImpulse() : timeOut(0.0f) {}
	};

	struct SFindMatchingSet;

	enum EImpulseState
	{
		Imp_None,
		Imp_Impulse,
		Imp_Recovery
	};

	// Private methods
	void ApplyDeathAngularImpulse(float fAngularImpulseScale, const Vec3& hitpos, const Vec3& playerPos, const HitInfo& lastHit);
	void LoadDeathImpulses(const IItemParamsNode* pDeathImpulses, ImpulseSetsContainer& impulseSets);

	// Private Attributes
	CActor&															m_actor;
	SSharedImpulseHandlerParamsConstPtr	m_pParams;
	IAnimationOperatorQueuePtr m_poseModifier;

	QueuedDeathImpulse	m_queuedDeathImpulse;
	
	pe_action_impulse m_onRagdollizeEventImpulse; 
	bool m_delayedDeathImpulse;
	bool m_onRagdollizeEventImpulseQueued; 
	i32  m_jointIdx[NUM_JOINTS];
	Quat m_targetOffsets[NUM_JOINTS];
	Quat m_initialOffsets[NUM_JOINTS];
	float m_impulseTime;
	float m_impulseDuration;
	EImpulseState		m_impulseState;

#ifndef _RELEASE
	struct SDebugImpulse
	{
		SDebugImpulse()
			: time(0.f), applied(false), valid(false)
		{}
		pe_action_impulse impulse;
		float time;
		bool applied;
		bool valid;
	};
	i32 m_currentDebugImpulse;
	static i32k kDebugImpulseMax = 4;
	SDebugImpulse m_debugImpulses[kDebugImpulseMax];
#endif //_RELEASE

};

#endif // __ACTOR_IMPULSE_HANDLER_H