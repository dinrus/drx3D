// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 11:8:2004   11:40 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameServerChannel.h>
#include <drx3D/Act/GameClientChannel.h>
#include <drx3D/Act/GameServerNub.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/GameRulesSystem.h>

ICVar* CGameServerChannel::sv_timeout_disconnect = 0;

CGameServerChannel::CGameServerChannel(INetChannel* pNetChannel, CGameContext* pGameContext, CGameServerNub* pServerNub)
	: m_pServerNub(pServerNub), m_channelId(0), m_hasLoadedLevel(false), m_onHold(false)
{
	Init(pNetChannel, pGameContext);
	DRX_ASSERT(pNetChannel);

#if NEW_BANDWIDTH_MANAGEMENT
	pNetChannel->SetServer(GetGameContext()->GetNetContext());
#else
	SetupNetChannel(pNetChannel);
#endif // NEW_BANDWIDTH_MANAGEMENT

	if (!sv_timeout_disconnect)
		sv_timeout_disconnect = gEnv->pConsole->GetCVar("sv_timeout_disconnect");

	gEnv->pConsole->AddConsoleVarSink(this);
	CDrxAction::GetDrxAction()->OnActionEvent(SActionEvent(eAE_channelCreated, 1));
}

CGameServerChannel::~CGameServerChannel()
{
	gEnv->pConsole->RemoveConsoleVarSink(this);
	CDrxAction::GetDrxAction()->OnActionEvent(SActionEvent(eAE_channelDestroyed, 1));

	Cleanup();
}

#if !NEW_BANDWIDTH_MANAGEMENT
void CGameServerChannel::SetupNetChannel(INetChannel* pNetChannel)
{
	pNetChannel->SetServer(GetGameContext()->GetNetContext(), true);
	INetChannel::SPerformanceMetrics pm;
	if (!gEnv->bMultiplayer)
		pm.pPacketRateDesired = gEnv->pConsole->GetCVar("g_localPacketRate");
	else
		pm.pPacketRateDesired = gEnv->pConsole->GetCVar("sv_packetRate");
	pm.pBitRateDesired = gEnv->pConsole->GetCVar("sv_bandwidth");
	pNetChannel->SetPerformanceMetrics(&pm);
}
#endif // !NEW_BANDWIDTH_MANAGEMENT

void CGameServerChannel::Release()
{
	if (GetNetChannel())
		delete this;
}

bool CGameServerChannel::OnBeforeVarChange(ICVar* pVar, tukk sNewValue)
{
	// This code is useful for debugging issues with networked cvars, but it's
	// very spammy so #if'ing out for now.
#if 0 // LOG_CVAR_USAGE
	i32 flags = pVar->GetFlags();
	bool netSynced = ((flags & VF_NET_SYNCED) != 0);

	DrxLog("[CVARS]: [CHANGED] CGameServerChannel::OnBeforeVarChange(): variable [%s] (%smarked VF_NET_SYNCED) with a value of [%s]; SERVER changing to [%s]",
	       pVar->GetName(),
	       (netSynced) ? "" : "not ",
	       pVar->GetString(),
	       sNewValue);
#endif // LOG_CVAR_USAGE

	return true;
}

void CGameServerChannel::OnAfterVarChange(ICVar* pVar)
{
	if (pVar->GetFlags() & VF_NET_SYNCED)
	{
		if (GetNetChannel() && !GetNetChannel()->IsLocal())
		{
			SClientConsoleVariableParams params(pVar->GetName(), pVar->GetString());
#if FAST_CVAR_SYNC
			SSendableHandle& id = GetConsoleStreamId(params.key);
			INetSendablePtr pSendable = new CSimpleNetMessage<SClientConsoleVariableParams>(params, CGameClientChannel::SetConsoleVariable);
			pSendable->SetGroup('cvar');
			GetNetChannel()->SubstituteSendable(pSendable, 1, &id, &id);
#else
			INetSendablePtr pSendable = new CSimpleNetMessage<SClientConsoleVariableParams>(params, CGameClientChannel::SetConsoleVariable);
			pSendable->SetGroup('cvar');
			GetNetChannel()->AddSendable(pSendable, 1, &m_consoleVarSendable, &m_consoleVarSendable);
#endif
		}
	}
}

