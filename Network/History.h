// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __HISTORY_H__
#define __HISTORY_H__

#pragma once

#include <drx3D/Network/RegularlySyncedItem.h>
#include <drx3D/Network/SyncContext.h>

enum EHistorySendResult
{
	eHSR_Ok,
	eHSR_NotNeeded,
	eHSR_Failed,
};

enum EHistoricalEvent
{
	eHE_BindAspect,
	eHE_UnbindAspect,
	eHE_Pollute,
	eHE_Reset
};

struct SHistoricalEvent
{
	SHistoricalEvent(EHistoricalEvent he) : event(he), seq(~u32(0)), index(0xcc), pView(0), pViewObj(0), pViewObjEx(0) {}
	EHistoricalEvent      event;
	u32                itemId;
	SNetObjectID          objId;
	NetworkAspectID       index;
	u32                seq;
	CContextView*         pView;
	SContextViewObject*   pViewObj;
	SContextViewObjectEx* pViewObjEx;
};

class CHistory : protected CRegularlySyncedItems
{
public:
	NetworkAspectType indexMask;

	CHistory(u32 idBase, u32 idMul) : CRegularlySyncedItems(idBase, idMul)
	{
		++g_objcnt.history;
	}
	virtual ~CHistory()
	{
		--g_objcnt.history;
	}

	void         Ack(const SSyncContext& ctx, bool ack);
	virtual void Flush(const SSyncContext& ctx);
	virtual bool ReadCurrentValue(const SReceiveContext& ctx, bool commit) = 0;
	SElem*       FindPrevElem(const SSyncContext& ctx);
	void         Reset();

	virtual void HandleEvent(const SHistoricalEvent& event) = 0;

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CHistory");

		pSizer->Add(*this);
		CRegularlySyncedItems::GetMemoryStatistics(pSizer);
	}

	class CSyncContext
	{
	public:
		CSyncContext() : m_pHistory(0), m_pElem(0) {}
		// MUST call one and only one of NeedToSync and PrepareToSync
		bool               NeedToSync(CHistory* pHistory, const SSyncContext& ctx);    // return true if we have new data
		bool               PrepareToSync(CHistory* pHistory, const SSyncContext& ctx); // return true if we could sync even if we wanted to
		EHistorySendResult Send(const SSendContext& ctx);

		SElem*             GetPrevElem(const SSyncContext& ctx)
		{
			u32 curResizeCount = m_item.GetResizeCount();
			if (m_resizesWhenElemTaken != curResizeCount)
			{
				m_pElem = m_item.FindPrevElem(ctx.basisSeq, ctx.currentSeq);
				m_resizesWhenElemTaken = curResizeCount;
			}
			return m_pElem;
		}

	private:
		CHistory*            m_pHistory;
		CRegularlySyncedItem m_item;
		SElem*               m_pElem;
		u32               m_resizesWhenElemTaken;
	};

private:
	virtual bool NeedToSync(const SSyncContext& ctx, CSyncContext* pSyncContext, const CRegularlySyncedItem& item) = 0;
	virtual bool CanSync(const SSyncContext& ctx, CSyncContext* pSyncContext, const CRegularlySyncedItem& item) = 0;
	virtual bool SendCurrentValue(const SSendContext& ctx, CSyncContext* pSyncContext, u32& newValue) = 0;
};

#endif
