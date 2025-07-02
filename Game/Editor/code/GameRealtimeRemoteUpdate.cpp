// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id: GameRealtimeRemoveUpdate.cpp,v 1.1 23/09/2009 by Johnmichael Quinlan 
	Описание:  This is the source file for the game module Realtime remote 
								update. The purpose of this module is to allow data update to happen 
								remotely so that you can, for example, edit the terrain and see the changes
								in the console.
-------------------------------------------------------------------------
История:
- 23:09:2009 :  Created by Johnmichael Quinlan
 ************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRealtimeRemoteUpdate.h>
#include <drx3D/Game/LivePreview/RealtimeRemoteUpdate.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Act/IViewSystem.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Act/ILevelSystem.h>


//0 no fly, 1 fly mode, 2 fly mode + noclip
enum eCameraModes
{
	eCameraModes_no_fly=0,
	eCameraModes_fly_mode=1,
	eCameraModes_fly_mode_noclip=2,
};

//////////////////////////////////////////////////////////////////////////
CGameRealtimeRemoteUpdateListener &	CGameRealtimeRemoteUpdateListener::GetGameRealtimeRemoteUpdateListener()
{
	static CGameRealtimeRemoteUpdateListener oRealtimeUpdateListener;
	return oRealtimeUpdateListener;
}

CGameRealtimeRemoteUpdateListener::CGameRealtimeRemoteUpdateListener()
{
	m_nPreviousFlyMode = eCameraModes_no_fly;
	m_bCameraSync = false;
	m_Position.zero();
	m_ViewDirection.zero();
	m_CameraUpdateInfo=NULL;
	m_headPositionDelta.zero();
}

CGameRealtimeRemoteUpdateListener::~CGameRealtimeRemoteUpdateListener()
{
}

bool CGameRealtimeRemoteUpdateListener::Enable(bool boEnable)
{
	if ( boEnable )
	{	
		g_pGame->GetIGameFramework()->GetIRealTimeRemoteUpdate()->Enable(true);
		g_pGame->GetIGameFramework()->GetIRealTimeRemoteUpdate()->AddGameHandler(this);
	}
	else
	{
		g_pGame->GetIGameFramework()->GetIRealTimeRemoteUpdate()->Enable(false);
		g_pGame->GetIGameFramework()->GetIRealTimeRemoteUpdate()->RemoveGameHandler(this);
	}

	return boEnable;
}

bool CGameRealtimeRemoteUpdateListener::Update()
{
	if (m_CameraUpdateInfo)
	{
		UpdateCamera(m_CameraUpdateInfo);
		m_CameraUpdateInfo=NULL;
	}

	return true;
}

bool CGameRealtimeRemoteUpdateListener::UpdateGameData(XmlNodeRef oXmlNode, u8 * auchBinaryData)
{
	string oSyncType = oXmlNode->getAttr("Type");

	if ( oSyncType.compare("ChangeLevel")==0 )
	{
		CloseLevel();
	}
	else if ( oSyncType.compare("Camera")==0 )
	{
		m_CameraUpdateInfo=oXmlNode;
		return false;
	}
	return true;
}

void CGameRealtimeRemoteUpdateListener::UpdateCamera(XmlNodeRef oXmlNode)
{
	i32	nStartSync(0);

	if (!oXmlNode->getAttr("Position",m_Position))
		return;

	if (!oXmlNode->getAttr("Direction",m_ViewDirection))
		return;

	if (oXmlNode->getAttr("StartSync",nStartSync))
	{
		if (nStartSync)
		{				
			CameraSync();
		}
		else
		{
			EndCameraSync();
		}
	}
}

void CGameRealtimeRemoteUpdateListener::CameraSync()
{
	IGame *pGame = gEnv->pGame;
	if(!pGame)
		return;

	IGameFramework *pGameFramework=pGame->GetIGameFramework();
	if(!pGameFramework)
		return;

	IViewSystem *pViewSystem=pGameFramework->GetIViewSystem();	
	if(!pViewSystem)
		return;

	IView *pView=pViewSystem->GetActiveView();
	if(!pView)
		return;

	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(pView->GetLinkedId());
	if(!pEntity)
		return;

	CPlayer *pPlayer=static_cast<CPlayer *>(pGameFramework->GetClientActor());
	if ( !pPlayer )
		return;

	IEntity * pPlayerEntity = pPlayer->GetEntity();
	if (!pPlayerEntity)
		return;

	IPhysicalEntity * piPlayerPhysics = pPlayerEntity->GetPhysics();
	if ( !piPlayerPhysics )
		return;

	pe_player_dimensions dim;
	piPlayerPhysics->GetParams( &dim );

	// if delta from head to center of character is zero, then we need to recompute it
	if( m_headPositionDelta.GetLength() == 0 )
	{
		Vec3 camPos  = pViewSystem->GetActiveView()->GetCurrentParams()->position;
		const Vec3& entPos = pEntity->GetPos();
		m_headPositionDelta = camPos - entPos;
	}
	
	// offset the camera position by the current delta to put the head eye position to center of character
	m_Position -= m_headPositionDelta;

	pPlayerEntity->Hide(false);
	pEntity->SetPos(m_Position);
	pViewSystem->SetOverrideCameraRotation(true,Quat::CreateRotationVDir(m_ViewDirection));

#ifndef _RELEASE
	if ( m_bCameraSync == false && m_nPreviousFlyMode != eCameraModes_fly_mode_noclip ) 
	{
		m_nPreviousFlyMode=pPlayer->GetFlyMode();
		pPlayer->SetFlyMode(eCameraModes_fly_mode_noclip);
	}
#endif

	pPlayerEntity->Hide(true);
	m_bCameraSync = true;
}

void CGameRealtimeRemoteUpdateListener::EndCameraSync()
{
	IGame *pGame = gEnv->pGame;
	if(!pGame)
		return;

	IGameFramework *pGameFramework=pGame->GetIGameFramework();								
	if(!pGameFramework)
		return;

	IViewSystem *pViewSystem=pGameFramework->GetIViewSystem();	
	if(!pViewSystem)
		return;

	IView *pView=pViewSystem->GetActiveView();
	if(!pView)
		return;

	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(pView->GetLinkedId());
	if(!pEntity)
		return;

	CPlayer *pPlayer=static_cast<CPlayer *>(pGameFramework->GetClientActor());
	if (!pPlayer)
		return;

	IEntity * pPlayerEntity = pPlayer->GetEntity();
	if ( !pPlayerEntity )
		return;

#ifndef _RELEASE
	pPlayer->SetFlyMode(m_nPreviousFlyMode);
#endif
	pPlayerEntity->Hide(false);
	pViewSystem->SetOverrideCameraRotation(false,Quat::CreateRotationVDir(m_ViewDirection));
	pEntity->SetRotation(Quat::CreateRotationVDir(m_ViewDirection));
	m_bCameraSync = false;
	// reset our head-position delta on end camera sync
	m_headPositionDelta.zero();
}

void CGameRealtimeRemoteUpdateListener::CloseLevel()
{
	if ( m_bCameraSync )
		EndCameraSync();

	char packPath[256];
	g_pGame->GetIGameFramework()->EndGameContext();
	
	if ( !g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel() )
		return;

	tukk fullPath = g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel()->GetPath();

	drx_sprintf(packPath, "%s\\*.pak", fullPath);
	gEnv->pDrxPak->ClosePacks(packPath,0);
}

