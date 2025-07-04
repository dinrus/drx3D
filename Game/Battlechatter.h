// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание:
Automatically plays sounds for nearby actors
**************************************************************************/

#ifndef __BATTLECHATTER_H__
#define __BATTLECHATTER_H__

#include <drx3D/Game/PlayerPlugin.h>
#include <drx3D/Game/AutoEnum.h>
#include <drx3D/Game/GameRulesModules/IGameRulesKillListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesTeamChangedListener.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>
#include <drx3D/Game/Audio/GameAudio.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>

//Note - enum is used as lookup in for AudioSignalPlayer;
#define BATTLECHATTER_LIST(f) \
	f(BC_Reloading) \
	f(BC_LowHealth) \
	f(BC_EnemyKilled) \
	f(BC_PlayerKilledCongrats) \
	f(BC_GrenadeThrow) \
	f(BC_Ripoff) \
	f(BC_MovingUp) \
	f(BC_LowAmmo) \
	f(BC_SecondaryWeapon) \
	f(BC_Sniper) \
	f(BC_MissileCountermeasures) \
	f(BC_Ditto) \
	f(BC_Copy) \
	f(BC_Mandown) \
	f(BC_WatchMyBack) \
	f(BC_Blinded) \
	f(BC_CloakedEnemy) \
	f(BC_SeeGrenade) \
	f(BC_Quiet) \
	f(BC_EnterCrashsite) \
	f(BC_NearbyEnemyDetected) \
	f(BC_ObjectivePickup) \
	f(BC_TakingFire) \
	f(BC_PinnedDown) \
	f(BC_AssistCongrats) \
	f(BC_TimeRemainingGood) \
	f(BC_TimeRemainingBad) \
	f(BC_AssaultDownload) \
	f(BC_VisualAssaultAttacker) \
	f(BC_VisualObjectiveCarried) \
	f(BC_Battlecry) \
	f(BC_Spawn) \
	f(BC_Cloaking) \
	f(BC_FriendlyFire) \
	f(BC_AreaClear) \
	f(BC_PlayerDown) \
	f(BC_Death) \
	f(BC_MovingUp_Cloaked) \
	f(BC_ObjectivePickup_Cloaked) \
	f(BC_Reloading_Cloaked) \
	f(BC_EnemyKilled_Cloaked) \
	f(BC_ArmorMode) \
	f(BC_TakingFire_Armour) \
	f(BC_SeeVTOL) \
	f(BC_DataTransferring) \
	f(BC_SeeGamma) \
	f(BC_Drained) \
	f(BC_TaggedHostiles) \
	f(BC_Download) \
	f(BC_Electrocuted) \
	f(BC_SkillShotKill) \
	f(BC_PlayerObjectivePickup) \
	f(BC_Enemy_HMG) \
	f(BC_EMPBlastActivatedFriendly) \
	f(BC_EMPBlastActivatedHostile) 

	// TODO - add predator battle chatters when the mode is finalised. Current ones in data are now stale as the mode is so different

AUTOENUM_BUILDENUMWITHTYPE_WITHINVALID_WITHNUM(EBattlechatter, BATTLECHATTER_LIST, BC_Null, BC_Last);

class CActor;
class CActorUpr;
struct SActorData;

class CBattlechatter: public IGameRulesKillListener, public IGameRulesTeamChangedListener
{
public:
	struct SVoiceInfo
	{
		SVoiceInfo(i32 voiceIndex)
		{
			m_voiceIndex = voiceIndex;
			m_lastTimePlayed = 0.0f;
		}

		u32 m_voiceIndex;
		float m_lastTimePlayed;
	};

	CBattlechatter();
	virtual ~CBattlechatter();

	void Update(const float dt);
	void Event(EBattlechatter chatter, EntityId actorId);
	void NearestActorEvent(EBattlechatter chatter);

	void SetLocalPlayer(CPlayer* pPlayer);

	//IGameRulesKillListener
	virtual void OnEntityKilledEarly(const HitInfo &hitInfo) {};
	void OnEntityKilled(const HitInfo &hitInfo);
	// ~IGameRulesKillListener

