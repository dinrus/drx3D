// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание:		Declares Structures and Enums needed for stats gathering/storing

История:
- 18:04:2012		Created by Andrew Blackwell
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/StatsStructures.h>
#include <drx3D/Game/PersistantStats.h>

#include <drx3D/Game/Utility/DrxWatch.h>

#include <drx3D/Game/Network/Lobby/GameLobby.h>

#include <drx3D/Game/UI/HUD/HUDUtils.h>

#include <drx3D/Game/GameRulesModules/GameRulesModulesUpr.h>
#include <drx3D/Game/ItemSharedParams.h>

// SSessionStats
//---------------------------------------
SSessionStats::SSessionStats()
{
	Clear();
}


//---------------------------------------
void SSessionStats::UpdateClientGUID()
{
	UpdateGUID( gEnv->pGame->GetIGameFramework()->GetClientActorId() );
}

//---------------------------------------
void SSessionStats::UpdateGUID(EntityId actorId)
{
	if( gEnv->bMultiplayer )
	{
		CGameLobby* pLobby = g_pGame->GetGameLobby();
		if( pLobby )
		{
			m_guid = pLobby->GetGUIDFromActorID(actorId);
		}
	}

}

//---------------------------------------
void SSessionStats::Add(const SSessionStats* pSessionStats)
{
	for(i32 i = 0; i < EIPS_Max; i++)
	{
		m_intStats[i] += pSessionStats->m_intStats[i];
	}
	for(i32 i = 0; i < EFPS_Max; i++)
	{
		m_floatStats[i] += pSessionStats->m_floatStats[i];
	}
	for(i32 i = 0; i < ESIPS_Max; i++)
	{
		m_streakIntStats[i].m_maxVal = max(m_streakIntStats[i].m_maxVal, pSessionStats->m_streakIntStats[i].m_maxVal);
	}
	for(i32 i = 0; i < ESFPS_Max; i++)
	{
		m_streakFloatStats[i].m_maxVal = max(m_streakFloatStats[i].m_maxVal, pSessionStats->m_streakFloatStats[i].m_maxVal);
	}
	for(i32 i = 0; i < EMPS_Max; i++)
	{
		SSessionStats::SMap::MapNameToCount::const_iterator it = pSessionStats->m_mapStats[i].m_map.begin();
		SSessionStats::SMap::MapNameToCount::const_iterator end = pSessionStats->m_mapStats[i].m_map.end();
		for ( ; it!=end; ++it)
		{
			m_mapStats[i].Update(it->first, it->second);
		}
	}
}

//---------------------------------------
void SSessionStats::Clear()
{
	m_guid = "";
	m_lastKillTime = 0.0f;

	memset(&m_intStats, 0, sizeof(m_intStats));
	memset(&m_floatStats, 0, sizeof(m_floatStats));

	memset(&m_streakIntStats, 0, sizeof(m_streakIntStats));
	memset(&m_streakFloatStats, 0, sizeof(m_streakFloatStats));

	i32k mapCount = DRX_ARRAY_COUNT(m_mapStats);
	for(i32 i = 0; i < mapCount; i++)
	{
		m_mapStats[i].Clear();
	}
	m_xpHistoryDelta = 0; 
}

//---------------------------------------
void SSessionStats::ResetClientSessionStats()
{
	COMPILE_TIME_ASSERT(ESIPS_Max == DRX_ARRAY_COUNT(m_streakIntStats));
	for(i32 i = 0; i < ESIPS_Max; i++)
	{
		m_streakIntStats[i].ResetSession();
	}
	COMPILE_TIME_ASSERT(ESFPS_Max == DRX_ARRAY_COUNT(m_streakFloatStats));
	for(i32 i = 0; i < ESFPS_Max; i++)
	{
		m_streakFloatStats[i].ResetSession();
	}
}

//---------------------------------------
i32 SSessionStats::GetStat(tukk name, EMapPersistantStats stat) const
{
	DRX_ASSERT(stat >= 0 && stat < DRX_ARRAY_COUNT(m_mapStats));
	return m_mapStats[stat].GetStat(name);
}

