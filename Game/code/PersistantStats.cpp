// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 04:02:2009		Created by Ben Parbury
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/PersistantStats.h>

#include <IPlayerProfiles.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <time.h>

#include <drx3D/Sys/Scaleform/IFlashPlayer.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStateModule.h>
#include <drx3D/Game/GameRulesModules/GameRulesModulesUpr.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesPlayerStatsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesScoringModule.h>
#include <drx3D/Game/GameRulesModules/GameRulesObjective_Predator.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/Utility/DrxHash.h>
#include <drx3D/Game/Utility/StringUtils.h>
#include <ILevelSystem.h>
#include <drx3D/CoreX/DrxEndian.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/ITelemetryCollector.h>
#include <drx3D/Game/Network/Squad/SquadUpr.h>
#include <drx3D/Game/Network/Lobby/GameAchievements.h>
#include <drx3D/Game/Network/Lobby/GameLobbyData.h>
#include <drx3D/Game/Utility/DrxDebugLog.h>
#include <drx3D/Game/ActorUpr.h>
#include <drx3D/Game/Battlechatter.h>
#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/Projectile.h>
#include <drx3D/Game/WeaponSystem.h>
#include <drx3D/Game/PlayerVisTable.h>
#include <drx3D/CoreX/Lobby/IDrxLobby.h>
#include <drx3D/Game/StatsRecordingMgr.h>
#include <drx3D/Game/Melee.h>
#include <drx3D/Game/SkillKill.h>
#include <drx3D/Game/PlaylistUpr.h>
#include <drx3D/Game/UI/HUD/HUDMissionObjectiveSystem.h>
#include <drx3D/Game/DLCUpr.h>
#include <drx3D/Game/PatchPakUpr.h>
#include <drx3D/Game/ItemSharedParams.h>
#include <drx3D/Game/EnvironmentalWeapon.h>
#include <drx3D/Game/VTOLVehicleUpr/VTOLVehicleUpr.h>

#if !defined(_RELEASE)
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/UICVars.h>
#endif


#define BINARY_ENDIAN_SWAP_PLATFORM 1
#define ENEMY_PLAYER_COUNT 64		// Needs to be high to allow for join/leave players persisting (for times killed etc.)
#define PERSISTANT_STATS_MAP_PARAM_DEFINITION_FILE "Scripts/Progression/PersistantStatMapParams.xml"

#define ADD_STATS_STRINGS(stat) \
	GetStatStrings(stat, &statsStrings); \
	fixedArrayStats.push_back(statsStrings);

#define ADD_STATS_STRINGS_USE_STRING(stat, str) \
	GetStatStrings(stat, &statsStrings); \
	statsStrings.m_title = str; \
	fixedArrayStats.push_back(statsStrings);

#define ADD_MAP_STATS_STRINGS(stat, name, paramString) \
	GetStatStrings(name, stat, paramString, &statsStrings); \
	fixedArrayStats.push_back(statsStrings);

#define ADD_HEADING_STRING(name) \
	statsStrings.m_title = "_HEADING"; \
	statsStrings.m_value = name; \
	fixedArrayStats.push_back(statsStrings);


#define BLAZE_REPORT_WEAPON(...)
#define BLAZE_REPORT_PLAYER_LOCAL_OFFLINE(...)

CPersistantStats* CPersistantStats::s_persistantStats_instance = NULL;
bool CPersistantStats::s_multiplayer = false;

#define LONG_TERM_AWARD_DAYS (182)
#define GET_PERSISTANT_FLAGS(a,b,c) b,

#define TEST_GENERAL_STATS 0
//#define TEST_GENERAL_STATS 1
#if defined(_RELEASE)
#undef TEST_GENERAL_STATS
#define TEST_GENERAL_STATS 0
#endif // defined(_RELEASE)

#define IDLE_FOR_TELEM_TIME 60

CPersistantStats::SMapParam CPersistantStats::s_mapParams[] = 
{
	CPersistantStats::SMapParam("Weapon", eSF_MapParamWeapon),
	CPersistantStats::SMapParam("HitType", eSF_MapParamHitType),
	CPersistantStats::SMapParam("GameRules", eSF_MapParamGameRules),
	CPersistantStats::SMapParam("Level", eSF_MapParamLevel),
	CPersistantStats::SMapParam("MPWeapons", eSF_MapParamWeapon),
};

const float CPersistantStats::k_cloakedVictimTimeFromCloak = 2.0f;
const float CPersistantStats::k_fastGrenadeKillTimeout = 1.0f;
const float CPersistantStats::SEnemyTeamMemberInfo::k_timeInKillStateTillReset = 10.0f;
const float CPersistantStats::k_actorStats_inAirMinimum = 0.25f;
const float CPersistantStats::kTimeAllowedToKillEntireEnemyTeam = 15.f;
const float CPersistantStats::k_longDistanceThrowKillMinDistanceSquared = sqr(20.f);

//s_levelNamesVersion needs to change if you change the level names
i32 CPersistantStats::s_levelNamesVersion = 2;
tukk CPersistantStats::sz_levelNames[] =
{
	//release
	"c3mp_bridge",
	"c3mp_dam",
	"c3mp_museum",
	"c3mp_rooftop_gardens",
	"c3mp_airport",
	"c3mp_canyon",
	"c3mp_cave",
	"c3mp_con_ed",
	"c3mp_fields",
	"c3mp_river",
	"c3mp_swamp_boat",
	"c3mp_tanker"
};

tukk CPersistantStats::sz_weaponNames[] = 
{
	//release
	"AY69",
	"C4",
	"DSG1",
	"Feline",
	"FlashBangGrenades",
	"FragGrenades",
	"Gauss",
	"Grendel",
	"HMG",
	"Hammer",
	"JAW",
	"Jackal",
	"K-Volt",
	"LTag",
	"Marshall",
	"Mk60",
	"Nova",
	"Revolver",
	"SCAR",
	"SCARAB",
	"mike",

	//DLC two
	"Fy71",
	"SmokeGrenades"
};

namespace
{
	const static i32 k_descBufferSize = 65535;
	const static i32 k_maxLeaderboardColumns = 5;

	static AUTOENUM_BUILDNAMEARRAY(s_intPersistantNames, IntPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_floatPersistantNames, FloatPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_streakIntPersistantNames, StreakIntPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_streakFloatPersistantNames, StreakFloatPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_mapPersistantNames, MapPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_intDerivedPersistantNames, DerivedIntPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_floatDerivedPersistantNames, DerivedFloatPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_stringDerivedPersistantNames, DerivedStringPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_intMapDerivedPersistantNames, DerivedIntMapPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_floatMapDerivedPersistantNames, DerivedFloatMapPersistantStats);
	static AUTOENUM_BUILDNAMEARRAY(s_stringMapDerivedPersistantNames, DerivedStringMapPersistantStats);

	const static i32 k_ProfileVersionNumber = 3;	//WARNING - If you change this after release you'll have to adjust the LoadFromProfile accordingly!

	const static u32 s_intStatsFlags[EIPS_Max] = 
	{
		IntPersistantStats(GET_PERSISTANT_FLAGS)
	};

	const static u32 s_floatStatsFlags[EFPS_Max] = 
	{
		FloatPersistantStats(GET_PERSISTANT_FLAGS)
	};

	const static u32 s_streakIntStatsFlags[ESIPS_Max] = 
	{
		StreakIntPersistantStats(GET_PERSISTANT_FLAGS)
	};

	const static u32 s_streakFloatStatsFlags[ESFPS_Max] = 
	{
		StreakFloatPersistantStats(GET_PERSISTANT_FLAGS)
	};

	const static u32 s_mapStatsFlags[EMPS_Max] = 
	{
		MapPersistantStats(GET_PERSISTANT_FLAGS)
	};

	#undef GET_PERSISTANT_FLAGS

	#define MISC_STATS_SAVE_TIME 600

	#ifndef _RELEASE
	struct SPersistantStatsAutoComplete : public IConsoleArgumentAutoComplete
	{
		virtual i32 GetCount() const { return EIPS_Max + EFPS_Max + ESIPS_Max + ESFPS_Max + EMPS_Max; };
		virtual tukk GetValue( i32 i ) const
		{
			if(i < EIPS_Max)
			{
				return s_intPersistantNames[i];
			}
			i -= EIPS_Max;

			if(i < EFPS_Max)
			{
				return s_floatPersistantNames[i];
			}
			i -= EFPS_Max;

			if(i < ESIPS_Max)
			{
				return s_streakIntPersistantNames[i];
			}
			i -= ESIPS_Max;

			if(i < ESFPS_Max)
			{
				return s_streakFloatPersistantNames[i];
			}
			i -= ESFPS_Max;

			if(i < EMPS_Max)
			{
				return s_mapPersistantNames[i];
			}

			DRX_ASSERT(false);
			return NULL;
		};
	};

	static SPersistantStatsAutoComplete s_persistantStatsAutoComplete;
	#endif

	bool IsMeleeAttack( const HitInfo& hitInfo )
	{
		if( gEnv->bMultiplayer )
		{
			return hitInfo.type == CGameRules::EHitType::Melee;
		}

		const HitTypeInfo* pHitTypeInfo = g_pGame->GetGameRules()->GetHitTypeInfo( hitInfo.type );
		if( pHitTypeInfo && ((pHitTypeInfo->m_flags & CGameRules::EHitTypeFlag::IsMeleeAttack) == CGameRules::EHitTypeFlag::IsMeleeAttack) )
		{
			return true;
		}

		return false;
	}
}

i32 g_zeus = 0;
void zeusEngage()
{
	g_zeus = 1;
}

//static---------------------------------
CPersistantStats* CPersistantStats::GetInstance()
{
	DRX_ASSERT(s_persistantStats_instance);
	return s_persistantStats_instance;
}

//---------------------------------------
CPersistantStats::CPersistantStats()
	: m_fSecondTimer(0.0f)
	, m_lastTimeMPTimeUpdated(0.f)
	, m_localPlayerInVTOL(false)
{
	DRX_ASSERT(s_persistantStats_instance == NULL);
	s_persistantStats_instance = this;


	Reset();
	m_nearGrenadeMap.reserve(ENEMY_PLAYER_COUNT);
	m_previousWeaponHitMap.reserve(ENEMY_PLAYER_COUNT);
	m_enemyTeamMemberInfoMap.reserve(ENEMY_PLAYER_COUNT);

#ifndef _RELEASE
	if (gEnv->pConsole)
	{
		REGISTER_COMMAND("ps_DumpTelemetryDescription", CmdDumpTelemetryDescription, VF_CHEAT, "Dumps an xml description of stats in telemetry");
		REGISTER_COMMAND("ps_set", CmdSetStat, VF_CHEAT, "Sets persistant stats");
		REGISTER_COMMAND("ps_testSetandSend", CmdTestStats, VF_CHEAT, "Set stats to test values and send so we can check they come back correctly");
		gEnv->pConsole->RegisterAutoComplete("ps_set", &s_persistantStatsAutoComplete);
		
	}
#endif
	
	SetInGame(false);

	Init();
	m_afterMatchAwards.Init();
}

//---------------------------------------
CPersistantStats::~CPersistantStats()
{
	if(IPlayerProfileUpr *pProfileMan = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileUpr())
	{
		pProfileMan->RemoveListener(this);
	}

	ClearListeners();

	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	DRX_ASSERT(s_persistantStats_instance == this);
	s_persistantStats_instance = NULL;

	if (gEnv->pInput)
	{
		gEnv->pInput->RemoveEventListener(this);
	}

}

//---------------------------------------
void CPersistantStats::Init()
{
	InitStreakParams();
	
	InitMapParams();

	CPlayerProgression::GetInstance()->AddEventListener(this);

	if(IPlayerProfileUpr *pProfileMan = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileUpr())
	{
		pProfileMan->AddListener(this, true);
	}

	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this);

	// we can't just reserve here sadly (deque) so resize will have to do. 
	m_clientPersistantStatHistory.resize(NUM_HISTORY_ENTRIES); 
	m_clientPreviousKillData.resize(k_previousKillsToTrack);

	if (gEnv->pInput)
	{
		gEnv->pInput->AddEventListener(this);
	}
}

//---------------------------------------
void CPersistantStats::RegisterLevelTimeListeners()
{
	g_pGame->GetIGameFramework()->GetIItemSystem()->RegisterListener(this);
}

//---------------------------------------
void CPersistantStats::UnRegisterLevelTimeListeners()
{
	RemoveAllWeaponListeners();	// these guys all look like they're reregistered at every game start
	g_pGame->GetIGameFramework()->GetIItemSystem()->UnregisterListener(this);
}

//---------------------------------------
void CPersistantStats::InitMapParams()
{
	ClearMapParams();

	XmlNodeRef xmlData = GetISystem()->LoadXmlFromFile(PERSISTANT_STATS_MAP_PARAM_DEFINITION_FILE);
	if(xmlData)
	{
		i32k typeCount = xmlData->getChildCount();
		for(i32 i = 0; i < typeCount; i++)
		{
			XmlNodeRef typeNode = xmlData->getChild(i);
			tukk typeName = typeNode->getAttr("name");
	
			TMapParams *pMapParams = GetMapParams(typeName);
			if(pMapParams)
			{
				i32k attrCount = typeNode->getChildCount();

				pMapParams->reserve(attrCount);

				for(i32 j = 0; j < attrCount; j++)
				{
					XmlNodeRef attrNode = typeNode->getChild(j);
					tukk attrName = attrNode->getAttr("name");
					pMapParams->push_back(attrName);
				}
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Unable to find MapParam '%s' - not loading", typeName);
			}
		}
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Unable to load '%s'", PERSISTANT_STATS_MAP_PARAM_DEFINITION_FILE);
	}
}

//---------------------------------------
void CPersistantStats::InitStreakParams()
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	COMPILE_TIME_ASSERT(ESIPS_Max == DRX_ARRAY_COUNT(pSessionStats->m_streakIntStats));
	for(i32 i = 0; i < ESIPS_Max; i++)
	{
		if(s_streakIntStatsFlags[i] & eSF_StreakMultiSession)
		{
			pSessionStats->m_streakIntStats[i].m_multiSession = true;
		}
	}
	COMPILE_TIME_ASSERT(ESFPS_Max == DRX_ARRAY_COUNT(pSessionStats->m_streakFloatStats));
	for(i32 i = 0; i < ESFPS_Max; i++)
	{
		if(s_streakFloatStatsFlags[i] & eSF_StreakMultiSession)
		{
			pSessionStats->m_streakFloatStats[i].m_multiSession = true;
		}
	}
}

//---------------------------------------
tukk CPersistantStats::GetWeaponMapParamName() const
{
	if( s_multiplayer )
	{
		return "MPWeapons";
	}
	else
	{
		return "Weapon";
	}
}

//---------------------------------------
CPersistantStats::TMapParams* CPersistantStats::GetMapParams(tukk name)
{
	i32k mapParamSize = DRX_ARRAY_COUNT(s_mapParams);
	for(i32 i = 0; i < mapParamSize; i++)
	{
		if(strcmpi(s_mapParams[i].m_name, name) == 0)
		{
			return &s_mapParams[i].m_mapParam;
		}
	}

	return NULL;
}

//---------------------------------------
const CPersistantStats::TMapParams* CPersistantStats::GetMapParamsExt(tukk name) const
{
	i32k mapParamSize = DRX_ARRAY_COUNT(s_mapParams);
	for(i32 i = 0; i < mapParamSize; i++)
	{
		if(strcmpi(s_mapParams[i].m_name, name) == 0)
		{
			return &s_mapParams[i].m_mapParam;
		}
	}

	return NULL;
}

//---------------------------------------
void CPersistantStats::ClearMapParams()
{
	i32k mapParamSize = DRX_ARRAY_COUNT(s_mapParams);
	for(i32 i = 0; i < mapParamSize; i++)
	{
		s_mapParams->m_mapParam.clear();
	}
}

//---------------------------------------
void CPersistantStats::Reset()
{
	m_clientPersistantStatsAtGameStart.Clear();
	m_clientPersistantStats.Clear();
	m_clientPreviousKillData.clear();
	m_sessionStats.clear();
	m_actorWeaponListener.clear();

	m_retaliationTargetId = 0;
	m_pickAndThrowWeaponClassId = ~u16(0);
	
	m_lastFiredTime = 0.0f;
	m_weaponDamaged = false;
	m_crouching = false;

	m_gamemodeTimeValid = false;
	m_gamemodeStartTime = 0.0f;
	m_lastModeChangedTime = 0.0f;
	memset( &m_crouchToggleTime, 0, sizeof(m_crouchToggleTime) );

	m_iCalculatedTotalWeaponsUsedTime = 0;
	
	m_DateFirstPlayed_Year = 0;
	m_DateFirstPlayed_Month = 0;
	m_DateFirstPlayed_Day = 0;
	m_bHasCachedStartingStats = false;
	m_bSentStats = false;

	m_idleTime = 0;
}

//---------------------------------------
void CPersistantStats::SetMultiplayer(const bool multiplayer)
{
	DRX_ASSERT(multiplayer == gEnv->bMultiplayer);

	IPlayerProfile *pProfile = NULL;

	IPlayerProfileUpr* pProfileUpr = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr();
	if(pProfileUpr)
	{
		tukk user = pProfileUpr->GetCurrentUser();
		pProfile = pProfileUpr->GetCurrentProfile(user);
	}

	//Save for current game type
	if(pProfile)
	{
		SaveToProfile(pProfile, false, ePR_All);
	}

	s_multiplayer = multiplayer;

	//Load for new game type
	if(pProfile)
	{
		LoadFromProfile(pProfile, false, ePR_All);
	}
}

//static---------------------------------------
tukk CPersistantStats::GetAttributePrefix()
{
	if(s_multiplayer)
	{
		return "MP/PersistantStats";
	}
	else
	{
		return "SP/PersistantStats";
	}
}

//---------------------------------------
void CPersistantStats::SaveToProfile(IPlayerProfile* pProfile, bool online, u32 reason)
{
#if USE_STEAM
	if (online || gEnv->bMultiplayer)
	{
		// Steam has its own persistant stats online
		return;
	}
#endif

	if(reason & ePR_Game)
	{
		SSessionStats* pSessionStats = GetClientPersistantStats();

		DrxFixedStringT<64> temp;
		tukk pAttributePrefix = GetAttributePrefix();

		temp.Format("%s/Version", pAttributePrefix);
		pProfile->SetAttribute(temp.c_str(), k_ProfileVersionNumber);
		for(i32 i = 0; i < EIPS_Max; i++)
		{
			temp.Format("%s/%s", pAttributePrefix, s_intPersistantNames[i]);
			pProfile->SetAttribute(temp.c_str(), pSessionStats->m_intStats[i]);
		}
		for(i32 i = 0; i < EFPS_Max; i++)
		{
			temp.Format("%s/%s", pAttributePrefix, s_floatPersistantNames[i]);
			pProfile->SetAttribute(temp.c_str(), pSessionStats->m_floatStats[i]);
		}
		for(i32 i = 0; i < ESIPS_Max; i++)
		{
			temp.Format("%s/%s", pAttributePrefix, s_streakIntPersistantNames[i]);
			pProfile->SetAttribute(temp.c_str(), pSessionStats->m_streakIntStats[i].m_maxVal);
			if(gEnv->bMultiplayer && pSessionStats->m_streakIntStats[i].m_multiSession)
			{
				temp.Format("%s/%s/multi", pAttributePrefix, s_streakIntPersistantNames[i]);
				pProfile->SetAttribute(temp.c_str(), pSessionStats->m_streakIntStats[i].m_maxThisSessionVal);
				temp.Format("%s/%s/cur", pAttributePrefix, s_streakIntPersistantNames[i]);
				pProfile->SetAttribute(temp.c_str(), pSessionStats->m_streakIntStats[i].m_curVal);
			}
		}
		for(i32 i = 0; i < ESFPS_Max; i++)
		{
			temp.Format("%s/%s", pAttributePrefix, s_streakFloatPersistantNames[i]);
			pProfile->SetAttribute(temp.c_str(), pSessionStats->m_streakFloatStats[i].m_maxVal);
			if(gEnv->bMultiplayer && pSessionStats->m_streakIntStats[i].m_multiSession)
			{
				temp.Format("%s/%s/multi", pAttributePrefix, s_streakFloatPersistantNames[i]);
				pProfile->SetAttribute(temp.c_str(), pSessionStats->m_streakFloatStats[i].m_curVal);
			}
			else if(gEnv->bMultiplayer && pSessionStats->m_streakFloatStats[i].m_multiSession)
			{
				temp.Format("%s/%s/multi", pAttributePrefix, s_streakFloatPersistantNames[i]);
				pProfile->SetAttribute(temp.c_str(), pSessionStats->m_streakFloatStats[i].m_maxThisSessionVal);
				temp.Format("%s/%s/cur", pAttributePrefix, s_streakFloatPersistantNames[i]);
				pProfile->SetAttribute(temp.c_str(), pSessionStats->m_streakFloatStats[i].m_curVal);
			}
		}
		if (online && gEnv->pNetwork)
		{
			IDrxLobby* pLobby = gEnv->pNetwork->GetLobby();
			if (pLobby != NULL)
			{
				IDrxLobbyService* pLobbyService = pLobby->GetLobbyService(eCLS_Online);
				if ((pLobbyService != NULL) && (m_DateFirstPlayed_Year == 0))
				{
					SDrxSystemTime time;
					if (pLobbyService->GetSystemTime(0, &time) == eCLE_Success)
					{
						m_DateFirstPlayed_Year = time.m_Year;
						m_DateFirstPlayed_Month = time.m_Month;
						m_DateFirstPlayed_Day = time.m_Day;
						temp.Format("%s/DateFirstPlayed/Year", pAttributePrefix);
						pProfile->SetAttribute(temp.c_str(), m_DateFirstPlayed_Year);
						temp.Format("%s/DateFirstPlayed/Month", pAttributePrefix);
						pProfile->SetAttribute(temp.c_str(), m_DateFirstPlayed_Month);
						temp.Format("%s/DateFirstPlayed/Day", pAttributePrefix);
						pProfile->SetAttribute(temp.c_str(), m_DateFirstPlayed_Day);
					}
				}
			}
		}
		for(i32 i = 0; i < EMPS_Max; i++)
		{
			SaveMapStat(pProfile, pSessionStats, i);
		}
	}
}


