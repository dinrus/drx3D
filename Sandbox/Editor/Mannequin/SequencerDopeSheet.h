// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __sequencerkeylist_h__
#define __sequencerkeylist_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "SequencerDopeSheetBase.h"

/** List of tracks.
 */
class CSequencerDopeSheet : public CSequencerDopeSheetBase
{
	DECLARE_DYNAMIC(CSequencerDopeSheet)
public:
	// public stuff.

	CSequencerDopeSheet();
	~CSequencerDopeSheet();

protected:
	DECLARE_MESSAGE_MAP()

	void DrawTrack(i32 item, CDC* dc, CRect& rcItem);
	void DrawKeys(CSequencerTrack* track, CDC* dc, CRect& rc, Range& timeRange, EDSRenderFlags renderFlags);
	void DrawNodeItem(CSequencerNode* pAnimNode, CDC* dc, CRect& rcItem);

	// Overrides from CSequencerKeys.
	i32  FirstKeyFromPoint(CPoint point, bool exact = false) const;
	i32  LastKeyFromPoint(CPoint point, bool exact = false) const;
	void SelectKeys(const CRect& rc);

	i32  NumKeysFromPoint(CPoint point) const;
};

#endif // __sequencerkeylist_h__

