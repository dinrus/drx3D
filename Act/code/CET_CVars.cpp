// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/CET_CVars.h>
#include <drx3D/Act/GameServerChannel.h>
#include <drx3D/Network/NetHelpers.h>

class CConsoleVarSender : public ICVarDumpSink
{
public:
	CConsoleVarSender(IConsoleVarSink* pSink) : m_pSink(pSink) {}

	virtual void OnElementFound(ICVar* pCVar)
	{
		i32 flags = pCVar->GetFlags();
		if ((flags & VF_REQUIRE_APP_RESTART) || ((flags & VF_NET_SYNCED) == 0))
		{
			return;
		}
		m_pSink->OnAfterVarChange(pCVar);
	}

private:
	IConsoleVarSink* m_pSink;
};

class CCET_CVarSync : public CCET_Base
{
public:
	CCET_CVarSync(CGameServerChannel* pChannel) : m_pChannel(pChannel) {}

	tukk                 GetName() { return "CVarSync"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CBatchSyncVarsSink pSink(m_pChannel);
		CConsoleVarSender cvarSender(&pSink);
		gEnv->pConsole->DumpCVars(&cvarSender, 0);
		pSink.OnAfterVarChange(0);
		return eCETR_Ok;
	}

private:
	CGameServerChannel* m_pChannel;
};

void AddCVarSync(IContextEstablisher* pEst, EContextViewState state, CGameServerChannel* pChannel)
{
	pEst->AddTask(state, new CCET_CVarSync(pChannel));
}

CBatchSyncVarsSink::CBatchSyncVarsSink(CGameServerChannel* pChannel)
{
	m_pChannel = pChannel;
}

// IConsoleVarSink
bool CBatchSyncVarsSink::OnBeforeVarChange(ICVar* pVar, tukk sNewValue)
{
#if LOG_CVAR_USAGE
	DrxLog("[CVARS]: [CHANGED] CBatchSyncVarsSink::OnBeforeVarChange(): variable [%s] with a value of [%s]; %s changing to [%s]",
	       pVar->GetName(),
	       pVar->GetString(),
	       (gEnv->bServer) ? "SERVER" : "CLIENT",
	       sNewValue);
#endif // LOG_CVAR_USAGE
	return true;
}

void CBatchSyncVarsSink::OnAfterVarChange(ICVar* pVar)
{
	if (!pVar || (pVar->GetFlags() & VF_NET_SYNCED))
	{
		if (m_pChannel->GetNetChannel() && !m_pChannel->GetNetChannel()->IsLocal())
		{
			if (!pVar || !m_params.Add(pVar->GetName(), pVar->GetString()))
			{
				INetSendablePtr pSendable = new CSimpleNetMessage<SClientBatchConsoleVariablesParams>(m_params, CGameClientChannel::SetBatchConsoleVariables);
				pSendable->SetGroup('cvar');
				m_pChannel->GetNetChannel()->AddSendable(pSendable, 1, &m_consoleVarSendable, &m_consoleVarSendable);

				m_params.Reset();
				if (pVar)
					m_params.Add(pVar->GetName(), pVar->GetString());
			}
		}
	}
}