	// IGameRulesTeamChangedListener
	virtual void OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId);
	// ~IGameRulesTeamChangedListener

	void ClientNetEvent(EBattlechatter chatter);

	void NetSerialize(CPlayer* pPlayer, TSerialize ser, EEntityAspects aspect);

	void AddVisualChatter(EntityId entityId, EBattlechatter chatter);
	void RemoveVisualChatter(EntityId entityId);

	//void Play(SVoiceInfo* pInfo, EBattlechatter chatter, CActor* pActor, EntityId actorId, float currentTime, ESoundSemantic semanticOverride = eSoundSemantic_None, i32 specificVariation = -1, i32 listeningActorTeamId = -1);
	SVoiceInfo* GetVoiceInfo(EntityId actorId);

	void RegisterCVars();
	void UnRegisterCVars();

#if !defined(_RELEASE)
	static void CmdPlay(IConsoleCmdArgs* pCmdArgs);
	static void CmdClearRecentlyPlayed(IConsoleCmdArgs *pCmdArgs);
	static void CmdDump(IConsoleCmdArgs *pCmdArgs);
	static void CmdDumpPlayed(IConsoleCmdArgs *pCmdArgs);
	static void CmdDumpUnPlayed(IConsoleCmdArgs *pCmdArgs);
	static void CmdClearPlayCount(IConsoleCmdArgs *pCmdArgs);
#endif

	void PlayerHasEnteredAVTOL(const IEntity* pPlayerEntity);
	void MicrowaveBeamActivated(EntityId ownerPlayerEntityId, const Vec3 *deployedPos);
	void EMPBlastActivated(const EntityId ownerPlayerEntityId, i32k playerTeam, const bool friendly);
	void LocalPlayerHasGotObjective();
	void PlayerIsDownloadingDataFromTerminal(const EntityId playerEntityId);

	void StopAllBattleChatter(CActor& rActor);

	void ClearRecentlyPlayed();
	TAudioSignalID GetChatterSignalForActor(SVoiceInfo *pInfo, EBattlechatter chatter, CActor *pActor, i32 listeningActorTeamId);

protected:

#define BATTLECHATTER_VARS(f) \
	f(float,	  bc_minTimeBetweenBattlechatter) \
	f(float,		bc_maxTimeCanDitto) \
	f(float,    bc_timeNeededForQuiet) \
	f(float,    bc_NearEndOfGameTime) \
	f(i32 ,			bc_NearEndOfGameScore) \
	f(float,    bc_AreaClearedDistance) \
	f(i32 ,     bc_FriendlyActorVoiceIndex) \
	f(i32 ,     bc_FriendlyActorVoiceRange) \
	f(i32 ,     bc_EnemyActorVoiceIndex) \
	f(i32 ,     bc_EnemyActorVoiceRange) \
	f(i32 ,			bc_FriendlyHunterActorVoiceIndex) \
	f(i32 ,			bc_FriendlyHunterActorVoiceRange) \
	f(i32 ,			bc_EnemyHunterActorVoiceIndex) \
	f(i32 ,			bc_EnemyHunterActorVoiceRange) \
	f(float,		bc_defaultLoadTimeout) \
	f(float,		bc_canSeeActorEnterVTOL_rangeSq) \
	f(float,		bc_canSeeMicrowaveBeam_rangeSq) \
	f(float,		bc_canSeeLocalPlayerGetObjective_rangeSq) \
	f(float,		bc_minSkillShot_rangeSq) \
	f(float,		bc_canSeePlayerDownloadingData_rangeSq) \

#define INSTANCE_BATTTLECHATTER_VARS(myType,myName) myType myName;

	BATTLECHATTER_VARS(INSTANCE_BATTTLECHATTER_VARS)

