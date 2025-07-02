// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "SequencerUtils.h"
#include "SequencerSequence.h"
#include "SequencerNode.h"

//////////////////////////////////////////////////////////////////////////
static void AddBeginKeyTime(i32k nkey,
                            CSequencerNode* pAnimNode, CSequencerTrack* pAnimTrack,
                            CSequencerUtils::SelectedKeys& selectedKeys,
                            const bool selectedOnly, const bool noTimeRange,
                            const float t0 = 0.0f, const float t1 = 0.0f)
{
	const float keyTime = pAnimTrack->GetKeyTime(nkey);
	const bool timeRangeOk = noTimeRange || (t0 <= keyTime && keyTime <= t1);
	if ((!selectedOnly || pAnimTrack->IsKeySelected(nkey)) && timeRangeOk)
	{
		selectedKeys.keys.push_back(CSequencerUtils::SelectedKey(pAnimNode, pAnimTrack, nkey, keyTime, CSequencerUtils::SelectedKey::eKeyBegin));
	}
}

static void AddSecondarySelPtTimes(i32k nkey,
                                   CSequencerNode* pAnimNode, CSequencerTrack* pAnimTrack,
                                   CSequencerUtils::SelectedKeys& selectedKeys,
                                   const bool selectedOnly, const bool noTimeRange,
                                   const float t0 = 0.0f, const float t1 = 0.0f)
{
	i32k num2ndPts = pAnimTrack->GetNumSecondarySelPts(nkey);
	for (i32 n2ndPt = 1; n2ndPt <= num2ndPts; n2ndPt++)
	{
		const float f2ndPtTime = pAnimTrack->GetSecondaryTime(nkey, n2ndPt);
		const bool timeRangeOk = noTimeRange || (t0 <= (f2ndPtTime) && (f2ndPtTime) <= t1);
		if ((!selectedOnly || pAnimTrack->IsKeySelected(nkey)) && timeRangeOk)
		{
			selectedKeys.keys.push_back(CSequencerUtils::SelectedKey(pAnimNode, pAnimTrack, nkey, f2ndPtTime, CSequencerUtils::SelectedKey::eKeyBlend));
		}
	}
}

static void AddEndKeyTime(i32k nkey,
                          CSequencerNode* pAnimNode, CSequencerTrack* pAnimTrack,
                          CSequencerUtils::SelectedKeys& selectedKeys,
                          const bool selectedOnly, const bool noTimeRange,
                          const float t0 = 0.0f, const float t1 = 0.0f)
{
	const float keyTime = pAnimTrack->GetKeyTime(nkey);
	const float duration = pAnimTrack->GetKeyDuration(nkey);
	const bool timeRangeOk = noTimeRange || (t0 <= (keyTime + duration) && (keyTime + duration) <= t1);
	if ((!selectedOnly || pAnimTrack->IsKeySelected(nkey)) && timeRangeOk)
	{
		selectedKeys.keys.push_back(CSequencerUtils::SelectedKey(pAnimNode, pAnimTrack, nkey, keyTime + duration, CSequencerUtils::SelectedKey::eKeyEnd));
	}
}

//////////////////////////////////////////////////////////////////////////
static i32 GetKeys(const CSequencerSequence* pSequence, CSequencerUtils::SelectedKeys& selectedKeys, const bool selectedOnly, const float t0 = 0, const float t1 = 0)
{
	if (pSequence == NULL)
	{
		assert("(pSequence == NULL) in CSequencerUtils::GetSelectedKeys()" && false);
		return 0;
	}

	i32 trackType = -1;
	const bool noTimeRange = !(t0 < t1);

	i32 nNodeCount = pSequence->GetNodeCount();
	for (i32 node = 0; node < nNodeCount; node++)
	{
		CSequencerNode* pAnimNode = pSequence->GetNode(node);

		i32 nTrackCount = pAnimNode->GetTrackCount();
		for (i32 track = 0; track < nTrackCount; track++)
		{
			CSequencerTrack* pAnimTrack = pAnimNode->GetTrackByIndex(track);

			i32 nKeyCount = pAnimTrack->GetNumKeys();
			for (i32 nkey = 0; nkey < nKeyCount; nkey++)
			{
				AddBeginKeyTime(nkey, pAnimNode, pAnimTrack, selectedKeys, selectedOnly, noTimeRange, t0, t1);
			}
		}
	}

	return (i32)selectedKeys.keys.size();
}

