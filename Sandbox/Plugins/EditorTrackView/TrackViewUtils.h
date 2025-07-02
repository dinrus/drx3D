// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Header File.
// Copyright (C), DinrusPro 3D, 1999-2015.

#pragma once

#include <DrxMovie/AnimKey.h>
#include "Timeline.h"
#include "CurveEditor.h"
/*
   class CTimeline;
   class CCurveEditor;

   struct SAnimTime;
   struct m_timelineContent;
   struct m_curveEditorContent;
 */

enum ETrackViewSequenceResult
{
	eSequence_OK = 0,
	eSequence_AlreadyExist,
	eSequence_UnknownError,
	eSequence_UnknownSequence,
};

struct SSequenceData
{
	SSequenceData()
		: m_startTime(0.0f)
		, m_endTime(0.0f)
		, m_bPlaying(false)
		, m_pDopeSheet(nullptr)
		, m_pCurveEditor(nullptr)
	{}

	SAnimTime                          m_startTime;
	SAnimTime                          m_endTime;
	bool                               m_bPlaying;

	CTimeline*                         m_pDopeSheet;
	CCurveEditor*                      m_pCurveEditor;

	STimelineContent                   m_timelineContent;
	SCurveEditorContent                m_curveEditorContent;
	std::map<DrxGUID, STimelineTrack*> m_uIdToTimelineTrackMap;
};