#undef INSTANCE_BATTTLECHATTER_VARS

	typedef enum
	{
		k_SHH_flags_none										= 0,
		k_SHH_flags_consider_teammates			= BIT(0),
		k_SHH_flags_consider_enemies				= BIT(1),
		k_SHH_flags_2D_range_test						= BIT(2),
	} ESHHFlags;

	void Init();
	void InitVoices();
	void InitData();
	void InitVars();

	EBattlechatter ReadChatterFromNode(tukk chatterName, XmlNodeRef chatterNode);
	bool PlayedRecently(const SVoiceInfo* pInfo, const float currentTime) const;
	bool ShouldIgnoreRecentlyPlayed( const EBattlechatter chatter, const EBattlechatter requestedChatter ) const;
	bool CanPlayDitto(const float currentTime) const;
	bool ShouldClientHear(const EBattlechatter chatter) const;
	bool ShouldPlayForActor(EntityId actorId, float range) const;
	bool ChanceToPlay(EBattlechatter chatter);
	bool ShouldHearActor(EntityId actorId) const;
	float GetMPTeamParam(CActor* pActor, EntityId actorId);
	bool IsSkillShot(const HitInfo &hitInfo);

	float DistanceToActor(EntityId actorId) const;

	bool CheckClientNetEvent(EBattlechatter chatter);
	bool UpdateEvent(EBattlechatter& chatter, EntityId &actorId, IActor* pActor);

	EntityId GetNearestHearableFriendlyActor(CActor& rActor) const;
	
	void UpdateActorBattlechatter(i32 actorIndex, const CActorUpr * pActorUpr);

	void UpdateVisualEvents(CPlayer* pPlayer, const SActorData& rActorData);
	void UpdateMovingUpEvent(CPlayer* pPlayer, const SActorData& rActorData);
	void UpdateSeeGrenade(CPlayer* pPlayer, const SActorData& rActorData);
	void UpdateQuiet(CPlayer* pPlayer, const SActorData& rActorData);
	void UpdateNearEndOfGame(CPlayer* pPlayer, const SActorData& rActorData);

	typedef void (CBattlechatter::*UpdateFunc)(CPlayer*, const SActorData&);

	static UpdateFunc s_updateFunctions[];

	float GetForwardAmount(IEntity* pEntity);

	EBattlechatter GetVisualChatter(EntityId entityId);

	bool IsLastPlayed(EBattlechatter chatter);
	bool IsRecentlyPlayed(EBattlechatter chatter);
	void AddPlayed(EBattlechatter chatter);
	bool IsAreaCleared(EntityId shooterId, EntityId targetId);
	
	//void OnLengthCallback(const bool& success, const float& length, ISound* pSound);

	const bool ShouldPlay() const;
	void Debug();

	u32k GetVoiceIndex(bool friendly);

	void SomethingHasHappened(
										i32k inByTeamId,
										const Vec3 *inAtPos,
										const EBattlechatter inRequestedChatter, 
										const float inMaxRangeSqToReact, 
										u32k inFlags);

	struct SVoice
	{
		SVoice();
		bool Init(i32 voiceIndex);
		void clear();
		TAudioSignalID GetChatterSignal(i32 voiceIndex, i32 chatterIndex, tukk suffixStr);

		struct SVoiceSignals
		{
			TAudioSignalID m_standard;
			TAudioSignalID m_marine;
			TAudioSignalID m_hunter;
			TAudioSignalID m_hunterMarine;	// a marine in hunter mode only

			SVoiceSignals() :
				m_standard(INVALID_AUDIOSIGNAL_ID),
				m_marine(INVALID_AUDIOSIGNAL_ID),
				m_hunter(INVALID_AUDIOSIGNAL_ID),
				m_hunterMarine(INVALID_AUDIOSIGNAL_ID) { }
		};

		typedef DrxFixedArray<SVoiceSignals, BC_Last> VoiceChatterArray;
		VoiceChatterArray m_chatter;
	};

	CPlayer* m_clientPlayer;

	const static i32 k_recentlyPlayedListSize = 2;
	typedef DrxFixedArray<EBattlechatter, k_recentlyPlayedListSize> TRecentlyPlayedList;
	TRecentlyPlayedList m_recentlyPlayed;
	i32 m_recentlyPlayedIndex;

	i32 m_currentActorIndex;
	i32 m_currentActorEventIndex;

	float m_lastTimePlayed;
	float m_lastTimeExpectedFinished;

	std::vector <SVoice> m_voice;

	typedef std::map<EntityId, SVoiceInfo> ActorVoiceMap;
	ActorVoiceMap m_actorVoice;

	struct SBattlechatterData
	{
		float m_chance;
		float m_range;
		float m_netMinTimeBetweenSends;
		float m_netLastTimePlayed;				// net only for now
		EBattlechatter m_repeatChatter;
		EBattlechatter m_cloakedChatter;
		EBattlechatter m_armourChatter;

#ifndef _RELEASE
		u32				 m_playCount;
#endif

		typedef enum
		{
			k_BCD_flags_none															= 0,
			k_BCD_flags_enemy_can_speak										= BIT(0),
			k_BCD_flags_friendly_hunter_can_speak					= BIT(1),
			k_BCD_flags_enemy_hunter_can_speak						= BIT(2)
		} E_BCDFlags;

		u8 m_flags;

		SBattlechatterData() : 
			m_chance(0.f),
			m_range(0.f),
			m_netMinTimeBetweenSends(0.f),
			m_netLastTimePlayed(0.f),
			m_repeatChatter(BC_Null),
			m_cloakedChatter(BC_Null),
			m_armourChatter(BC_Null),
			m_flags(k_BCD_flags_none)
		{
#ifndef _RELEASE
			m_playCount=0;
#endif
		}
	};

	typedef DrxFixedArray<SBattlechatterData, BC_Last> BattleChatterDataArray;
	BattleChatterDataArray m_data;
	EBattlechatter m_clientNetChatter;
	u8 m_serialiseCounter;

	const static i32 k_maxAlternativeVisuals = 4;
	typedef std::pair<EntityId, EBattlechatter> TVisualPair;
	typedef DrxFixedArray<TVisualPair, k_maxAlternativeVisuals> TVisual;
	TVisual m_visualChatter;

	CAudioSignalPlayer m_signalPlayer;
};

