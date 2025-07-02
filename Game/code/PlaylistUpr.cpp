// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Upr for playlists

-------------------------------------------------------------------------
История:
- 06:03:2010 : Created by Tom Houghton

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/PlaylistUpr.h>

#include <drx3D/Sys/ILocalizationUpr.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Game/UI/ProfileOptions.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/PlayerProgression.h>

#include <drx3D/Game/UI/UICVars.h>
#include <drx3D/Game/UI/UIUpr.h>

#define LOCAL_WARNING(cond, msg)  do { if (!(cond)) { DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "%s", msg); } } while(0)
//#define LOCAL_WARNING(cond, msg)  DRX_ASSERT_MESSAGE(cond, msg)

static CPlaylistUpr*  s_pPlaylistUpr = NULL;

#define MAX_ALLOWED_NEGATIVE_OPTION_AMOUNT 50

#define OPTION_RESTRICTIONS_FILENAME "Scripts/Playlists/OptionRestrictions.xml"

//=======================================
// SPlaylistVariantAutoComplete

//---------------------------------------
// Used by console auto completion.
struct SPlaylistVariantAutoComplete : public IConsoleArgumentAutoComplete
{
	virtual i32 GetCount() const 
	{
		i32 count = 0;
		CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
		if (pPlaylistUpr)
		{
			i32k numPlaylists = pPlaylistUpr->GetNumPlaylists();
			for (i32 i = 0; i < numPlaylists; ++ i)
			{
				const SPlaylist *pPlaylist = pPlaylistUpr->GetPlaylist(i);
				if (pPlaylist->IsEnabled())
				{
					const SPlaylistRotationExtension::TVariantsVec &supportedVariants = pPlaylist->GetSupportedVariants();
					count += supportedVariants.size();
				}
			}
		}
		return count;
	}

	bool GetPlaylistAndVariant(i32 nIndex, const SPlaylist **pPlaylistOut, const SGameVariant **pVariantOut) const
	{
		CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
		if (pPlaylistUpr)
		{
			i32k numPlaylists = pPlaylistUpr->GetNumPlaylists();
			for (i32 i = 0; i < numPlaylists; ++ i)
			{
				const SPlaylist *pPlaylist = pPlaylistUpr->GetPlaylist(i);
				if (pPlaylist->IsEnabled())
				{
					const SPlaylistRotationExtension::TVariantsVec &supportedVariants = pPlaylist->GetSupportedVariants();
					i32k numVariants = supportedVariants.size();
					if (nIndex < numVariants)
					{
						const SGameVariant *pVariant = pPlaylistUpr->GetVariant(supportedVariants[nIndex].m_id);
						if (pVariant)
						{
							*pPlaylistOut = pPlaylist;
							*pVariantOut = pVariant;
							return true;
						}
					}
					else
					{
						nIndex -= numVariants;
					}
				}
			}
		}
		return false;
	}

	tukk GetOptionName(const SPlaylist *pPlaylist, const SGameVariant *pVariant) const
	{
		static DrxFixedStringT<128> combinedName;
		combinedName.Format("%s  %s", pPlaylist->GetUniqueName(), pVariant->m_name.c_str());
		combinedName.replace(' ', '_');
		return combinedName.c_str();
	}

	virtual tukk GetValue( i32 nIndex ) const 
	{ 
		const SPlaylist *pPlaylist = NULL;
		const SGameVariant *pVariant = NULL;

		if (GetPlaylistAndVariant(nIndex, &pPlaylist, &pVariant))
		{
			return GetOptionName(pPlaylist, pVariant);
		}
		return "UNKNOWN";
	};
};
// definition and declaration must be separated for devirtualization
SPlaylistVariantAutoComplete gl_PlaylistVariantAutoComplete;

//=======================================
// SPlaylist

//---------------------------------------
void SPlaylist::ResolveVariants()
{
	if (!rotExtInfo.m_resolvedVariants)
	{
		// First time in, find all the variant ids
		CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
		if (pPlaylistUpr)
		{
			bool hasDefault = false;
			i32 defaultIndex = pPlaylistUpr->GetDefaultVariant();

			i32 numVariants = rotExtInfo.m_supportedVariants.size();
			for (i32 i = 0; i < numVariants; ++ i)
			{
				i32 variantIndex = pPlaylistUpr->GetVariantIndex(rotExtInfo.m_supportedVariants[i].m_name.c_str());
				if (variantIndex >= 0)
				{
					rotExtInfo.m_supportedVariants[i].m_id = variantIndex;
					if (variantIndex == defaultIndex)
					{
						hasDefault = true;
					}
				}
				else
				{
					// Unknown variant, throw it away
					SPlaylistRotationExtension::TVariantsVec::iterator it = rotExtInfo.m_supportedVariants.begin() + i;
					rotExtInfo.m_supportedVariants.erase(it);
					-- i;
					-- numVariants;
				}
			}

			if (rotExtInfo.m_allowDefaultVariant && (!hasDefault) && (defaultIndex != -1))
			{
				const SGameVariant *pVariant = pPlaylistUpr->GetVariant(defaultIndex);
				SSupportedVariantInfo defaultVariant;
				defaultVariant.m_name = pVariant->m_name;
				defaultVariant.m_id = pVariant->m_id;

				rotExtInfo.m_supportedVariants.insert(rotExtInfo.m_supportedVariants.begin(), 1, defaultVariant);
			}
		}
		rotExtInfo.m_resolvedVariants = true;
	}
}

//=======================================
// SPlaylistRotationExtension

//---------------------------------------
// Suppress passedByValue for smart pointers like XmlNodeRef
// cppcheck-suppress passedByValue
bool SPlaylistRotationExtension::LoadFromXmlNode(const XmlNodeRef xmlNode)
{
	bool  loaded = false;

	if (const XmlNodeRef infoNode=xmlNode->findChild("ExtendedInfo"))
	{
		if (!infoNode->getAttr("enabled", m_enabled))
		{
			LOCAL_WARNING(0,"Level Rotation is a Playlist but its \"PlaylistInfo\" entry doesn't have an \"enabled\" attribute");
		}

		infoNode->getAttr("requiresUnlocking", m_requiresUnlocking);

		infoNode->getAttr("hidden", m_hidden);

		tukk  pPath;
		infoNode->getAttr("image", &pPath);
		imagePath = pPath;

		uniqueName = infoNode->getAttr("uniquename");
		LOCAL_WARNING(uniqueName, "Level Rotation is a Playlist but its \"PlaylistInfo\" entry doesn't have a \"uniqueName\" attribute");

		tukk  pCurrentLang = gEnv->pSystem->GetLocalizationUpr()->GetLanguage();
		DRX_ASSERT(pCurrentLang);

		CPlaylistUpr::GetLocalisationSpecificAttr<TLocaliNameStr>(infoNode, "localname", pCurrentLang, &localName);

		CPlaylistUpr::GetLocalisationSpecificAttr<TLocaliDescStr>(infoNode, "localdescription", pCurrentLang, &localDescription);

		tukk  typeStr = NULL;
		if (infoNode->getAttr("supportedvariants", &typeStr))
		{
			i32 foundAtIndex = 0;
			tukk startFrom = typeStr;

			do
			{
				char variant[32];
				foundAtIndex = drx_copyStringUntilFindChar(variant, startFrom, sizeof(variant), '|');
				startFrom += foundAtIndex;

				m_supportedVariants.push_back(SSupportedVariantInfo(variant));
			} while (foundAtIndex);
		}

		infoNode->getAttr("useDefaultVariant", m_allowDefaultVariant);

		i32 maxPlayers = MAX_PLAYER_LIMIT;
		if (infoNode->getAttr("maxPlayers", maxPlayers))
		{
			maxPlayers = CLAMP(maxPlayers, 2, MAX_PLAYER_LIMIT);
		}
		m_maxPlayers = maxPlayers;

		loaded = true;
	}
	else
	{
		LOCAL_WARNING(0, "Level Rotation xmlNode doesn't have a \"ExtendedInfo\" entry");
	}

	return loaded;
}

//=======================================
// SGameModeOption

//---------------------------------------
void SGameModeOption::CopyToCVar( CProfileOptions *pProfileOptions, bool useDefault, tukk pGameModeName )
{
	if (m_pCVar)
	{
#ifdef _DEBUG
		if (m_pCVar->GetFlags() & VF_WASINCONFIG)
		{
			// Allow gamemode options to be overridden in user.cfg when in debug mode
			return;
		}
#endif

		TFixedString optionName;
		if (m_gameModeSpecific)
		{
			optionName.Format(m_profileOption.c_str(), pGameModeName);
		}
		else
		{
			optionName = m_profileOption.c_str();
		}

		if ((!optionName.empty()) && pProfileOptions->IsOption(optionName.c_str()))
		{
			if (m_pCVar->GetType() == CVAR_INT)
			{
				m_pCVar->Set(pProfileOptions->GetOptionValueAsInt(optionName.c_str(), useDefault));
			}
			else if (m_pCVar->GetType() == CVAR_FLOAT)
			{
				m_pCVar->Set(pProfileOptions->GetOptionValueAsFloat(optionName.c_str(), useDefault) * m_profileMultiplyer);
			}
			else if (m_pCVar->GetType() == CVAR_STRING)
			{
				m_pCVar->Set(pProfileOptions->GetOptionValue(optionName.c_str(), useDefault));
			}
		}
		else
		{
			if (m_pCVar->GetType() == CVAR_INT)
			{
				m_pCVar->Set(m_iDefault);
			}
			else if (m_pCVar->GetType() == CVAR_FLOAT)
			{
				m_pCVar->Set(m_fDefault);
			}
			else if (m_pCVar->GetType() == CVAR_STRING)
			{
				m_pCVar->Set("");
			}
		}
	}
}

//---------------------------------------
void SGameModeOption::CopyToProfile( CProfileOptions *pProfileOptions, tukk pGameModeName )
{
	if (m_pCVar && !m_profileOption.empty())
	{
		TFixedString optionName;
		if (m_gameModeSpecific)
		{
			optionName.Format(m_profileOption.c_str(), pGameModeName);
		}
		else
		{
			optionName = m_profileOption.c_str();
		}

		if (m_pCVar->GetType() == CVAR_INT)
		{
			pProfileOptions->SetOptionValue(optionName.c_str(), m_pCVar->GetIVal());
		}
		else if (m_pCVar->GetType() == CVAR_FLOAT)
		{
			pProfileOptions->SetOptionValue(optionName.c_str(), int_round(m_pCVar->GetFVal() / m_profileMultiplyer));
		}
	}
}

//---------------------------------------
void SGameModeOption::SetInt( ICVar *pCvar, tukk pProfileOption, bool gameModeSpecific, i32 iDefault )
{
	SetCommon(pCvar, pProfileOption, gameModeSpecific, 1.f, 1.f);
	m_iDefault = iDefault;
}

//---------------------------------------
void SGameModeOption::SetFloat( ICVar *pCvar, tukk pProfileOption, bool gameModeSpecific, float netMultiplyer, float profileMultiplyer, float fDefault, i32 floatPrecision )
{
	SetCommon(pCvar, pProfileOption, gameModeSpecific, netMultiplyer, profileMultiplyer);
	m_fDefault = fDefault;
	m_floatPrecision = floatPrecision;
}

//---------------------------------------
void SGameModeOption::SetString( ICVar *pCvar, tukk pProfileOption, bool gameModeSpecific )
{
	SetCommon(pCvar, pProfileOption, gameModeSpecific, 1.f, 1.f);
}


//---------------------------------------
void SGameModeOption::SetCommon( ICVar *pCvar, tukk pProfileOption, bool gameModeSpecific, float netMultiplyer, float profileMultiplyer )
{
	m_pCVar = pCvar;
	m_profileOption = pProfileOption;
	m_gameModeSpecific = gameModeSpecific;
	m_netMultiplyer = netMultiplyer;
	m_profileMultiplyer = profileMultiplyer;
}

//=======================================
// CPlaylistUpr

//---------------------------------------
CPlaylistUpr::CPlaylistUpr()
{
	DRX_ASSERT_MESSAGE(!s_pPlaylistUpr, "There should only ever be one Playlist Upr - and there's one already!");
	s_pPlaylistUpr = this;

	m_playlists.resize(0);
	SetCurrentPlaylist(0);
	m_inited = false;
	m_defaultVariantIndex = -1;
	m_customVariantIndex = -1;
	m_activeVariantIndex = -1;
	m_bIsSettingOptions = false;

	if (gEnv->IsDedicated())
	{
		gEnv->pConsole->AddConsoleVarSink(this);
	}
}

