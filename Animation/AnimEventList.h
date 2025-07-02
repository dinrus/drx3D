// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

 class  CAnimEventList
	: public IAnimEventList
{
public:
	CAnimEventList();
	virtual ~CAnimEventList();

	virtual u32                GetCount() const;
	virtual const CAnimEventData& GetByIndex(u32 animEventIndex) const;
	virtual CAnimEventData&       GetByIndex(u32 animEventIndex);
	virtual void                  Append(const CAnimEventData& animEvent);
	virtual void                  Remove(u32 animEventIndex);
	virtual void                  Clear();

	size_t                        GetAllocSize() const;
	void                          GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	DynArray<CAnimEventData> m_animEvents;
};