//---------------------------------------
void CPersistantStats::LoadFromProfile(IPlayerProfile* pProfile, bool online, u32 reason)
{
#if USE_STEAM
	if (online || gEnv->bMultiplayer)
	{
		// Steam has its own persistant stats online
		return;
	}
#endif

	if(reason & ePR_Game)
	{
		// clear everything first
		Reset();

		DrxFixedStringT<64> temp;
		tukk pAttributePrefix = GetAttributePrefix();

		SSessionStats* pSessionStats = GetClientPersistantStats();
		i32 version = 0;
		temp.Format("%s/Version", pAttributePrefix);
		pProfile->GetAttribute(temp.c_str(), version);
		if(version == k_ProfileVersionNumber)
		{
			for(i32 i = 0; i < EIPS_Max; i++)
			{
				temp.Format("%s/%s", pAttributePrefix, s_intPersistantNames[i]);
				pProfile->GetAttribute(temp.c_str(), pSessionStats->m_intStats[i]);
			}
			for(i32 i = 0; i < EFPS_Max; i++)
			{
				temp.Format("%s/%s", pAttributePrefix, s_floatPersistantNames[i]);
				pProfile->GetAttribute(temp.c_str(), pSessionStats->m_floatStats[i]);
			}
			for(i32 i = 0; i < ESIPS_Max; i++)
			{
				temp.Format("%s/%s", pAttributePrefix, s_streakIntPersistantNames[i]);
				pProfile->GetAttribute(temp.c_str(), pSessionStats->m_streakIntStats[i].m_maxVal);
				if(gEnv->bMultiplayer && pSessionStats->m_streakIntStats[i].m_multiSession)
				{
					temp.Format("%s/%s/multi", pAttributePrefix, s_streakIntPersistantNames[i]);
					pProfile->GetAttribute(temp.c_str(), pSessionStats->m_streakIntStats[i].m_maxThisSessionVal);
					temp.Format("%s/%s/cur", pAttributePrefix, s_streakIntPersistantNames[i]);
					pProfile->GetAttribute(temp.c_str(), pSessionStats->m_streakIntStats[i].m_curVal);
				}
			}
			for(i32 i = 0; i < ESFPS_Max; i++)
			{
				temp.Format("%s/%s", pAttributePrefix, s_streakFloatPersistantNames[i]);
				pProfile->GetAttribute(temp.c_str(), pSessionStats->m_streakFloatStats[i].m_maxVal);
				if(gEnv->bMultiplayer && pSessionStats->m_streakIntStats[i].m_multiSession)
				{
					temp.Format("%s/%s/multi", pAttributePrefix, s_streakFloatPersistantNames[i]);
					pProfile->GetAttribute(temp.c_str(), pSessionStats->m_streakFloatStats[i].m_curVal);
				}
				else if(gEnv->bMultiplayer && pSessionStats->m_streakFloatStats[i].m_multiSession)
				{
					temp.Format("%s/%s/multi", pAttributePrefix, s_streakFloatPersistantNames[i]);
					pProfile->GetAttribute(temp.c_str(), pSessionStats->m_streakFloatStats[i].m_maxThisSessionVal);
					temp.Format("%s/%s/cur", pAttributePrefix, s_streakFloatPersistantNames[i]);
					pProfile->GetAttribute(temp.c_str(), pSessionStats->m_streakFloatStats[i].m_curVal);
				}
			}
			if (online)
			{
				temp.Format("%s/DateFirstPlayed/Year", pAttributePrefix);
				pProfile->GetAttribute(temp.c_str(), m_DateFirstPlayed_Year);
				temp.Format("%s/DateFirstPlayed/Month", pAttributePrefix);
				pProfile->GetAttribute(temp.c_str(), m_DateFirstPlayed_Month);
				temp.Format("%s/DateFirstPlayed/Day", pAttributePrefix);
				pProfile->GetAttribute(temp.c_str(), m_DateFirstPlayed_Day);
			}

			for(i32 i = 0; i < EMPS_Max; i++)
			{
				LoadMapStat(pProfile, pSessionStats, i);
			}
		}
		else
		{
			DrxLog("PersistantStats profile version mismatch, got %d, expected %d, so ignoring", version, k_ProfileVersionNumber);
		}
	}
}

//---------------------------------------
void CPersistantStats::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
		{
			if( s_multiplayer )
			{ 
				if( m_bHasCachedStartingStats && !m_bSentStats )
				{
					//private game has finished (or client has disconnected), lets reset any stats....
					m_clientPersistantStats = m_clientPersistantStatsAtGameStart;
				}

				m_bHasCachedStartingStats = false;
			}

			stl::free_container(m_previousWeaponHitMap);
			break;
		}
	}
}

//---------------------------------------
bool CPersistantStats::OnInputEvent(const SInputEvent &rInputEvent)
{
	//we use this to detect that the user is not idle (career playtime should not increase while idle)
	u32 deviceIndex = g_pGame->GetExclusiveControllerDeviceIndex();

	if (deviceIndex == rInputEvent.deviceIndex && rInputEvent.deviceType != eIDT_Unknown)
	{
		m_idleTime = 0;
	}

	return false;
}

//---------------------------------------
void CPersistantStats::LoadMapStat(IPlayerProfile* pProfile, SSessionStats* pSessionStats, i32 index)
{
	SSessionStats::SMap* pMap = &pSessionStats->m_mapStats[(EMapPersistantStats) index];
	pMap->m_map.reserve(25);
	tukk mapName = s_mapPersistantNames[index];

	//load anything that hasn't been pre-defined
	string data = "";
	pProfile->GetAttribute(string().Format("%s/%s", GetAttributePrefix(), mapName), data);
	i32 start = 0, middle = 0, end = 0;
	while((middle = data.find(',', start)) != -1 && (end = data.find(',', middle + 1)) != -1)
	{
		string substr = data.substr(start, middle - start);
		string subnum = data.substr(middle + 1, end - (middle + 1));

		i32 num = atoi(subnum);
		pMap->m_map.insert( SSessionStats::SMap::MapNameToCount::value_type(substr.c_str(), num) );

		start = end + 1;
	}

	//Load stats that have been defined map params (load this second to override anything that might have been defined twice)
	i32k mapParamCount = MapParamCount(s_mapStatsFlags[index]);
	DrxFixedStringT<64> temp;
	for(i32 j = 0; j < mapParamCount; j++)
	{
		tukk paramName = MapParamName(s_mapStatsFlags[index], j);
		i32 value = 0;
		temp.Format("%s/%s/%s", GetAttributePrefix(), mapName, paramName);
		pProfile->GetAttribute(temp.c_str(), value);
		pMap->SetStat(paramName, value);
	}
}

//---------------------------------------
void CPersistantStats::SaveMapStat(IPlayerProfile* pProfile, SSessionStats* pSessionStats, i32 index)
{
	SSessionStats::SMap* pMap = &pSessionStats->m_mapStats[(EMapPersistantStats) index];
	tukk mapName = s_mapPersistantNames[index];
	DrxFixedStringT<64> temp;

	//Save map defined params
	i32k mapParamCount = MapParamCount(s_mapStatsFlags[index]);
	for(i32 j = 0; j < mapParamCount; j++)
	{
		tukk paramName = MapParamName(s_mapStatsFlags[index], j);
		temp.Format("%s/%s/%s", GetAttributePrefix(), mapName, paramName);
		pProfile->SetAttribute(temp.c_str(), pMap->GetStat(paramName));
	}

	string data = "";
	SSessionStats::SMap::MapNameToCount::const_iterator it = pMap->m_map.begin();
	SSessionStats::SMap::MapNameToCount::const_iterator end = pMap->m_map.end();
	for ( ; it!=end; ++it)
	{
		tukk paramName =  it->first.c_str();
		i32 paramValue = it->second;
		//Save as s string if it's not already been saved
		if(!IsMapParam(s_mapStatsFlags[index], paramName))
		{
			temp.Format("%s,%d,", paramName, paramValue);
			data.append(temp.c_str());
		}
	}

	if(data.size() > 0)
	{
		temp.Format("%s/%s", GetAttributePrefix(), mapName);
		pProfile->SetAttribute(temp.c_str(), data);
	}
}

// static
// this call back is used to free the data allocated by SaveTelemetryInternal and passed to the telemetry uploader
static void FreeArrayData(uk inData)
{
	delete [] ((tuk)inData);
}

// Helper
void GetClientScoreInfo( CGameRules* pGameRules, i32 &position, i32 &points, EntityId &outClosestCompetitorEntityId, i32 &outClosestCompetitorScore )
{
	i32 clientPosition = 1; //1 being top position (not zero)
	IGameRulesPlayerStatsModule* pPlayStatsMod = pGameRules->GetPlayerStatsModule();
	DRX_ASSERT(pPlayStatsMod);
	if(pPlayStatsMod)
	{
		const SGameRulesPlayerStat* pClosestCompetitorStat = NULL;
		i32 closestCompetitorStatDiff = INT_MAX;

		CActor* pLocalActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
		if(pLocalActor)
		{
			EntityId clientId = pLocalActor->GetEntityId(); 
			const SGameRulesPlayerStat* pClientStat = pPlayStatsMod->GetPlayerStats(clientId);
			if(pClientStat)
			{
				i32k clientPoints = pClientStat->points;
				points = clientPoints;

				i32k  numStats = pPlayStatsMod->GetNumPlayerStats();
				for (i32 i=0; i<numStats; i++)
				{
					const SGameRulesPlayerStat* pPlayerStat = pPlayStatsMod->GetNthPlayerStats(i);
					if(pPlayerStat->playerId != clientId)
					{
						//They have more points so drop our position down
						if(pPlayerStat->points > clientPoints)
						{
							clientPosition++;
						}

						if (!pLocalActor->IsFriendlyEntity(pPlayerStat->playerId))
						{
							i32 pointsDiff = abs(pPlayerStat->points - clientPoints);
							if (pClosestCompetitorStat)
							{
								if (pointsDiff < closestCompetitorStatDiff)
								{
									pClosestCompetitorStat = pPlayerStat;
									closestCompetitorStatDiff = pointsDiff;
								}
							}
							else
							{
								pClosestCompetitorStat = pPlayerStat;
								closestCompetitorStatDiff = pointsDiff;
							}
						}
					}
				}
			}
		}

		if (pClosestCompetitorStat)
		{
			outClosestCompetitorEntityId = pClosestCompetitorStat->playerId;
			outClosestCompetitorScore = pClosestCompetitorStat->points;
		}
	}

	position = clientPosition;
	
}

//---------------------------------------
void CPersistantStats::SaveTelemetryInternal(tukk filenameNoExt, const SSessionStats* pSessionStats, u32 flags, bool description, bool toDisk)
{
	//ScopedSwitchToGlobalHeap useGlobalHeap;
#ifndef _RELEASE
	#define DescriptionXML(description, pos, descBuffer, name, type, mapName, testValue) \
		if(description) \
		{ \
			CreateDescriptionNode(pos, descBuffer, name, sizeof(type), #type, mapName, testValue); \
		}
#else
	#define DescriptionXML(description, pos, descBuffer, name, type, mapName, testValue)  (void) 0
#endif

	string filename = filenameNoExt;
	filename.append(".stats");

#ifndef _RELEASE
	string descriptionFilename = filenameNoExt;
	descriptionFilename.append("_map.xml");
#endif

	IDrxPak *pak = gEnv->pDrxPak;
	FILE *file = NULL;
	if(toDisk)
	{
		file = pak->FOpen(filename.c_str(),"wb");
		if(!file)
		{
			DRX_ASSERT_MESSAGE(false, ("Unable to write file to disk '%s'", filename.c_str()));
			toDisk = false;	//don't use file ptr
		}
	}
	
	i32k k_bufferSize = 4096;
	tuk buffer = new char[k_bufferSize];
	
	i32 bufferPosition = 0;

	FILE *descFile = NULL;
	TDescriptionVector descVector;

#ifndef _RELEASE
	i32 pos = 0;

	if(description)
	{
		descVector.reserve(k_descBufferSize);

		if(toDisk)
		{
			descFile = pak->FOpen(descriptionFilename.c_str(),"wb");
			if(!descFile)
			{
				DRX_ASSERT_MESSAGE(false, ("Unable to write file to disk '%s'", descriptionFilename.c_str()));
				toDisk = false;	//don't use file ptr
			}
		}
	}
#endif

	//Version Info
	i32 version = GetBinaryVersionHash(flags);

#ifndef _RELEASE
	if(description)	//add in version explicitly to top of xml
	{
		string descVersion;
		descVersion.Format("<PersistantStatsDescriptions Version=\"%d\">\n", version);

		WriteDescBuffer(descVector, descVersion);
	}
#endif

	DescriptionXML(description, pos, descVector, "TelemetryVersion", i32, 0, version);
	WriteToBuffer(buffer, bufferPosition, k_bufferSize, &version, sizeof(i32), 1);

	//Int stats
	i32 intStatsCount = 0;
	i32 intData[EIPS_Max];
	for(i32 i = 0; i < EIPS_Max; i++)
	{
		if((s_intStatsFlags[i] & flags) == flags)
		{
			DescriptionXML(description, pos, descVector, s_intPersistantNames[i], i32, 0, pSessionStats->m_intStats[i]);
			intData[intStatsCount] = pSessionStats->m_intStats[i];
			intStatsCount++;
		}
	}
	WriteToBuffer(buffer, bufferPosition, k_bufferSize, &intData[0], sizeof(i32), intStatsCount);

	//float stats
	i32 floatDataCount = 0;
	float floatData[EFPS_Max];
	for(i32 i = 0; i < EFPS_Max; i++)
	{
		if((s_floatStatsFlags[i] & flags) == flags)
		{
			DescriptionXML(description, pos, descVector, s_floatPersistantNames[i], float, 0, pSessionStats->m_floatStats[i]);
			floatData[floatDataCount] = pSessionStats->m_floatStats[i];
			floatDataCount++;
		}
	}
	WriteToBuffer(buffer, bufferPosition, k_bufferSize, &floatData[0], sizeof(float), floatDataCount);

	//Streak stats
	i32 streakDataCount = 0;
	i32 streakData[ESIPS_Max];
	for(i32 i = 0; i < ESIPS_Max; i++)
	{
		if((s_streakIntStatsFlags[i] & flags) == flags)
		{
			DescriptionXML(description, pos, descVector, s_streakIntPersistantNames[i], i32, 0, pSessionStats->m_streakIntStats[i].m_maxVal);
			streakData[streakDataCount] = pSessionStats->m_streakIntStats[i].m_maxVal;
			streakDataCount++;
		}
	}
	WriteToBuffer(buffer, bufferPosition, k_bufferSize, &streakData[0], sizeof(i32), streakDataCount);

	//Streak float stats
	i32 streakFloatDataCount = 0;
	float streakFloatData[ESFPS_Max];
	for(i32 i = 0; i < ESFPS_Max; i++)
	{
		if((s_streakFloatStatsFlags[i] & flags) == flags)
		{
			DescriptionXML(description, pos, descVector, s_streakFloatPersistantNames[i], float, 0, pSessionStats->m_streakFloatStats[i].m_maxVal);
			streakFloatData[streakFloatDataCount] = pSessionStats->m_streakFloatStats[i].m_maxVal;
			streakFloatDataCount++;
		}
	}
	WriteToBuffer(buffer, bufferPosition, k_bufferSize, &streakFloatData[0], sizeof(float), streakFloatDataCount);

	//GameRules map stats
	IGameRulesModulesUpr *pGameRulesModulesUpr = CGameRulesModulesUpr::GetInstance();
	i32 rulesDataCount = 0;
	i32k rulesCount = pGameRulesModulesUpr->GetRulesCount();
	i32k gamemodeRange = EMPS_GamesLost + 1 - EMPS_Gamemodes;
	i32 rulesData[128];
	DRX_ASSERT(rulesCount < DRX_ARRAY_COUNT(rulesData));
	for(i32 i = 0; i < rulesCount; i++)
	{
		tukk name = pGameRulesModulesUpr->GetRules(i);
		for(i32 j = EMPS_Gamemodes; j < EMPS_GamesLost + 1; j++)
		{
			if((s_mapStatsFlags[j] & flags) == flags)
			{
				DescriptionXML(description, pos, descVector, s_mapPersistantNames[j], i32, name, pSessionStats->m_mapStats[j].GetStat(name));
				rulesData[rulesDataCount] = pSessionStats->m_mapStats[j].GetStat(name);
				rulesDataCount++;
			}
		}
	}
	WriteToBuffer(buffer, bufferPosition, k_bufferSize, &rulesData[0], sizeof(i32), rulesDataCount);

	//level map stats
	i32 levelsData[32];
	i32k levelNamesCount = DRX_ARRAY_COUNT(sz_levelNames);
	COMPILE_TIME_ASSERT(levelNamesCount <= DRX_ARRAY_COUNT(levelsData));

	if((s_mapStatsFlags[EMPS_Levels] & flags) == flags)
	{
		for(i32 i = 0; i < levelNamesCount; i++)
		{
			tukk name = sz_levelNames[i];
			DescriptionXML(description, pos, descVector, s_mapPersistantNames[EMPS_Levels], i32, name, pSessionStats->m_mapStats[EMPS_Levels].GetStat(name));
			levelsData[i] = pSessionStats->m_mapStats[EMPS_Levels].GetStat(name);
		}
		WriteToBuffer(buffer, bufferPosition, k_bufferSize, &levelsData[0], sizeof(i32), levelNamesCount);
	}


	//Weapon map stats
	i32 weaponDataCount = 0;
	i32k weaponRange = EMPS_WeaponUsage + 1 - EMPS_WeaponHits;

	i32 weaponData[256];
	i32k weaponNamesCount = DRX_ARRAY_COUNT(sz_weaponNames);
	COMPILE_TIME_ASSERT(weaponNamesCount * weaponRange <= DRX_ARRAY_COUNT(weaponData));

	for(i32 i = 0; i < weaponNamesCount; i++)
	{
		tukk name = sz_weaponNames[i];
		for(i32 j = EMPS_WeaponHits; j < EMPS_WeaponUsage + 1; j++)
		{
			if((s_mapStatsFlags[j] & flags) == flags)
			{
				DescriptionXML(description, pos, descVector, s_mapPersistantNames[j], i32, name, pSessionStats->m_mapStats[j].GetStat(name));
				weaponData[weaponDataCount] = pSessionStats->m_mapStats[j].GetStat(name);
				weaponDataCount++;
			}
		}
	}
	WriteToBuffer(buffer, bufferPosition, k_bufferSize, &weaponData[0], sizeof(i32), weaponDataCount);

	//Hit Types
	CGameRules* pGameRules = g_pGame->GetGameRules();
	i32k hitTypeCount = pGameRules->GetHitTypesCount();
	i32 hitTypeDataCount = 0;
	i32 hitTypeData[128];
	DRX_ASSERT(hitTypeCount < DRX_ARRAY_COUNT(hitTypeData));
	for(i32 i = 1; i <= hitTypeCount; i++)	//hit types start from 1 for some reason
	{
		if((s_mapStatsFlags[EMPS_KillsByDamageType] & flags) == flags)
		{
			tukk name = pGameRules->GetHitType(i);
			if(name)
			{
				DescriptionXML(description, pos, descVector, s_mapPersistantNames[EMPS_KillsByDamageType], i32, name, pSessionStats->m_mapStats[EMPS_KillsByDamageType].GetStat(name));
				hitTypeData[hitTypeDataCount] = pSessionStats->m_mapStats[EMPS_KillsByDamageType].GetStat(name);
				hitTypeDataCount++;
			}
		}
	}
	WriteToBuffer(buffer, bufferPosition, k_bufferSize, &hitTypeData[0], sizeof(i32), hitTypeDataCount);

	//Int derived stats
	i32 intDerivedStatsCount = 0;
	i32 intDerivedData[EDIPS_Max];
	for(i32 i = 0; i < EDIPS_Max; i++)
	{
		if((eSF_LocalClient & flags) == flags)
		{
			i32 value = pSessionStats->GetDerivedStat((EDerivedIntPersistantStats)i);
			DescriptionXML(description, pos, descVector, s_intDerivedPersistantNames[i], i32, 0, value);
			intDerivedData[intDerivedStatsCount] = value;
			intDerivedStatsCount++;
		}
	}
	WriteToBuffer(buffer, bufferPosition, k_bufferSize, &intDerivedData[0], sizeof(i32), intDerivedStatsCount);

	// Patching and runonce data patching state
	{
		i32 patchState[3] = {0,0,0};

		if (CPatchPakUpr *pPatchPakUpr = g_pGame->GetPatchPakUpr())
		{
			i32 patchPakVersion = pPatchPakUpr->GetPatchPakVersion();
			patchState[0] = patchPakVersion;
		}

		if( IPlayerProfileUpr *pProfileMan = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileUpr() )
		{
			tukk user = pProfileMan->GetCurrentUser();
			IPlayerProfile* pProfile = pProfileMan->GetCurrentProfile( user );
			if (pProfile)
			{
				i32 runOnceVersion=CStatsRecordingMgr::GetIntAttributeFromProfile(pProfile, "RunOnceVersion");
				i32 runOnceTrackingVersion=CStatsRecordingMgr::GetIntAttributeFromProfile(pProfile, "RunOnceTrackingVersion");
				patchState[1] = runOnceVersion;
				patchState[2] = runOnceTrackingVersion;
			}
		}

#ifndef _RELEASE
		string patchStateString;

		patchStateString.Format(" <Patched_state patch_pak_version=\"%d\" run_once_version=\"%d\" run_once_tracking_version=\"%d\" />\n", patchState[0], patchState[1], patchState[2]);
		WriteDescBuffer(descVector, patchStateString);
		pos += sizeof(i32)*3;
#endif

		WriteToBuffer(buffer, bufferPosition, k_bufferSize, patchState, sizeof(i32), 3);
	}


	if(toDisk)
	{
		pak->FWrite(buffer, bufferPosition, 1, file);
		pak->FClose(file);
	}
	
	CTelemetryCollector *tc=static_cast<CTelemetryCollector*>(static_cast<CGame*>(gEnv->pGame)->GetITelemetryCollector());
	if(tc)
	{
		CStatsRecordingMgr	*pSRM=g_pGame->GetStatsRecorder();
		if (pSRM && (pSRM->GetSubmitPermissions()&CStatsRecordingMgr::k_submitPlayerStats) && ((flags&eSF_RemoteClients)==0 || (pSRM->GetSubmitPermissions()&CStatsRecordingMgr::k_submitRemotePlayerStats)))
		{
			CTelemetryMemBufferProducer			*pProducer=new CTelemetryMemBufferProducer(buffer,bufferPosition,FreeArrayData,NULL);

			buffer=NULL;									// buffer has been adopted by pProducer, don't free it here
			tc->SubmitTelemetryProducer(pProducer,filename.c_str());

#ifndef _RELEASE
			if(description)
			{
				string descCloseNode = "</PersistantStatsDescriptions>\n";
				WriteDescBuffer(descVector, descCloseNode);

				tc->AppendToFile(descriptionFilename.c_str(), (tukk) &descVector[0], descVector.size());

				if(toDisk)
				{
					pak->FWrite((tukk) &descVector[0], descVector.size(), 1, descFile);
					pak->FClose(descFile);

					DRX_ASSERT(pos == gEnv->pDrxPak->FGetSize(filename));	//checks description matches output
				}

				DRX_ASSERT(pos == bufferPosition);	//checks description matches output
			}
#endif
		}
		else
		{
			DrxLog("did not sumbit player stats as online permissions forbade it");
		}
	}

	if (buffer)
	{
		delete [] buffer;
	}
#undef DescriptionXML
}

//---------------------------------------
template <class T>
void CPersistantStats::WriteToBuffer(tuk buffer, i32 &bufferPosition, i32k bufferSize, T *data, size_t length, size_t elems)
{
#if BINARY_ENDIAN_SWAP_PLATFORM
	SwapEndian(data, elems, true);		//swap to BigEndian on PC
#endif

	size_t dataSize = length * elems;
	DRX_ASSERT(bufferSize > bufferPosition + (i32)dataSize);
	if(bufferSize > bufferPosition + (i32)dataSize)
	{
		memcpy(&buffer[bufferPosition], data, dataSize);
		bufferPosition += dataSize;
	}

#if BINARY_ENDIAN_SWAP_PLATFORM
	SwapEndian(data, elems, true);	//swap back (don't trash any live data pointers)
#endif
}

//---------------------------------------
void CPersistantStats::SaveTelemetry(bool description, bool toDisk)
{
	SSessionStats *pClientStats = GetClientPersistantStats();
	pClientStats->UpdateClientGUID();

	string clientStatsFilenameNoExt = "playerstats/local/";

	tukk clientFolderName = pClientStats->m_guid.c_str();

#if defined( _RELEASE )
	u32k clientHash = CCrc32::Compute( clientFolderName );
	stack_string clientHashStr;
	clientHashStr.Format( "%lu",clientHash );
	clientFolderName = clientHashStr.c_str();
#endif
	clientStatsFilenameNoExt.append( clientFolderName );

	SaveTelemetryInternal(clientStatsFilenameNoExt.c_str(), pClientStats, eSF_LocalClient, description, toDisk);

	stack_string remoteHashStr;
	ActorSessionMap::const_iterator it = m_sessionStats.begin();
	ActorSessionMap::const_iterator end = m_sessionStats.end();
	for ( ; it!=end; ++it)
	{
		string remoteStatsFilenameNoExt = "playerstats/remote/";
		remoteStatsFilenameNoExt.append( clientFolderName );
		remoteStatsFilenameNoExt.append("/");

		tukk remoteFolderName = it->second.m_guid.c_str();
#if defined( _RELEASE )
		u32k remoteHash = CCrc32::Compute( remoteFolderName );
		remoteHashStr.Format( "%lu", remoteHash );
		remoteFolderName = remoteHashStr.c_str();
#endif

		remoteStatsFilenameNoExt.append( remoteFolderName );

		SaveTelemetryInternal(remoteStatsFilenameNoExt.c_str(), &it->second, eSF_RemoteClients, description, toDisk);
	}

	// Reset time spent waiting for a game
	pClientStats->m_intStats[EIPS_LobbyTime] = 0;
}

#ifndef _RELEASE
//static//---------------------------------
void CPersistantStats::CmdDumpTelemetryDescription(IConsoleCmdArgs* pCmdArgs)
{
	CPersistantStats* pStats = CPersistantStats::GetInstance();
	pStats->SaveTelemetry(true, true);	//also do description maps and write to disk
}

//static//---------------------------------
template <class T>
void CPersistantStats::CreateDescriptionNode(i32 &pos, TDescriptionVector& descBuffer, tukk codeName, size_t size, tukk type, tukk mapName, T testValue)
{
	string descNode;
	descNode.Format(" <Stat Position=\"%d\" CodeName=\"%s\"", pos, codeName);
	if(mapName)
	{
		descNode = string().Format("%s MapName=\"%s\"", descNode.c_str(), mapName);
	}

	if(strcmp(type, "float") == 0)
	{
		descNode = string().Format("%s Type=\"%s\" TestValue=\"%f\"/>\n", descNode.c_str(), type, testValue);	//floats
	}
	else
	{
		DRX_ASSERT_MESSAGE((strcmp(type, "i32") == 0), "Expected type i32");
		descNode = string().Format("%s Type=\"%s\" TestValue=\"%d\"/>\n", descNode.c_str(), type, testValue);	//Treat any non-floats as ints
	}

	WriteDescBuffer(descBuffer, descNode);

	pos += size;
}

//static
void CPersistantStats::WriteDescBuffer(TDescriptionVector& buffer, string text)
{
	size_t len = text.length();
	size_t bufferStart = buffer.size();
	buffer.resize(bufferStart + len);
	memcpy(&buffer[bufferStart], text.c_str(), len);
}
#endif

//static//---------------------------------
i32 CPersistantStats::GetBinaryVersionHash(u32 flags)
{
	//slow but robust hashing of names of everything!
	u32 hash = 40503;

	for(i32 i = 0; i < EIPS_Max; i++)
	{
		hash = DrxStringUtils::HashStringSeed(s_intPersistantNames[i], hash);
	}
	for(i32 i = 0; i < EFPS_Max; i++)
	{
		hash = DrxStringUtils::HashStringSeed(s_floatPersistantNames[i], hash);
	}
	for(i32 i = 0; i < ESIPS_Max; i++)
	{
		hash = DrxStringUtils::HashStringSeed(s_streakIntPersistantNames[i], hash);
	}
	for(i32 i = 0; i < ESFPS_Max; i++)
	{
		hash = DrxStringUtils::HashStringSeed(s_streakFloatPersistantNames[i], hash);
	}
	for(i32 i = 0; i < EMPS_Max; i++)
	{
		hash = DrxStringUtils::HashStringSeed(s_mapPersistantNames[i], hash);
	}
	for(i32 i = 0; i < EDIPS_Max; i++)
	{
		hash = DrxStringUtils::HashStringSeed(s_intDerivedPersistantNames[i], hash);
	}
	for(i32 i = 0; i < EAMA_Max; i++)
	{
		hash = DrxStringUtils::HashStringSeed(m_afterMatchAwards.GetNameForAward((EAfterMatchAwards)i), hash);
	}

	hash += s_levelNamesVersion;

	i32k weaponNamesCount = DRX_ARRAY_COUNT(sz_weaponNames);
	for(i32 i = 0; i < weaponNamesCount; i++)
	{
		tukk name = sz_weaponNames[i];
		hash = DrxStringUtils::HashStringSeed(name, hash);
	}

	IGameRulesModulesUpr *pGameRulesModulesUpr = CGameRulesModulesUpr::GetInstance();
	i32k rulesCount = pGameRulesModulesUpr->GetRulesCount();
	for(i32 i = 0; i < rulesCount; i++)
	{
		tukk name = pGameRulesModulesUpr->GetRules(i);
		hash = DrxStringUtils::HashStringSeed(name, hash);
	}

	CGameRules* pGameRules = g_pGame->GetGameRules();
	i32k hitTypeCount = pGameRules->GetHitTypesCount();
	for(i32 i = 1; i <= hitTypeCount; i++)
	{
		tukk name = pGameRules->GetHitType(i);
		if(name)
		{
			hash = DrxStringUtils::HashStringSeed(name, hash);
		}
	}

	hash ^= flags;

	return (i32) hash;
}

//---------------------------------------
i32 CPersistantStats::GetStat(EIntPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetStat(stat);
}

//---------------------------------------
float CPersistantStats::GetStat(EFloatPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetStat(stat);
}

//---------------------------------------
i32 CPersistantStats::GetStat(EStreakIntPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetStat(stat);
}

//---------------------------------------
float CPersistantStats::GetStat(EStreakFloatPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetStat(stat);
}

//---------------------------------------
i32 CPersistantStats::GetStatThisSession(EStreakIntPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetMaxThisSessionStat(stat);
}

//---------------------------------------
float CPersistantStats::GetStatThisSession(EStreakFloatPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetMaxThisSessionStat(stat);
}

//---------------------------------------
void CPersistantStats::ResetStat(EStreakIntPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	DRX_ASSERT(stat >= 0 && stat < DRX_ARRAY_COUNT(pSessionStats->m_streakIntStats));
	SSessionStats::SStreak<i32> &streak = pSessionStats->m_streakIntStats[stat];
	if(streak.m_multiSession)
	{
		streak.m_maxThisSessionVal = 0;
	}
}

//---------------------------------------
void CPersistantStats::ResetStat(EStreakFloatPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	DRX_ASSERT(stat >= 0 && stat < DRX_ARRAY_COUNT(pSessionStats->m_streakFloatStats));
	SSessionStats::SStreak<float> &streak = pSessionStats->m_streakFloatStats[stat];
	if(streak.m_multiSession)
	{
		streak.m_maxThisSessionVal = 0;
	}
}

//---------------------------------------
i32 CPersistantStats::GetStat(tukk name, EMapPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetStat(name, stat);
}

//---------------------------------------
const SSessionStats::SMap::MapNameToCount& CPersistantStats::GetStat(EMapPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->m_mapStats[stat].m_map;
}

//---------------------------------------
i32 CPersistantStats::GetDerivedStat(EDerivedIntPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetDerivedStat(stat);
}

//---------------------------------------
float CPersistantStats::GetDerivedStat(EDerivedFloatPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetDerivedStat(stat);
}



//---------------------------------------
i32 CPersistantStats::GetDerivedStat(tukk name, EDerivedIntMapPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetDerivedStat(name, stat);
}



//---------------------------------------
float CPersistantStats::GetDerivedStat(tukk name, EDerivedFloatMapPersistantStats stat)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetDerivedStat(name, stat);
}

