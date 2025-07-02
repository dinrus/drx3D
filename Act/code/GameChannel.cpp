// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Common functionality for CGameServerChannel, CGameClientChannel

   -------------------------------------------------------------------------
   История:
   - 6:10:2004   11:38 : Created by Craig Tiller

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameChannel.h>
#include <drx3D/Act/PhysicsSync.h>
#include <drx3D/Act/GameObject.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/GameContext.h>

CGameChannel::CGameChannel() : m_pNetChannel(NULL), m_pContext(NULL), m_playerId(0)
{
	m_pPhysicsSync.reset(new CPhysicsSync(this));
}

CGameChannel::~CGameChannel()
{
}

void CGameChannel::Init(INetChannel* pNetChannel, CGameContext* pGameContext)
{
	m_pNetChannel = pNetChannel;
	m_pContext = pGameContext;
}

void CGameChannel::ConfigureContext(bool isLocal)
{
	if (isLocal)
		return;

	GameWarning("Need to set physics time here");
}

void CGameChannel::SetPlayerId(EntityId id)
{
	m_playerId = id;
	if (id)
		m_pNetChannel->DeclareWitness(id);
}
