// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FACIALPREVIEWOPTIONSDIALOG_H__
#define __FACIALPREVIEWOPTIONSDIALOG_H__

#include "Controls/PropertiesPanel.h"

class CModelViewport;
class CFacialPreviewDialog;
class CFacialEdContext;

class CFacialPreviewOptionsDialog : public CDialog
{
	DECLARE_DYNAMIC(CFacialPreviewOptionsDialog)
public:

	CFacialPreviewOptionsDialog();
	~CFacialPreviewOptionsDialog();

	void SetViewport(CFacialPreviewDialog* pPreviewDialog);
	void SetContext(CFacialEdContext* pContext);

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK()     {};
	virtual void OnCancel() {};
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnSize(UINT nType, i32 cx, i32 cy);

	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	CPropertiesPanel*     m_panel;
	CModelViewport*       m_pModelViewportCE;
	CFacialPreviewDialog* m_pPreviewDialog;
	HACCEL                m_hAccelerators;
	CFacialEdContext*     m_pContext;
};

#endif //__FACIALPREVIEWOPTIONSDIALOG_H__

