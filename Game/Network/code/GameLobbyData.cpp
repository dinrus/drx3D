// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameLobbyData.h>

#include <drx3D/Game/Utility/DrxHash.h>

#include <drx3D/Game/GameRulesModules/IGameRulesModulesUpr.h>
#include <drx3D/Game/GameRulesModules/GameRulesModulesUpr.h>
#include <ILevelSystem.h>
#include <drx3D/CoreX/String/NameCRCHelper.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/PlaylistUpr.h>
#include <drx3D/Game/DataPatchDownloader.h>

namespace GameLobbyData
{
	char const * const g_sUnknown = "Unknown";

	u32k ConvertGameRulesToHash(tukk gameRules)
	{
		if (gameRules)
		{
			return DrxStringUtils::HashStringLower(gameRules);
		}
		else
		{
			return 0;
		}
	}

	tukk GetGameRulesFromHash(u32 hash, tukk unknownStr/*="Unknown"*/)
	{
		IGameRulesModulesUpr *pGameRulesModulesUpr = CGameRulesModulesUpr::GetInstance();
		i32k rulesCount = pGameRulesModulesUpr->GetRulesCount();
		for(i32 i = 0; i < rulesCount; i++)
		{
			tukk name = pGameRulesModulesUpr->GetRules(i);
			if(ConvertGameRulesToHash(name) == hash)
			{
				return name;
			}
		}

		return unknownStr;
	}

	u32k ConvertMapToHash(tukk mapName)
	{
		if (mapName)
		{
			return DrxStringUtils::HashStringLower(mapName);
		}
		else
		{
			return 0;
		}
	}

	tukk GetMapFromHash(u32 hash, tukk pUnknownStr)
	{
		ILevelSystem* pLevelSystem = g_pGame->GetIGameFramework()->GetILevelSystem();
		i32k levelCount = pLevelSystem->GetLevelCount();
		for(i32 i = 0; i < levelCount; i++)
		{
			tukk name = pLevelSystem->GetLevelInfo(i)->GetName();
			if(ConvertMapToHash(name) == hash)
			{
				return name;
			}
		}
		for(i32 i = 0; i < levelCount; i++)
		{
			tukk name = pLevelSystem->GetLevelInfo(i)->GetName();
			tukk pTrimmedLevelName = strrchr(name, '/');
			if(pTrimmedLevelName && ConvertMapToHash(pTrimmedLevelName+1) == hash)
			{
				return name;
			}
		}
		return pUnknownStr;
	}

	u32k GetVersion()
	{
#if defined(DEDICATED_SERVER)
		CDownloadMgr* pDownloadMgr = g_pGame->GetDownloadMgr();
		if (pDownloadMgr && pDownloadMgr->IsWaitingToShutdown())
		{
			// Return a bogus version number so that nobody is able to connect to the server
			return 0xDEADBEEF;
		}
#endif
		// matchmaking version defaults to build id, i've chose the bit shifts here to ensure we don't unnecessary truncate the version and matchmake against the wrong builds
		u32k version = (g_pGameCVars->g_MatchmakingVersion & 8191) + (g_pGameCVars->g_MatchmakingBlock * 8192);
		return version^GetGameDataPatchCRC();
	}

	const bool IsCompatibleVersion(u32 version)
	{
		return version == GetVersion();
	}

	u32k GetPlaylistId()
	{
		CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
		if (pPlaylistUpr)
		{
			const SPlaylist* pPlaylist = pPlaylistUpr->GetCurrentPlaylist();
			if(pPlaylist)
			{
				return pPlaylist->id;
			}
		}

		return 0;
	}

	u32k GetVariantId()
	{
		CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
		if (pPlaylistUpr)
		{
			return pPlaylistUpr->GetActiveVariantIndex();
		}

		return 0;
	}

	u32k GetGameDataPatchCRC()
	{
		u32		result=0;

		if (CDataPatchDownloader *pDP=g_pGame->GetDataPatchDownloader())
		{
			result=pDP->IsPatchingEnabled()?pDP->GetDataPatchHash():0;
		}

		return result;
	}

	i32k GetSearchResultsData(SDrxSessionSearchResult* session, DrxLobbyUserDataID id)
	{

		for (u32 i = 0; i < session->m_data.m_numData; i++)
		{
			if (session->m_data.m_data[i].m_id == id)
			{
				return session->m_data.m_data[i].m_int32;
			}
		}

		DRX_ASSERT(0);

		return 0;
	}

	const bool IsValidServer(SDrxSessionSearchResult* session)
	{
		bool isValidServer = false;

		if(GameLobbyData::IsCompatibleVersion(GameLobbyData::GetSearchResultsData(session, LID_MATCHDATA_VERSION)))
		{
			if(GameLobbyData::GetSearchResultsData(session, LID_MATCHDATA_GAMEMODE) != 0 && GameLobbyData::GetSearchResultsData(session, LID_MATCHDATA_MAP) != 0)
			{
				isValidServer = true;
			}
		}

		return isValidServer;
	}
}
