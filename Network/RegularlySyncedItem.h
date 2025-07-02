// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __REGULARLYSYNCEDITEM_H__
#define __REGULARLYSYNCEDITEM_H__

#include <drx3D/Network/Config.h>
#include <drx3D/Network/SyncContext.h>

class CRegularlySyncedItem;

class CRegularlySyncedItems_StaticDefs
{
public:
	struct SElem
	{
		SElem()
		{
			++g_objcnt.rselem;
		}
		~SElem()
		{
			--g_objcnt.rselem;
		}
		SElem(const SElem& rhs) : seq(rhs.seq), data(rhs.data)
		{
			++g_objcnt.rselem;
		}
		SElem& operator=(const SElem& rhs)
		{
			seq = rhs.seq;
			data = rhs.data;
			return *this;
		}
		u32 seq;
		u32 data;
	};

protected:
	struct SLinkedElem : public SElem
	{
#if DETAIL_REGULARLY_SYNCED_ITEM_DEBUG
		SLinkedElem() : nextSetter(-1) {}
		i32    nextSetter;
#endif
		u32 next  : 31;
		u32 valid : 1;
	};
};

class CRegularlySyncedItems : public CRegularlySyncedItems_StaticDefs
{
	friend class CRegularlySyncedItem;

public:
	CRegularlySyncedItems(u32 idBase, u32 idMul) : m_idBase(idBase), m_idMul(idMul), m_curSeq(0), m_resizes(10) {}
	virtual ~CRegularlySyncedItems() { NET_ASSERT(m_elems.empty()); }

	void                 GetMemoryStatistics(IDrxSizer* pSizer);
	CRegularlySyncedItem GetItem(u32 id, SHistoryRootPointer* historyElems);

	void                 Reset();

	ILINE u32         GetResizeCount() const { return m_resizes; }

private:
	virtual void Destroy(u32 data) {}

	void         GotCurSeq(u32 curSeq)
	{
		if (curSeq > m_curSeq)
			m_curSeq = curSeq;
	}

	u32                   m_idMul;
	u32                   m_idBase;
	u32                   m_curSeq;

	std::vector<SLinkedElem> m_elems;
	std::vector<u32>      m_freeElems;
	u32                   m_resizes;
	void Verify();
};

class CRegularlySyncedItem : public CRegularlySyncedItems_StaticDefs
{
	friend class CRegularlySyncedItems;

public:
	CRegularlySyncedItem() : m_pParent(nullptr), m_pRoot(nullptr), m_id(0) {}

	typedef CRegularlySyncedItems::SElem SElem;

	void          AddElem(u32 seq, u32 data);
	// WARNING: returned pointer is only valid until AddElem is called on *ANY* other item
	SElem*        FindPrevElem(u32 basisSeq, u32 curSeq) const;
	void          RemoveOlderThan(u32 seq);
	void          Flush();
	void          AckUpdate(u32 seq, bool ack);

	void          DumpStuff(tukk txt, SNetObjectID id, u32 basis, u32 cur);

	ILINE u32  GetResizeCount() const { return m_pParent->GetResizeCount(); }

	EHistoryCount GetHistoryCount() const;

	struct ValueIter : public CRegularlySyncedItems_StaticDefs
	{
	public:
		friend class CRegularlySyncedItem;

		ValueIter() : m_cur(LINKEDELEM_TERMINATOR), m_pElems(0) {}

		SElem* Current();
		void   Next();

	private:
		ValueIter(u32 idx, SLinkedElem* pElems) : m_cur(idx), m_pElems(pElems) {}
		u32       m_cur;
		SLinkedElem* m_pElems;
	};
	struct ConstValueIter : public CRegularlySyncedItems_StaticDefs
	{
	public:
		friend class CRegularlySyncedItem;

		ConstValueIter() : m_cur(LINKEDELEM_TERMINATOR), m_pElems(0) {}

		const SElem* Current();
		void         Next();

	private:
		ConstValueIter(u32 idx, SLinkedElem* pElems) : m_cur(idx), m_pElems(pElems) {}
		u32             m_cur;
		const SLinkedElem* m_pElems;
	};

	ValueIter      GetIter();
	ConstValueIter GetConstIter() const;

private:
	void          RemoveFrom(u32 start, u32 assertIfSeqGTE = ~0);
	void          CompleteAck(u32& prevnextidx, bool ack, u32 current);
	EHistoryCount GetHistoryCount_Slow() const;
	void          CheckValid()
	{
		//NET_ASSERT(m_pRoot->historyCount == eHC_Invalid || m_pRoot->historyCount == GetHistoryCount_Slow());
		m_pParent->Verify();
	}

	typedef CRegularlySyncedItems::SLinkedElem SLinkedElem;
	ILINE CRegularlySyncedItem(CRegularlySyncedItems* pParent, SHistoryRootPointer* pRoot, u32 id) : m_pParent(pParent), m_pRoot(pRoot), m_id(id)
	{
	}

	CRegularlySyncedItems* m_pParent;
	SHistoryRootPointer*   m_pRoot;
	u32                 m_id;
};

ILINE CRegularlySyncedItem CRegularlySyncedItems::GetItem(u32 id, SHistoryRootPointer* historyElems)
{
	return CRegularlySyncedItem(this, historyElems + m_idBase + m_idMul * id, id);
}

#endif
