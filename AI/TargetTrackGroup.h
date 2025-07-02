// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Contains an agent's target tracks and handles updating them

   -------------------------------------------------------------------------
   История:
   - 02:01:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __TARGET_TRACK_GROUP_H__
#define __TARGET_TRACK_GROUP_H__

#include <drx3D/AI/TargetTrackCommon.h>

class CTargetTrack;
struct IDebugHistoryUpr;

// Needed for CreateDummyPotentialTarget
#include <drx3D/AI/IPerceptionHandler.h>

class CTargetTrackGroup
{
public:
	CTargetTrackGroup(TargetTrackHelpers::ITargetTrackPoolProxy* pTrackPoolProxy, tAIObjectID aiObjectId, u32 uConfigHash, i32 nTargetLimit);
	~CTargetTrackGroup();

	tAIObjectID GetAIObjectID() const       { return m_aiObjectId; }
	tAIObjectID GetLastBestTargetID() const { return m_aiLastBestTargetId; }
	u32      GetConfigHash() const       { return m_uConfigHash; }
	i32         GetTargetLimit() const      { return m_nTargetLimit; }
	bool        IsEnabled() const           { return m_bEnabled; }
	void        SetEnabled(bool bEnabled)   { m_bEnabled = bEnabled; }

	void        Reset();
	void        Serialize_Write(TSerialize ser);
	void        Serialize_Read(TSerialize ser);

	void        Update(TargetTrackHelpers::ITargetTrackConfigProxy* pConfigProxy);

	bool        HandleStimulusEvent(const TargetTrackHelpers::STargetTrackStimulusEvent& stimulusEvent, u32 uStimulusNameHash);
	bool        TriggerPulse(tAIObjectID targetID, u32 uStimulusNameHash, u32 uPulseNameHash);
	bool        TriggerPulse(u32 uStimulusNameHash, u32 uPulseNameHash);
	bool        GetDesiredTarget(TargetTrackHelpers::EDesiredTargetMethod eMethod, CWeakRef<CAIObject>& outTarget, SAIPotentialTarget*& pOutTargetInfo);
	u32      GetBestTrack(TargetTrackHelpers::EDesiredTargetMethod eMethod, CTargetTrack** tracks, u32 maxCount);

	bool        IsPotentialTarget(tAIObjectID aiTargetId) const;
	bool        IsDesiredTarget(tAIObjectID aiTargetId) const;

#ifdef TARGET_TRACK_DEBUG
	// Debugging
	void DebugDrawTracks(TargetTrackHelpers::ITargetTrackConfigProxy* pConfigProxy, bool bLastDraw);
	void DebugDrawTargets(i32 nMode, i32 nTargetedCount, bool bExtraInfo = false);
#endif //TARGET_TRACK_DEBUG

	typedef VectorMap<tAIObjectID, CTargetTrack*> TTargetTrackContainer;

	TTargetTrackContainer& GetTargetTracks() { return m_TargetTracks; }

private:
	// Copy-construction and assignment not supported
	CTargetTrackGroup(CTargetTrackGroup const&);
	CTargetTrackGroup& operator=(CTargetTrackGroup const&);

	void               UpdateSortedTracks(bool bForced = false);
	bool               TestTrackAgainstFilters(CTargetTrack* pTrack, TargetTrackHelpers::EDesiredTargetMethod eMethod) const;

	// Stimulus handling helpers
	bool HandleStimulusEvent_All(const TargetTrackHelpers::STargetTrackStimulusEvent& stimulusEvent, u32 uStimulusNameHash);
	bool HandleStimulusEvent_Target(const TargetTrackHelpers::STargetTrackStimulusEvent& stimulusEvent, u32 uStimulusNameHash, CTargetTrack* pTrack);

	// Returns the target track to be used for the given id
	CTargetTrack* GetTargetTrack(tAIObjectID aiTargetId);
	void          DeleteTargetTracks();

	//void UpdateTargetRepresentation(const CTargetTrack *pBestTrack, tAIObjectID &outTargetId, SAIPotentialTarget* &pOutTargetInfo);
	void UpdateTargetRepresentation(const CTargetTrack* pBestTrack, CWeakRef<CAIObject>& outTarget, SAIPotentialTarget*& pOutTargetInfo);

	void InitDummy(); // set common dummy properties after creation.

private:
	TTargetTrackContainer m_TargetTracks;

	typedef std::vector<CTargetTrack*> TSortedTracks;
	TSortedTracks                              m_SortedTracks;

	TargetTrackHelpers::ITargetTrackPoolProxy* m_pTrackPoolProxy;
	tAIObjectID                                m_aiObjectId;
	tAIObjectID                                m_aiLastBestTargetId;
	u32             m_uConfigHash;
	i32                m_nTargetLimit;
	bool               m_bNeedSort;
	bool               m_bEnabled;

	SAIPotentialTarget m_dummyPotentialTarget;

#ifdef TARGET_TRACK_DEBUG
	// Debugging
	bool FindFreeGraphSlot(u32& outIndex) const;

	float                 m_fLastGraphUpdate;
	IDebugHistoryUpr* m_pDebugHistoryUpr;
	enum { DEBUG_GRAPH_OCCUPIED_SIZE = 16 };
	bool                  m_bDebugGraphOccupied[DEBUG_GRAPH_OCCUPIED_SIZE];
#endif //TARGET_TRACK_DEBUG
};

#endif //__TARGET_TRACK_GROUP_H__