//---------------------------------------
CPlaylistUpr::~CPlaylistUpr()
{
	LOCAL_WARNING(!m_inited, "PlaylistUpr is being deleted before being de-initialised - unless this is a proper game shutdown, cvars (etc.) will be being leaked");
	s_pPlaylistUpr = NULL;

	if (gEnv->IsDedicated())
	{
		gEnv->pConsole->RemoveConsoleVarSink(this);
	}

	i32 numRestrictions = m_optionRestrictions.size();
	for (i32 i = 0; i < numRestrictions; ++ i)
	{
		if (m_optionRestrictions[i].m_operand1.m_pVar)
		{
			m_optionRestrictions[i].m_operand1.m_pVar->SetOnChangeCallback(NULL);
		}
		if (m_optionRestrictions[i].m_operand2.m_pVar)
		{
			m_optionRestrictions[i].m_operand2.m_pVar->SetOnChangeCallback(NULL);
		}
	}

	Deinit();
}

//---------------------------------------
void CPlaylistUpr::Init(tukk pPath)
{
	DRX_ASSERT(!m_inited);

	AddPlaylistsFromPath(pPath);
	AddVariantsFromPath(pPath);

	// Add the custom variant
	SGameVariant customVariant;
	customVariant.m_allowInCustomGames = true;
	customVariant.m_supportsAllModes = true;
	customVariant.m_name = PLAYLIST_MANAGER_CUSTOM_NAME;
	customVariant.m_localName = "@ui_variant_Custom";
	customVariant.m_localDescription = "@ui_tooltip_variant_Custom";
	customVariant.m_localDescriptionUpper = "@ui_tooltip_variant_Custom_upper";
	customVariant.m_id = m_variants.size();
	customVariant.m_enabled = true;
	customVariant.m_requiresUnlock = false;

	m_variants.push_back(customVariant);
	m_customVariantIndex = customVariant.m_id;

#if !defined(_RELEASE)
		REGISTER_COMMAND("playlists_list", CmdPlaylistsList, VF_CHEAT, "List all available playlists (and show current)");
		REGISTER_COMMAND("playlists_choose", CmdPlaylistsChoose, VF_CHEAT, "Choose a listed playlist to be the current playlist");
		REGISTER_COMMAND("playlists_unchoose", CmdPlaylistsUnchoose, VF_CHEAT, "Unchoose the current playlist (ie. leaves none chosen)");
		REGISTER_COMMAND("playlists_show_variants", CmdPlaylistsShowVariants, VF_CHEAT, "Show all the variants available for current playlist (and show the active)");
		REGISTER_COMMAND("playlists_select_variant", CmdPlaylistsSelectVariant, VF_CHEAT, "Select a variant to be active for the current playlist");
#endif

	m_inited = true;

	// Add gamemode specific options
	AddGameModeOptionInt("g_scoreLimit", "MP/Options/%s/ScoreLimit", true, 75 );
	AddGameModeOptionFloat("g_timelimit", "MP/Options/%s/TimeLimit", true, 0.1f, 1.f, 10.f, 1);
	AddGameModeOptionInt("g_minplayerlimit", "MP/Options/%s/MinPlayers", true, DEFAULT_MINPLAYERLIMIT);	
	AddGameModeOptionFloat("g_autoReviveTime", "MP/Options/%s/RespawnDelay", true, 1.f, 1.f, 9.f, 0);
	AddGameModeOptionInt("g_roundlimit", "MP/Options/%s/NumberOfRounds", true, 2);
	AddGameModeOptionInt("g_numLives", "MP/Options/%s/NumberOfLives", true, 0);

	// Add global options
	AddGameModeOptionFloat("g_maxHealthMultiplier", "MP/Options/MaximumHealth", false, 0.01f, 0.01f, 1.f, 0);
	AddGameModeOptionInt("g_mpRegenerationRate", "MP/Options/HealthRegen", false, 1);
	AddGameModeOptionFloat("g_friendlyfireratio", "MP/Options/FriendlyFire", false, 0.01f, 0.01f, 0.f, 0);
	AddGameModeOptionInt("g_mpHeadshotsOnly", "MP/Options/HeadshotsOnly", false, 0);
	AddGameModeOptionInt("g_mpNoVTOL", "MP/Options/NoVTOL", false, 0);
	AddGameModeOptionInt("g_mpNoEnvironmentalWeapons", "MP/Options/NoEnvWeapons", false, 0);
	AddGameModeOptionInt("g_allowCustomLoadouts", "MP/Options/AllowCustomClasses", false, 1);
	AddGameModeOptionInt("g_allowFatalityBonus", "MP/Options/AllowFatalityBonus", false, 1);
	AddGameModeOptionInt("g_modevarivar_proHud", "MP/Options/ProHUD", false, 0);
	AddGameModeOptionInt("g_modevarivar_disableKillCam", "MP/Options/DisableKillcam", false, 0);
	AddGameModeOptionInt("g_modevarivar_disableSpectatorCam", "MP/Options/DisableSpectatorCam", false, 0);
	//AddGameModeOptionFloat("g_xpMultiplyer", "", false, 1.f, 1.f, 1.f, 0);		// Don't link this to a profile attribute to prevent it being used in custom variants
	AddGameModeOptionInt("g_allowExplosives", "MP/Options/AllowExplosives", false, 1);
	AddGameModeOptionInt("g_forceWeapon", "MP/Options/ForceWeapon", false, -1);
	AddGameModeOptionInt("g_infiniteAmmo", "MP/Options/InfiniteAmmo", false, 0);
	AddGameModeOptionInt("g_infiniteCloak", "MP/Options/InfiniteCloak", false, 0);
	AddGameModeOptionInt("g_allowWeaponCustomisation", "MP/Options/AllowWeaponCustomise", false, 1);
	AddGameModeOptionString("g_forceHeavyWeapon", "MP/Options/ForceHeavyWeapon", false);
	AddGameModeOptionString("g_forceLoadoutPackage", "MP/Options/ForceLoadoutPackage", false);

	AddConfigVar("g_autoAssignTeams", true);
	AddConfigVar("gl_initialTime", true);
	AddConfigVar("gl_time", true);
	AddConfigVar("g_gameRules_startTimerLength", true);
	AddConfigVar("sv_maxPlayers", false);
	AddConfigVar("g_forceHeavyWeapon", true);
	AddConfigVar("g_forceLoadoutPackage", true);
	AddConfigVar("g_switchTeamAllowed", true);
	AddConfigVar("g_switchTeamRequiredPlayerDifference", true);
	AddConfigVar("g_switchTeamUnbalancedWarningDifference", true);
	AddConfigVar("g_switchTeamUnbalancedWarningTimer", true);

	// First time through here, register startPlaylist command and check to see if we need to start straight away
	static bool bOnlyOnce = true;
	if (bOnlyOnce)
	{
		DRX_ASSERT(gEnv);
		IConsole *pConsole = gEnv->pConsole;
		if (pConsole)
		{
			bOnlyOnce = false;

			// Create a temporary console variable so that the startPlaylist command can be used in a config
			ICVar *pInitialPlaylist = REGISTER_STRING("startPlaylist", "", VF_NULL, "");
			if (pInitialPlaylist->GetFlags() & VF_MODIFIED)
			{
				DoStartPlaylistCommand(pInitialPlaylist->GetString());
			}
			pConsole->UnregisterVariable("startPlaylist", true);

			// Now register the actual command
			REGISTER_COMMAND("startPlaylist", CmdStartPlaylist, VF_NULL, "Start the specified playlist/variant");
			pConsole->RegisterAutoComplete("startPlaylist", &gl_PlaylistVariantAutoComplete);
		}
	}

	LoadOptionRestrictions();

#if USE_DEDICATED_LEVELROTATION
	if (gEnv->IsDedicated())
	{
		LoadLevelRotation();
	}
#endif
}

//---------------------------------------
void CPlaylistUpr::AddGameModeOptionInt( tukk pCVarName, tukk pProfileOption, bool gameModeSpecific, i32 iDefault )
{
	DRX_ASSERT(gEnv);
	m_options.push_back(SGameModeOption());
	SGameModeOption &newOption = m_options[m_options.size() - 1];
	ICVar *pCVar = gEnv->pConsole->GetCVar(pCVarName); 
	DRX_ASSERT(pCVar); 
	newOption.SetInt(pCVar, pProfileOption, gameModeSpecific, iDefault); 
	if (!pCVar)
	{
		GameWarning("PlaylistUpr: Failed to find cvar %s", pCVarName);
	}
}

//---------------------------------------
void CPlaylistUpr::AddGameModeOptionFloat( tukk pCVarName, tukk pProfileOption, bool gameModeSpecific, float netMultiplyer, float profileMultiplyer, float fDefault, i32 floatPrecision )
{
	DRX_ASSERT(gEnv);
	m_options.push_back(SGameModeOption());
	SGameModeOption &newOption = m_options[m_options.size() - 1];
	ICVar *pCVar = gEnv->pConsole->GetCVar(pCVarName); 
	DRX_ASSERT(pCVar); 
	newOption.SetFloat(pCVar, pProfileOption, gameModeSpecific, netMultiplyer, profileMultiplyer, fDefault, floatPrecision); 
	if (!pCVar)
	{
		GameWarning("PlaylistUpr: Failed to find cvar %s", pCVarName);
	}
}

//---------------------------------------
void CPlaylistUpr::AddGameModeOptionString( tukk pCVarName, tukk pProfileOption, bool gameModeSpecific )
{
	DRX_ASSERT(gEnv);
	m_options.push_back(SGameModeOption());
	SGameModeOption &newOption = m_options[m_options.size() - 1];
	ICVar *pCVar = gEnv->pConsole->GetCVar(pCVarName); 
	DRX_ASSERT(pCVar); 
	newOption.SetString(pCVar, pProfileOption, gameModeSpecific); 
	if (!pCVar)
	{
		GameWarning("PlaylistUpr: Failed to find cvar %s", pCVarName);
	}
}

//---------------------------------------
SGameModeOption *CPlaylistUpr::GetGameModeOptionStruct( tukk pOptionName )
{
	DRX_ASSERT(gEnv);
	CGameLobby *pGameLobby = g_pGame->GetGameLobby();
	IGameRulesSystem *pGameRulesSystem = gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem();
	tukk  sv_gamerules = gEnv->pConsole->GetCVar("sv_gamerules")->GetString();

	tukk  fullGameRulesName = (pGameLobby && pGameLobby->IsCurrentlyInSession()) ? pGameLobby->GetCurrentGameModeName() : pGameRulesSystem->GetGameRulesName(sv_gamerules);

	SGameModeOption *pResult = NULL;

	if (fullGameRulesName)
	{
		DrxFixedStringT<32> gameModeOptionName;
		i32k numOptions = m_options.size();
		for (i32 i = 0; i < numOptions; ++ i)
		{
			SGameModeOption &option = m_options[i];
			if (option.m_gameModeSpecific)
			{
				gameModeOptionName.Format(option.m_profileOption.c_str(), fullGameRulesName);
				if (!stricmp(gameModeOptionName.c_str(), pOptionName))
				{
					pResult = &option;
					break;
				}
			}
			else
			{
				if (!stricmp(option.m_profileOption.c_str(), pOptionName))
				{
					pResult = &option;
					break;
				}
			}
		}
	}
	return pResult;
}

//---------------------------------------
void CPlaylistUpr::AddConfigVar(tukk pCVarName, bool bNetSynched)
{
	ICVar *pCVar = gEnv->pConsole->GetCVar(pCVarName);
	if (pCVar)
	{
		SConfigVar var;
		var.m_pCVar = pCVar;
		var.m_bNetSynched = bNetSynched;
		m_configVars.push_back(var);
	}
}

//---------------------------------------
void CPlaylistUpr::Deinit()
{
	if (m_inited)
	{
#if !defined(_RELEASE)
		if (gEnv && gEnv->pConsole)
		{
			gEnv->pConsole->RemoveCommand("playlists_list");
			gEnv->pConsole->RemoveCommand("playlists_choose");
			gEnv->pConsole->RemoveCommand("playlists_unchoose");
			gEnv->pConsole->RemoveCommand("playlists_show_variants");
			gEnv->pConsole->RemoveCommand("playlists_select_variant");
		}
#endif

		if (ILevelSystem* pLevelSystem=g_pGame->GetIGameFramework()->GetILevelSystem())
		{
			pLevelSystem->ClearExtendedLevelRotations();
		}

		ClearAllVariantCVars();

		m_inited = false;
	}
}

//---------------------------------------
void CPlaylistUpr::ClearAllVariantCVars()
{
	i32k numOptions = m_options.size();
	for (i32 i = 0; i < numOptions; ++ i)
	{
		SGameModeOption &option = m_options[i];
		if (option.m_pCVar)
		{
#ifdef _DEBUG
			if (option.m_pCVar->GetFlags() & VF_WASINCONFIG)
			{
				// Allow gamemode options to be overridden in user.cfg when in debug mode
				continue;
			}
#endif
			if (option.m_pCVar->GetType() == CVAR_INT)
			{
				option.m_pCVar->Set(option.m_iDefault);
			}
			else if (option.m_pCVar->GetType() == CVAR_FLOAT)
			{
				option.m_pCVar->Set(option.m_fDefault);
			}
			else if (option.m_pCVar->GetType() == CVAR_STRING)
			{
				option.m_pCVar->Set("");
			}
		}
	}
}

