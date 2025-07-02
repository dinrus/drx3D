// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameSerialize.h>

#include <drx3D/Act/XmlSaveGame.h>
#include <drx3D/Act/XmlLoadGame.h>

#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Act/IPlayerProfiles.h>

namespace
{
ISaveGame* CSaveGameCurrentUser()
{
	IPlayerProfileUpr* pPPMgr = CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();
	if (pPPMgr)
	{
		i32 userCount = pPPMgr->GetUserCount();
		if (userCount > 0)
		{
			IPlayerProfileUpr::SUserInfo info;
			if (pPPMgr->GetUserInfo(pPPMgr->GetCurrentUserIndex(), info))
			{
				IPlayerProfile* pProfile = pPPMgr->GetCurrentProfile(info.userId);
				if (pProfile)
					return pProfile->CreateSaveGame();
				else
					GameWarning("CSaveGameCurrentUser: User '%s' has no active profile. No save created.", info.userId);
			}
			else
				GameWarning("CSaveGameCurrentUser: Invalid logged in user. No save created.");
		}
		else
			GameWarning("CSaveGameCurrentUser: No User logged in. No save created.");
	}

	// can't save without a profile
	return NULL;
}

ILoadGame* CLoadGameCurrentUser()
{
	IPlayerProfileUpr* pPPMgr = CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();
	if (pPPMgr)
	{
		i32 userCount = pPPMgr->GetUserCount();
		if (userCount > 0)
		{
			IPlayerProfileUpr::SUserInfo info;
			if (pPPMgr->GetUserInfo(pPPMgr->GetCurrentUserIndex(), info))
			{
				IPlayerProfile* pProfile = pPPMgr->GetCurrentProfile(info.userId);
				if (pProfile)
					return pProfile->CreateLoadGame();
				else
					GameWarning("CLoadGameCurrentUser: User '%s' has no active profile. Can't load game.", info.userId);
			}
			else
				GameWarning("CSaveGameCurrentUser: Invalid logged in user. Can't load game.");
		}
		else
			GameWarning("CLoadGameCurrentUser: No User logged in. Can't load game.");
	}

	// can't load without a profile
	return NULL;
}

};

void CGameSerialize::RegisterFactories(IGameFramework* pFW)
{
	// save/load game factories
	REGISTER_FACTORY(pFW, "xml", CXmlSaveGame, false);
	REGISTER_FACTORY(pFW, "xml", CXmlLoadGame, false);
	//	REGISTER_FACTORY(pFW, "binary", CXmlSaveGame, false);
	//	REGISTER_FACTORY(pFW, "binary", CXmlLoadGame, false);

	pFW->RegisterFactory("xml", CLoadGameCurrentUser, false);
	pFW->RegisterFactory("xml", CSaveGameCurrentUser, false);

}
