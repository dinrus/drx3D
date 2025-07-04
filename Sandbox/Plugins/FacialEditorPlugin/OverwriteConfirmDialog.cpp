// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "OverwriteConfirmDialog.h"
#include "Resource.h"

BEGIN_MESSAGE_MAP(COverwriteConfirmDialog, CDialog)
ON_COMMAND_EX(IDYES, OnCommand)
ON_COMMAND_EX(IDNO, OnCommand)
ON_COMMAND_EX(ID_YES_ALL, OnCommand)
ON_COMMAND_EX(ID_NO_ALL, OnCommand)
END_MESSAGE_MAP()

COverwriteConfirmDialog::COverwriteConfirmDialog(CWnd* pParentWindow, tukk szMessage, tukk szCaption)
	: CDialog(IDD_CONFIRM_OVERWRITE, pParentWindow), message(szMessage), caption(szCaption)
{
}

BOOL COverwriteConfirmDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	messageEdit.SetWindowText(message.c_str());
	SetWindowText(message.c_str());

	return TRUE;
}

void COverwriteConfirmDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MESSAGE, messageEdit);
}

BOOL COverwriteConfirmDialog::OnCommand(UINT uID)
{
	EndDialog(uID);

	return TRUE;
}