//---------------------------------------
void CPlaylistUpr::SaveCurrentSettingsToProfile()
{
	CProfileOptions *pProfileOptions = g_pGame->GetProfileOptions();
	DRX_ASSERT(pProfileOptions);
	if (pProfileOptions)
	{
		CGameLobby *pGameLobby = g_pGame->GetGameLobby();
		DRX_ASSERT(gEnv);
		IGameRulesSystem *pGameRulesSystem = gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem();
		tukk  sv_gamerules = gEnv->pConsole->GetCVar("sv_gamerules")->GetString();

		tukk  fullGameRulesName = (pGameLobby && pGameLobby->IsCurrentlyInSession()) ? pGameLobby->GetCurrentGameModeName() : pGameRulesSystem->GetGameRulesName(sv_gamerules);

		DRX_ASSERT(fullGameRulesName);
		if (fullGameRulesName)
		{
			i32k numOptions = m_options.size();
			for (i32 i = 0; i < numOptions; ++ i)
			{
				SGameModeOption &option = m_options[i];
				option.CopyToProfile(pProfileOptions, fullGameRulesName);
			}
		}
	}
}

//---------------------------------------
void CPlaylistUpr::OnPromoteToServer()
{
	// If we take ownership of a game using a custom variant, we overwrite our settings with the game settings
	// so that they aren't lost
	if (m_activeVariantIndex == m_customVariantIndex)
	{
		SaveCurrentSettingsToProfile();
	}
}

//---------------------------------------
bool CPlaylistUpr::HaveUnlockedPlaylist(const SPlaylist *pPlaylist, DrxFixedStringT<128>* pUnlockString/*=NULL*/)
{
	bool result = true;
	if (pPlaylist && pPlaylist->RequiresUnlocking())
	{
		CPlayerProgression *pPlayerProgression = CPlayerProgression::GetInstance();
		if (pPlayerProgression)
		{
			SPPHaveUnlockedQuery results;
			pPlayerProgression->HaveUnlocked(eUT_Playlist, pPlaylist->GetUniqueName(), results);

			if (results.exists && results.unlocked == false)
			{
				if (pUnlockString)
				{
					*pUnlockString = results.unlockString.c_str();
				}
				result = false;
			}
		}
	}

	return result;
}

//---------------------------------------
bool CPlaylistUpr::HaveUnlockedPlaylistVariant(const SPlaylist *pPlaylist, i32k variantId, DrxFixedStringT<128>* pUnlockString/*=NULL*/)
{
	bool result = true;

	const SGameVariant* pVariant = GetVariant(variantId);
	if (pPlaylist && pVariant && pVariant->m_requiresUnlock)
	{
		DrxFixedStringT<32> variantUnlockName;
		variantUnlockName.Format("%s", pVariant->m_name.c_str());

		CPlayerProgression *pPlayerProgression = CPlayerProgression::GetInstance();
		if (pPlayerProgression)
		{
			SPPHaveUnlockedQuery results;
			pPlayerProgression->HaveUnlocked(eUT_Playlist, variantUnlockName.c_str(), results);

			if (results.exists && results.unlocked == false)
			{
				if (pUnlockString)
				{
					*pUnlockString = results.unlockString.c_str();
				}
				result = false;
			}
		}
	}

	return result;
}

//---------------------------------------
void CPlaylistUpr::SetCurrentPlaylist(const ILevelRotation::TExtInfoId id)
{
	m_currentPlaylistId = id;
	if (!m_variants.empty())
	{
		DRX_ASSERT_MESSAGE((m_defaultVariantIndex != -1), "Failed to find a default variant");
		SetActiveVariant(m_defaultVariantIndex);
	}

	i32 maxPlayers = MAX_PLAYER_LIMIT;

	const SPlaylist *pPlaylist = GetCurrentPlaylist();
	if (pPlaylist)
	{
		maxPlayers = pPlaylist->rotExtInfo.m_maxPlayers;
	}

	ICVar* pMaxPlayersCVar = gEnv->pConsole->GetCVar("sv_maxplayers");
	if (pMaxPlayersCVar)
	{
		pMaxPlayersCVar->Set(maxPlayers);
	}
}

//---------------------------------------
void  CPlaylistUpr::SetActiveVariant(i32 variantIndex)
{
	DRX_ASSERT((variantIndex >= 0) && (variantIndex < (i32)m_variants.size()));
	m_activeVariantIndex = variantIndex;

	SetModeOptions();
}

//---------------------------------------
ILevelRotation* CPlaylistUpr::GetRotationForCurrentPlaylist()
{
	DrxLog("CPlaylistUpr::GetRotationForCurrentPlaylist()");

	ILevelRotation*  pRot = NULL;

	ILevelSystem*  pLevelSystem = g_pGame->GetIGameFramework()->GetILevelSystem();
	DRX_ASSERT(pLevelSystem);

	if (HavePlaylistSet())
	{
		if (SPlaylist* pList=FindPlaylistById(m_currentPlaylistId))
		{
			pRot = pLevelSystem->FindLevelRotationForExtInfoId(pList->id);
		}
		else
		{
			DrxLog("  Error: couldn't find playlist matching m_currentPlaylistId '%u', so returning NULL", m_currentPlaylistId);
		}
	}
	else
	{
		DrxLog("  m_currentPlaylistId is not set, so returning NULL");
	}

	return pRot;
}

//---------------------------------------
ILevelRotation* CPlaylistUpr::GetLevelRotation()
{
	ILevelRotation*  lrot = NULL;
	if (HavePlaylistSet())
	{
		lrot = GetRotationForCurrentPlaylist();
		DRX_ASSERT(lrot);
	}
	else
	{
		ILevelSystem*  pLevelSystem = g_pGame->GetIGameFramework()->GetILevelSystem();
		DRX_ASSERT(pLevelSystem);
		lrot = pLevelSystem->GetLevelRotation();
	}
	return lrot;
}

//---------------------------------------
bool CPlaylistUpr::DisablePlaylist(tukk uniqueName)
{
	bool  disabled = false;

	if (SPlaylist* pList=FindPlaylistByUniqueName(uniqueName))
	{
		pList->SetEnabled(false);
		DrxLog("CPlaylistUpr::DisablePlaylist: disabled playlist '%s'", uniqueName);

		if (m_currentPlaylistId == pList->id)
		{
			DrxLog("... playlist being disabled was set as current, so un-setting current");
			SetCurrentPlaylist(0);
		}

		disabled = true;
	}

	return disabled;
}

//---------------------------------------
void CPlaylistUpr::AddPlaylistsFromPath(tukk pPath)
{
	DrxLog ("Adding playlists from path '%s'", pPath);
	INDENT_LOG_DURING_SCOPE();

	DrxStringLocal  indexPath;
	indexPath.Format("%s/PlaylistIndex.xml", pPath);

	if (XmlNodeRef indexRoot=GetISystem()->LoadXmlFromFile(indexPath))
	{
		DRX_ASSERT(gEnv);
		i32  n = indexRoot->getChildCount();
		for (i32 i=0; i<n; i++)
		{
			if (XmlNodeRef child=indexRoot->getChild(i))
			{
				tukk  pTag = child->getTag();
				if (stricmp(pTag, "PlaylistFile") == 0)
				{
					bool allow = true;
					i32 allowVal = 1;

					if (child->getAttr("EPD", allowVal))
					{
						//allow = ((allowVal&g_pGameCVars->g_EPD)!=0);
						allow = (allowVal == g_pGameCVars->g_EPD);
						DrxLog ("Playlist '%s' EPD=%u so %s", child->getAttr("name"), allowVal, allow ? "valid" : "invalid");
					}
					else
					{
						allow = (g_pGameCVars->g_EPD==0);
					}

					if (tukk pName = allow ? child->getAttr("name") : NULL)
					{
						DrxStringLocal  filePath;
						filePath.Format("%s/%s", pPath, pName);
						
						if (XmlNodeRef fileNode=gEnv->pSystem->LoadXmlFromFile(filePath))
						{
							AddPlaylistFromXmlNode(fileNode);
						}
						else
						{
							LOCAL_WARNING(0, string().Format("Failed to load '%s'!", filePath.c_str()).c_str());
						}
					}
					else
					{
						LOCAL_WARNING(!allow, string().Format("PlaylistFile element '%s' in '%s' didn't have a \"name\" attribute", pTag, indexPath.c_str()).c_str());
					}
				}
				else
				{
					LOCAL_WARNING(0, string().Format("Unexpected xml child '%s' in '%s'", pTag, indexPath.c_str()).c_str());
				}

			}

		}

	}
	else
	{
		LOCAL_WARNING(0, string().Format("Couldn't open playlists index file '%s'", indexPath.c_str()).c_str());
	}

}

//---------------------------------------
ILevelRotation::TExtInfoId CPlaylistUpr::GetPlaylistId( tukk pUniqueName ) const
{
	const ILevelRotation::TExtInfoId  genId = CCrc32::ComputeLowercase(pUniqueName);
	return genId;
}

//---------------------------------------
void CPlaylistUpr::AddPlaylistFromXmlNode(XmlNodeRef xmlNode)
{
	m_playlists.push_back(SPlaylist());
	SPlaylist*  p = &m_playlists.back();

	p->Reset();

	if (p->rotExtInfo.LoadFromXmlNode(xmlNode))
	{
		DRX_ASSERT(gEnv);
		const ILevelRotation::TExtInfoId  genId = GetPlaylistId(p->rotExtInfo.uniqueName.c_str());

		if (!FindPlaylistById(genId))
		{
			ILevelSystem*  pLevelSystem = g_pGame->GetIGameFramework()->GetILevelSystem();
			DRX_ASSERT(pLevelSystem);

			if (pLevelSystem->AddExtendedLevelRotationFromXmlRootNode(xmlNode, "playlist", genId))
			{
				p->id = genId;
				DRX_ASSERT(FindPlaylistById(genId));
				DrxLog("CPlaylistUpr::AddPlaylistFromXmlNode: added new playlist '%s' (enabled: %d) and gave it an id of '%u'", p->rotExtInfo.uniqueName.c_str(), p->IsEnabled(), p->id);

				ILevelRotation*  pRot = pLevelSystem->FindLevelRotationForExtInfoId(genId);
				DRX_ASSERT_TRACE(pRot, ("Cannot find newly added extended rotation '%s' with id '%u'", p->rotExtInfo.uniqueName.c_str(), genId));
			}
			else
			{
				LOCAL_WARNING(0, "Failed to add extended level rotation!");
				m_playlists.pop_back();
			}
		}
		else
		{
			LOCAL_WARNING(0, string().Format("Error adding loaded playlist as a playlist with id '%u' has already been added!", genId).c_str());
			m_playlists.pop_back();
		}
	}
	else
	{
		LOCAL_WARNING(0, "Failed to load extended level rotation info from xml");
		m_playlists.pop_back();
	}
}

//---------------------------------------
SPlaylist* CPlaylistUpr::FindPlaylistById(ILevelRotation::TExtInfoId findId)
{
	SPlaylist*  pList = NULL;

	TPlaylists::iterator  begin = m_playlists.begin();
	TPlaylists::iterator  end = m_playlists.end();
	for (TPlaylists::iterator i = begin; i != end; ++i)
	{
		SPlaylist*  pIterList = &(*i);
		if (pIterList->id == findId)
		{
			pList = pIterList;
			break;
		}
	}

	return pList;
}

//---------------------------------------
SPlaylist* CPlaylistUpr::FindPlaylistByUniqueName(tukk uniqueName)
{
	DRX_ASSERT(gEnv);
	const ILevelRotation::TExtInfoId  findId = GetPlaylistId(uniqueName);
	return FindPlaylistById(findId);
}

//---------------------------------------
i32k CPlaylistUpr::GetNumPlaylists() const
{
	return m_playlists.size();
}

//---------------------------------------
const SPlaylist* CPlaylistUpr::GetPlaylist(i32k index)
{
	if ((index >= 0) && (index < (i32)m_playlists.size()))
	{
		SPlaylist *pList = &m_playlists[index];
		pList->ResolveVariants();
		return pList;
	}

	return NULL;
}

//---------------------------------------
const SPlaylist* CPlaylistUpr::GetCurrentPlaylist()
{
	if (HavePlaylistSet())
	{
		SPlaylist *pList = FindPlaylistById(m_currentPlaylistId);
		if (pList)
		{
			pList->ResolveVariants();
			return pList;
	}
	}

	return NULL;
}


//---------------------------------------
i32 CPlaylistUpr::GetPlaylistIndex( ILevelRotation::TExtInfoId findId ) const
{
	i32 iFound = -1;
	for( uint iPlaylist = 0; iPlaylist < m_playlists.size(); ++iPlaylist )
	{
		if( m_playlists[ iPlaylist ].id == findId )
		{
			iFound = iPlaylist;
			break;
		}
	}
	return iFound;
}

