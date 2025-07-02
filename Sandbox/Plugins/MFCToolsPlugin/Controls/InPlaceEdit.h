// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __inplaceedit_h__
#define __inplaceedit_h__

#if _MSC_VER > 1000
	#pragma once
#endif

class CInPlaceEdit : public CXTPEdit
{
public:
	typedef Functor0 OnChange;

	CInPlaceEdit(const CString& srtInitText, OnChange onchange);
	virtual ~CInPlaceEdit();

	// Attributes
	void SetText(const CString& strText);

	void EnableUpdateOnKillFocus(bool bEnable) { m_bUpdateOnKillFocus = bEnable; }
	void SetUpdateOnEnChange(bool bEnable)     { m_bUpdateOnEnChange = bEnable; }

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceEdit)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	afx_msg i32  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnEnChange();

	DECLARE_MESSAGE_MAP()

	// Data
protected:

	bool     m_bUpdateOnKillFocus;
	bool     m_bUpdateOnEnChange;
	CString  m_strInitText;
	OnChange m_onChange;
};

#endif // __inplaceedit_h__

