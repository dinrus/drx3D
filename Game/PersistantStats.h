// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 04:02:2010		Created by Ben Parbury
*************************************************************************/

#ifndef __PERSISTANTSTATS_H__
#define __PERSISTANTSTATS_H__


#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/GameRulesModules/IGameRulesKillListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesClientScoreListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsListener.h>
#include <drx3D/Game/PlayerProgression.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/CoreX/Lobby/IDrxStats.h>
#include <drx3D/CoreX/Lobby/IDrxLobby.h>
#include <drx3D/Act/IPlayerProfiles.h>

#include <drx3D/Game/AutoEnum.h>
#include <drx3D/CoreX/TypeInfo_impl.h>

#include <drx3D/Game/AfterMatchAwards.h>
#include <drx3D/Game/Network/Lobby/GameAchievements.h>

#include <drx3D/Game/ScriptBind_ProtectedBinds.h>

#include <drx3D/Game/StatsStructures.h>

struct IPlayerProfile;
struct IDrxPak;
struct IFlashPlayer;
struct IFlashVariableObject;
class CPlayer;
class CHUDMissionObjective;
struct SDatabaseEntry;
struct SCrysis3GeneralPlayerStats;
struct SCrysis3MiscPlayerStats;
struct SCrysis3ScopedPlayerStats;
struct SCrysis3WeaponStats;

#define MELEE_WEAPON_NAME "Melee"
#define POLE_WEAPON_NAME "pole"
#define PANEL_WEAPON_NAME "panel"
#define ENV_WEAPON_THROWN "_thrown"

#define NUM_HISTORY_ENTRIES 5

static tukk s_unlockableAttachmentNames[] =
{
	// SP unlockable attachments
	"Silencer",
	"SilencerPistol",
	"LaserSight",
	"PistolLaserSight",
	"MatchBarrel",
	"MuzzleBreak",
	"Bayonet",
	"GrenadeLauncher",
	"ExtendedClipSCAR",
	"ExtendedClipFeline",
	"ExtendedClipJackal",
	"DoubleClipFeline",
	"DoubleClipJackal",
	"ForeGrip",
	"TyphoonAttachment",
	"Flashlight",
	"Reflex",
	"AssaultScope",
	"SniperScope",
	"TechScope",
	// MP exclusive (add here below)
	"GaussAttachment",
};

static i32k s_numUnlockableAttachments = sizeof(s_unlockableAttachmentNames)/sizeof(tuk);

static tukk s_displayableAttachmentNamesSP[] =
{
	"Silencer",
	"SilencerPistol",
	"LaserSight",
	"PistolLaserSight",
	"MatchBarrel",
	"MuzzleBreak",
	"Bayonet",
	"GrenadeLauncher",
	"ExtendedClipSCAR",
	"ExtendedClipFeline",
	"ExtendedClipJackal",
	"DoubleClipFeline",
	"DoubleClipJackal",
	"ForeGrip",
	"TyphoonAttachment",
	"Flashlight",
	"Reflex",
	"AssaultScope",
	"SniperScope",
	"TechScope"
};

static i32k s_numDisplayableAttachmentsSP = sizeof(s_displayableAttachmentNamesSP)/sizeof(tuk);


enum EStatsFlags
{
	eSF_LocalClient			= 0,				//stats that are calculated for local client (All stats hence it's zero)
	eSF_RemoteClients		= BIT(0),		//stats that are tracked for remote clients
	//eST_ServerClient		=	BIT(1),		//stats that are tracked on the server and for local client

	eSF_MapParamNone		= BIT(2),		//Map stat that stores anything added (can't use online storage as it's stored as a string)
	eSF_MapParamWeapon	= BIT(3),		//Map stat that only stores Weapon names from the persistent map stats
	eSF_MapParamHitType	= BIT(4),		//Map stat that only stores HitType names from the persistent map stats
	eSF_MapParamGameRules	= BIT(5),	//Map stat that only stores GameRules names from the persistent map stats
	eSF_MapParamLevel		=	 BIT(6),	//Map stat that only stores Level names from the persistent map stats

	eSF_StreakMultiSession	=	 BIT(8),	//Streaks that shouldn't be reset per session, such as games won in a row
};

struct SPersistantStatsStrings
{
	DrxFixedStringT<64> m_title;
	DrxFixedStringT<64> m_value;
	float m_numericValue;
	bool m_highestWins;

