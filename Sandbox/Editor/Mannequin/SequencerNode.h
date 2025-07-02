// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SEQUENCER_NODE_h__
#define __SEQUENCER_NODE_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "ISequencerSystem.h"
#include "MannequinBase.h"

struct SControllerDef;
class CSequencerSequence;
class CSequencerTrack;

class CSequencerNode
	: virtual public _i_reference_target_t
{
public:
	enum ESupportedParamFlags
	{
		PARAM_MULTIPLE_TRACKS = 0x01, // Set if parameter can be assigned multiple tracks.
	};

	struct SParamInfo
	{
		SParamInfo() : name(""), paramId(SEQUENCER_PARAM_UNDEFINED), flags(0) {};
		SParamInfo(tukk _name, ESequencerParamType _paramId, i32 _flags) : name(_name), paramId(_paramId), flags(_flags) {};

		tukk         name;
		ESequencerParamType paramId;
		i32                 flags;
	};

public:
	CSequencerNode(CSequencerSequence* sequence, const SControllerDef& controllerDef);
	virtual ~CSequencerNode();

	void                       SetName(tukk name);
	tukk                GetName();

	virtual ESequencerNodeType GetType() const;

	void                       SetSequence(CSequencerSequence* pSequence);
	CSequencerSequence*        GetSequence();

	virtual IEntity*           GetEntity();

	void                       UpdateKeys();

	i32                        GetTrackCount() const;
	void                       AddTrack(ESequencerParamType param, CSequencerTrack* track);
	bool                       RemoveTrack(CSequencerTrack* pTrack);
	CSequencerTrack*           GetTrackByIndex(i32 nIndex) const;

	virtual CSequencerTrack*   CreateTrack(ESequencerParamType nParamId);
	CSequencerTrack*           GetTrackForParameter(ESequencerParamType nParamId) const;
	CSequencerTrack*           GetTrackForParameter(ESequencerParamType nParamId, u32 index) const;

	void                       SetTimeRange(Range timeRange);

	bool                       GetStartExpanded() const
	{
		return m_startExpanded;
	}

	virtual void InsertMenuOptions(CMenu& menu);
	virtual void ClearMenuOptions(CMenu& menu);
	virtual void OnMenuOption(i32 menuOption);

	virtual bool CanAddTrackForParameter(ESequencerParamType nParamId) const;

	virtual i32  GetParamCount() const;
	virtual bool GetParamInfo(i32 nIndex, SParamInfo& info) const;

	bool         GetParamInfoFromId(ESequencerParamType paramId, SParamInfo& info) const;
	bool         IsParamValid(ESequencerParamType paramId) const;

	void         Mute(bool bMute) { m_muted = bMute; }
	bool         IsMuted() const  { return m_muted; }

	virtual void UpdateMutedLayerMasks(u32 mutedAnimLayerMask, u32 mutedProcLayerMask);

protected:
	string                m_name;
	CSequencerSequence*   m_sequence;
	const SControllerDef& m_controllerDef;

	bool                  m_muted;

	bool                  m_startExpanded;

	// Tracks.
	struct TrackDesc
	{
		ESequencerParamType         paramId; // Track parameter id.
		_smart_ptr<CSequencerTrack> track;
	};

	std::vector<TrackDesc> m_tracks;
};

#endif // __SEQUENCER_NODE_h__

