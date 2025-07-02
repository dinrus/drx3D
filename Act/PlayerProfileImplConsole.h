// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   PlayerProfileImplConsole.cpp
//  Created:     21/12/2009 by Alex Weighell.
//  Описание: Player profile implementation for consoles which manage
//               the profile data via the OS and not via a file system
//               which may not be present.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PLAYERPROFILECONSOLE_H__
#define __PLAYERPROFILECONSOLE_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "PlayerProfileImplFS.h"

class CPlayerProfileImplConsole : public CPlayerProfileUpr::IPlatformImpl, public ICommonProfileImpl
{
public:
	CPlayerProfileImplConsole();

	// CPlayerProfileUpr::IPlatformImpl
	virtual bool                Initialize(CPlayerProfileUpr* pMgr);
	virtual void                Release();
	virtual bool                LoginUser(SUserEntry* pEntry);
	virtual bool                LogoutUser(SUserEntry* pEntry);
	virtual bool                SaveProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name, bool initialSave = false, i32 /*reason*/ = ePR_All);
	virtual bool                LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name);
	virtual bool                DeleteProfile(SUserEntry* pEntry, tukk name);
	virtual bool                RenameProfile(SUserEntry* pEntry, tukk newName);
	virtual bool                GetSaveGames(SUserEntry* pEntry, CPlayerProfileUpr::TSaveGameInfoVec& outVec, tukk altProfileName);
	virtual ISaveGame*          CreateSaveGame(SUserEntry* pEntry);
	virtual ILoadGame*          CreateLoadGame(SUserEntry* pEntry);
	virtual bool                DeleteSaveGame(SUserEntry* pEntry, tukk name);
	virtual ILevelRotationFile* GetLevelRotationFile(SUserEntry* pEntry, tukk name);
	virtual bool                GetSaveGameThumbnail(SUserEntry* pEntry, tukk saveGameName, SThumbnail& thumbnail);
	virtual void                GetMemoryStatistics(IDrxSizer* s);
	// ~CPlayerProfileUpr::IPlatformImpl

	// ICommonProfileImpl
	virtual void                   InternalMakeFSPath(SUserEntry* pEntry, tukk profileName, string& outPath);
	virtual void                   InternalMakeFSSaveGamePath(SUserEntry* pEntry, tukk profileName, string& outPath, bool bNeedFolder);
	virtual CPlayerProfileUpr* GetUpr() { return m_pMgr; }
	// ~ICommonProfileImpl

protected:
	virtual ~CPlayerProfileImplConsole();

private:
	CPlayerProfileUpr* m_pMgr;
};

#endif
