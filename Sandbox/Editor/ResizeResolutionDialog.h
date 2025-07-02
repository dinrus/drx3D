// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_RESIZERESOLUTIONDIALOG_H__C8E98A11_BBE9_499F_AB62_6EA12BDBCF85__INCLUDED_)
#define AFX_RESIZERESOLUTIONDIALOG_H__C8E98A11_BBE9_499F_AB62_6EA12BDBCF85__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000
// ResizeResolutionDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResizeResolutionDialog dialog

class CResizeResolutionDialog : public CDialog
{
	// Construction
public:
	CResizeResolutionDialog(CWnd* pParent = NULL);   // standard constructor

	// Dialog Data
	//{{AFX_DATA(CResizeResolutionDialog)
	enum { IDD = IDD_RESIZETILERESOLUTION };
	//}}AFX_DATA

	void   SetSize(u32 dwSize);
	u32 GetSize();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizeResolutionDialog)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions

	virtual BOOL OnInitDialog();
	void         OnCbnSelendokResolution();

	//{{AFX_MSG(CResizeResolutionDialog)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CComboBox m_resolution;
	u32    m_dwInitSize;
	i32       m_curSel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZERESOLUTIONDIALOG_H__C8E98A11_BBE9_499F_AB62_6EA12BDBCF85__INCLUDED_)

