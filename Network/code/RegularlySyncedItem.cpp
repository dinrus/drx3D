// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/RegularlySyncedItem.h>
#include  <drx3D/Network/NetCVars.h>
#include  <drx3D/Network/SyncContext.h>
#include  <drx3D/Network/STLMementoAllocator.h>

static ILINE void AdvanceHistoryCount(SHistoryRootPointer* hrp)
{
	hrp->historyCount += hrp->historyCount < eHC_MoreThanOne;
}

void CRegularlySyncedItems::GetMemoryStatistics(IDrxSizer* pSizer)
{
	SIZER_COMPONENT_NAME(pSizer, "CRegularlySyncedItems");

	//	pSizer->Add(*this);
	pSizer->AddContainer(m_elems);
}

void CRegularlySyncedItems::Reset()
{
	for (u32 i = 0; i < m_elems.size(); i++)
	{
		if (m_elems[i].valid)
		{
			Destroy(m_elems[i].data);
		}
	}
	std::vector<SLinkedElem>().swap(m_elems);
	std::vector<u32>().swap(m_freeElems);
}

void CRegularlySyncedItem::AddElem(u32 seq, u32 data)
{
	CheckValid();
	SLinkedElem el;
	el.data = data;
	el.seq = seq;
	el.next = m_pRoot->index;
	el.valid = 1;
#if DETAIL_REGULARLY_SYNCED_ITEM_DEBUG
	el.nextSetter = 1;
#endif
	std::vector<SLinkedElem>& elems = m_pParent->m_elems;
	NET_ASSERT(el.next == LINKEDELEM_TERMINATOR || elems[el.next].seq < seq);
#if NET_ASSERT_LOGGING
	if (el.next != LINKEDELEM_TERMINATOR && elems[el.next].seq >= seq)
	{
		NetLog("el.next     = %.8x", el.next);
		if (el.next != LINKEDELEM_TERMINATOR)
		{
			NetLog("seq         = %.8x", seq);
			NetLog("el.next seq = %.8x", elems[el.next].seq);
		}
	}
#endif
#if DETAIL_REGULARLY_SYNCED_ITEM_DEBUG
	if (el.next != LINKEDELEM_TERMINATOR)
		for (size_t i = 0; i < elems.size(); i++)
			NET_ASSERT(elems[i].next != el.next);
#endif

	std::vector<u32>& freeElems = m_pParent->m_freeElems;
	if (freeElems.empty())
	{
		m_pParent->m_resizes++;
		elems.push_back(el);
		m_pRoot->index = elems.size() - 1;
	}
	else
	{
		NET_ASSERT(elems[freeElems.back()].valid == 0);
		elems[m_pRoot->index = freeElems.back()] = el;
		freeElems.pop_back();
	}
	AdvanceHistoryCount(m_pRoot);
	CheckValid();
}

CRegularlySyncedItems::SElem* CRegularlySyncedItem::FindPrevElem(u32 basisSeq, u32 curSeq) const
{
	m_pParent->GotCurSeq(curSeq);

	std::vector<SLinkedElem>& elems = m_pParent->m_elems;
	u32 current = m_pRoot->index;
	while (current != LINKEDELEM_TERMINATOR)
	{
		SLinkedElem& el = elems[current];
		if (el.seq <= basisSeq)
			return &el;
		current = el.next;
	}
	return NULL;
}

void CRegularlySyncedItem::RemoveOlderThan(u32 seq)
{
	std::vector<SLinkedElem>& elems = m_pParent->m_elems;
	u32 current = m_pRoot->index;
	while (current != LINKEDELEM_TERMINATOR)
	{
		SLinkedElem& el = elems[current];
		if (el.seq < seq)
		{
			RemoveFrom(current, seq);
			break;
		}
		current = el.next;
	}
}

