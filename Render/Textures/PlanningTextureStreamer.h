// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef PLANNINGTEXTURESTREAMER_H
#define PLANNINGTEXTURESTREAMER_H

#include <drx3D/Render/ITextureStreamer.h>

//#define TEXSTRM_DEFER_UMR

struct SPlanningMemoryState
{
	ptrdiff_t nMemStreamed;
	ptrdiff_t nStaticTexUsage;
	ptrdiff_t nPhysicalLimit;
	ptrdiff_t nTargetPhysicalLimit;
	ptrdiff_t nMemLimit;
	ptrdiff_t nMemFreeSlack;
	ptrdiff_t nUnknownPoolUsage;
	ptrdiff_t nMemBoundStreamed;
	ptrdiff_t nMemBoundStreamedPers;
	ptrdiff_t nMemTemp;
	ptrdiff_t nMemFreeLower;
	ptrdiff_t nMemFreeUpper;
	ptrdiff_t nStreamLimit;
	ptrdiff_t nStreamMid;
	ptrdiff_t nStreamDelta;
};

struct SPlanningUMRState
{
	i32 arrRoundIds[MAX_PREDICTION_ZONES];
};

struct SPlanningAction
{
	enum
	{
		Unknown,
		Relink,
		Abort,
	};

	SPlanningAction()
		: eAction(Unknown)
	{
	}

	SPlanningAction(i32 eAction, size_t nTexture, u8 nMip = 0)
		: nTexture((u16)nTexture)
		, nMip(nMip)
		, eAction((u8)eAction)
	{
	}

	u16 nTexture;
	u8  nMip;
	u8  eAction;
};

typedef DynArray<std::pair<CTexture*, i32>> TPlanningTextureReqVec;
typedef DynArray<SPlanningAction>           TPlanningActionVec;

struct SPlanningSortState
{
	// In
	TStreamerTextureVec* pTextures;
	size_t               nStreamLimit;
	i32                arrRoundIds[MAX_PREDICTION_ZONES];
	i32                  nFrameId;
	i32                  nBias;
	i32                  fpMinBias;
	i32                  fpMaxBias;
	i32                  fpMinMip;
	SPlanningMemoryState memState;

	// In/Out
	size_t nTextures;

	// Out
	size_t                  nBalancePoint;
	size_t                  nOnScreenPoint;
	size_t                  nPrecachedTexs;
	size_t                  nListSize;
	TPlanningTextureReqVec* pRequestList;
	TStreamerTextureVec*    pTrimmableList;
	TStreamerTextureVec*    pUnlinkList;
	TPlanningActionVec*     pActionList;
};

struct SPlanningScheduleState
{
	i32                    nFrameId;

	i32                    nBias;
	SPlanningMemoryState   memState;

	TPlanningTextureReqVec requestList;
	TStreamerTextureVec    trimmableList;
	TStreamerTextureVec    unlinkList;
	TPlanningActionVec     actionList;
	size_t                 nBalancePoint;
	size_t                 nOnScreenPoint;
};

struct SPlanningUpdateMipRequest
{
	CTexture* pTexture;
	float     fMipFactor;
	i32       nFlags;
	i32       nUpdateId;
};

struct SPlanningTextureOrderKey
{
	// 1 force stream high res
	// 1 high priority
	// 1 is visible
	// 1 is in zone[0]
	// 1 is in zone[1]
	// 16 fp min mip cur, biased
	CTexture* pTexture;
	u32    nKey;

	u16    nWidth;
	u16    nHeight;

	u8     nMips           : 4;
	u8     nMipsPersistent : 4;
	u8     nCurMip;
	u8     nFormatCode;
	u8     eTF;

	u32    nPersistentSize  : 31;
	u32    bIsStreaming     : 1;

	u32    bUnloaded        : 1;
	u32    nStreamPrio      : 3;
	u32    nSlicesMinus1    : 9;
	u32    nSlicesPotMinus1 : 9;

	enum
	{
		InBudgetMask   = 0xffffffff ^ ((1 << 30) | (1 << 29)),
		OverBudgetMask = 0xffffffff,
		PackedFpBias   = 0x7f00
	};

	bool   IsForceStreamHighRes() const { return (nKey & (1 << 31)) == 0; }
	bool   IsHighPriority() const       { return (nKey & (1 << 30)) == 0; }
	bool   IsVisible() const            { return (nKey & (1 << 29)) == 0; }
	bool   IsInZone(i32 z) const        { return (nKey & (1 << (28 - z))) == 0; }
	bool   IsPrecached() const          { return (nKey & ((1 << 31) | (1 << 28) | (1 << 27))) != ((1 << 31) | (1 << 28) | (1 << 27)); }
	i32    GetFpMinMipCur() const       { return static_cast<i16>(static_cast<i32>(nKey & 0xffff) - PackedFpBias); }
	u16 GetFpMinMipCurBiased() const { return (u16)nKey; }

