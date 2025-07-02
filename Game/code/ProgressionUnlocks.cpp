// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 12:05:2010		Created by Ben Parbury
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ProgressionUnlocks.h>

#include <drx3D/Game/PlayerProgression.h>
#include <drx3D/Game/EquipmentLoadout.h>
#include <drx3D/Game/Utility/DesignerWarning.h>
#include <drx3D/Game/GameParameters.h>
#include <drx3D/Game/ItemSharedParams.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>
#include <drx3D/Game/PersistantStats.h>

struct SUnlockData
{
	EUnlockType			m_type;
	tukk 		m_name;
	tukk 		m_description;

	SUnlockData(EUnlockType type, tukk  name, tukk  desc)
	{
		m_type = type;
		m_name = name;
		m_description = desc;
	}
};

const SUnlockData s_unlockData[] =
{
	SUnlockData (eUT_CreateCustomClass, "customClass",				"@ui_menu_unlock_createcustomclass"),
	SUnlockData (eUT_Loadout,						"loadout",						"@ui_menu_unlock_loadout"),
	SUnlockData (eUT_Weapon,						"weapon",							"@ui_menu_unlock_weapon"),
	SUnlockData (eUT_Playlist,					"playlist",						"@ui_menu_unlock_playlist"),
	SUnlockData (eUT_Attachment,				"attachment",					"@ui_menu_unlock_attachment"),
};

struct SUnlockReasonData
{
	EUnlockReason m_reason;
	tukk m_name;

	SUnlockReasonData(EUnlockReason reason, tukk  name)
	{
		m_reason = reason;
		m_name = name;
	}

};

//TODO: not used currently, remove if this doesn't change
const SUnlockReasonData s_unlockReasonData[] =
{
	SUnlockReasonData(eUR_None, "none"),
	SUnlockReasonData(eUR_SuitRank, "suitrank"),
	SUnlockReasonData(eUR_Rank, "rank"),
	SUnlockReasonData(eUR_Token, "token"),
	SUnlockReasonData(eUR_Assessment, "assessment"),
};

SUnlock::SUnlock(XmlNodeRef node, i32 rank)
{
	m_name[0] = '\0';
	m_rank = rank;
	m_reincarnation = 0;
	m_unlocked = false;
	m_type = eUT_Invalid;

	DesignerWarning(strcmpi(node->getTag(), "unlock") == 0 || strcmpi(node->getTag(), "allow") == 0, "expect tag of unlock or allow at %d", node->getLine());

	tukk  theName = node->getAttr("name");
	tukk  theType = node->getAttr("type");
	tukk  theReincarnationLevel = node->getAttr("reincarnation");

	// These pointers should always be valid... if an attribute isn't found, getAttr returns a pointer to an empty string [TF]
	assert (theName && theType);

	if (theType && theType[0] != '\0')
	{
		m_type = SUnlock::GetUnlockTypeFromName(theType);
		drx_strcpy(m_name, theName);

		bool expectName = (m_type == eUT_Loadout || m_type == eUT_Weapon || m_type == eUT_Attachment || m_type == eUT_Playlist || m_type == eUT_CreateCustomClass);
		bool gotName = (theName[0] != '\0');

		if (expectName != gotName && m_type != eUT_Invalid) // If it's invalid, we'll already have displayed a warning...
		{
			GameWarning("[PROGRESSION] An unlock of type '%s' %s have a name but XML says name='%s'", theType, expectName ? "should" : "shouldn't", theName);
		}
	}
	else
	{
		GameWarning("[PROGRESSION] XML node contains an 'unlock' tag with no type (name='%s')", theName);
	}

	if (theReincarnationLevel != NULL && theReincarnationLevel[0] != '\0')
	{
		m_reincarnation = atoi(theReincarnationLevel);
		CPlayerProgression *pPlayerProgression = CPlayerProgression::GetInstance();
		DesignerWarning(m_reincarnation > 0 && m_reincarnation < pPlayerProgression->GetMaxReincarnations()+1, "Unlock %s reincarnation parameter is outside of the range 0 - %d", theName, pPlayerProgression->GetMaxReincarnations()+1);
	}
}

void SUnlock::Unlocked(bool isNew)
{
	if(!m_unlocked)
	{
    m_unlocked = true;

		CPlayerProgression *pPlayerProgression = CPlayerProgression::GetInstance();

		switch(m_type)
		{				
		case eUT_CreateCustomClass:
			{
				pPlayerProgression->UnlockCustomClassUnlocks( isNew );
					
				if( isNew )
				{
					pPlayerProgression->AddUINewDisplayFlags(eMBF_CustomLoadout);
				}
			}
			break;
		case eUT_Weapon:
			{
				if( isNew )
				{
					if (CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout())
					{
						pEquipmentLoadout->FlagItemAsNew(m_name);
					}
				}

			}
			break;
		}
	}

  if ( m_unlocked )
  {
	  //This needs to always happen because of switching between SP and MP
	  switch(m_type)
	  {
			case eUT_Weapon:
		  {
				CPersistantStats::GetInstance()->IncrementMapStats(EMPS_WeaponUnlocked,m_name);
		  }
		  break;
	  }
  }
}

