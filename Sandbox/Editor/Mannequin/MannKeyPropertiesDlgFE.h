// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MANN_KEY_PROPERTIES_DLG_H__
#define __MANN_KEY_PROPERTIES_DLG_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "Dialogs/ToolbarDialog.h"
//#include "SequencerSplitter.h"
//#include "SequencerNodes.h"
//#include "SequencerDopeSheet.h"

#include "SequencerKeyPropertiesDlg.h"
//#include "SequencerUtils.h"
#include "Dialogs\BaseFrameWnd.h"

//class CMannequinDialog;
//class CFragment;
class IAnimationDatabase;
struct SMannequinContexts;

//////////////////////////////////////////////////////////////////////////
class SequencerKeys;
class CMannKeyPropertiesDlgFE : public CSequencerKeyPropertiesDlg
{
public:
	CMannKeyPropertiesDlgFE();

	enum { IDD = IDD_PANEL_PROPERTIES_FE };

	//////////////////////////////////////////////////////////////////////////
	// ISequencerEventsListener interface implementation
	//////////////////////////////////////////////////////////////////////////
	virtual void OnKeySelectionChange();
	//////////////////////////////////////////////////////////////////////////

	void ForceUpdate()
	{
		m_forceUpdate = true;
	}

	void OnUpdate();

	void RedrawProps();

	void RepopulateUI(CSequencerKeyUIControls* controls);

	SMannequinContexts* m_contexts;

protected:
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();

	i32  m_lastPropertyType;
	bool m_forceUpdate;
};

#endif // __MANN_KEY_PROPERTIES_DLG_H__