//////////////////////////////////////////////////////////////////////////
static i32 GetKeysByAnyTimeMarker(const CSequencerSequence* pSequence, CSequencerUtils::SelectedKeys& selectedKeys, const bool selectedOnly, const float t0 = 0.0f, const float t1 = 0.0f)
{
	if (pSequence == NULL)
	{
		assert("(pSequence == NULL) in CSequencerUtils::GetSelectedKeys()" && false);
		return 0;
	}

	i32 trackType = -1;
	const bool noTimeRange = !(t0 < t1);

	i32k nNodeCount = pSequence->GetNodeCount();
	for (i32 node = 0; node < nNodeCount; node++)
	{
		CSequencerNode* pAnimNode = pSequence->GetNode(node);

		i32k nTrackCount = pAnimNode->GetTrackCount();
		for (i32 track = 0; track < nTrackCount; track++)
		{
			CSequencerTrack* pAnimTrack = pAnimNode->GetTrackByIndex(track);

			i32k nKeyCount = pAnimTrack->GetNumKeys();
			for (i32 nkey = 0; nkey < nKeyCount; nkey++)
			{
				AddBeginKeyTime(nkey, pAnimNode, pAnimTrack, selectedKeys, selectedOnly, noTimeRange, t0, t1);
				AddSecondarySelPtTimes(nkey, pAnimNode, pAnimTrack, selectedKeys, selectedOnly, noTimeRange, t0, t1);
				AddEndKeyTime(nkey, pAnimNode, pAnimTrack, selectedKeys, selectedOnly, noTimeRange, t0, t1);
			}
		}
	}

	return (i32)selectedKeys.keys.size();
}

//////////////////////////////////////////////////////////////////////////
i32 CSequencerUtils::GetSelectedKeys(CSequencerSequence* pSequence, SelectedKeys& selectedKeys)
{
	return GetKeys(pSequence, selectedKeys, true);
}

//////////////////////////////////////////////////////////////////////////
i32 CSequencerUtils::GetAllKeys(CSequencerSequence* pSequence, SelectedKeys& selectedKeys)
{
	return GetKeys(pSequence, selectedKeys, false);
}

//////////////////////////////////////////////////////////////////////////
i32 CSequencerUtils::GetSelectedTracks(CSequencerSequence* pSequence, SelectedTracks& selectedTracks)
{
	i32 trackType = -1;

	i32 nNodeCount = pSequence ? pSequence->GetNodeCount() : 0;
	for (i32 node = 0; node < nNodeCount; node++)
	{
		CSequencerNode* pAnimNode = pSequence->GetNode(node);

		i32 nTrackCount = pAnimNode->GetTrackCount();
		for (i32 track = 0; track < nTrackCount; track++)
		{
			CSequencerTrack* pAnimTrack = pAnimNode->GetTrackByIndex(track);

			if (pAnimTrack->GetFlags() & CSequencerTrack::SEQUENCER_TRACK_SELECTED)
			{
				SelectedTrack t;
				t.pNode = pAnimNode;
				t.pTrack = pAnimTrack;
				t.m_nSubTrackIndex = -1;
				selectedTracks.tracks.push_back(t);
			}
		}
	}

	return (i32)selectedTracks.tracks.size();
}

//////////////////////////////////////////////////////////////////////////
bool CSequencerUtils::IsOneTrackSelected(const SelectedTracks& selectedTracks)
{
	if (selectedTracks.tracks.size() == 0)
		return false;

	if (selectedTracks.tracks.size() == 1)
		return true;

	i32 baseIndex = -1;
	for (i32 i = 0; i < (i32)selectedTracks.tracks.size(); ++i)
	{
		// It's not a subtrack.
		if (selectedTracks.tracks[i].m_nSubTrackIndex < 0)
			return false;
		if (i == 0)
			baseIndex = selectedTracks.tracks[i].pTrack->GetParameterType()
			            - selectedTracks.tracks[i].m_nSubTrackIndex;
		else
		{
			// It's not a subtrack belong to a same parent track.
			if (baseIndex !=
			    selectedTracks.tracks[i].pTrack->GetParameterType()
			    - selectedTracks.tracks[i].m_nSubTrackIndex)
				return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
i32 CSequencerUtils::GetKeysInTimeRange(const CSequencerSequence* pSequence, SelectedKeys& selectedKeys, const float t0, const float t1)
{
	return GetKeysByAnyTimeMarker(pSequence, selectedKeys, false, t0, t1);
}

//////////////////////////////////////////////////////////////////////////
bool CSequencerUtils::CanAnyKeyBeMoved(const SelectedKeys& selectedKeys)
{
	bool canAnyKeyBeMoved = false;
	for (i32 k = 0; k < (i32)selectedKeys.keys.size(); ++k)
	{
		const CSequencerUtils::SelectedKey& skey = selectedKeys.keys[k];
		canAnyKeyBeMoved |= skey.pTrack->CanMoveKey(skey.nKey);
	}
	return canAnyKeyBeMoved;
}

