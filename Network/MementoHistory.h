// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MEMENTOHISTORY_H__
#define __MEMENTOHISTORY_H__

#pragma once

#include <drx3D/Network/History.h>

class CMementoReadSyncedItems : public CRegularlySyncedItems
{
public:
	virtual ~CMementoReadSyncedItems(){}
	CMementoReadSyncedItems();

private:
	void Destroy(u32 data);
};

class CMementoHistory : public CHistory
{
public:
	CMementoHistory(CContextView* pView);

	virtual bool ReadCurrentValue(const SReceiveContext& ctx, bool commit);
	virtual void HandleEvent(const SHistoricalEvent& event);
	virtual void Flush(const SSyncContext& ctx);

private:
	// bits for the bonus byte in the memento stream
	static u8k POLLUTION_BIT = 0x80;
	static u8k OWNER_BIT = 0x40;
	static u8k ILLEGAL_BITS = 0x08 | 0x10 | 0x20;
	static u8k PROFILE_MASK = 0x07;

	bool         OwnData(bool writing, const SSyncContext& ctx);

	virtual bool NeedToSync(const SSyncContext& ctx, CSyncContext* pSyncContext, const CRegularlySyncedItem& item);
	virtual bool CanSync(const SSyncContext& ctx, CSyncContext* pSyncContext, const CRegularlySyncedItem& item);
	virtual bool SendCurrentValue(const SSendContext& ctx, CSyncContext* pSyncContext, u32& newValue);
	virtual void Destroy(u32 data);

	struct SSerializeParams
	{
		SSerializeParams(const SSyncContext& c, TSerialize s)
			: ctx(c)
			, ser(s)
			, pTimeValue(nullptr)
			, profile(0)
			, pElem(nullptr)
			, commit(false)
			, isOwner(false)
			, pAspectData(nullptr)
			, pNewValue(nullptr)
		{
		}
		const SSyncContext& ctx;
		TSerialize          ser;
		CTimeValue*         pTimeValue;
		u8               profile;
		ChunkID             chunkID;
		SElem*              pElem;
		bool                commit;
		bool                isOwner;
		TMemHdl*            pAspectData;
		// out values
		u32*             pNewValue;
	};

	bool Serialize(const SSerializeParams& params);

	class CMementoInputStream;

	CMementoReadSyncedItems m_receiveBuffer;
};

#endif
