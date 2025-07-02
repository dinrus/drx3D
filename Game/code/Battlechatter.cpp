// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание:
Plays sounds for nearby actors
**************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Battlechatter.h>

#include <drx3D/Game/Player.h>
#include <drx3D/Game/GameRules.h>

#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Game/Audio/GameAudioUtils.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/WeaponSystem.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/Game/Projectile.h>
#include <drx3D/Game/RecordingSystem.h>
#include <drx3D/Game/ActorUpr.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStateModule.h>
#include <drx3D/Game/PlayerVisTable.h>
#include <drx3D/Game/Utility/DesignerWarning.h>
#include <drx3D/Game/Utility/DrxDebugLog.h>
#include <drx3D/Game/GameRulesModules/GameRulesObjective_Predator.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>

#define DbgLog(...) DRX_DEBUG_LOG(BATTLECHATTER, __VA_ARGS__)
#define DbgLogAlways(...) DRX_DEBUG_LOG_ALWAYS(BATTLECHATTER, __VA_ARGS__)

static AUTOENUM_BUILDNAMEARRAY(s_battlechatterName, BATTLECHATTER_LIST);

#define BATTLECHATTER_DATA_FILE "Scripts/Sounds/BattlechatterData.xml"
#define BATTLECHATTER_VARS_FILE "Scripts/Sounds/BattlechatterVars.xml"


CBattlechatter::UpdateFunc CBattlechatter::s_updateFunctions[] = 
{
	&CBattlechatter::UpdateMovingUpEvent,
	&CBattlechatter::UpdateSeeGrenade,
	&CBattlechatter::UpdateQuiet,
	&CBattlechatter::UpdateNearEndOfGame,
};



#if !defined(_RELEASE)
static i32 bc_debug = 0;
static i32 bc_debugPlayAlways = 0;
static i32 bc_signals = 0;
static i32 bc_signalsDetail = -1;
static i32 bc_playForAllActors = 0;
static i32 bc_chatter = 0;
static i32 bc_allowRepeats = 0;
static float bc_chatterFreq = 0.005f;
#else
const static i32 bc_allowRepeats = 0;
#endif

#if !defined(_RELEASE)
struct SBattlehchatterAutoComplete : public IConsoleArgumentAutoComplete
{
	virtual i32 GetCount() const { return BC_Last; };
	virtual tukk GetValue( i32 nIndex ) const { return s_battlechatterName[nIndex]; };
};

static SBattlehchatterAutoComplete s_battlechatterAutoComplete;
#endif
//-------------------------------------------------------
CBattlechatter::CBattlechatter()
{
	m_clientPlayer = NULL;
	m_lastTimePlayed = 0.0f;
	m_lastTimeExpectedFinished = 0.0f;
	for(i32 i = 0; i < k_recentlyPlayedListSize; i++)
	{
		m_recentlyPlayed.push_back(BC_Null);
	}
	m_recentlyPlayedIndex = 0;
	m_clientNetChatter = BC_Null;
	m_serialiseCounter = 0;
	m_currentActorIndex = 0;
	m_currentActorEventIndex = 0;

	m_voice.clear();

	m_visualChatter.clear();

	Init();
}

//-------------------------------------------------------
CBattlechatter::~CBattlechatter()
{
	UnRegisterCVars();

	if(CGameRules* pGameRules = g_pGame->GetGameRules())
	{
		pGameRules->UnRegisterKillListener(this);
		pGameRules->UnRegisterTeamChangedListener(this);
	}

	if(g_pGame->GetGameAudio())
	{
		// Remove any previously registered functors from this class.
		g_pGame->GetGameAudio()->GetUtils()->UnregisterSignal(m_signalPlayer);
	}
}

//-------------------------------------------------------
void CBattlechatter::Init()
{
	DrxLog("[Battlechatter] Initializing...");
	INDENT_LOG_DURING_SCOPE();

	InitVoices();
	
	InitData();
	InitVars();

	RegisterCVars();

	CGameRules* pGameRules = g_pGame->GetGameRules();
	pGameRules->RegisterKillListener(this);
	pGameRules->RegisterTeamChangedListener(this);
}

//-------------------------------------------------------
void CBattlechatter::InitVoices()
{
	i32 voiceCount = 0;
	SVoice voice;
	while(voice.Init(voiceCount))
	{
		DrxLog("[Battlechatter] Successfully loaded voice %d", voiceCount + 1);
		m_voice.push_back(voice);
		voice.clear();
		voiceCount++;
	}
}

//-------------------------------------------------------
void CBattlechatter::InitData()
{
	XmlNodeRef xmlData = GetISystem()->LoadXmlFromFile(BATTLECHATTER_DATA_FILE);

	if(xmlData)
	{
		if( (i32)BC_Last != xmlData->getChildCount() )
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Missing Battlechatter Valid Data, expected %d nodes, got %d!", (i32)BC_Last, xmlData->getChildCount());

		i32k maxChatter = min((i32)BC_Last, xmlData->getChildCount());
		for(i32 i = 0; i < maxChatter; i++)
		{
			XmlNodeRef chatterNode = xmlData->getChild(i);
			if(chatterNode)
			{
				DRX_ASSERT_MESSAGE(strcmpi(chatterNode->getTag(), s_battlechatterName[i]) == 0, ("Expected %s, not %s", s_battlechatterName[i], chatterNode->getTag()));
				DRX_ASSERT_MESSAGE(m_data.size() == i, "mismatch of battlechatter inserting into our m_data array");

				SBattlechatterData newData;

				float range = 30.0f;
				chatterNode->getAttr("range", range);
				newData.m_range = range;

				float chance = 1.0f;
				chatterNode->getAttr("chance", chance);
				newData.m_chance = chance;

				newData.m_repeatChatter = ReadChatterFromNode("repeatChatter", chatterNode);
				newData.m_cloakedChatter = ReadChatterFromNode("cloakedChatter", chatterNode);
				newData.m_armourChatter = ReadChatterFromNode("armourChatter", chatterNode);

				i32 enemyCanSpeak = 0;
				i32 enemyHunterCanSpeak = 0;
				i32 friendlyHunterCanSpeak = 0;
				chatterNode->getAttr("enemyCanSpeak", enemyCanSpeak);
				chatterNode->getAttr("enemyHunterCanSpeak", enemyHunterCanSpeak);
				chatterNode->getAttr("friendlyHunterCanSpeak", friendlyHunterCanSpeak);

				newData.m_flags |= (enemyCanSpeak==1) ? SBattlechatterData::k_BCD_flags_enemy_can_speak : SBattlechatterData::k_BCD_flags_none;
				newData.m_flags |= (enemyHunterCanSpeak==1) ? SBattlechatterData::k_BCD_flags_enemy_hunter_can_speak : SBattlechatterData::k_BCD_flags_none;
				newData.m_flags |= (friendlyHunterCanSpeak==1) ? SBattlechatterData::k_BCD_flags_friendly_hunter_can_speak : SBattlechatterData::k_BCD_flags_none;

				newData.m_netMinTimeBetweenSends=0.f;
				chatterNode->getAttr("netMinTimeBetweenSends", newData.m_netMinTimeBetweenSends);
				newData.m_netLastTimePlayed=0.f;

				m_data.push_back(newData);
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Missing Battlechatter Valid Data, expected '%s' in '%s'!", s_battlechatterName[i], BATTLECHATTER_DATA_FILE);
			}
		}
	}
}

