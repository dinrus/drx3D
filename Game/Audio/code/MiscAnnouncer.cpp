// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

	Plays Announcements based upon general game events

История:
- 17:11:2012		Created by Jim Bamford

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Audio/MiscAnnouncer.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/Announcer.h>
#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/WeaponSystem.h>

i32 static ma_debug = 0;

i32 static ma_enabled = 1;

#if !defined(_RELEASE) && !DRX_PLATFORM_ORBIS
	#define DbgLog(...) \
		if(ma_debug >= 1) \
		{ \
            DrxLogAlways((string().Format("<MA> " __VA_ARGS__)).c_str()); \
		}
#else
	#define DbgLog(...) (void)0
#endif


//---------------------------------------
CMiscAnnouncer::CMiscAnnouncer() 
{
	DrxLog("CMiscAnnouncer::CMiscAnnouncer()");

	REGISTER_CVAR(ma_enabled, ma_enabled, VF_NULL, "Stops misc announcements being played or updated");
	REGISTER_CVAR_DEV_ONLY(ma_debug, ma_debug, VF_NULL, "Enable/Disables Misc announcer debug messages: >= 1 - verbose logging; 2 - debug onWeaponFired map; 3 - debug actor listener map");
}

//---------------------------------------
CMiscAnnouncer::~CMiscAnnouncer()
{
	DrxLog("CMiscAnnouncer::~CMiscAnnouncer()");

	if(gEnv->pConsole)
	{
		gEnv->pConsole->UnregisterVariable("ma_enabled");
#if !defined(_RELEASE)
		gEnv->pConsole->UnregisterVariable("ma_debug");
#endif
	}

	CGameRules *pGameRules = g_pGame->GetGameRules();
	if(pGameRules)
	{
		pGameRules->UnRegisterRoundsListener(this);
		pGameRules->RemoveGameRulesListener(this);
	}

	RemoveAllWeaponListeners();	
	g_pGame->GetIGameFramework()->GetIItemSystem()->UnregisterListener(this);
}

//---------------------------------------
void CMiscAnnouncer::Init()
{
	DrxLog("CMiscAnnouncer::Init()");

	IEntityClassRegistry *pEntityClassRegistry = gEnv->pEntitySystem->GetClassRegistry();
	
	XmlNodeRef xmlData = GetISystem()->LoadXmlFromFile("Scripts/Sounds/MiscAnnouncements.xml");
	InitXML(xmlData);

	CGameRules *pGameRules = g_pGame->GetGameRules();
	pGameRules->RegisterRoundsListener(this);
	pGameRules->AddGameRulesListener(this);

	g_pGame->GetIGameFramework()->GetIItemSystem()->RegisterListener(this);
}

//---------------------------------------
void CMiscAnnouncer::Reset()
{
	DrxLog("CMiscAnnouncer::Reset()");

	for (TWeaponFiredMap::iterator it=m_weaponFiredMap.begin(); it != m_weaponFiredMap.end(); ++it)
	{
		SOnWeaponFired &onWeaponFired = it->second;
		onWeaponFired.Reset();
	}
}

//---------------------------------------
void CMiscAnnouncer::Update(const float dt)
{
	if(!ma_enabled || !AnnouncerRequired())
		return;

#if !defined(_RELEASE)
	switch(ma_debug)
	{
		case 2:    // debug OnWeaponFiredMap
			for (TWeaponFiredMap::iterator it=m_weaponFiredMap.begin(); it != m_weaponFiredMap.end(); ++it)
			{
				SOnWeaponFired &onWeaponFired = it->second;
				DrxWatch("weaponFired: weapon=%s; announce=%s; played: friend=%s; enemy=%s", onWeaponFired.m_weaponEntityClass->GetName(), onWeaponFired.m_announcementName.c_str(), onWeaponFired.m_havePlayedFriendly ? "true" : "false", onWeaponFired.m_havePlayedEnemy ? "true" : "false");
			}
			break;
		case 3:			// debug listeners
		{
			CGameRules *pGameRules = g_pGame->GetGameRules();
			for (ActorWeaponListenerMap::iterator it=m_actorWeaponListener.begin(); it != m_actorWeaponListener.end(); ++it)
			{
				DrxWatch("ActorWeaponListener: Actor=%s; Weapon=%s", pGameRules->GetEntityName(it->first), pGameRules->GetEntityName(it->second));
			}
		}
	}
#endif
}

