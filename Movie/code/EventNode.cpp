// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Movie/StdAfx.h>
#include <drx3D/Movie/EventNode.h>
#include <drx3D/Movie/AnimTrack.h>
#include <drx3D/Movie/Tracks.h>

#include <drx3D/Sys/ISystem.h>

CAnimEventNode::CAnimEventNode(i32k id) : CAnimNode(id)
{
	SetFlags(GetFlags() | eAnimNodeFlags_CanChangeName);
	m_lastEventKey = -1;
}

void CAnimEventNode::CreateDefaultTracks()
{
	CreateTrack(eAnimParamType_TrackEvent);
}

u32 CAnimEventNode::GetParamCount() const
{
	return 1;
}

CAnimParamType CAnimEventNode::GetParamType(u32 nIndex) const
{
	if (nIndex == 0)
	{
		return eAnimParamType_TrackEvent;
	}

	return eAnimParamType_Invalid;
}

bool CAnimEventNode::GetParamInfoFromType(const CAnimParamType& animParamType, SParamInfo& info) const
{
	if (animParamType.GetType() == eAnimParamType_TrackEvent)
	{
		info.flags = IAnimNode::ESupportedParamFlags(0);
		info.name = "Track Event";
		info.paramType = eAnimParamType_TrackEvent;
		info.valueType = eAnimValue_Unknown;
		return true;
	}

	return false;
}

void CAnimEventNode::Animate(SAnimContext& animContext)
{
	// Get track event
	i32 trackCount = NumTracks();

	for (i32 paramIndex = 0; paramIndex < trackCount; ++paramIndex)
	{
		CAnimParamType trackType = m_tracks[paramIndex]->GetParameterType();
		IAnimTrack* pTrack = m_tracks[paramIndex];

		if (pTrack && pTrack->GetFlags() & IAnimTrack::eAnimTrackFlags_Disabled)
		{
			continue;
		}

		// Check for fire
		if (CTrackEventTrack* pEventTrack = (CTrackEventTrack*)pTrack)
		{
			STrackEventKey key;
			i32 nEventKey = pEventTrack->GetActiveKey(animContext.time, &key);

			if (nEventKey != m_lastEventKey && nEventKey >= 0)
			{
				bool bKeyAfterStartTime = key.m_time >= animContext.startTime;

				if (bKeyAfterStartTime)
				{
					animContext.pSequence->TriggerTrackEvent(key.m_event, key.m_eventValue);
				}
			}

			m_lastEventKey = nEventKey;
		}
	}
}

void CAnimEventNode::OnReset()
{
	m_lastEventKey = -1;
}