//---------------------------------------
const SSessionStats::SMap::MapNameToCount& SSessionStats::GetStat(EMapPersistantStats stat) const
{
	DRX_ASSERT(stat >= 0 && stat < DRX_ARRAY_COUNT(m_mapStats));
	return m_mapStats[stat].m_map;
}

//static---------------------------------------
float SSessionStats::GetRatio(i32 a, i32 b)
{
	float ratio = (float) a;
	if(b > 0)
	{
		ratio = a / (float) b;
	}
	return ratio;
}

//---------------------------------------
i32 SSessionStats::GetDerivedStat(EDerivedIntPersistantStats stat) const
{
	DRX_ASSERT(stat > EDIPS_Invalid && stat < EDIPS_Max);

	switch(stat)
	{
	case EDIPS_GamesWon:
		return m_mapStats[EMPS_GamesWon].GetTotal();

	case EDIPS_GamesDrawn:
		return m_mapStats[EMPS_GamesDrawn].GetTotal();

	case EDIPS_GamesLost:
		return m_mapStats[EMPS_GamesLost].GetTotal();

	case EDIPS_GamesMVP:
		return m_mapStats[EMPS_GamemodesMVP].GetTotal();

	case EDIPS_GamesPlayed:
		return GetDerivedStat(EDIPS_GamesWon) + GetDerivedStat(EDIPS_GamesDrawn) + GetDerivedStat(EDIPS_GamesLost);

	case EDIPS_Kills:
		return GetStat(EIPS_KillsSuitDefault) + GetStat(EIPS_KillsSuitArmor) + GetStat(EIPS_KillsSuitStealth) + GetStat(EIPS_KillsNoSuit);

	case EDIPS_Deaths:
		return GetStat(EIPS_DeathsSuitDefault) + GetStat(EIPS_DeathsSuitArmor) + GetStat(EIPS_DeathsSuitStealth) + GetStat(EIPS_DeathsNoSuit);

	case EDIPS_NanoSuitStregthKills:
		return GetStat(EIPS_MeleeTakeDownKills) + GetStat(EIPS_GrabAndThrow) + GetStat(EIPS_ThrownObjectKill) + GetStat(EIPS_KickedCarKills) + GetStat(EIPS_PowerStompKills);

	case EDIPS_Hits:
		return m_mapStats[EMPS_WeaponHits].GetTotal();

	case EDIPS_Shots:
		return m_mapStats[EMPS_WeaponShots].GetTotal();

	case EDIPS_Misses:
		return GetDerivedStat(EDIPS_Shots) - GetDerivedStat(EDIPS_Hits);

	case EDIPS_Headshots:
		return m_mapStats[EMPS_WeaponHeadshots].GetTotal();

	case EDIPS_HeadshotKills:
		return m_mapStats[EMPS_WeaponHeadshotKills].GetTotal();

	case EDIPS_XP:
		return CPlayerProgression::GetInstance()->GetData(EPP_XP);

	case EDIPS_Rank:
		return CPlayerProgression::GetInstance()->GetData(EPP_DisplayRank);

	case EDIPS_Reincarnation:
		return CPlayerProgression::GetInstance()->GetData(EPP_Reincarnate);

	case EDIPS_AliveTime:
		return GetStat(EIPS_SuitDefaultTime) + GetStat(EIPS_SuitArmorTime) + GetStat(EIPS_SuitStealthTime) + GetStat(EIPS_NoSuitTime);

	case EDIPS_SupportBonus:
		return GetStat(EIPS_TeamRadar) + GetStat(EIPS_MicrowaveBeam) + GetStat(EIPS_SuitBoost);

	case EDIPS_EnvironmentalWeaponKills:
		return m_mapStats[ EMPS_WeaponKills ].GetStat( PANEL_WEAPON_NAME ) + m_mapStats[ EMPS_WeaponKills ].GetStat( POLE_WEAPON_NAME ) + m_mapStats[ EMPS_WeaponKills ].GetStat( PANEL_WEAPON_NAME ENV_WEAPON_THROWN) + m_mapStats[ EMPS_WeaponKills ].GetStat( POLE_WEAPON_NAME ENV_WEAPON_THROWN);

	case EDIPS_NumSPLevels_Hard:
		{
			i32 num = 0;

			const SSessionStats::SMap::MapNameToCount& map = m_mapStats[EMPS_SPLevelByDifficulty].m_map;
			for( SSessionStats::SMap::MapNameToCount::const_iterator it = map.begin(), end = map.end(); it != end; ++it)
			{
				if(it->second >= eDifficulty_Hard)
				{
					++num;
				}
			}
			return num;
		}

	case EDIPS_NumSPLevels_Delta:
		{
			i32 num = 0;

			const SSessionStats::SMap::MapNameToCount& map = m_mapStats[EMPS_SPLevelByDifficulty].m_map;
			for( SSessionStats::SMap::MapNameToCount::const_iterator it = map.begin(), end = map.end(); it != end; ++it )
			{
				if(it->second >= eDifficulty_Delta)
				{
					++num;
				}
			}
			return num;
		}
	default:
		DRX_ASSERT_MESSAGE(false, string().Format("Failed to find EDerivedIntPersistantStats %d", stat));
		return 0;
	} 
}

