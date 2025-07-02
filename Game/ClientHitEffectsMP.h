// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CLIENT_HIT_EFFECTS_MP_H__
#define __CLIENT_HIT_EFFECTS_MP_H__

#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Act/IMaterialEffects.h>
#include <drx3D/Entity/IEntityClass.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/CoreX/ParticleSys/IParticles.h>

class CActor;
class CPlayer;
class CGameRules;

class CClientHitEffectsMP
{
protected:
	struct SHitEffectInfo
	{
		SHitEffectInfo() : pAmmoClass(NULL), effectId(InvalidEffectId) {};

		IEntityClass* pAmmoClass;
		TMFXEffectId effectId;
	};

	typedef std::vector<SHitEffectInfo> THitEffectVector;

	struct SHitEffectInfoSet
	{
		SHitEffectInfoSet() : m_default(InvalidEffectId), m_melee(InvalidEffectId) {};

		TMFXEffectId m_default;
		TMFXEffectId m_melee;
		THitEffectVector m_effectInfos;
	};

	typedef std::vector<SHitEffectInfoSet> THitEffectSetVector;

public:

	CClientHitEffectsMP();
	virtual ~CClientHitEffectsMP();
	void Feedback(const CGameRules* pGameRules, const CPlayer* pTargetPlayer, const HitInfo &hitInfo);
	void KillFeedback(const CActor* pTarget, const HitInfo &hitInfo);
	void Initialise();
	
protected:
	void SpawnMaterialEffect(const CGameRules* pGameRules, const CPlayer* pTargetPlayer, const HitInfo &hitInfo, bool inArmour);
	TMFXEffectId FindEffectIdForClass(IEntityClass* pEntityClass, SHitEffectInfoSet& hitEffectInfos);
	void ProcessEffectInfo(SHitEffectInfoSet& hitEffectSet, XmlNodeRef xmlNode, tukk libraryName);

	CAudioSignalPlayer m_hitTargetSignal;
	CAudioSignalPlayer m_hitTargetHeadshotSignal;
	TAudioSignalID m_hitTargetMelee;
	
	SHitEffectInfoSet m_normalEffects;
	THitEffectSetVector m_armourTeamEffects;
};

#endif
