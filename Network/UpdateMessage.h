// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __UPDATEMESSAGE_H__
#define __UPDATEMESSAGE_H__

#pragma once

#include <drx3D/Network/INetwork.h>
#include <drx3D/Network/NetContext.h>
#include <drx3D/Network/SyncContext.h>
#include <drx3D/Network/ContextView.h>
#include <drx3D/Network/History.h>

class CContextView;
typedef _smart_ptr<CContextView> CContextViewPtr;

struct IUpdateAck
{
	virtual ~IUpdateAck(){}
	virtual void   Execute(u32 nFromSeq, bool bAck) = 0;
	virtual size_t GetSize() = 0;
};

struct SUpdateMessageConfig
{
	SUpdateMessageConfig() : m_pStartUpdateDef(0), m_pView(0) {}
	const SNetMessageDef* m_pStartUpdateDef;
	CContextViewPtr       m_pView;
	SNetObjectID          m_netID;
};

class CUpdateMessage : public INetSendable, protected SUpdateMessageConfig
{
public:
	CUpdateMessage(const SUpdateMessageConfig& cfg, u32 flags);
	~CUpdateMessage();

	virtual tukk GetDescription();

#if ENABLE_PACKET_PREDICTION
	virtual SMessageTag        GetMessageTag(INetSender* pSender, IMessageMapper* mapper);
#endif
	virtual EMessageSendResult Send(INetSender* pSender);
	virtual void UpdateState(u32 nFromSeq, ENetSendableStateUpdate);
	virtual size_t             GetSize();
	virtual void               GetPositionInfo(SMessagePositionInfo& pos);

	void                       SetHandle(SSendableHandle hdl) { m_handle = hdl; }

protected:
	enum EWriteHook
	{
		eWH_BeforeBegin,
		eWH_DuringBegin,
	};

	struct SHistorySyncContext
	{
		CHistory::CSyncContext ctx[eH_NUM_HISTORIES][NumAspects];
	};

#if ENABLE_PACKET_PREDICTION
	virtual NetworkAspectType ComputeSendAspectMask(INetSender* pSender, SHistorySyncContext& hsc, SContextViewObject* pViewObj, SContextViewObjectEx* pViewObjEx, bool& willSend);
#endif

	virtual EMessageSendResult InitialChecks(SHistorySyncContext& hsc, INetSender* pSender, SContextViewObject* pViewObj, SContextViewObjectEx* pViewObjEx);
	virtual EMessageSendResult WriteHook(EWriteHook when, INetSender* pSender) { return eMSR_SentOk; }
	virtual bool               CheckHook()                                     { return false; }

	void                       SentData()                                      { m_anythingSent = true; m_wasSent = true; }

#if ENABLE_THIN_BINDS
	NetworkAspectType m_bindAspectMask;
#endif // ENABLE_THIN_BINDS

private:
	u32            m_syncFlags;
	NetworkAspectType m_maybeSendHistories[eH_NUM_HISTORIES];
	NetworkAspectType m_sentHistories[eH_NUM_HISTORIES];
	u32            m_sentAspectVersions[NumAspects];
	NetworkAspectType m_immediateResendAspects;
	bool              m_wasSent;

	SSendableHandle   m_handle;

	EMessageSendResult  SendMain(SHistorySyncContext& hsc, INetSender* pSender);
	virtual tukk GetBasicDescription() const { return "Update"; }
	void                UpdateMsgHandle();

	// send any scheduled attachments
	EMessageSendResult SendAttachments(INetSender* pSender, ERMIAttachmentType attachments);
	void               UpdateAttachmentsState(u32 nFromSeq, ENetSendableStateUpdate update);
	// are there any scheduled attachments?
	static bool        AnyAttachments(CContextView* pView, SNetObjectID id);

	struct SSentAttachment
	{
		SAttachmentIndex       idx;
		IRMIMessageBodyPtr     pMsg;
		CContextViewObjectLock lock;
	};

	typedef std::vector<SSentAttachment> TAttachments;
	TAttachments m_attachments;

	bool         m_anythingSent;

	string       m_description;
};

class CRegularUpdateMessage : public CUpdateMessage
{
public:
	static CRegularUpdateMessage* Create(const SUpdateMessageConfig& cfg, u32 flags);

	virtual void UpdateState(u32 nFromSeq, ENetSendableStateUpdate);

private:
	CRegularUpdateMessage(const SUpdateMessageConfig& cfg, u32 flags) : CUpdateMessage(cfg, flags) {}

	virtual void DeleteThis();

	TMemHdl m_memhdl;
};

#endif
