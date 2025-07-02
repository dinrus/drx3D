// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ItemDescriptionDlg.h"
#include "SmartObjectStateDialog.h"
#include "SmartObjectPatternDialog.h"
#include "AI\AIManager.h"

// CSmartObjectPatternDialog dialog

IMPLEMENT_DYNAMIC(CSmartObjectPatternDialog, CDialog)

CSmartObjectPatternDialog::CSmartObjectPatternDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSmartObjectPatternDialog::IDD, pParent)
{
}

CSmartObjectPatternDialog::~CSmartObjectPatternDialog()
{
}

void CSmartObjectPatternDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTBOX, m_wndList);
}

BEGIN_MESSAGE_MAP(CSmartObjectPatternDialog, CDialog)
ON_LBN_DBLCLK(IDC_LISTBOX, OnLbnDblClk)
ON_LBN_SELCHANGE(IDC_LISTBOX, OnLbnSelchange)
ON_BN_CLICKED(IDC_NEW, OnNewBtn)
ON_BN_CLICKED(IDEDIT, OnLbnDblClk)
ON_BN_CLICKED(IDDELETE, OnDeleteBtn)
ON_BN_CLICKED(IDOK, OnOkBtn)
END_MESSAGE_MAP()

// CSmartObjectStateDialog message handlers

void CSmartObjectPatternDialog::OnOkBtn()
{
	m_sPattern.Empty();
	CString item;
	for (i32 i = 0; i < m_wndList.GetCount(); ++i)
	{
		m_wndList.GetText(i, item);
		if (i)
			m_sPattern += " | ";
		m_sPattern += item;
	}
	EndDialog(IDOK);
}

void CSmartObjectPatternDialog::OnNewBtn()
{
	CSmartObjectStateDialog dlg(this, true);
	if (dlg.DoModal() == IDOK)
	{
		CString item = dlg.GetSOState();
		if (!item.IsEmpty())
		{
			m_wndList.SetCurSel(m_wndList.AddString(item));
			GetDlgItem(IDEDIT)->EnableWindow(TRUE);
			GetDlgItem(IDDELETE)->EnableWindow(TRUE);

			SetDlgItemText(IDCANCEL, "Cancel");
			GetDlgItem(IDOK)->EnableWindow(TRUE);
		}
	}
}

void CSmartObjectPatternDialog::OnDeleteBtn()
{
	i32 sel = m_wndList.GetCurSel();
	if (sel != LB_ERR)
	{
		m_wndList.DeleteString(sel);

		SetDlgItemText(IDCANCEL, "Cancel");
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	GetDlgItem(IDEDIT)->EnableWindow(FALSE);
	GetDlgItem(IDDELETE)->EnableWindow(FALSE);
}

void CSmartObjectPatternDialog::OnLbnDblClk()
{
	i32 nSel = m_wndList.GetCurSel();
	if (nSel >= 0)
	{
		CSmartObjectStateDialog dlg(this, true);
		CString item;
		m_wndList.GetText(nSel, item);
		dlg.SetSOState(item);
		if (dlg.DoModal() == IDOK)
		{
			item = dlg.GetSOState();
			if (!item.IsEmpty())
			{
				m_wndList.DeleteString(nSel);
				nSel = m_wndList.AddString(item);
				m_wndList.SetCurSel(nSel);

				SetDlgItemText(IDCANCEL, "Cancel");
				GetDlgItem(IDOK)->EnableWindow(TRUE);
			}
		}
	}
}

void CSmartObjectPatternDialog::OnLbnSelchange()
{
	i32 nSel = m_wndList.GetCurSel();
	GetDlgItem(IDEDIT)->EnableWindow(nSel != LB_ERR);
	GetDlgItem(IDDELETE)->EnableWindow(nSel != LB_ERR);
}

BOOL CSmartObjectPatternDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDEDIT)->EnableWindow(FALSE);
	GetDlgItem(IDDELETE)->EnableWindow(FALSE);

	m_wndList.ResetContent();

	CString token;
	i32 i = 0;
	while (!(token = m_sPattern.Tokenize("|", i)).IsEmpty())
	{
		token = token.Trim();
		if (!token.IsEmpty())
			m_wndList.AddString(token);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