//---------------------------------------
tukk CPlaylistUpr::GetActiveVariant()
{
	tukk pResult = "";

	if (m_activeVariantIndex >= 0)
	{
		pResult = m_variants[m_activeVariantIndex].m_name.c_str();
	}

	return pResult;
}

//---------------------------------------
i32k CPlaylistUpr::GetActiveVariantIndex() const
{
	return m_activeVariantIndex;
}

//---------------------------------------
tukk CPlaylistUpr::GetVariantName(i32k index)
{
	tukk pResult = NULL;

	if ((index >= 0) && (index < (i32)m_variants.size()))
	{
		pResult = m_variants[index].m_name.c_str();
	}

	return pResult;
}

//---------------------------------------
i32 CPlaylistUpr::GetVariantIndex(tukk pName)
{
	i32 result = -1;

	i32k numVariants = m_variants.size();
	for (i32 i = 0; i < numVariants; ++ i)
	{
		if (!stricmp(m_variants[i].m_name.c_str(), pName))
		{
			result = i;
			break;
		}
	}

	return result;
}

//---------------------------------------
i32k CPlaylistUpr::GetNumVariants() const
{
	return m_variants.size();
}

//---------------------------------------
bool CPlaylistUpr::ChoosePlaylist(i32k chooseIdx)
{
	i32k  n = m_playlists.size();
	if ((chooseIdx >= 0) && (chooseIdx < n))
	{
		SPlaylist*  pList = &m_playlists[chooseIdx];
		const SPlaylistRotationExtension*  pInfo = &pList->rotExtInfo;

		if (pInfo->m_enabled)
		{
			SetCurrentPlaylist(pList->id);

			// Make sure the gamerules are set before the variant so we get correct player limit
			ILevelRotation *pRotation = GetRotationForCurrentPlaylist();
			if (pRotation)
			{
				ICVar *pCVar = gEnv->pConsole->GetCVar("sv_gamerules");
				pCVar->Set(pRotation->GetNextGameRules());
			}

			return true;
		}
		else
		{
			DrxLog("  Error: that playlist cannot be chosen as the current because it is NOT ENABLED");
		}
	}
	else
	{
		DrxLog("  Error: playlist index '%d' is OUT OF RANGE. It should be >= 0 and < %d", chooseIdx, n);
	}

	return false;
}

//---------------------------------------
bool CPlaylistUpr::ChoosePlaylistById( const ILevelRotation::TExtInfoId playlistId )
{
	SPlaylist *pPlaylist = FindPlaylistById(playlistId);
	if (pPlaylist && pPlaylist->IsEnabled())
	{
		SetCurrentPlaylist(playlistId);
		return true;
	}
	else
	{
		// TODO: Replace this assert with a "you can't join this game" message
		DRX_ASSERT(!"Failed to choose playlist by Id, playlist not found or not enabled");
		SetCurrentPlaylist(0);
		return false;
	}
}

//---------------------------------------
bool CPlaylistUpr::ChooseVariant(i32 selectIdx)
{
	if (HavePlaylistSet())
	{
		if (SPlaylist* pList=FindPlaylistById(m_currentPlaylistId))
		{
			if (selectIdx >= 0)
				{
					SetActiveVariant(selectIdx);
					return true;
				}
				else
				{
				DrxLog("  Error: variant index '%d' is OUT OF RANGE. It should be >= 0 and < %" PRISIZE_T, selectIdx, m_variants.size());
			}
		}
		else
		{
			DrxLog("  Error: couldn't find playlist matching m_currentPlaylistId '%u', so cannot select a variant.", m_currentPlaylistId);
		}
	}
	else
	{
		// No playlist set, must be in a private match, all variants are allowed
		SetActiveVariant(selectIdx);
		return true;
	}

	return false;
}

//---------------------------------------
void CPlaylistUpr::SetModeOptions()
{
	CGameLobby *pLobby = g_pGame->GetGameLobby();
	if (pLobby)
	{
		if ((m_activeVariantIndex == m_customVariantIndex) && (pLobby->IsServer() == false))
		{
			// If we're in the custom variant, bail so we don't modify the settings (otherwise we wouldn't know what to set them back to!)
			// Note: Server still needs to set the cvars since it collects them from the profile - just clients that can't
			return;
		}

		m_bIsSettingOptions = true;

		if (CProfileOptions *pProfileOptions = g_pGame->GetProfileOptions())
		{
			// Use default settings if we're on an actual variant, use profile settings for custom variant
			bool useDefaults = (m_activeVariantIndex != m_customVariantIndex);

			IGameRulesSystem *pGameRulesSystem = gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem();
			tukk  sv_gamerules = gEnv->pConsole->GetCVar("sv_gamerules")->GetString();

			tukk  fullGameRulesName = (pLobby && pLobby->IsCurrentlyInSession()) ? pLobby->GetCurrentGameModeName(sv_gamerules) : pGameRulesSystem->GetGameRulesName(sv_gamerules);

			if (fullGameRulesName)
			{
				i32k numOptions = m_options.size();
				for (i32 i = 0; i < numOptions; ++ i)
				{
					SGameModeOption &option = m_options[i];
					option.CopyToCVar(pProfileOptions, useDefaults, fullGameRulesName);
				}
			}

		}

		IConsole *pConsole = gEnv->pConsole;
		DRX_ASSERT(pConsole && (m_activeVariantIndex >= 0));
		if (pConsole && (m_activeVariantIndex >= 0))
		{
			SGameVariant &activeVariant = m_variants[m_activeVariantIndex];

			i32k numOptions = activeVariant.m_options.size();
			for (i32 i = 0; i < numOptions; ++ i)
			{
				tukk pOption = activeVariant.m_options[i].c_str();
				pConsole->ExecuteString(pOption);
			}
		}

		pLobby->OnOptionsChanged();

		m_bIsSettingOptions = false;
	}
}

//---------------------------------------
void CPlaylistUpr::AddVariantsFromPath(tukk pPath)
{
	DrxStringLocal  indexPath;
	indexPath.Format("%s/VariantsIndex.xml", pPath);

	if (XmlNodeRef indexRoot=GetISystem()->LoadXmlFromFile(indexPath))
	{
		i32  n = indexRoot->getChildCount();
		for (i32 i=0; i<n; i++)
		{
			if (XmlNodeRef child=indexRoot->getChild(i))
			{
				tukk  pTag = child->getTag();
				if (stricmp(pTag, "VariantsFile") == 0)
				{
					if (tukk pName=child->getAttr("name"))
					{
						DrxStringLocal  filePath;
						filePath.Format("%s/%s", pPath, pName);
						
						LoadVariantsFile(filePath.c_str());
					}
					else
					{
						LOCAL_WARNING(0, string().Format("VariantsFile element '%s' in '%s' didn't have a \"name\" attribute", pTag, indexPath.c_str()).c_str());
					}
				}
				else
				{
					LOCAL_WARNING(0, string().Format("Unexpected xml child '%s' in '%s'", pTag, indexPath.c_str()).c_str());
				}
			}
		}
	}
	else
	{
		LOCAL_WARNING(0, string().Format("Couldn't open playlists index file '%s'", indexPath.c_str()).c_str());
	}

}

//---------------------------------------
void CPlaylistUpr::LoadVariantsFile(tukk pPath)
{
	DRX_ASSERT(gEnv);
	if (XmlNodeRef xmlRoot = gEnv->pSystem->LoadXmlFromFile(pPath))
	{
		i32k numChildren = xmlRoot->getChildCount();
		for (i32 i = 0; i < numChildren; ++ i)
		{
			XmlNodeRef xmlChild = xmlRoot->getChild(i);
			AddVariantFromXmlNode(xmlChild);
		}
	}
	else
	{
		LOCAL_WARNING(0, string().Format("Failed to load '%s'!", "%s", pPath).c_str());
	}
}

//---------------------------------------
void CPlaylistUpr::AddVariantFromXmlNode( XmlNodeRef xmlNode )
{
	if (!stricmp(xmlNode->getTag(), "Variant"))
	{
		tukk pName = 0;
		if (xmlNode->getAttr("name", &pName))
		{
			SGameVariant newVariant;

			newVariant.m_name = pName;

			tukk  pCurrentLang = gEnv->pSystem->GetLocalizationUpr()->GetLanguage();
			DRX_ASSERT(pCurrentLang);

			CPlaylistUpr::GetLocalisationSpecificAttr<SGameVariant::TFixedString>(xmlNode, "localName", pCurrentLang, &newVariant.m_localName);

			CPlaylistUpr::GetLocalisationSpecificAttr<SGameVariant::TLongFixedString>(xmlNode, "localDescription", pCurrentLang, &newVariant.m_localDescription);

			CPlaylistUpr::GetLocalisationSpecificAttr<SGameVariant::TLongFixedString>(xmlNode, "localDescriptionUpper", pCurrentLang, &newVariant.m_localDescriptionUpper);
			if (newVariant.m_localDescriptionUpper.empty())
			{
				if (!newVariant.m_localDescription.empty() && (newVariant.m_localDescription.at(0) == '@'))
				{
					newVariant.m_localDescriptionUpper.Format("%s_upper", newVariant.m_localDescription.c_str());
				}
				else
				{
					LOCAL_WARNING(0, string().Format("AddVariantFromXmlNode: no \"localDescriptionUpper\" attribute inside variant '%s', and can't auto-reference a string-table entry based on the \"localDescription\" attribute because that attribute is set explicitly as opposed to being a \"@xxxxx\"-style string-table reference. Will copy the lowercase attribute, as that's probably better than nothing!", newVariant.m_name.c_str()).c_str());
					newVariant.m_localDescriptionUpper = newVariant.m_localDescription;
				}
			}

			i32 iValue = 0;
			if (xmlNode->getAttr("enabled", iValue))
			{
				newVariant.m_enabled = (iValue != 0);
			}

			if (xmlNode->getAttr("requiresUnlocking", iValue))
			{
				newVariant.m_requiresUnlock = (iValue != 0);
			}

			if (xmlNode->getAttr("allowInCustomGames", iValue))
			{
				newVariant.m_allowInCustomGames = (iValue != 0);
			}

			if (xmlNode->getAttr("restrictRank", iValue))
			{
				newVariant.m_restrictRank = iValue;
			}

			if (xmlNode->getAttr("requireRank", iValue))
			{
				newVariant.m_requireRank = iValue;
			}

			if (xmlNode->getAttr("allowSquads", iValue))
			{
				newVariant.m_allowSquads = (iValue != 0);
			}

			if (xmlNode->getAttr("allowClans", iValue))
			{
				newVariant.m_allowClans = (iValue != 0);
			}

			XmlString stringValue;
			if (xmlNode->getAttr("imgsuffix", stringValue))
			{
				newVariant.m_suffix = stringValue.c_str();
			}

			bool isDefault = false;
			if (xmlNode->getAttr("default", iValue))
			{
				isDefault = (iValue != 0);
			}

			bool bOk = true;

			i32k numChildren = xmlNode->getChildCount();
			for (i32 i = 0; i < numChildren; ++ i)
			{
				XmlNodeRef xmlChild = xmlNode->getChild(i);

				if (!stricmp(xmlChild->getTag(), "Options"))
				{
					i32k numOptions = xmlChild->getChildCount();
					for (i32 j = 0; j < numOptions; ++ j)
					{
						XmlNodeRef xmlOption = xmlChild->getChild(j);
						if (!stricmp(xmlOption->getTag(), "Option"))
						{
							tukk pSetting = 0;
							if (xmlOption->getAttr("setting", &pSetting))
							{
								newVariant.m_options.push_back(pSetting);
							}
							else
							{
								LOCAL_WARNING(0, string().Format("AddVariantFromXmlNode: Expected 'setting' attribute, inside variant '%s'", newVariant.m_name.c_str()).c_str());
								bOk = false;
							}
						}
						else
						{
							LOCAL_WARNING(0, string().Format("AddVariantFromXmlNode: Expected 'Option' tag, got '%s' instead, inside variant '%s'", xmlOption->getTag(), newVariant.m_name.c_str()).c_str());
							bOk = false;
						}
					}
				}
				else if (!stricmp(xmlChild->getTag(), "Modes"))
				{
					newVariant.m_supportsAllModes = false;
					i32k numModes = xmlChild->getChildCount();
					for (i32 j = 0; j < numModes; ++ j)
					{
						XmlNodeRef xmlMode = xmlChild->getChild(j);
						if (!stricmp(xmlMode->getTag(), "Mode"))
						{
							tukk pSetting = 0;
							if (xmlMode->getAttr("name", &pSetting))
							{
								i32 rulesId = 0;
								if (AutoEnum_GetEnumValFromString(pSetting, CGameRules::S_GetGameModeNamesArray(), eGM_NUM_GAMEMODES, &rulesId))
								{
									newVariant.m_supportedModes.push_back(rulesId);
								}
								else
								{
									LOCAL_WARNING(0, string().Format("AddVariantFromXmlNode: Unknown mode '%s' found, inside variant '%s'", pSetting, newVariant.m_name.c_str()).c_str());
									bOk = false;
								}
							}
						}
						else
						{
							LOCAL_WARNING(0, string().Format("AddVariantFromXmlNode: Expected 'Mode' tag, got '%s' instead, inside variant '%s'", xmlMode->getTag(), newVariant.m_name.c_str()).c_str());
							bOk = false;
						}
					}
				}
				else
				{
					LOCAL_WARNING(0, string().Format("AddVariantFromXmlNode: Unexpected '%s' tag, inside variant '%s'", xmlChild->getTag(), newVariant.m_name.c_str()).c_str());
					bOk = false;
				}
			}
			if (bOk)
			{
				newVariant.m_id = m_variants.size();
				m_variants.push_back(newVariant);

				if (isDefault)
				{
					m_defaultVariantIndex = newVariant.m_id;
				}
			}
		}
		else
		{
			LOCAL_WARNING(0, "AddVariantFromXmlNode: No name attribute specified");
		}
	}
	else
	{
		LOCAL_WARNING(0, string().Format("AddVariantFromXmlNode: Expected node tag to be 'Variant', got %s instead", xmlNode->getTag()).c_str());
	}
}

