// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 07:12:2009		Created by Ben Parbury
*************************************************************************/

#ifndef __PLAYERPROGRESSION_H__
#define __PLAYERPROGRESSION_H__


#include <drx3D/Game/AutoEnum.h>
#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/PlayerProgressionTypes.h>
#include <drx3D/Game/GameRulesModules/IGameRulesClientScoreListener.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>
#include <drx3D/Game/ProgressionUnlocks.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Game/Network/Lobby/GameLobbyUpr.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>

class CPlayer;
struct IFlashPlayer;

#define DEBUG_XP_ALLOCATION !defined(_RELEASE) && !DRX_PLATFORM_ORBIS

#define MAX_EXPECTED_UNLOCKS_PER_RANK 5

enum EPPData
{
	EPP_Rank = 0,
	EPP_MaxRank,
	EPP_DisplayRank,
	EPP_MaxDisplayRank,
	EPP_XP,
	EPP_LifetimeXP,
	EPP_XPToNextRank,
	EPP_XPLastMatch,
	EPP_XPMatchStart,
	EPP_MatchStartRank,
	EPP_MatchStartDisplayRank,
	EPP_MatchStartXPInCurrentRank,
	EPP_MatchStartXPToNextRank,
	EPP_MatchBonus,
	EPP_MatchScore,
	EPP_MatchXP,
	EPP_XPInCurrentRank,
	EPP_NextRank,
	EPP_NextDisplayRank,
	EPP_Reincarnate,
	EPP_CanReincarnate,
	EPP_SkillRank,
	EPP_SkillKillXP
};

enum ECompletionStartValues
{
	eCSV_Weapons = 0,
	eCSV_Loadouts,
	eCSV_Attachments,
	eCSV_GameTypes,
	eCSV_Assessments,
	eCSV_NUM
};

struct IPlayerProgressionEventListener
{
	virtual	~IPlayerProgressionEventListener(){}
	virtual void OnEvent(EPPType type, bool skillKill, uk data) = 0;
};

class CPlayerProgression : public SGameRulesListener, public IGameRulesClientScoreListener, public IPlayerProfileListener,  public IPrivateGameListener
{
public:
	
	const static i32 k_PresaleCount = 4;

public:

	CPlayerProgression();
	virtual ~CPlayerProgression();

	static CPlayerProgression* GetInstance();

	void Init();
	void PostInit();

	void ResetUnlocks();
	void OnUserChanged();

	void Event(EPPType type, bool skillKill = false, uk data = NULL);
	void ClientScoreEvent(EGameRulesScoreType type, i32 points, EXPReason inReason, i32 currentTeamScore);

	bool SkillKillEvent(CGameRules* pGameRules, IActor* pTargetActor, IActor* pShooterActor, const HitInfo &hitInfo, bool firstBlood);
	void SkillAssistEvent(CGameRules *pGameRules, IActor* pTargetActor, IActor* pShooterActor, const HitInfo &hitInfo);
	i32 SkillAssessmentEvent(i32 points /*, EXPReason inReason*/);
	
	void Update(CPlayer* pPlayer, float deltaTime, float fHealth);
#ifndef _RELEASE
	void UpdateDebug();
#endif

	//SGameRulesListener
	virtual void EnteredGame();
	virtual void GameOver(EGameOverType localWinner, bool isClientSpectator);

	//IPlayerProfileListener
	virtual void SaveToProfile(IPlayerProfile* pProfile, bool online, u32 reason);
	virtual void LoadFromProfile(IPlayerProfile* pProfile, bool online, u32 reason);

	//IPrivateGameListener
	virtual void SetPrivateGame(const bool privateGame);

	i32k GetData(EPPData dataType);
	i32 GetMatchBonus(EGameOverType localWinner, float fracTimePlayed) const;

	float GetProgressWithinRank(i32k forRank, i32k forXp);
	i32k GetMaxRanks() const { return k_maxPossibleRanks; }
	static i32k GetMaxDisplayableRank() { return k_maxDisplayableRank; }
	i32k GetMaxReincarnations() const { return k_maxReincarnation; }


	i32k GetXPForRank(u8k rank) const;
	u8 GetRankForXP(i32k xp) const;
	tukk GetRankName(u8 rank, bool localize=true);
	tukk GetRankNameUpper(u8 rank, bool localize=true);
  bool HaveUnlocked(EUnlockType type, tukk name, SPPHaveUnlockedQuery &results) const;

	//only checks rank and custom class unlocks (only needed for weapons and modules atm)
	void GetUnlockedCount( EUnlockType type, u32* outUnlocked, u32* outTotal ) const;

#if !defined(_RELEASE)
	static void CmdGainXP(IConsoleCmdArgs* pCmdArgs);
	static void CmdGameEnd(IConsoleCmdArgs* pCmdArgs);
	static void CmdUnlockAll(IConsoleCmdArgs* pCmdArgs);
	static void CmdUnlocksNow(IConsoleCmdArgs* pCmdArgs);
	static void CmdResetXP(IConsoleCmdArgs* pCmdArgs);
	static void CmdGainLevels( IConsoleCmdArgs* pCmdArgs );

	void DebugFakeSetProgressions(int8 fakeRank, int8 fakeDefault, int8 fakeStealth, int8 fakeArmour);
	static void CmdFakePlayerProgression(IConsoleCmdArgs *pArgs);
	static void CmdGenerateProfile(IConsoleCmdArgs *pArgs);
#endif

	void AddEventListener(IPlayerProgressionEventListener* pEventListener);
	void RemoveEventListener(IPlayerProgressionEventListener* pEventListener);