	SPersistantStatsStrings()
		: m_numericValue(0.f)
		, m_highestWins(true)
	{}
};

enum EDatabaseStatValueFlag
{
	eDatabaseStatValueFlag_None = 0,
	eDatabaseStatValueFlag_Available = BIT(0),
	eDatabaseStatValueFlag_Viewed = BIT(1),
	eDatabaseStatValueFlag_Decrypted = BIT(2),
};

//Helper func
void GetClientScoreInfo( CGameRules* pGameRules, i32 &position, i32 &points, EntityId &outClosestCompetitorEntityId, i32 &outClosestCompetitorScore );

class CPersistantStats : public SGameRulesListener,
	public IGameRulesKillListener,
	public IPlayerProgressionEventListener,
	public IWeaponEventListener,
	public IItemSystemListener,
	public IGameRulesClientScoreListener,
	public IGameRulesRoundsListener,
	public IPlayerProfileListener,
	public ISystemEventListener,
	public IInputEventListener
{
public:
	struct SMVPCompare
	{
		SMVPCompare(i32 _kills, i32 _deaths, i32 _assists, i32 _points, i32 _gamemodePoints)
			: kills(_kills), deaths(_deaths), assists(_assists), points(_points), gamemodePoints(_gamemodePoints)
		{ }

		const bool CompareForMVP(const EGameMode gamemode, const SMVPCompare& otherPlayer) const;	// Compares against other player for MVP, returns true if better or draw, false if worse.
		i32k MVPScore(const EGameMode gamemode) const;	// Returns the score used for MVP per gamemode

		i32 kills;
		i32 deaths;
		i32 assists;
		i32 points;
		i32 gamemodePoints;
	};

	friend class CPersistantStatsDebugScreen;

	typedef std::vector<string> TMapParams;

public:
	friend class CAfterMatchAwards;
	friend class CScriptBind_ProtectedBinds;

	CPersistantStats();
	virtual ~CPersistantStats();

	static CPersistantStats* GetInstance();

	void Init();
	void RegisterLevelTimeListeners();
	void UnRegisterLevelTimeListeners();
	void Update(const float dt);
	void Reset();
	void AddListeners();

	i32 GetStat(EIntPersistantStats stat);
	float GetStat(EFloatPersistantStats stat);
	i32 GetStat(EStreakIntPersistantStats stat);
	float GetStat(EStreakFloatPersistantStats stat);
	i32 GetStat(tukk name, EMapPersistantStats);
	const SSessionStats::SMap::MapNameToCount& GetStat(EMapPersistantStats stat);
	i32 GetDerivedStat(EDerivedIntPersistantStats stat);
	float GetDerivedStat(EDerivedFloatPersistantStats stat);
	tukk GetDerivedStat(EDerivedStringPersistantStats);
	tukk GetDerivedStat(EDerivedStringMapPersistantStats);
	i32 GetDerivedStat(tukk name, EDerivedIntMapPersistantStats);
	float GetDerivedStat(tukk name, EDerivedFloatMapPersistantStats);

	i32 GetStatThisSession(EStreakIntPersistantStats stat);
	float GetStatThisSession(EStreakFloatPersistantStats stat);

	void ResetStat(EStreakIntPersistantStats stat);
	void ResetStat(EStreakFloatPersistantStats stat);

	i32 GetStatForActorThisSession(EStreakIntPersistantStats stat, EntityId inActorId);
	float GetStatForActorThisSession(EStreakFloatPersistantStats stat, EntityId inActorId);
	i32 GetStatForActorThisSession(EIntPersistantStats stat, EntityId inActorId);
	float GetStatForActorThisSession(EFloatPersistantStats stat, EntityId inActorId);
	i32 GetStatForActorThisSession(EMapPersistantStats stat, tukk param, EntityId inActorId);
	i32 GetDerivedStatForActorThisSession(EDerivedIntPersistantStats stat, EntityId inActorId);
	float GetDerivedStatForActorThisSession(EDerivedFloatPersistantStats stat, EntityId inActorId);
	void GetMapStatForActorThisSession(EMapPersistantStats stat, EntityId inActorId, std::vector<i32> &result);

	static EIntPersistantStats GetIntStatFromName(tukk name);
	static EFloatPersistantStats GetFloatStatFromName(tukk name);
	static EStreakIntPersistantStats GetStreakIntStatFromName(tukk name);
	static EStreakFloatPersistantStats GetStreakFloatStatFromName(tukk name);
	static EMapPersistantStats GetMapStatFromName(tukk name);
	static EDerivedIntPersistantStats GetDerivedIntStatFromName(tukk name);
	static EDerivedFloatPersistantStats GetDerivedFloatStatFromName(tukk name);
	static EDerivedIntMapPersistantStats GetDerivedIntMapStatFromName(tukk name);
	static EDerivedFloatMapPersistantStats GetDerivedFloatMapStatFromName(tukk name);

	static tukk GetNameFromIntStat(EIntPersistantStats stat);
	static tukk GetNameFromFloatStat(EFloatPersistantStats stat);
	static tukk GetNameFromStreakIntStat(EStreakIntPersistantStats stat);
	static tukk GetNameFromStreakFloatStat(EStreakFloatPersistantStats stat);
	static tukk GetNameFromMapStat(EMapPersistantStats stat);
	static tukk GetNameFromDerivedIntStat(EDerivedIntPersistantStats stat);
	static tukk GetNameFromDerivedFloatStat(EDerivedFloatPersistantStats stat);
	static tukk GetNameFromDerivedIntMapStat(EDerivedIntMapPersistantStats stat);
	static tukk GetNameFromDerivedFloatMapStat(EDerivedFloatMapPersistantStats stat);

	i32 GetCurrentStat(EntityId actorId, EStreakIntPersistantStats stat);

	CAfterMatchAwards *GetAfterMatchAwards() { return &m_afterMatchAwards; }

	bool IncrementWeaponHits(CProjectile& projectile, EntityId targetId);
	void ClientHit(const HitInfo&);
	void ClientShot(CGameRules* pGameRules, u8 hitType, const Vec3 & dir);
	void ClientDelayedExplosion(EntityId projectileDelayed);
	void ClientRegenerated();
	void ClientDied( CPlayer* pClientPlayer );
	void OnSPLevelComplete(tukk levelName);
	void OnGiveAchievement(i32 achievement);

	void IncrementStatsForActor( EntityId inActorId, EIntPersistantStats stats, i32 amount = 1 );
	void IncrementStatsForActor( EntityId inActorId, EFloatPersistantStats stats, float amount = 1.0f );
	void IncrementClientStats(EIntPersistantStats stats, i32 amount = 1);
	void IncrementClientStats(EFloatPersistantStats stats, float amount = 1.0f);
	void IncrementMapStats(EMapPersistantStats stats, tukk  name);
	void SetClientStat(EIntPersistantStats stat, i32 value );

	void SetMapStat(EMapPersistantStats stats, tukk  name, i32 amount);
	void ResetMapStat(EMapPersistantStats stats);

	void HandleTaggingEntity(EntityId shooterId, EntityId targetId);
	void HandleLocalWinningKills();

	void OnClientDestroyedVehicle(const SVehicleDestroyedParams& vehicleInfo);

	//SGameRulesListener
	virtual void GameOver(EGameOverType localWinner, bool isClientSpectator);
	virtual void EnteredGame();
	virtual void ClientDisconnect( EntityId clientId );
	virtual void ClTeamScoreFeedback(i32 teamId, i32 prevScore, i32 newScore);
	//~SGameRulesListener

	//IGameRulesKillListener
	virtual void OnEntityKilledEarly(const HitInfo& hitInfo){}
	virtual void OnEntityKilled(const HitInfo &hitInfo);
	//~IGameRulesKillListener

	//IPlayerProgressionEventListener
	virtual void OnEvent(EPPType type, bool skillKill, uk data);
	//~IPlayerProgressionEventListener

	// IWeaponEventListener
	virtual void OnShoot(IWeapon *pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3 &pos, const Vec3 &dir, const Vec3 &vel);
	virtual void OnStartFire(IWeapon *pWeapon, EntityId shooterId) {}
	virtual void OnStopFire(IWeapon *pWeapon, EntityId shooterId) {}
	virtual void OnFireModeChanged(IWeapon *pWeapon, i32 currentFireMode){}
	virtual void OnStartReload(IWeapon *pWeapon, EntityId shooterId, IEntityClass* pAmmoType);
	virtual void OnEndReload(IWeapon *pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {}
	virtual void OnOutOfAmmo(IWeapon *pWeapon, IEntityClass* pAmmoType) {}
	virtual void OnReadyToFire(IWeapon *pWeapon) {}
	virtual void OnPickedUp(IWeapon *pWeapon, EntityId actorId, bool destroyed) {}
	virtual void OnDropped(IWeapon *pWeapon, EntityId actorId) {}
	virtual void OnMelee(IWeapon* pWeapon, EntityId shooterId);
	virtual void OnStartTargetting(IWeapon *pWeapon) {}
	virtual void OnStopTargetting(IWeapon *pWeapon) {}
	virtual void OnSelected(IWeapon *pWeapon, bool selected);
	virtual void OnSetAmmoCount(IWeapon *pWeapon, EntityId shooterId) {}
	virtual void OnEndBurst(IWeapon *pWeapon, EntityId shooterId) {}
	//~IWeaponEventListener

	//IItemSystemListener
	virtual void OnSetActorItem(IActor *pActor, IItem *pItem );
	virtual void OnDropActorItem(IActor *pActor, IItem *pItem ) {}
	virtual void OnSetActorAccessory(IActor *pActor, IItem *pItem ) {}
	virtual void OnDropActorAccessory(IActor *pActor, IItem *pItem ){}
	//~IItemSystemListener

	//IGameRulesClientScoreListener
	virtual void ClientScoreEvent(EGameRulesScoreType scoreType, i32 points, EXPReason inReason, i32 currentTeamScore);
	//~IGameRulesClientScoreListener

	//IGameRulesRoundsListener
	virtual void OnRoundStart() {}
	virtual void OnRoundEnd();
	virtual void OnSuddenDeath() {}
	virtual void ClRoundsNetSerializeReadState(i32 newState, i32 curState) {}
	virtual void OnRoundAboutToStart() {}
	//~IGameRulesRoundsListener

	//IPlayerProfileListener
	virtual void SaveToProfile(IPlayerProfile* pProfile, bool online, u32 reason);
	virtual void LoadFromProfile(IPlayerProfile* pProfile, bool online, u32 reason);
	//~IPlayerProfileListener

	//ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	//~ISystemEventListener

	// IInputEventListener
	virtual bool OnInputEvent(const SInputEvent &rInputEvent);
	// ~IInputEventListener

	void OnEnteredVTOL( EntityId playerId );
	void OnExitedVTOL( EntityId playerId );

	const bool ShouldSaveClientTelemetry() const;

	static tukk  GetAdditionalWeaponNameForSharedStats(tukk  name);

	void UpdateClientGrenadeBounce(const Vec3 pos, const float radius);
	void AddEntityToNearGrenadeList(EntityId entityId, Vec3 actorPos);
	bool HasClientFlushedTarget(EntityId targetId, Vec3 targetPos);
	bool IsClientDualWeaponKill(EntityId targetId);

	float GetLastFiredTime() const { return m_lastFiredTime; }

	void UpdateMultiKillStreak(EntityId shooterId, EntityId targetId);
	bool CheckRetaliationKillTarget(EntityId victimId);

	void SetMultiplayer(const bool multiplayer);
	i32	 GetOnlineAttributesVersion();

	void OnQuit();

	void GetMemoryUsage(IDrxSizer *pSizer ) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	ILINE bool IsInGame() const { return m_inGame; }
	ILINE void SetInGame(const bool inGame) { m_inGame = inGame; }

	void UnlockAll();

	void OnEnterFindGame();
	void OnGameActuallyStarting();

	void GetStatStrings(EIntPersistantStats stat, SPersistantStatsStrings *statsStrings);
	void GetStatStrings(EFloatPersistantStats stat, SPersistantStatsStrings *statsStrings);
	void GetStatStrings(EStreakIntPersistantStats stat, SPersistantStatsStrings *statsStrings);
	void GetStatStrings(EStreakFloatPersistantStats stat, SPersistantStatsStrings *statsStrings);

	void GetStatStrings(EDerivedIntPersistantStats stat, SPersistantStatsStrings *statsStrings);
	void GetStatStrings(EDerivedFloatPersistantStats stat, SPersistantStatsStrings *statsStrings);
	void GetStatStrings(EDerivedStringPersistantStats stat, SPersistantStatsStrings *statsStrings);
	void GetStatStrings(EDerivedStringMapPersistantStats stat, SPersistantStatsStrings *statsStrings);

	void GetStatStrings(tukk name, EDerivedIntMapPersistantStats stat, tukk paramString, SPersistantStatsStrings *statsStrings);
	void GetStatStrings(tukk name, EDerivedFloatMapPersistantStats stat, tukk paramString, SPersistantStatsStrings *statsStrings);
	void GetStatStrings(tukk name, EMapPersistantStats stat, tukk paramString, SPersistantStatsStrings *statsStrings);

	void GetStatStrings( EMapPersistantStats stat, SPersistantStatsStrings *statsStrings);

	tukk  GetMapParamAt(tukk  name, u32 at) const;
	const TMapParams* GetMapParamsExt(tukk name) const;
	tukk GetWeaponMapParamName() const;

	const static float	k_actorStats_inAirMinimum;
	const static i32		kNumEnemyRequiredForAwards = 4;
	const static float	kTimeAllowedToKillEntireEnemyTeam;

	// Get Session stats for previous game - 0 == previous game, 1 == game before that etc. Up to max of previous DRXSIS3_NUM_HISTORY_ENTRIES-1
	const SSessionStats* GetPreviousGameSessionStatsForClient(u8 previousGameIndex) const;
	u32 GetAverageDeltaPreviousGameXp(u8k desiredNumGamesToAverageOver) const;

	tukk GetFavoriteWeaponStr() const { return m_favoriteWeapon; }
	tukk GetFavoriteAttachmentStr() const { return m_favoriteAttachment; }

	static i32k k_nLoadoutModules = 3;

protected:
	static CPersistantStats* s_persistantStats_instance;

	void SetWeaponStat(tukk weaponName, IFlashPlayer *pFlashPlayer, IFlashVariableObject *pPushArray);
	void SetSPWeaponStat(tukk weaponName, IFlashPlayer *pFlashPlayer, IFlashVariableObject *pPushArray, tukk nameOverride=NULL);
	void SetGamemodeStat(tukk gamemodeName, IFlashPlayer *pFlashPlayer, IFlashVariableObject *pPushArray);
	void SetMissionObjectivesStat(const CHUDMissionObjective& objective, IFlashPlayer *pFlashPlayer, IFlashVariableObject *pPushArray);
	void SetDatabaseStat(i32 iDatabaseStatValueFlags, const SDatabaseEntry& entry, IFlashPlayer *pFlashPlayer, IFlashVariableObject *pPushArray);

	bool ShouldUpdateStats();

	void SaveLatestGameHistoryDeltas( const SSessionStats& latestSessionStats, const SSessionStats& startSessionStats, SSessionStats& outStatHistory );


	float m_lastFiredTime;

	EntityId m_retaliationTargetId;
	bool m_weaponDamaged;

	const static i32 MAX_GROUP_STATS = 64;
	typedef DrxFixedArray<SPersistantStatsStrings, MAX_GROUP_STATS> TPersistantStatsStringsArray;
	typedef std::vector<tukk > TStringsPushArray;

	void CalculateOverallWeaponStats();
	void CalculateOverallAttachmentStats();

	i32 m_iCalculatedTotalWeaponsUsedTime;
	DrxFixedStringT<16> m_favoriteWeapon;
	DrxFixedStringT<32> m_favoriteAttachment;
	DrxFixedStringT<32> m_dummyStr;

	CAfterMatchAwards m_afterMatchAwards;

	//Sessions are stored for every player for the current session
	typedef std::map<EntityId, SSessionStats, std::less<EntityId>, stl::STLGlobalAllocator<std::pair<EntityId, SSessionStats> > > ActorSessionMap;
	ActorSessionMap m_sessionStats;

	//local clients stats that are updated at the end of session
	SSessionStats m_clientPersistantStats;

	//local clients stats that are cached at the start of the session so deltas can be observed
	SSessionStats m_clientPersistantStatsAtGameStart;

	// We maintain a history of local client stats (initially populated via blaze game history.. but then maintained locally).
	// Represents 'deltas' (??)
	typedef std::deque<SSessionStats> TSessionStatHistoryQueue; // random access + constant time insertion at front + removal at end:)
	TSessionStatHistoryQueue m_clientPersistantStatHistory;

	typedef VectorMap<EntityId, float> NearGrenadeMap;
	NearGrenadeMap m_nearGrenadeMap;

	struct SPreviousWeaponHit
	{
		SPreviousWeaponHit()
		{
			m_curWeaponLastHitTime = 0.f;
			m_prevWeaponLastHitTime = 0.f;
			m_curWeaponClassId = 0;
			m_prevWeaponClassId = 0;
		}

		SPreviousWeaponHit(i32 _weaponClassId, float _currentTime)
		{
			m_curWeaponLastHitTime = _currentTime;
			m_prevWeaponLastHitTime = 0.f;
			m_curWeaponClassId = _weaponClassId;
			m_prevWeaponClassId = 0;
		}

		float m_curWeaponLastHitTime;
		float m_prevWeaponLastHitTime;

		i32 m_curWeaponClassId;
		i32 m_prevWeaponClassId;
	};

	typedef VectorMap<EntityId, SPreviousWeaponHit> PreviousWeaponHitMap;
	PreviousWeaponHitMap m_previousWeaponHitMap;

	struct SGrenadeKillElement
	{
		EntityId m_victimId;
		float m_atTime;

		SGrenadeKillElement()
		{
			m_victimId=0;
			m_atTime=0.f;
		}

		SGrenadeKillElement(EntityId inVictimId, float inAtTime)
		{
			m_victimId=inVictimId;
			m_atTime=inAtTime;
		}
	};
	typedef DrxFixedArray<SGrenadeKillElement, 3> TGrenadeKills;
	TGrenadeKills m_grenadeKills;
	static const float k_fastGrenadeKillTimeout;

	struct SEnemyTeamMemberInfo
	{
		EntityId m_entityId;											// duplicated from the map index
		float m_timeEnteredKilledState;
		float m_timeOfLastKill;
		static const float k_timeInKillStateTillReset;

		enum EState
		{
			k_state_initial=0,
			k_state_tagged,
			k_state_killed,
			k_state_crouchedOver,
		};
		EState m_state;

		i32 m_killedThisMatch;
		i32 m_beenKilledByThisMatch;
		bool m_killedThisMatchNotDied;
		bool m_teabaggedThisDeath;
		bool m_taggedThisMatch;

		SEnemyTeamMemberInfo()
			: m_entityId(INVALID_ENTITYID)
			, m_state(k_state_initial)
			, m_timeEnteredKilledState(0.0f)
			, m_timeOfLastKill(0.0f)
			, m_killedThisMatch(0)
			, m_beenKilledByThisMatch(0)
			, m_killedThisMatchNotDied(false)
			, m_teabaggedThisDeath(false)
			, m_taggedThisMatch(false)
		{}
	};

	struct SSortStat
	{
		SSortStat(tukk name );
		static bool WeaponCompare ( SSortStat elem1, SSortStat elem2 );
		static bool GamemodeCompare ( SSortStat elem1, SSortStat elem2 );

		tukk m_name;
	};

	struct SPreviousKillData
	{
		SPreviousKillData();
		SPreviousKillData(i32 hitType, bool bEnvironmental, bool bWasPole);
		i32 m_hitType;
		bool m_bEnvironmental;
		bool m_bWasPole;
	};

	typedef VectorMap<EntityId, SEnemyTeamMemberInfo> EnemyTeamMemberInfoMap;
	EnemyTeamMemberInfoMap m_enemyTeamMemberInfoMap;

	const static u32 k_previousKillsToTrack = 2;
	std::deque<SPreviousKillData> m_clientPreviousKillData;

	void AddClientHitActorWithWeaponClassId(EntityId actorHit, i32 weaponClassId, float currentTime);
	SEnemyTeamMemberInfo *GetEnemyTeamMemberInfo(EntityId inEntityId);

	SSessionStats* GetActorSessionStats(EntityId actorId);
	SSessionStats* GetClientPersistantStats();
	SSessionStats* GetClientPersistantStatsAtSessionStart();

	void PostGame( bool bAllowToDoStats );

	bool LoadFromProfile(SSessionStats* pSessionStats);
	bool SaveToProfile(const SSessionStats* pSessionStats);

	static tukk GetAttributePrefix();
	void SaveTelemetry(bool description, bool toDisk);

	i32 GetBinaryVersionHash(u32 flags);
	void SaveTelemetryInternal(tukk filenameNoExt, const SSessionStats* pSessionStats, u32 flags, bool description, bool toDisk);

	template <class T>
	void WriteToBuffer(tuk buffer, i32 &bufferPosition, i32k bufferSize, T *data, size_t length, size_t elems);

	void ClearListeners();

	typedef std::map<EntityId, EntityId> ActorWeaponListenerMap;
	ActorWeaponListenerMap m_actorWeaponListener;

	void SetNewWeaponListener(IWeapon* pWeapon, EntityId weaponId, EntityId actorId);
	void RemoveWeaponListener(EntityId weaponId);
	void RemoveAllWeaponListeners();

	tukk GetActorItemName(EntityId actorId);
	tukk GetItemName(EntityId weaponId);
	tukk GetItemName(IItem* pItem);

	const bool IsClientMVP(CGameRules* pGameRules) const;

	static float GetRatio(i32 a, i32 b);

	typedef std::vector<char> TDescriptionVector;
#ifndef _RELEASE
	static void CmdDumpTelemetryDescription(IConsoleCmdArgs* pCmdArgs);
	static void CmdSetStat(IConsoleCmdArgs* pCmdArgs);
	static void CmdTestStats(IConsoleCmdArgs* pCmdArgs);
	template <class T>
	static void CreateDescriptionNode(i32 &pos, TDescriptionVector& buffer, tukk codeName, size_t size, tukk type, tukk mapName, T testValue);

	static void WriteDescBuffer(TDescriptionVector& buffer, string text);
#endif

	struct SMapParam
	{
		SMapParam(tukk name, EStatsFlags flag)
		{
			m_name = name;
			m_flag = flag;
		};

		tukk m_name;
		EStatsFlags m_flag;
		TMapParams m_mapParam;
	};

	static SMapParam s_mapParams[];
	void InitMapParams();
	void InitStreakParams();
	void ClearMapParams();
	TMapParams* GetMapParams(tukk name);
	void SaveMapStat(IPlayerProfile* pProfile, SSessionStats* pSessionStats, i32 index);
	void LoadMapStat(IPlayerProfile* pProfile, SSessionStats* pSessionStats, i32 index);

	i32k MapParamCount(u32k flags);
	tukk MapParamName(u32k flags, i32k index);
	const bool IsMapParam(u32k flag, tukk paramName);

	bool IsMultiplayerMapName(tukk name) const;

	bool NearFriendly(CPlayer *pClientPlayer, float distanceSqr = 16.0f);
	bool NearEnemy(CPlayer *pClientPlayer, float distanceSqr = 16.0f);
	EntityId ClientNearCorpse(CPlayer *pClientActor);

	void OnEntityKilledCheckSpecificMultiKills(const HitInfo &hitInfo, CPlayer* pShooterPlayer);
	bool CheckPreviousKills(u32 killsToCheck, i32 desiredHitType, bool bEnvironmental, bool bIsPole) const;

	bool ShouldProcessOnEntityKilled(const HitInfo &hitInfo);
	void UpdateGamemodeTime(tukk gamemodeName, SSessionStats* pClientStats, CActor* pClientActor);
	void UpdateWeaponTime(CWeapon* pCWeapon);
	void UpdateModulesTimes();
	void OnGameStarted();

	const static i32 k_noMoreMerryMenHunterKillsNeeded = 10;
	const static i32 k_20MetreHighClubVTOLHMGKillsNeeded = 10;
	const static i32 k_crouchesToggleNeeded = 4;
	const static i32 k_warbirdTimeFromCloak = 5;
	const static float k_cloakedVictimTimeFromCloak;
	const static float k_delayDetonationTime;
	const static float k_longDistanceThrowKillMinDistanceSquared;
	static bool s_multiplayer;

	bool m_crouching;
	float m_crouchToggleTime[k_crouchesToggleNeeded];

	u16 m_pickAndThrowWeaponClassId;

	CWeapon* m_selectedWeapon;

	u16 m_selectedWeaponId;

	float m_selectedWeaponTime;
	float m_gamemodeStartTime;
	float m_lastTimeMPTimeUpdated;

	float m_lastModeChangedTime;
	float m_fSecondTimer;
	i32		m_idleTime;

	bool m_gamemodeTimeValid;
	bool m_inGame;
	bool m_bHasCachedStartingStats;
	bool m_bSentStats;	//will be false at level unload if we weren't allowed to send stats
	bool m_localPlayerInVTOL;

	i32	m_DateFirstPlayed_Year;
	i32	m_DateFirstPlayed_Month;
	i32	m_DateFirstPlayed_Day;

	//Used for Telemetry uploading
	static i32 s_levelNamesVersion;
	static tukk sz_levelNames[];
	static tukk sz_weaponNames[];
};

#endif