//---------------------------------------
tukk  CPersistantStats::GetDerivedStat(EDerivedStringPersistantStats stat)
{
	DRX_ASSERT(stat > EDSPS_Invalid && stat < EDSPS_Max);
	switch(stat)
	{
	case EDSPS_FavouriteWeapon:
		{
			if (const CItemSharedParams* pItemShared = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(m_favoriteWeapon.c_str(), false))
			{
				m_dummyStr = pItemShared->params.display_name.c_str();
			}
			return m_dummyStr;
		}
	case EDSPS_FavouriteAttachment:
		{
			if (const CItemSharedParams* pItemShared = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(m_favoriteAttachment.c_str(), false))
			{
				m_dummyStr = pItemShared->params.display_name.c_str();
			}
			return m_dummyStr;
		}

	default:
		DRX_ASSERT_MESSAGE(false, string().Format("Failed to find EDerivedStringPersistantStats %d", stat));
		return NULL;
	}
}

//---------------------------------------
tukk  CPersistantStats::GetDerivedStat(EDerivedStringMapPersistantStats stat)
{
	DRX_ASSERT(stat > EDSMPS_Invalid && stat < EDSMPS_Max);

	SSessionStats *pSessionStats = GetClientPersistantStats();
	return pSessionStats->GetDerivedStat(stat);	
}

i32 CPersistantStats::GetStatForActorThisSession(EStreakIntPersistantStats stat, EntityId inActorId)
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	bool isLocalClient = (inActorId == localClientId);
	i32 result=0;

	DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() stat=%d; inActorId=%d; isLocalClient=%d", stat, inActorId, isLocalClient);
	if (isLocalClient)
	{
		SSessionStats * pSessionStats = GetClientPersistantStats();
		result = pSessionStats->GetMaxThisSessionStat(stat); 
		
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a local client calculated result=%d", result);
	}
	else
	{
		SSessionStats *stats = GetActorSessionStats(inActorId);
		DRX_ASSERT_MESSAGE(stats, string().Format("GetStatForActorThisSession() failed to find session stats for actor=%d", inActorId));
		if (stats)
		{
			result=stats->GetMaxThisSessionStat(stat);
		}
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a remote client just used session stats directly with result=%d", result);
	}

	return result;
}

float CPersistantStats::GetStatForActorThisSession(EStreakFloatPersistantStats stat, EntityId inActorId)
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	bool isLocalClient = (inActorId == localClientId);
	float result=0;

	DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() stat=%d; inActorId=%d; isLocalClient=%d", stat, inActorId, isLocalClient);
	if (isLocalClient)
	{
		SSessionStats * pSessionStats = GetClientPersistantStats();
		result = pSessionStats->GetMaxThisSessionStat(stat); 
		
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a local client calculated result=%f", result);
	}
	else
	{
		SSessionStats *stats = GetActorSessionStats(inActorId);
		DRX_ASSERT_MESSAGE(stats, string().Format("GetStatForActorThisSession() failed to find session stats for actor=%d", inActorId));
		if (stats)
		{
			result=stats->GetMaxThisSessionStat(stat);
		}
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a remote client just used session stats directly with result=%f", result);
	}

	return result;
}

//---------------------------------------
i32 CPersistantStats::GetStatForActorThisSession(EIntPersistantStats stat, EntityId inActorId)
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	bool isLocalClient = (inActorId == localClientId);
	i32 result=0;

	DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() stat=%d; inActorId=%d; isLocalClient=%d", stat, inActorId, isLocalClient);
	if (isLocalClient)
	{
		SSessionStats *pSessionStartStats = GetClientPersistantStatsAtSessionStart();
		i32 statAtSessionStart = pSessionStartStats->GetStat(stat);
		SSessionStats *pSessionEndStats = GetClientPersistantStats();
		i32 statAtSessionEnd = pSessionEndStats->GetStat(stat);
		result = statAtSessionEnd - statAtSessionStart;
		
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a local client calculated statAtSessionStart=%d; statAtSessionEnd=%d; delta result=%d", statAtSessionStart, statAtSessionEnd, result);
	}
	else
	{
		SSessionStats *stats = GetActorSessionStats(inActorId);
		DRX_ASSERT_MESSAGE(stats, string().Format("GetStatForActorThisSession() failed to find session stats for actor=%d", inActorId));
		if (stats)
		{
			result=stats->GetStat(stat);
		}
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a remote client just used session stats directly with result=%d", result);
	}

	return result;
}

//---------------------------------------
float CPersistantStats::GetStatForActorThisSession(EFloatPersistantStats stat, EntityId inActorId)
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	bool isLocalClient = (inActorId == localClientId);
	float result=0;

	DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() stat=%d; inActorId=%d; isLocalClient=%d", stat, inActorId, isLocalClient);
	if (isLocalClient)
	{
		SSessionStats *pSessionStartStats = GetClientPersistantStatsAtSessionStart();
		float statAtSessionStart = pSessionStartStats->GetStat(stat);
		float statAtSessionEnd = GetStat(stat);
		result = statAtSessionEnd - statAtSessionStart;
		
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a local client calculated statAtSessionStart=%f; statAtSessionEnd=%f; delta result=%f", statAtSessionStart, statAtSessionEnd, result);
	}
	else
	{
		SSessionStats *stats = GetActorSessionStats(inActorId);
		DRX_ASSERT_MESSAGE(stats, string().Format("GetStatForActorThisSession() failed to find session stats for actor=%d", inActorId));
		if (stats)
		{
			result=stats->GetStat(stat);
		}
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a remote client just used session stats directly with result=%f", result);
	}

	return result;
}

//---------------------------------------
i32 CPersistantStats::GetStatForActorThisSession(EMapPersistantStats stat, tukk  param, EntityId inActorId)
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	bool isLocalClient = (inActorId == localClientId);
	i32 result=0;

	DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() stat=%d; param=%s; inActorId=%d; isLocalClient=%d", stat, param, inActorId, isLocalClient);
	if (isLocalClient)
	{
		SSessionStats *pSessionStartStats = GetClientPersistantStatsAtSessionStart();
		i32 statAtSessionStart = pSessionStartStats->GetStat(param, stat);
		i32 statAtSessionEnd = GetStat(param,stat);
		result = statAtSessionEnd - statAtSessionStart;
		
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a local client calculated statAtSessionStart=%d; statAtSessionEnd=%d; delta result=%d", statAtSessionStart, statAtSessionEnd, result);
	}
	else
	{
		SSessionStats *stats = GetActorSessionStats(inActorId);
		DRX_ASSERT_MESSAGE(stats, string().Format("GetStatForActorThisSession() failed to find session stats for actor=%d", inActorId));
		if (stats)
		{
			result=stats->GetStat(param,stat);
		}
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetStatForActorThisSession() is a remote client just used session stats directly with result=%d", result);
	}

	return result;
}
//---------------------------------------
i32 CPersistantStats::GetDerivedStatForActorThisSession(EDerivedIntPersistantStats stat, EntityId inActorId)
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	bool isLocalClient = (inActorId == localClientId);
	i32 statAtSessionStart=0;
	i32 statAtSessionEnd=0;
	i32 result=0;

	DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetDerivedStatForActorThisSession() stat=%d; inActorId=%d; isLocalClient=%d", stat, inActorId, isLocalClient);
	if (isLocalClient)
	{
		SSessionStats *pSessionStartStats = GetClientPersistantStatsAtSessionStart();
		statAtSessionStart = pSessionStartStats->GetDerivedStat(stat);
		statAtSessionEnd = GetDerivedStat(stat);
		result = statAtSessionEnd - statAtSessionStart;
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetDerivedStatForActorThisSession() is a local client calculated statAtSessionStart=%d; statAtSessionEnd=%d; delta result=%d", statAtSessionStart, statAtSessionEnd, result);
	}
	else
	{
		SSessionStats *stats = GetActorSessionStats(inActorId);
		DRX_ASSERT_MESSAGE(stats, string().Format("GetDerivedStatForActorThisSession() failed to find session stats for actor=%d", inActorId));
		if (stats)
		{
			result=stats->GetDerivedStat(stat);
		}
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetDerivedStatForActorThisSession() is a remote client just used session stats directly with result=%d", result);
	}

	return result;
}

//---------------------------------------
float CPersistantStats::GetDerivedStatForActorThisSession(EDerivedFloatPersistantStats stat, EntityId inActorId)
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	bool isLocalClient = (inActorId == localClientId);
	float statAtSessionStart=0.f;
	float statAtSessionEnd=0.f;
	float result=0.f;

	DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetDerivedStatsForActor() stat=%d; inActorId=%d; isLocalClient=%d", stat, inActorId, isLocalClient);
	if (isLocalClient)
	{
		SSessionStats *pSessionStartStats = GetClientPersistantStatsAtSessionStart();
		statAtSessionStart = pSessionStartStats->GetDerivedStat(stat);
		statAtSessionEnd = GetDerivedStat(stat);
		result = statAtSessionEnd - statAtSessionStart;
		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetDerivedStatForActorThisSession() is a local client calculated statAtSessionStart=%f; statAtSessionEnd=%f; delta result=%f", statAtSessionStart, statAtSessionEnd, result);
	}
	else
	{
		SSessionStats *stats = GetActorSessionStats(inActorId);
		DRX_ASSERT_MESSAGE(stats, string().Format("GetDerivedStatForActor() failed to find session stats for actor=%d", inActorId));
		if (stats)
		{
			result=stats->GetDerivedStat(stat);
		}

		DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetDerivedStatForActorThisSession() is a remote client just used session stats directly with result=%f", result);
	}

	return result;
}

void CPersistantStats::GetMapStatForActorThisSession(EMapPersistantStats stat, EntityId inActorId, std::vector<i32> &result)
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	bool isLocalClient = (inActorId == localClientId);

	DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetMapStatForActor() stat=%d; inActorId=%d; isLocalClient=%d", stat, inActorId, isLocalClient);
	if (isLocalClient)
	{
		SSessionStats *pStartSessionStats = GetClientPersistantStatsAtSessionStart();
		const SSessionStats::SMap::MapNameToCount& statAtSessionStart = pStartSessionStats->GetStat(stat);
		const SSessionStats::SMap::MapNameToCount& statAtSessionEnd = GetStat(stat);
		SSessionStats::SMap::MapNameToCount::const_iterator enditem = statAtSessionEnd.begin();
		SSessionStats::SMap::MapNameToCount::const_iterator endend = statAtSessionEnd.end();

		for ( ; enditem != endend; ++enditem )
		{
			SSessionStats::SMap::MapNameToCount::const_iterator item = statAtSessionStart.find((*enditem).first);
			if ( item != statAtSessionStart.end() )
			{
				result.push_back( ((*enditem).second - (*item).second) );
				DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetMapStatForActor() is a local client calculated statAtSessionStart=%d; statAtSessionEnd=%d; delta result=%d", (*item).second, (*enditem).second, result[result.size()-1]);
			}
			else
			{
				result.push_back( (*enditem).second );
				DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetMapStatForActor() is a local client calculated statAtSessionStart=%d; statAtSessionEnd=%d; delta result=%d", (*enditem).second, (*enditem).second, result[result.size()-1]);
			}
		}
	}
	else
	{
		SSessionStats *stats = GetActorSessionStats(inActorId);
		DRX_ASSERT_MESSAGE(stats, string().Format("GetActorSessionStats() failed to find session stats for actor=%d", inActorId));
		if (stats)
		{
			const SSessionStats::SMap::MapNameToCount& statmap = stats->GetStat(stat);
			SSessionStats::SMap::MapNameToCount::const_iterator item = statmap.begin();
			SSessionStats::SMap::MapNameToCount::const_iterator end = statmap.end();
			for( ; item != end; ++item )
			{
				result.push_back((*item).second);
				DRX_DEBUG_LOG(AFTER_MATCH_AWARDS, "CPersistantStats::GetMapStatForActor() is a remote client just used session stats directly with result=%d", result[result.size()-1]);
			}
		}
	}
}


//---------------------------------------
i32 CPersistantStats::GetCurrentStat(EntityId actorId, EStreakIntPersistantStats stat)
{
	const SSessionStats* pActorStats = GetActorSessionStats(actorId);
	if(pActorStats)
	{
		DRX_ASSERT(stat >= 0 && stat < DRX_ARRAY_COUNT(pActorStats->m_streakIntStats));
		return pActorStats->m_streakIntStats[stat].m_curVal;
	}
	return 0;
}



//---------------------------------------
SSessionStats* CPersistantStats::GetActorSessionStats(EntityId actorId)
{
	EntityId clientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	if(clientId == actorId && clientId != 0)
	{
		return GetClientPersistantStats();
	}

	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(actorId);
	if(!pActor || !pActor->IsPlayer())
		return NULL;

	ActorSessionMap::iterator it = m_sessionStats.find(actorId);
	if(it != m_sessionStats.end())
	{
		return &it->second;
	}
	else
	{
		SSessionStats stats;
		stats.UpdateGUID(actorId);
		ActorSessionMap::iterator inserted = m_sessionStats.insert(ActorSessionMap::value_type(actorId, stats)).first;
		return &inserted->second;
	}
}

//---------------------------------------
SSessionStats* CPersistantStats::GetClientPersistantStats()
{
	return &m_clientPersistantStats;
}

//---------------------------------------
SSessionStats* CPersistantStats::GetClientPersistantStatsAtSessionStart()
{
	return &m_clientPersistantStatsAtGameStart;
}