//---------------------------------------
SGameVariant *CPlaylistUpr::FindVariantByName(tukk pName)
{
	SGameVariant *pResult = NULL;

	i32k numVariants = m_variants.size();
	for (i32 i = 0; i < numVariants; ++ i)
	{
		SGameVariant *pVariant = &m_variants[i];
		if (!stricmp(pVariant->m_name.c_str(), pName))
		{
			pResult = pVariant;
		break;
		}
	}

	return pResult;
}

//---------------------------------------
bool CPlaylistUpr::DisableVariant(tukk pName)
{
	bool result = false;
	if (SGameVariant *pVariant = FindVariantByName(pName))
	{
		pVariant->m_enabled = false;
		result = true;
	}
	return result;
}

//---------------------------------------
void CPlaylistUpr::GetVariantsForGameMode( tukk pGameMode, TVariantsPtrVec &result )
{
	if ((!pGameMode) || !(pGameMode[0]))
	{
		return;
	}

	i32 gamemodeId = 0;
	if (!AutoEnum_GetEnumValFromString(pGameMode, CGameRules::S_GetGameModeNamesArray(), eGM_NUM_GAMEMODES, &gamemodeId))
	{
		return;
	}

	i32k numVariants = m_variants.size();

	result.clear();
	result.reserve(numVariants);

	for (i32 i = 0; i < numVariants; ++ i)
	{
		SGameVariant *pVariant = &m_variants[i];
		if (pVariant->m_enabled && pVariant->m_allowInCustomGames && (pVariant->m_id != m_customVariantIndex))
		{
			if (pVariant->m_supportsAllModes)
			{
				result.push_back(pVariant);
			}
			else
			{
				i32k numSupportedModes = pVariant->m_supportedModes.size();
				for (i32 j = 0; j < numSupportedModes; ++ j)
				{
					if (pVariant->m_supportedModes[j] == gamemodeId)
					{
						result.push_back(pVariant);
						break;
					}
				}
			}
		}
	}

	// Add custom variant last
	if (m_customVariantIndex != -1)
	{
		SGameVariant *pCustomVariant = &m_variants[m_customVariantIndex];
		result.push_back(pCustomVariant);
	}
}

//---------------------------------------
void CPlaylistUpr::GetTotalPlaylistCounts(u32 *outUnlocked, u32 *outTotal)
{
	u32 total = *outTotal;
	u32 unlocked = *outUnlocked;

	i32k num = m_playlists.size();
	if (num>0)
	{
		// Playlists
		for (i32 i=0; i<num; ++i)
		{
			const SPlaylist *pPlaylist = &m_playlists[i];
			if (pPlaylist && pPlaylist->IsEnabled() && pPlaylist->IsSelectable() && !pPlaylist->IsHidden())
			{
				// Variants
				i32k numVariants = pPlaylist->rotExtInfo.m_supportedVariants.size();
				for (i32 j = 0; j < numVariants; ++j)
				{
					tukk pVariantName = pPlaylist->rotExtInfo.m_supportedVariants[j].m_name.c_str();
					i32 variantIndex = GetVariantIndex(pVariantName);
					
					if (const SGameVariant *pVariant = GetVariant(variantIndex))
					{
						++total;
						if (HaveUnlockedPlaylist(pPlaylist) && HaveUnlockedPlaylistVariant(pPlaylist, variantIndex))
						{
							++unlocked;
						}
					}
				}
			}
		}
	}

	*outTotal = total;
	*outUnlocked = unlocked;
}

//---------------------------------------
void CPlaylistUpr::SetGameModeOption( tukk pOption, tukk pValue )
{
	if (m_activeVariantIndex != m_customVariantIndex)
	{
		DrxLog("CPlaylistUpr::SetGameModeOption() option '%s' has been changed, switching to custom variant", pOption);
		// Copy current default options into profile
		SaveCurrentSettingsToProfile();

		SetActiveVariant(m_customVariantIndex);
	}
	CProfileOptions *pProfileOptions = g_pGame->GetProfileOptions();
	pProfileOptions->SetOptionValue(pOption, pValue);

	// Need to set options immediately in order for the restrictions to activate
	SGameModeOption *pOptionStruct = GetGameModeOptionStruct(pOption);
	if (pOptionStruct)
	{
		IGameRulesSystem *pGameRulesSystem = g_pGame->GetIGameFramework()->GetIGameRulesSystem();
		CGameLobby *pLobby = g_pGame->GetGameLobby();
		tukk pGameRulesCVarString = gEnv->pConsole->GetCVar("sv_gamerules")->GetString();
		tukk pActualRulesName = (pLobby && pLobby->IsCurrentlyInSession()) ? pLobby->GetCurrentGameModeName(pGameRulesCVarString) : pGameRulesSystem->GetGameRulesName(pGameRulesCVarString);
		if (pActualRulesName)
		{
			pOptionStruct->CopyToCVar(pProfileOptions, false, pActualRulesName);
		}
	}
}

//---------------------------------------
void CPlaylistUpr::GetGameModeOption(tukk pOption, DrxFixedStringT<32> &result)
{
	if (m_activeVariantIndex != m_customVariantIndex)
	{
		// Normal variant (i.e. not the custom one), take the current cvar value (defaults get changed by the variants)
		SGameModeOption *pOptionStruct = GetGameModeOptionStruct(pOption);
		DRX_ASSERT(pOptionStruct && pOptionStruct->m_pCVar);
		if (pOptionStruct && pOptionStruct->m_pCVar)
		{
			if (pOptionStruct->m_pCVar->GetType() == CVAR_INT)
			{
				result.Format("%d", pOptionStruct->m_pCVar->GetIVal());
			}
			else if (pOptionStruct->m_pCVar->GetType() == CVAR_FLOAT)
			{
				const float scaledFloat = pOptionStruct->m_pCVar->GetFVal() / pOptionStruct->m_profileMultiplyer;
				if ((pOptionStruct->m_floatPrecision == 0) || (floorf(scaledFloat) == scaledFloat))
				{
					result.Format("%d", int_round(scaledFloat));
				}
				else
				{
					DrxFixedStringT<8> formatString;
					formatString.Format("%%.%df", pOptionStruct->m_floatPrecision);

					result.Format(formatString.c_str(), scaledFloat);
				}
			}
		}
	}
	else
	{
		CProfileOptions *pProfileOptions = g_pGame->GetProfileOptions();
		DRX_ASSERT(pProfileOptions->IsOption(pOption));
		if (pProfileOptions->IsOption(pOption))
		{
			// Custom variant, use whatever value is in the profile
			string optionValue = pProfileOptions->GetOptionValue(pOption, false);
			result = optionValue.c_str();
		}
	}
}

//---------------------------------------
void CPlaylistUpr::GetGameModeProfileOptionName(u32k index, DrxFixedStringT<32> &result)
{
	if (index < m_options.size())
	{
		SGameModeOption &option = m_options[index];
		result = option.m_profileOption.c_str();
	}
}

u16 CPlaylistUpr::PackCustomVariantOption(u32 index)
{
	if (index < m_options.size())
	{
		SGameModeOption &option = m_options[index];
		DRX_ASSERT(option.m_pCVar);
		i32 iValue = 0;
		i32 cvarType = option.m_pCVar->GetType();
		
		if (option.m_pCVar)
		{
			if (cvarType == CVAR_INT)
			{
				iValue = option.m_pCVar->GetIVal();
			}
			else if (cvarType == CVAR_FLOAT)
			{
				// Convert into i32 (same as if saving to the profile)
				iValue = int_round((option.m_pCVar->GetFVal() / option.m_netMultiplyer));
			}

			//DrxLog("Write %d %d %f %d", index, iValue, option.m_pCVar->GetFVal(), cvarType);

			// Add a constant amount (to allow for negative values)
			u16 valueToSend = (u16) (iValue + MAX_ALLOWED_NEGATIVE_OPTION_AMOUNT);
			return valueToSend;
		}
	}

	return 0;
}

// Unpacks both i32 and float as some Gamemode options have both as options e.g. timelimit 2.5f. The option will know which it wants to use.
i32 CPlaylistUpr::UnpackCustomVariantOptionProfileValues(u16 value, u32 index, i32* pIntValue, float* pFloatValue, i32* pFloatPrecision)
{
	if (index < m_options.size())
	{
		SGameModeOption &option = m_options[index];
		DRX_ASSERT(option.m_pCVar);
		if (option.m_pCVar)
		{
			// Read the value and convert back into an i32
			i32 iValue = ((i32)value) - MAX_ALLOWED_NEGATIVE_OPTION_AMOUNT;
			if (option.m_pCVar->GetType() == CVAR_INT) 
			{
				if (pIntValue)
				{
					*pIntValue = iValue;
				}

				if (pFloatValue)
				{
					*pFloatValue = (float)iValue;
				}
			}
			else if (option.m_pCVar->GetType() == CVAR_FLOAT)
			{
				float fValue = ((float)iValue * option.m_netMultiplyer) / option.m_profileMultiplyer;

				if ((option.m_floatPrecision == 0) || (floorf(fValue) == fValue))
				{
					i32k roundedIntValue = int_round(fValue);
					if (pIntValue)
					{
						*pIntValue = roundedIntValue;
					}

					if (pFloatValue)
					{
						*pFloatValue = (float)roundedIntValue;
					}
				}
				else
				{
					if (pFloatValue)
					{
						*pFloatValue = fValue;
					}

					if (pIntValue)
					{
						*pIntValue = iValue;
					}
				}

				if (pFloatPrecision)
				{
					*pFloatPrecision = option.m_floatPrecision;
				}
			}

			return option.m_pCVar->GetType();
		}
	}	

	return 0;
}

i32 CPlaylistUpr::UnpackCustomVariantOption(u16 value, u32 index, i32* pIntValue, float* pFloatValue)
{
	if (index < m_options.size())
	{
		SGameModeOption &option = m_options[index];
		DRX_ASSERT(option.m_pCVar);
		if (option.m_pCVar)
		{
			// Read the value and convert back into an i32
			i32 iValue = ((i32)value) - MAX_ALLOWED_NEGATIVE_OPTION_AMOUNT;

			if (option.m_pCVar->GetType() == CVAR_INT) 
			{
				if (pIntValue)
				{
					*pIntValue = iValue;
					return CVAR_INT;
				}
			}
			else if (option.m_pCVar->GetType() == CVAR_FLOAT)
			{
				// Convert into float (same as if reading from the profile)
				float fValue = ((float)iValue) * option.m_netMultiplyer;

				if (pFloatValue)
				{
					*pFloatValue = fValue;
					return CVAR_FLOAT;
				}
			}
		}
	}	

	return 0;
}

void CPlaylistUpr::ReadDetailedServerInfo(u16 *pOptions, u32 numOptions)
{
	DrxLog("CPlaylistUpr::ReadDetailedServerInfo numOptions %d", numOptions);

	for (u32 i = 0; i < numOptions; ++ i)
	{
		SGameModeOption &option = m_options[i];
		DRX_ASSERT(option.m_pCVar);
		if (option.m_pCVar)
		{
			// Read the value and convert back into an i32
			i32 iValue = 0;
			float fValue = 0.f;
			i32 cvarType = 0;
			
			m_bIsSettingOptions = true;

			cvarType = UnpackCustomVariantOption(pOptions[i], i, &iValue, &fValue);
			if (cvarType == CVAR_INT)
			{
				option.m_pCVar->Set(iValue);
			}
			else if (cvarType == CVAR_FLOAT)
			{
				option.m_pCVar->Set(fValue);
			}

			m_bIsSettingOptions = false;
			//DrxLog("Read %d %d %f %d", i, iValue, fValue, cvarType);
		}
	}
}


