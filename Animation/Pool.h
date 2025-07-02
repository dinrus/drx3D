// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IAllocator.h>

namespace Memory {
class CContext;
}

namespace Memory
{

class CPool
{
	friend class CContext;

public:

	virtual uk  Allocate(u32 length)                 { return nullptr; }

	virtual void   Free(uk pAddress)                    {}

	virtual size_t GetGuaranteedAlignment() const          { return 1; }

	virtual void   Update()                                {}

	virtual void   GetMemoryUsage(IDrxSizer* pSizer) const {}

protected:

	CPool(CContext& context);
	virtual ~CPool();

private:

	CContext* m_pContext;
	CPool*    m_pNext;
};

class CPoolFrameLocal : public CPool
{
public:

	static CPoolFrameLocal* Create(CContext& context, u32 bucketLength);

	virtual uk           Allocate(u32 length) override;

	virtual size_t          GetGuaranteedAlignment() const override;

	virtual void            Update() override;

	virtual void            GetMemoryUsage(IDrxSizer* pSizer) const override;

	void                    ReleaseBuckets();
	void                    Reset();

private:

	CPoolFrameLocal(CContext& context, u32 bucketLength);
	~CPoolFrameLocal();

	class CBucket
	{
	public:

		static CBucket* Create(u32 length);
		void            Release() { delete this; }

		void            Reset();

		static size_t   GetGuaranteedAlignment();
		uk           Allocate(u32 length);

		ILINE void      GetMemoryUsage(IDrxSizer* pSizer) const;

	private:

		CBucket(const CBucket&);        // = delete
		void operator=(const CBucket&); // = delete

		CBucket(u32 length);
		~CBucket();

		u8* m_buffer;
		u32 m_bufferSize;
		u32 m_length;
		u32 m_used;
	};

	CBucket* CreateBucket();

	DrxCriticalSection    m_criticalSection;
	u32                m_bucketLength;
	std::vector<CBucket*> m_buckets;
	u32                m_bucketCurrent;
};

ILINE void CPoolFrameLocal::CBucket::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_buffer);
}

} //endns Memory