///static
EUnlockType SUnlock::GetUnlockTypeFromName(tukk name)
{
	if (name && name[0] != '\0')
	{
		for(i32 i = 0; i < eUT_Max; i++)
		{
			if (s_unlockData[i].m_name && s_unlockData[i].m_name[0] != '\0')
			{
				if(strcmpi(name, s_unlockData[i].m_name) == 0)
				{
					return s_unlockData[i].m_type;
				}
			}
		}

		GameWarning("[PROGRESSION] Invalid unlock type '%s'", name);
	}

	return eUT_Invalid;
}


///static
EUnlockReason SUnlock::GetUnlockReasonFromName( tukk name )
{
	for(i32 i = 0; i < eUR_Max; i++)
	{
		if(strcmpi(name, s_unlockReasonData[i].m_name) == 0)
		{
			return s_unlockReasonData[i].m_reason;
		}
	}

	GameWarning("[PROGRESSION] Invalid unlock reason '%s'", name);
	return eUR_Invalid;
}


///static
tukk  SUnlock::GetUnlockReasonName( EUnlockReason reason )
{
	for(i32 i = 0; i < eUR_Max; i++)
	{
		if(reason == s_unlockReasonData[i].m_reason)
		{
			return s_unlockReasonData[i].m_name;
		}
	}

	DRX_ASSERT(0);
	return "";
}


///static
tukk SUnlock::GetUnlockTypeName(EUnlockType type)
{
	for(i32 i = 0; i < eUT_Max; i++)
	{
		if(type == s_unlockData[i].m_type)
		{
			return s_unlockData[i].m_name;
		}
	}

	DRX_ASSERT(0);
	return "";
}

//static
tukk  SUnlock::GetUnlockTypeDescriptionString(EUnlockType type)
{
#ifndef _RELEASE
	tukk result = "Unknown";
#else
	tukk result = "";
#endif

	for(i32 i = 0; i < eUT_Max; i++)
	{
		if(type == s_unlockData[i].m_type)
		{
			return s_unlockData[i].m_description;
		}
	}

	DRX_ASSERT(0);
	return result;
}

//static
tukk  SUnlock::GetUnlockReasonDescriptionString(EUnlockReason reason, i32 data)
{
#ifndef _RELEASE
	tukk result = "Other";
#else
	tukk result = "";
#endif
	switch (reason)
	{
	case eUR_Rank:
		result = "@ui_menu_unlock_reason_rank";
		break;
	};

	return result;
}

bool SUnlock::operator ==(const SUnlock &rhs) const
{
	if(m_type != rhs.m_type || m_rank != rhs.m_rank || m_reincarnation != rhs.m_reincarnation || m_unlocked != rhs.m_unlocked || strcmp(m_name, rhs.m_name) != 0)
	{
		return false;
	}

	return true;
}

bool SUnlock::GetUnlockDisplayString( EUnlockType type, tukk name, DrxFixedStringT<32>& outStr )
{
	// TODO: Setup Playlist unlocks and any others
	bool retval = false;

	switch( type )
	{
	case eUT_Weapon:
		{
			const CItemSharedParams* pItemShared = g_pGame->GetGameSharedParametersStorage()->GetItemSharedParameters( name, false );
			if( pItemShared )
			{
				outStr.Format( pItemShared->params.display_name.c_str() );
				retval = true;
			}
			break;
		}
	case eUT_CreateCustomClass:
		{
			CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout();
			if( pEquipmentLoadout )
			{
				tukk packageName = pEquipmentLoadout->GetPackageDisplayFromName( name );
				if( packageName )
				{
					outStr.Format( CHUDUtils::LocalizeString(packageName) );
					retval = true;
				}
			}

			break;
		}
	case eUT_Attachment:
		{
			tukk pAttachmentName = strstr(name, ".");
			if( pAttachmentName && pAttachmentName[0] )
			{	
				CEquipmentLoadout* pEquipmentLoadout = g_pGame->GetEquipmentLoadout();
				if( pEquipmentLoadout )
				{
					if( const CEquipmentLoadout::SEquipmentItem *pUnlockItem = pEquipmentLoadout->GetItemByName( pAttachmentName+1 ) )
					{
						outStr.Format( pUnlockItem->m_displayName.c_str() );
						retval = true;
					}
				}
			}
			break;
		}
	}

	return retval;
}