//---------------------------------------
float SSessionStats::GetDerivedStat(EDerivedFloatPersistantStats stat) const
{
	DRX_ASSERT(stat > EDFPS_Invalid && stat < EDFPS_Max);

	switch(stat)
	{
	case EDFPS_WinLoseRatio:
		return GetRatio(GetDerivedStat(EDIPS_GamesWon), GetDerivedStat(EDIPS_GamesLost));

	case EDFPS_KillDeathRatio:
		return GetRatio(GetDerivedStat(EDIPS_Kills), GetDerivedStat(EDIPS_Deaths));

	case EDFPS_SkillKillKillRatio:
		return GetRatio(GetStat(EIPS_SkillKills), GetDerivedStat(EDIPS_Kills));

	case EDFPS_Accuracy:
		return GetRatio(GetDerivedStat(EDIPS_Hits), GetDerivedStat(EDIPS_Shots)) * 100.0f; //percentage

	case EDFPS_LifeExpectancy:
		return GetRatio( GetDerivedStat( EDIPS_AliveTime ), GetDerivedStat( EDIPS_Deaths ) );

	case EDFPS_MicrowaveBeamEfficiency:
		return GetRatio(GetStat(EIPS_MicrowaveBeamKills), GetStat(EIPS_MicrowaveBeam));

	case EDFPS_SuitArmorUsagePercent:
		{
			const float suitArmorTime = (float)GetStat(EIPS_SuitArmorTime);
			const float totalTime = (float)(suitArmorTime + GetStat(EIPS_SuitStealthTime) + GetStat(EIPS_SuitDefaultTime));
			float usagePercent(0.0f);
			if (totalTime > 0.0f)
			{
				usagePercent = ((suitArmorTime / totalTime) * 100.0f);
			}

			return usagePercent;
		}

	case EDFPS_SuitStealthUsagePercent:
		{
			const float suitStealthTime = (float)GetStat(EIPS_SuitStealthTime);
			const float totalTime = (float)(suitStealthTime + GetStat(EIPS_SuitArmorTime) + GetStat(EIPS_SuitDefaultTime));
			float usagePercent(0.0f);
			if (totalTime > 0.0f)
			{
				usagePercent = ((suitStealthTime / totalTime) * 100.0f);
			}

			return usagePercent;
		}

	case EDFPS_SuitDefaultUsagePercent:
		{
			const float suitDefaultTime = (float)GetStat(EIPS_SuitDefaultTime);
			const float totalTime = (float)(suitDefaultTime + GetStat(EIPS_SuitArmorTime) + GetStat(EIPS_SuitStealthTime));

			float usagePercent(0.0f);
			if (totalTime > 0.0f)
			{
				usagePercent = ((suitDefaultTime / totalTime) * 100.0f);
			}

			return usagePercent;
		}

	case EDFPS_SuitArmorKillDeathRatio:
		return GetRatio(GetStat(EIPS_KillsSuitArmor), GetStat(EIPS_DeathsSuitArmor));

	case EDFPS_SuitStealthKillDeathRatio:
		return GetRatio(GetStat(EIPS_KillsSuitStealth), GetStat(EIPS_DeathsSuitStealth));

	case EDFPS_SuitDefaultKillDeathRatio:
		return GetRatio(GetStat(EIPS_KillsSuitDefault), GetStat(EIPS_DeathsSuitDefault));

	default:
		DRX_ASSERT_MESSAGE(false, string().Format("Failed to find EDerivedFloatPersistantStats %d", stat));
		return 0.0f;
	}
}

