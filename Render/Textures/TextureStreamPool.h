// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef TEXTURESTREAMPOOL_H
#define TEXTURESTREAMPOOL_H

#define TEXSTRM_USE_FREEPOOL

struct STexPool;

struct STexPoolItemHdr
{
	static DrxCriticalSection s_sSyncLock;

	STexPoolItemHdr*          m_Next;
	STexPoolItemHdr*          m_Prev;
	STexPoolItemHdr*          m_NextFree;
	STexPoolItemHdr*          m_PrevFree;

	STexPoolItemHdr()
		: m_Next(NULL)
		, m_Prev(NULL)
		, m_NextFree(NULL)
		, m_PrevFree(NULL)
	{
	}

	inline void Unlink();
	inline void Link(STexPoolItemHdr* Before);
	inline void UnlinkFree();
	inline void LinkFree(STexPoolItemHdr* Before);
};

struct STexPoolItem : STexPoolItemHdr
{
	STexPool* const       m_pOwner;
	CTexture*             m_pTex;
	CDeviceTexture* const m_pDevTexture;
	size_t const          m_nDevTextureSize;

	u32                m_nFreeTick;
	u8                 m_nActiveLod;

	STexPoolItem(STexPool* pOwner, CDeviceTexture* pDevTexture, size_t devTextureSize);
	~STexPoolItem();

	bool IsFree() const { return m_NextFree != NULL; }

	i32  GetSize()
	{
		i32 nSize = sizeof(*this);
		return nSize;
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pDevTexture);
	}
	bool IsStillUsedByGPU(u32 nCurTick);
};

struct STexPool
{
	u16          m_Width;
	u16          m_Height;
	u16          m_nArraySize;
	D3DFormat       m_eFormat;
	size_t          m_nDevTextureSize;
	STexPoolItemHdr m_ItemsList;
	ETEX_Type       m_eTT;
	u8           m_nMips;

	i32             m_nItems;
	i32             m_nItemsFree;

	~STexPool();

	size_t GetSize()
	{
		size_t nSize = sizeof(*this);
		STexPoolItemHdr* pIT = m_ItemsList.m_Next;
		while (pIT != &m_ItemsList)
		{
			nSize += static_cast<STexPoolItem*>(pIT)->GetSize();
			pIT = pIT->m_Next;
		}

		return nSize;
	}

	u32 GetNumSlices() const
	{
		assert((m_eTT != eTT_Cube && m_eTT != eTT_CubeArray) || !(m_nArraySize % 6));
		return m_nArraySize;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		STexPoolItemHdr* pIT = m_ItemsList.m_Next;
		while (pIT != &m_ItemsList)
		{
			pSizer->AddObject(static_cast<STexPoolItem*>(pIT));
			pIT = pIT->m_Next;
		}
	}
};

class CTextureStreamPoolMgr
{
public:
	struct SFrameStats
	{
		SFrameStats()
		{
			memset(this, 0, sizeof(*this));
		}

		i32 nSoftCreates;
		i32 nSoftFrees;
		i32 nHardCreates;
		i32 nHardFrees;
	};

	struct SPoolStats
	{
		i32       nWidth;
		i32       nHeight;
		i32       nMips;
		u32    nFormat;
		ETEX_Type eTT;

		i32       nInUse;
		i32       nFree;

		i32       nHardCreatesPerFrame;
		i32       nSoftCreatesPerFrame;
	};

public:
	CTextureStreamPoolMgr();
	~CTextureStreamPoolMgr();

	void Flush();

	// Fetch and reset last frame stats
#if !defined(_RELEASE)
	void        EnableStatsComputation(bool bCompute) { m_bComputeStats = bCompute; }
	SFrameStats FetchFrameStats()
	{
		SFrameStats frameStats = m_frameStats;
		m_frameStats = SFrameStats();
		return frameStats;
	}

	void FetchPoolStats(std::vector<SPoolStats>& poolStats)
	{
		DrxAutoLock<DrxCriticalSection> lock(m_statsLock);
		poolStats = m_poolStats;
		m_poolStats.clear();
	}
#endif