//---------------------------------------
void CPersistantStats::Update(const float dt)
{
	DRX_ASSERT(gEnv->bMultiplayer == s_multiplayer);

	//bUpdateTimers should be used instead of dt so that you don't get floating point errors when timers become large
	bool bUpdateTimers = false;
	m_fSecondTimer += dt;
	if(m_fSecondTimer > 1.0f)
	{
		bUpdateTimers = true;
		m_fSecondTimer -= 1.0f;
		m_idleTime++;
	}

	SSessionStats *pSessionStats = GetClientPersistantStats();
	
	if( bUpdateTimers && gEnv->bMultiplayer )
	{
		//Update Multiplayer timers
		pSessionStats->m_intStats[EIPS_Overall]++;

		bool inGame = false;

		CGameLobby* pGameLobby = g_pGame->GetGameLobby();
		if( pGameLobby )
		{
			const ELobbyState lobbyState = pGameLobby->GetState();

			if( (lobbyState == eLS_Game) || pGameLobby->IsGameStarting() )
			{
				inGame = true;
			}

			if( (lobbyState != eLS_None) && !inGame )
			{
				pSessionStats->m_intStats[EIPS_LobbyTime]++;
			}
		}
		
		if( m_localPlayerInVTOL )
		{
			pSessionStats->m_intStats[EIPS_TimeInVTOLs]++;
		}
	}
	
	if(!ShouldUpdateStats())
	{
		return;
	}

	m_afterMatchAwards.Update(dt);

	if( bUpdateTimers && m_idleTime < IDLE_FOR_TELEM_TIME )
	{
		pSessionStats->m_intStats[EIPS_TimePlayed]++;
	}

	CGameRules* pGameRules = g_pGame->GetGameRules();
  if(pGameRules != NULL && pGameRules->IsGameInProgress())
  {
    IActor* pClientActor =gEnv->pGame->GetIGameFramework()->GetClientActor();
    if(pClientActor != NULL && !pClientActor->IsDead())
    {
			CPlayer* pClientPlayer = static_cast<CPlayer*>(pClientActor);

	    float distance = pClientPlayer->GetActorPhysics().velocity.len() * dt;

	    if(pClientPlayer->IsInAir())
	    {
		    pSessionStats->m_floatStats[EFPS_DistanceAir]+= distance;
		    pSessionStats->m_streakFloatStats[ESFPS_DistanceAir].Increment(distance);
	    }
	    else if(pClientPlayer->IsSwimming())
	    {
		    pSessionStats->m_streakFloatStats[ESFPS_DistanceAir].Reset();

	  		pSessionStats->m_floatStats[EFPS_DistanceSwum]+= distance;
	    }
	    else if(pClientPlayer->IsSliding())
	    {
		    pSessionStats->m_floatStats[EFPS_DistanceSlid]+= distance;
	    }
	    else
	    {
		    pSessionStats->m_floatStats[EFPS_DistanceRan]+= distance;
				if(pClientPlayer->IsSprinting())
				{
					pSessionStats->m_floatStats[EFPS_DistanceSprint]+= distance;
				}
		    pSessionStats->m_streakFloatStats[ESFPS_DistanceAir].Reset();
	    }

	    const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();

	    EnemyTeamMemberInfoMap::iterator it = m_enemyTeamMemberInfoMap.begin();
	    EnemyTeamMemberInfoMap::iterator end = m_enemyTeamMemberInfoMap.end();

	    for ( ; it!=end; ++it)
	    {
		    if (it->second.m_state == SEnemyTeamMemberInfo::k_state_killed)
		    {
			    if ((currentTime - it->second.m_timeEnteredKilledState) > SEnemyTeamMemberInfo::k_timeInKillStateTillReset)
			    {
				    it->second.m_state = SEnemyTeamMemberInfo::k_state_initial;
			    }
		    }
	    }

	    bool crouching = pClientPlayer->GetStance() == STANCE_CROUCH;
	    if(crouching != m_crouching)
	    {
		    const static float k_timeNeededForCrouches = 1.5f;

		    i32 indexToReplace = 0;
		    float oldestTime = 1e+37f;
		    for(i32 i = 0; i < k_crouchesToggleNeeded; i++)
		    {
			    if(m_crouchToggleTime[i] < oldestTime)
			    {
				    indexToReplace = i;
				    oldestTime = m_crouchToggleTime[i];
			    }
		    }

		    m_crouchToggleTime[indexToReplace] = currentTime;

		    if(oldestTime + k_timeNeededForCrouches > currentTime)
		    {
			    // get entity of person being tea bagged
			    // and ....
			    EntityId corpseId=ClientNearCorpse(pClientPlayer);
			    if(corpseId)
					{
				    SEnemyTeamMemberInfo *enemyTeamMemberInfo = GetEnemyTeamMemberInfo(corpseId);
				    if (enemyTeamMemberInfo != NULL && !enemyTeamMemberInfo->m_teabaggedThisDeath)
						{
							enemyTeamMemberInfo->m_teabaggedThisDeath = true;

							pSessionStats->m_intStats[EIPS_CrouchingOverCorpses]++;
							memset(&m_crouchToggleTime, 0, sizeof(m_crouchToggleTime));

							CGameRules* pNewGameRules = g_pGame->GetGameRules();

							EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
							i32 localClientTeamId = pNewGameRules->GetTeam(localClientId);
							i32 corpseTeamId = pNewGameRules->GetTeam(corpseId);

							if (!pClientPlayer->IsFriendlyEntity(corpseId))
							{
								{
									if (enemyTeamMemberInfo->m_state == SEnemyTeamMemberInfo::k_state_killed)
									{
										enemyTeamMemberInfo->m_state = SEnemyTeamMemberInfo::k_state_crouchedOver;
										pSessionStats->m_intStats[EIPS_TaggedAndBagged]++;
									}
								}
							}
						}
			    }
		    }

		    m_crouching = crouching;
	    }

			if(bUpdateTimers)
			{
				pSessionStats->m_streakIntStats[ESIPS_TimeAlive].Increment(1);

				if ( crouching )
				{
					pSessionStats->m_intStats[EIPS_TimeCrouched]++;
				}
				
				//Cloaking near enemy
				if(pClientPlayer->IsCloaked())
				{
					if(NearEnemy(pClientPlayer))
					{
						pSessionStats->m_intStats[EIPS_CloakedNearEnemy]++;
					}
				}
			}

    }
    else
    {
	    pSessionStats->m_streakIntStats[ESIPS_TimeAlive].Reset();
	    pSessionStats->m_streakIntStats[ESIPS_HeadshotsPerLife].Reset();
	    pSessionStats->m_streakIntStats[ESIPS_HeadshotKillsPerLife].Reset();
    }
    
  }

	float currentTime=gEnv->pTimer->GetCurrTime();
	i32 len=m_grenadeKills.size();
	//DrxWatch("%d grenadeKills currentTime=%f", m_grenadeKills.size(), currentTime);

	for (i32 i=0;i<len;i++)
	{
		//DrxWatch("grenadeKills[%d] time=%f; victim=%s (%d)", i, m_grenadeKills[i].m_atTime, g_pGame->GetGameRules()->GetEntityName(m_grenadeKills[i].m_victimId), m_grenadeKills[i].m_victimId);

		if (currentTime - m_grenadeKills[i].m_atTime > k_fastGrenadeKillTimeout)
		{
			m_grenadeKills.removeAt(i);
			i--;
			len--;
		}
	}
}

//---------------------------------------
bool CPersistantStats::ShouldUpdateStats()
{
	return true;
}

//---------------------------------------
bool CPersistantStats::NearFriendly(CPlayer *pClientPlayer, float fCheckDistSq /* = 16.0f */)
{
	//Does not work for non-team-games. Can be changed to fairly easily...
	CActorUpr * pActorUpr = CActorUpr::GetActorUpr();

	pActorUpr->PrepareForIteration();

	IEntity * pEntity		= pClientPlayer->GetEntity();
	EntityId  playerId	= pClientPlayer->GetEntityId();		//This is actually faster than getting it from the entity
																												// as the actor stores it in a member var and it is likely
																												// to already be in cache

	Vec3 playerPos				= pEntity->GetWorldPos();
	i32k kNumActors	= pActorUpr->GetNumActors();
	i32k iPlayerTeam = g_pGame->GetGameRules()->GetTeam(playerId);

	for(i32 i = 0; i < kNumActors; i++)
	{
		SActorData actorData;
		pActorUpr->GetNthActorData(i, actorData);

		if(actorData.teamId == iPlayerTeam && actorData.health > 0)
		{
			const float fDistanceSq = playerPos.GetSquaredDistance(actorData.position);
			if(fCheckDistSq > fDistanceSq)
			{
				return true;
			}
		}
	}
	
	return false;
}