//---------------------------------------
i32 SSessionStats::GetDerivedStat(tukk name, EDerivedIntMapPersistantStats stat) const
{
	DRX_ASSERT(stat > EDIMPS_Invalid && stat < EDIMPS_Max);
	switch(stat)
	{
	case EDIMPS_GamesPlayed:
		return GetStat(name, EMPS_GamesWon) + GetStat(name, EMPS_GamesDrawn) + GetStat(name, EMPS_GamesLost);
	case EDIMPS_DisplayedWeaponKills:
		if (strcmp(name, MELEE_WEAPON_NAME) != 0)
		{
			return GetStat(name, EMPS_WeaponKills);
		}
		else // Melee needs to combine normal melee + stealth kills
		{
			return GetStat(name, EMPS_WeaponKills) + GetStat("stealthKill", EMPS_KillsByDamageType);
		}


	default:
		DRX_ASSERT_MESSAGE(false, string().Format("Failed to find EDerivedIntMapPersistantStats %d", stat));
		return 0;
	}
}

//---------------------------------------
float SSessionStats::GetDerivedStat(tukk name, EDerivedFloatMapPersistantStats stat) const
{
	DRX_ASSERT(stat > EDFMPS_Invalid && stat < EDFMPS_Max);
	switch(stat)
	{
	case EDFMPS_Accuracy:
		{
			return GetRatio(GetStat(name, EMPS_WeaponHits), GetStat(name, EMPS_WeaponShots)) * 100.0f;	//percentage
		}
		break;
	case EDFMPS_WeaponKDRatio:
		{
			return GetRatio(GetStat(name, EMPS_WeaponKills), GetStat(name, EMPS_WeaponDeaths));
		}
		break;
	default:
		DRX_ASSERT_MESSAGE(false, string().Format("Failed to find EDerivedFloatMapPersistantStats %d", stat));
		return 0.0f;
	}
}

//---------------------------------------
tukk  SSessionStats::GetDerivedStat(EDerivedStringPersistantStats stat)
{
	DRX_ASSERT(stat > EDSPS_Invalid && stat < EDSPS_Max);
	switch(stat)
	{
	case EDSPS_FavouriteWeapon:
		{
			CPersistantStats *pPersistantStats = g_pGame->GetPersistantStats();
			if (pPersistantStats)
			{
				if (const CItemSharedParams* pItemShared = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(pPersistantStats->GetFavoriteWeaponStr(), false))
				{
					m_dummyStr = pItemShared->params.display_name.c_str();
				}
			}
			return m_dummyStr;
		}
	case EDSPS_FavouriteAttachment:
		{
			CPersistantStats *pPersistantStats = g_pGame->GetPersistantStats();
			if (pPersistantStats)
			{
				if (const CItemSharedParams* pItemShared = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters(pPersistantStats->GetFavoriteAttachmentStr(), false))
				{
					m_dummyStr = pItemShared->params.display_name.c_str();
				}
			}
			return m_dummyStr;
		}

	default:
		DRX_ASSERT_MESSAGE(false, string().Format("Failed to find EDerivedStringPersistantStats %d", stat));
		return NULL;
	}
}