	STexPool*     GetPool(const STextureLayout& pLayout);
	STexPoolItem* GetItem(const STextureLayout& pLayout, bool bShouldBeCreated, tukk sName, const STexturePayload* pPayload = nullptr, bool bCanCreate = true, bool bWaitForIdle = true);

	void          ReleaseItem(STexPoolItem* pItem);
	void          GarbageCollect(size_t* nCurTexPoolSize, size_t nLowerPoolLimit, i32 nMaxItemsToFree);

	void          GetMemoryUsage(IDrxSizer* pSizer);

	size_t        GetInUseSize() const    { return m_nDeviceMemInUse; }
	size_t        GetReservedSize() const { return m_nDeviceMemReserved; }

private:
	union TexturePoolKey
	{
		struct
		{
			uint64 a, b;
		};
		struct
		{
			u16 nWidth;
			u16 nHeight;
			u32 nFormat;
			u8  nTexType;
			u8  nMips;
			u16 nArraySize;
		};

		TexturePoolKey(u16 nWidth, u16 nHeight, u32 nFormat, u8 nTexType, u8 nMips, u16 nArraySize)
		{
			memset(this, 0, sizeof(*this));
			this->nWidth = nWidth;
			this->nHeight = nHeight;
			this->nFormat = nFormat;
			this->nTexType = nTexType;
			this->nMips = nMips;
			this->nArraySize = nArraySize;
		}

		void        GetMemoryUsage(IDrxSizer* pSizer) const {}

		friend bool operator<(const TexturePoolKey& a, const TexturePoolKey& b)
		{
			if (a.a != b.a)
				return a.a < b.a;
			return a.b < b.b;
		}
	};

	enum
	{
		MaxFreePool = 64,
	};

	typedef VectorMap<TexturePoolKey, STexPool*> TexturePoolMap;

private:
	STexPool* GetOrCreatePool(i32 nWidth, i32 nHeight, i32 nMips, i32 nArraySize, D3DFormat eTF, ETEX_Type eTT);
	void      FlushFree();

private:
	 size_t m_nDeviceMemReserved;
	 size_t m_nDeviceMemInUse;

#if !defined(_RELEASE)
	DrxCriticalSection      m_statsLock;
	bool                    m_bComputeStats;
	std::vector<SPoolStats> m_poolStats;
	SFrameStats             m_frameStats;
#endif

	u32          m_nTick;

	TexturePoolMap  m_TexturesPools;
	STexPoolItemHdr m_FreeTexPoolItems;

#ifdef TEXSTRM_USE_FREEPOOL
	uk  m_FreePool[MaxFreePool];
	size_t m_nFreePoolBegin;
	size_t m_nFreePoolEnd;
#endif
};

inline void STexPoolItemHdr::Unlink()
{
	if (!m_Next || !m_Prev)
		return;
	m_Next->m_Prev = m_Prev;
	m_Prev->m_Next = m_Next;
	m_Next = m_Prev = NULL;
}

inline void STexPoolItemHdr::Link(STexPoolItemHdr* pAfter)
{
	if (m_Next || m_Prev)
		return;
	m_Next = pAfter->m_Next;
	pAfter->m_Next->m_Prev = this;
	pAfter->m_Next = this;
	m_Prev = pAfter;
}

inline void STexPoolItemHdr::UnlinkFree()
{
	if (!m_NextFree || !m_PrevFree)
		return;
	m_NextFree->m_PrevFree = m_PrevFree;
	m_PrevFree->m_NextFree = m_NextFree;
	m_NextFree = m_PrevFree = NULL;
}
inline void STexPoolItemHdr::LinkFree(STexPoolItemHdr* pAfter)
{
	if (m_NextFree || m_PrevFree)
		return;
	m_NextFree = pAfter->m_NextFree;
	pAfter->m_NextFree->m_PrevFree = this;
	pAfter->m_NextFree = this;
	m_PrevFree = pAfter;
}

#endif