void CRegularlySyncedItem::RemoveFrom(u32 start, u32 assertIfSeqGTE)
{
	CheckValid();

	if (start == LINKEDELEM_TERMINATOR)
		return;

	// remove everything from tail
	std::vector<SLinkedElem>& elems = m_pParent->m_elems;
	u32 current = start;
	do
	{
		NET_ASSERT(elems[current].valid);
		u32 remove = current;
		NET_ASSERT(elems[current].seq < assertIfSeqGTE);
		current = elems[current].next;
		m_pParent->Destroy(elems[remove].data);
		elems[remove].next = LINKEDELEM_TERMINATOR;
		elems[remove].valid = 0;
#if DETAIL_REGULARLY_SYNCED_ITEM_DEBUG
		elems[remove].nextSetter = 12;
#endif
		m_pParent->m_freeElems.push_back(remove);
	}
	while (current != LINKEDELEM_TERMINATOR);

	// now walk from head and patch the removal
	if (m_pRoot->index == start)
	{
		m_pRoot->index = LINKEDELEM_TERMINATOR;
		m_pRoot->historyCount = eHC_Zero;
	}
	else
	{
		current = m_pRoot->index;
		m_pRoot->historyCount = eHC_One;
		while (elems[current].next != start)
		{
			current = elems[current].next;
			NET_ASSERT(current != LINKEDELEM_TERMINATOR);
			m_pRoot->historyCount = eHC_MoreThanOne;
		}
		elems[current].next = LINKEDELEM_TERMINATOR;
#if DETAIL_REGULARLY_SYNCED_ITEM_DEBUG
		elems[current].nextSetter = 2;
#endif
	}

	CheckValid();
}

void CRegularlySyncedItem::Flush()
{
	RemoveFrom(m_pRoot->index);
}

void CRegularlySyncedItem::DumpStuff(tukk text, SNetObjectID id, u32 basis, u32 cur)
{
	typedef DrxFixedStringT<1024> STR;

	STR stuff = text;
	if (id)
	{
		stuff += STR().Format("id=%s:%u", id.GetText(), m_id);
	}
	if (basis != ~u32(0))
	{
		stuff += STR().Format(" basis=%u", basis);
	}
	if (cur != ~u32(0))
	{
		stuff += STR().Format(" cur=%u", cur);
	}
	if (m_pRoot->index == LINKEDELEM_TERMINATOR)
	{
		stuff += " iter=term";
	}
	else
	{
		stuff += STR().Format(" iter=%u", m_pParent->m_elems[m_pRoot->index].seq);
	}
	if (m_pRoot->index == LINKEDELEM_TERMINATOR)
	{
		stuff += " empty";
	}
	else
	{
		stuff += " seqs:";
		u32 current = m_pRoot->index;
		while (current != LINKEDELEM_TERMINATOR)
		{
			stuff += STR().Format(" %u", m_pParent->m_elems[current].seq);
			current = m_pParent->m_elems[current].next;
		}
	}
	NetLog("%s", stuff.c_str());
}

void CRegularlySyncedItem::AckUpdate(u32 seq, bool ack)
{
	CheckValid();
	if (m_pRoot->index == LINKEDELEM_TERMINATOR)
	{
		// can happen if a message was rejected
		return;
	}

	u32 current = m_pRoot->index;
	std::vector<SLinkedElem>& elems = m_pParent->m_elems;
	NET_ASSERT(current != LINKEDELEM_TERMINATOR);
	if (elems[current].seq == seq)
	{
		u32 prevnextidx = m_pRoot->index;
		CompleteAck(prevnextidx, ack, current);
		m_pRoot->index = prevnextidx;
	}
	else
		while (elems[current].next != LINKEDELEM_TERMINATOR)
		{
			u32 prev = current;
			current = elems[current].next;

			u32 curseq = elems[current].seq;
			if (curseq == seq)
			{
				u32 n = elems[prev].next;
				CompleteAck(n, ack, current);
				elems[prev].next = n;
				break;
			}
			else if (curseq < seq)
			{
				// can happen if a message was rejected
				break;
			}
		}

	NET_ASSERT(current != LINKEDELEM_TERMINATOR);
	CheckValid();
}