// SGameRulesListener
//---------------------------------------
void CMiscAnnouncer::ClientDisconnect( EntityId clientId )
{
	DbgLog("CMiscAnnouncer::ClientDisconnect() clientId=%s", g_pGame->GetGameRules()->GetEntityName(clientId));
	RemoveWeaponListenerForActor(clientId);
}

// IGameRulesRoundsListener
//---------------------------------------
void CMiscAnnouncer::OnRoundStart()
{
	DrxLog("CMiscAnnouncer::OnRoundStart() resetting");
	Reset();
}

//IItemSystemListener
//---------------------------------------
void CMiscAnnouncer::OnSetActorItem(IActor *pActor, IItem *pItem )
{
	if( pItem && pActor && pActor->IsPlayer() )
	{
		IWeapon* pWeapon=pItem->GetIWeapon();
		DbgLog("CMiscAnnouncer::OnSetActorItem() pActor=%s; pItem=%s; pWeapon=%p", pActor->GetEntity()->GetName(), pItem->GetDisplayName(), pWeapon);

		if(pWeapon)
		{
			EntityId actorId = pActor->GetEntityId();
			EntityId weaponId = pItem->GetEntityId();
			SetNewWeaponListener(pWeapon, weaponId, actorId);
		}
	}
}

// IWeaponEventListener
//---------------------------------------
void CMiscAnnouncer::OnShoot(IWeapon *pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3 &pos, const Vec3 &dir, const Vec3 &vel)
{
	if(!ma_enabled || !AnnouncerRequired())
		return;

	CWeapon *pWeaponImpl = static_cast<CWeapon*>(pWeapon);
	IEntityClass *pWeaponEntityClass = pWeaponImpl->GetEntity()->GetClass();
	g_pGame->GetWeaponSystem()->GetWeaponAlias().UpdateClass(&pWeaponEntityClass);

	DbgLog("CMiscAnnouncer::OnShoot() pWeaponImpl=%s; shooter=%d; pAmmoType=%s", pWeaponImpl->GetEntity()->GetName(), g_pGame->GetGameRules()->GetEntityName(shooterId), pAmmoType->GetName());
	
	TWeaponFiredMap::iterator it = m_weaponFiredMap.find(pWeaponEntityClass);
	if(it != m_weaponFiredMap.end())
	{
		SOnWeaponFired &onWeaponFired = it->second;

		DbgLog("CMiscAnnouncer::OnShoot() has found the firing weaponClass in our weaponFiredMap. With announcement=%s", onWeaponFired.m_announcementName.c_str());

		// we only want to play the announcement once each game/round
		IActor *pClientActor = gEnv->pGame->GetIGameFramework()->GetClientActor();
		CGameRules *pGameRules = g_pGame->GetGameRules();
		i32 clientTeam = pGameRules->GetTeam(pClientActor->GetEntityId());
		i32 shooterTeam = pGameRules->GetTeam(shooterId);
		
		if (clientTeam == shooterTeam)
		{
			if (!onWeaponFired.m_havePlayedFriendly)
			{
				DbgLog("CMiscAnnouncer::OnShoot() we've not played this friendly annoucement already. Let's do it");
				CAnnouncer::GetInstance()->Announce(shooterId, onWeaponFired.m_announcementID, CAnnouncer::eAC_inGame);
			}
			else
			{
				DbgLog("CMiscAnnouncer::OnShoot() we've already played this friendly announcement. Not playing again");
			}
			onWeaponFired.m_havePlayedFriendly = true;
		}
		else
		{
			if (!onWeaponFired.m_havePlayedEnemy)
			{
				DbgLog("CMiscAnnouncer::OnShoot() we've not played this enemy announcement already. Let's do it");
				CAnnouncer::GetInstance()->Announce(shooterId, onWeaponFired.m_announcementID, CAnnouncer::eAC_inGame);
			}
			else
			{
				DbgLog("CMiscAnnouncer::OnShoot() we've already played this enemy announcement. Not playing again.");
			}
			onWeaponFired.m_havePlayedEnemy = true;
		}
	}
}