//-------------------------------------------------------
void CBattlechatter::InitVars()
{
	XmlNodeRef xml = GetISystem()->LoadXmlFromFile(BATTLECHATTER_VARS_FILE);
	if( xml )
	{
		i32 iChild = 0;
		i32 iChildCount = xml->getChildCount();

		//Load each node in-order, expect it to match the vars defined in the header file
#define READ_BATTLECHATTER_VARS_FROM_XML(myType, myName)	\
		if(iChild >= iChildCount)																													\
		{																																									\
			DesignerWarning(false, "Unable to find node for %s", #myName);									\
			return;																																					\
		}																																									\
		XmlNodeRef myName##Xml = xml->getChild(iChild);																		\
		if(strcmpi(myName##Xml->getTag(), #myName) == 0)																	\
		{																																									\
			myName##Xml->getAttr("value", myName);																					\
			DrxLog("CBattlechatter::InitVars() found var=%s value of %f from the xml", #myName, (float)(myName)); \
		}																																									\
		else																																							\
		{																																									\
			DesignerWarning(false, "Expected tag %s (tags must be in order)", #myName);			\
			return;																																					\
		}																																									\
		iChild++;																																					\

		BATTLECHATTER_VARS(READ_BATTLECHATTER_VARS_FROM_XML);

#undef READ_BATTLECHATTER_VARS_FROM_XML
	}
}

//-------------------------------------------------------
EBattlechatter CBattlechatter::ReadChatterFromNode(tukk chatterName, XmlNodeRef chatterNode)
{
	tukk  pChatterNameFromNode = NULL;
	if(chatterNode->getAttr(chatterName, &pChatterNameFromNode))
	{
		i32 chatterType = BC_Null;
		if(AutoEnum_GetEnumValFromString(pChatterNameFromNode, s_battlechatterName, BC_Last, &chatterType))
		{
			return (EBattlechatter) chatterType;
		}
		DRX_ASSERT_MESSAGE(false, ("Unknown chatter found in tag '%s'", chatterName));
	}
	return BC_Null;
}

//-------------------------------------------------------
CBattlechatter::SVoice::SVoice()
{
	clear();
}

//-------------------------------------------------------
void CBattlechatter::SVoice::clear()
{
	m_chatter.clear();
}

//-------------------------------------------------------
bool CBattlechatter::SVoice::Init(i32 voiceIndex)
{
	i32 validChatterCount = 0;
	CGameRules *pGameRules = g_pGame->GetGameRules();
	u8 requiredPlayerTypeForGameMode = pGameRules->GetRequiredPlayerTypesForGameMode();

	for(i32 chatterIndex = 0; chatterIndex < BC_Last; chatterIndex++)
	{
		SVoice::SVoiceSignals voiceSignals;

		if (requiredPlayerTypeForGameMode & CGameRules::k_rptfgm_standard)
		{
			voiceSignals.m_standard = GetChatterSignal(voiceIndex, chatterIndex, "");
		}
		if (requiredPlayerTypeForGameMode & CGameRules::k_rptfgm_marines)
		{
			voiceSignals.m_marine = GetChatterSignal(voiceIndex, chatterIndex, "_Marine");
		}
		if (requiredPlayerTypeForGameMode & CGameRules::k_rptfgm_hunter)
		{
			voiceSignals.m_hunter = GetChatterSignal(voiceIndex, chatterIndex, "_Hunter");
		}
		if (requiredPlayerTypeForGameMode & CGameRules::k_rptfgm_hunter_marine)
		{
			voiceSignals.m_hunterMarine = GetChatterSignal(voiceIndex, chatterIndex, "_HunterMarine");
		}

		DbgLog("CBattlechatter::SVoice::Init() voiceIndex=%d; chatter=%s (%d) signals: standard=%s (%d); marine=%s (%d); hunter=%s (%d); hunterMarine=%s (%d)",
				voiceIndex, s_battlechatterName[chatterIndex], chatterIndex, 
				CAudioSignalPlayer::GetSignalName(voiceSignals.m_standard), voiceSignals.m_standard,
				CAudioSignalPlayer::GetSignalName(voiceSignals.m_marine), voiceSignals.m_marine,
				CAudioSignalPlayer::GetSignalName(voiceSignals.m_hunter), voiceSignals.m_hunter,
				CAudioSignalPlayer::GetSignalName(voiceSignals.m_hunterMarine), voiceSignals.m_hunterMarine);

		m_chatter.push_back(voiceSignals);	// have to push back now that we're using a cryfixedarray to ensure spacing is correct for each chatter

		if (voiceSignals.m_standard != INVALID_AUDIOSIGNAL_ID)
		{
			validChatterCount++;
		}
	}

	i32k k_minChatterCount = (BC_Last/2);
	const bool validVoice = validChatterCount > k_minChatterCount;
#if !defined(_RELEASE)
	if(validVoice == false && validChatterCount > 0)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "[Battlechatter] Tried to load voice %d but only found %d chatter types (%d needed)", voiceIndex + 1, validChatterCount, k_minChatterCount);
	}
#endif
	return validVoice;
}

//-------------------------------------------------------
TAudioSignalID CBattlechatter::SVoice::GetChatterSignal(i32 voiceIndex, i32 chatterIndex, tukk suffixStr)
{
	string signalName = "";
	signalName.Format("%s_TM%d%s", s_battlechatterName[chatterIndex], voiceIndex + 1, suffixStr);

	return g_pGame->GetGameAudio()->GetSignalID(signalName);
}

//-------------------------------------------------------
void CBattlechatter::SetLocalPlayer(CPlayer* pPlayer)
{
	m_clientPlayer = pPlayer;
}

//-------------------------------------------------------
void CBattlechatter::Update(const float dt)
{
	if(m_clientPlayer)
	{
		const CActorUpr * pActorUpr = CActorUpr::GetActorUpr();
		pActorUpr->PrepareForIteration();
		i32k nNumActors = pActorUpr->GetNumActors();
		if(nNumActors > 0)
		{
			if(m_currentActorIndex >= nNumActors)
			{
				m_currentActorIndex = 0;
			}

			UpdateActorBattlechatter(m_currentActorIndex, pActorUpr);	//increments m_currentActorIndex
		}

#if !defined(_RELEASE)
		Debug();
#endif
	}
}

//-------------------------------------------------------
void CBattlechatter::NearestActorEvent(EBattlechatter chatter)
{
	if(m_clientPlayer)
	{
		Event(chatter, GetNearestHearableFriendlyActor(*m_clientPlayer));
	}
}

//-------------------------------------------------------
void CBattlechatter::Event(EBattlechatter requestedChatter, EntityId actorId)
{
	DRX_ASSERT_MESSAGE(requestedChatter > BC_Null && requestedChatter < BC_Last, "invalid chatter index");

	CActor* pActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(actorId));
	if(pActor && UpdateEvent(requestedChatter, actorId, pActor))
	{
		float currentTime = gEnv->pTimer->GetCurrTime();

		EBattlechatter chatter = requestedChatter;
		if(IsLastPlayed(chatter) && CanPlayDitto(currentTime))
		{
			EBattlechatter repeatChatter = m_data[requestedChatter].m_repeatChatter;
			if(repeatChatter != BC_Null)
			{
				chatter = repeatChatter;
			}
		}

		if(m_clientPlayer && !IsRecentlyPlayed(chatter) && ShouldClientHear(chatter) && ShouldPlayForActor(actorId, m_data[chatter].m_range) && ChanceToPlay(chatter))
		{
			SVoiceInfo* pInfo = GetVoiceInfo(actorId);
			if(pInfo && (ShouldIgnoreRecentlyPlayed(chatter, requestedChatter) || !PlayedRecently(pInfo, currentTime)))
			{
				//Play(pInfo, chatter, pActor, actorId, currentTime);
				REINST("needs verification!");
				m_lastTimePlayed = currentTime;
				m_lastTimeExpectedFinished = currentTime + bc_defaultLoadTimeout;
				//g_pGame->GetGameAudio()->GetUtils()->GetPlayingSignalLength(m_signalPlayer, functor(*this, &CBattlechatter::OnLengthCallback));
				AddPlayed(requestedChatter);	//stores requested chatter (not Dittos, so you can have multiple Dittos)
			}
		}
	}
}

//------------------------------------------------------------------------
//void CBattlechatter::OnLengthCallback(const bool& success, const float& length, ISound* pSound)
//{
//	if(success)
//	{
//		m_lastTimeExpectedFinished = m_lastTimePlayed + length;
//	}
//	else
//	{
//		m_lastTimeExpectedFinished = m_lastTimePlayed;
//	}
//}

//-------------------------------------------------------
bool CBattlechatter::UpdateEvent(EBattlechatter& chatter, EntityId &actorId, IActor* pActor)
{

	if(pActor->IsPlayer() && m_clientPlayer)
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(pActor);

		EBattlechatter inChatter = chatter;	// cache chatter so tests are always done against original "parent" chatter rather than any new stealth/armour variants
		CGameRules* pGameRules = g_pGame->GetGameRules();
		
		// if we don't have a valid chatter then we MUST return false to ensure chatter isn't used to index out of bounds of m_data
		if (chatter == BC_Null)
		{
			return false;
		}

		if (pGameRules->GetGameMode() == eGM_Gladiator)
		{
			i32 localTeamId = pGameRules->GetTeam(m_clientPlayer->GetEntityId());
			
			if (localTeamId == CGameRulesObjective_Predator::TEAM_SOLDIER)
			{
				// local player as soldier can only hear friendlies (handled by falling through to last return), and enemy hunter lines as flagged
				if (!m_clientPlayer->IsFriendlyEntity(actorId))
				{
					return ((m_data[inChatter].m_flags & SBattlechatterData::k_BCD_flags_enemy_hunter_can_speak) != 0);
				}
			}
			else
			{
				// local player as hunter can only hear friendly hunters that are flagged, and enemy marines that are flagged
				if (m_clientPlayer->IsFriendlyEntity(actorId))
				{
					return ((m_data[inChatter].m_flags & SBattlechatterData::k_BCD_flags_friendly_hunter_can_speak) != 0);
				}
				else
				{
					return ((m_data[inChatter].m_flags & SBattlechatterData::k_BCD_flags_enemy_can_speak) != 0);
				}
			}
		}
		else
		{
			// local player can only hear friendlies (handled by falling through to last return), and enemies that are flagged
			if(!m_clientPlayer->IsFriendlyEntity(actorId))
			{
				return ((m_data[inChatter].m_flags & SBattlechatterData::k_BCD_flags_enemy_can_speak) != 0);
			}
		}
	}

	return true;
}

//-------------------------------------------------------
CBattlechatter::SVoiceInfo* CBattlechatter::GetVoiceInfo(EntityId actorId)
{
	i32k voiceCount = m_voice.size();
	if(voiceCount > 0)
	{
		ActorVoiceMap::iterator it = m_actorVoice.find(actorId);

		if(it == m_actorVoice.end())	//add new voice if we need one
		{
			SVoiceInfo voiceInfo(GetVoiceIndex(m_clientPlayer && m_clientPlayer->IsFriendlyEntity(actorId)));
			it = m_actorVoice.insert(ActorVoiceMap::value_type(actorId, voiceInfo)).first;
		}

		return &it->second;
	}
	return NULL;
}

//-------------------------------------------------------
void CBattlechatter::RegisterCVars()
{
#if !defined(_RELEASE)

	if (gEnv->pConsole)
	{
#define BATTLECHATTER_VAR_REGISTER(myType, myName)    REGISTER_CVAR2(# myName, & myName, myName, VF_NULL, "");

		BATTLECHATTER_VARS(BATTLECHATTER_VAR_REGISTER)

#undef BATTLECHATTER_VAR_REGISTER
	}


	REGISTER_CVAR(bc_debug, 0, 0, "Enable/Disables battlechatter debug messages");
	REGISTER_CVAR(bc_debugPlayAlways, 0, 0, "Always play battlechatter rather than only if ChanceToPlay() allows it");
	REGISTER_CVAR(bc_signals, 0, 0, "Enables watch for nth voice showing battlechatter signal counts");
	REGISTER_CVAR(bc_signalsDetail, -1, 0, "Enable watch battlechatter signal names and signalIds - set to larger than signal max to show all");
	REGISTER_CVAR(bc_playForAllActors, bc_playForAllActors, 0, "lets your character say battlechatter");
	REGISTER_CVAR(bc_chatter, bc_chatter, 0, "Enable/Disable random battlechatter playing repeatedly on all hearable players");
	REGISTER_CVAR(bc_chatterFreq, bc_chatterFreq, 0, "frequency of chatter for bc_chatter");
	REGISTER_CVAR(bc_allowRepeats, bc_allowRepeats, 0, "allow the same line to play repeatedly");

	if (gEnv->pConsole)
	{
		REGISTER_COMMAND("bc_play", CmdPlay, VF_CHEAT, "play battlechatter and nearby player");
		gEnv->pConsole->RegisterAutoComplete("bc_play", & s_battlechatterAutoComplete);
		REGISTER_COMMAND("bc_clearRecentlyPlayed", CmdClearRecentlyPlayed, VF_CHEAT, "clear recently played list. Makes it easier when debugging battlechatter");
		REGISTER_COMMAND("bc_dump", CBattlechatter::CmdDump, VF_CHEAT, "dump all battlechatter data");
		REGISTER_COMMAND("bc_dumpPlayed", CBattlechatter::CmdDumpPlayed, VF_CHEAT, "dump all played battlechatter data");
		REGISTER_COMMAND("bc_dumpUnPlayed", CBattlechatter::CmdDumpUnPlayed, VF_CHEAT, "dump all unplayed battlechatter data");
		REGISTER_COMMAND("bc_clearPlayCount", CBattlechatter::CmdClearPlayCount, VF_CHEAT, "clear the playcount of all battlechatter data");	
	}
#endif
}

//-------------------------------------------------------
void CBattlechatter::UnRegisterCVars()
{
#if !defined(_RELEASE)

	if (gEnv->pConsole)
	{
#define BATTLECHATTER_VAR_UNREGISTER(myType, myName)    gEnv->pConsole->UnregisterVariable(# myName);

		BATTLECHATTER_VARS(BATTLECHATTER_VAR_UNREGISTER)

#undef BATTLECHATTER_VAR_UNREGISTER

		gEnv->pConsole->UnregisterVariable("bc_debug");
		gEnv->pConsole->UnregisterVariable("bc_debugPlayAlways");
		gEnv->pConsole->UnregisterVariable("bc_signals");
		gEnv->pConsole->UnregisterVariable("bc_signalsDetail");
		gEnv->pConsole->UnregisterVariable("bc_playForAllActors");
		gEnv->pConsole->UnregisterVariable("bc_chatter");
		gEnv->pConsole->UnregisterVariable("bc_chatterFreq");
		gEnv->pConsole->UnregisterVariable("bc_allowRepeats");

		gEnv->pConsole->RemoveCommand("bc_play");
		gEnv->pConsole->UnRegisterAutoComplete("bc_play");
		gEnv->pConsole->RemoveCommand("bc_clearRecentlyPlayed");
	
		gEnv->pConsole->RemoveCommand("bc_dump");
		gEnv->pConsole->RemoveCommand("bc_dumpPlayed");
		gEnv->pConsole->RemoveCommand("bc_dumpUnPlayed");
		gEnv->pConsole->RemoveCommand("bc_clearPlayCount");
	}
#endif //#if !defined(_RELEASE)
}

//-------------------------------------------------------
// a player has entered a VTOL can any hearable enemy actors see this player doing it
void CBattlechatter::PlayerHasEnteredAVTOL(const IEntity* pPlayerEntity)
{
	i32 playerTeam = g_pGame->GetGameRules()->GetTeam(pPlayerEntity->GetId());
	const Vec3 &playerPos = pPlayerEntity->GetPos();
	SomethingHasHappened(playerTeam, &playerPos, BC_SeeVTOL, bc_canSeeActorEnterVTOL_rangeSq, k_SHH_flags_consider_enemies);
}

//-------------------------------------------------------
// a player has activated a microwave beam can any hearable enemy actors see this beam
void CBattlechatter::MicrowaveBeamActivated(EntityId ownerPlayerEntityId, const Vec3 *deployedPos)
{
	i32 playerTeam = g_pGame->GetGameRules()->GetTeam(ownerPlayerEntityId);
	SomethingHasHappened(playerTeam, deployedPos, BC_SeeGamma, bc_canSeeMicrowaveBeam_rangeSq, k_SHH_flags_consider_enemies|k_SHH_flags_2D_range_test);
}

//-------------------------------------------------------
// A player has activated the the EMP Blast - a global attack (so ignore distance)
void CBattlechatter::EMPBlastActivated( const EntityId ownerPlayerEntityId, i32k playerTeam, const bool friendly )
{
	const Vec3 zero(ZERO);
	if(friendly)
	{
		SomethingHasHappened(playerTeam, &zero, BC_EMPBlastActivatedFriendly, FLT_MAX, k_SHH_flags_consider_teammates);
	}
	else
	{
		SomethingHasHappened(playerTeam, &zero, BC_EMPBlastActivatedHostile, FLT_MAX, k_SHH_flags_consider_enemies);
	}
}

void CBattlechatter::LocalPlayerHasGotObjective()
{
	if(m_clientPlayer)
	{
		i32 localPlayerTeam = g_pGame->GetGameRules()->GetTeam(m_clientPlayer->GetEntityId());
		const Vec3 &localPlayerPos = m_clientPlayer->GetEntity()->GetPos();
		SomethingHasHappened(localPlayerTeam, &localPlayerPos, BC_PlayerObjectivePickup, bc_canSeeLocalPlayerGetObjective_rangeSq, k_SHH_flags_consider_teammates);
	}
}

void CBattlechatter::PlayerIsDownloadingDataFromTerminal(const EntityId playerEntityId)
{
	i32 playerTeam = g_pGame->GetGameRules()->GetTeam(playerEntityId);
	IEntity* pPlayerEntity = gEnv->pEntitySystem->GetEntity(playerEntityId);
	if (pPlayerEntity)
	{
		const Vec3 &playerPos = pPlayerEntity->GetPos();

		SomethingHasHappened(playerTeam, &playerPos, BC_DataTransferring, bc_canSeePlayerDownloadingData_rangeSq, k_SHH_flags_consider_teammates);
	}
}

//-------------------------------------------------------
void CBattlechatter::ClearRecentlyPlayed()
{
	for(i32 i = 0; i < k_recentlyPlayedListSize; i++)
	{
		m_recentlyPlayed[i] = BC_Null;
	}
	m_recentlyPlayedIndex = 0;
}

TAudioSignalID CBattlechatter::GetChatterSignalForActor(SVoiceInfo *pInfo, EBattlechatter chatter, CActor *pActor, i32 listeningActorTeamId)
{
	TAudioSignalID signal = INVALID_AUDIOSIGNAL_ID;
	CGameRules *pGameRules = g_pGame->GetGameRules();
	i32 actorEntityId = pActor->GetEntityId();
	i32 actorTeamId = pGameRules->GetTeam(actorEntityId);

	if (listeningActorTeamId == -1)
	{
		listeningActorTeamId = pGameRules->GetTeam(m_clientPlayer->GetEntityId());
	}

	DbgLog("CBattlechatter::GetChatterSignalForActor() voiceIndex=%d; chatter=%d (%s); actor=%s", pInfo->m_voiceIndex, chatter, s_battlechatterName[chatter], pActor->GetEntity()->GetName());

	if(pInfo->m_voiceIndex >= m_voice.size())
	{
		DrxLog("CBattleChatter::GetChatterSignalForActor() is trying to access beyond the ends of the array. This shouldn't happen!!");
#ifndef _RELEASE
		DrxFatalError("CBattleChatter::GetChatterSignalForActor() is trying to access beyond the ends of the array. voiceIndex=%d; m_voices.size()=%" PRISIZE_T "; This shouldn't happen!!", pInfo->m_voiceIndex, m_voice.size());
#endif
		return signal;
	}

	SVoice::SVoiceSignals &voiceSignals = m_voice[pInfo->m_voiceIndex].m_chatter[chatter];

	u8 playerTypeForConversation = pGameRules->GetRequiredPlayerTypeForConversation(actorTeamId, listeningActorTeamId);
	switch (playerTypeForConversation)
	{
		case CGameRules::k_rptfgm_standard:
			signal = voiceSignals.m_standard;
			break;
		case CGameRules::k_rptfgm_marines:
			signal = voiceSignals.m_marine;
			break;
		case CGameRules::k_rptfgm_hunter:
			signal = voiceSignals.m_hunter;
			break;
		case CGameRules::k_rptfgm_hunter_marine:
			signal = voiceSignals.m_hunterMarine;
			break;
	}

#if !defined(_RELEASE)
	if (signal == INVALID_AUDIOSIGNAL_ID)
	{
		DbgLog("CBattleChatter::GetChatterSignalForActor() failed to find a valid signal for chatter");
	}
#endif
	
	return signal;
}


//-------------------------------------------------------
//void CBattlechatter::Play(SVoiceInfo* pInfo, EBattlechatter chatter, CActor* pActor, EntityId actorId, float currentTime, ESoundSemantic semanticOverride/*=eSoundSemantic_None*/, i32 specificVariation/*=-1*/, i32 listeningActorTeamId /*=-1*/)
//{
//	if(ShouldPlay())
//	{
//		TAudioSignalID playedSignal = GetChatterSignalForActor(pInfo, chatter, pActor, listeningActorTeamId); 
//
//		if (playedSignal == INVALID_AUDIOSIGNAL_ID)
//		{
//			DbgLog("CBattlechatter::Play() failed to find valid signal from chatter=%s; voiceIndex=%d", s_battlechatterName[chatter], pInfo->m_voiceIndex);
//		}
//		else
//		{
//			m_signalPlayer.SetSignal(playedSignal);
//
//			DbgLog("CBattlechatter::Play() playing signal=%s (%d) from chatter=%s; voiceIndex=%d", m_signalPlayer.GetSignalName(), playedSignal, s_battlechatterName[chatter], pInfo->m_voiceIndex);
//		
//#ifndef _RELEASE
//			++m_data[chatter].m_playCount;
//#endif
//
//			m_signalPlayer.Play(actorId, "mpteam", GetMPTeamParam(pActor, actorId), semanticOverride, &specificVariation);
//			pInfo->m_lastTimePlayed = currentTime;
//
//			CRecordingSystem* pRecordingSystem = g_pGame->GetRecordingSystem();
//			if (pRecordingSystem)
//			{
//				// on playback this will use GetChatterSignalForActor() to pick the right voice
//				pRecordingSystem->OnBattleChatter(chatter, actorId, specificVariation);
//			}
//		}
//
//	}
//}

//-------------------------------------------------------
const bool CBattlechatter::ShouldPlay() const
{
	IGameRulesStateModule *pStateModule = g_pGame->GetGameRules()->GetStateModule();
	if (pStateModule != NULL && pStateModule->GetGameState() == IGameRulesStateModule::EGRS_InGame)
	{
		return (g_pGame->GetHostMigrationState() == CGame::eHMS_NotMigrating);
	}

	return false;
}

//-------------------------------------------------------
float CBattlechatter::GetMPTeamParam(CActor* pActor, EntityId actorId)
{
	if(pActor)
	{
		return 0.0f;
	}
	else if(m_clientPlayer && m_clientPlayer->IsFriendlyEntity(actorId))
	{
		return 1.0f;
	}
	else
	{
		return 2.0f;	//enemy
	}
}

bool CBattlechatter::IsSkillShot(const HitInfo &hitInfo)
{
	IActorSystem *pActorSystem = gEnv->pGame->GetIGameFramework()->GetIActorSystem();
	CActor* pShooterActor = static_cast<CActor*>(pActorSystem->GetActor(hitInfo.shooterId));

	if (pShooterActor)
	{
		const Vec3& shooterPos = pShooterActor->GetEntity()->GetPos();
		const Vec3& targetPos = hitInfo.pos;
		Vec3 posDiff = targetPos - shooterPos;
	
		if (posDiff.GetLengthSquared() > bc_minSkillShot_rangeSq)
		{
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------
void CBattlechatter::OnEntityKilled(const HitInfo &hitInfo)
{
	if(m_clientPlayer && hitInfo.shooterId && hitInfo.targetId)
	{
		const EntityId clientId = m_clientPlayer->GetEntityId();
		if(hitInfo.targetId != hitInfo.shooterId)
		{
			if(clientId == hitInfo.shooterId)
			{
				EntityId actorId = GetNearestHearableFriendlyActor(*m_clientPlayer);
				if(actorId)
				{
					Event(BC_PlayerKilledCongrats, actorId);
				}
			}
		}

		if(m_clientPlayer->IsFriendlyEntity(hitInfo.shooterId) != m_clientPlayer->IsFriendlyEntity(hitInfo.targetId))
		{
			if(clientId == hitInfo.targetId)
			{
				Event(BC_PlayerDown, hitInfo.shooterId);
			}
			else if(IsAreaCleared(hitInfo.shooterId, hitInfo.targetId))
			{
				Event(BC_AreaClear, hitInfo.shooterId);
			}
			else
			{
				if (m_clientPlayer->IsFriendlyEntity(hitInfo.shooterId) && IsSkillShot(hitInfo))
				{
	 				Event(BC_SkillShotKill, hitInfo.shooterId);	
				}
				else
				{
					Event(BC_EnemyKilled, hitInfo.shooterId);
				}
			}
		}

		
		EntityId actorId = 0;
		
		if(CActor * pTargetActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(hitInfo.targetId)))
		{
			actorId = GetNearestHearableFriendlyActor(*pTargetActor);

			StopAllBattleChatter(*pTargetActor);

			if(!pTargetActor->IsHeadShot(hitInfo))
				Event(BC_Death, hitInfo.targetId);
		}

		if(actorId)
		{
			Event(BC_Mandown, actorId);
		}
	}
}

//-------------------------------------------------------
void CBattlechatter::StopAllBattleChatter(CActor& rActor)
{
	//If some battle chatter is playing
	//m_signalPlayer.Stop(rActor.GetEntityId());
}

//-------------------------------------------------------
void CBattlechatter::OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId)
{
	// an entity has changed team. We must update their voice to ensure they're on the right team
	ActorVoiceMap::iterator it = m_actorVoice.find(entityId);
	if (it != m_actorVoice.end())
	{
		// just erased from the map. We'll generate a new voice next time someone requests it for this actor
		m_actorVoice.erase(it);
	}
}

//-------------------------------------------------------
bool CBattlechatter::IsAreaCleared(EntityId shooterId, EntityId targetId)
{
	CActorUpr * pActorUpr = CActorUpr::GetActorUpr();

	pActorUpr->PrepareForIteration();

	i32k kNumActors		= pActorUpr->GetNumActors();
	i32k kPlayerTeamId = g_pGame->GetGameRules()->GetTeam(shooterId);

	//position based off you as you'll be hearing the line
	const Vec3 clientPos = m_clientPlayer->GetEntity()->GetPos();
	const float minDistSq = sqr(bc_AreaClearedDistance);

	for(i32 i = 0; i < kNumActors; i++)
	{
		SActorData actorData;
		pActorUpr->GetNthActorData(i, actorData);

		if(actorData.health > 0 && actorData.teamId != kPlayerTeamId && actorData.entityId != targetId)
		{
			//no-one right around
			float distSq = actorData.position.GetSquaredDistance(clientPos);
			if(distSq < minDistSq)
			{
				return false;
			}
			//local player can't see anyone
			else if(g_pGame->GetPlayerVisTable()->CanLocalPlayerSee(actorData.entityId, 30))
			{
				return false;
			}
		}
	}

	return true;
}

//-------------------------------------------------------
EntityId CBattlechatter::GetNearestHearableFriendlyActor(CActor& rActor) const
{
	EntityId nearestActorId = 0, actorId = rActor.GetEntityId();

	CActorUpr * pActorUpr = CActorUpr::GetActorUpr();

	pActorUpr->PrepareForIteration();

	i32k nActorTeam = g_pGame->GetGameRules()->GetTeam(actorId);

	//If the actor team is zero then there won't be any friendlies
	if(nActorTeam != 0)
	{
		const Vec3& rActorPosn = rActor.GetEntity()->GetWorldPos();
		i32k hearableActorCount = pActorUpr->GetNumActors();
		float fMinDistanceSq = sqr(1024.f);

		for(i32 i = 0; i < hearableActorCount; i++)
		{
			SActorData actorData;
			pActorUpr->GetNthActorData(i, actorData);
			if( actorId != actorData.entityId && actorData.teamId == nActorTeam && actorData.spectatorMode == CActor::eASM_None && actorData.health > 0.0f )
			{					
				const float fDistanceSq = rActorPosn.GetSquaredDistance(actorData.position);
				if(fDistanceSq < fMinDistanceSq)
				{
					fMinDistanceSq = fDistanceSq;
					nearestActorId = actorData.entityId;
				}						
			}
		}
	}

	return nearestActorId;
}

//-------------------------------------------------------
bool CBattlechatter::CanPlayDitto(const float currentTime) const
{
	return m_lastTimeExpectedFinished < currentTime && (m_lastTimeExpectedFinished + bc_maxTimeCanDitto) > currentTime;
}

//-------------------------------------------------------
bool CBattlechatter::PlayedRecently(const SVoiceInfo* pInfo, const float currentTime) const
{
	return (pInfo->m_lastTimePlayed + bc_minTimeBetweenBattlechatter) > currentTime;
}

//-------------------------------------------------------
bool CBattlechatter::ShouldIgnoreRecentlyPlayed( const EBattlechatter chatter, const EBattlechatter requestedChatter ) const
{
	return (chatter == BC_Death || requestedChatter == BC_TakingFire || requestedChatter == BC_LowHealth);
}

//-------------------------------------------------------
bool CBattlechatter::ShouldClientHear(const EBattlechatter chatter) const
{
	//have health and not spectating (or player down)
	return (!m_clientPlayer->IsDead() && m_clientPlayer->GetSpectatorMode() == CActor::eASM_None) || (chatter == BC_PlayerDown);
}

//-------------------------------------------------------
bool CBattlechatter::ChanceToPlay(EBattlechatter chatter)
{
#if !defined(_RELEASE)
	if (bc_debugPlayAlways)
	{
		return true;
	}
#endif

	return drx_random(0.0f, 1.0f) < m_data[chatter].m_chance;
}

//-------------------------------------------------------
bool CBattlechatter::ShouldPlayForActor(EntityId actorId, float range) const
{
#if !defined(_RELEASE)
	if(bc_playForAllActors)
	{
		return true;
	}
#endif

	//only chatter for friendlies (and not self)
	if(ShouldHearActor(actorId) && DistanceToActor(actorId) < range)
	{
		return true;
	}

	return false;
}

//-------------------------------------------------------
bool CBattlechatter::ShouldHearActor(EntityId actorId) const
{
	IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(actorId);
	if(pActor != NULL && actorId != m_clientPlayer->GetEntityId())
	{
		return true;
	}
	return false;
}

//-------------------------------------------------------
float CBattlechatter::DistanceToActor(EntityId actorId) const
{
	IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(actorId);
	if(pActor)
	{
		Vec3 playerPos = m_clientPlayer->GetEntity()->GetWorldPos();
		return playerPos.GetDistance(pActor->GetEntity()->GetWorldPos());
	}

	return FLT_MAX;
}

//-------------------------------------------------------
void CBattlechatter::UpdateActorBattlechatter(i32 actorIndex, const CActorUpr * pActorUpr)
{
	SActorData actorData;
	pActorUpr->GetNthActorData(actorIndex, actorData);

	if(actorData.spectatorMode != CActor::eASM_None || actorData.health <= 0.0f)
	{
		m_currentActorIndex++;
	}
	else if(CPlayer * pPlayer = static_cast<CPlayer*>(gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(actorData.entityId)))
	{
		UpdateVisualEvents(pPlayer, actorData);	//not in update functions list so it will trigger more often and is relatively cheap

		(this->*s_updateFunctions[m_currentActorEventIndex])(pPlayer, actorData);

		m_currentActorEventIndex++;
		const static i32 updateSize = DRX_ARRAY_COUNT(s_updateFunctions);
		if(m_currentActorEventIndex == updateSize)
		{
			m_currentActorIndex++;
			m_currentActorEventIndex = 0;
		}
	}
	else
	{
		// a null actor sneaking in here used to leave battle chatter stuck never to update again
		// seen with dummy players, not sure why it wouldn't fire with real players
		m_currentActorEventIndex = 0;
		m_currentActorIndex = 0;
	}
}

// TODO - perhaps this could be removed entirely.. only one usage of it in code atm 
//-------------------------------------------------------
void CBattlechatter::UpdateVisualEvents(CPlayer* pPlayer, const SActorData& rPlayerData)
{
	//using whoever the actor is targetting
	if(EntityId targettingId = pPlayer->GetCurrentTargetEntityId())
	{
		IActor* pTargetActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(targettingId);
		if(pTargetActor != NULL)
		{
			if(rPlayerData.teamId == 0 || rPlayerData.teamId != g_pGame->GetGameRules()->GetTeam(targettingId))
			{
				Vec3 dir = pTargetActor->GetEntity()->GetWorldPos() - rPlayerData.position;
				if(dir.len2() > sqr(5.0f))
				{
					EBattlechatter visual = GetVisualChatter(targettingId);

					if (visual != BC_Null)
					{
						Event(visual, rPlayerData.entityId);
					}
				}
			}
		}
	}
}

//-------------------------------------------------------
EBattlechatter CBattlechatter::GetVisualChatter(EntityId entityId)
{
	i32k visualChatterSize = m_visualChatter.size();
	for(i32 i = 0; i < visualChatterSize; i++)
	{
		const TVisualPair chatter = m_visualChatter[i];
		if(entityId == chatter.first)
		{
			return chatter.second;
		}
	}

	return BC_Null;
}

//-------------------------------------------------------
void CBattlechatter::AddVisualChatter(EntityId entityId, EBattlechatter chatter)
{
	m_visualChatter.push_back(TVisualPair(entityId, chatter));
}

//-------------------------------------------------------
void CBattlechatter::RemoveVisualChatter(EntityId entityId)
{
	i32k visualChatterSize = m_visualChatter.size();
	for(i32 i = 0; i < visualChatterSize; i++)
	{
		if(entityId == m_visualChatter[i].first)
		{
			m_visualChatter.removeAt(i);
			return;
		}
	}
}

//-------------------------------------------------------
void CBattlechatter::UpdateMovingUpEvent(CPlayer* pPlayer, const SActorData& rPlayerData)
{
	IEntity* pClientEntity = m_clientPlayer->GetEntity();
	IEntity* pNearEntity = pPlayer->GetEntity();

	float dot = pClientEntity->GetForwardDir().Dot(pNearEntity->GetForwardDir());
	//if client and nearest player are facing in similar direction and near player is moving forwards
	if(dot > 0.9f && GetForwardAmount(pNearEntity) > 0.2f && GetForwardAmount(pClientEntity) < 0.2f)
	{
		Event(BC_MovingUp, rPlayerData.entityId);
	}
}

//-------------------------------------------------------
void CBattlechatter::UpdateSeeGrenade(CPlayer* pPlayer, const SActorData& rPlayerData)
{
	const static float k_nearGrenadeDist = 2.0f;
	Vec3 actorPos = rPlayerData.position;
	Vec3 boxMin(actorPos.x - k_nearGrenadeDist, actorPos.y - k_nearGrenadeDist, actorPos.z - k_nearGrenadeDist);
	Vec3 boxMax(actorPos.x + k_nearGrenadeDist, actorPos.y + k_nearGrenadeDist, actorPos.z + k_nearGrenadeDist);

	SProjectileQuery projQuery;
	projQuery.box = AABB(boxMin, boxMax);
	projQuery.ammoName = "explosivegrenade";
	if(g_pGame->GetWeaponSystem()->QueryProjectiles(projQuery) > 0)
	{
		Event(BC_SeeGrenade, rPlayerData.entityId);
	}
}

//-------------------------------------------------------
void CBattlechatter::UpdateQuiet(CPlayer* pPlayer, const SActorData& rPlayerData)
{
	float lastFiredTime = CPersistantStats::GetInstance()->GetLastFiredTime();
	if(lastFiredTime + bc_timeNeededForQuiet < gEnv->pTimer->GetCurrTime())	//haven't seen anyone in x seconds
	{
		CActorUpr * pActorUpr = CActorUpr::GetActorUpr();

		if(!pActorUpr->CanSeeAnyEnemyActor(rPlayerData.entityId))
		{
			Event(BC_Quiet, rPlayerData.entityId);
		}
	}
}

//-------------------------------------------------------
void CBattlechatter::UpdateNearEndOfGame(CPlayer* pPlayer, const SActorData& rPlayerData)
{
	CGameRules* pGameRules = g_pGame->GetGameRules();

	i32 ownTeam = pGameRules->GetTeam(m_clientPlayer->GetEntityId());
	i32 ownTeamScore = pGameRules->GetTeamsScore(ownTeam);
	i32 enemyTeamScore = pGameRules->GetTeamsScore(3 - ownTeam);

	i32 highestScore = max(ownTeamScore, enemyTeamScore);

	i32k scoreLimit = pGameRules->GetScoreLimit();

	//near score limit or time limit (if they exist)
	if((scoreLimit != 0 && highestScore + bc_NearEndOfGameScore > scoreLimit) ||
		pGameRules->IsTimeLimited() && pGameRules->GetRemainingGameTime() < bc_NearEndOfGameTime)
	{
		//Time Remaining good and bad are used for near end of game (typically want to finish game on score limit)
		EBattlechatter chatter = BC_TimeRemainingBad;

		if(ownTeamScore > enemyTeamScore)
		{
			chatter = BC_TimeRemainingGood;
		}

		Event(chatter, rPlayerData.entityId);
	}
}

//-------------------------------------------------------
float CBattlechatter::GetForwardAmount(IEntity* pEntity)
{
	if (IPhysicalEntity* pPhysicalEntity = pEntity->GetPhysics())
	{
		pe_status_living status;
		if(pPhysicalEntity->GetStatus(&status))
		{
			return status.vel.Dot(pEntity->GetForwardDir());
		}
	}

	return 0.0f;
}

#if !defined(_RELEASE)
//static-------------------------------------------------------
void CBattlechatter::CmdPlay(IConsoleCmdArgs* pCmdArgs)
{
	float minDistance = 10000.0f;
	CPlayer* clientPlayer = static_cast<CPlayer*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
	EntityId closestActorId = clientPlayer->GetEntityId();
	CActor *pClosestActor = clientPlayer;

	Vec3 clientPos = clientPlayer->GetEntity()->GetWorldPos();

	IActorIteratorPtr pIter = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
	while (CActor* pActor = (CActor*)pIter->Next())
	{
		if(pActor->GetEntityId() != clientPlayer->GetEntityId())
		{
			float distance = clientPos.GetDistance(pActor->GetEntity()->GetWorldPos());
			if(distance < minDistance)
			{
				minDistance = distance;
				closestActorId = pActor->GetEntityId();
				pClosestActor = pActor;
			}
		}
	}

	EBattlechatter chatter = BC_Reloading;
	if(pCmdArgs->GetArgCount() > 1)
	{
		tukk chatterName = pCmdArgs->GetArg(1);
		for(i32 i = 0; i < BC_Last; i++)
		{
			if(strcmpi(s_battlechatterName[i], chatterName) == 0)
			{
				chatter = (EBattlechatter) i;
			}
		}
	}

	/*SVoiceInfo* pInfo = g_pGame->GetGameRules()->GetBattlechatter()->GetVoiceInfo(closestActorId);
	if(pInfo)
	{
		DrxLogAlways("bc_play - Entity %s at %.0fm away with signal %s voiceIndex=%d", pClosestActor->GetEntity()->GetName(), minDistance, s_battlechatterName[chatter], pInfo->m_voiceIndex);	
		const float currentTime = gEnv->pTimer->GetCurrTime();
		g_pGame->GetGameRules()->GetBattlechatter()->Play(pInfo, chatter, pClosestActor, closestActorId, currentTime);
	}
	else
	{
		DrxLogAlways("bc_play - Unable to find voice info - No voices loaded");
	}*/
}

//-------------------------------------------------------
/*static*/ void CBattlechatter::CmdClearRecentlyPlayed(IConsoleCmdArgs *pCmdArgs)
{
	g_pGame->GetGameRules()->GetBattlechatter()->ClearRecentlyPlayed();
}

//-------------------------------------------------------
/*static*/ void CBattlechatter::CmdDump(IConsoleCmdArgs *pCmdArgs)
{
	DrxLog("----------------------------------------------------------------------");
	DrxLog("Dump all");
	DrxLog("----------------------------------------------------------------------");
	i32 count=0;

	CBattlechatter *pBC = g_pGame->GetGameRules()->GetBattlechatter();
	for (i32 i=BC_Null+1; i<BC_Last; i++)
	{
		DrxLog("BattleChatter: Name=%s playCount=%d", s_battlechatterName[i], pBC->m_data[i].m_playCount);
		count++;
	}

	DrxLog("----------------------------------------------------------------------");
	DrxLog("Total: %d", count);
	DrxLog("----------------------------------------------------------------------");
}

//-------------------------------------------------------
/*static*/ void CBattlechatter::CmdDumpPlayed(IConsoleCmdArgs *pCmdArgs)
{
	CBattlechatter *pBC = g_pGame->GetGameRules()->GetBattlechatter();
	DrxLog("----------------------------------------------------------------------");
	DrxLog("Dump Played");
	DrxLog("----------------------------------------------------------------------");
	i32 count=0;

	for (i32 i=BC_Null+1; i<BC_Last; i++)
	{
		if (pBC->m_data[i].m_playCount > 0)
		{
			DrxLog("BattleChatter: Name=%s playCount=%d", s_battlechatterName[i], pBC->m_data[i].m_playCount);
			count++;
		}
	}
	DrxLog("----------------------------------------------------------------------");
	DrxLog("Total Played: %d", count);
	DrxLog("----------------------------------------------------------------------");
}

//-------------------------------------------------------
/*static*/ void CBattlechatter::CmdDumpUnPlayed(IConsoleCmdArgs *pCmdArgs)
{
	DrxLog("----------------------------------------------------------------------");
	DrxLog("Dump UnPlayed");
	DrxLog("----------------------------------------------------------------------");
	i32 count=0;

	CBattlechatter *pBC = g_pGame->GetGameRules()->GetBattlechatter();
	for (i32 i=BC_Null+1; i<BC_Last; i++)
	{
		if (pBC->m_data[i].m_playCount == 0)
		{
			DrxLog("BattleChatter: Name=%s playCount=%d", s_battlechatterName[i], pBC->m_data[i].m_playCount);
			count++;
		}
	}

	DrxLog("----------------------------------------------------------------------");
	DrxLog("Total UnPlayed: %d", count);
	DrxLog("----------------------------------------------------------------------");
}

//-------------------------------------------------------
/*static*/ void CBattlechatter::CmdClearPlayCount(IConsoleCmdArgs *pCmdArgs)
{
	i32 count=0;

	CBattlechatter *pBC = g_pGame->GetGameRules()->GetBattlechatter();
	for (i32 i=BC_Null+1; i<BC_Last; i++)
	{
		if (pBC->m_data[i].m_playCount > 0)
		{
			count++;
		}
		pBC->m_data[i].m_playCount = 0;
	}

	DrxLog("----------------------------------------------------------------------");
	DrxLog("Cleared PlayCount on %d chatters", count);
	DrxLog("----------------------------------------------------------------------");
}

#endif

//-------------------------------------------------------
void CBattlechatter::ClientNetEvent(EBattlechatter chatter)
{
	if(m_clientPlayer && CheckClientNetEvent(chatter))
	{
		m_clientNetChatter = chatter;
		CHANGED_NETWORK_STATE(m_clientPlayer, CPlayer::ASPECT_BATTLECHATTER_CLIENT);
	}
}

//-------------------------------------------------------
bool CBattlechatter::CheckClientNetEvent(EBattlechatter chatter)
{
	if(chatter == BC_NearbyEnemyDetected)
	{
		//check you can't see anyone but detected them
		CActorUpr * pActorUpr = CActorUpr::GetActorUpr();

		pActorUpr->PrepareForIteration();

		i32k kNumActors		= pActorUpr->GetNumActors();
		i32k kPlayerTeamId = g_pGame->GetGameRules()->GetTeam(m_clientPlayer->GetEntityId());
		for(i32 i = 0; i < kNumActors; i++)
		{
			SActorData actorData;
			pActorUpr->GetNthActorData(i, actorData);

			if(kPlayerTeamId == 0 || kPlayerTeamId != actorData.teamId)
			{
				if(g_pGame->GetPlayerVisTable()->CanLocalPlayerSee(actorData.entityId, 30))
				{
					return false;
				}
			}
		}
	}

	// are we ok to send this chatter event if it imposes a minTimeBetween sends
	float minTimeBetweenSends = m_data[chatter].m_netMinTimeBetweenSends;
	if (minTimeBetweenSends > 0.f)
	{
		float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		float lastTime = m_data[chatter].m_netLastTimePlayed;
		float diffTime = currentTime - lastTime;
		
		if (diffTime > minTimeBetweenSends)
		{
			m_data[chatter].m_netLastTimePlayed = currentTime;
			return true;
		}
	}

	return true;
}

//-------------------------------------------------------
void CBattlechatter::NetSerialize(CPlayer* pPlayer, TSerialize ser, EEntityAspects aspect)
{
	i32 chatter = (i32) m_clientNetChatter;
	
	DRX_ASSERT(chatter < 128);
	ser.Value("clientBattlechatter", chatter, 'i8');
	ser.Value("serialiseCounter", m_serialiseCounter, 'ui2');

	if(ser.IsReading() && chatter != BC_Null)	//read in something someone should say
	{
		Event((EBattlechatter) chatter, pPlayer->GetEntityId());
	}

	if(ser.IsWriting())	//said what we want, clear it (encase something else forces us to serialize)
	{
		m_clientNetChatter = BC_Null;
		m_serialiseCounter = (m_serialiseCounter + 1) % 4;
	}
}

//-------------------------------------------------------
bool CBattlechatter::IsLastPlayed(EBattlechatter chatter)
{
	return m_recentlyPlayed[m_recentlyPlayedIndex] == chatter;
}

//-------------------------------------------------------
bool CBattlechatter::IsRecentlyPlayed(EBattlechatter chatter)
{
	if(bc_allowRepeats)
		return false;

	if (chatter == BC_Death)
		return false;

	for(i32 i = 0; i < k_recentlyPlayedListSize; i++)
	{
		if(chatter == m_recentlyPlayed[i])
		{
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------
void CBattlechatter::AddPlayed(EBattlechatter chatter)
{
	++m_recentlyPlayedIndex;
	m_recentlyPlayedIndex = m_recentlyPlayedIndex % k_recentlyPlayedListSize;
	m_recentlyPlayed[m_recentlyPlayedIndex] = chatter;
}

//-------------------------------------------------------
u32k CBattlechatter::GetVoiceIndex(bool friendly)
{
	u32 voiceIndex = 0;
	CGameRules *pGameRules = g_pGame->GetGameRules();
	EGameMode gameMode = pGameRules->GetGameMode();

	if (gameMode == eGM_Gladiator)
	{
		if(friendly)
		{
			voiceIndex = bc_FriendlyHunterActorVoiceIndex + drx_random(0, bc_FriendlyHunterActorVoiceRange - 1);
		}
		else
		{
			voiceIndex = bc_EnemyHunterActorVoiceIndex + drx_random(0, bc_EnemyHunterActorVoiceRange - 1);
		}
	}
	else
	{
		if(friendly)
		{
			voiceIndex = bc_FriendlyActorVoiceIndex + drx_random(0, bc_FriendlyActorVoiceRange - 1);
		}
		else
		{
			voiceIndex = bc_EnemyActorVoiceIndex + drx_random(0, bc_EnemyActorVoiceRange - 1);
		}
	}


	u32 voiceArrayUpperLimit = m_voice.size()-1;
	if(voiceIndex > voiceArrayUpperLimit)
	{
		voiceIndex = voiceArrayUpperLimit;
		GameWarning("Trying to play voice index %d, but there are only %" PRISIZE_T, voiceIndex, m_voice.size());
	}

	return voiceIndex;
}

//-------------------------------------------------------
void CBattlechatter::SomethingHasHappened(
	i32k inByTeamId,
	const Vec3 *inAtPos,
	const EBattlechatter inRequestedChatter, 
	const float inMaxRangeSqToReact, 
	u32k inFlags)
{
	DRX_ASSERT(inAtPos);

	const CActorUpr * pActorUpr = CActorUpr::GetActorUpr();
	pActorUpr->PrepareForIteration();

	CGameRules *pGameRules = g_pGame->GetGameRules();
	if (!pGameRules->IsTeamGame())
	{
		return;
	}

	for (i32 i=0, nNumActors = pActorUpr->GetNumActors(); i < nNumActors; i++)
	{
		SActorData actorData;
		pActorUpr->GetNthActorData(i, actorData);

		bool validPlayer=false;

		if(inByTeamId == actorData.teamId)
		{
			if (inFlags & k_SHH_flags_consider_teammates)
				validPlayer = true;
		}
		else if (inFlags & k_SHH_flags_consider_enemies)
		{
			validPlayer=true;
		}

		if (validPlayer)
		{
			Vec3 dirToViewed = actorData.position - *inAtPos;
			float rangeSq;

			if (inFlags & k_SHH_flags_2D_range_test)
			{
				rangeSq = dirToViewed.GetLengthSquared2D();
			}
			else
			{
				rangeSq = dirToViewed.GetLengthSquared();
			}

			if (rangeSq < inMaxRangeSqToReact)
			{
				Event(inRequestedChatter, actorData.entityId);
				break;
			}
		}
	}
}

//-------------------------------------------------------
void CBattlechatter::Debug()
{
#if !defined(_RELEASE)
	CActorUpr * pActorUpr = CActorUpr::GetActorUpr();
	if(bc_debug > 0)
	{
		if(bc_debug == 1)
		{
			if(m_voice.size() > 0)
			{
				DrxWatch("Battlechatter Actor Voice Test Assignment");
				IActorIteratorPtr pIter = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
				while (CActor* pActor = (CActor*)pIter->Next())
				{
					SVoiceInfo *pInfo = GetVoiceInfo(pActor->GetEntityId());
					DrxWatch("EntityId %d - Voice %d Last spoke %.2f", pActor->GetEntityId(), pInfo->m_voiceIndex, pInfo->m_lastTimePlayed);
				}
			}
			else
			{
				DrxWatch("No voices found!");
			}
		}
		else if(bc_debug == 2)
		{
			DrxWatch("Battlechatter");
			DrxWatch("ShouldPlay %d", ShouldPlay());
			DrxWatch("Client %p", m_clientPlayer);
			i32k hearableActorCount = pActorUpr->GetNumActors();
			for(i32 i = 0; i < hearableActorCount; i++)
			{
				SActorData actorData;
				pActorUpr->GetNthActorData(i, actorData);
				DrxWatch("Player %d, hearable: %s", actorData.entityId, actorData.health > 0.0f && actorData.spectatorMode == CActor::eASM_None ? "TRUE" : "FALSE");
			}
			DrxWatch("hearable actor %d (current event %d)", m_currentActorIndex, m_currentActorEventIndex);
			DrxWatch("recentlyPlayedIndex %d", m_recentlyPlayedIndex);
			for(i32 i = 0; i < k_recentlyPlayedListSize; i++)
			{
				if(m_recentlyPlayed[i] != BC_Null)
				{
					DrxWatch("	Recently played %s", s_battlechatterName[m_recentlyPlayed[i]]);
				}
			}
			if(m_clientPlayer)
				DrxWatch("ShouldClientHear %d", ShouldClientHear(BC_Null));

			IActorIteratorPtr actorIt = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
			IActor *actor;

			while (actor=actorIt->Next())
			{
				DrxWatch("	%s - ShouldPlayForActor %d (%.2f) within 30m", actor->GetEntity()->GetName(), ShouldPlayForActor(actor->GetEntityId(), 30.0f), DistanceToActor(actor->GetEntityId()));
			}

			DrxWatch("Chatter Net Details:");
			for(i32 i = 0; i < BC_Last; i++)
			{
				if (m_data[i].m_netLastTimePlayed > 0.f)
				{
					DrxWatch("chatter %s (%d) lastTimePlayed=%f",  s_battlechatterName[i], i, m_data[i].m_netLastTimePlayed);
				}
			}
		}
		else if(bc_debug == 3)
		{
			for(i32 i = 0; i < BC_Last; i++)
			{
				DrxWatch("\t%d - %s is %.2f (chance %.4f) minTimeBetweenSends=%.2f; m_netLastTimePlayed=%.2f", i, s_battlechatterName[i], m_data[i].m_range, m_data[i].m_chance, m_data[i].m_netMinTimeBetweenSends, m_data[i].m_netLastTimePlayed);
				if(i == bc_signalsDetail)
				{
					gEnv->pRenderer->GetIRenderAuxGeom()->SetRenderFlags( e_Mode3D | e_AlphaBlended | e_DrawInFrontOff | e_FillModeSolid | e_CullModeNone );
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawCylinder(m_clientPlayer->GetEntity()->GetWorldPos(), Vec3(0, 0, 1), m_data[i].m_range/2.0f, 1.0f, ColorB(0, 100, 255, 55));
				}
			}
		}
		else if(bc_debug == 4)
		{
			DrxWatch("Repeat Chatter");
			for(i32 i = 0; i < BC_Last; i++)
			{
				DrxWatch("\t%d - %s repeated says %s", i, s_battlechatterName[i], s_battlechatterName[m_data[i].m_repeatChatter]);
			}
		}
		else if(bc_debug == 5)
		{
			DrxWatch("Cloaked Chatter");
			for(i32 i = 0; i < BC_Last; i++)
			{
				DrxWatch("\t%d - %s --- %s", i, s_battlechatterName[i], s_battlechatterName[m_data[i].m_cloakedChatter]);
			}
		}
		else if(bc_debug == 6)
		{
			DrxWatch("Armour Chatter");
			for(i32 i = 0; i < BC_Last; i++)
			{
				DrxWatch("\t%d - %s --- %s", i, s_battlechatterName[i], s_battlechatterName[m_data[i].m_armourChatter]);
			}
		}
		else if(bc_debug == 7)
		{
			i32k visualChatterSize = m_visualChatter.size();
			DrxWatch("Alternative Visual Chatter (%d)", visualChatterSize);	
			for(i32 i = 0; i < visualChatterSize; i++)
			{
				const TVisualPair chatter = m_visualChatter[i];
				IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(chatter.first);
				if(pActor)
				{
					DrxWatch("[%d]%s - %s", i, pActor->GetEntity()->GetName(), s_battlechatterName[chatter.second]);
				}
			}
		}
	}

	if(bc_signals > 0)
	{
		i32 voiceIndex = bc_signals - 1;
		i32k voiceCount = m_voice.size();
		DrxWatch("Voice %d (of %d)", voiceIndex + 1, voiceCount);
		if(voiceIndex >= 0 && voiceIndex < (i32)m_voice.size())
		{
			for(i32 chatterIndex = 0; chatterIndex < BC_Last; chatterIndex++)
			{
				SVoice::SVoiceSignals &voiceSignals = m_voice[voiceIndex].m_chatter[chatterIndex];

				string signalNameStandard;
				signalNameStandard.Format("%s_TM%d", s_battlechatterName[chatterIndex], voiceIndex + 1);
				string signalNameMarine;
				signalNameMarine.Format("%s_TM%d_Marine", s_battlechatterName[chatterIndex], voiceIndex + 1);
				string signalNameHunter;
				signalNameHunter.Format("%s_TM%d_Hunter", s_battlechatterName[chatterIndex], voiceIndex + 1);
				string signalNameHunterMarine;
				signalNameHunterMarine.Format("%s_TM%d_HunterMarine", s_battlechatterName[chatterIndex], voiceIndex + 1);

				DrxWatch("\t%d - %s - [%s,%s,%s,%s] - [%d,%d,%d,%d]", chatterIndex, s_battlechatterName[chatterIndex], signalNameStandard.c_str(), signalNameMarine.c_str(), signalNameHunter.c_str(), signalNameHunterMarine.c_str(), voiceSignals.m_standard, voiceSignals.m_marine, voiceSignals.m_hunter, voiceSignals.m_hunterMarine);
			}
		}
	}

	if(bc_chatter && !m_signalPlayer.IsPlaying())
	{
		i32k hearableActorCount = pActorUpr->GetNumActors();
		for(i32 i = 0; i < hearableActorCount; i++)
		{
			SActorData actorData;
			pActorUpr->GetNthActorData(i, actorData);

			if(actorData.spectatorMode == CActor::eASM_None && actorData.health > 0.0f)
			{
				if(drx_random(0.0f, 1.0f) < bc_chatterFreq)
				{
					
					EntityId actorId = actorData.entityId;
					CActor* pActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(actorId));
					SVoiceInfo* pInfo = GetVoiceInfo(actorId);
					if(pInfo && pActor != NULL)
					{
						EBattlechatter chatter = (EBattlechatter) drx_random(0, BC_Last - 1);
						DrxLogAlways("bc_chatter - Entity %s saying %s", pActor->GetEntity()->GetName(), s_battlechatterName[chatter]);

						Event(chatter, actorId);
					}
				}
			}
		}
	}
#endif
}

// play nice with uberfiles compiling
#undef DbgLog
#undef DbgLogAlways 