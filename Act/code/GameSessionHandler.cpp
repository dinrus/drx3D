// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Дефолтная имплементация обработчика сессии.

   -------------------------------------------------------------------------
   История:
   - 08:12:2009 : Created By Ben Johnson

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>
#include <drx3D/Act/GameSessionHandler.h>
#include <drx3D/Act/DinrusAction.h>

//-------------------------------------------------------------------------
CGameSessionHandler::CGameSessionHandler()
{
}

//-------------------------------------------------------------------------
CGameSessionHandler::~CGameSessionHandler()
{
}

//-------------------------------------------------------------------------
void CGameSessionHandler::JoinSessionFromConsole(DrxSessionID sessionID)
{
}

//-------------------------------------------------------------------------
i32 CGameSessionHandler::EndSession()
{
	return 1;
}

//-------------------------------------------------------------------------
i32 CGameSessionHandler::StartSession()
{
	return 1;
}

//-------------------------------------------------------------------------
void CGameSessionHandler::LeaveSession()
{
}

//-------------------------------------------------------------------------
void CGameSessionHandler::OnUserQuit()
{
}

//-------------------------------------------------------------------------
void CGameSessionHandler::OnGameShutdown()
{
}

//-------------------------------------------------------------------------
DrxSessionHandle CGameSessionHandler::GetGameSessionHandle() const
{
	return DrxSessionInvalidHandle;
}

//-------------------------------------------------------------------------
bool CGameSessionHandler::ShouldMigrateNub() const
{
	return true;
}

//-------------------------------------------------------------------------
bool CGameSessionHandler::IsMultiplayer() const
{
	return false;
}

//-------------------------------------------------------------------------
i32 CGameSessionHandler::GetNumberOfExpectedClients()
{
	return 0;
}

//-------------------------------------------------------------------------
bool CGameSessionHandler::IsGameSessionMigrating() const
{
	return false;
}

//-------------------------------------------------------------------------
bool CGameSessionHandler::IsMidGameLeaving() const
{
	return false;
}

bool CGameSessionHandler::ShouldCallMapCommand(tukk pLevelName, tukk pGameRules)
{
	IPlatformOS* pPlatform = gEnv->pSystem->GetPlatformOS();

	SStreamingInstallProgress progress;
	pPlatform->QueryStreamingInstallProgressForLevel(pLevelName, &progress);
	const bool bLevelReady = SStreamingInstallProgress::eState_Completed == progress.m_state;
	if (!bLevelReady)
	{
		gEnv->pSystem->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_NOT_READY, (UINT_PTR)pLevelName, 0);
	}
	return bLevelReady;
}
