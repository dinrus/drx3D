// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_GENERATIONPARAM_H__B6D3C41A_7769_4657_AD03_0BBEF3D13DEB__INCLUDED_)
#define AFX_GENERATIONPARAM_H__B6D3C41A_7769_4657_AD03_0BBEF3D13DEB__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000
// GenerationParam.h : header file
//

struct SNoiseParams;

/////////////////////////////////////////////////////////////////////////////
// CGenerationParam dialog

class CGenerationParam : public CDialog
{
	// Construction
public:
	void LoadParam(SNoiseParams* pParam);
	void FillParam(SNoiseParams* pParam);
	CGenerationParam(CWnd* pParent = NULL);   // standard constructor

	// Dialog Data
	//{{AFX_DATA(CGenerationParam)
	enum { IDD = IDD_GENERATION };
	i32 m_sldPasses;
	i32 m_sldFrequency;
	i32 m_sldFrequencyStep;
	i32 m_sldFade;
	i32 m_sldCover;
	i32 m_sldRandomBase;
	i32 m_sldSharpness;
	i32 m_sldBlur;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenerationParam)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	void UpdateStaticNum();

	// Generated message map functions
	//{{AFX_MSG(CGenerationParam)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERATIONPARAM_H__B6D3C41A_7769_4657_AD03_0BBEF3D13DEB__INCLUDED_)

