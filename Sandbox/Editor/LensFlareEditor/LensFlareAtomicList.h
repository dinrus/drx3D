// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2011.
// -------------------------------------------------------------------------
//  File name:   LensFlareAtomicList.h
//  Created:     7/Dec/2012 by Jaesik.
////////////////////////////////////////////////////////////////////////////

#include "Controls/ImageListCtrl.h"

class CLensFlareEditor;

class CLensFlareAtomicList : public CImageListCtrl
{
public:

	struct CFlareImageItem : public CImageListCtrlItem
	{
		EFlareType flareType;
	};

	CLensFlareAtomicList();
	virtual ~CLensFlareAtomicList();

	void FillAtomicItems();

protected:

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	bool OnBeginDrag(UINT nFlags, CPoint point);
	void OnDropItem(CPoint point);
	void OnDraggingItem(CPoint point);
	bool InsertItem(const FlareInfo& flareInfo);

};