//---------------------------------------
tukk  SSessionStats::GetDerivedStat(EDerivedStringMapPersistantStats stat)
{
	DRX_ASSERT(stat > EDSMPS_Invalid && stat < EDSMPS_Max);
	static DrxFixedStringT<32> returnString;
	switch(stat)
	{
	case EDSMPS_FavouriteGamemode:
		{
			i32 best = -1;
			i32 statValue = 0;
			tukk  bestGamemode = "";

			IGameRulesModulesUpr *pGameRulesModulesUpr = CGameRulesModulesUpr::GetInstance();
			i32k rulesCount = pGameRulesModulesUpr->GetRulesCount();
			for(i32 i = 0; i < rulesCount; i++)
			{
				tukk name = pGameRulesModulesUpr->GetRules(i);
				statValue = GetDerivedStat(name, EDIMPS_GamesPlayed);	
				if (statValue > best)
				{	
					best = statValue;
					bestGamemode = name;
				}
			}

			returnString = bestGamemode;
			return returnString.c_str();
		}
	case EDSMPS_FavouritePrimaryWeapon:
	case EDSMPS_FavouritePrimaryWeaponLocalized:
		{
			CPersistantStats * pPersistantStats =  g_pGame->GetPersistantStats();
			if (pPersistantStats)
			{
				const CPersistantStats::TMapParams *pWeaponParams = pPersistantStats->GetMapParamsExt( pPersistantStats->GetWeaponMapParamName() );
				i32k arraySize = pWeaponParams->size();

				i32 best = -1;
				i32 bestValue = 0;

				for (i32 i=0; i<arraySize; ++i)
				{
					i32 statValue = GetStat((*pWeaponParams)[i],EMPS_WeaponTime);
					if( statValue > bestValue )
					{
						best = i;
						bestValue = statValue;
					}
				}

				if (best>=0)
				{
					if (stat == EDSMPS_FavouritePrimaryWeaponLocalized)
					{
						m_dummyStr.Format("@mp_e%s", (*pWeaponParams)[best].c_str());
						return m_dummyStr.c_str();
					}
					else // EDSMPS_FavouritePrimaryWeapon
					{
						return (*pWeaponParams)[best];
					}
				}
				else
					return NULL;
			}
			return NULL;
		}
			
	default:
		DRX_ASSERT_MESSAGE(false, string().Format("Failed to find EDerivedStringMapPersistantStats %d", stat));
		return NULL;
	}
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings(EIntPersistantStats stat, DrxFixedStringT<64>& valueString )
{
	// Add a switch statement when specific strings are required e.g. 3 "miles"
	i32 statValue = GetStat(stat);

	switch (stat)
	{
		case EIPS_Overall:
		case EIPS_SuitDefaultTime:	// deliberate fall throughs
		case EIPS_SuitArmorTime:
		case EIPS_SuitStealthTime:
		case EIPS_NoSuitTime:
		case EIPS_TimeInVTOLs:
			{
				valueString.Format("%s", GetTimeString(statValue, false, true, true));
				break;
			}
		default:
			{
				valueString.Format("%d", statValue);
			}
	}
	
	return (float)statValue;
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings(EFloatPersistantStats stat, DrxFixedStringT<64>& valueString)
{
	float statValue = GetStat(stat);

	switch (stat)
	{
	case EFPS_DistanceAir:	// deliberate fall throughs
	case EFPS_DistanceRan:
	case EFPS_DistanceSlid:
	case EFPS_DistanceSprint:
		{
			string tempString;
			if (statValue > 1000.f)
			{
				tempString.Format("%.2f", statValue/1000.f);
				valueString = CHUDUtils::LocalizeString("@ui_mp_km", tempString.c_str());
			}
			else
			{
				tempString.Format("%.2f", statValue);
				valueString = CHUDUtils::LocalizeString("@ui_mp_meters", tempString.c_str());
			}
		}
		break;
	case EFPS_CrashSiteHeldTime:
	case EFPS_IntelCollectedTime:
		valueString.Format("%s", GetTimeString(statValue, false, true, true));
		break;
	case EFPS_EnergyUsed:
		valueString.Format("%d", (i32) statValue);
		break;
	default:
		valueString.Format("%.2f", statValue);
		break;
	};

	return statValue;
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings(EStreakIntPersistantStats stat, DrxFixedStringT<64>& valueString)
{
	i32 statValue = GetStat(stat);

	switch (stat)
	{
	case ESIPS_TimeAlive:	// deliberate fall throughs
		{
			valueString.Format("%s", GetTimeString(statValue));
		}
		break;
	default:
		valueString.Format("%d", statValue);
	}
	return (float)statValue;
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings(EStreakFloatPersistantStats stat, DrxFixedStringT<64>& valueString)
{
	float statValue = GetStat(stat);

	switch (stat)
	{
	case ESFPS_DistanceAir:
		{
			string tempString;
			if (statValue > 1000.f)
			{
				tempString.Format("%.2f", statValue/1000.f);
				valueString = CHUDUtils::LocalizeString("@ui_mp_km", tempString.c_str());
			}
			else
			{
				tempString.Format("%.2f", statValue);
				valueString = CHUDUtils::LocalizeString("@ui_mp_meters", tempString.c_str());
			}
		}
		break;
	default:
		valueString.Format("%.2f", statValue);
		break;
	};

	return statValue;
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings(EDerivedIntPersistantStats stat, DrxFixedStringT<64>& valueString)
{
	i32 statValue = GetDerivedStat(stat);
	switch (stat)
	{
	case EDIPS_AliveTime:
		valueString.Format("%s", GetTimeString(statValue));
		break;
	default:
		valueString.Format("%d", statValue);
		break;
	}

	return (float)statValue;
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings(EDerivedFloatPersistantStats stat, DrxFixedStringT<64>& valueString)
{
	float statValue = GetDerivedStat(stat);

	switch (stat)
	{
	case EDFPS_Accuracy:
		valueString.Format("%.2f %%", MIN(statValue,100.f));
		break;
	case EDFPS_LifeExpectancy:
		valueString.Format("%s", GetTimeString(statValue));
		break;
	case EDFPS_SuitArmorUsagePercent:
	case EDFPS_SuitStealthUsagePercent:
	case EDFPS_SuitDefaultUsagePercent:
		valueString.Format("%.2f %%", statValue);
		break;
	default:
		valueString.Format("%.2f", statValue);
		break;
	};

	return statValue;
}

//----------------------------------------------------------
void SSessionStats::GetStatStrings(EDerivedStringPersistantStats stat,DrxFixedStringT<64>& valueString)
{
	valueString = GetDerivedStat(stat);
}

//----------------------------------------------------------
void SSessionStats::GetStatStrings(EDerivedStringMapPersistantStats stat, DrxFixedStringT<64>& valueString)
{
	switch(stat)
	{
	case EDSMPS_FavouriteGamemode:
		valueString.Format("@ui_%s", GetDerivedStat(stat));
		break;
	default:
		valueString = GetDerivedStat(stat);
		break;
	}
	
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings(tukk name, EDerivedIntMapPersistantStats stat, DrxFixedStringT<64>& valueString)
{
	i32 statValue = GetDerivedStat(name, stat);

	valueString.Format("%d", statValue);

	return (float)statValue;
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings(tukk name, EDerivedFloatMapPersistantStats stat, DrxFixedStringT<64>& valueString)
{
	float statValue = GetDerivedStat(name, stat);

	switch (stat)
	{
	case EDFMPS_Accuracy:
		valueString.Format("%.2f %%", statValue);
		break;
	default:
		valueString.Format("%.2f", statValue);
		break;
	};

	return (float)statValue;
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings(tukk name, EMapPersistantStats stat, DrxFixedStringT<64>& valueString)
{
	i32 statValue = GetStat(name, stat);
	return GetStatStringsFromValue(stat, statValue, valueString);
}

//----------------------------------------------------------
float SSessionStats::GetStatStrings( EMapPersistantStats stat, DrxFixedStringT<64>& valueString )
{
	const SMap::MapNameToCount& statsMap = GetStat(stat);

	i32 statValue = 0;

	SMap::MapNameToCount::const_iterator end = statsMap.end();
	for (SMap::MapNameToCount::const_iterator it = statsMap.begin(); it != end; ++ it)
	{
		statValue += it->second;
	}

	return GetStatStringsFromValue(stat, statValue, valueString);
}

//----------------------------------------------------------
float SSessionStats::GetStatStringsFromValue( EMapPersistantStats stat, i32 statValue, DrxFixedStringT<64>& valueString )
{
	switch (stat)
	{
	case EMPS_GamemodesTime:
	case EMPS_WeaponTime:
	default:
		{
			valueString.Format("%d", statValue);
		}
		break;
	}
	

	return (float)statValue;
}


/*
//----------------------------------------------------------
void SSessionStats::GetStatStrings(tukk name, EDerivedIntMapPersistantStats stat, tukk paramString, SPersistantStatsStrings *statsStrings)
{
	DrxFixedStringT<32> localizedString;
	localizedString.Format("@ui_menu_stats_%s", s_intMapDerivedPersistantNames[(i32)stat]);

	statsStrings->m_title = CHUDUtils::LocalizeString(localizedString.c_str(), paramString);

	i32 statValue = GetDerivedStat(name, stat);

	statsStrings->m_value.Format("%d", statValue);
}

//----------------------------------------------------------
void SSessionStats::GetStatStrings(tukk name, EDerivedFloatMapPersistantStats stat, tukk paramString, SPersistantStatsStrings *statsStrings)
{
	DrxFixedStringT<32> localizedString;
	localizedString.Format("@ui_menu_stats_%s", s_floatMapDerivedPersistantNames[(i32)stat]);

	statsStrings->m_title = CHUDUtils::LocalizeString(localizedString.c_str(), paramString);

	float statValue = GetDerivedStat(name, stat);

	switch (stat)
	{
	case EDFMPS_Accuracy:
		statsStrings->m_value.Format("%.2f %%", statValue);
		break;
	default:
		statsStrings->m_value.Format("%.2f", statValue);
		break;
	};
}

//----------------------------------------------------------
void SSessionStats::GetStatStrings(tukk name, EMapPersistantStats stat, tukk paramString, SPersistantStatsStrings *statsStrings)
{
	DrxFixedStringT<32> localizedString;
	localizedString.Format("@ui_menu_stats_%s", s_mapPersistantNames[(i32)stat]);

	statsStrings->m_title = CHUDUtils::LocalizeString(localizedString.c_str(), paramString);

	i32 statValue = GetStat(name, stat);

	statsStrings->m_value.Format("%d", statValue);
}
*/

// SMap Functions
//----------------------------------------------------------
SSessionStats::SMap::SMap()
{
	Clear();
}

//----------------------------------------------------------
void SSessionStats::SMap::Clear()
{
	m_map.clearAndFreeMemory();
}

//----------------------------------------------------------
void SSessionStats::SMap::Update(tukk name, i32 amount)
{
	MapNameToCount::iterator it = FindOrInsert(name);
	it->second += amount;
}

//----------------------------------------------------------
SSessionStats::SMap::MapNameToCount::iterator SSessionStats::SMap::FindOrInsert(tukk name)
{
	DrxFixedStringT<64> lower = name;
	lower.MakeLower();

	MapNameToCount::iterator it = m_map.find(lower);
	if (it == m_map.end())
	{
		MapNameToCount::iterator inserted = m_map.insert(MapNameToCount::value_type(lower, 0)).first;
		return inserted;
	}
	else
	{
		return it;
	}
}

//----------------------------------------------------------
void SSessionStats::SMap::SetStat(tukk name, i32 amount)
{
	MapNameToCount::iterator it = FindOrInsert(name);
	it->second = amount;
}

//----------------------------------------------------------
i32 SSessionStats::SMap::GetStat(tukk name) const
{
	DrxFixedStringT<64> lower = name;
	lower.MakeLower();

	MapNameToCount::const_iterator it = m_map.find(lower);
	if(it != m_map.end())
	{
		return it->second;
	}
	return 0;
}

//----------------------------------------------------------
i32 SSessionStats::SMap::GetTotal() const
{
	i32 total = 0;
	MapNameToCount::const_iterator it = m_map.begin();
	MapNameToCount::const_iterator end = m_map.end();
	for ( ; it!=end; ++it)
	{
		total += it->second;
	}
	return total;
}

#ifndef _RELEASE
//----------------------------------------------------------
void SSessionStats::SMap::watch() const
{
	MapNameToCount::const_iterator it = m_map.begin();
	MapNameToCount::const_iterator end = m_map.end();
	for ( ; it!=end; ++it)
	{
		DrxWatch("\t%s - %d", it->first.c_str(), it->second);
	}
}
#endif
