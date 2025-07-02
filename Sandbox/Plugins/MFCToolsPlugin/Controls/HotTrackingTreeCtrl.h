// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class PLUGIN_API CHotTrackingTreeCtrl : public CTreeCtrl
{
public:
	CHotTrackingTreeCtrl();
	virtual ~CHotTrackingTreeCtrl(){};

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

private:
	HTREEITEM m_hHoverItem;
};