void CGameServerChannel::OnDisconnect(EDisconnectionCause cause, tukk description)
{
	//DrxLogAlways("CGameServerChannel::OnDisconnect(%d, '%s')", cause, description?описание:"");
	CDrxAction::GetDrxAction()->OnActionEvent(SActionEvent(eAE_clientDisconnected, i32(cause), description));

	if (!IsOnHold() && sv_timeout_disconnect && sv_timeout_disconnect->GetIVal() > 0)
	{
		// Check if any clients want to keep this player
		for (INetworkedClientListener* pListener : CDrxAction::GetDrxAction()->GetNetworkClientListeners())
		{
			if(!pListener->OnClientTimingOut(GetChannelId(), cause, description))
			{
				if (m_pServerNub->PutChannelOnHold(this))
				{
					for (INetworkedClientListener* pRecursiveListener : CDrxAction::GetDrxAction()->GetNetworkClientListeners())
					{
						pRecursiveListener->OnClientDisconnected(GetChannelId(), cause, description, true);
					}

					m_hasLoadedLevel = false;
					return;
				}

				break;
			}
		}
	}

	for (INetworkedClientListener* pListener : CDrxAction::GetDrxAction()->GetNetworkClientListeners())
	{
		pListener->OnClientDisconnected(GetChannelId(), cause, description, false);
	}

	Cleanup();
}

void CGameServerChannel::Cleanup()
{
	m_pServerNub->RemoveChannel(GetChannelId());

	if (GetPlayerId())
	{
		gEnv->pEntitySystem->RemoveEntity(GetPlayerId(), true);
	}
}

void CGameServerChannel::DefineProtocol(IProtocolBuilder* pBuilder)
{
	pBuilder->AddMessageSink(this, CGameClientChannel::GetProtocolDef(), CGameServerChannel::GetProtocolDef());
	CDrxAction* cca = CDrxAction::GetDrxAction();
	if (cca->GetIGameObjectSystem())
		cca->GetIGameObjectSystem()->DefineProtocol(true, pBuilder);
	if (cca->GetGameContext())
		cca->GetGameContext()->DefineContextProtocols(pBuilder, true);
	cca->DefineProtocolRMI(pBuilder);
}

void CGameServerChannel::SetPlayerId(EntityId playerId)
{
	//check for banned status here
	if (m_pServerNub->CheckBanned(GetNetChannel()))
		return;

	if (CGameServerChannel* pServerChannel = m_pServerNub->GetOnHoldChannelFor(GetNetChannel()))
	{
		//cleanup onhold channel if it was not associated with us
		//normally it should be taken while creating channel, but for now, this doesn't happen
		m_pServerNub->RemoveOnHoldChannel(pServerChannel, false);
	}

	CGameChannel::SetPlayerId(playerId);
	if (GetNetChannel()->IsLocal())
		CGameClientChannel::SendSetPlayerId_LocalOnlyWith(playerId, GetNetChannel());
}

bool CGameServerChannel::CheckLevelLoaded() const
{
	return m_hasLoadedLevel;
}

void CGameServerChannel::AddUpdateLevelLoaded(IContextEstablisher* pEst)
{
	if (!m_hasLoadedLevel)
		AddSetValue(pEst, eCVS_InGame, &m_hasLoadedLevel, true, "AllowChaining");
}

#ifndef OLD_VOICE_SYSTEM_DEPRECATED
NET_IMPLEMENT_SIMPLE_ATSYNC_MESSAGE(CGameServerChannel, MutePlayer, eNRT_ReliableUnordered, eMPF_BlocksStateChange)
{
	if (GetNetChannel()->IsLocal())
		return true;

	CDrxAction::GetDrxAction()->GetGameContext()->GetNetContext()->GetVoiceContext()->Mute(param.requestor, param.id, param.mute);

	return true;
}
#endif

#if defined(GAME_CHANNEL_SYNC_CLIENT_SERVER_TIME)
NET_IMPLEMENT_SIMPLE_ATSYNC_MESSAGE(CGameServerChannel, SyncTimeServer, eNRT_ReliableOrdered, eMPF_NoSendDelay)
{
	CTimeValue value = gEnv->pTimer->GetAsyncTime();
	INetSendablePtr msg = new CSimpleNetMessage<SSyncTimeClient>(SSyncTimeClient(param.id, param.clientTime, param.serverTime, value.GetValue()), CGameClientChannel::SyncTimeClient);
	GetNetChannel()->AddSendable(msg, 0, NULL, NULL);

	return true;
}
#endif
