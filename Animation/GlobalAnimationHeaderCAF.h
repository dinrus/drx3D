// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#ifndef RESOURCE_COMPILER
#include <drx3D/Animation/GlobalAnimationHeader.h>
#include <drx3D/Animation/Controller.h>
#include <drx3D/Animation/ControllerPQLog.h>
#include <drx3D/Animation/ControllerTCB.h>
#include <drx3D/Animation/ControllerDefragHeap.h>
#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/CoreX/String/NameCRCHelper.h>
#include <drx3D/Animation/AnimEventList.h>
#include <drx3D/CoreX/Memory/PoolAllocator.h>

struct CInternalSkinningInfoDBA;
class ILoaderCAFListener;
class IControllerRelocatableChain;

#define CAF_MAX_SEGMENTS (5) //> Number of animation segment separators (including lower and upper bound).

struct GlobalAnimationHeaderCAFStreamContent:public IStreamCallback
{
	i32 m_id;
	tukk m_pPath;

	u32 m_nFlags;
	u32 m_FilePathDBACRC32;

	f32 m_fStartSec;
	f32 m_fEndSec;
	f32 m_fTotalDuration;

	QuatT m_StartLocation;

	DynArray<IController_AutoPtr> m_arrController;
	CControllerDefragHdl m_hControllerData;

	GlobalAnimationHeaderCAFStreamContent();
	~GlobalAnimationHeaderCAFStreamContent();

	uk StreamOnNeedStorage(IReadStream* pStream, unsigned nSize, bool& bAbortOnFailToAlloc);
	void  StreamAsyncOnComplete (IReadStream* pStream, unsigned nError);
	void  StreamOnComplete (IReadStream* pStream, unsigned nError);

	u32 ParseChunkHeaders(IChunkFile* pChunkFile, bool& bLoadOldChunksOut, bool& bLoadInPlaceOut, size_t& nUsefulSizeOut);
	bool   ParseChunkRange(IChunkFile* pChunkFile, u32 min, u32 max, bool bLoadOldChunks, tuk& pStorage, IControllerRelocatableChain*& pRelocateHead);
	bool   ReadMotionParameters (IChunkFile::ChunkDesc* pChunkDesc);
	bool   ReadController(IChunkFile::ChunkDesc* pChunkDesc, bool bLoadUncompressedChunks, tuk& pStorage, IControllerRelocatableChain*& pRelocateHead);
	bool   ReadTiming(IChunkFile::ChunkDesc* pChunkDesc);
	u32 FlagsSanityFilter(u32 flags);

private:
	// Prevent copy construction
	GlobalAnimationHeaderCAFStreamContent(const GlobalAnimationHeaderCAFStreamContent&);
	GlobalAnimationHeaderCAFStreamContent& operator = (const GlobalAnimationHeaderCAFStreamContent&);
};

//////////////////////////////////////////////////////////////////////////
// this is the animation information on the module level (not on the per-model level)
// it doesn't know the name of the animation (which is model-specific), but does know the file name
// Implements some services for bone binding and ref counting
struct DRX_ALIGN (16) GlobalAnimationHeaderCAF:public GlobalAnimationHeader
{
	friend class CAnimationUpr;
	friend class CAnimationSet;
	friend struct GlobalAnimationHeaderCAFStreamContent;

	~GlobalAnimationHeaderCAF()
	{
		ClearControllers();
	};

	//! Check total duration of a clip segment.
	//! \param segmentIndex Index of the segment to check. Shall be lower than the value returned by GetTotalSegments().
	//! \return Segment duration, expressed in seconds.
	ILINE f32 GetSegmentDuration(u32 segmentIndex) const
	{
		assert(segmentIndex < m_Segments);
		assert(segmentIndex + 1 < DRX_ARRAY_COUNT(m_SegmentsTime));

		const f32 t0 = m_SegmentsTime[segmentIndex + 0];
		const f32 t1 = m_SegmentsTime[segmentIndex + 1];
		const f32 t  = t1 - t0;
		return GetTotalDuration() * t;
	}

