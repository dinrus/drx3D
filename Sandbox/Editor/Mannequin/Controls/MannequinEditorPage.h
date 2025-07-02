// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MannequinEditorPage_h__
#define __MannequinEditorPage_h__
#pragma once

#include "Dialogs/ToolbarDialog.h"

class CMannequinModelViewport;
class CMannDopeSheet;
class CMannNodesCtrl;

class CMannequinEditorPage : public CToolbarDialog
{
	DECLARE_DYNAMIC(CMannequinEditorPage)

public:
	CMannequinEditorPage(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CMannequinEditorPage();

	virtual CMannequinModelViewport* ModelViewport() const { return NULL; }
	virtual CMannDopeSheet*          TrackPanel()          { return NULL; }
	virtual CMannNodesCtrl*          Nodes()               { return NULL; }

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void ValidateToolbarButtonsState() {};

private:
};

#endif // __MannequinEditorPage_h__