// chatter event is being said by actorId on local client only
#define BATTLECHATTER(chatter, actorId)	\
	if (CGameRules* pBCGameRules = g_pGame->GetGameRules()) \
	{	\
		CBattlechatter* pBattlechatter = pBCGameRules->GetBattlechatter();	\
		if(pBattlechatter) \
		{	\
			pBattlechatter->Event(chatter, actorId);	\
		}	\
	}	\

// chatter event is being said by nearest actor to local client, on local client only
#define BATTLECHATTER_NEAREST_ACTOR(chatter) \
	if (CGameRules* pBCGameRules = g_pGame->GetGameRules()) \
	{	\
		CBattlechatter* pBattlechatter = pBCGameRules->GetBattlechatter();	\
		if(pBattlechatter) \
		{	\
			pBattlechatter->NearestActorEvent(chatter);	\
		}	\
	}	\

// chatter event is being said by our local client player, but we want our player to say this event on everyone else's clients
#define NET_BATTLECHATTER(chatter, pClientPlayer) \
	if(pClientPlayer && pClientPlayer->IsClient()) \
	{ \
		CGameRules* pBCGameRules = g_pGame->GetGameRules(); \
		if(pBCGameRules) \
		{	\
			CBattlechatter* pBattlechatter = pBCGameRules->GetBattlechatter();	\
			if(pBattlechatter) \
			{	\
				pBattlechatter->ClientNetEvent(chatter);	\
			}	\
		}	\
	}

#define ADD_VISUAL_BATTLECHATTER(actorId, chatter)	\
	if (CGameRules* pBCGameRules = g_pGame->GetGameRules()) \
	{	\
		CBattlechatter* pBattlechatter = pBCGameRules->GetBattlechatter();	\
		if(pBattlechatter) \
		{	\
			pBattlechatter->AddVisualChatter(actorId, chatter);	\
		}	\
	} \

#define REMOVE_VISUAL_BATTLECHATTER(actorId)	\
	if (CGameRules* pBCGameRules = g_pGame->GetGameRules()) \
	{	\
		CBattlechatter* pBattlechatter = pBCGameRules->GetBattlechatter();	\
		if(pBattlechatter) \
		{	\
			pBattlechatter->RemoveVisualChatter(actorId);	\
		}	\
	}

#endif // __BATTLECHATTER_H__
