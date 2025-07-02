// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/Network/SerializeFwd.h>
#include <drx3D/AI/IFactionMap.h> // <> требуется для Interfuscator

//! В этой структуре должна содержаться вся соответствующая информация для детерминации perception/visibility
// (восприятия/ видимости).
struct AgentPerceptionParameters
{
	// Способности восприятия.

	//! Как далеко может видеть агент (действующее лицо).
	float sightRange;
	//! Насколько близко видит агент.
	float sightNearRange;
	//! Как далеко видит агент, если мишенью является ТС (транспортное средство).
	float sightRangeVehicle;
	//! Как долго агент может наблюдать цель, пока не распознает её.
	float sightDelay;

	//! Нормальное и переферийное fov (градусы).
	float FOVPrimary;
	float FOVSecondary;
	//! Насколько высота цели влияет на видимость.
	float stanceScale;
	//! Чувствительность к звукам. 0=глухо, 1=нормально.
	float audioScale;

	// Воспринимаемые параметры
	float targetPersistence;

	float reactionTime;
	float collisionReactionScale;
	float stuntReactionTimeOut;

	float forgetfulness;             //!< overall scaling, controlled by FG.
	float forgetfulnessTarget;
	float forgetfulnessSeek;
	float forgetfulnessMemory;

	bool  isAffectedByLight;       //!< flag indicating if the agent perception is affected by light conditions.
	float minAlarmLevel;

	float bulletHitRadius;             //!< radius for perceiving bullet hits nearby
	float minDistanceToSpotDeadBodies; //!< min distance allowed to be able to spot a dead body

	float cloakMaxDistStill;
	float cloakMaxDistMoving;
	float cloakMaxDistCrouchedAndStill;
	float cloakMaxDistCrouchedAndMoving;

	struct SPerceptionScale { float visual; float audio; } perceptionScale;

	AgentPerceptionParameters()
		: sightRange(0)
		, sightNearRange(0.f)
		, sightRangeVehicle(-1)
		, sightDelay(0.f)
		, FOVPrimary(-1)
		, FOVSecondary(-1)
		, stanceScale(1.0f)
		, audioScale(1)
		, targetPersistence(0.f)
		, reactionTime(1.f)
		, collisionReactionScale(1.0f)
		, stuntReactionTimeOut(3.0f)
		, forgetfulness(1.f)
		, forgetfulnessTarget(1.f)
		, forgetfulnessSeek(1.f)
		, forgetfulnessMemory(1.f)
		, isAffectedByLight(false)
		, minAlarmLevel(0.0f)
		, bulletHitRadius(0.f)
		, minDistanceToSpotDeadBodies(10.0f)
		, cloakMaxDistStill(4.0f)
		, cloakMaxDistMoving(4.0f)
		, cloakMaxDistCrouchedAndStill(4.0f)
		, cloakMaxDistCrouchedAndMoving(4.0f)
	{
		perceptionScale.visual = 1.f;
		perceptionScale.audio = 1.f;
	}

	void Serialize(TSerialize ser);
};

typedef struct AgentParameters
{
	AgentPerceptionParameters m_PerceptionParams;
	//! боевой класс этого агента
	i32                       m_CombatClass;

	float                     m_fAccuracy;
	float                     m_fPassRadius;
	float                     m_fStrafingPitch; //!< if this > 0, will do a strafing draw line firing. 04/12/05 Tetsuji

	// Поведение
	float m_fAttackRange;
	float m_fCommRange;
	float m_fAttackZoneHeight;
	float m_fProjectileLaunchDistScale; //!< Controls the preferred launch distance of a projectile (grenade, etc.) in range [0..1].

	i32   m_weaponAccessories;

	// Melee
	float m_fMeleeRange;
	float m_fMeleeRangeShort;
	float m_fMeleeHitRange;
	float m_fMeleeAngleCosineThreshold;
	float m_fMeleeDamage;
	float m_fMeleeKnowckdownChance;
	float m_fMeleeImpulse;

	// Группирующие данные
	bool  factionHostility;
	u8 factionID;

	i32   m_nGroup;

	bool  m_bAiIgnoreFgNode;
	bool  m_bPerceivePlayer;
	float m_fAwarenessOfPlayer;

	//! скрыто/невидимость
	bool  m_bInvisible;
	bool  m_bCloaked;
	float m_fCloakScale;        //!< 0- заметно, 1- соверщенно скрыто (невидимо)
	float m_fCloakScaleTarget;  //!< шкала скрытность падает до этого значения
	float m_fLastCloakEventTime;

	// Включение скорости

	float m_lookIdleTurnSpeed;   //!< Как скоро персонаж поворачивается к мишени при поиске её (радианы/сек). -1=сразу же
	float m_lookCombatTurnSpeed; //!< Как скоро персонаж поворачивается к мишени при поиске её (радианы/сек). -1=сразу же
	float m_aimTurnSpeed;        //!< Как скоро персонаж поворачивается к мишени при прицеливании (радианы/сек)(radians/sec). -1=сразу же
	float m_fireTurnSpeed;       //!< Как скоро персонаж поворачивается к мишени, прицеливаясь и стреляя (радианы/сек). -1=сразу же

	// Укрытие
	float distanceToCover;
	float effectiveCoverHeight;
	float effectiveHighCoverHeight;
	float inCoverRadius;

	// Territory name set by designers - the only string used in this structure but should be fine
	string m_sTerritoryName;
	string m_sWaveName;

	AgentParameters()
	{
		Reset();
	}

	void Serialize(TSerialize ser);

	void Reset()
	{
		m_CombatClass = -1;
		m_fAccuracy = 0.0f;
		m_fPassRadius = 0.4f;
		m_fStrafingPitch = 0.0f;
		m_fAttackRange = 0.0f;
		m_fCommRange = 0.0f;
		m_fAttackZoneHeight = 0.0f;
		m_fMeleeRange = 2.0f;
		m_fMeleeRangeShort = 1.f;
		m_fMeleeAngleCosineThreshold = cosf(DEG2RAD(20.f));
		m_fMeleeHitRange = 1.5f;
		m_fMeleeDamage = 400.0f;
		m_fMeleeKnowckdownChance = .0f;
		m_fMeleeImpulse = 600.0f;
		factionID = IFactionMap::InvalidFactionID;
		factionHostility = true;
		m_nGroup = 0;
		m_bAiIgnoreFgNode = false;
		m_bPerceivePlayer = true;
		m_fAwarenessOfPlayer = 0;
		m_bInvisible = false;
		m_bCloaked = false;
		m_fCloakScale = 0.0f;
		m_fCloakScaleTarget = 0.0f;
		m_fLastCloakEventTime = 0.0f;
		m_weaponAccessories = 0;
		m_lookIdleTurnSpeed = -1;
		m_lookCombatTurnSpeed = -1;
		m_aimTurnSpeed = -1;
		m_fireTurnSpeed = -1;
		m_fProjectileLaunchDistScale = 1.0f;
		distanceToCover = m_fPassRadius + 0.05f;
		inCoverRadius = m_fPassRadius - 0.05f;
		effectiveCoverHeight = 0.85f;
		effectiveHighCoverHeight = 1.75f;

		m_sTerritoryName.clear();
		m_sWaveName.clear();
	}

} AgentParameters;

//! \endcond