	//! Convert a timepoint from normalized segment time to normalized clip time.
	//! \param segmentIndex Index of the segment. Shall be lower than the value returned by GetTotalSegments().
	//! \param normalizedSegmentTime Timepoint within the specified segment, normalized against the segment duration. Shall belong to the [0, 1] range.
	//! \return Input timepoint, converted to a single value normalized against the entire clip duration.
	ILINE f32 GetNTimeforEntireClip(u32 segmentIndex, f32 normalizedSegmentTime) const
	{
		assert(segmentIndex < m_Segments);
		assert((0.f <= normalizedSegmentTime) && (normalizedSegmentTime <= 1.f));

		const f32 totalDuration             = GetTotalDuration();
		const f32 segmentDuration           = GetSegmentDuration(segmentIndex);
		const f32 normalizedSegmentOffset   = m_SegmentsTime[segmentIndex];
		const f32 normalizedSegmentDuration = segmentDuration / totalDuration;
		return (normalizedSegmentTime * normalizedSegmentDuration) + normalizedSegmentOffset;
	}

	//! Check total duration of this animation clip.
	//! \return Total duration, expressed in seconds.
	ILINE f32 GetTotalDuration() const
	{
		return m_fTotalDuration ? m_fTotalDuration : (1.0f / ANIMATION_30Hz);
	}

	//! Check total number of segments within this animation clip.
	//! \return Number of segments.
	ILINE u32 GetTotalSegments() const
	{
		return m_Segments;
	}

	f32 NTime2KTime(f32 ntime) const
	{
		ntime = min(ntime, 1.0f);
		assert(ntime >= 0 && ntime <= 1);
		f32 duration = m_fEndSec - m_fStartSec;
		f32 start    = m_fStartSec;
		f32 key      = (ntime * ANIMATION_30Hz * duration + start * ANIMATION_30Hz); ///40.0f;
		return key;
	}

	void AddRef()
	{
		++m_nRef_by_Model;
	}

	void Release()
	{
		if (!--m_nRef_by_Model)
			ClearControllers();
	}

#ifdef _DEBUG
	// returns the maximum reference counter from all controllers. 1 means that nobody but this animation structure refers to them
	i32 MaxControllerRefCount() const
	{
		if (m_arrController.size() == 0)
			return 0;

		i32 nMax = m_arrController[0]->UseCount();
		for (i32 i = 0; i < m_nControllers; ++i)
			if (m_arrController[i]->UseCount() > nMax)
				nMax = m_arrController[i]->UseCount();
		return nMax;
	}
#endif

	//count how many position controllers this animation has
	u32 GetTotalPosKeys() const;

	//count how many rotation controllers this animation has
	u32 GetTotalRotKeys() const;

	size_t SizeOfCAF(const bool bForceControllerCalcu = false) const;
	void   LoadControllersCAF();
	void   LoadDBA();

	void GetMemoryUsage(IDrxSizer* pSizer) const;

	u32 IsAssetLoaded() const
	{
		if (m_nControllers2)
			return m_nControllers;
		return m_FilePathDBACRC32 != -1;   //return 0 in case of an ANM-file
	}

	ILINE u32 IsAssetOnDemand() const
	{
		if (m_nFlags & CA_ASSET_TCB)
			return 0;
		return m_FilePathDBACRC32 == 0;
	}

#ifdef EDITOR_PCDEBUGCODE
	bool        Export2HTR(tukk szAnimationName, tukk saveDirectory, const CDefaultSkeleton* pDefaultSkeleton);
	static bool SaveHTR(tukk szAnimationName, tukk saveDirectory, const std::vector<string>& jointNameArray, const std::vector<string>& jointParentArray, const std::vector< DynArray<QuatT> >& arrAnimation, const QuatT* parrDefJoints);
	static bool SaveICAF(tukk szAnimationName, tukk saveDirectory, const std::vector<string>& arrJointNames, const std::vector< DynArray<QuatT> >& arrAnimation);
#endif

