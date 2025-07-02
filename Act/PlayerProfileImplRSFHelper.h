// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PLAYERPROFILEIMPLFSF_H__
#define __PLAYERPROFILEIMPLFSF_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "PlayerProfileImplFS.h"

class CRichSaveGameHelper
{
public:
	CRichSaveGameHelper(ICommonProfileImpl* pImpl) : m_pImpl(pImpl) {}
	bool       GetSaveGames(CPlayerProfileUpr::SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk profileName);
	ISaveGame* CreateSaveGame(CPlayerProfileUpr::SUserEntry* pEntry);
	ILoadGame* CreateLoadGame(CPlayerProfileUpr::SUserEntry* pEntry);
	bool       DeleteSaveGame(CPlayerProfileUpr::SUserEntry* pEntry, tukk name);
	bool       GetSaveGameThumbnail(CPlayerProfileUpr::SUserEntry* pEntry, tukk saveGameName, CPlayerProfileUpr::SThumbnail& thumbnail);
	bool       MoveSaveGames(CPlayerProfileUpr::SUserEntry* pEntry, tukk oldProfileName, tukk newProfileName);

protected:
	bool FetchMetaData(XmlNodeRef& root, CPlayerProfileUpr::SSaveGameMetaData& metaData);
	ICommonProfileImpl* m_pImpl;
};

#endif
