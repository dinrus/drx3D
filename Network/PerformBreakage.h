// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _NET_PERFROM_BREAKAGE__H__
#define _NET_PERFROM_BREAKAGE__H__

#pragma once

#include <drx3D/Network/ClientContextView.h>

/*
   ================================================================================================================
   Perform Simple Breakage
   ================================================================================================================
 */
class CPerformBreakSimpleBase
{
public:
	CPerformBreakSimpleBase();
	virtual ~CPerformBreakSimpleBase();

	void SerialiseNetIds(TSerialize ser);

public:
	enum { MaxEntitiesPerBreak = 32 };

	SNetObjectID m_entityNetIds[MaxEntitiesPerBreak];
	i32          m_numEntities;
};

class CPerformBreakSimpleServer : public INetSendable, public CPerformBreakSimpleBase //, public CMultiThreadRefCount
{
public:
	// Invoke server breakage
	static CPerformBreakSimpleServer* GotBreakage(CServerContextView* pView, const SNetIntBreakDescription* pDesc);

private:
	CPerformBreakSimpleServer();
	void               Init(CServerContextView* pView, const SNetIntBreakDescription* pDesc, SSendableHandle msgHandle, SNetObjectID dependentId);
#if ENABLE_PACKET_PREDICTION
	SMessageTag        GetMessageTag(INetSender* pSender, IMessageMapper* mapper);
#endif
	EMessageSendResult Send(INetSender* pSender);
	void               UpdateState(u32 nFromSeq, ENetSendableStateUpdate state);
	void               GetPositionInfo(SMessagePositionInfo& pos) {}
	size_t             GetSize()                                  { return sizeof(*this); }
	tukk        GetDescription()                           { return "PerformBreakSimpleServerSendable"; }

private:
	_smart_ptr<CServerContextView> m_pView;
	_smart_ptr<CNetContextState>   m_pContextState;
	const SNetIntBreakDescription* m_pDesc;
	SSendableHandle                m_msgHandle;
	SNetObjectID                   m_dependentNetId;
};

class CPerformBreakSimpleClient : public CPerformBreakSimpleBase, public INetBreakageSimplePlayback
{
public:
	CPerformBreakSimpleClient(CClientContextView* pClientView);
	~CPerformBreakSimpleClient();

	// INetBreakageSimplePlayback
	virtual void BeginBreakage();
	virtual void FinishedBreakage();
	virtual void BindSpawnedEntity(EntityId id, i32 spawnIdx);
	// ~INetBreakageSimplePlayback

	void SerialiseNetIds(TSerialize ser);

public:
	enum {k_invalid = 0, k_started = 1, k_stopped = 2};

	_smart_ptr<CClientContextView> m_pClientView;
	i16                          m_numEntitiesCollected;
	i16                          m_state;
};

#endif
