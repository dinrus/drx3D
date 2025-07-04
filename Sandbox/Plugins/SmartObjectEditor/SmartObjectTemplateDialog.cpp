// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include <DrxAISystem/IAISystem.h>
#include "ItemDescriptionDlg.h"
#include "AI/AIManager.h"

#include "SmartObjectTemplateDialog.h"

// CSmartObjectTemplateDialog dialog

IMPLEMENT_DYNAMIC(CSmartObjectTemplateDialog, CDialog)
CSmartObjectTemplateDialog::CSmartObjectTemplateDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSmartObjectTemplateDialog::IDD, pParent)
{
}

CSmartObjectTemplateDialog::~CSmartObjectTemplateDialog()
{
}

void CSmartObjectTemplateDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANCHORS, m_wndTemplateList);
}

BEGIN_MESSAGE_MAP(CSmartObjectTemplateDialog, CDialog)
ON_LBN_DBLCLK(IDC_ANCHORS, OnLbnDblClk)
ON_LBN_SELCHANGE(IDC_ANCHORS, OnLbnSelchangeTemplate)
//	ON_BN_CLICKED(IDC_NEW, OnNewBtn)
//	ON_BN_CLICKED(IDEDIT, OnEditBtn)
//	ON_BN_CLICKED(IDDELETE, OnDeleteBtn)
//	ON_BN_CLICKED(IDREFRESH, OnRefreshBtn)
END_MESSAGE_MAP()

// CSmartObjectTemplateDialog message handlers

void CSmartObjectTemplateDialog::OnDeleteBtn()
{
}

void CSmartObjectTemplateDialog::OnNewBtn()
{
}

void CSmartObjectTemplateDialog::OnEditBtn()
{
}

void CSmartObjectTemplateDialog::OnRefreshBtn()
{
	// add empty string item
	m_wndTemplateList.ResetContent();

	IAIManager* pAIMgr = GetIEditor()->GetAIManager();
	ASSERT(pAIMgr);

	const MapTemplates& mapTemplates = pAIMgr->GetMapTemplates();
	MapTemplates::const_iterator it, itEnd = mapTemplates.end();
	for (it = mapTemplates.begin(); it != itEnd; ++it)
	{
		i32 index = m_wndTemplateList.AddString(it->second->name);
		m_wndTemplateList.SetItemData(index, it->first);
	}

	if (m_idSOTemplate >= 0)
	{
		it = mapTemplates.find(m_idSOTemplate);
		if (it != mapTemplates.end())
			m_wndTemplateList.SelectString(-1, it->second->name);
	}
	UpdateDescription();
}

void CSmartObjectTemplateDialog::OnLbnDblClk()
{
	if (m_wndTemplateList.GetCurSel() >= 0)
		EndDialog(IDOK);
}

void CSmartObjectTemplateDialog::OnLbnSelchangeTemplate()
{
	SetDlgItemText(IDCANCEL, "Cancel");

	i32 nSel = m_wndTemplateList.GetCurSel();
	if (nSel == LB_ERR)
	{
		m_idSOTemplate = -1;
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return;
	}
	m_idSOTemplate = m_wndTemplateList.GetItemData(nSel);
	GetDlgItem(IDOK)->EnableWindow(m_idSOTemplate != -1);
	UpdateDescription();
}

void CSmartObjectTemplateDialog::UpdateDescription()
{
	tukk sTemplateDesc = "";
	if (m_idSOTemplate >= 0)
	{
		IAIManager* pAIMgr = GetIEditor()->GetAIManager();
		ASSERT(pAIMgr);

		const MapTemplates& mapTemplates = pAIMgr->GetMapTemplates();
		MapTemplates::const_iterator it = mapTemplates.find(m_idSOTemplate);
		if (it != mapTemplates.end())
			sTemplateDesc = it->second->description;
	}
	SetDlgItemText(IDC_DESCRIPTION, sTemplateDesc);
}

BOOL CSmartObjectTemplateDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetWindowText("Smart Object Templates");
	SetDlgItemText(IDC_LISTCAPTION, "&Choose Smart Object Template:");

	GetDlgItem(IDC_NEW)->EnableWindow(FALSE);
	GetDlgItem(IDEDIT)->EnableWindow(FALSE);
	GetDlgItem(IDDELETE)->EnableWindow(FALSE);
	GetDlgItem(IDREFRESH)->EnableWindow(FALSE);

	OnRefreshBtn();

	GetDlgItem(IDOK)->EnableWindow(m_idSOTemplate != -1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSmartObjectTemplateDialog::SetSOTemplate(tukk sSOTemplate)
{
	m_idSOTemplate = -1;

	const MapTemplates& mapTemplates = GetIEditor()->GetAIManager()->GetMapTemplates();
	MapTemplates::const_iterator it, itEnd = mapTemplates.end();
	for (it = mapTemplates.begin(); it != itEnd; ++it)
		if (it->second->name == sSOTemplate)
		{
			m_idSOTemplate = it->first;
			break;
		}
}

tukk CSmartObjectTemplateDialog::GetSOTemplate() const
{
	tukk sName = "";
	if (m_idSOTemplate >= 0)
	{
		IAIManager* pAIMgr = GetIEditor()->GetAIManager();
		ASSERT(pAIMgr);

		const MapTemplates& mapTemplates = pAIMgr->GetMapTemplates();
		MapTemplates::const_iterator it = mapTemplates.find(m_idSOTemplate);
		if (it != mapTemplates.end())
			sName = it->second->name;
	}
	return sName;
};