	//---------------------------------------------------------------
	IController* GetControllerByJointCRC32(u32 nControllerID) const
	{
		u32 IsCreated = IsAssetCreated();
		if (IsCreated == 0)
			return NULL;                   //doesn't exist at all. there is nothing we can do
		if (IsAssetLoaded() == 0)
			return NULL;

		i32 nSize = m_arrControllerLookupVector.size();

		assert(nSize <= m_arrController.size());
		assert(m_arrController.size() == nSize);

		IF (m_arrController.size() != nSize, false)
			return NULL;

		IF (nSize == 0, false)   // don't try to search in empty arrays
			return NULL;

		u32k* arrControllerLookup = &m_arrControllerLookupVector[0];
		i32 low  = 0;
		i32 high = nSize - 1;
		while (low <= high)
		{
			i32 mid = (low + high) / 2;
			if (arrControllerLookup[mid] > nControllerID)
				high = mid - 1;
			else if (arrControllerLookup[mid] < nControllerID)
				low = mid + 1;
			else
			{
				IController* pController = m_arrController[mid].get();
				DrxPrefetch(pController);
				return pController;
			}
		}
		return NULL;                     // not found
	}

	size_t GetControllersCount() const
	{
		return m_nControllers;
	}

	void   ClearControllers();
	void   ClearControllerData();
	u32 LoadCAF( );
	u32 DoesExistCAF();
	void   StartStreamingCAF();
	void   ControllerInit();

	void ConnectCAFandDBA();

	void InitControllerLookup(u32 numControllers)
	{
		if (numControllers > 512)
		{
			DrxLogAlways("ERROR, controller array size too big(size = %u", numControllers);
		}
		m_arrControllerLookupVector.resize(numControllers);
		for (u32 i = 0; i < numControllers; ++i)
		{
			m_arrControllerLookupVector[i] = m_arrController[i]->GetID();
		}
	}

	GlobalAnimationHeaderCAF ()
	{
		m_FilePathDBACRC32 = -1;
		m_nRef_by_Model    = 0;
		m_nRef_at_Runtime  = 0;
		m_nTouchedCounter  = 0;
		m_bEmpty           = 0;

		m_fStartSec      = -1;                                   // Start time in seconds.
		m_fEndSec        = -1;                                   // End time in seconds.
		m_fTotalDuration = -1.0f;                                  //asset-features
		m_StartLocation.SetIdentity();                           //asset-features

		m_Segments        = 1;                                   //asset-features
		m_SegmentsTime[0] = 0.0f;                                  //asset-features
		m_SegmentsTime[1] = 1.0f;                                  //asset-features
		m_SegmentsTime[2] = 1.0f;                                  //asset-features
		m_SegmentsTime[3] = 1.0f;                                  //asset-features
		m_SegmentsTime[4] = 1.0f;                                  //asset-features

		m_nControllers  = 0;
		m_nControllers2 = 0;
	}

public:
	u32 m_FilePathDBACRC32;                                 //hash value (if the file is comming from a DBA)
	IReadStreamPtr m_pStream;

	f32 m_fStartSec;                                           //asset-feature: Start time in seconds.
	f32 m_fEndSec;                                             //asset-feature: End time in seconds.
	f32 m_fTotalDuration;                                      //asset-feature: asset-feature: total duration in seconds.

	u16 m_nControllers;
	u16 m_nControllers2;

	DynArray<IController_AutoPtr> m_arrController;
	DynArray<u32>              m_arrControllerLookupVector; // used to speed up controller lookup (binary search on the CRC32 only, load controller from a associative array)
	CControllerDefragHdl          m_hControllerData;

	CAnimEventList m_AnimEventsCAF;

	u16 m_nRef_by_Model;                                    //counter how many models are referencing this animation
	u16 m_nRef_at_Runtime;                                  //counter how many times we use this animation at run-time at the same time (needed for streaming)
	u16 m_nTouchedCounter;                                  //for statistics: did we use this asset at all?
	u8  m_bEmpty;                                           //Loaded from database
	u8  m_Segments;                                         //asset-feature: amount of segments
	f32 m_SegmentsTime[CAF_MAX_SEGMENTS];                      //asset-feature: normalized-time for each segment

	QuatT m_StartLocation;                                     //asset-feature: the original location of the animation in world-space

private:
	void CommitContent(GlobalAnimationHeaderCAFStreamContent& content);
	void FinishLoading(unsigned nError);

};

// mark GlobalAnimationHeaderCAF as moveable, to prevent problems with missing copy-constructors
// and to not waste performance doing expensive copy-constructor operations
template<>
inline bool raw_movable<GlobalAnimationHeaderCAF>(GlobalAnimationHeaderCAF const& dest)
{
	return true;
}
#endif