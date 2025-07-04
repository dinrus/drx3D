// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __databasedialog_h__
#define __databasedialog_h__
#pragma once

#include "Dialogs/ToolbarDialog.h"
#include "IDataBaseItem.h"

#define DATABASE_VIEW_NAME "DataBase View"
#define DATABASE_VIEW_VER  "1.00"

class CEntityProtLibDialog;

//////////////////////////////////////////////////////////////////////////
// Pages of the data base dialog.
//////////////////////////////////////////////////////////////////////////
class CDataBaseDialogPage : public CToolbarDialog
{
	DECLARE_DYNAMIC(CDataBaseDialogPage)
public:
	CDataBaseDialogPage(UINT nIDTemplate, CWnd* pParentWnd = NULL) : CToolbarDialog(nIDTemplate, pParentWnd) {};

	//! This dialog is activated.
	virtual void SetActive(bool bActive) = 0;
	virtual void Update() = 0;
};

/** Main dialog window of DataBase window.
 */
class CDataBaseDialog : public CToolbarDialog
{
	DECLARE_DYNCREATE(CDataBaseDialog)
public:
	CDataBaseDialog(CWnd* pParent = NULL);
	virtual ~CDataBaseDialog();

	enum { IDD = IDD_DATABASE };

	//! Select Object/Terrain
	CDataBaseDialogPage* SelectDialog(EDataBaseItemType type, IDataBaseItem* pItem = NULL);

	CDataBaseDialogPage* GetPage(i32 num);
	i32                  GetSelection() { return m_selectedCtrl; };

	void                 AddTab(tukk szTitle, CDataBaseDialogPage* wnd);
	CDataBaseDialogPage* GetCurrent();

	//! Called every frame.
	void Update();

protected:
	virtual void OnOK()     {};
	virtual void OnCancel() {};
	virtual void PostNcDestroy();

	void         DoDataExchange(CDataExchange* pDX);
	BOOL         OnInitDialog();
	BOOL         OnCmdMsg(UINT nID, i32 nCode, uk pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	//! Activates/Deactivates dialog window.
	void         Activate(CDataBaseDialogPage* dlg, bool bActive);
	void         Select(i32 num);

	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);
	afx_msg void OnTabSelect(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

	CTabCtrl m_tabCtrl;
	CImageList                        m_tabImageList;
	std::vector<CDataBaseDialogPage*> m_windows;
	i32                               m_selectedCtrl;

	//////////////////////////////////////////////////////////////////////////
	// Database dialogs.
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Menu.
	//CXTMenuBar m_menubar;
};

#endif // __databasedialog_h__

