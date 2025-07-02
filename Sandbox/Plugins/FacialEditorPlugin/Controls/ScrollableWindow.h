// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SCROLLABLEWINDOW_H__
#define __SCROLLABLEWINDOW_H__

#pragma once

class CScrollableWindow : public CWnd
{
public:
	CScrollableWindow();
	virtual ~CScrollableWindow();

	void         SetClientSize(u32 nWidth, u32 nHeight);

	void         SetAutoScrollWindowFlag(bool boAutoScrollWindow);
protected:
	void         OnClientSizeUpdated();
	void         UpdateScrollBars();

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);

	DECLARE_MESSAGE_MAP()

	CRect m_stDesiredClientSize;
	bool m_bHVisible;
	bool m_bVVisible;
	bool m_boShowing;

	bool m_boAutoScrollWindow;
};

#endif // __SCROLLABLEWINDOW_H__