//---------------------------------------
void CMiscAnnouncer::InitXML(XmlNodeRef root)
{
	IEntityClassRegistry *pEntityClassRegistry = gEnv->pEntitySystem->GetClassRegistry();
	XmlNodeRef onWeaponFiringRoot = root->findChild("OnWeaponFiring");

	DrxLog("CMiscAnnouncer::InitXML()");

	if(onWeaponFiringRoot)
	{
		i32k numChildren = onWeaponFiringRoot->getChildCount();

		DrxLog("CMiscAnnouncer::InitXML() found OnWeaponFiringRoot with %d children", numChildren);
	
		for(i32 i=0; i<numChildren; ++i)
		{
			XmlNodeRef child = onWeaponFiringRoot->getChild(i);
			if(child->isTag("OnWeaponFired"))
			{
				tukk pWeaponClassName = child->getAttr("weaponClass");
				DRX_ASSERT(pWeaponClassName && pWeaponClassName[0] != 0);

				DrxLog("CMiscAnnouncer::InitXML() OnWeaponFired tag - pWeaponClassName=%s", pWeaponClassName ? pWeaponClassName : "NULL");

				if(IEntityClass* pWeaponEntityClass = pEntityClassRegistry->FindClass(pWeaponClassName))
				{
					tukk pAnnouncement = child->getAttr("announcement");
					DRX_ASSERT(pAnnouncement && pAnnouncement[0] != 0);

					EAnnouncementID announcementID = CAnnouncer::NameToID(pAnnouncement);
					
					DrxLog("CMiscAnnouncer::InitXML() found weapon entity class for pWeaponClassName=%s; found pAnnouncement=%s announcementID=%x", pWeaponClassName, pAnnouncement ? pAnnouncement : "NULL", announcementID);

					SOnWeaponFired newWeaponFired(pWeaponEntityClass, pAnnouncement, announcementID);
					m_weaponFiredMap.insert(TWeaponFiredMap::value_type(pWeaponEntityClass, newWeaponFired));
				}
				else
				{
					DrxLog("CMiscAnnouncer::InitXML() failed to find entityClass for pWeaponClassName=%s", pWeaponClassName);
					DRX_ASSERT_MESSAGE(0, string().Format("CMiscAnnouncer::InitXML() failed to find entityClass for pWeaponClassName=%s", pWeaponClassName));
				}
			}
			else
			{
				DrxLog("CMiscAnnouncer::InitXML() unhandled childtag of %s found", child->getTag());
			}
		}
	}
}

//---------------------------------------
void CMiscAnnouncer::SetNewWeaponListener(IWeapon* pWeapon, EntityId weaponId, EntityId actorId)
{
	DbgLog("CMiscAnnouncer::SetNewWeaponListener() pWeapon=%p; actor=%s", pWeapon, g_pGame->GetGameRules()->GetEntityName(actorId));

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
	
	pWeapon->AddEventListener(this, "CMiscAnnouncer");
}

//---------------------------------------
void CMiscAnnouncer::RemoveWeaponListener(EntityId weaponId)
{
	DbgLog("CMiscAnnouncer::RemoveWeaponListener() weapon=%s", g_pGame->GetGameRules()->GetEntityName(weaponId));

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
void CMiscAnnouncer::RemoveAllWeaponListeners()
{
	DrxLog("CMiscAnnouncer::RemoveAllWeaponListeners()");

	ActorWeaponListenerMap::const_iterator it = m_actorWeaponListener.begin();
	ActorWeaponListenerMap::const_iterator end = m_actorWeaponListener.end();
	for ( ; it!=end; ++it)
	{
		RemoveWeaponListener(it->second);
	}

	m_actorWeaponListener.clear();
}

//---------------------------------------
void CMiscAnnouncer::RemoveWeaponListenerForActor(EntityId actorId)
{
	DbgLog("CMiscAnnouncer::RemoveWeaponListenerForActor() actor=%s", g_pGame->GetGameRules()->GetEntityName(actorId));

	ActorWeaponListenerMap::iterator it = m_actorWeaponListener.find(actorId);
	if(it != m_actorWeaponListener.end())
	{
		//remove previous weapon listener for actor
		RemoveWeaponListener(it->second);
	}
}


//---------------------------------------
bool CMiscAnnouncer::AnnouncerRequired()
{
	bool required=true;

	if (gEnv->IsDedicated())
	{
		required=false;
	}

#if 0
	EGameMode gameMode = g_pGame->GetGameRules()->GetGameMode();

	switch(gameMode)
	{
		case eGM_Gladiator:
		case eGM_Assault:
			required=false;
			break;
	}
#endif
	return required;
}

#undef DbgLog
