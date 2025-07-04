// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  an INetSendable that doesn't send anything (useful as a
               placeholder in the message queue)
   -------------------------------------------------------------------------
   История:
   - 26/07/2004   : Created by Craig Tiller
*************************************************************************/

#ifndef __NULLSENDABLE_H__
#define __NULLSENDABLE_H__

#pragma once

#include <drx3D/Network/Network.h>

class CNullSendable : public INetSendable
{
public:
	~CNullSendable()
	{
		--g_objcnt.nullSendable;
	}

	virtual size_t      GetSize() { return sizeof(*this); }
#if ENABLE_PACKET_PREDICTION
	virtual SMessageTag GetMessageTag(INetSender* pSender, IMessageMapper* mapper)
	{
		SMessageTag mTag;
		mTag.messageId = 0xFFFFFFFF;

		return mTag;
	}
#endif
	virtual EMessageSendResult Send(INetSender* pSender)                                    { return eMSR_SentOk; }
	virtual void               UpdateState(u32 nFromSeq, ENetSendableStateUpdate update) {}
	virtual tukk        GetDescription()                                             { return "NullSendable"; }
	virtual void               GetPositionInfo(SMessagePositionInfo& pos)                   {}

	static CNullSendable*      Alloc(u32 flags, ENetReliabilityType reliability);

private:
	CNullSendable(u32 flags, ENetReliabilityType reliability) : INetSendable(flags, reliability)
	{
		++g_objcnt.nullSendable;
	}
	virtual void DeleteThis();

	TMemHdl m_memhdl;
};

#endif
