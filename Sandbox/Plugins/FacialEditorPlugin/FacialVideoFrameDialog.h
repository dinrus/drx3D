// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FACIALVIDEOFRAMEDIALOG_H__
#define __FACIALVIDEOFRAMEDIALOG_H__

#include "FacialEdContext.h"
#include "Dialogs/ToolbarDialog.h"
//#include <atlimage.h>

class CImage2;

class CFacialVideoFrameDialog : public CToolbarDialog
{
	DECLARE_DYNAMIC(CFacialVideoFrameDialog)
public:

	CFacialVideoFrameDialog();
	~CFacialVideoFrameDialog();

	void         SetContext(CFacialEdContext* pContext);
	void         SetResolution(i32 width, i32 height, i32 bpp);
	uk        GetBits();
	i32          GetPitch();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK()     {};
	virtual void OnCancel() {};
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnSize(UINT nType, i32 cx, i32 cy);
	virtual void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	CBitmap           m_offscreenBitmap;

	CFacialEdContext* m_pContext;
	HACCEL            m_hAccelerators;
	//ATL::CImage m_image;
	CImage2*          m_image;
	//CStatic m_static;
};

#endif //__FACIALVIDEOFRAMEDIALOG_H__