	SPlanningTextureOrderKey() {}
	SPlanningTextureOrderKey(CTexture* pTex, i32 nFrameId, i32k nZoneIds[])
	{
		nKey =
		  (pTex->IsForceStreamHighRes() ? 0 : (1 << 31)) |
		  (pTex->IsStreamHighPriority() ? 0 : (1 << 30)) |
		  (pTex->GetAccessFrameId() >= nFrameId ? 0 : (1 << 29)) |
		  (pTex->GetStreamRoundInfo(0).nRoundUpdateId >= nZoneIds[0] ? 0 : (1 << 28)) |
		  (pTex->GetStreamRoundInfo(1).nRoundUpdateId >= nZoneIds[1] ? 0 : (1 << 27)) |
		  static_cast<u16>(pTex->GetRequiredMipFP() + PackedFpBias);

		pTexture = pTex;

		nWidth = pTex->GetWidth();
		nHeight = pTex->GetHeight();
		nMips = pTex->GetNumMips();
		nMipsPersistent = pTex->IsForceStreamHighRes() ?  pTex->GetNumMips() : pTex->GetNumPersistentMips();
		nFormatCode = pTex->StreamGetFormatCode();

		u32 nSlices = pTex->StreamGetNumSlices();
		nSlicesMinus1 = nSlices - 1;
		nSlicesPotMinus1 = (1u << (32 - (nSlices > 1 ? countLeadingZeros32(nSlices - 1) : 32))) - 1;

		nCurMip = pTex->StreamGetLoadedMip();
		eTF = pTex->GetDstFormat();
		nPersistentSize = pTex->GetPersistentSize();
		bIsStreaming = pTex->IsStreaming();
		bUnloaded = pTex->IsUnloaded();
		nStreamPrio = pTex->StreamGetPriority();
	}
};

struct SPlanningRequestIdent
{
	SPlanningRequestIdent() {}
	SPlanningRequestIdent(u32 nSortKey, i32 nKey, i32 nMip)
		: nSortKey(nSortKey)
		, nKey(nKey)
		, nMip(nMip)
	{
	}

	u32 nSortKey;
	i32    nKey : 27;
	i32    nMip : 5;
};

class CPlanningTextureStreamer : public ITextureStreamer
{
public:
	CPlanningTextureStreamer();

public:
	virtual void  BeginUpdateSchedule();
	virtual void  ApplySchedule(EApplyScheduleFlags asf);

	virtual bool  BeginPrepare(CTexture* pTexture, tukk sFilename, u32 nFlags);
	virtual void  EndPrepare(STexStreamPrepState*& pState);

	virtual void  Precache(CTexture* pTexture);
	virtual void  UpdateMip(CTexture* pTexture, const float fMipFactor, i32k nFlags, i32k nUpdateId, i32k nCounter);

	virtual void  OnTextureDestroy(CTexture* pTexture);

	virtual void  FlagOutOfMemory();
	virtual void  Flush();

	virtual bool  IsOverflowing() const;
	virtual float GetBias() const { return m_nBias / 256.0f; }

public: // Job entry points - do not call directly!
	void Job_UpdateEntry();

private:
	enum State
	{
		S_Idle,
		S_QueuedForUpdate,
		S_Updating,
		S_QueuedForSync,
		S_QueuedForSchedule,
		S_QueuedForScheduleDiscard,
	};

	typedef DynArray<SPlanningUpdateMipRequest> UpdateMipRequestVec;

private:
	SPlanningMemoryState GetMemoryState();

	void                 StartUpdateJob();

	void                 Job_UpdateMip(CTexture* pTexture, const float fMipFactor, i32k nFlags, i32k nUpdateId);
	void                 Job_Sort();
	i32                  Job_Bias(SPlanningSortState& sortState, SPlanningTextureOrderKey* pKeys, size_t nNumPrecachedTexs, size_t nStreamLimit);
	size_t               Job_Plan(SPlanningSortState& sortState, const SPlanningTextureOrderKey* pKeys, size_t nTextures, size_t nNumPrecachedTexs, size_t nBalancePoint, i32 nMinMip, i32 fpSortStateBias);
	void                 Job_InitKeys(SPlanningTextureOrderKey* pKeys, CTexture** pTexs, size_t nTextures, i32 nFrameId, i32k nZoneIds[]);
	void                 Job_CommitKeys(CTexture** pTextures, SPlanningTextureOrderKey* pKeys, size_t nTextures);
	void                 Job_ConfigureSchedule();
	void                 SyncWithJob_Locked();

private:

	bool TryBegin_FromDisk(CTexture* pTex, u32 nTexPersMip, u32 nTexWantedMip, u32 nTexAvailMip, i32 nBias, i32 nBalancePoint,
	                       TStreamerTextureVec& textures, TStreamerTextureVec& trimmable, ptrdiff_t& nMemFreeLower, ptrdiff_t& nMemFreeUpper, i32& nKickIdx,
	                       i32& nNumSubmittedLoad, size_t& nAmtSubmittedLoad);

#if defined(TEXSTRM_TEXTURECENTRIC_MEMORY)
	bool      TrimTexture(i32 nBias, TStreamerTextureVec& trimmable, STexPool* pPrioritise);
#endif
	ptrdiff_t TrimTextures(ptrdiff_t nRequired, i32 nBias, TStreamerTextureVec& trimmable);
	ptrdiff_t KickTextures(CTexture** pTextures, ptrdiff_t nRequired, i32 nBalancePoint, i32& nKickIdx);
	void      Job_CheckEnqueueForStreaming(CTexture* pTexture, const float fMipFactor, bool bHighPriority);

private:
	DrxCriticalSection                    m_lock;
	std::vector<SPlanningTextureOrderKey> m_keys;
	i32                   m_nRTList;
	i32                   m_nJobList;

	 State        m_state;

	JobUpr::SJobState m_JobState;
	SPlanningUMRState     m_umrState;
	SPlanningSortState    m_sortState;

#if defined(TEXSTRM_DEFER_UMR)
	UpdateMipRequestVec m_updateMipRequests[2];
#endif

	SPlanningScheduleState m_schedule;

	i32                    m_nBias;
	i32                    m_nStreamAllocFails;
	bool                   m_bOverBudget;
	size_t                 m_nPrevListSize;
};

struct SPlanningTextureRequestOrder
{
	bool operator()(const SPlanningRequestIdent& a, const SPlanningRequestIdent& b) const
	{
		return a.nSortKey < b.nSortKey;
	}
};

#endif
