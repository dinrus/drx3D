// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   PlayerProfileImplConsole.cpp
//  Created:     21/12/2009 by Alex Weighell.
//  Описание: Player profile implementation for console demo releases.
//               Doesn't save player profile data!
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PLAYERPROFILENOSAVE_H__
#define __PLAYERPROFILENOSAVE_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "PlayerProfileImplConsole.h"
#include "PlayerProfile.h"
#include <drx3D/CoreX/Platform/IPlatformOS.h>

class CPlayerProfileImplNoSave : public CPlayerProfileImplConsole
{
public:
	// CPlayerProfileUpr::IPlatformImpl
	virtual bool LoginUser(SUserEntry* pEntry);
	virtual bool SaveProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name);
	virtual bool LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name);
	virtual bool DeleteProfile(SUserEntry* pEntry, tukk name);
	virtual bool DeleteSaveGame(SUserEntry* pEntry, tukk name);
	// ~CPlayerProfileUpr::IPlatformImpl

	// ICommonProfileImpl
	// ~ICommonProfileImpl

protected:
	virtual ~CPlayerProfileImplNoSave() {}

private:
	CPlayerProfileUpr* m_pMgr;
};

//------------------------------------------------------------------------
bool CPlayerProfileImplNoSave::SaveProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name)
{
	// This would cause a delete from disk - but we have READ ONLY media in this case
	if (m_pMgr->HasEnabledOnlineAttributes() && m_pMgr->CanProcessOnlineAttributes() && !pProfile->IsDefault())
	{
		m_pMgr->SaveOnlineAttributes(pProfile);
	}
	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplNoSave::LoadProfile(SUserEntry* pEntry, CPlayerProfile* pProfile, tukk name)
{
	// load the profile from a specific location. Only load the default profile since there is no saving!
	using namespace PlayerProfileImpl;

	if (0 == strcmp(name, "default"))
	{
		// XML for now
		string path;
		InternalMakeFSPath(pEntry, name, path);

		XmlNodeRef rootNode = GetISystem()->CreateXmlNode(PROFILE_ROOT_TAG);
		CSerializerXML serializer(rootNode, true);
		XmlNodeRef attrNode = LoadXMLFile(path + "attributes.xml");
		XmlNodeRef actionNode = LoadXMLFile(path + "actionmaps.xml");
		//	serializer.AddSection(attrNode);
		//	serializer.AddSection(actionNode);
		serializer.SetSection(CPlayerProfileUpr::ePPS_Attribute, attrNode);
		serializer.SetSection(CPlayerProfileUpr::ePPS_Actionmap, actionNode);

		bool ok = pProfile->SerializeXML(&serializer);
		return ok;
	}
	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplNoSave::LoginUser(SUserEntry* pEntry)
{
	// lookup stored profiles of the user (pEntry->userId) and fill in the pEntry->profileDesc
	// vector
	pEntry->profileDesc.clear();

	IPlatformOS* os = gEnv->pSystem->GetPlatformOS();

	bool signedIn = os->UserIsSignedIn(pEntry->userIndex);
	DrxLogAlways("LoginUser::UserIsSignedIn %d\n", signedIn);

	if (!signedIn)
	{
#if !defined(_RELEASE)
		if (!CDrxActionCVars::Get().g_userNeverAutoSignsIn)
#endif
		{
			signedIn = os->UserDoSignIn(1, g_pGame->GetExclusiveControllerDeviceIndex()); // disable this on a cvar to remove the signin box during autotests
			DrxLogAlways("LoginUser::UserDoSignIn %d\n", signedIn);
		}
	}
	if (signedIn)
	{
		IPlatformOS::TUserName profileName;

		if (os->UserGetName(pEntry->userIndex, profileName))
		{
			pEntry->profileDesc.push_back(SLocalProfileInfo(profileName.c_str()));
		}
		else
		{
			printf("OS FAILED to get signed in User name\n");
		}
	}
	else
	{
		printf("OS No User signed in\n");
	}

	return signedIn;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplNoSave::DeleteProfile(SUserEntry* pEntry, tukk name)
{
	// This would cause a delete from disk - but we have READ ONLY media in this case
	return true;
}

//------------------------------------------------------------------------
bool CPlayerProfileImplNoSave::DeleteSaveGame(SUserEntry* pEntry, tukk name)
{
	// This would cause a delete from disk - but we have READ ONLY media in this case
	return true;
}

#endif