	static tukk GetEPPTypeName(EPPType type);
	static void UpdateLocalUserData();

	void AddUINewDisplayFlags(u32 flags) { m_uiNewDisplayFlags |= flags; }
	void RemoveUINewDisplayFlags(u32 flags) { m_uiNewDisplayFlags &= ~flags; }
	u32 CheckUINewDisplayFlags(u32 flags) const { return ((m_uiNewDisplayFlags&flags)!=0); }
	u32 GetUINewDisplayFlags() const { return m_uiNewDisplayFlags; }

	void Reincarnate();

	void UnlockCustomClassUnlocks(bool rewards = true);

	// Unlocks given unlocks. Returns given XP (doesn't apply XP itself)
	i32 UnlockPresale( u32 id, bool showPopup, bool isNew );

	i32 IncrementXPPresale(i32 ammount, EXPReason inReason);
	i32 GetXPForEvent(EPPType type) const;
	

	void OnQuit();
	void OnEndSession();
	void OnFinishMPLoading();

	static void ReserveUnlocks(XmlNodeRef xmlNode, TUnlockElements& unlocks, TUnlockElements& allowUnlocks);
	static void LoadUnlocks(XmlNodeRef xmlNode, i32k rank, TUnlockElements& unlocks, TUnlockElements& allowUnlocks);

	i32 GetUnlockedTokenCount(EUnlockType type, tukk paramName);

	void GetRankUnlocks(i32 firstrank, i32 lastrank, i32 reincarnations, TUnlockElementsVector &unlocks) const;

	void HaveUnlockedByRank(EUnlockType type, tukk name, SPPHaveUnlockedQuery &results, i32k rank, i32k  reincarnations, const bool bResetResults) const;

	bool AllowedWriteStats() const;

	bool WillReincarnate () const;

protected:
	void Reset();
	void InitRanks(tukk filename);
	void InitEvents(tukk filename);
	void InitCustomClassUnlocks(tukk filename);
	void InitConsoleCommands();

	void InitPresaleUnlocks();
	void InitPresaleUnlocks( tukk pScriptName );
	i32 UnlockPresale( tukk filename, bool showPopup, bool isNew );

	void SanityCheckRanks();
	void SanityCheckSuitLevels();

	i32 CalculateRankFromXp(i32 xp);

	i32 EventInternal(i32 points, EXPReason inReason, bool bSkillKill=false);

	i32 IncrementXP(i32 amount, EXPReason inReason);

	void MatchBonus(const EGameOverType localWinner, const float totalTime);
	float WinModifier(const EGameOverType localWinner) const;

	void ReincarnateUnlocks();

	void UpdateAllUnlocks(bool fromProgress);	//from Progress as opposed to from Load
	
	void ResetXP();
	void UpdateStartRank();

	void SendEventToListeners(EPPType, bool, uk );

	const bool IsSkillAssist(const EPPType type) const;

	static i32 GetUnlockedTokenCount(const TUnlockElements& unlockList, EUnlockType type);

	typedef std::vector<IPlayerProgressionEventListener*> TEventListener;
	TEventListener m_eventListeners;

	struct SRank
	{
		SRank(XmlNodeRef node);

		const static i32 k_maxNameLength = 16;
		char m_name[k_maxNameLength];
		i32 m_xpRequired;
	};

  struct SUnlockedItem
  {
    SUnlockedItem(EUnlockType type = eUT_Invalid, tukk  name = NULL, i32 rank = -1)
    {
      m_type = type;
      m_name = name;
      m_rank = rank;
    }

    EUnlockType m_type;
    tukk  m_name;
    i32 m_rank;
  };

	static CPlayerProgression* s_playerProgression_instance;
	const static float k_queuedRankTime;
	enum { k_maxPossibleRanks=51,
	       k_maxDisplayableRank=50,
	       k_risingStarRank=20 }; //Unlock achievement when reaching this rank
	typedef DrxFixedArray<SRank,k_maxPossibleRanks > TRankElements;
	TRankElements m_ranks;
	i32 m_maxRank;

	const static i32 s_nPresaleScripts = 1;
	static tukk s_presaleScriptNames[ s_nPresaleScripts ];

	const static i32 k_maxEvents = EPP_Max;
	i32 m_events[k_maxEvents];

	TUnlockElements m_unlocks;				//unlocks in the ranks.xml
	TUnlockElements m_customClassUnlocks;	//unlocks for the custom class (customclass.xml)
	TUnlockElements m_presaleUnlocks;	//unlocks from presale

	TUnlockElements m_allowUnlocks; 	//Determine when you're allowed to unlock things

	float m_time;

	CAudioSignalPlayer m_rankUpSignal;
	i32 m_xp;
	i32 m_lifetimeXp;
	i32 m_matchScore;
	i32 m_gameStartXp;
	i32 m_rank;
	i32 m_gameStartRank;
	i32 m_matchBonus;
	const static i32 k_maxReincarnation = 5;
	i32 m_reincarnation;
	i32 m_initialLoad;
	i32 m_skillRank;
	i32 m_skillKillXP;

	u32	m_uiNewDisplayFlags;	// Flags for which buttons wants want to display a 'new' icon.

	i32 m_queuedRankForHUD;
	i32 m_queuedXPRequiredForHUD;
	float m_rankUpQueuedTimer;
	bool m_rankUpQueuedForHUD;

	bool m_hasNewStats;
	bool m_privateGame;

#if DEBUG_XP_ALLOCATION
	typedef std::vector<string> TDebugXPVector;
	TDebugXPVector m_debugXp;
#endif
};

#endif // __PLAYERPROGRESSION_H__
