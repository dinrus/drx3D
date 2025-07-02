// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "SequencerNode.h"

#include "SequencerSequence.h"
#include "SequencerTrack.h"

CSequencerNode::CSequencerNode(CSequencerSequence* sequence, const SControllerDef& controllerDef)
	: m_sequence(sequence)
	, m_controllerDef(controllerDef)
	, m_startExpanded(true)
	, m_muted(false)
{
}

CSequencerNode::~CSequencerNode()
{
}

void CSequencerNode::SetName(tukk name)
{
	m_name = name;
}

tukk CSequencerNode::GetName()
{
	return m_name.c_str();
}

void CSequencerNode::SetSequence(CSequencerSequence* pSequence)
{
	m_sequence = pSequence;
}

CSequencerSequence* CSequencerNode::GetSequence()
{
	return m_sequence;
}

ESequencerNodeType CSequencerNode::GetType() const
{
	return SEQUENCER_NODE_UNDEFINED;
}

IEntity* CSequencerNode::GetEntity()
{
	return NULL;
}

void CSequencerNode::UpdateKeys()
{
	//--- Force an update on all tracks to ensure that they are sorted
	u32k numTracks = m_tracks.size();
	for (u32 i = 0; i < numTracks; i++)
	{
		CSequencerTrack* pTrack = m_tracks[i].track;
		pTrack->UpdateKeys();
	}
}

i32 CSequencerNode::GetTrackCount() const
{
	return m_tracks.size();
}

void CSequencerNode::UpdateMutedLayerMasks(u32 mutedAnimLayerMask, u32 mutedProcLayerMask)
{
	// base class does nothing
}

CSequencerTrack* CSequencerNode::GetTrackByIndex(i32 nIndex) const
{
	if (m_tracks.size() <= nIndex)
	{
		return NULL;
	}
	return m_tracks[nIndex].track;
}

void CSequencerNode::AddTrack(ESequencerParamType param, CSequencerTrack* track)
{
	TrackDesc td;
	td.paramId = param;
	td.track = track;
	track->SetParameterType(param);
	track->SetTimeRange(GetSequence()->GetTimeRange());
	m_tracks.push_back(td);
}

bool CSequencerNode::RemoveTrack(CSequencerTrack* pTrack)
{
	u32k numTracks = m_tracks.size();
	for (u32 i = 0; i < numTracks; i++)
	{
		if (m_tracks[i].track == pTrack)
		{
			m_tracks.erase(m_tracks.begin() + i);

			return true;
		}
	}
	return false;
}

CSequencerTrack* CSequencerNode::CreateTrack(ESequencerParamType nParamId)
{
	return false;
}

CSequencerTrack* CSequencerNode::GetTrackForParameter(ESequencerParamType nParamId) const
{
	u32k numTracks = m_tracks.size();
	for (u32 i = 0; i < numTracks; i++)
	{
		if (m_tracks[i].paramId == nParamId)
		{
			return m_tracks[i].track;
		}
	}
	return NULL;
}

CSequencerTrack* CSequencerNode::GetTrackForParameter(ESequencerParamType nParamId, u32 index) const
{
	u32k numTracks = m_tracks.size();
	u32 idx = 0;
	for (u32 i = 0; i < numTracks; i++)
	{
		if (m_tracks[i].paramId == nParamId)
		{
			if (idx == index)
			{
				return m_tracks[i].track;
			}
			idx++;
		}
	}
	return NULL;
}

void CSequencerNode::SetTimeRange(Range timeRange)
{
	u32k numTracks = GetTrackCount();
	for (u32 t = 0; t < numTracks; t++)
	{
		CSequencerTrack* pSeqTrack = GetTrackByIndex(t);
		pSeqTrack->SetTimeRange(timeRange);
	}
}

i32 CSequencerNode::GetParamCount() const
{
	return 0;
}

bool CSequencerNode::GetParamInfo(i32 nIndex, SParamInfo& info) const
{
	return false;
}

bool CSequencerNode::GetParamInfoFromId(ESequencerParamType paramId, SParamInfo& info) const
{
	i32k paramCount = GetParamCount();
	for (i32 i = 0; i < paramCount; i++)
	{
		SParamInfo paramInfo;
		GetParamInfo(i, paramInfo);
		if (paramInfo.paramId == paramId)
		{
			info = paramInfo;
			return true;
		}
	}

	return false;
}

bool CSequencerNode::IsParamValid(ESequencerParamType paramId) const
{
	SParamInfo paramInfo;
	const bool isParamValid = GetParamInfoFromId(paramId, paramInfo);
	return isParamValid;
}

bool CSequencerNode::CanAddTrackForParameter(ESequencerParamType paramId) const
{
	SParamInfo paramInfo;
	if (!GetParamInfoFromId(paramId, paramInfo))
		return false;

	i32 flags = 0;
	CSequencerTrack* track = GetTrackForParameter(paramId);
	if (track && !(paramInfo.flags & CSequencerNode::PARAM_MULTIPLE_TRACKS))
	{
		return false;
	}

	return true;
}

void CSequencerNode::InsertMenuOptions(CMenu& menu)
{
}

void CSequencerNode::ClearMenuOptions(CMenu& menu)
{
}

void CSequencerNode::OnMenuOption(i32 menuOption)
{
}