void CPlaylistUpr::WriteSetCustomVariantOptions(CDrxLobbyPacket* pPacket, CPlaylistUpr::TOptionsVec pOptions, u32 numOptions)
{
	for (u32 i = 0; i < numOptions; ++ i)
	{
		SGameModeOption &option = pOptions[i];
		DRX_ASSERT(option.m_pCVar);
		if (option.m_pCVar)
		{
			u16 valueToSend = PackCustomVariantOption(i);
			pPacket->WriteUINT16(valueToSend);
		}
		else
		{
			// Have to write something because the other end is going to read something!
			pPacket->WriteUINT16(0);
		}
	}
}

//---------------------------------------
void CPlaylistUpr::WriteSetVariantPacket( CDrxLobbyPacket *pPacket )
{
	i32k variantNum = m_activeVariantIndex;
	DRX_ASSERT(variantNum > -1);
	if (variantNum > -1)
	{
		const bool bIsCustomVariant = (m_activeVariantIndex == m_customVariantIndex);

		if (!bIsCustomVariant)
		{
			u32k bufferSize = DrxLobbyPacketHeaderSize + DrxLobbyPacketUINT8Size + GetSynchedVarsSize();
			if (pPacket->CreateWriteBuffer(bufferSize))
			{
				pPacket->StartWrite(eGUPD_SetGameVariant, true);
				pPacket->WriteUINT8((u8) variantNum);
				WriteSynchedVars(pPacket);
			}
		}
		else
		{
			// Custom variant, need to send all the options as well :-(
			i32k numOptions = m_options.size();

			// This is horribly large at the moment, lots of scope for packing it properly at a later date
			u32k bufferSize = DrxLobbyPacketHeaderSize + DrxLobbyPacketUINT8Size + (DrxLobbyPacketUINT16Size * numOptions) + GetSynchedVarsSize();
			if (pPacket->CreateWriteBuffer(bufferSize))
			{
				pPacket->StartWrite(eGUPD_SetGameVariant, true);
				pPacket->WriteUINT8((u8) variantNum);
				WriteSynchedVars(pPacket);
				WriteSetCustomVariantOptions(pPacket, m_options, numOptions);
			}
		}
	}
}

//---------------------------------------
i32 CPlaylistUpr::GetSynchedVarsSize()
{
	i32 size = 0;
	i32 numVars = m_configVars.size();
	for (i32 i = 0; i < numVars; ++ i)
	{
		if (m_configVars[i].m_bNetSynched)
		{
			if(m_configVars[i].m_pCVar->GetType() != CVAR_STRING)
			{
				size += DrxLobbyPacketUINT16Size;				
			}
			else
			{
				size += DrxLobbyPacketUINT8Size;
				size += strlen(m_configVars[i].m_pCVar->GetString());
			}
		}
	}
	return size;
}

//---------------------------------------
void CPlaylistUpr::WriteSynchedVars(CDrxLobbyPacket* pPacket)
{
	i32 numVars = m_configVars.size();
	for (i32 i = 0; i < numVars; ++ i)
	{
		if (m_configVars[i].m_bNetSynched)
		{
			switch (m_configVars[i].m_pCVar->GetType())
			{
			case CVAR_INT:
				pPacket->WriteUINT16((u16)m_configVars[i].m_pCVar->GetIVal());
				break;
			case CVAR_FLOAT:
				pPacket->WriteUINT16((u16)m_configVars[i].m_pCVar->GetFVal());
				break;
			case CVAR_STRING:
				DrxFixedStringT<24> temp;
				temp.Format("%s", m_configVars[i].m_pCVar->GetString());
				u8k length = static_cast<u8> (strlen(m_configVars[i].m_pCVar->GetString()) + 1); //+1 == escape character
				pPacket->WriteUINT8(length);
				pPacket->WriteString(temp.c_str(), length);
				break;
			}
		}
	}
}

//---------------------------------------
void CPlaylistUpr::ReadSynchedVars(CDrxLobbyPacket* pPacket)
{
	i32 numVars = m_configVars.size();
	for (i32 i = 0; i < numVars; ++ i)
	{
		if (m_configVars[i].m_bNetSynched)
		{
			switch (m_configVars[i].m_pCVar->GetType())
			{
			case CVAR_INT:
				{
					u16k intVal = pPacket->ReadUINT16();
					m_configVars[i].m_pCVar->Set((i32) intVal);
					break;
				}
			case CVAR_FLOAT:
				{
					u16k floatVal = pPacket->ReadUINT16();
					m_configVars[i].m_pCVar->Set((float) floatVal);
					break;
				}
			case CVAR_STRING:
				{
					u8k length = pPacket->ReadUINT8();
					char strVal[24];
					pPacket->ReadString(strVal, length);
					m_configVars[i].m_pCVar->Set(strVal);
					break;
				}
			}
		}
	}
}

//---------------------------------------
void CPlaylistUpr::ReadSetCustomVariantOptions(CDrxLobbyPacket* pPacket, CPlaylistUpr::TOptionsVec pOptions, u32 numOptions)
{
	m_bIsSettingOptions = true;
	for (u32 i = 0; i < numOptions; ++ i)
	{
		SGameModeOption &option = m_options[i];
		DRX_ASSERT(option.m_pCVar);
		if (option.m_pCVar)
		{
			// Read the value and convert back into an i32
			i32 iValue = 0;
			float fValue = 0.f;
			i32 cvarType = 0;
			
			cvarType = UnpackCustomVariantOption(pPacket->ReadUINT16(), i, &iValue, &fValue);
			if (cvarType == CVAR_INT)
			{
				option.m_pCVar->Set(iValue);
			}
			else if (cvarType == CVAR_FLOAT)
			{
				option.m_pCVar->Set(fValue);
			}
		}
		else
		{
			// Have to read something because the other end is going to send something!
			pPacket->ReadUINT16();
		}
	}
	m_bIsSettingOptions = false;
}

void CPlaylistUpr::ReadSetVariantPacket( CDrxLobbyPacket *pPacket )
{
	i32k variantNum = (i32) pPacket->ReadUINT8();
	DRX_ASSERT((variantNum >= 0) && (variantNum < (i32)m_variants.size()));

	if ((variantNum >= 0) && (variantNum < (i32)m_variants.size()))
	{
		SetActiveVariant(variantNum);
		ReadSynchedVars(pPacket);

		const bool bIsCustomVariant = (variantNum == m_customVariantIndex);
		if (bIsCustomVariant)
		{
			// Custom variant, need to read all the options as well :-(
			i32k numOptions = m_options.size();

			ReadSetCustomVariantOptions(pPacket, m_options, numOptions);
		}
	}
}

//---------------------------------------
bool CPlaylistUpr::AdvanceRotationUntil(i32k newNextIdx)
{
	bool  ok = false;

	if (HavePlaylistSet())
	{
		if (ILevelRotation* pLevelRotation=GetLevelRotation())
		{
			if (pLevelRotation->GetLength() > 0)
			{
				DRX_ASSERT(newNextIdx < pLevelRotation->GetLength());

				ok = true;

				i32k  oldNext = pLevelRotation->GetNext();

				while (pLevelRotation->GetNext() != newNextIdx)
				{
					if (!pLevelRotation->Advance())
					{
						pLevelRotation->First();
					}

					if (pLevelRotation->GetNext() == oldNext)
					{
						DRX_ASSERT_TRACE(0, ("Caught potential infinite loop whilst looking (and failing to find) newNextIdx '%d' in playlist rotation. Breaking loop.", newNextIdx));
						ok = false;
						break;
					}
				}
			}
		}
	}

	return ok;
}


#if !defined(_RELEASE)  /////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------
void CPlaylistUpr::DbgPlaylistsList()
{
	i32k  n = m_playlists.size();
	if (n > 0)
	{
		DrxLogAlways("  The currently loaded playlists are:");
		for (i32 i=0; i<n; i++)
		{
			SPlaylist*  pIterList = &m_playlists[i];
			const SPlaylistRotationExtension*  pInfo = &pIterList->rotExtInfo;
			const bool  chosen = (pIterList->id == m_currentPlaylistId);
			// [tlh] TODO would be nice to print the localized name here too...
			DrxLogAlways("    %s: %u \"%s\"", (pInfo->m_enabled ? (chosen ? "*" : string().Format("%d", i).c_str()) : "X"), pIterList->id, pInfo->uniqueName.c_str());
		}
		DrxLogAlways("  An \"*\" in the first column denotes the CURRENT chosen playlist (can be none).");
		DrxLogAlways("  An \"X\" in the first column denotes a DISABLED playlist (this playlist cannot be chosen).");
		DrxLogAlways("  Use the \"playlists_choose #\" cmd to CHOOSE one of the above playlists,");
		DrxLogAlways("  or \"playlists_unchoose\" to UNSELECT the currently chosen playlist.");
	}
	else
	{
		DrxLogAlways("  There are currently no playlists loaded, so nothing to show.");
	}
}

//---------------------------------------
void CPlaylistUpr::DbgPlaylistsChoose(i32k chooseIdx)
{
	i32k  n = m_playlists.size();
	if ((chooseIdx >= 0) && (chooseIdx < n))
	{
		SPlaylist*  pList = &m_playlists[chooseIdx];
		const SPlaylistRotationExtension*  pInfo = &pList->rotExtInfo;

		if (pInfo->m_enabled)
		{
			SetCurrentPlaylist(pList->id);

			DrxLogAlways("  You have successfully CHOSEN the following playlist to be the current:");
			// [tlh] TODO would be nice to print the localized name here too...
			DrxLogAlways("    %d: %u \"%s\"", chooseIdx, pList->id, pInfo->uniqueName.c_str());
		}
		else
		{
			DrxLogAlways("  Error: that playlist cannot be chosen as the current because it is NOT ENABLED");
		}
	}
	else
	{
		DrxLogAlways("  Error: playlist index '%d' is OUT OF RANGE. It should be >= 0 and < %d", chooseIdx, n);
	}
}

//---------------------------------------
void CPlaylistUpr::DbgPlaylistsUnchoose()
{
	SetCurrentPlaylist(0);
	DrxLogAlways("  You have successfully UNCHOSEN the current playlist. There is now no current playlist");
}

//---------------------------------------
void CPlaylistUpr::DbgPlaylistsShowVariants()
{
	if (HavePlaylistSet())
	{
		if (SPlaylist* pList=FindPlaylistById(m_currentPlaylistId))
		{
			DrxLogAlways("  The variants supported by the current playlist are:");
			i32k numVariants = pList->rotExtInfo.m_supportedVariants.size();
			for (i32 i = 0; i < numVariants; ++ i)
			{
				tukk pVariantName = pList->rotExtInfo.m_supportedVariants[i].m_name.c_str();
				{
					i32 variantIndex = GetVariantIndex(pVariantName);
					const bool  chosen = (variantIndex == m_activeVariantIndex);
					DrxLogAlways("    %s: \"%s\"", (chosen ? "*" : string().Format("%d", i).c_str()), pVariantName);
				}
			}
			DrxLogAlways("  An \"*\" in the first column denotes the CURRENT active variant.");
			DrxLogAlways("  Use the \"playlists_select_variant #\" cmd to SELECT a different active variant.");
		}
		else
		{
			DrxLog("  Error: couldn't find playlist matching m_currentPlaylistId '%u', so nothing to show.", m_currentPlaylistId);
		}
	}
	else
	{
		DrxLogAlways("  There's currently no playlist selected as current, so nothing to show.");
	}
}

//---------------------------------------
void CPlaylistUpr::DbgPlaylistsSelectVariant(i32 selectIdx)
{
	if (HavePlaylistSet())
	{
		if (SPlaylist* pList=FindPlaylistById(m_currentPlaylistId))
		{
			if ((selectIdx >= 0) && (selectIdx < (i32)m_variants.size()))
			{
				const SPlaylistRotationExtension::TVariantsVec &supportedVariants = pList->GetSupportedVariants();
				bool supported = false;
				i32k numSupportedVariants = supportedVariants.size();
				for (i32 i = 0; i < numSupportedVariants; ++ i)
				{
					if (supportedVariants[i].m_id == selectIdx)
				{
						supported = true;
						break;
					}
				}
				if (supported)
				{
					SetActiveVariant(selectIdx);

					DrxLogAlways("  You have successfully SELECTED the following variant to be active:");
					DrxLogAlways("    %d: \"%s\"", selectIdx, GetVariantName(selectIdx));
				}
				else
				{
					DrxLogAlways("  Error: that variant cannot be selected to be active because it is NOT SUPPORTED by the current playlist");
				}
			}
			else
			{
				DrxLogAlways("  Error: variant index '%d' is OUT OF RANGE. It should be >= 0 and < %" PRISIZE_T, selectIdx, m_variants.size());
			}
		}
		else
		{
			DrxLog("  Error: couldn't find playlist matching m_currentPlaylistId '%u', so cannot select a variant.", m_currentPlaylistId);
		}
	}
	else
	{
		// No playlist set, must be in a private match, all variants are allowed
		SetActiveVariant(selectIdx);
	}
}

