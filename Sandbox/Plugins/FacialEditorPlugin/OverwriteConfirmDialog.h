// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __OVERWRITECONFIRMDIALOG_H__
#define __OVERWRITECONFIRMDIALOG_H__

class COverwriteConfirmDialog : public CDialog
{
public:

	COverwriteConfirmDialog(CWnd* pParentWindow, tukk szMessage, tukk szCaption);

private:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg BOOL OnCommand(UINT uID);

	DECLARE_MESSAGE_MAP()

	string message;
	string caption;
	CEdit  messageEdit;
};

#endif //__OVERWRITECONFIRMDIALOG_H__

