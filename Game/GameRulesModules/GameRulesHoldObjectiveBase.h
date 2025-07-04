// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Base implementation for a take and hold objective
			- Handles keeping track of who is inside the objective area
	-------------------------------------------------------------------------
	История:
	- 10:02:2010  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GAME_RULES_HOLD_OBJECTIVE_BASE_H_
#define _GAME_RULES_HOLD_OBJECTIVE_BASE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesModules/IGameRulesEntityObjective.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Game/GameRulesModules/IGameRulesKillListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesClientConnectionListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesTeamChangedListener.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/Effects/Tools/LerpParam.h>

struct IParticleEffect;

class CGameRulesHoldObjectiveBase :	public IGameRulesEntityObjective,
																		public IEntityEventListener,
																		public IGameRulesKillListener,
																		public IGameRulesClientConnectionListener,
																		public IGameRulesTeamChangedListener,
																		public IGameFrameworkListener,
																		public IMovieListener
{
public:
	CGameRulesHoldObjectiveBase();
	~CGameRulesHoldObjectiveBase();

	// IGameRulesEntityObjective
	virtual void Init(XmlNodeRef xml);
	virtual void Update(float frameTime);

	virtual void OnStartGame() {}

	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );

	virtual bool IsComplete(i32 teamId) { return false; }

	virtual void AddEntityId(i32 type, EntityId entityId, i32 index, bool isNewEntity, const CTimeValue &timeAdded);
	virtual void RemoveEntityId(i32 type, EntityId entityId);
	virtual void ClearEntities(i32 type);
	virtual bool IsEntityFinished(i32 type, i32 index)	{ return false; }
	virtual bool CanRemoveEntity(i32 type, i32 index)		{ return true; }

	virtual void OnHostMigration(bool becomeServer) 		{ }

	virtual void OnTimeTillRandomChangeUpdated(i32 type, float fPercLiveSpan) {}
	virtual bool IsPlayerEntityUsingObjective(EntityId playerId);
	// ~IGameRulesEntityObjective

	// IEntityEventListener
	virtual void OnEntityEvent( IEntity *pEntity,SEntityEvent &event );
	// ~IEntityEventListener

	// IGameRulesKillListener
	virtual void OnEntityKilledEarly(const HitInfo &hitInfo) {};
	virtual void OnEntityKilled(const HitInfo &hitInfo);
	// ~IGameRulesKillListener

	// IGameRulesClientConnectionListener
	virtual void OnClientConnect(i32 channelId, bool isReset, EntityId playerId) {}
	virtual void OnClientDisconnect(i32 channelId, EntityId playerId);
	virtual void OnClientEnteredGame(i32 channelId, bool isReset, EntityId playerId) {}
	virtual void OnOwnClientEnteredGame();
	// ~IGameRulesClientConnectionListener

	// IGameRulesTeamChangedListener
	virtual void OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId);
	// ~IGameRulesTeamChangedListener

	// IGameFrameworkListener
	virtual void OnPostUpdate(float fDeltaTime) {}
	virtual void OnSaveGame(ISaveGame* pSaveGame) {}
	virtual void OnLoadGame(ILoadGame* pLoadGame) {}
	virtual void OnLevelEnd(tukk pNextLevel) {}
	virtual void OnActionEvent(const SActionEvent& event);
	// ~IGameFrameworkListener

	// IMovieListener
	virtual void OnMovieEvent(IMovieListener::EMovieEvent movieEvent, IAnimSequence* pAnimSequence);
	// ~IMovieListener

	static i32k HOLD_OBJECTIVE_MAX_ENTITIES = 10;
protected:
	static i32k NUM_TEAMS = 2;
	static i32k CONTESTED_TEAM_ID = -1;

	typedef std::vector<EntityId> TEntityIdVec;

	struct IHoldEntityAdditionalDetails
	{
	};

	struct SHoldEntityDetails
	{
		static i32k RADIUS_EFFECT_SLOT_NONE = -1;

		SHoldEntityDetails()
		{
			Reset();
		}

		void Reset()
		{
			m_id = 0;
			m_pendingId = 0;
			m_pAdditionalData = NULL;
			m_localPlayerIsWithinRange = false;
			m_isNewEntity = false;
			m_controllingTeamId = 0;
			m_totalInsideBoxCount = 0;
			m_controlRadiusSqr = 25.f;
			m_controlRadius = 5.f;
			m_controlHeight = 5.f;
			m_controlOffsetZ = 0.f;

			for (i32 i = 0; i < NUM_TEAMS; ++ i)
			{
				m_insideCount[i] = 0;
				m_serializedInsideCount[i] = 0;

				m_insideEntities[i].clear();
				m_insideEntities[i].reserve(MAX_PLAYER_LIMIT);

				m_insideBoxEntities[i].clear();
				m_insideBoxEntities[i].reserve(MAX_PLAYER_LIMIT);
			}
		}

		TEntityIdVec m_insideBoxEntities[NUM_TEAMS];			// Entities inside the bounding box
		TEntityIdVec m_insideEntities[NUM_TEAMS];					// Entities inside the cylinder
		IHoldEntityAdditionalDetails *m_pAdditionalData;
		EntityId m_id;
		EntityId m_pendingId;
		i32 m_controllingTeamId;
		i32 m_insideCount[NUM_TEAMS];
		i32 m_serializedInsideCount[NUM_TEAMS];
		i32 m_totalInsideBoxCount;
		float m_controlRadiusSqr;
		float m_controlRadius;
		float m_controlHeight;
		float m_controlOffsetZ;
		bool m_localPlayerIsWithinRange;
		bool m_isNewEntity;
		CAudioSignalPlayer m_signalPlayer;
	};

	enum ESpawnPOIType
	{
		eSPT_None,
		eSPT_Avoid,
	};

	// Functions that can optionally be overridden in the child class
	virtual void DetermineControllingTeamId(SHoldEntityDetails *pDetails, i32k team1Count, i32k team2Count);
	// ~ Functions that can optionally be overridden in the child class

	virtual bool AreObjectivesStatic() { return false; }

	// Functions to be overridden in the child class
	virtual void OnInsideStateChanged(SHoldEntityDetails *pDetails);
	virtual void OnNewHoldEntity(SHoldEntityDetails *pDetails, i32 index) {}
	virtual void OnRemoveHoldEntity(SHoldEntityDetails *pDetails) {}
	virtual void OnNetSerializeHoldEntity(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags, SHoldEntityDetails *pDetails, i32 index) {}
	// ~ Functions to be overridden in the child class

	// Functions that aren't really part of this class but are needed by multiple children
	static void AwardPlayerPoints(TEntityIdVec *pEntityVec, EGRST scoreType);
	static void ReadAudioSignal(XmlNodeRef node, tukk name, CAudioSignalPlayer* signalPlayer);
	// ~ Functions that aren't really part of this class but are needed by multiple children

	enum eRadiusPulseType
	{
		eRPT_Neutral,
		eRPT_Friendly,
		eRPT_Hostile,
	};

	eRadiusPulseType	GetPulseType(SHoldEntityDetails *pDetails) const;

