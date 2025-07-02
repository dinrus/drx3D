// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	This file implements dispatching RMI calls in C++ to
                relevant game object code

   -------------------------------------------------------------------------
   История:
    - 24 Oct 2005 : Created by Craig Tiller

*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameObject.h>
#include <drx3D/Act/GameObjectDispatch.h>
#include <drx3D/Act/GameContext.h>

CGameObjectDispatch* CGameObjectDispatch::m_pGOD = NULL;

CGameObjectDispatch::CGameObjectDispatch() : m_bSafety(false)
{
	DRX_ASSERT(m_pGOD == NULL);
	m_pGOD = this;
}

CGameObjectDispatch::~CGameObjectDispatch()
{
	m_pGOD = NULL;
}

void CGameObjectDispatch::RegisterInterface(SGameObjectExtensionRMI* pMessages, size_t nCount)
{
	if (nCount == 0)
		return;

	// watch out for duplicate registrations...
	for (size_t i = 0; i < m_messages.size(); i++)
		if (m_messages[i]->pBase == pMessages)
			return;
	// messages already registered

	if (m_bSafety)
	{
		DrxFatalError("CGameObjectDispatch::RegisterInterface: occurs too late");
		return;
	}

	// actually build protocol definitions for the messages
	for (size_t i = 0; i < nCount; i++)
	{
		std::vector<SNetMessageDef>* pMsgDef = pMessages[i].isServerCall ? &m_serverCalls : &m_clientCalls;

		SNetMessageDef md;
		md.description = pMessages[i].description;
		md.handler = Trampoline;
		md.nUser = m_messages.size();
		switch (pMessages[i].attach)
		{
		case eRAT_NoAttach:     // Intentional fall-through
		case eRAT_Urgent:       // Intentional fall-through
		case eRAT_Independent:
			// NoAttach, Urgent and Independent get the reliability specified at RMI declaration
			md.reliability = pMessages[i].reliability;
			break;
		default:
			// Everything else's reliability/orderedness is based on the object they're attached to
			md.reliability = eNRT_UnreliableOrdered;
		}
		md.parallelFlags = eMPF_BlocksStateChange | eMPF_DiscardIfNoEntity;
		if (pMessages[i].lowDelay)
			md.parallelFlags |= eMPF_NoSendDelay;

		pMsgDef->push_back(md); // Outgoing message definitions
		m_messages.push_back(pMessages + i); // Incoming message definitions
	}
}

TNetMessageCallbackResult CGameObjectDispatch::Trampoline(
  u32 userId,
  INetMessageSink* handler,
  TSerialize serialize,
  u32 curSeq,
  u32 oldSeq,
  u32 timeFraction32,
  EntityId* pEntityId,
  INetChannel* pNetChannel)
{
	const SGameObjectExtensionRMI* pRMI = m_pGOD->m_messages[userId];
	IRMIAtSyncItem* pItem = (IRMIAtSyncItem*) pRMI->decoder(serialize, pEntityId, pNetChannel);
	if (pItem)
	{
		CDrxAction* pDrxAction = CDrxAction::GetDrxAction();
		CGameContext* pGameContext = pDrxAction ? pDrxAction->GetGameContext() : NULL;
		INetContext* pNetContext = pGameContext ? pGameContext->GetNetContext() : NULL;

		if (pNetContext)
			pNetContext->LogCppRMI(*pEntityId, pItem);
	}
	return TNetMessageCallbackResult(pItem != 0, pItem);
}

void CGameObjectDispatch::LockSafety()
{
	if (m_bSafety)
		return;

	for (size_t i = 0; i < m_serverCalls.size(); i++)
		m_messages[m_serverCalls[i].nUser]->pMsgDef = &m_serverCalls[i];
	for (size_t i = 0; i < m_clientCalls.size(); i++)
		m_messages[m_clientCalls[i].nUser]->pMsgDef = &m_clientCalls[i];

	m_bSafety = true;
}

bool CGameObjectDispatch::CProtocolDef::IsServer()
{
	return this == &m_pGOD->m_serverDef;
}

void CGameObjectDispatch::CProtocolDef::DefineProtocol(IProtocolBuilder* pBuilder)
{
	m_pGOD->LockSafety();

	std::vector<SNetMessageDef>* pSending = IsServer() ? &m_pGOD->m_clientCalls : &m_pGOD->m_serverCalls;
	std::vector<SNetMessageDef>* pReceiving = IsServer() ? &m_pGOD->m_serverCalls : &m_pGOD->m_clientCalls;

	SNetProtocolDef protoSending;
	protoSending.nMessages = pSending->size();
	protoSending.vMessages = protoSending.nMessages ? &*pSending->begin() : 0;
	SNetProtocolDef protoReceiving;
	protoReceiving.nMessages = pReceiving->size();
	protoReceiving.vMessages = protoReceiving.nMessages ? &*pReceiving->begin() : 0;

	pBuilder->AddMessageSink(this, protoSending, protoReceiving);
}

bool CGameObjectDispatch::CProtocolDef::HasDef(const SNetMessageDef* pDef)
{
	return
	  pDef >= &*m_pGOD->m_serverCalls.begin() && pDef < &*m_pGOD->m_serverCalls.end() ||
	  pDef >= &*m_pGOD->m_clientCalls.begin() && pDef < &*m_pGOD->m_clientCalls.end();
}

void CGameObjectDispatch::GetMemoryUsage(IDrxSizer* s) const
{
	//s->AddObject(m_messages);
	//s->AddObject(m_clientCalls);
	//s->AddObject(m_serverCalls);
}