//---------------------------------------
// static
void CPlaylistUpr::CmdPlaylistsList(IConsoleCmdArgs* pCmdArgs)
{
	DRX_ASSERT(s_pPlaylistUpr);
	s_pPlaylistUpr->DbgPlaylistsList();
}

//---------------------------------------
// static
void CPlaylistUpr::CmdPlaylistsChoose(IConsoleCmdArgs* pCmdArgs)
{
	if (pCmdArgs->GetArgCount() == 2)
	{
		i32  chooseIdx = atoi(pCmdArgs->GetArg(1));
		DRX_ASSERT(s_pPlaylistUpr);
		s_pPlaylistUpr->DbgPlaylistsChoose(chooseIdx);
	}
	else
	{
		DrxLogAlways("  Usage: \"playlists_choose [index_of_playlist_to_choose]\"");
	}
}

//---------------------------------------
// static
void CPlaylistUpr::CmdPlaylistsUnchoose(IConsoleCmdArgs* pCmdArgs)
{
	DRX_ASSERT(s_pPlaylistUpr);
	s_pPlaylistUpr->DbgPlaylistsUnchoose();
}

//---------------------------------------
// static
void CPlaylistUpr::CmdPlaylistsShowVariants(IConsoleCmdArgs* pCmdArgs)
{
	DRX_ASSERT(s_pPlaylistUpr);
	s_pPlaylistUpr->DbgPlaylistsShowVariants();
}

//---------------------------------------
// static
void CPlaylistUpr::CmdPlaylistsSelectVariant(IConsoleCmdArgs* pCmdArgs)
{
	if (pCmdArgs->GetArgCount() == 2)
	{
		i32k selectIdx = atoi(pCmdArgs->GetArg(1));
		DRX_ASSERT(s_pPlaylistUpr);
		s_pPlaylistUpr->DbgPlaylistsSelectVariant(selectIdx);
	}
	else
	{
		DrxLogAlways("  Usage: \"playlists_select_variant [index_of_variant_to_select]\"");
	}
}

#endif  // !_RELEASE ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//---------------------------------------
// static
void CPlaylistUpr::CmdStartPlaylist(IConsoleCmdArgs* pCmdArgs)
{
	if (pCmdArgs->GetArgCount() == 2)
	{
		tukk pConsoleArg = pCmdArgs->GetArg(1);
		if (!DoStartPlaylistCommand(pConsoleArg))
		{
			DrxLogAlways("Invalid option");
		}
	}
	else
	{
		DrxLogAlways("Invalid format, use 'startPlaylist <playlist>'");
	}
}

//---------------------------------------
// static
bool CPlaylistUpr::DoStartPlaylistCommand( tukk pPlaylistArg )
{
	DRX_ASSERT(gEnv);
	// Use the auto complete struct so that we compare against the same strings that are used in the auto-complete
	bool success = false;
	i32k numPossibleOptions = gl_PlaylistVariantAutoComplete.GetCount();
	for (i32 i = 0; i < numPossibleOptions; ++ i)
	{
		const SPlaylist *pPlaylist = NULL;
		const SGameVariant *pVariant = NULL;
		if (gl_PlaylistVariantAutoComplete.GetPlaylistAndVariant(i, &pPlaylist, &pVariant))
		{
			tukk pOptionName = gl_PlaylistVariantAutoComplete.GetOptionName(pPlaylist, pVariant);
			if (!stricmp(pPlaylistArg, pOptionName))
			{
				CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
				if (pPlaylistUpr)
				{
					// Activate the playlist and variant
					pPlaylistUpr->SetCurrentPlaylist(pPlaylist->id);
					pPlaylistUpr->SetActiveVariant(pVariant->m_id);

					i32 numOptions = pPlaylistUpr->m_options.size();
					i32 numConfigOptions = pPlaylistUpr->m_configVars.size();

					CGame::TStringStringMap *pConfigOptions = g_pGame->GetVariantOptions();
					CGame::TStringStringMap::iterator end = pConfigOptions->end();
					for (CGame::TStringStringMap::iterator it = pConfigOptions->begin(); it != end; ++ it)
					{
						tukk pOption = it->first.c_str();
						tukk pValue = it->second.c_str();
						for (i32 j = 0; j < numOptions; ++ j)
						{
							ICVar *pOptionVar = pPlaylistUpr->m_options[j].m_pCVar;
							if (pOptionVar && !stricmp(pOptionVar->GetName(), pOption))
							{
								DrxLog("CPlaylistUpr::DoStartPlaylistCommand() setting config option %s from %s to %s", pOption, pOptionVar->GetString(), pValue);
								pOptionVar->Set(pValue);
								break;
							}
						}
						for (i32 k = 0; k < numConfigOptions; ++ k)
						{
							ICVar *pConfigVar = pPlaylistUpr->m_configVars[k].m_pCVar;
							if (pConfigVar && !stricmp(pConfigVar->GetName(), pOption))
							{
								DrxLog("CPlaylistUpr::DoStartPlaylistCommand() setting config option %s from %s to %s", pOption, pConfigVar->GetString(), pValue);
								pConfigVar->Set(pValue);
							}
						}
					}

					CGameLobby *pGameLobby = g_pGame->GetGameLobby();
					if (pGameLobby)
					{
						DrxLogAlways("Starting playlist %s...", pPlaylistArg);
						pGameLobby->OnStartPlaylistCommandIssued();
					}
				}
				else
				{
					DrxLog("DoStartPlaylist called with %s, but we have no playlist manager", pPlaylistArg ? pPlaylistArg : "<unknown arg>");
				}

				success = true;
				break;
			}
		}
	}
	return success;
}

//---------------------------------------
// [static]
template <class T>
void CPlaylistUpr::GetLocalisationSpecificAttr(const XmlNodeRef& rNode, tukk pAttrKey, tukk pLang, T* pOutVal)
{
	DRX_ASSERT(rNode!=0);
	DRX_ASSERT(pAttrKey);
	DRX_ASSERT(pOutVal);

	bool  useEnglish = false;

	DrxFixedStringT<32>  attrKeyWithLang = pAttrKey;
	if (pLang && (stricmp(pLang, "english") != 0))
	{
		attrKeyWithLang	+= "_";
		attrKeyWithLang	+= pLang;
	}
	else
	{
		useEnglish = true;
	}

	tukk pAttrVal = NULL;

	if (rNode)
	{
		if (!useEnglish)
		{
			if (!rNode->getAttr(attrKeyWithLang, &pAttrVal))
			{
				useEnglish = true;
				LOCAL_WARNING(0, string().Format("Could not get attribute '%s' for language '%s', will try falling-back to English...", pAttrKey, pLang).c_str());
			}
		}

		if (useEnglish)
		{
			const bool  gotEng = rNode->getAttr(pAttrKey, &pAttrVal);
			LOCAL_WARNING(gotEng, string().Format("Could not get attribute '%s' for English language! Will not have a valid value for this attribute.", pAttrKey).c_str());
		}
	}

	if (pAttrVal && (pAttrVal[0] != '\0'))
	{
		if (pOutVal)
		{
			(*pOutVal) = pAttrVal;
		}
	}
	else
	{
		LOCAL_WARNING(0, string().Format("Could not get an attribute named '%s' in node '%s' (when pAttrKey='%s' and pLang='%s')", attrKeyWithLang.c_str(), (rNode ? rNode->getTag() : NULL), pAttrKey, pLang).c_str());
		if (pOutVal)
		{
			(*pOutVal).clear();
		}
	}
}

//---------------------------------------
ILevelRotation::TExtInfoId CPlaylistUpr::CreateCustomPlaylist(tukk pName)
{
	const ILevelRotation::TExtInfoId playlistId = GetPlaylistId(pName);

	SPlaylist* pList = FindPlaylistById(playlistId);
	if (!pList)
	{
		m_playlists.push_back(SPlaylist());
		pList = &m_playlists.back();
	}

	pList->Reset();
	pList->rotExtInfo.uniqueName = pName;
	pList->id = playlistId;
	pList->bSynchedFromServer = true;
	pList->SetEnabled(true);

	return playlistId;
}

//---------------------------------------
void CPlaylistUpr::OnAfterVarChange( ICVar *pVar )
{
	DRX_ASSERT(gEnv->IsDedicated());
	
	
#ifndef _RELEASE
	if(g_pGameCVars->g_disableSwitchVariantOnSettingChanges)
	{
		return; 
	}
#endif //#ifndef _RELEASE

	if (!m_bIsSettingOptions)
	{
		u32k numOptions = m_options.size();
		for (u32 i = 0; i < numOptions; ++ i)
		{
			SGameModeOption &option = m_options[i];
			if (option.m_pCVar == pVar)
			{
				CProfileOptions *pProfileOptions = g_pGame->GetProfileOptions();
				CGameLobby *pGameLobby = g_pGame->GetGameLobby();
				if (pProfileOptions && pGameLobby)
				{
					DrxLog("Saving current settings");
					SaveCurrentSettingsToProfile();  // these settings will get read back out of the profile in the OnVarianteChanged() function below, so this essentially copies the current variant options (with the var changes that triggered this call included) onto the custom variant

					if (m_activeVariantIndex != m_customVariantIndex)
					{
						DrxLog("CPlaylistUpr::OnAfterVarChange() change detected to cvar %s, switching to custom variant", pVar->GetName());
						ChooseVariant(m_customVariantIndex);
					}

					pGameLobby->QueueSessionUpdate();
				}

				break;
			}
		}
	}
}

#if USE_DEDICATED_LEVELROTATION
//---------------------------------------
void CPlaylistUpr::LoadLevelRotation()
{
	// Check for and load levelrotation.xml
	string path = PathUtil::GetGameFolder();

	DrxFixedStringT<128> levelRotationFileName;
	levelRotationFileName.Format("./%s/LevelRotation.xml", path.c_str());

	DrxLog("[dedicated] checking for LevelRotation.xml at %s", levelRotationFileName.c_str());

	CDrxFile file;
	if (file.Open( levelRotationFileName.c_str(), "rb", IDrxPak::FOPEN_HINT_QUIET | IDrxPak::FOPEN_ONDISK ))
	{
		const size_t fileSize = file.GetLength();
		char *pBuffer = new char [fileSize];

		file.ReadRaw(pBuffer, fileSize);

		XmlNodeRef root = gEnv->pSystem->LoadXmlFromBuffer(pBuffer, fileSize);

		if (root)
		{
			if (!stricmp(root->getTag(), "levelRotation"))
			{
				tukk pRotationName = NULL;
				if (!root->getAttr("name", &pRotationName))
				{
					pRotationName = PLAYLIST_MANAGER_CUSTOM_NAME;
				}

				
				bool bUsingCustomRotation = false;
				ILevelRotation *pRotation = NULL;
				bool success = true;
				SPlaylist *pPlaylist = FindPlaylistByUniqueName(pRotationName);
				if (!pPlaylist)
				{
					const ILevelRotation::TExtInfoId playlistId = CreateCustomPlaylist(pRotationName);
					pPlaylist = FindPlaylistById( playlistId );
					if (pPlaylist)
					{
						pPlaylist->m_bIsCustomPlaylist = true;
					}
					
					bUsingCustomRotation = true;

					ILevelSystem*  pLevelSystem = g_pGame->GetIGameFramework()->GetILevelSystem();
					
					if( pLevelSystem->AddExtendedLevelRotationFromXmlRootNode( root, NULL, playlistId ) )
					{						
						pRotation = pLevelSystem->FindLevelRotationForExtInfoId(  playlistId);
					}
					else
					{
						LOCAL_WARNING(0, "Failed to add extended level rotation!");
						m_playlists.pop_back();

						success = false;
					}

				}


				if( success )
				{
					bool bReadServerInfo = false;
					i32 variantIndex = -1;

					IGameRulesSystem *pGameRulesSystem = g_pGame->GetIGameFramework()->GetIGameRulesSystem();
					i32k numChildren = root->getChildCount();
					for (i32 i = 0; i < numChildren; ++ i)
					{
						XmlNodeRef childXml = root->getChild(i);
						tukk pTag = childXml->getTag();
						if (!stricmp(pTag, "ServerInfo"))
						{
							if (bReadServerInfo == false)
							{
								bReadServerInfo = true;

								i32 maxPlayers = MAX_PLAYER_LIMIT;

								ReadServerInfo(childXml, variantIndex, maxPlayers);

								if (pPlaylist)
								{
									if (bUsingCustomRotation)
									{
										pPlaylist->rotExtInfo.m_maxPlayers = maxPlayers;
									}
								}
							}
						}
					}

					if (bReadServerInfo == false)
					{
						variantIndex = m_defaultVariantIndex;
					}

					if (variantIndex != -1)
					{
						const SGameVariant *pVariant = GetVariant(variantIndex);
						DRX_ASSERT(pVariant);
						if (pVariant)
						{
							if (pPlaylist && ((variantIndex == m_customVariantIndex) || bUsingCustomRotation))
							{
								pPlaylist->rotExtInfo.m_supportedVariants.push_back(SSupportedVariantInfo(pVariant->m_name.c_str(), pVariant->m_id));
							}
							DrxFixedStringT<128> startCommand;
							startCommand.Format("startPlaylist %s__%s", pRotationName, pVariant->m_name.c_str());
							gEnv->pConsole->ExecuteString(startCommand);
						}
					}

				}
				else
				{
					DrxLog( "Error in Creating Level Rotation for playlist" );
				}
			}
		}
		delete[] pBuffer;
	}
}