private:
	void InitEffectData(XmlNodeRef xmlEffectNode);
	void UpdateEffect(float frameTime);
	void SetNewEffectColor(eRadiusPulseType newPulseType);
	void SetMaterialDiffuseColor(IMaterial* pMaterial,const Vec3& diffuseColor) const;
	void SetMaterialDiffuseAlpha(IMaterial* pMaterial,float diffuseAlpha) const;
	void SetRingAlpha(IEntity* pRingEntity,float alpha);
	void CleanUpEntity(SHoldEntityDetails *pDetails);
	void InsideStateChanged(SHoldEntityDetails *pDetails);
	void CheckCylinder(SHoldEntityDetails *pDetails, EntityId localPlayerId);
	void CheckLocalPlayerInside(SHoldEntityDetails *pDetails, const IEntity *pHoldEntity, const IEntity *pLocalPlayer);
	void CheckLocalPlayerInsideAllEntities();
	void DoAddEntityId(i32 type, EntityId entityId, i32 index, bool isNewEntity);
	void OnLocalPlayerInsideStateChanged(SHoldEntityDetails *pDetails);
	bool IsActorEligible(const IActor *pActor) const;

#ifndef _RELEASE
	void DebugDrawCylinder(SHoldEntityDetails *pDetails);
#endif

protected:
	typedef DrxFixedStringT<32> TFixedString;

	SHoldEntityDetails m_entities[HOLD_OBJECTIVE_MAX_ENTITIES];

	struct SEffectData
	{
		SEffectData()
		{
			neutralColor.Set(1.0f,1.0f,1.0f);
			friendlyColor.Set(0.0f,1.0f,0.0f);
			enemyColor.Set(1.0f,0.0f,0.0f);
			pPrevCol = &neutralColor;;
			pDestCol = &neutralColor;
			pParticleEffect = NULL;
			ringEntityID = 0;
			pParticleGeomMaterial = NULL;
			alphaFadeInDelayDuration = 0.0f;
			alphaFadeInDelay = 0.0f;
			particleEffectScale = 1.0f;
			ringGeomScale = 1.0f;
		}

		CLerpParam					materialColorLerp;
		CLerpParam					particleColorLerp;
		CLerpParam					alphaLerp;
		Vec3								neutralColor;
		Vec3								friendlyColor;
		Vec3								enemyColor;
		Vec3*								pPrevCol;
		Vec3*								pDestCol;
		IParticleEffect*		pParticleEffect;
		IMaterial*					pParticleGeomMaterial;
		EntityId						ringEntityID;
		float								alphaFadeInDelayDuration;
		float								alphaFadeInDelay;
		float								particleEffectScale;
		float								ringGeomScale;
	};

	SEffectData	m_effectData;
	CTimeValue m_pendingTimeAdded;

	IAnimSequence *m_pStartingAnimSequence;

	eRadiusPulseType	m_currentPulseType;

	ESpawnPOIType m_spawnPOIType;
	float m_spawnPOIDistance;
	float m_deferredTrackViewTime;
	bool m_shouldPlayIncomingAudio;
	bool m_bHasNetSerialized;
	bool m_bExpectingMovieStart;
	bool m_bAddedMovieListener;

	enum TAnnounceType
	{
		k_announceType_None=0,
		k_announceType_CS_Incoming,
		k_announceType_CS_Destruct,
	};

	virtual void Announce(tukk announcement, TAnnounceType inType, const bool shouldPlayAudio = true) const;
	void RadiusEffectPulse(EntityId entityId, eRadiusPulseType pulseType, float fScale);

	virtual void OnControllingTeamChanged(SHoldEntityDetails *pDetails, i32k oldControllingTeam);
};

#endif // _GAME_RULES_HOLD_OBJECTIVE_BASE_H_