//---------------------------------------
bool CPersistantStats::NearEnemy(CPlayer *pClientPlayer, float fCheckDistSq /* = 16.0f */)
{
	//Does not work for non-team-games. Can be changed to fairly easily...
	CActorUpr * pActorUpr = CActorUpr::GetActorUpr();

	pActorUpr->PrepareForIteration();

	IEntity * pEntity		= pClientPlayer->GetEntity();
	EntityId  playerId	= pClientPlayer->GetEntityId();		//This is actually faster than getting it from the entity
	// as the actor stores it in a member var and it is likely
	// to already be in cache

	Vec3 playerPos				= pEntity->GetWorldPos();
	i32k kNumActors	= pActorUpr->GetNumActors();
	i32k iPlayerTeam = g_pGame->GetGameRules()->GetTeam(playerId);

	for(i32 i = 0; i < kNumActors; i++)
	{
		SActorData actorData;
		pActorUpr->GetNthActorData(i, actorData);

		if(actorData.teamId != iPlayerTeam && actorData.health > 0)
		{
			const float fDistanceSq = playerPos.GetSquaredDistance(actorData.position);
			if(fCheckDistSq > fDistanceSq)
			{
				if(gEnv->bMultiplayer)
				{
					return true;
				}
				else
				{
					IEntity* pEnemyEntity = gEnv->pEntitySystem->GetEntity(actorData.entityId);
					if(pEnemyEntity != NULL)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//---------------------------------------
EntityId CPersistantStats::ClientNearCorpse(CPlayer *pClientPlayer)
{
	EntityId corpseId=0;

	//Does not work for non-team-games. Can be changed to fairly easily...
	CActorUpr * pActorUpr = CActorUpr::GetActorUpr();

	pActorUpr->PrepareForIteration();

	IEntity * pEntity		= pClientPlayer->GetEntity();
	EntityId  playerId	= pClientPlayer->GetEntityId();		//This is actually faster than getting it from the entity
																												// as the actor stores it in a member var and it is likely
																												// to already be in cache
	const static float k_nearCorpseDistance2 = 2.0f;

	Vec3 playerPos				= pEntity->GetWorldPos();
	i32k kNumActors	= pActorUpr->GetNumActors();
	i32k iPlayerTeam = g_pGame->GetGameRules()->GetTeam(playerId);

	for(i32 i = 0; i < kNumActors; i++)
	{
		SActorData actorData;
		pActorUpr->GetNthActorData(i, actorData);

		if(actorData.health <= 0 && actorData.teamId != iPlayerTeam)
		{
			const float fDistanceSq = playerPos.GetSquaredDistance(actorData.position);
			if(k_nearCorpseDistance2 > fDistanceSq)
			{
				corpseId = actorData.entityId;
				break;
			}
		}
	}	

	return corpseId;
}

//---------------------------------------
void CPersistantStats::EnteredGame()
{
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();

	bool found = pGameFramework->GetNetworkSafeClassId(m_pickAndThrowWeaponClassId, "PickAndThrowWeapon");
	DRX_ASSERT_MESSAGE(found, "Unable to find PickAndThrowWeapon");

	m_clientPreviousKillData.clear();
	m_sessionStats.clear();

	m_clientPersistantStatsAtGameStart = m_clientPersistantStats;
	m_bHasCachedStartingStats = true;
	m_bSentStats = false;
	m_afterMatchAwards.EnteredGame();
	m_enemyTeamMemberInfoMap.clear();

#if !USE_PC_PREMATCH
	OnGameStarted();
#endif

	SetInGame(true);
}


void CPersistantStats::ClientDisconnect( EntityId clientId )
{
	if(clientId == gEnv->pGame->GetIGameFramework()->GetClientActorId())
	{
		ClearListeners();
	}
}

void CPersistantStats::HandleTaggingEntity(EntityId shooterId, EntityId targetId)
{
	CGameRules *pGameRules = g_pGame->GetGameRules();
	CActor* pLocalActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
	EntityId localClientId = 0;
	if (pLocalActor)
	{
		localClientId = pLocalActor->GetEntityId();
	
		// if shooter is local client
		if (shooterId == localClientId)
		{
			i32 localClientTeamId = pGameRules->GetTeam(localClientId);

			IncrementClientStats(EIPS_TaggedEntities);
			
			if (!pLocalActor->IsFriendlyEntity(targetId))
			{
				SEnemyTeamMemberInfo *enemyTeamMemberInfo = GetEnemyTeamMemberInfo(targetId);
				if (enemyTeamMemberInfo)
				{
					if (enemyTeamMemberInfo->m_state == SEnemyTeamMemberInfo::k_state_initial)
					{
						enemyTeamMemberInfo->m_state = SEnemyTeamMemberInfo::k_state_tagged;
					}

					//Check to see if we have now tagged everyone
					if(!enemyTeamMemberInfo->m_taggedThisMatch)
					{
						enemyTeamMemberInfo->m_taggedThisMatch = true;

						bool taggedAll = true;

						CActorUpr* pActorUpr = CActorUpr::GetActorUpr();
						pActorUpr->PrepareForIteration();
						i32k numActors = pActorUpr->GetNumActors();
						const EnemyTeamMemberInfoMap::const_iterator end = m_enemyTeamMemberInfoMap.end();

						i32 numEnemies = 0;
						for (i32 i = 0; i < numActors; ++i)
						{
							SActorData actorData;
							pActorUpr->GetNthActorData(i, actorData);
							if (actorData.teamId != localClientTeamId)
							{
								++numEnemies;

								EnemyTeamMemberInfoMap::const_iterator it = m_enemyTeamMemberInfoMap.find(actorData.entityId);
								if(it == end || !it->second.m_taggedThisMatch)
								{
									taggedAll = false;
									break;
								}
							}
						}

						if(taggedAll && numEnemies >= kNumEnemyRequiredForAwards)
						{
							IncrementClientStats(EIPS_TaggedEntireEnemyTeam);
						}
					}
				}
			}
		}
	}
}

//---------------------------------------
void CPersistantStats::HandleLocalWinningKills()
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();

	i32 firstBloodThisSession=GetStatForActorThisSession(EIPS_FirstBlood, localClientId);
	if (firstBloodThisSession>0)
	{
		i32 winningKillThisSession=GetStatForActorThisSession(EIPS_WinningKill, localClientId);
		if(winningKillThisSession>0)
		{
			SSessionStats *pSessionStats = GetClientPersistantStats();
			pSessionStats->m_intStats[EIPS_WinningKillAndFirstBlood]++;
		}
	}
}

//---------------------------------------
void CPersistantStats::UpdateGamemodeTime(tukk gamemodeName, SSessionStats* pClientStats, CActor* pClientActor)
{
	if(m_gamemodeTimeValid)
	{
		const float currentTime = gEnv->pTimer->GetCurrTime();
		i32 timeInGamemode = (i32)( currentTime - m_gamemodeStartTime );
		pClientStats->m_mapStats[EMPS_GamemodesTime].Update(gamemodeName, timeInGamemode);

		if( pClientActor )
		{
			BLAZE_REPORT_PLAYER_LOCAL_OFFLINE(pClientActor, timePlayed, timeInGamemode);
		}

		m_gamemodeTimeValid = false;
	}
}

//---------------------------------------
void CPersistantStats::UpdateWeaponTime(CWeapon* pCWeapon)
{
	const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	i32 usageTime = (i32)( currentTime - m_selectedWeaponTime );

	SSessionStats *pSessionStats = GetClientPersistantStats();
	tukk weaponName = GetItemName((IItem*) pCWeapon);
	pSessionStats->m_mapStats[EMPS_WeaponTime].Update(weaponName, usageTime);

	if( CActor* pOwner = pCWeapon->GetOwnerActor() )
	{
		if( pOwner->IsClient() )
		{
			IEntityClass* pWeaponClass = pCWeapon->GetEntity()->GetClass();
			g_pGame->GetWeaponSystem()->GetWeaponAlias().UpdateClass(&pWeaponClass);
			BLAZE_REPORT_WEAPON( pWeaponClass, pOwner, timeUsed, usageTime);
		}
	}
}

void CPersistantStats::UpdateModulesTimes()
{
}

//---------------------------------------
void CPersistantStats::GameOver(EGameOverType localWinner, bool isClientSpectator)
{
	DrxLog("CPersistantStats::GameOver()");

	SSessionStats *pSessionStats = GetClientPersistantStats();
	CGameRules* pGameRules = g_pGame->GetGameRules();
	i32 clientPosition = 0;
	i32 clientScore = 0;
	EntityId closestCompetitorId = 0;
	i32 closestCompetitorScore = 0;
	CGameLobby *pGameLobby = g_pGame->GetGameLobby();

	bool bAllowToDoStats = ( !isClientSpectator && pGameLobby->IsRankedGame() );

	if(!isClientSpectator)
	{
		GetClientScoreInfo(pGameRules, clientPosition, clientScore, closestCompetitorId, closestCompetitorScore);

		tukk gamemodeName = pGameRules->GetEntity()->GetClass()->GetName();
		if(gamemodeName)
		{
			CActor* pClientActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());

			if(pClientActor)
			{
				EntityId itemId = pClientActor->GetCurrentItemId();
				if(itemId != 0)
				{
					CWeapon* pCurrentWeapon = pClientActor->GetWeapon(itemId);
					if(pCurrentWeapon && pCurrentWeapon == m_selectedWeapon )
					{
						UpdateWeaponTime(pCurrentWeapon);
					}
				}

				UpdateModulesTimes();
				UpdateGamemodeTime(gamemodeName, pSessionStats, pClientActor);
			}
			

			if( bAllowToDoStats )
			{
				if(gamemodeName)
				{
					pSessionStats->m_mapStats[EMPS_Gamemodes].Update(gamemodeName);
				}

				tukk mapName = gEnv->pGame->GetIGameFramework()->GetLevelName();
				if(mapName)
				{
					const string& lowerMapName = DrxStringUtils::toLower(mapName);
					tukk pLowerMapName = lowerMapName.c_str();

					if( tukk pStrippedLevelName = strstr( pLowerMapName, "/") )
					{
						pLowerMapName = (pStrippedLevelName + 1);
					}

					pSessionStats->m_mapStats[EMPS_Levels].Update(pLowerMapName);
				}

				if ( IsClientMVP(pGameRules) )
				{
					pSessionStats->m_mapStats[EMPS_GamemodesMVP].Update(gamemodeName);
				}

				i32 previousHighscore = pSessionStats->m_mapStats[EMPS_GamemodesHighscore].GetStat(gamemodeName);
				if(previousHighscore < clientScore)
				{
					pSessionStats->m_mapStats[EMPS_GamemodesHighscore].SetStat(gamemodeName, clientScore);
				}

				switch (localWinner)
				{
				case EGOT_Lose:
					pSessionStats->m_mapStats[EMPS_GamesLost].Update(gamemodeName);
					pSessionStats->m_streakIntStats[ESIPS_Win].Reset();
					pSessionStats->m_streakIntStats[ESIPS_Lose].Increment();

					if (pGameLobby->IsOnlineGame() && pGameLobby->IsRankedGame())
					{
						pSessionStats->m_streakIntStats[ESIPS_OnlineRankedWin].Reset();
					}
					break;
				case EGOT_Draw:
					pSessionStats->m_mapStats[EMPS_GamesDrawn].Update(gamemodeName);
					pSessionStats->m_streakIntStats[ESIPS_Win].Reset();
					pSessionStats->m_streakIntStats[ESIPS_Lose].Reset();

					if (pGameLobby->IsOnlineGame() && pGameLobby->IsRankedGame())
					{
						pSessionStats->m_streakIntStats[ESIPS_OnlineRankedWin].Reset();
					}
					break;
				case EGOT_Win:
					{
						pSessionStats->m_mapStats[EMPS_GamesWon].Update(gamemodeName);
						pSessionStats->m_streakIntStats[ESIPS_Win].Increment();
						pSessionStats->m_streakIntStats[ESIPS_Lose].Reset();

						if (pGameLobby->IsOnlineGame() && pGameLobby->IsRankedGame())
						{
							pSessionStats->m_streakIntStats[ESIPS_OnlineRankedWin].Increment();
						}

						i32 clientTeam = pGameRules->GetTeam(gEnv->pGame->GetIGameFramework()->GetClientActorId());
						i32 opponentsTeam = 3 - clientTeam;
						i32 opponentsTeamScore = pGameRules->GetTeamsScore(opponentsTeam);

						EGameMode mode = pGameRules->GetGameMode();
						switch(mode)
						{
						case eGM_CaptureTheFlag:
							if (opponentsTeamScore == 0)
							{
								pSessionStats->m_intStats[EIPS_WonCTFWithoutGivingUpAScore]++;
							}
							break;
						}
					}
					break;
				default:
					DRX_ASSERT_MESSAGE(gEnv->IsDedicated(), "Unknown EGameOverType - stats won't track exactly");
				}
			}

		}
	}

	bool resetLateAndLow = true;

	IDrxLobby* pLobby = gEnv->pNetwork->GetLobby();
	if (pLobby != NULL)
	{
		DrxLog("CPersistantStats::GameOver() - has lobby");

		IDrxLobbyService* pLobbyService = pLobby->GetLobbyService(eCLS_Online);
		if (pLobbyService != NULL)
		{
			SDrxSystemTime today;

			DrxLog("CPersistantStats::GameOver() - has lobby service");

			if (pLobbyService->GetSystemTime(0, &today) == eCLE_Success)
			{
				DrxLog("CPersistantStats::GameOver() - managed to GetSystemTime() today.hour=%d; today.minute=%d", today.m_Hour, today.m_Minute);

				if((today.m_Hour >= 2) && (today.m_Hour <= 4))	//between 2am and 4am
				{
					pSessionStats->m_intStats[EIPS_GameOverAfter2Before4]++;
				}

				float timeToday = today.m_Hour + (today.m_Minute / 60.0f);	// previous separate checks of hours && minutes were flawed!
				DrxLog("CPersistantStats::GameOver() - timeToday=%f", timeToday);

				if (timeToday >= 23.5f || timeToday <= 5.5f)	// after 23:30 and before 05:30
				{
					DrxLog("CPersistantStats::GameOver() - timeToday=%f is in range for drunk and disorderly", timeToday);
					i32 playerCount = pGameRules->GetPlayerCount();
					i32 cutoff = (2 * playerCount) / 3;
					DrxLog("CPersistantStats::GameOver() - playerCount=%d; cutOff=%d; clientPosition=%d", playerCount, cutoff, clientPosition);
					if(clientPosition > cutoff)
					{
						DrxLog("CPersistantStats::GameOver() - WE HAVE COME IN THE BOTTOM THIRD incrementing stat playerCount=%d; cutOff=%d; clientPosition=%d", playerCount, cutoff, clientPosition);
						pSessionStats->m_streakIntStats[ESIPS_GameOverLateAndLowScore].Increment();
						resetLateAndLow = false;
						DrxLog("CPersistantStats::GameOver() ESIPS_GameOverLateAndLowScore is now %d", pSessionStats->m_streakIntStats[ESIPS_GameOverLateAndLowScore].m_curVal);
					}
				}
			}
		}
	}

	if(!isClientSpectator)
	{
		if(resetLateAndLow)
		{
			pSessionStats->m_streakIntStats[ESIPS_GameOverLateAndLowScore].Reset();
		}

		if ( pGameLobby->IsOnlineGame() )
		{
			pSessionStats->m_intStats[EIPS_CompleteOnlineMatches]++;
		}

		pSessionStats->m_streakIntStats[ESIPS_MeleeKillsThisSession].Reset();

		EnemyTeamMemberInfoMap::iterator it = m_enemyTeamMemberInfoMap.begin();
		EnemyTeamMemberInfoMap::iterator end = m_enemyTeamMemberInfoMap.end();
		for ( ; it!=end; ++it)
		{
			it->second.m_killedThisMatch=0;
			it->second.m_beenKilledByThisMatch=0;
			it->second.m_killedThisMatchNotDied=false;
			it->second.m_taggedThisMatch=false;
			it->second.m_timeOfLastKill = 0.f;
		}

		HandleLocalWinningKills();
	}

	PostGame( bAllowToDoStats );

	SetInGame(false);
}

//---------------------------------------
const bool CPersistantStats::IsClientMVP(CGameRules* pGameRules) const
{
	bool bResult = false;

	IGameRulesPlayerStatsModule* pPlayStatsMod = pGameRules->GetPlayerStatsModule();
	DRX_ASSERT(pPlayStatsMod);
	if(pPlayStatsMod)
	{
		EntityId clientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
		const SGameRulesPlayerStat* pClientStat = pPlayStatsMod->GetPlayerStats(clientId);
		if(pClientStat)
		{
			EGameMode gamemode = pGameRules->GetGameMode();
			SMVPCompare clientScore(pClientStat->kills, pClientStat->deaths, pClientStat->assists, pClientStat->points, pClientStat->gamemodePoints);
			bResult = true;

			i32k  numStats = pPlayStatsMod->GetNumPlayerStats();
			for (i32 i=0; i<numStats; i++)
			{
				const SGameRulesPlayerStat* pCurrentStat = pPlayStatsMod->GetNthPlayerStats(i);
				CPersistantStats::SMVPCompare otherScore(pCurrentStat->kills, pCurrentStat->deaths, pCurrentStat->assists, pCurrentStat->points, pCurrentStat->gamemodePoints);
				if(clientScore.CompareForMVP(gamemode, otherScore)==false)
				{
					bResult = false;
					break;
				}
			}
		}
	}

	return bResult;
}

//---------------------------------------
void CPersistantStats::PostGame( bool bAllowToDoStats )
{
	m_nearGrenadeMap.clear();
	m_afterMatchAwards.StartCalculatingAwards();

	CGameRules* pGameRules = g_pGame->GetGameRules();
	if(pGameRules)
	{
		bool		uploadDescriptions=true;

#if defined(_RELEASE)
		uploadDescriptions=false;
#endif

		SaveTelemetry(uploadDescriptions, false);// no descriptions and no writing to disk
	}

	if( bAllowToDoStats )
	{
		m_bSentStats = true;

		// We can now cache our local client's delta session stats
		u32k gamesCompleted = static_cast<u32>(m_clientPersistantStats.GetDerivedStat(EDIPS_GamesPlayed));	
	
		// Pop off the last entry if would exceed capacity
		if(gamesCompleted > NUM_HISTORY_ENTRIES)
		{
			m_clientPersistantStatHistory.pop_back(); 
		}
		m_clientPersistantStatHistory.push_front(SSessionStats());

		// configure Deltas in new history entry
		SaveLatestGameHistoryDeltas(m_clientPersistantStats, m_clientPersistantStatsAtGameStart, m_clientPersistantStatHistory.front()); 
	}
}

//---------------------------------------
void CPersistantStats::SaveLatestGameHistoryDeltas( const SSessionStats& latestSessionStats, const SSessionStats& startSessionStats, SSessionStats& outStatHistory )
{
	for(i32 i = 0; i < EIPS_Max; i++)
	{
		EIntPersistantStats stat = (EIntPersistantStats) i;
		i32 delta = latestSessionStats.GetStat(stat) - startSessionStats.GetStat(stat);
		outStatHistory.SetStat(stat,delta); 
	}

	for(i32 i = 0; i < EFPS_Max; i++)
	{
		EFloatPersistantStats stat = (EFloatPersistantStats) i;
		float curr		 = latestSessionStats.GetStat(stat);
		float previous = startSessionStats.GetStat(stat);
		float delta		 = curr - previous;
		outStatHistory.SetStat(stat,delta);
	}

	for(i32 i = 0; i < ESIPS_Max; i++)
	{
		EStreakIntPersistantStats stat = (EStreakIntPersistantStats) i;
		i32 curr		 = latestSessionStats.GetStat(stat);
		i32 previous = startSessionStats.GetStat(stat);
		i32 delta		 = curr - previous;
		outStatHistory.SetStat(stat,delta);
	}

	for(i32 i = 0; i < ESFPS_Max; i++)
	{
		EStreakFloatPersistantStats stat = (EStreakFloatPersistantStats) i;
		float curr			= latestSessionStats.GetStat(stat);
		float previous	= startSessionStats.GetStat(stat);
		float delta			= curr - previous;
		outStatHistory.SetStat(stat,delta);
	}

	for(i32 i = 0; i < EMPS_Max; i++)
	{
		i32k mapParamCount = MapParamCount(s_mapStatsFlags[i]);
		for(i32 j = 0; j < mapParamCount; j++)
		{
			tukk pParamName		= MapParamName(s_mapStatsFlags[i], j);
			EMapPersistantStats stat	= (EMapPersistantStats) i;
			i32 delta									= latestSessionStats.GetStat(pParamName, stat) - startSessionStats.GetStat(pParamName, stat);
			outStatHistory.m_mapStats->SetStat(pParamName, delta); 
		}
	}

	// XP 
	CPlayerProgression*  pp = CPlayerProgression::GetInstance();
	outStatHistory.m_xpHistoryDelta = pp->GetData(EPP_XPLastMatch); 
}

//---------------------------------------
void CPersistantStats::UnlockAll()
{
	//Unlock attachments
	for (i32 i = 0; i < s_numUnlockableAttachments; ++i)
	{
		i32k statValue = GetStat(s_unlockableAttachmentNames[i], EMPS_AttachmentUnlocked);
		if (statValue == 0)
		{
			IncrementMapStats(EMPS_AttachmentUnlocked, s_unlockableAttachmentNames[i]);
		}
	}
}

//---------------------------------------
const bool CPersistantStats::ShouldSaveClientTelemetry() const
{
	i32k gamesCompleted = m_clientPersistantStats.GetDerivedStat(EDIPS_GamesPlayed);	//Happens on GameOver
	if(gamesCompleted > 0)
	{
		i32k gamesEntered = m_clientPersistantStats.m_mapStats[EMPS_Gamemodes].GetTotal(); //Happens on EnteredGame
		float gamesCompletedFraction = (gamesEntered / (float)gamesCompleted);
		DrxLog("CPersistantStats::ShouldSaveClientTelemetry %.2f (%d/%d)", gamesCompletedFraction, gamesEntered, gamesCompleted);
		if(gamesCompletedFraction >= g_pGameCVars->g_persistantStats_gamesCompletedFractionNeeded)
		{
			return true;
		}
	}

	return false;
}

//---------------------------------------
void CPersistantStats::ClientScoreEvent(EGameRulesScoreType scoreType, i32 points, EXPReason inReason, i32 currentTeamScore)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	switch(scoreType)
	{
	case EGRST_PlayerKillAssist:
		pSessionStats->m_intStats[EIPS_KillAssists]++;
		break;
	case EGRST_CaptureObjectiveCompleted:
		pSessionStats->m_intStats[EIPS_CaptureObjectives]++;
		break;
	case EGRST_CarryObjectiveCompleted:
		{
			CGameRules* pGameRules = g_pGame->GetGameRules();
			EGameMode mode = pGameRules->GetGameMode();
			switch(mode)
			{
			case eGM_Extraction:
				{
					pSessionStats->m_intStats[EIPS_CarryObjectives]++;
				}
				break;
			case eGM_CaptureTheFlag:
				{
					pSessionStats->m_intStats[EIPS_FlagCaptures]++;

					IGameRulesRoundsModule*  pRoundsMo = pGameRules->GetRoundsModule();
					if (!pRoundsMo || (pRoundsMo->GetRoundsRemaining() == 0))
					{
						const float  timeRem = (pGameRules->IsTimeLimited() ? pGameRules->GetRemainingGameTime() : FLT_MAX);
						if (timeRem < (g_pGameCVars->g_neverFlagging_maxMatchTimeRemaining + 1.f))  // the "less than (X + 1)" business is because gametime is rounded /down/ before it's displayed, so even if the time left is actually X.99999 the game clock would show "X" and the player would therefore expect to get their award
						{
							IGameRulesScoringModule *pScoringModule = pGameRules->GetScoringModule();
							if (pScoringModule)
							{
								i32k numPointsForCapture = pScoringModule->GetTeamPointsByType((EGRST)scoreType);

								i32k  clientTeam = pGameRules->GetTeam(gEnv->pGame->GetIGameFramework()->GetClientActorId());

								i32k  otherTeam = (3 - clientTeam);
								DRX_ASSERT((otherTeam == 1) || (otherTeam == 2));

								i32k otherTeamScore = pGameRules->GetTeamsScore(otherTeam);

								if (((currentTeamScore - numPointsForCapture) <= otherTeamScore) && (currentTeamScore > otherTeamScore))  // ie. the points scored for this flag capture have put us into the lead
								{
									pSessionStats->m_intStats[EIPS_TakeLateFlagCaptureLead]++;
								}
							}
						}
					}
				}
				break;
			default:
				DRX_ASSERT(0);
				break;
			}		
		}
		break;
	case EGRST_CarryObjectiveCarrierKilled:
	{
		CGameRules* pGameRules = g_pGame->GetGameRules();
		EGameMode mode = pGameRules->GetGameMode();
		switch(mode)
		{
			case eGM_CaptureTheFlag:
				pSessionStats->m_intStats[EIPS_FlagCarrierKills]++;
				break;
		}
		break;
	}
	case EGRST_BombTheBaseCompleted:
		pSessionStats->m_intStats[EIPS_BombTheBase]++;
		break;
	case EGRST_Tagged_PlayerKillAssist:
		{
			pSessionStats->m_intStats[EIPS_TagAssist]++;
		}
		break;
	case EGRST_HoldObj_OffensiveKill:
	{
		CGameRules* pGameRules = g_pGame->GetGameRules();
		EGameMode mode = pGameRules->GetGameMode();

		if (mode == eGM_CrashSite)
		{
			pSessionStats->m_intStats[EIPS_CrashSiteAttackingKills]++;
		}
		break;
	}
	case EGRST_HoldObj_DefensiveKill:
	{
		CGameRules* pGameRules = g_pGame->GetGameRules();
		EGameMode mode = pGameRules->GetGameMode();
		if (mode == eGM_CrashSite)
		{
			pSessionStats->m_intStats[EIPS_CrashSiteDefendingKills]++;
		}
		break;
	}
	case EGRST_CombiCapObj_Def_PlayerKill:
	{
		CGameRules* pGameRules = g_pGame->GetGameRules();
		EGameMode mode = pGameRules->GetGameMode();
		if (mode == eGM_Assault)
		{
			pSessionStats->m_intStats[EIPS_AssaultDefendingKills]++;
		}
		break;
	}
	case EGRST_Extraction_DefendingKill:
	{
		CGameRules* pGameRules = g_pGame->GetGameRules();
		EGameMode mode = pGameRules->GetGameMode();
		if (mode == eGM_Extraction)
		{
			pSessionStats->m_intStats[EIPS_ExtractionDefendingKills]++;
		}
		break;
	}
	}
}

//---------------------------------------
void CPersistantStats::OnRoundEnd()
{
	CGameRules* pGameRules = g_pGame->GetGameRules();
	EGameMode mode = pGameRules->GetGameMode();
	SSessionStats *pSessionStats = GetClientPersistantStats();
	i32 clientTeam = pGameRules->GetTeam(gEnv->pGame->GetIGameFramework()->GetClientActorId());
	i32 opponentsTeam = 3 - clientTeam;

	switch (mode)
	{
		case eGM_Assault:
		{
			i32 opponentCount = pGameRules->GetTeamPlayerCountWhoHaveSpawned(opponentsTeam);
			if(opponentCount != 0 && pSessionStats->m_streakIntStats[ESIPS_AssaultAttackersKilled].m_curVal >= opponentCount)
			{
				pSessionStats->m_intStats[EIPS_KillAllAssaultAttackers]++;
			}

			pSessionStats->m_streakIntStats[ESIPS_AssaultAttackersKilled].Reset();
			break;
		}
		case eGM_Extraction:
		{
			IGameRulesRoundsModule *pRoundsModule=pGameRules->GetRoundsModule();
			DRX_ASSERT(pRoundsModule);
			if (pRoundsModule)
			{
				i32 previousRoundWinnerTeamId = pRoundsModule->GetPreviousRoundWinnerTeamId();
			
				if (previousRoundWinnerTeamId == clientTeam)
				{
					i32k *previousRoundsTeamScores = pRoundsModule->GetPreviousRoundTeamScores();
					assert(previousRoundsTeamScores);
					if (previousRoundsTeamScores)
					{
						i32 opponentsTeamScore = previousRoundsTeamScores[opponentsTeam-1];
						
						if (opponentsTeamScore == 0)
						{
							i32  primaryTeam = pRoundsModule->GetPrimaryTeam();	// attacking
							i32 secondaryTeam = 3 - primaryTeam; // defending

							if (clientTeam == secondaryTeam)
							{
								pSessionStats->m_intStats[EIPS_WonExtractDefendingNoGiveUp]++;
							}
						}
					}
				}
			}

			break;
		}
	}

	UpdateModulesTimes();
}

//---------------------------------------
bool CPersistantStats::ShouldProcessOnEntityKilled(const HitInfo &hitInfo)
{
#if USE_PC_PREMATCH
	if(CGameRules * pGameRules = g_pGame->GetGameRules())
	{
		if(pGameRules->HasGameActuallyStarted() == false)
			return false;
	}
#endif

	if(hitInfo.shooterId != 0 && hitInfo.targetId != 0)
	{
		IActor* pTargetActor =gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(hitInfo.targetId);
		return pTargetActor != NULL;
	}

	return false;
}

//---------------------------------------
void CPersistantStats::OnEntityKilled(const HitInfo &hitInfo)
{
	if(ShouldProcessOnEntityKilled(hitInfo))
	{
		CGameLobby *pGameLobby = g_pGame->GetGameLobby();

		SSessionStats* pShooterStats = GetActorSessionStats(hitInfo.shooterId);
		if(pShooterStats)
		{
			CGameRules* pGameRules = g_pGame->GetGameRules();
			
			i32k shooterTeam = pGameRules->GetTeam(hitInfo.shooterId);

			if(hitInfo.shooterId == hitInfo.targetId)
			{
				pShooterStats->m_intStats[EIPS_Suicides]++;
				pShooterStats->m_streakIntStats[ESIPS_Kills].Reset();
				pShooterStats->m_streakIntStats[ESIPS_KillsNoReloadWeapChange].Reset();
				pShooterStats->m_streakIntStats[ESIPS_Deaths].Increment();
	
				if(hitInfo.type == CGameRules::EHitType::PunishFall || hitInfo.type == CGameRules::EHitType::Fall)
				{
					pShooterStats->m_intStats[EIPS_SuicidesByFalling]++;
				}
				else if(hitInfo.type == CGameRules::EHitType::Frag)
				{
					pShooterStats->m_intStats[EIPS_SuicidesByFrag]++;
				}
			}
			else
			{
				CPlayer* pShooterPlayer = static_cast<CPlayer*>(pGameRules->GetActorByEntityId(hitInfo.shooterId));
				CActor* pTargetActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(hitInfo.targetId));

				// Is this an enemy
				bool enemy = true;
				if(pTargetActor != NULL && pShooterPlayer != NULL)
				{
					if(!gEnv->bMultiplayer)
					{
						// check AI faction of the target, and only count enemies
						IAIObject* pShooterAI = pShooterPlayer->GetEntity()->GetAI();
						IAIObject* pTargetAI = pTargetActor->GetEntity()->GetAI();
						if (pShooterAI != NULL && pTargetAI != NULL)
						{
							enemy = (gEnv->pAISystem->GetFactionMap().GetReaction(pShooterAI->GetFactionID(), pTargetAI->GetFactionID()) == IFactionMap::Hostile);
						}
					}
					else
					{
						enemy = !pTargetActor->IsFriendlyEntity(hitInfo.shooterId);
					}
				}

				const bool meleeHit = IsMeleeAttack(hitInfo);
				if(meleeHit)
				{
					if(enemy && pTargetActor)
					{
						pShooterStats->m_mapStats[EMPS_WeaponKills].Update(MELEE_WEAPON_NAME);
						pShooterStats->m_streakIntStats[ESIPS_MeleeKillsThisSession].Increment();

						if(pTargetActor->IsHeadShot(hitInfo))
						{
							pShooterStats->m_mapStats[EMPS_WeaponHeadshotKills].Update(MELEE_WEAPON_NAME);
						}
					}
				}
				else if(enemy && hitInfo.type == CGameRules::EHitType::EventDamage)
				{
					tukk mapName = gEnv->pGame->GetIGameFramework()->GetLevelName();
					if(mapName)
					{
						const string& lowerMapName = DrxStringUtils::toLower(mapName);
						tukk pLowerMapName = lowerMapName.c_str();
						if( tukk pStrippedLevelName = strstr( pLowerMapName, "/") )
						{
							pLowerMapName = (pStrippedLevelName+1);
						}
						pShooterStats->m_mapStats[EMPS_LevelsInteractiveKills].Update(pLowerMapName);
					}
				}
				else if (enemy)
				{
					tukk weaponName = GetItemName(hitInfo.weaponId);
					if(weaponName)
					{
						DrxFixedStringT<64> fullWeaponName;
						tukk killWeaponName = weaponName;
						if (hitInfo.type == CGameRules::EHitType::EnvironmentalThrow)
						{
							// Thrown environmental weapons are treated separately for kills
							fullWeaponName.Format("%s" ENV_WEAPON_THROWN, weaponName);
							killWeaponName = fullWeaponName.c_str();

							const CEnvironmentalWeapon* pEnvWeap = static_cast<const CEnvironmentalWeapon*>(g_pGame->GetIGameFramework()->QueryGameObjectExtension(hitInfo.weaponId, "EnvironmentalWeapon"));	
							if(pEnvWeap)
							{
								const float throwDistSquared = hitInfo.pos.GetSquaredDistance(pEnvWeap->GetInitialThrowPos());
								if(throwDistSquared >= k_longDistanceThrowKillMinDistanceSquared)
								{
									pShooterStats->m_intStats[EIPS_ThrownObjectDistantKills]++;
								}
							}
						}

						pShooterStats->m_mapStats[EMPS_WeaponKills].Update(killWeaponName);
					}
					else
					{
						// if running over another player, the vehicle entity id is passed as the weapon id
						if(g_pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle(hitInfo.weaponId))
							pShooterStats->m_intStats[EIPS_RunOver]++;
					}

					if(pTargetActor)
					{
						if(pTargetActor->IsHeadShot(hitInfo))
						{
							if(weaponName)
							{
								pShooterStats->m_mapStats[EMPS_WeaponHeadshotKills].Update(weaponName);
							}

							pShooterStats->m_streakIntStats[ESIPS_HeadshotKillsPerLife].Increment();
							pShooterStats->m_streakIntStats[ESIPS_HeadshotKillsPerMatch].Increment();

							SActorStats* pTargetStats = pTargetActor->GetActorStats();
							if ( pTargetStats != NULL && pTargetStats->inAir > k_actorStats_inAirMinimum )
							{
								pShooterStats->m_intStats[EIPS_AirHeadshots]++;
							}
						}
						if(pShooterPlayer && pShooterPlayer->IsClient() )
						{							
							IItem *pItem = gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(hitInfo.weaponId);
							if(pItem)
							{
								CWeapon* pWeapon = static_cast<CWeapon*>(pItem->GetIWeapon());
								if( pWeapon )
								{
									const bool isZoomed = pWeapon->IsZoomed();
									const bool isFootShot = pTargetActor->IsFootShot(hitInfo);
									const IItemSystem* pItemSystem = gEnv->pGame->GetIGameFramework()->GetIItemSystem();
									const CItem::TAccessoryArray& accessoryArray = pWeapon->GetAccessories();
									i32k accesssoryCount = accessoryArray.size();
									for(i32 i = 0; i < accesssoryCount; i++)
									{
										if(isZoomed)
										{											
											if(isFootShot)
											{
												CItem* pAccessoryItem = static_cast<CItem*>(pItemSystem->GetItem(accessoryArray[i].accessoryId));
												if(pAccessoryItem && pAccessoryItem->GetParams().scopeAttachment)
												{
													pShooterStats->m_intStats[EIPS_SnipedFoot]++;
												}
											}
										}
									}	//for accessory

									if(pWeapon->GetOriginalOwnerId() == pTargetActor->GetEntityId())
									{
										pShooterStats->m_intStats[EIPS_PlayersKilledWithTheirGun]++;
									}
								}
							}
						}
					}
				}
				
				if (enemy)
				{
					pShooterStats->m_streakIntStats[ESIPS_Kills].Increment();
					pShooterStats->m_streakIntStats[ESIPS_KillsNoReloadWeapChange].Increment();
					pShooterStats->m_streakIntStats[ESIPS_Deaths].Reset();		

					if(pTargetActor)
					{
						// Increment the EnemyKilled stat.
						char targetId[32];
						drx_sprintf(targetId, "%d", hitInfo.targetId);
						pShooterStats->m_mapStats[EMPS_EnemyKilled].Update(targetId);
					}
				}

				CVTOLVehicleUpr* pVTOLVehicleUpr = pGameRules->GetVTOLVehicleUpr();
				if (pVTOLVehicleUpr && pVTOLVehicleUpr->IsPlayerInVTOL(hitInfo.targetId))
				{
					pShooterStats->m_intStats[EIPS_PlayersInVTOLKilled]++; 
				}
				
				tukk name = pGameRules->GetHitType(hitInfo.type);
				if(name && enemy)
				{
					pShooterStats->m_mapStats[EMPS_KillsByDamageType].Update(name);

					if(pShooterPlayer != NULL && pShooterPlayer->IsClient())
					{
						//assault mode
						EGameMode mode = pGameRules->GetGameMode();
						if(mode == eGM_Assault)
						{
							IGameRulesRoundsModule* pRoundsModule = g_pGame->GetGameRules()->GetRoundsModule();
							if (pRoundsModule)
							{
								//if defender
								i32 clientTeam = pGameRules->GetTeam(gEnv->pGame->GetIGameFramework()->GetClientActorId());
								if(pRoundsModule->GetPrimaryTeam() != clientTeam)
								{
									pShooterStats->m_streakIntStats[ESIPS_AssaultAttackersKilled].Increment();
								}
							}
						}
						else if(mode == eGM_Gladiator && pGameLobby && !pGameLobby->IsPrivateGame())
						{
							i32k targetTeam = pGameRules->GetTeam(hitInfo.targetId);
							if(shooterTeam == CGameRulesObjective_Predator::TEAM_SOLDIER && targetTeam == CGameRulesObjective_Predator::TEAM_PREDATOR)
							{
								if(++pShooterStats->m_intStats[EIPS_HunterKillsAsMarine] >= k_noMoreMerryMenHunterKillsNeeded)
								{
									g_pGame->GetGameAchievements()->GiveAchievement(eC3A_MP_No_More_Merry_Men);
								}
							}
							else if(shooterTeam == CGameRulesObjective_Predator::TEAM_PREDATOR && targetTeam == CGameRulesObjective_Predator::TEAM_SOLDIER)
							{
								pShooterStats->m_intStats[EIPS_MarineKillsAsHunter]++;
							}
						}

						const bool crouched = pShooterPlayer->GetStance() == STANCE_CROUCH;
						if(crouched)
						{
							pShooterStats->m_intStats[EIPS_CrouchedKills]++;
						}
						
						if(hitInfo.type == CGameRules::EHitType::Melee)
						{
							if(crouched)
							{
								pShooterStats->m_intStats[EIPS_CrouchedMeleeKills]++;
							}
							
							CWeapon *shooterWeapon = pShooterPlayer->GetWeapon(hitInfo.weaponId);
							DRX_ASSERT_MESSAGE(shooterWeapon, "OnEntityKilled() failed to get shooter's weapon");
							if (shooterWeapon)
							{
								if (shooterWeapon->OutOfAmmo(false))
								{
									pShooterStats->m_intStats[EIPS_MeleeTakeDownKillsNoAmmo]++;
								}
							}

						}
						
						if ( NearFriendly(pShooterPlayer, sqr(15.0f)) )
						{
							pShooterStats->m_intStats[EIPS_SafetyInNumbersKills]++;
						}
						else
						{
							pShooterStats->m_intStats[EIPS_LoneWolfKills]++;
						}

						IItem * pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(hitInfo.weaponId);
						if ( pItem )
						{
							CWeapon * pWeapon = static_cast<CWeapon*>(pItem->GetIWeapon());

							// IsMountable() seems to return true for things like the SCAR!
							if ( pWeapon != NULL && pWeapon->IsMountable() )
							{
								if ( pWeapon->IsMounted() )
								{
									pShooterStats->m_intStats[EIPS_MountedKills]++;

									tukk weaponName = GetItemName(pItem);
									if(weaponName)
									{
										if( stricmp( weaponName, "VTOLHMG" ) == 0 )
										{
											if(++pShooterStats->m_intStats[EIPS_MountedVTOLKills] >= k_20MetreHighClubVTOLHMGKillsNeeded)
											{
												g_pGame->GetGameAchievements()->GiveAchievement(eC3A_MP_20_Metre_High_Club);
											}
										}
									}
								}
								else if (pWeapon->IsRippedOff())
								{
									pShooterStats->m_intStats[EIPS_UnmountedKills]++;
								}
							}
						}

						if(pTargetActor)
						{
							const float currentTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
							float targetUncloakTime = pTargetActor->GetLastUnCloakTime();

							if ((currentTime - targetUncloakTime) < k_cloakedVictimTimeFromCloak)
							{
								pShooterStats->m_intStats[EIPS_NumCloakedVictimKills]++;
							}
						}
					}

					if(pTargetActor != NULL && pShooterPlayer != NULL)
					{
						if(enemy && pTargetActor->IsHeadShot(hitInfo))
						{
							pShooterStats->m_streakIntStats[ESIPS_Headshots].Increment();
						}
						else
						{
							pShooterStats->m_streakIntStats[ESIPS_Headshots].Reset();
						}

						if (pShooterPlayer->IsClient())
						{
							float dist2killed = (pTargetActor->GetEntity()->GetWorldPos() - pShooterPlayer->GetEntity()->GetWorldPos()).len2();
							const float currentTime = gEnv->pTimer->GetCurrTime();
							float clientPlayerUncloakTime = pShooterPlayer->GetLastUnCloakTime();
							if ( (currentTime - clientPlayerUncloakTime) < (float)k_warbirdTimeFromCloak)
							{
								pShooterStats->m_intStats[EIPS_WarBirdKills]++;
							}

							CWeapon * pWeapon = pShooterPlayer->GetWeapon(hitInfo.weaponId);
							if(pWeapon)
							{
								if(pWeapon->IsHeavyWeapon())
								{
									pShooterStats->m_intStats[EIPS_HeavyWeaponKills]++;
								}
							}
							
							if ( hitInfo.type == CGameRules::EHitType::Explosion || hitInfo.type == CGameRules::EHitType::Frag )
							{
								char projectile[32];
								drx_sprintf(projectile, "%d", hitInfo.projectileId);
								pShooterStats->m_mapStats[EMPS_KillsFromExplosion].Update(projectile);
							}

							if ( hitInfo.penetrationCount > 0 )
							{
								pShooterStats->m_intStats[EIPS_BulletPenetrationKills]++;
							}

							if(pTargetActor->IsPlayer())
							{
								CPlayer* pTargetPlayer = static_cast<CPlayer*>(pTargetActor);

								if (SkillKill::IsAirDeath(pTargetPlayer))
								{
									pShooterStats->m_intStats[EIPS_AirKillKills]++;
								}

								OnEntityKilledCheckSpecificMultiKills(hitInfo, pShooterPlayer);
							}

							if ( hitInfo.type == CGameRules::EHitType::Frag )
							{
								if (m_grenadeKills.size() < m_grenadeKills.max_size()) 
								{
									i32 len=m_grenadeKills.size();
									bool addNewKill=true;

									for (i32 i=0; i<len; i++)					
									{
										if (m_grenadeKills[i].m_victimId == hitInfo.targetId)
										{
											addNewKill=false;
										}
									}

									if (addNewKill)
									{
										if (m_grenadeKills.size() == 2)
										{
											pShooterStats->m_intStats[EIPS_3FastGrenadeKills]++;
											m_grenadeKills.clear();	// clear out the already acknowledged grenade kills. This stat is a got once award so we dont need to be too worried about any further calculations
										}
										else
										{
											SGrenadeKillElement ele(hitInfo.targetId, currentTime);
											m_grenadeKills.push_back(ele);
										}
									}
								}
								
								SActorStats* pTargetStats = pTargetActor->GetActorStats();
								if (pTargetStats != NULL && pTargetStats->inAir > k_actorStats_inAirMinimum )
								{
									pShooterStats->m_intStats[EIPS_InAirGrenadeKills]++;
								}
							}

							if(pShooterPlayer->IsSliding())
							{
								pShooterStats->m_intStats[EIPS_SlidingKills]++;
							}

							SEnemyTeamMemberInfo *enemyTeamMemberInfo = GetEnemyTeamMemberInfo(hitInfo.targetId);
							DRX_ASSERT(enemyTeamMemberInfo);
							if (enemyTeamMemberInfo)
							{
								CActorUpr *pActorUpr = CActorUpr::GetActorUpr();
								pActorUpr->PrepareForIteration();

								enemyTeamMemberInfo->m_killedThisMatch++;

								enemyTeamMemberInfo->m_killedThisMatchNotDied=true;
								enemyTeamMemberInfo->m_teabaggedThisDeath=false;

								const float fFrameTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
								enemyTeamMemberInfo->m_timeOfLastKill = fFrameTime;

								if (enemyTeamMemberInfo->m_state == SEnemyTeamMemberInfo::k_state_tagged)
								{
									enemyTeamMemberInfo->m_state = SEnemyTeamMemberInfo::k_state_killed;
									enemyTeamMemberInfo->m_timeEnteredKilledState=fFrameTime;
								}

								i32k kNumActors = pActorUpr->GetNumActors();

								EnemyTeamMemberInfoMap::const_iterator end = m_enemyTeamMemberInfoMap.end();

								i32 numKilledThisMatch=0;
								i32 numKilledThisMatchNotDied=0;
								i32 numOnEnemyTeam=0;
								bool killedAllEnemyTeamInXSeconds = true;
								for (i32 i=0; i<kNumActors; i++)
								{
									SActorData actorData;
									pActorUpr->GetNthActorData(i, actorData);
									if (actorData.teamId != shooterTeam)
									{
										numOnEnemyTeam++;

										EnemyTeamMemberInfoMap::const_iterator it = m_enemyTeamMemberInfoMap.find(actorData.entityId);
										if(it != end)
										{
											if (it->second.m_killedThisMatch)
											{
												numKilledThisMatch++;
											}
											if (it->second.m_killedThisMatchNotDied)
											{
												numKilledThisMatchNotDied++;
											}

											if(killedAllEnemyTeamInXSeconds && (it->second.m_timeOfLastKill == 0.f || fFrameTime - it->second.m_timeOfLastKill > kTimeAllowedToKillEntireEnemyTeam))
											{
												killedAllEnemyTeamInXSeconds = false;
											}
										}
									}
								}

								if(killedAllEnemyTeamInXSeconds && numOnEnemyTeam >= kNumEnemyRequiredForAwards)
								{
									pShooterStats->m_intStats[EIPS_EnemyTeamKilledInXSeconds]++;

									//Reset
									EnemyTeamMemberInfoMap::iterator enemyIter = m_enemyTeamMemberInfoMap.begin();
									while(enemyIter != end)
									{
										enemyIter->second.m_timeOfLastKill = 0.f;
										++enemyIter;
									}
								}

								if (numKilledThisMatch == numOnEnemyTeam && numOnEnemyTeam >= kNumEnemyRequiredForAwards)
								{
									pShooterStats->m_intStats[EIPS_KilledAllEnemies]++;
								}
								if (numKilledThisMatchNotDied == numOnEnemyTeam && numOnEnemyTeam >= kNumEnemyRequiredForAwards)
								{
									pShooterStats->m_intStats[EIPS_KilledAllEnemiesNotDied]++;								
								}
							}
						}
						else if (pTargetActor->IsClient())
						{
							SEnemyTeamMemberInfo *enemyShooterTeamMemberInfo = GetEnemyTeamMemberInfo(hitInfo.shooterId);
							DRX_ASSERT(enemyShooterTeamMemberInfo);
							if (enemyShooterTeamMemberInfo)
							{
								enemyShooterTeamMemberInfo->m_beenKilledByThisMatch++;
							}
						}
					}
				}

				CActor* pClientActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
				if(pClientActor)
				{
					//we're not the shooter or the victim
					EntityId clientId = pClientActor->GetEntityId();
					if(hitInfo.shooterId != clientId && hitInfo.targetId != clientId)
					{
						//we're cloaked
						if(pClientActor->IsCloaked())
						{
							//victim is an enemy
							if(!pClientActor->IsFriendlyEntity(hitInfo.targetId))
							{
								//Can see the victim
								if(g_pGame->GetPlayerVisTable()->CanLocalPlayerSee(hitInfo.targetId, 10))
								{
									//within 8 meters
									const float range2 = 64;
									IEntity* pTargetEntity = pTargetActor->GetEntity();
									IEntity* pClientEntity = pClientActor->GetEntity();
									Vec3 dir = pTargetEntity->GetWorldPos() - pClientEntity->GetWorldPos();
									float distance2 = dir.len2();
									if ( distance2 < range2 )
									{
										//facing the event
										float dot = pClientEntity->GetForwardDir().Dot(dir.normalized());
										if(dot > 0.7f)
										{
											SSessionStats *pSessionStats = GetClientPersistantStats();
											pSessionStats->m_intStats[EIPS_CloakedWatchNearbyKill]++;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		CGameRules* pGameRules = g_pGame->GetGameRules();
		IActor* pClientActor = pGameRules->GetActorByEntityId(hitInfo.targetId);
		if(pClientActor != NULL && pClientActor->IsClient())
		{
			m_nearGrenadeMap.clear();
			m_previousWeaponHitMap.clear();

			EnemyTeamMemberInfoMap::iterator it = m_enemyTeamMemberInfoMap.begin();
			EnemyTeamMemberInfoMap::iterator end = m_enemyTeamMemberInfoMap.end();

			for ( ; it!=end; ++it)
			{
				it->second.m_killedThisMatchNotDied=false;
				it->second.m_state = SEnemyTeamMemberInfo::k_state_initial;
			}

			ClientDied(static_cast<CPlayer*>(pClientActor));
		}

		//remove them when they die
		m_previousWeaponHitMap.erase(hitInfo.targetId);

		SSessionStats* pTargetStats = GetActorSessionStats(hitInfo.targetId);
		if(pTargetStats)
		{
			pTargetStats->m_streakIntStats[ESIPS_Kills].Reset();
			pTargetStats->m_streakIntStats[ESIPS_HealthRestoresPerLife].Reset();
			pTargetStats->m_streakIntStats[ESIPS_Deaths].Increment();

			CPlayer* pShooterPlayer = static_cast<CPlayer*>(pGameRules->GetActorByEntityId(hitInfo.shooterId));
			CPlayer* pTargetPlayer = static_cast<CPlayer*>(pGameRules->GetActorByEntityId(hitInfo.targetId));
			if ( pTargetPlayer != NULL && pTargetPlayer->IsClient() )
			{
				if ( SkillKill::IsAirDeath(pTargetPlayer) )
				{
					pTargetStats->m_intStats[EIPS_InAirDeaths]++;
				}

				if(hitInfo.type == CGameRules::EHitType::Melee)
				{
					pTargetStats->m_intStats[EIPS_MeleeDeaths]++;
				}

				m_retaliationTargetId = (hitInfo.shooterId != hitInfo.targetId) ? hitInfo.shooterId : 0;
			}
			else if (pShooterPlayer && pShooterPlayer->IsClient())
			{
				// test this and ensure that you can still get retaliation on the first kill after being killed.. stacks look like it should work
				m_retaliationTargetId = 0;	// you've killed someone and if it's not been rewarded as a retaliation then we need to clear it now
			}
		}
	}   
}

//---------------------------------------
void CPersistantStats::OnEntityKilledCheckSpecificMultiKills( const HitInfo &hitInfo, CPlayer* pShooterPlayer )
{
	bool bEnvironmental = false;
	bool bIsPole = false;
	if(hitInfo.type == CGameRules::EHitType::EnvironmentalMelee ||
		hitInfo.type == CGameRules::EHitType::EnvironmentalThrow)
	{
		EntityId weaponId = hitInfo.weaponId;
		CEnvironmentalWeapon* pEnvWeap = static_cast<CEnvironmentalWeapon*>(g_pGame->GetIGameFramework()->QueryGameObjectExtension(weaponId, "EnvironmentalWeapon"));
		if(pEnvWeap)
		{
			bEnvironmental = true;
			bIsPole = (strcmpi(pEnvWeap->GetClassificationName(), "pole") == 0);
		}
	}

	EPPType multiKillType = SkillKill::IsMultiKill(pShooterPlayer);
	if(multiKillType != EPP_Invalid)
	{
		if(bEnvironmental)
		{
			EntityId shooterId = pShooterPlayer->GetEntityId();
			if(hitInfo.type == CGameRules::EHitType::EnvironmentalThrow)
			{
				if(CheckPreviousKills(1, CGameRules::EHitType::EnvironmentalThrow, false, false))
				{
					IncrementStatsForActor( shooterId, EIPS_ThrownObjectDoubleKills );
				}
			}

			if((multiKillType != EPP_DoubleKill) && CheckPreviousKills(2, CGameRules::EHitType::Invalid, true, bIsPole))
			{
				IncrementStatsForActor( shooterId, bIsPole ? EIPS_TripleKillswithPole : EIPS_TripleKillswithPanel );
			}
			else if(CheckPreviousKills(1, CGameRules::EHitType::Invalid, true, bIsPole))
			{
				IncrementStatsForActor( shooterId, bIsPole ? EIPS_DoubleKillsWithPole : EIPS_DoubleKillsWithPanel );
			}
		}
		else if(hitInfo.type == CGameRules::EHitType::Stamp)
		{
			EntityId shooterId = pShooterPlayer->GetEntityId();
			if((multiKillType != EPP_DoubleKill) && CheckPreviousKills(2, CGameRules::EHitType::Stamp, false, false))
			{
				IncrementStatsForActor( shooterId, EIPS_AirStompTripleKills );
			}
			else if(CheckPreviousKills(1, CGameRules::EHitType::Stamp, false, false))
			{
				IncrementStatsForActor( shooterId, EIPS_AirStompDoubleKills );
			}
		}
	}

	if(m_clientPreviousKillData.size() >= k_previousKillsToTrack)
	{
		m_clientPreviousKillData.pop_back();
	}
	m_clientPreviousKillData.push_front(SPreviousKillData(hitInfo.type, bEnvironmental, bIsPole));
}

//---------------------------------------
bool CPersistantStats::CheckPreviousKills(u32 killsToCheck, i32 desiredHitType, bool bEnvironmental, bool bIsPole) const
{
	DRX_ASSERT(killsToCheck <= k_previousKillsToTrack);

	if(m_clientPreviousKillData.size() >= killsToCheck)
	{
		for(u32 i = 0; i < killsToCheck; i++)
		{
			const SPreviousKillData& data = m_clientPreviousKillData[i];
			if((data.m_hitType == desiredHitType || desiredHitType == CGameRules::EHitType::Invalid) && (data.m_bEnvironmental == bEnvironmental && data.m_bWasPole == bIsPole))
			{
				continue;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	//don't have enough data - can't be true
	return false;
}

//---------------------------------------
bool CPersistantStats::CheckRetaliationKillTarget(EntityId victimId)
{
	bool retaliation = (victimId == m_retaliationTargetId);
	
	m_retaliationTargetId = 0;

	return retaliation;
}

//---------------------------------------
void CPersistantStats::UpdateMultiKillStreak(EntityId shooterId, EntityId targetId)
{
	SSessionStats* pShooterStats = GetActorSessionStats(shooterId);
	if(pShooterStats)
	{
		CActor* pShooter = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(shooterId));
		if(!pShooter->IsFriendlyEntity(targetId))
		{
			float currentTime = gEnv->pTimer->GetCurrTime();
			if(currentTime < pShooterStats->m_lastKillTime + g_pGameCVars->g_multikillTimeBetweenKills)
			{
				//include the original kill in the streak
				if(pShooterStats->m_streakIntStats[ESIPS_MultiKillStreak].m_curVal == 0)
				{
					pShooterStats->m_streakIntStats[ESIPS_MultiKillStreak].Increment();
				}

				//current kill in streak
				pShooterStats->m_streakIntStats[ESIPS_MultiKillStreak].Increment();
			}
			else
			{
				pShooterStats->m_streakIntStats[ESIPS_MultiKillStreak].Reset();
			}
			pShooterStats->m_lastKillTime = currentTime;
		}
	}
}

//---------------------------------------
void CPersistantStats::OnSetActorItem(IActor *pActor, IItem *pItem )
{
	if( pItem && pActor && pActor->IsPlayer() )
	{
		IWeapon* pWeapon=pItem->GetIWeapon();
		if(pWeapon)
		{
			EntityId actorId = pActor->GetEntityId();
			EntityId weaponId = pItem->GetEntityId();
			SetNewWeaponListener(pWeapon, weaponId, actorId);
			tukk weaponName = GetItemName(weaponId);
			if(weaponName)
			{
				GetActorSessionStats(actorId)->m_mapStats[EMPS_WeaponUsage].Update(weaponName);
			}
			
			if( pActor->IsClient() )
			{
				if( IEntityClass* pWeaponClass = pItem->GetEntity()->GetClass() )
				{
					g_pGame->GetWeaponSystem()->GetWeaponAlias().UpdateClass(&pWeaponClass);
					BLAZE_REPORT_WEAPON(pWeaponClass, pActor, usageCount, 1);
				}
			}
		}
	}
}

//---------------------------------------
void CPersistantStats::SetNewWeaponListener(IWeapon* pWeapon, EntityId weaponId, EntityId actorId)
{
	ActorWeaponListenerMap::iterator it = m_actorWeaponListener.find(actorId);
	if(it != m_actorWeaponListener.end())
	{
		//remove previous weapon listener for actor
		RemoveWeaponListener(it->second);
		//update with new weapon
		it->second = weaponId;
	}
	else
	{
		//aren't listener so add actor and weapon
		m_actorWeaponListener.insert(ActorWeaponListenerMap::value_type(actorId, weaponId));
	}
	
	pWeapon->AddEventListener(this, "CPersistantStats");
	
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	if (actorId == localClientId)
	{
		SSessionStats *pSessionStats = GetClientPersistantStats();
		pSessionStats->m_streakIntStats[ESIPS_KillsNoReloadWeapChange].Reset();
	}
}

//---------------------------------------
void CPersistantStats::RemoveWeaponListener(EntityId weaponId)
{
	IItem* pItem = gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(weaponId);
	if(pItem)
	{
		IWeapon *pWeapon = pItem->GetIWeapon();
		if(pWeapon)
		{
			pWeapon->RemoveEventListener(this);
		}
	}
}

//---------------------------------------
void CPersistantStats::RemoveAllWeaponListeners()
{
	ActorWeaponListenerMap::const_iterator it = m_actorWeaponListener.begin();
	ActorWeaponListenerMap::const_iterator end = m_actorWeaponListener.end();
	for ( ; it!=end; ++it)
	{
		RemoveWeaponListener(it->second);
	}

	m_actorWeaponListener.clear();
}

//---------------------------------------
void CPersistantStats::OnShoot(IWeapon *pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3 &pos, const Vec3 &dir, const Vec3 &vel)
{
	m_lastFiredTime = gEnv->pTimer->GetCurrTime();

	if( shooterId != 0 )
	{
		i32 ammoCost = 1;
		CProjectile *pProjectile = g_pGame->GetWeaponSystem()->GetProjectile(ammoId);
		if (pProjectile)
		{
			ammoCost = pProjectile->GetAmmoCost();
		}

		CWeapon *pWeaponImpl = static_cast<CWeapon*>(pWeapon);
		tukk weaponName = GetItemName(pWeaponImpl);
		SSessionStats* pSessionStats = GetActorSessionStats(shooterId);
		if (pSessionStats)
		{
			pSessionStats->m_mapStats[EMPS_WeaponShots].Update(weaponName, ammoCost);

			EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
			if (shooterId == localClientId)
			{
				IFireMode* pFireMode = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());
				if(pFireMode && pFireMode->IsSilenced())
				{
					pSessionStats->m_mapStats[EMPS_WeaponShotsSilenced].Update(weaponName, ammoCost);
				}
			}
		}

		if( shooterId == g_pGame->GetClientActorId() )
		{
			IActor *pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(shooterId);
			IEntityClass* pClass = pWeaponImpl->GetEntity()->GetClass();
			g_pGame->GetWeaponSystem()->GetWeaponAlias().UpdateClass(&pClass);
			BLAZE_REPORT_WEAPON(pClass, pActor, shots, ammoCost);
			IFireMode *pFM = pWeapon->GetFireMode(pWeapon->GetCurrentFireMode());
			if (pFM && pFM->IsSilenced())
			{
				BLAZE_REPORT_WEAPON(pClass, pActor, silencedShots, ammoCost);
			}
		}
	}
}

//---------------------------------------
void CPersistantStats::OnMelee(IWeapon* pWeapon, EntityId shooterId)
{
	if( shooterId != 0 )
	{
		SSessionStats* pSessionStats = GetActorSessionStats(shooterId);
		pSessionStats->m_mapStats[EMPS_WeaponShots].Update(MELEE_WEAPON_NAME);

		CWeapon* pCWeapon = static_cast<CWeapon*>(pWeapon);
		CMelee* pMelee = pCWeapon ? pCWeapon->GetMelee() : NULL;
		if (pMelee)
		{
			// Time is too small here, for melee only, multiply by 100 and divide by 100 on the retrieval
			i32k meleeUsedTime = (i32)((pMelee->GetDelay() + pMelee->GetDuration()) * 100.0f); // Count initial delay and total duration
			pSessionStats->m_mapStats[EMPS_WeaponTime].Update(MELEE_WEAPON_NAME, meleeUsedTime);

			//don't add this to time using the actuall weapon in the game reports
			//and game reports don't track melee time
		}
	}
}
//---------------------------------------
void CPersistantStats::OnStartReload(IWeapon *pWeapon, EntityId shooterId, IEntityClass* pAmmoType)
{
	EntityId localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	if (shooterId == localClientId)
	{
		SSessionStats *pSessionStats = GetClientPersistantStats();
		pSessionStats->m_streakIntStats[ESIPS_KillsNoReloadWeapChange].Reset();
	}
}

//---------------------------------------
void CPersistantStats::OnSelected(IWeapon *pWeapon, bool selected)
{
	CWeapon* pCWeapon = static_cast<CWeapon*>(pWeapon);
	if(pCWeapon && pCWeapon->IsOwnerClient())
	{
		if(!selected && m_selectedWeapon == pCWeapon)
		{
			UpdateWeaponTime(pCWeapon);
			m_selectedWeapon = NULL;
		}
		else if(selected && m_selectedWeapon != pCWeapon)
		{
			//if( m_selectedWeapon )
			//{
			//	tukk pOldWeaponName = GetItemName((IItem*) m_selectedWeapon );
			//	tukk pNewWeaponName = GetItemName((IItem*) pCWeapon );
			//	DrxLog( "CPersistantStats::OnSelected Error: Selecting a weapon when one is already selected. Old item %s, new item %s", pOldWeaponName, pNewWeaponName );
			//}
			m_selectedWeapon = pCWeapon;
			m_selectedWeaponTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		}
	}
}

//---------------------------------------
bool CPersistantStats::IncrementWeaponHits(CProjectile& projectile, EntityId targetId)
{
	IActorSystem* pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
	
	IActor* pActor = pActorSystem->GetActor(projectile.GetOwnerId());

	bool success = false;

	if(pActor != NULL && pActor->IsClient())
	{
		IActor* pTargetActor = pActorSystem->GetActor(targetId);
		success = pTargetActor != NULL;

		if(success)
		{
			tukk pWeaponName = GetItemName(projectile.GetWeaponId());

			i32 ammoCost = projectile.GetAmmoCost();

			if(pWeaponName)
			{
				GetClientPersistantStats()->m_mapStats[EMPS_WeaponHits].Update(pWeaponName, ammoCost);
				BLAZE_REPORT_WEAPON(pWeaponName, pActor, hits, ammoCost);
			}

			if(projectile.IsLinked())
			{
				CWeaponSystem* pWeaponSystem = g_pGame->GetWeaponSystem();

				const CWeaponSystem::TLinkedProjectileMap& projectileMap = pWeaponSystem->GetLinkedProjectiles();

				CWeaponSystem::TLinkedProjectileMap::const_iterator projIt = projectileMap.find(projectile.GetEntityId());
				CWeaponSystem::TLinkedProjectileMap::const_iterator projEnd = projectileMap.end();

				if(projIt != projEnd)
				{
					const CWeaponSystem::SLinkedProjectileInfo& info = projIt->second;

					for(CWeaponSystem::TLinkedProjectileMap::const_iterator currentProjIt = projectileMap.begin(); currentProjIt != projEnd; ++currentProjIt)
					{
						const CWeaponSystem::SLinkedProjectileInfo& curInfo = currentProjIt->second;
						if(curInfo.weaponId == info.weaponId && curInfo.shotId == info.shotId)
						{
							CProjectile* pProj = pWeaponSystem->GetProjectile(currentProjIt->first);

							if(pProj)
							{
								pProj->SetHitReported();
							}
						}
					}
				}
			}
		}
	}

	return success;
}

//---------------------------------------
void CPersistantStats::ClientHit(const HitInfo& hitInfo)
{
	IActorSystem* pActorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
	IActor* pShooter = pActorSystem->GetActor(hitInfo.shooterId);
	IActor* pTarget = pActorSystem->GetActor(hitInfo.targetId);

	i32 ammoCost = 1;
	CProjectile *pProjectile = g_pGame->GetWeaponSystem()->GetProjectile(hitInfo.projectileId);
	if (pProjectile)
	{
		ammoCost = pProjectile->GetAmmoCost();
	}
	
	if(gEnv->bMultiplayer)
	{
		if (pShooter != NULL && pTarget != NULL && pShooter->IsPlayer() && pShooter->IsClient() && pTarget->IsPlayer())
		{
			CPlayer* pTargetPlayer = static_cast<CPlayer*>(pTarget);
			CPlayer* pShooterPlayer = static_cast<CPlayer*>(pShooter);

			if(hitInfo.damage > 0)
			{
				const float currTime = gEnv->pTimer->GetCurrTime();
				SSessionStats *pSessionStats = GetClientPersistantStats();

				if(hitInfo.type != CGameRules::EHitType::Melee)
				{
					AddClientHitActorWithWeaponClassId(hitInfo.targetId, hitInfo.weaponClassId, currTime);
				}

				tukk weaponName = GetItemName(hitInfo.weaponId);
				if(weaponName)
				{
					if(pTargetPlayer->IsHeadShot(hitInfo))
					{
						pSessionStats->m_mapStats[EMPS_WeaponHeadshots].Update(weaponName, ammoCost);
						pSessionStats->m_streakIntStats[ESIPS_HeadshotsPerLife].Increment(ammoCost);
						BLAZE_REPORT_WEAPON(weaponName, pShooter, headshots, ammoCost);
					}
				}

				if ( pShooterPlayer->IsFriendlyEntity(hitInfo.targetId) )
				{
					if(hitInfo.shooterId != hitInfo.targetId)
					{
						pSessionStats->m_intStats[EIPS_FriendlyFires] += ammoCost;
						BATTLECHATTER(BC_FriendlyFire, hitInfo.targetId);
					}
				}
				else
				{
					pSessionStats->m_floatStats[EFPS_DamageDelt] += min(hitInfo.damage, pTargetPlayer->GetMaxHealth());
				}

				if ( pTargetPlayer->IsGroinShot(hitInfo) )
				{
					pSessionStats->m_intStats[EIPS_Groinhits] += ammoCost;
				}

				if(hitInfo.type == CGameRules::EHitType::Melee && pActorSystem->GetActor(hitInfo.targetId))
				{
					pSessionStats->m_mapStats[EMPS_WeaponHits].Update(MELEE_WEAPON_NAME);
				}

				const float dot = pTargetPlayer->GetEntity()->GetForwardDir().Dot(hitInfo.dir);
				if ( dot > 0.0f )
				{
					pSessionStats->m_intStats[EIPS_ShotInBack] += ammoCost;
				}

				if (hitInfo.targetId == hitInfo.shooterId && hitInfo.type == CGameRules::EHitType::Fall)
				{
					SSessionStats *pSessionStartStats = GetClientPersistantStatsAtSessionStart();
					float distance = pSessionStartStats->GetStat(EFPS_FallDistance);
					if ( pSessionStats->m_streakFloatStats[ESFPS_DistanceAir].m_curVal > (pSessionStats->m_floatStats[EFPS_FallDistance]-distance) )
					{
						pSessionStats->m_floatStats[EFPS_FallDistance] = pSessionStats->m_streakFloatStats[ESFPS_DistanceAir].m_curVal + distance;
					}
				}
			}
		}
	}
	else //Single player stats 
	{
		if (pShooter && pShooter->IsClient() && pTarget)
		{
			if(hitInfo.type == CGameRules::EHitType::Melee && pActorSystem->GetActor(hitInfo.targetId))
			{
				GetClientPersistantStats()->m_mapStats[EMPS_WeaponHits].Update(MELEE_WEAPON_NAME);
			}

			if ((pTarget->GetActorClass() == CPlayer::GetActorClassType()))
			{
				if(hitInfo.damage > 0)
				{
					tukk weaponName = GetItemName(hitInfo.weaponId);
					if(weaponName)
					{
						CPlayer* pTargetPlayer = static_cast<CPlayer*>(pTarget);

						if(pTargetPlayer->IsHeadShot(hitInfo))
						{
							SSessionStats *pSessionStats = GetClientPersistantStats();

							pSessionStats->m_mapStats[EMPS_WeaponHeadshots].Update(weaponName, ammoCost);
						}
					}
				}
			}
		}
	}
}

//---------------------------------------
void CPersistantStats::ClientShot(CGameRules* pGameRules, u8 hitType, const Vec3& dir)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	IActor * pActor = gEnv->pGame->GetIGameFramework()->GetClientActor();

	if (hitType == CGameRules::EHitType::Bullet)
	{
		m_weaponDamaged = true;
	}

	if ( pActor != NULL && pSessionStats != NULL )
	{
		if ( pActor->GetEntity() )
		{
			const float dot = pActor->GetEntity()->GetForwardDir().Dot(dir);
			if ( dot > 0.0f )
			{
				pSessionStats->m_intStats[EIPS_ShotsInMyBack]++;
			}
		}
		if ( !pActor->IsDead() )
		{
			if ( hitType == CGameRules::EHitType::Frag )
			{
				pSessionStats->m_intStats[EIPS_GrenadeSurvivals]++;
			}
		}
	}
}

//---------------------------------------
void CPersistantStats::ClientRegenerated()
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	if ( m_weaponDamaged )
	{
		pSessionStats->m_intStats[EIPS_HealthRestore]++;
		pSessionStats->m_streakIntStats[ESIPS_HealthRestoresPerLife].Increment();
		if (pSessionStats->m_streakIntStats[ESIPS_HealthRestoresPerLife].m_curVal == 5)
		{
			pSessionStats->m_intStats[EIPS_5HealthRestoresInOneLife]++;
		}
		m_weaponDamaged = false;
	}
}

//---------------------------------------
void CPersistantStats::ClientDied( CPlayer* pClientPlayer )
{
	//TODO: This function is currently called when the player is despawned (for round end ect.)
	//this should not count as a death, but should track the module times. Needs fix

	 // TODO: michiel
}

void CPersistantStats::ClientDelayedExplosion(EntityId projectileDelayed)
{
}
//---------------------------------------

void CPersistantStats::OnSPLevelComplete(tukk levelName)
{
	SSessionStats* pSessionStats = GetClientPersistantStats();

	i32 prevDifficulty = pSessionStats->m_mapStats[EMPS_SPLevelByDifficulty].GetStat(levelName);

	// Current difficulty is based on the lowest difficulty played
	i32 actualDifficultyCompleted = g_pGameCVars->g_difficultyLevel;

  if (g_pGameCVars->g_difficultyLevelLowestPlayed != -1)
	{
		actualDifficultyCompleted = g_pGameCVars->g_difficultyLevelLowestPlayed;
	}
	// storing the stats like this assumes that the valid difficulty levels are all non-zero, and higher value means
	//	harder difficulty.
	STATIC_CHECK((eDifficulty_Easy == 1 && eDifficulty_Delta == 4), Changing_difficulty_level_values_may_break_achievements);

	if(prevDifficulty < actualDifficultyCompleted)
		pSessionStats->m_mapStats[EMPS_SPLevelByDifficulty].Update(levelName, actualDifficultyCompleted - prevDifficulty);

	stl::free_container(m_previousWeaponHitMap);
}

//---------------------------------------
void CPersistantStats::OnGiveAchievement(i32 achievement)
{
	SSessionStats* pSessionStats = GetClientPersistantStats();
	//COMPILE_TIME_ASSERT(eC3A_NumAchievements <= 64);		// If this fails, consider adding EIPS_Achievements3
	DRX_ASSERT_MESSAGE(achievement>=0, "Invalid achievement index");
	if (achievement < 32)
	{
		pSessionStats->m_intStats[EIPS_Achievements1] |= BIT(achievement);
	}
	else if (achievement < 64)
	{
		pSessionStats->m_intStats[EIPS_Achievements2] |= BIT(achievement-32);
	}
	else
	{
		DRX_ASSERT_MESSAGE(false, "Too many achievements to fit in 2 ints, consider adding EIPS_Achievements3");
	}
}


//---------------------------------------

void CPersistantStats::AddListeners()
{
	RemoveAllWeaponListeners();

	CGameRules* pGameRules = g_pGame->GetGameRules();
	pGameRules->RegisterKillListener(this);
	pGameRules->RegisterClientScoreListener(this);
	pGameRules->RegisterRoundsListener(this);
}

//---------------------------------------
void CPersistantStats::ClearListeners()
{
	CGameRules* pGameRules = g_pGame->GetGameRules();
	if(pGameRules)
	{
		pGameRules->UnRegisterKillListener(this);
		pGameRules->UnRegisterClientScoreListener(this);
		pGameRules->UnRegisterRoundsListener(this);
	}
	RemoveAllWeaponListeners();
}

//---------------------------------------
void CPersistantStats::OnEvent(EPPType type, bool skillKill, uk data)
{
	SSessionStats *pSessionStats = GetClientPersistantStats();
	switch(type)
	{
		case EPP_TeamRadar:
			pSessionStats->m_intStats[EIPS_TeamRadar]++;
			break;
		case EPP_MicrowaveBeam:
			pSessionStats->m_intStats[EIPS_MicrowaveBeam]++;
			break;
		case EPP_SuitBoost:
			pSessionStats->m_intStats[EIPS_SuitBoost]++;
			break;
		case EPP_Swarmer:
			pSessionStats->m_intStats[EIPS_SwarmerActivations]++;
			break;
		case EPP_EMPBlast:
			pSessionStats->m_intStats[EIPS_EMPBlastActivations]++;
			break;
		case EPP_FirstBlood:
			pSessionStats->m_intStats[EIPS_FirstBlood]++;
			break;
		case EPP_StealthKill:
		case EPP_StealthKillWithSPModule:
			pSessionStats->m_intStats[EIPS_StealthKills]++;
			if(type == EPP_StealthKillWithSPModule)
			{
				pSessionStats->m_intStats[EIPS_StealthKillsWithSPModuleEnabled]++;
			}
			break;
		case EPP_AirDeath:
			pSessionStats->m_intStats[EIPS_AirDeathKills]++;
			break;
		case EPP_KillJoy:
			pSessionStats->m_intStats[EIPS_KillJoyKills]++;
			break;
		case EPP_BlindKill:
			pSessionStats->m_intStats[EIPS_BlindKills]++;
			break;
		case EPP_Rumbled:
			pSessionStats->m_intStats[EIPS_RumbledKills]++;
			break;
		case EPP_NearDeathExperience:
			pSessionStats->m_intStats[EIPS_NearDeathExperienceKills]++;
			break;
		case EPP_MeleeTakedown:
			pSessionStats->m_intStats[EIPS_MeleeTakeDownKills]++;
			break;
		case EPP_Headshot:
			pSessionStats->m_intStats[EIPS_HeadShotKills]++;
			break;
		case EPP_DoubleKill:
			pSessionStats->m_intStats[EIPS_DoubleKills]++;
			break;
		case EPP_TripleKill:
			pSessionStats->m_intStats[EIPS_TripleKills]++;
			break;
		case EPP_QuadKill:
			pSessionStats->m_intStats[EIPS_QuadKills]++;
			break;
		case EPP_QuinKill:
			pSessionStats->m_intStats[EIPS_QuinKills]++;
			break;
		case EPP_Recovery:
			pSessionStats->m_intStats[EIPS_RecoveryKills]++;
			break;
		case EPP_Retaliation:
			pSessionStats->m_intStats[EIPS_RetaliationKills]++;
			break;
		case EPP_GotYourBack:
			pSessionStats->m_intStats[EIPS_GotYourBackKills]++;
			break;
		case EPP_Piercing:
			pSessionStats->m_intStats[EIPS_PiercingKills]++;
			break;
		case EPP_Guardian:
			pSessionStats->m_intStats[EIPS_GuardianKills]++;
			break;
		case EPP_Blinding:
			pSessionStats->m_intStats[EIPS_BlindingKills]++;
			break;
		case EPP_Flushed:
			pSessionStats->m_intStats[EIPS_FlushedKills]++;
			break;
		case EPP_DualWeapon:
			pSessionStats->m_intStats[EIPS_DualWeaponKills]++;
			break;
		case EPP_Intervention:
			pSessionStats->m_intStats[EIPS_InterventionKills]++;
			break;
		case EPP_KickedCar:
			pSessionStats->m_intStats[EIPS_KickedCarKills]++;
			break;
		case EPP_SuitSuperChargedKill:
			pSessionStats->m_intStats[EIPS_KillsSuitSupercharged]++;
			break;
		case EPP_Incoming:
			pSessionStats->m_intStats[EIPS_PowerStompKills]++;
			break;
		default:
			DRX_ASSERT_MESSAGE(!skillKill,"EPP unknown to CPersistantStats::OnEvent - All skill kills Need to be tracked for Cleaner achievement");
			break;
	}

	if(skillKill)
	{
		pSessionStats->m_intStats[EIPS_SkillKills]++;
	}
}

//---------------------------------------
tukk CPersistantStats::GetActorItemName(EntityId actorId)
{
	IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(actorId);
	if(pActor)
	{
		IItem *pItem = pActor->GetCurrentItem();
		if(pItem)
		{
			return GetItemName(pItem);
		}
	}
	return NULL;
}

//---------------------------------------
tukk CPersistantStats::GetItemName(EntityId weaponId)
{
	IItem *pItem=gEnv->pGame->GetIGameFramework()->GetIItemSystem()->GetItem(weaponId);
	if(pItem)
	{
		return GetItemName(pItem);
	}

	//weaponId isn't ID of an item, could be something else
	//could be an environmental weapon
	if( CEnvironmentalWeapon *pEnvWeapon = static_cast<CEnvironmentalWeapon*>(g_pGame->GetIGameFramework()->QueryGameObjectExtension( weaponId, "EnvironmentalWeapon" )) )
	{
		return pEnvWeapon->GetClassificationName();
	}

	return NULL;
}

//---------------------------------------
tukk CPersistantStats::GetItemName(IItem* pItem)
{
	if( pItem )
	{
		IEntityClass* pClass = pItem->GetEntity()->GetClass();
		g_pGame->GetWeaponSystem()->GetWeaponAlias().UpdateClass(&pClass);
		tukk weaponName = pClass->GetName();
		return weaponName;
	}
	else
	{
		return NULL;
	}
}

//static---------------------------------------
float CPersistantStats::GetRatio(i32 a, i32 b)
{
	float ratio = (float) a;
	if(b > 0)
	{
		ratio = a / (float) b;
	}
	return ratio;
}


//----------------------------------------------------------
tukk  CPersistantStats::GetMapParamAt(tukk  name, u32 at) const
{
	const TMapParams* pMapParams = GetMapParamsExt(name);
	if(pMapParams && (at<pMapParams->size()))
	{
		return pMapParams->at(at).c_str();
	}

	return "";
}

//----------------------------------------------------------
void CPersistantStats::CalculateOverallWeaponStats()
{
	//SP Only

	TMapParams* pMapParams = GetMapParams("Weapon");
	if(!pMapParams)
		return;

	i32 calculatedTotalWeaponsUsedTime = 0;
	i32 highestWeaponTime = 0;
	i32 favoriteWeaponIndex = 0;

	for (size_t i = 0; i < pMapParams->size(); i++)
	{
		const string& weaponName = pMapParams->at(i);
		if (strcmp(weaponName, "FlashBangGrenades") == 0) // Not in SP
		{
			continue;
		}

		i32 weaponTime;
		if (strcmp(weaponName,MELEE_WEAPON_NAME) == 0) // Melee used time is really short, so its multiplied by 100
		{
			weaponTime = (i32)(GetStat(weaponName, EMPS_WeaponTime) / 100.0f);
		}
		else
		{
			weaponTime = GetStat(weaponName, EMPS_WeaponTime);
		}

		if(tukk  pAdditionalName = CPersistantStats::GetAdditionalWeaponNameForSharedStats(weaponName))
		{
			weaponTime += GetStat(pAdditionalName, EMPS_WeaponTime);
		}

		if (weaponTime > highestWeaponTime)
		{
			highestWeaponTime = weaponTime;
			favoriteWeaponIndex = (i32)i;
		}

		calculatedTotalWeaponsUsedTime += weaponTime;
	}

	// Want favorite weapon to be melee if dont have a favorite
	if (highestWeaponTime != 0)
	{
		m_favoriteWeapon = pMapParams->at(favoriteWeaponIndex).c_str();
	}
	else
	{
		m_favoriteWeapon = MELEE_WEAPON_NAME;
	}

	m_iCalculatedTotalWeaponsUsedTime = calculatedTotalWeaponsUsedTime;
}

//----------------------------------------------------------
void CPersistantStats::CalculateOverallAttachmentStats()
{
	i32 calculatedTotalAttachmentUsedTime = 0;
	i32 highestAttachmentTime = 0;
	i32 favoriteAttachmentIndex = 0;

	for (i32 i = 0; i < s_numDisplayableAttachmentsSP; i++)
	{
		tukk attachmentName = s_displayableAttachmentNamesSP[i];

		i32k attachmentTime = GetStat(attachmentName, EMPS_AttachmentTime);
		if (attachmentTime > highestAttachmentTime)
		{
			highestAttachmentTime = attachmentTime;
			favoriteAttachmentIndex = i;
		}

		calculatedTotalAttachmentUsedTime += attachmentTime;
	}

	// Want favorite attachment to be ironsight if dont have a favorite
	if (highestAttachmentTime != 0)
	{
		m_favoriteAttachment = s_displayableAttachmentNamesSP[favoriteAttachmentIndex];
	}
	else
	{
		m_favoriteAttachment = "Ironsight";
	}
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(EIntPersistantStats stat, SPersistantStatsStrings *statsStrings)
{
	statsStrings->m_title.Format("@ui_menu_stats_%s", s_intPersistantNames[(i32)stat]);
	
	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(EFloatPersistantStats stat, SPersistantStatsStrings *statsStrings)
{
	statsStrings->m_title.Format("@ui_menu_stats_%s", s_floatPersistantNames[(i32)stat]);

	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(EStreakIntPersistantStats stat, SPersistantStatsStrings *statsStrings)
{
	statsStrings->m_title.Format("@ui_menu_stats_%s", s_streakIntPersistantNames[(i32)stat]);
	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(EStreakFloatPersistantStats stat, SPersistantStatsStrings *statsStrings)
{
	statsStrings->m_title.Format("@ui_menu_stats_%s", s_streakFloatPersistantNames[(i32)stat]);

	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(EDerivedIntPersistantStats stat, SPersistantStatsStrings *statsStrings)
{
	statsStrings->m_title.Format("@ui_menu_stats_%s", s_intDerivedPersistantNames[(i32)stat]);

	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(EDerivedFloatPersistantStats stat, SPersistantStatsStrings *statsStrings)
{
	statsStrings->m_title.Format("@ui_menu_stats_%s", s_floatDerivedPersistantNames[(i32)stat]);

	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(EDerivedStringPersistantStats stat, SPersistantStatsStrings *statsStrings)
{
	statsStrings->m_title.Format("@ui_menu_stats_%s", s_stringDerivedPersistantNames[(i32)stat]);
	GetClientPersistantStats()->GetStatStrings( stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(EDerivedStringMapPersistantStats stat, SPersistantStatsStrings *statsStrings)
{
	statsStrings->m_title.Format("@ui_menu_stats_%s", s_stringMapDerivedPersistantNames[(i32)stat]);
	GetClientPersistantStats()->GetStatStrings( stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(tukk name, EDerivedIntMapPersistantStats stat, tukk paramString, SPersistantStatsStrings *statsStrings)
{
	DrxFixedStringT<32> localizedString;
	localizedString.Format("@ui_menu_stats_%s", s_intMapDerivedPersistantNames[(i32)stat]);

	if (paramString[0] == '\0')
	{
		localizedString.append("_NoParam");
	}

	statsStrings->m_title = CHUDUtils::LocalizeString(localizedString.c_str(), paramString);

	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( name, stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(tukk name, EDerivedFloatMapPersistantStats stat, tukk paramString, SPersistantStatsStrings *statsStrings)
{
	DrxFixedStringT<32> localizedString;
	localizedString.Format("@ui_menu_stats_%s", s_floatMapDerivedPersistantNames[(i32)stat]);

	if (paramString[0] == '\0')
	{
		localizedString.append("_NoParam");
	}

	statsStrings->m_title = CHUDUtils::LocalizeString(localizedString.c_str(), paramString);

	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( name, stat, statsStrings->m_value );
}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings(tukk name, EMapPersistantStats stat, tukk paramString, SPersistantStatsStrings *statsStrings)
{
	DrxFixedStringT<32> localizedString;
	localizedString.Format("@ui_menu_stats_%s", s_mapPersistantNames[(i32)stat]);

	if (paramString[0] == '\0')
	{
		localizedString.append("_NoParam");
	}

	statsStrings->m_title = CHUDUtils::LocalizeString(localizedString.c_str(), paramString);

	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( name, stat, statsStrings->m_value );

}

//----------------------------------------------------------
void CPersistantStats::GetStatStrings( EMapPersistantStats stat, SPersistantStatsStrings *statsStrings )
{
	statsStrings->m_title.clear();

	statsStrings->m_numericValue = GetClientPersistantStats()->GetStatStrings( stat, statsStrings->m_value );
}

void CPersistantStats::IncrementStatsForActor( EntityId inActorId, EIntPersistantStats stats, i32 amount /*= 1*/ )
{
	DRX_ASSERT((stats >= 0) && (stats < (i32)EIPS_Max));

	SSessionStats *pStats = GetActorSessionStats(inActorId);
	DRX_ASSERT_MESSAGE(pStats, string().Format("IncrementStatsForActor() failed to find session stats for actor=%d", inActorId));
	if (pStats)
	{
		pStats->m_intStats[stats] += amount;
	}
}

void CPersistantStats::IncrementStatsForActor( EntityId inActorId, EFloatPersistantStats stats, float amount /*= 1*/ )
{
	DRX_ASSERT((stats >= 0) && (stats < (i32)EFPS_Max));

	SSessionStats *pStats = GetActorSessionStats(inActorId);
	DRX_ASSERT_MESSAGE(pStats, string().Format("IncrementStatsForActor() failed to find session stats for actor=%d", inActorId));
	if (pStats)
	{
		pStats->m_floatStats[stats] += amount;
	}
}

void CPersistantStats::IncrementClientStats( EIntPersistantStats stats, i32 amount /*= 1*/ )
{
	DRX_ASSERT((stats >= 0) && (stats < (i32)EIPS_Max));

	SSessionStats* pSessionStats = GetClientPersistantStats();

	pSessionStats->m_intStats[stats] += amount;
}

void CPersistantStats::SetClientStat( EIntPersistantStats stat, i32 value )
{
	DRX_ASSERT((stat >= 0) && (stat < (i32)EIPS_Max));

	SSessionStats* pSessionStats = GetClientPersistantStats();

	pSessionStats->m_intStats[stat] = value;
}

void CPersistantStats::IncrementClientStats( EFloatPersistantStats stats, float amount /*= 1*/ )
{
	DRX_ASSERT((stats >= 0) && (stats < (i32)EFPS_Max));

	SSessionStats* pSessionStats = GetClientPersistantStats();

	pSessionStats->m_floatStats[stats] += amount;
}

void CPersistantStats::IncrementMapStats(EMapPersistantStats stats, tukk  name)
{
	DRX_ASSERT((stats >= 0) && (stats < (i32)EMPS_Max));
	SSessionStats* pSessionStats = GetClientPersistantStats();
	pSessionStats->m_mapStats[stats].Update(name);
}

//----------------------------------------------------------
void CPersistantStats::SetMapStat(EMapPersistantStats stats, tukk  name, i32 amount)
{
	DRX_ASSERT((stats >= 0) && (stats < (i32)EMPS_Max));
	SSessionStats* pSessionStats = GetClientPersistantStats();
	pSessionStats->m_mapStats[stats].SetStat(name, amount);
}

//----------------------------------------------------------
void CPersistantStats::ResetMapStat(EMapPersistantStats stats)
{
	DRX_ASSERT((stats >= 0) && (stats < (i32)EMPS_Max));
	SSessionStats* pSessionStats = GetClientPersistantStats();
	pSessionStats->m_mapStats[stats].Clear();
}

//----------------------------------------------------------
i32k CPersistantStats::MapParamCount(u32k flag)
{
	i32k mapParamSize = DRX_ARRAY_COUNT(s_mapParams);
	for(i32 i = 0; i < mapParamSize; i++)
	{
		if((flag & s_mapParams[i].m_flag) == s_mapParams[i].m_flag)
		{
			return s_mapParams[i].m_mapParam.size();
		}
	}

	return 0;
}

//----------------------------------------------------------
tukk CPersistantStats::MapParamName(u32k flag, i32k index)
{
	i32k mapParamSize = DRX_ARRAY_COUNT(s_mapParams);
	for(i32 i = 0; i < mapParamSize; i++)
	{
		if((flag & s_mapParams[i].m_flag) == s_mapParams[i].m_flag)
		{
			return s_mapParams[i].m_mapParam[index].c_str();
		}
	}

	DRX_ASSERT(0);
	return "";
}

//----------------------------------------------------------
const bool CPersistantStats::IsMapParam(u32k flag, tukk paramName)
{
	i32k mapParamCount = MapParamCount(flag);
	for(i32 j = 0; j < mapParamCount; j++)
	{
		tukk name = MapParamName(flag, j);
		if(strcmp(name, paramName) == 0)
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------
void CPersistantStats::UpdateClientGrenadeBounce(const Vec3 pos, const float radius)
{
	CActor* pClientActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
	if(pClientActor)
	{
		IActorIteratorPtr pIt = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
		float radius2 = (radius * radius);
		while (IActor* pActor = pIt->Next())
		{
			if(!pClientActor->IsFriendlyEntity(pActor->GetEntityId()) && !pActor->IsDead())
			{
				Vec3 actorPos = pActor->GetEntity()->GetWorldPos();
				const float distance2 = pos.GetSquaredDistance(actorPos);
				if(distance2 < radius2)
				{
					AddEntityToNearGrenadeList(pActor->GetEntityId(), actorPos);
				}
			}
		}
	}
}

//----------------------------------------------------------
void CPersistantStats::AddEntityToNearGrenadeList(EntityId entityId, Vec3 actorPos)
{
	float currentTime = gEnv->pTimer->GetCurrTime();

	NearGrenadeMap::iterator it = m_nearGrenadeMap.find(entityId);
	if(it != m_nearGrenadeMap.end())
	{
		it->second = currentTime;
	}
	else
	{
		m_nearGrenadeMap.insert(NearGrenadeMap::value_type(entityId, currentTime));
	}
}

//----------------------------------------------------------
bool CPersistantStats::HasClientFlushedTarget(EntityId targetId, Vec3 targetPos)
{
	float currentTime = gEnv->pTimer->GetCurrTime();

	NearGrenadeMap::iterator it = m_nearGrenadeMap.find(targetId);
	if(it != m_nearGrenadeMap.end())
	{
		if((currentTime - it->second) < g_pGameCVars->g_flushed_timeBetweenGrenadeBounceAndSkillKill)
		{
				return true;
			}
		}

	return false;
}

//----------------------------------------------------------
void CPersistantStats::AddClientHitActorWithWeaponClassId(EntityId actorHitId, i32 weaponClassId, float currentTime)
{
	PreviousWeaponHitMap::iterator it = m_previousWeaponHitMap.find(actorHitId);
	if(it != m_previousWeaponHitMap.end())
	{
		if(weaponClassId != it->second.m_curWeaponClassId)
		{
			it->second.m_prevWeaponClassId = it->second.m_curWeaponClassId;
			it->second.m_prevWeaponLastHitTime = it->second.m_curWeaponLastHitTime;
			it->second.m_curWeaponClassId = weaponClassId;
		}

		it->second.m_curWeaponLastHitTime = currentTime;
	}
	else
	{
		SPreviousWeaponHit weaponHit(weaponClassId, currentTime);
		m_previousWeaponHitMap.insert(PreviousWeaponHitMap::value_type(actorHitId, weaponHit));
	}
}

CPersistantStats::SEnemyTeamMemberInfo *CPersistantStats::GetEnemyTeamMemberInfo(EntityId inEntityId)
{
	SEnemyTeamMemberInfo *result=NULL;
	EnemyTeamMemberInfoMap::iterator it = m_enemyTeamMemberInfoMap.find(inEntityId);
	if (it == m_enemyTeamMemberInfoMap.end())
	{
		SEnemyTeamMemberInfo enemyTeamMemberInfo;
		enemyTeamMemberInfo.m_entityId = inEntityId;
		
		std::pair<EnemyTeamMemberInfoMap::iterator, bool> insertResult
			= m_enemyTeamMemberInfoMap.insert(EnemyTeamMemberInfoMap::value_type(inEntityId, enemyTeamMemberInfo));

		result = &insertResult.first->second;
	}
	else
	{
		result = &it->second;
	}

	return result;
}

//----------------------------------------------------------
bool CPersistantStats::IsClientDualWeaponKill(EntityId targetId)
{
	PreviousWeaponHitMap::iterator it = m_previousWeaponHitMap.find(targetId);
	if(it != m_previousWeaponHitMap.end())
	{
		float curTime = gEnv->pTimer->GetCurrTime();

		if(it->second.m_curWeaponClassId != 0 
				&& it->second.m_prevWeaponClassId != 0
				&& it->second.m_prevWeaponLastHitTime > 0.f 
				&& curTime - it->second.m_prevWeaponLastHitTime < g_pGameCVars->g_combinedFire_maxTimeBetweenWeapons)
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------
bool CPersistantStats::IsMultiplayerMapName(tukk name) const
{
	return DrxStringUtils::stristr(name, "multiplayer/") == name;
}

//----------------------------------------------------------
i32 CPersistantStats::GetOnlineAttributesVersion()
{
	return k_ProfileVersionNumber;
}

//----------------------------------------------------------
void CPersistantStats::OnQuit()
{
	DrxLog("[PersistantStats] OnQuit");

	CGameRules		*pGameRules			= g_pGame->GetGameRules();
	CGameLobby		*pGameLobby			= g_pGame->GetGameLobby();
	SSessionStats	*pSessionStats	= GetClientPersistantStats();
	const char		*gamemodeName		= pGameRules ? pGameRules->GetEntity()->GetClass()->GetName() : NULL;

	if(pSessionStats != NULL && gamemodeName)
	{
		pSessionStats->m_mapStats[EMPS_GamesLost].Update(gamemodeName);
		pSessionStats->m_streakIntStats[ESIPS_Win].Reset();
		pSessionStats->m_streakIntStats[ESIPS_Lose].Increment();

		if (pGameLobby->IsOnlineGame() && !pGameLobby->IsPrivateGame())
		{
			pSessionStats->m_streakIntStats[ESIPS_OnlineRankedWin].Reset();
		}
	}
}

//----------------------------------------------------------
// SGameRulesListener (interface)
void CPersistantStats::ClTeamScoreFeedback(i32 teamId, i32 prevScore, i32 newScore)
{
}

//----------------------------------------------------------
// CPersistantStats::SMVPCompare
const bool CPersistantStats::SMVPCompare::CompareForMVP(const EGameMode gamemode, const SMVPCompare& otherPlayer) const
{
	if ((gamemode == eGM_InstantAction) || (gamemode == eGM_AllOrNothing))
	{
		// Most kills
		if (kills > otherPlayer.kills)
			return true;
		else if (kills < otherPlayer.kills)
			return false;

		if (deaths > otherPlayer.deaths)
			return true;
		else if (deaths < otherPlayer.deaths)
			return false;

		if (points > otherPlayer.points)
			return true;
		else if (points < otherPlayer.points)
			return false;
	}
	else if (gamemode == eGM_TeamInstantAction)
	{
		// Best K/D
		i32 spread = kills - deaths;
		i32 otherSpread = otherPlayer.kills - otherPlayer.deaths;

		if (spread > otherSpread)
			return true;
		else if (spread < otherSpread)
			return false;

		if (kills > otherPlayer.kills)
			return true;
		else if (kills < otherPlayer.kills)
			return false;

		if (assists > otherPlayer.assists)
			return true;
		else if (assists < otherPlayer.assists)
			return false;

		if (deaths > otherPlayer.deaths)
			return true;
		else if (deaths < otherPlayer.deaths)
			return false;

		if (points > otherPlayer.points)
			return true;
		else if (points < otherPlayer.points)
			return false;
	}
	else // All other gamemodes
	{
		if (gamemodePoints > otherPlayer.gamemodePoints)
			return true;
		else if (gamemodePoints < otherPlayer.gamemodePoints)
			return false;

		if (points > otherPlayer.points)
			return true;
		else if (points < otherPlayer.points)
			return false;

		if (kills > otherPlayer.kills)
			return true;
		else if (kills < otherPlayer.kills)
			return false;

		if (assists > otherPlayer.assists)
			return true;
		else if (assists < otherPlayer.assists)
			return false;

		if (deaths > otherPlayer.deaths)
			return true;
		else if (deaths < otherPlayer.deaths)
			return false;
	}

	// If they are the same, go with current
	return true;
}

//----------------------------------------------------------
// CPersistantStats::SMVPCompare
i32k CPersistantStats::SMVPCompare::MVPScore(const EGameMode gamemode) const
{
	if ((gamemode == eGM_InstantAction) || (gamemode == eGM_AllOrNothing))
	{
		return kills;
	}
	else if (gamemode == eGM_TeamInstantAction)
	{
		return (kills - deaths);
	}
	else // All other gamemodes
	{
		return gamemodePoints;
	}
}

//----------------------------------------------------------
#define GET_STAT_FROM_NAME_FUNC(functionName, type, nameArray) \
	type CPersistantStats::functionName(tukk name) \
	{ \
		for(i32 i = 0; i < DRX_ARRAY_COUNT(nameArray); i++) \
		{ \
			if(strcmpi(nameArray[i], name) == 0) \
			{ \
				return (type) i; \
			} \
		} \
		return (type) -1; \
	}


//static----------------------------------------------------------
GET_STAT_FROM_NAME_FUNC(GetIntStatFromName, EIntPersistantStats, s_intPersistantNames);
GET_STAT_FROM_NAME_FUNC(GetFloatStatFromName, EFloatPersistantStats, s_floatPersistantNames);
GET_STAT_FROM_NAME_FUNC(GetStreakIntStatFromName, EStreakIntPersistantStats, s_streakIntPersistantNames);
GET_STAT_FROM_NAME_FUNC(GetStreakFloatStatFromName, EStreakFloatPersistantStats, s_streakFloatPersistantNames);
GET_STAT_FROM_NAME_FUNC(GetMapStatFromName, EMapPersistantStats, s_mapPersistantNames);
GET_STAT_FROM_NAME_FUNC(GetDerivedIntStatFromName, EDerivedIntPersistantStats, s_intDerivedPersistantNames);
GET_STAT_FROM_NAME_FUNC(GetDerivedFloatStatFromName, EDerivedFloatPersistantStats, s_floatDerivedPersistantNames);
GET_STAT_FROM_NAME_FUNC(GetDerivedIntMapStatFromName, EDerivedIntMapPersistantStats, s_intMapDerivedPersistantNames);
GET_STAT_FROM_NAME_FUNC(GetDerivedFloatMapStatFromName, EDerivedFloatMapPersistantStats, s_floatMapDerivedPersistantNames);

#undef GET_STAT_FROM_NAME_FUNC


#define GET_NAME_FROM_STAT(functionName, type, nameArray) \
	tukk CPersistantStats::functionName(type stat) \
	{ \
		return nameArray[stat]; \
	}

GET_NAME_FROM_STAT(GetNameFromIntStat, EIntPersistantStats, s_intPersistantNames);
GET_NAME_FROM_STAT(GetNameFromFloatStat, EFloatPersistantStats, s_floatPersistantNames);
GET_NAME_FROM_STAT(GetNameFromStreakIntStat, EStreakIntPersistantStats, s_streakIntPersistantNames);
GET_NAME_FROM_STAT(GetNameFromStreakFloatStat, EStreakFloatPersistantStats, s_streakFloatPersistantNames);
GET_NAME_FROM_STAT(GetNameFromMapStat, EMapPersistantStats, s_mapPersistantNames);
GET_NAME_FROM_STAT(GetNameFromDerivedIntStat, EDerivedIntPersistantStats, s_intDerivedPersistantNames);
GET_NAME_FROM_STAT(GetNameFromDerivedFloatStat, EDerivedFloatPersistantStats, s_floatDerivedPersistantNames);
GET_NAME_FROM_STAT(GetNameFromDerivedIntMapStat, EDerivedIntMapPersistantStats, s_intMapDerivedPersistantNames);
GET_NAME_FROM_STAT(GetNameFromDerivedFloatMapStat, EDerivedFloatMapPersistantStats, s_floatMapDerivedPersistantNames);

#undef GET_NAME_FROM_STAT

#ifndef _RELEASE
//static//---------------------------------
void CPersistantStats::CmdSetStat(IConsoleCmdArgs* pCmdArgs)
{
#define SET_STAT(nameArray, dataPointer, count, formatSpecifier, convertFromString) \
	for(i32 i = 0; i < count; i++) \
	{ \
		if(strcmpi(nameArray[i], statName) == 0) \
		{ \
			DrxLogAlways("Found %s", nameArray[i]); \
			DrxLogAlways("\tValue "#formatSpecifier, pSessionStats->dataPointer[i]); \
			if(shouldSet) \
			{ \
				pSessionStats->dataPointer[i] = convertFromString(statValue); \
				DrxLogAlways("\tSet "#formatSpecifier, pSessionStats->dataPointer[i]); \
			} \
			return; \
		} \
	}

#define SET_STREAK_STAT(nameArray, dataPointer, count, formatSpecifier, convertFromString) \
		for(i32 i = 0; i < count; i++) \
		{ \
			if(strcmpi(nameArray[i], statName) == 0) \
			{ \
				DrxLogAlways("Found %s", nameArray[i]); \
				DrxLogAlways("\tCurValue "#formatSpecifier, pSessionStats->dataPointer[i].m_curVal); \
				DrxLogAlways("\tMaxSessionValue "#formatSpecifier, pSessionStats->dataPointer[i].m_maxThisSessionVal); \
				DrxLogAlways("\tMaxValue "#formatSpecifier, pSessionStats->dataPointer[i].m_maxVal); \
				if(shouldSet) \
				{ \
					pSessionStats->dataPointer[i].Set(convertFromString(statValue)); \
					DrxLogAlways("\tSetCurValue "#formatSpecifier, pSessionStats->dataPointer[i].m_curVal); \
					DrxLogAlways("\tSetMaxSessionValue "#formatSpecifier, pSessionStats->dataPointer[i].m_maxThisSessionVal); \
					DrxLogAlways("\tSetMaxValue "#formatSpecifier, pSessionStats->dataPointer[i].m_maxVal); \
				} \
				return; \
			} \
		}

#define SET_MAP_STAT(nameArray, dataPointer, count, formatSpecifier, convertFromString) \
	for(i32 i = 0; i < count; i++) \
	{ \
		if(strcmpi(nameArray[i], statName) == 0) \
		{ \
			DrxLogAlways("Found %s", nameArray[i]); \
			DrxLogAlways("\tValue "#formatSpecifier, pSessionStats->dataPointer[i].GetStat(statParam)); \
			if(shouldSet) \
			{ \
				pSessionStats->dataPointer[i].SetStat(statParam, convertFromString(statValue)); \
				DrxLogAlways("\tSet "#formatSpecifier, pSessionStats->dataPointer[i].GetStat(statParam)); \
			} \
			return; \
		} \
	}

	if(pCmdArgs->GetArgCount() < 2)
	{
		DrxLogAlways("Expected ps_set <statName> (optional value)");
	}
	else
	{
		CPlayerProgression* pProgression = CPlayerProgression::GetInstance();
		if( pProgression->AllowedWriteStats() )
		{
			CPersistantStats* pStats = CPersistantStats::GetInstance();
			SSessionStats* pSessionStats = pStats->GetClientPersistantStats();

			tukk statName = pCmdArgs->GetArg(1);
			tukk statValue = pCmdArgs->GetArg(2);
			bool shouldSet = pCmdArgs->GetArgCount() == 3;
	
			SET_STAT(s_intPersistantNames, m_intStats, EIPS_Max, %d, atoi);

			SET_STAT(s_floatPersistantNames, m_floatStats, EFPS_Max, %f, (float) atof);	

			SET_STREAK_STAT(s_streakIntPersistantNames, m_streakIntStats, ESIPS_Max, %d, atoi);	

			SET_STREAK_STAT(s_streakFloatPersistantNames, m_streakFloatStats, ESFPS_Max, %f, (float) atof);	

			if(pCmdArgs->GetArgCount() > 4)
			{
				DrxLogAlways("Expected ps_set <statName> <statMap> (optional value)");
			}

			shouldSet = pCmdArgs->GetArgCount() == 4;
			tukk statParam = pCmdArgs->GetArg(2);
			statValue = pCmdArgs->GetArg(3);
			SET_MAP_STAT(s_mapPersistantNames, m_mapStats, EMPS_Max, %d, atoi);
		}
		else
		{
			DrxLogAlways( "ps_set can only be used in MP while in a Ranked Game" );
			return;
		}
	}

	DrxLogAlways("Failed to find param");
#undef SET_STREAK_STAT
#undef SET_STAT
}

//static//---------------------------------
void CPersistantStats::CmdTestStats(IConsoleCmdArgs* pCmdArgs)
{
 // TODO: michiel
}

#endif

/////////////////////////////////////////
CPersistantStats::SSortStat::SSortStat(tukk name)
{
	m_name = name;
}

//static---------------------------------
bool CPersistantStats::SSortStat::WeaponCompare ( SSortStat elem1, SSortStat elem2 )
{
	i32 elem1kills = CPersistantStats::GetInstance()->GetStat(elem1.m_name, EMPS_WeaponKills);
	i32 elem2kills = CPersistantStats::GetInstance()->GetStat(elem2.m_name, EMPS_WeaponKills);
	if(elem1kills != elem2kills)
	{
		return (elem1kills > elem2kills);
	}
	else
	{
		return false;
	}
}

//static---------------------------------
bool CPersistantStats::SSortStat::GamemodeCompare ( SSortStat elem1, SSortStat elem2 )
{
	i32 elem1played = CPersistantStats::GetInstance()->GetStat(elem1.m_name, EMPS_GamemodesTime);
	i32 elem2played = CPersistantStats::GetInstance()->GetStat(elem2.m_name, EMPS_GamemodesTime);
	if(elem1played != elem2played)
	{
		return (elem1played > elem2played);
	}
	else
	{
		i32 elem1won = CPersistantStats::GetInstance()->GetStat(elem1.m_name, EMPS_GamesWon);
		i32 elem2won = CPersistantStats::GetInstance()->GetStat(elem2.m_name, EMPS_GamesWon);
		if(elem1won != elem2won)
		{
			return (elem1won > elem2won);
		}
		else
		{
			return false;
		}
	}
}

//------------------------------------------
void CPersistantStats::OnEnterFindGame()
{
	SSessionStats *pClientStats = GetClientPersistantStats();
	pClientStats->m_intStats[EIPS_LobbyTime] = 0;
}

//------------------------------------------
void CPersistantStats::OnGameActuallyStarting()
{
	if (m_bHasCachedStartingStats)
	{
		m_clientPersistantStats = m_clientPersistantStatsAtGameStart;
	}

#if USE_PC_PREMATCH
	OnGameStarted();
#endif
}

//------------------------------------------
void CPersistantStats::OnGameStarted()
{
	CGameRules* pGameRules = g_pGame->GetGameRules();
	IGameRulesStateModule *stateModule = pGameRules->GetStateModule();
	if (!stateModule || (stateModule->GetGameState() != IGameRulesStateModule::EGRS_PostGame))
	{
		m_gamemodeStartTime = gEnv->pTimer->GetCurrTime();
		m_gamemodeTimeValid = true;

		SSessionStats *pSessionStats = GetClientPersistantStats();
		
		pSessionStats->m_streakIntStats[ESIPS_HeadshotKillsPerMatch].Reset();
		pSessionStats->ResetClientSessionStats();
	}
}

//------------------------------------------
void CPersistantStats::OnEnteredVTOL( EntityId playerId )
{
	if( playerId == g_pGame->GetClientActorId() )
	{
		m_localPlayerInVTOL = true;
	}
}

//------------------------------------------
void CPersistantStats::OnExitedVTOL( EntityId playerId )
{
	if( playerId == g_pGame->GetClientActorId()  )
	{
		m_localPlayerInVTOL = false;
	}
}
//------------------------------------------
const SSessionStats* CPersistantStats::GetPreviousGameSessionStatsForClient( u8 previousGameIndex ) const
{
	if(!m_clientPersistantStatHistory.empty() && previousGameIndex < m_clientPersistantStatHistory.size())
	{
		// we resized this container on Init, so safe to return *'s as will never re-alloc. 
		return &(m_clientPersistantStatHistory[previousGameIndex]);
	}
	return NULL;
}

//------------------------------------------
u32 CPersistantStats::GetAverageDeltaPreviousGameXp( u8k desiredNumGamesToAverageOver ) const
{
	u32 totalXp = 0;
	u32k maxCount = MIN(desiredNumGamesToAverageOver, m_clientPersistantStatHistory.size()); 
	for(u32 i = 0; i < maxCount; ++i)
	{
		totalXp += m_clientPersistantStatHistory[i].m_xpHistoryDelta; 
	}
	u32 avgXp = totalXp / maxCount; // just chopping off any decimal part
	return avgXp; 
}

//------------------------------------------
//NOTE: This only gets called on client who killed the vehicle
void CPersistantStats::OnClientDestroyedVehicle( const SVehicleDestroyedParams& vehicleDestroyedInfo )
{
	if(gEnv->bMultiplayer)
	{
		IGameFramework* pFrameWork = g_pGame->GetIGameFramework(); 
		bool bIsVTOL = false;
		IVehicle* pVehicle = pFrameWork->GetIVehicleSystem()->GetVehicle(vehicleDestroyedInfo.vehicleEntityId);
		IVehicleMovement* pVehicleMovement(NULL);

		if( pVehicle && (pVehicleMovement = pVehicle->GetMovement()) )
		{
			const IVehicleMovement::EVehicleMovementType movementType = pVehicleMovement->GetMovementType();
			if(movementType == IVehicleMovement::eVMT_Air)
			{
				CVTOLVehicleUpr* pVTOLVehicleUpr = g_pGame->GetGameRules()->GetVTOLVehicleUpr();
				bIsVTOL = pVTOLVehicleUpr && pVTOLVehicleUpr->IsVTOL(vehicleDestroyedInfo.vehicleEntityId);
			}
		}

		EntityId clientActorId = g_pGame->GetClientActorId();

		CGameRules* pGameRules = g_pGame->GetGameRules(); 

		if(bIsVTOL)
		{
			if(SSessionStats* pShooterStats = GetActorSessionStats(clientActorId))
			{
				pShooterStats->m_intStats[EIPS_VTOLsDestroyed]++; 

				if( CVTOLVehicleUpr* pVTOLVehicleUpr = pGameRules->GetVTOLVehicleUpr() )
				{
					if( pVTOLVehicleUpr->AnyEnemiesInside( clientActorId ) )
					{
						pShooterStats->m_intStats[ EIPS_EmergencyStat3 ]++;
					}
				}
			}
		}
	}
}

/*static*/ tukk  CPersistantStats::GetAdditionalWeaponNameForSharedStats( tukk  name )
{
	if(strcmp(name,"Hammer") == 0)
	{
		return "CellHammer";
	}
	else if (strcmp(name,"SCARAB") == 0)
	{
		return "CellSCARAB";
	}
	else if (strcmp(name,"SCAR") == 0)
	{
		return "CellSCAR";
	}
	else if (strcmp(name,"Feline") == 0)
	{
		return "CellFeline";
	}
	else if (strcmp(name,"HMG") == 0)
	{
		return "VTOLHMG";
	}
	return NULL;
}


CPersistantStats::SPreviousKillData::SPreviousKillData()
	: m_hitType(CGameRules::EHitType::Invalid)
	, m_bEnvironmental(false)
	, m_bWasPole(false)
{
}

CPersistantStats::SPreviousKillData::SPreviousKillData( i32 hitType, bool bEnvironmental, bool bWasPole )
	: m_hitType(hitType)
	, m_bEnvironmental(bEnvironmental)
	, m_bWasPole(bWasPole)
{
}
