// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************

GameLocalizationUpr is designed to look after the loading of strings
and be the sole place in the game that loads and unloads localization tags

When loading into the game we load <init> tag
Loading into singleplayer/multiplayer we load (and unload) <singleplayer>/<multiplayer> tags
Loading into a level we load <levelname> tags for sp and <gamemode name> for mp
Also as a special case credits are dynamically loaded
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameLocalizationUpr.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>

CGameLocalizationUpr::CGameLocalizationUpr()
{
	LoadLocalizationData();

#if !defined(_RELEASE)
	REGISTER_COMMAND("LocalizationDumpLoadedTags", LocalizationDumpLoadedTags, VF_NULL, "Dump out into about the loaded localization tags");
#endif //#if !defined(_RELEASE)
}

CGameLocalizationUpr::~CGameLocalizationUpr()
{
	if(gEnv->pConsole)
	{
		gEnv->pConsole->RemoveCommand("LocalizationDumpLoadedTags");
	}
}

void CGameLocalizationUpr::LoadLocalizationData()
{
	ILocalizationUpr *pLocMan = GetISystem()->GetLocalizationUpr();

	string locaFile = "Libs/Localization/localization.xml";

	DrxLog("CGameLocalizationUpr::LoadLocalizationData() is loading localization file=%s", locaFile.c_str());

	if (pLocMan->InitLocalizationData(locaFile.c_str()))
	{
		LoadTag(eLT_Init);
	}
	else
	{	
		LegacyLoadLocalizationData();
	}
}


void CGameLocalizationUpr::LegacyLoadLocalizationData()
{
	// fallback to old system if localization.xml can not be found
	GameWarning("Using Legacy localization loading");

	ILocalizationUpr *pLocMan = GetISystem()->GetLocalizationUpr();

	string const szLocalizationFolderPath(PathUtil::GetGameFolder() + DRX_NATIVE_PATH_SEPSTR + PathUtil::GetLocalizationFolder() + DRX_NATIVE_PATH_SEPSTR);
	string const search(szLocalizationFolderPath + "*.xml");

	IDrxPak *pPak = gEnv->pDrxPak;

	_finddata_t fd;
	intptr_t handle = pPak->FindFirst(search.c_str(), &fd);

	if (handle > -1)
	{
		do
		{
			DRX_ASSERT_MESSAGE(stricmp(PathUtil::GetExt(fd.name), "xml") == 0, "expected xml files only");

			string filename = szLocalizationFolderPath + fd.name;
			pLocMan->LoadExcelXmlSpreadsheet(filename.c_str());
		}
		while (pPak->FindNext(handle, &fd) >= 0);

		pPak->FindClose(handle);
	}
	else
	{
		GameWarning("Unable to find any Localization Data!");
	}
}

void CGameLocalizationUpr::SetGameType()
{
	UnloadTag(eLT_GameType);
	LoadTag(eLT_GameType);
}

void CGameLocalizationUpr::SetCredits( bool enable )
{
	if(enable)
	{
		LoadTag(eLT_Credits);
	}
	else
	{
		UnloadTag(eLT_Credits);
	}
}

void CGameLocalizationUpr::LoadTag( ELocalizationTag tag )
{
	if(gEnv->IsEditor())// in editor all loca files are loaded during level load to get the info in the log
		return;

	switch(tag)
	{
	case eLT_Init:
		{
			LoadTagInternal(tag, "init");
		}
		break;
	case eLT_GameType:
		{
			LoadTagInternal(tag, gEnv->bMultiplayer ? "multiplayer" : "singleplayer");
		}
		break;
	case eLT_Credits:
		{
			LoadTagInternal(tag, "credits");
		}
		break;
	default:
		DRX_ASSERT_MESSAGE(0, "Unknown GameLocalizationManger Tag");
		break;
	}
}

void CGameLocalizationUpr::LoadTagInternal( ELocalizationTag tag , tukk pTag )
{
	DRX_ASSERT(tag >= 0 && tag < eLT_Num);
	DRX_ASSERT(pTag);

	stack_string sTagLowercase(pTag);
	sTagLowercase.MakeLower();

	m_loadedTag[tag] = sTagLowercase.c_str();

	ILocalizationUpr *pLocMan = GetISystem()->GetLocalizationUpr();
	if(!pLocMan->LoadLocalizationDataByTag(sTagLowercase.c_str()))
	{
		GameWarning("Failed to load localization tag %s", sTagLowercase.c_str());
	}
}

void CGameLocalizationUpr::UnloadTag( ELocalizationTag tag )
{
	if(gEnv->IsEditor())// in editor all loca files are loaded during level load to get the info in the log
		return;

	if(!m_loadedTag[tag].empty())
	{
		tukk pTag = m_loadedTag[tag].c_str();
		ILocalizationUpr *pLocMan = GetISystem()->GetLocalizationUpr();
		if(!pLocMan->ReleaseLocalizationDataByTag(pTag))
		{
			GameWarning("Failed to release localization tag %s", pTag);
		}

		m_loadedTag[tag].clear();
		DRX_ASSERT(m_loadedTag[tag].empty());
	}
}



//////////////////////////////////////////////////////////////////////////
#if !defined(_RELEASE)
void CGameLocalizationUpr::LocalizationDumpLoadedTags( IConsoleCmdArgs* pArgs )
{
	CGameLocalizationUpr* pMgr = g_pGame->GetGameLocalizationUpr();
	for(u32 i = 0; i < eLT_Num; i++)
	{
		if(!pMgr->m_loadedTag[i].empty())
		{
			DrxLogAlways("[%d] %s", i, pMgr->m_loadedTag[i].c_str());
		}
	}
}

#endif //#if !defined(_RELEASE)