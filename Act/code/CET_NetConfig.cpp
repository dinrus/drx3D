// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/CET_NetConfig.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/GameClientChannel.h>
#include <drx3D/Act/GameClientNub.h>
#include <drx3D/Act/GameServerChannel.h>
#include <drx3D/Act/GameServerNub.h>
#include <drx3D/Network/NetHelpers.h>

/*
 * Established context
 */

class CCET_EstablishedContext : public CCET_Base
{
public:
	CCET_EstablishedContext(i32 token) : m_token(token) {}

	tukk                 GetName() { return "EstablishedContext"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CDrxAction::GetDrxAction()->GetGameContext()->GetNetContext()->EstablishedContext(m_token);
		return eCETR_Ok;
	}

private:
	i32 m_token;
};

void AddEstablishedContext(IContextEstablisher* pEst, EContextViewState state, i32 token)
{
	pEst->AddTask(state, new CCET_EstablishedContext(token));
}

/*
 * Declare witness
 */

class CCET_DeclareWitness : public CCET_Base
{
public:
	tukk                 GetName() { return "DeclareWitness"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		if (CGameClientNub* pNub = CDrxAction::GetDrxAction()->GetGameClientNub())
		{
			if (CGameClientChannel* pChannel = pNub->GetGameClientChannel())
			{
				if (IActor* pActor = CDrxAction::GetDrxAction()->GetClientActor())
				{
					pChannel->GetNetChannel()->DeclareWitness(pActor->GetEntityId());
					return eCETR_Ok;
				}
			}
		}
		return eCETR_Failed;
	}
};

void AddDeclareWitness(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_DeclareWitness);
}

/*
 * Delegate authority to player
 */

class CCET_DelegateAuthority_ToClientActor : public CCET_Base
{
public:
	tukk GetName() { return "DelegateAuthorityToClientActor"; }

public:
	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		EntityId entityId = GetEntity(state);
		if (!entityId || !gEnv->pEntitySystem->GetEntity(entityId))
			return eCETR_Ok; // Proceed even if there is no Actor
		CDrxAction::GetDrxAction()->GetGameContext()->GetNetContext()->DelegateAuthority(entityId, state.pSender);
		return eCETR_Ok;
	}

private:
	EntityId GetEntity(SContextEstablishState& state)
	{
		if (!state.pSender)
			return 0;
		CGameChannel* pGameChannel = static_cast<CGameChannel*>(state.pSender->GetGameChannel());
		if (pGameChannel)
			return pGameChannel->GetPlayerId();
		return 0;
	}
};

void AddDelegateAuthorityToClientActor(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_DelegateAuthority_ToClientActor());
}

/*
 * Clear player ids
 */

class CCET_ClearPlayerIds : public CCET_Base
{
public:
	tukk                 GetName() { return "ClearPlayerIds"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		if (CGameServerNub* pNub = CDrxAction::GetDrxAction()->GetGameServerNub())
		{
			TServerChannelMap* pMap = pNub->GetServerChannelMap();
			for (TServerChannelMap::iterator it = pMap->begin(); it != pMap->end(); ++it)
				it->second->ResetPlayerId();
		}
		if (CGameClientNub* pCNub = CDrxAction::GetDrxAction()->GetGameClientNub())
		{
			if (CGameChannel* pChannel = pCNub->GetGameClientChannel())
				pChannel->ResetPlayerId();
		}
		return eCETR_Ok;
	}
};

void AddClearPlayerIds(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_ClearPlayerIds());
}
