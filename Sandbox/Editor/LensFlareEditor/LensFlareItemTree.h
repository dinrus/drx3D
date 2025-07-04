// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2011.
// -------------------------------------------------------------------------
//  File name:   LensFlareItemTree.h
//  Created:     7/Jan/2013 by Jaesik.
////////////////////////////////////////////////////////////////////////////

#include "LensFlareElement.h"
#include "ILensFlareListener.h"

class CLensFlareItem;

class CLensFlareItemTree : public CXTTreeCtrl
{
public:

	CLensFlareItemTree();
	~CLensFlareItemTree();

	void UpdateLensFlareItem(HTREEITEM hSelectedItem, const CPoint& screenPos);
	void UpdateDraggingFromOtherWindow();
	bool SelectItemByCursorPos(bool* pOutChanged = NULL);
	void Drop(XmlNodeRef xmlNode, const CPoint& screenPos);

private:

	DECLARE_MESSAGE_MAP()

	void         AssignLensFlareToLightEntity(XmlNodeRef xmlNode, const CPoint& screenPos) const;

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);

};

