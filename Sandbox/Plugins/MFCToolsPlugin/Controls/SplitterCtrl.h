// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   splitterwndex.h
//  Created:     24/4/2002 by Timur.
//  Description: CSplitterCtrl (former CSplitterWndEx) class.
//
////////////////////////////////////////////////////////////////////////////

class PLUGIN_API CSplitterCtrl : public CSplitterWnd
{
public:
	DECLARE_DYNAMIC(CSplitterCtrl)

	CSplitterCtrl();
	~CSplitterCtrl();

	virtual CWnd* GetActivePane(i32* pRow = NULL, i32* pCol = NULL);
	//! Assign any Window to splitter window pane.
	void          SetPane(i32 row, i32 col, CWnd* pWnd, SIZE sizeInit);
	// Override this for flat look.
	void          OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg);
	void          SetNoBorders();
	void          SetTrackable(bool bTrackable);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	// override CSplitterWnd special command routing to frame
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	bool m_bTrackable;
};

