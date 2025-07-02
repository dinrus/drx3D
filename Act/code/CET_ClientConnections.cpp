// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/CET_GameRules.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Act/GameClientChannel.h>
#include <drx3D/Act/GameServerChannel.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/ActionGame.h>

#include <drx3D/Network/NetHelpers.h>
#include <drx3D/Network/INetwork.h>

class CCET_OnClient : public CCET_Base
{
public:
	CCET_OnClient(bool(INetworkedClientListener::* func)(i32, bool), tukk name, bool isReset) : m_func(func), m_name(name), m_isReset(isReset) {}

	tukk                 GetName() { return m_name; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		if (INetChannel* pNC = state.pSender)
		{
			if (CGameServerChannel* pSC = (CGameServerChannel*)pNC->GetGameChannel())
			{
				for (INetworkedClientListener* pListener : CDrxAction::GetDrxAction()->GetNetworkClientListeners())
				{
					if (!(pListener->*m_func)(pSC->GetChannelId(), m_isReset))
					{
						return eCETR_Failed;
					}
				}

				return eCETR_Ok;
			}
		}
		
		GameWarning("%s: No channel id", m_name);
		return eCETR_Failed;
	}

private:
	bool (INetworkedClientListener::* m_func)(i32, bool);
	tukk m_name;
	bool        m_isReset;
};

void AddOnClientConnect(IContextEstablisher* pEst, EContextViewState state, bool isReset)
{
	if (!(gEnv->bHostMigrating && gEnv->bMultiplayer))
	{
		pEst->AddTask(state, new CCET_OnClient(&INetworkedClientListener::OnClientConnectionReceived, "OnClientConnectionReceived", isReset));
	}
}

void AddOnClientEnteredGame(IContextEstablisher* pEst, EContextViewState state, bool isReset)
{
	if (!(gEnv->bHostMigrating && gEnv->bMultiplayer))
	{
		pEst->AddTask(state, new CCET_OnClient(&INetworkedClientListener::OnClientReadyForGameplay, "OnClientReadyForGameplay", isReset));
	}
}