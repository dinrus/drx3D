// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "HotTrackingTreeCtrl.h"

BEGIN_MESSAGE_MAP(CHotTrackingTreeCtrl, CTreeCtrl)
ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CHotTrackingTreeCtrl::CHotTrackingTreeCtrl()
	: CTreeCtrl()
{
	m_hHoverItem = NULL;
}

BOOL CHotTrackingTreeCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (nFlags)
	{
		HTREEITEM hItem = GetFirstVisibleItem();

		if (hItem)
		{
			HTREEITEM hTmpItem = 0;
			i32 d = abs(zDelta / WHEEL_DELTA) * 2;
			for (i32 i = 0; i < d; i++)
			{
				if (zDelta < 0)
					hTmpItem = GetNextVisibleItem(hItem);
				else
					hTmpItem = GetPrevVisibleItem(hItem);
				if (hTmpItem)
					hItem = hTmpItem;
			}
			if (hItem)
				SelectSetFirstVisible(hItem);
		}
	}
	return CTreeCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