//---------------------------------------
void CPlaylistUpr::ReadServerInfo(XmlNodeRef xmlNode, i32 &outVariantIndex, i32 &outMaxPlayers)
{
	tukk pString = NULL;

	outMaxPlayers = MAX_PLAYER_LIMIT;

	i32k numChildren = xmlNode->getChildCount();
	for (i32 i = 0; i < numChildren; ++ i)
	{
		XmlNodeRef xmlChild = xmlNode->getChild(i);
		tukk pTag = xmlChild->getTag();
		if (!stricmp(pTag, "Details"))
		{
			if (xmlChild->getAttr("name", &pString))
			{
				ICVar *pCVar = gEnv->pConsole->GetCVar("sv_servername");
				DRX_ASSERT(pCVar);
				if (pCVar)
				{
					pCVar->Set(pString);
				}
			}
			if (xmlChild->getAttr("password", &pString))
			{
				ICVar *pCVar = gEnv->pConsole->GetCVar("sv_password");
				DRX_ASSERT(pCVar);
				if (pCVar)
				{
					pCVar->Set(pString);
				}
			}
			if (xmlChild->getAttr("motd", &pString))
			{
				g_pGameCVars->g_messageOfTheDay->Set(pString);
			}
			if (xmlChild->getAttr("imageURL", &pString))
			{
				g_pGameCVars->g_serverImageUrl->Set(pString);
			}
			if (xmlChild->getAttr("maxPlayers", outMaxPlayers))
			{
				outMaxPlayers = CLAMP(outMaxPlayers, 2, MAX_PLAYER_LIMIT);
			}
		}
		else if (!stricmp(pTag, "Variant"))
		{
			if (!xmlChild->getAttr("name", &pString))
			{
				pString = PLAYLIST_MANAGER_CUSTOM_NAME;
			}

			outVariantIndex = -1;
			i32k numVariants = m_variants.size();
			for (i32 j = 0; j < numVariants; ++ j)
			{
				if (!stricmp(m_variants[j].m_name.c_str(), pString))
				{
					outVariantIndex = j;
					break;
				}
			}

			if ((outVariantIndex == -1) || (outVariantIndex == m_customVariantIndex))
			{
				if ((m_customVariantIndex >= 0) && (m_customVariantIndex < m_variants.size()))
				{
					outVariantIndex = m_customVariantIndex;
					SGameVariant *pVariant = &m_variants[m_customVariantIndex];
					SetOptionsFromXml(xmlChild, pVariant);
				}
			}
		}
	}
}

//---------------------------------------
void CPlaylistUpr::SetOptionsFromXml(XmlNodeRef xmlNode, SGameVariant *pVariant)
{
	i32k numOptions = xmlNode->getChildCount();
	for (i32 i = 0; i < numOptions; ++ i)
	{
		XmlNodeRef xmlChild = xmlNode->getChild(i);
		if (!stricmp(xmlChild->getTag(), "Option"))
		{
			tukk pOption = NULL;
			if (xmlChild->getAttr("setting", &pOption))
			{
				pVariant->m_options.push_back(pOption);
			}
		}
	}
}

//---------------------------------------
bool CPlaylistUpr::IsUsingCustomRotation()
{
	SPlaylist *pPlaylist = FindPlaylistById(m_currentPlaylistId);
	
	bool bResult = false;
	if (pPlaylist)
	{
		bResult = (pPlaylist->m_bIsCustomPlaylist || (m_activeVariantIndex == m_customVariantIndex));
	}
	return bResult;
}
#endif

//---------------------------------------
void CPlaylistUpr::LoadOptionRestrictions()
{
	DrxLog("CPlaylistUpr::LoadOptionRestrictions()");
	if (XmlNodeRef xmlRoot = gEnv->pSystem->LoadXmlFromFile(OPTION_RESTRICTIONS_FILENAME))
	{
		i32k numChildren = xmlRoot->getChildCount();
		m_optionRestrictions.reserve(numChildren);
		for (i32 i = 0; i < numChildren; ++ i)
		{
			XmlNodeRef xmlChild = xmlRoot->getChild(i);
			if (!stricmp(xmlChild->getTag(), "Restriction"))
			{
				i32k numOperands = xmlChild->getChildCount();
				if (numOperands == 2)
				{
					XmlNodeRef xmlOperand1 = xmlChild->getChild(0);
					XmlNodeRef xmlOperand2 = xmlChild->getChild(1);

					SOptionRestriction restriction;
					if (LoadOperand(xmlOperand1, restriction.m_operand1) && LoadOperand(xmlOperand2, restriction.m_operand2))
					{
#ifndef _RELEASE
						ConsoleVarFunc pCallbackFunc = restriction.m_operand1.m_pVar->GetOnChangeCallback();
						if (pCallbackFunc && pCallbackFunc != CPlaylistUpr::OnCustomOptionCVarChanged)
						{
							DrxFatalError("Can't add callback on operand 1 (%s - %p) - one already exists", restriction.m_operand1.m_pVar->GetName(), restriction.m_operand1.m_pVar);
						}
						pCallbackFunc = restriction.m_operand2.m_pVar->GetOnChangeCallback();
						if (pCallbackFunc && pCallbackFunc != CPlaylistUpr::OnCustomOptionCVarChanged)
						{
							DrxFatalError("Can't add callback on operand 2 (%s - %p) - one already exists", restriction.m_operand2.m_pVar->GetName(), restriction.m_operand2.m_pVar);
						}
#endif

						restriction.m_operand1.m_pVar->SetOnChangeCallback(CPlaylistUpr::OnCustomOptionCVarChanged);
						restriction.m_operand2.m_pVar->SetOnChangeCallback(CPlaylistUpr::OnCustomOptionCVarChanged);

						m_optionRestrictions.push_back(restriction);
					}
					else
					{
						DrxLog("  badly formed file - failed to read operand in restriction %d", i);
					}
				}
				else
				{
					DrxLog("  badly formed file - expected 2 operands in restriction %d", i);
				}
			}
		}
	}
}

//---------------------------------------
bool CPlaylistUpr::LoadOperand( XmlNodeRef operandXml, SOptionRestriction::SOperand &outResult )
{
	if (!stricmp(operandXml->getTag(), "Operand"))
	{
		tukk pVarName;
		if (operandXml->getAttr("var", &pVarName))
		{
			outResult.m_pVar = gEnv->pConsole->GetCVar(pVarName);
			if (outResult.m_pVar)
			{
				tukk pValue;
				if (operandXml->getAttr("equal", &pValue))
				{
					outResult.m_type = SOptionRestriction::eOT_Equal;
				}
				else if (operandXml->getAttr("notEqual", &pValue))
				{
					outResult.m_type = SOptionRestriction::eOT_NotEqual;
				}
				else if (operandXml->getAttr("lessThan", &pValue))
				{
					outResult.m_type = SOptionRestriction::eOT_LessThan;
				}
				else if (operandXml->getAttr("greaterThan", &pValue))
				{
					outResult.m_type = SOptionRestriction::eOT_GreaterThan;
				}
				else
				{
					return false;
				}

				tukk pFallbackValue;
				if (operandXml->getAttr("fallback", &pFallbackValue))
				{
					i32 varType = outResult.m_pVar->GetType();
					switch (varType)
					{
					case CVAR_INT:
						outResult.m_comparisionValue.m_int = atoi(pValue);
						outResult.m_fallbackValue.m_int = atoi(pFallbackValue);
						break;
					case CVAR_FLOAT:
						outResult.m_comparisionValue.m_float = (float) atof(pValue);
						outResult.m_fallbackValue.m_float = (float) atof(pFallbackValue);
						break;
					case CVAR_STRING:
						outResult.m_comparisionValue.m_string = pValue;
						outResult.m_fallbackValue.m_string = pFallbackValue;
						break;
					}

					return true;
				}
				else
				{
					return false;
				}
			}
		}
	}

	return false;
}

//---------------------------------------
void CPlaylistUpr::CheckRestrictions( ICVar *pChangedVar )
{
	i32 numRestrictions = m_optionRestrictions.size();
	for (i32 i = 0; i < numRestrictions; ++ i)
	{
		SOptionRestriction &restriction = m_optionRestrictions[i];
		if (restriction.m_operand1.m_pVar == pChangedVar)
		{
			ApplyRestriction(restriction.m_operand1, restriction.m_operand2);
		}
		else if (restriction.m_operand2.m_pVar == pChangedVar)
		{
			ApplyRestriction(restriction.m_operand2, restriction.m_operand1);
		}
	}
	SaveCurrentSettingsToProfile();
}

//---------------------------------------
void CPlaylistUpr::ApplyRestriction( SOptionRestriction::SOperand &operand1, SOptionRestriction::SOperand &operand2 )
{
	if (CheckOperation(operand1, false))
	{
		if (CheckOperation(operand2, true))
		{
			DrxLog("CPlaylistUpr::ApplyRestriction() found violated restriction, '%s' set to fallback value", operand2.m_pVar->GetName());
		}
	}
}

//---------------------------------------
bool CPlaylistUpr::CheckOperation( SOptionRestriction::SOperand &operand, bool bEnsureFalse )
{
	bool bComparisonResult = false;

	i32 varType = operand.m_pVar->GetType();
	switch (varType)
	{
	case CVAR_INT:
		{
			switch (operand.m_type)
			{
			case SOptionRestriction::eOT_Equal:
				bComparisonResult = (operand.m_pVar->GetIVal() == operand.m_comparisionValue.m_int);
				break;
			case SOptionRestriction::eOT_NotEqual:
				bComparisonResult = (operand.m_pVar->GetIVal() != operand.m_comparisionValue.m_int);
				break;
			case SOptionRestriction::eOT_LessThan:
				bComparisonResult = (operand.m_pVar->GetIVal() < operand.m_comparisionValue.m_int);
				break;
			case SOptionRestriction::eOT_GreaterThan:
				bComparisonResult = (operand.m_pVar->GetIVal() > operand.m_comparisionValue.m_int);
				break;
			}
			if (bComparisonResult && bEnsureFalse)
			{
				operand.m_pVar->Set(operand.m_fallbackValue.m_int);
			}
		}
		break;
	case CVAR_FLOAT:
		{
			switch (operand.m_type)
			{
			case SOptionRestriction::eOT_Equal:
				bComparisonResult = (operand.m_pVar->GetFVal() == operand.m_comparisionValue.m_float);
				break;
			case SOptionRestriction::eOT_NotEqual:
				bComparisonResult = (operand.m_pVar->GetFVal() != operand.m_comparisionValue.m_float);
				break;
			case SOptionRestriction::eOT_LessThan:
				bComparisonResult = (operand.m_pVar->GetFVal() < operand.m_comparisionValue.m_float);
				break;
			case SOptionRestriction::eOT_GreaterThan:
				bComparisonResult = (operand.m_pVar->GetFVal() > operand.m_comparisionValue.m_float);
				break;
			}
			if (bComparisonResult && bEnsureFalse)
			{
				operand.m_pVar->Set(operand.m_fallbackValue.m_float);
			}
		}
		break;
	case CVAR_STRING:
		{
			switch (operand.m_type)
			{
			case SOptionRestriction::eOT_Equal:
				bComparisonResult = (stricmp(operand.m_pVar->GetString(), operand.m_comparisionValue.m_string.c_str()) == 0);
				break;
			case SOptionRestriction::eOT_NotEqual:
				bComparisonResult = (stricmp(operand.m_pVar->GetString(), operand.m_comparisionValue.m_string.c_str()) != 0);
				break;
			}
			if (bComparisonResult && bEnsureFalse)
			{
				operand.m_pVar->Set(operand.m_fallbackValue.m_string.c_str());
			}
		}
		break;
	}

	return bComparisonResult;
}

//---------------------------------------
void CPlaylistUpr::OnCustomOptionCVarChanged( ICVar *pCVar )
{
	CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
	if (pPlaylistUpr)
	{
		pPlaylistUpr->CheckRestrictions(pCVar);
	}
}

#undef LOCAL_WARNING
#undef MAX_ALLOWED_NEGATIVE_OPTION_AMOUNT
