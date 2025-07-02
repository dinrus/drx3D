// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Controls/ColorCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// This notification sent to parent after some page is expanded.
#define  ROLLUPCTRLN_EXPAND (0x0001)

struct CRollupCtrlNotify
{
	NMHDR hdr;
	i32   nPageId;
	bool  bExpand;
};

/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl structure and defines

struct RC_PAGEINFO
{
	CWnd*                pwndTemplate;
	CButton*             pwndButton;
	CColorCtrl<CButton>* pwndGroupBox;
	BOOL                 bExpanded;
	BOOL                 bEnable;
	BOOL                 bAutoDestroyTpl;
	WNDPROC              pOldDlgProc; //Old wndTemplate(Dialog) window proc
	WNDPROC              pOldButProc; //Old wndTemplate(Dialog) window proc
	i32                  id;
};

#define RC_PGBUTTONHEIGHT 18
#define RC_SCROLLBARWIDTH 12
#define RC_GRPBOXINDENT   6
const COLORREF RC_SCROLLBARCOLOR = ::GetSysColor(COLOR_BTNFACE); //RGB(150,180,180)
#define RC_ROLLCURSOR     MAKEINTRESOURCE(32649)                 // see IDC_HAND (WINVER >= 0x0500)

//Popup Menu Ids
#define RC_IDM_EXPANDALL   0x100
#define RC_IDM_COLLAPSEALL 0x101
#define RC_IDM_STARTPAGES  0x102

/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl class definition

class PLUGIN_API CRollupCtrl : public CWnd
{
	DECLARE_DYNCREATE(CRollupCtrl)

public:

	// Constructor-Destructor
	CRollupCtrl();
	virtual ~CRollupCtrl();

	// Methods
	BOOL         Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	i32          InsertPage(LPCTSTR caption, CDialog* pwndTemplate, BOOL bAutoDestroyTpl = TRUE, i32 idx = -1, BOOL bAutoExpand = TRUE); //Return page zero-based index
	i32          InsertPage(LPCTSTR caption, UINT nIDTemplate, CRuntimeClass* rtc, i32 idx = -1);                                        //Return page zero-based index

	void         RemovePage(i32 idx); //idx is a zero-based index
	void         RemoveAllPages();

	void         ExpandPage(i32 idx, BOOL bExpand = TRUE, BOOL bScrool = TRUE, BOOL bFromUI = FALSE); //idx is a zero-based index
	void         ExpandAllPages(BOOL bExpand = TRUE, BOOL bFromUI = FALSE);

	void         EnablePage(i32 idx, BOOL bEnable = TRUE); //idx is a zero-based index
	void         EnableAllPages(BOOL bEnable = TRUE);

	void         RenamePage(i32 idx, tukk caption);

	i32          GetPagesCount() { return m_PageList.size(); }

	RC_PAGEINFO* GetPageInfo(i32 idx);  //idx is a zero-based index

	// New v1.01 Methods
	void ScrollToPage(i32 idx, BOOL bAtTheTop = TRUE);
	i32  MovePageAt(i32 idx, i32 newidx);   //newidx can be equal to -1 (move at end)
	BOOL IsPageExpanded(i32 idx);
	BOOL IsPageEnabled(i32 idx);

	void SetBkColor(COLORREF bkColor);

protected:

	// Internal methods
	void         RecalLayout();
	void         RecalcHeight();
	i32          GetPageIdxFromButtonHWND(HWND hwnd);
	void         _RemovePage(i32 idx);
	void         _ExpandPage(RC_PAGEINFO* pi, BOOL bExpand, BOOL bFromUI);
	void         _EnablePage(RC_PAGEINFO* pi, BOOL bEnable);
	i32          _InsertPage(LPCTSTR caption, CDialog* dlg, i32 idx, BOOL bAutoDestroyTpl, BOOL bAutoExpand = TRUE);
	void         _RenamePage(RC_PAGEINFO* pi, LPCTSTR caption);
	void         DestroyAllPages();

	RC_PAGEINFO* FindPage(i32 id);
	i32          FindPageIndex(i32 id);

	// Datas
	CString                   m_strMyClass;
	std::vector<RC_PAGEINFO*> m_PageList;
	i32                       m_nStartYPos, m_nPageHeight;
	i32                       m_nOldMouseYPos, m_nSBOffset;
	i32                       m_lastId;
	bool                      m_bRecalcLayout;

	std::map<CString, bool>   m_expandedMap;

	CBrush                    m_bkgBrush;
	COLORREF                  m_bkColor;

	// Window proc
	static LRESULT CALLBACK DlgWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK ButWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRollupCtrl)
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CRollupCtrl)
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg i32  OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC*, CWnd * pWnd, UINT);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



