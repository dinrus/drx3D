// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __BitmapToolTip_h__
#define __BitmapToolTip_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "Controls/ImageHistogramCtrl.h"

//////////////////////////////////////////////////////////////////////////
class CBitmapToolTip : public CWnd
{
	// Construction
public:

	enum EShowMode
	{
		ESHOW_RGB = 0,
		ESHOW_ALPHA,
		ESHOW_RGBA,
		ESHOW_RGB_ALPHA,
		ESHOW_RGBE
	};

	CBitmapToolTip();
	virtual ~CBitmapToolTip();

	BOOL Create(const RECT& rect);

	// Attributes
public:

	// Operations
public:
	void RefreshLayout();
	void RefreshViewmode();

	bool LoadImage(const CString& imageFilename);
	void SetTool(CWnd* pWnd, const CRect& rect);
	void CorrectPosition();

	// Generated message map functions
protected:
	virtual void PreSubclassWindow();
	afx_msg i32  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	DECLARE_MESSAGE_MAP()

private:
	void        GetShowMode(EShowMode& showMode, bool& showInOriginalSize) const;
	tukk GetShowModeDescription(EShowMode showMode, bool showInOriginalSize) const;

	CStatic             m_staticBitmap;
	CStatic             m_staticText;
	CString             m_filename;
	CBitmap             m_previewBitmap;
	BOOL                m_bShowHistogram;
	EShowMode           m_eShowMode;
	bool                m_bShowFullsize;
	bool                m_bHasAlpha;
	bool                m_bIsLimitedHDR;
	CImageHistogramCtrl m_rgbaHistogram, m_alphaChannelHistogram;
	i32                 m_nTimer;
	HWND                m_hToolWnd;
	CRect               m_toolRect;
};

#endif //__BitmapToolTip_h__