void CRegularlySyncedItem::CompleteAck(u32& prevnextidx, bool ack, u32 current)
{
	CheckValid();

	std::vector<SLinkedElem>& elems = m_pParent->m_elems;
	if (ack)
	{
		u32 next = elems[current].next;
		elems[current].next = LINKEDELEM_TERMINATOR;
#if DETAIL_REGULARLY_SYNCED_ITEM_DEBUG
		elems[current].nextSetter = 3;
#endif
		current = next;
		while (current != LINKEDELEM_TERMINATOR)
		{
			NET_ASSERT(current != LINKEDELEM_TERMINATOR);
			u32 next2 = elems[current].next;
			m_pParent->Destroy(elems[current].data);
			elems[current].next = LINKEDELEM_TERMINATOR;
#if DETAIL_REGULARLY_SYNCED_ITEM_DEBUG
			elems[current].nextSetter = 11;
#endif
			elems[current].valid = 0;
			m_pParent->m_freeElems.push_back(current);
			current = next2;
		}
	}
	else
	{
		prevnextidx = elems[current].next;
		m_pParent->Destroy(elems[current].data);
		elems[current].next = LINKEDELEM_TERMINATOR;
		elems[current].valid = 0;
#if DETAIL_REGULARLY_SYNCED_ITEM_DEBUG
		elems[current].nextSetter = 10;
#endif
		m_pParent->m_freeElems.push_back(current);
	}
	m_pRoot->historyCount = eHC_Invalid; // don't know anymore!

	CheckValid();
}

CRegularlySyncedItem::ValueIter CRegularlySyncedItem::GetIter()
{
	if (m_pRoot->index == LINKEDELEM_TERMINATOR)
		return ValueIter();
	else
		return ValueIter(m_pRoot->index, &m_pParent->m_elems[0]);
}

CRegularlySyncedItem::ConstValueIter CRegularlySyncedItem::GetConstIter() const
{
	if (m_pRoot->index == LINKEDELEM_TERMINATOR)
		return ConstValueIter();
	else
		return ConstValueIter(m_pRoot->index, &m_pParent->m_elems[0]);
}

CRegularlySyncedItem::SElem* CRegularlySyncedItem::ValueIter::Current()
{
	if (m_cur == LINKEDELEM_TERMINATOR)
		return NULL;
	return &m_pElems[m_cur];
}

void CRegularlySyncedItem::ValueIter::Next()
{
	NET_ASSERT(m_cur != LINKEDELEM_TERMINATOR);
	m_cur = m_pElems[m_cur].next;
}

const CRegularlySyncedItem::SElem* CRegularlySyncedItem::ConstValueIter::Current()
{
	if (m_cur == LINKEDELEM_TERMINATOR)
		return NULL;
	return &m_pElems[m_cur];
}

void CRegularlySyncedItem::ConstValueIter::Next()
{
	NET_ASSERT(m_cur != LINKEDELEM_TERMINATOR);
	m_cur = m_pElems[m_cur].next;
}

EHistoryCount CRegularlySyncedItem::GetHistoryCount_Slow() const
{
	if (m_pRoot->index == LINKEDELEM_TERMINATOR)
		return eHC_Zero;
	else if (m_pParent->m_elems[m_pRoot->index].next == LINKEDELEM_TERMINATOR)
		return eHC_One;
	else
		return eHC_MoreThanOne;
}

EHistoryCount CRegularlySyncedItem::GetHistoryCount() const
{
	if (m_pRoot->historyCount != eHC_Invalid)
	{
		NET_ASSERT(m_pRoot->historyCount == GetHistoryCount_Slow());
		return (EHistoryCount) m_pRoot->historyCount;
	}
	else
		return (EHistoryCount) (m_pRoot->historyCount = GetHistoryCount_Slow());
}

void CRegularlySyncedItems::Verify()
{
	/*
	   #if DETAIL_REGULARLY_SYNCED_ITEM_DEBUG
	   std::map<u32, u32, std::less<u32>, STLMementoAllocator<std::pair<u32k, u32> > have;

	   for (size_t i=0; i<m_freeElems.size(); i++)
	   {
	    NET_ASSERT(m_elems[m_freeElems[i]].next == LINKEDELEM_TERMINATOR);
	    have[m_freeElems[i]] = LINKEDELEM_TERMINATOR;
	   }

	   for (size_t i=0; i<m_elems.size(); i++)
	   {
	    if (m_elems[i].next != LINKEDELEM_TERMINATOR)
	      NET_ASSERT(have.insert(std::make_pair(m_elems[i].next,i)).second);
	   }
	   #endif
	   //*/
}
