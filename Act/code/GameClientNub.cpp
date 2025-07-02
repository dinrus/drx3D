// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 24:8:2004   10:58 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/GameClientNub.h>
#include <drx3D/Act/GameClientChannel.h>

CGameClientNub::~CGameClientNub()
{
	delete m_pClientChannel;
}

void CGameClientNub::Release()
{
	// don't delete because it's not dynamically allocated
}

SCreateChannelResult CGameClientNub::CreateChannel(INetChannel* pChannel, tukk pRequest)
{
	if (pRequest)
	{
		GameWarning("CGameClientNub::CreateChannel: pRequest is non-null, it should not be");
		DRX_ASSERT(false);
		SCreateChannelResult res(eDC_GameError);
		drx_strcpy(res.errorMsg, "CGameClientNub::CreateChannel: pRequest is non-null, it should not be");
		return res;
	}

	if (m_pClientChannel)
	{
		GameWarning("CGameClientNub::CreateChannel: m_pClientChannel is non-null, it should not be");
		DRX_ASSERT(false);
		SCreateChannelResult res(eDC_GameError);
		drx_strcpy(res.errorMsg, "CGameClientNub::CreateChannel: m_pClientChannel is non-null, it should not be");
		return res;
	}

	if (CDrxAction::GetDrxAction()->IsGameSessionMigrating())
	{
		pChannel->SetMigratingChannel(true);
	}

	m_pClientChannel = new CGameClientChannel(pChannel, m_pGameContext, this);

	ICVar* pPass = gEnv->pConsole->GetCVar("sv_password");
	if (pPass && gEnv->bMultiplayer)
		pChannel->SetPassword(pPass->GetString());

	return SCreateChannelResult(m_pClientChannel);
}

void CGameClientNub::FailedActiveConnect(EDisconnectionCause cause, tukk description)
{
	GameWarning("Failed connecting to server: %s", description);
	CDrxAction::GetDrxAction()->OnActionEvent(SActionEvent(eAE_connectFailed, i32(cause), description));
}

INetChannel* CGameClientNub::GetNetChannel()
{
	return m_pClientChannel != nullptr ? m_pClientChannel->GetNetChannel() : nullptr; 
}

void CGameClientNub::Disconnect(EDisconnectionCause cause, tukk msg)
{
	if (m_pClientChannel)
		m_pClientChannel->GetNetChannel()->Disconnect(cause, msg);
}

void CGameClientNub::ClientChannelClosed()
{
	DRX_ASSERT(m_pClientChannel);
	m_pClientChannel = NULL;
}

void CGameClientNub::GetMemoryUsage(IDrxSizer* s) const
{
	s->Add(*this);
	if (m_pClientChannel)
		m_pClientChannel->GetMemoryStatistics(s);
}
