// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Controls/NumberCtrl.h"

class CJoystickPropertiesDlg : public CDialog
{
	DECLARE_DYNAMIC(CJoystickPropertiesDlg)

public:
	CJoystickPropertiesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CJoystickPropertiesDlg();

	void  SetChannelName(i32 axis, const string& channelName);
	void  SetChannelFlipped(i32 axis, bool flipped);
	bool  GetChannelFlipped(i32 axis) const;
	void  SetVideoScale(i32 axis, float offset);
	float GetVideoScale(i32 axis) const;
	void  SetVideoOffset(i32 axis, float offset);
	float GetVideoOffset(i32 axis) const;

	void  SetChannelEnabled(i32 axis, bool enabled);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	BOOL         OnInitDialog();
	virtual void OnOK();

	CEdit       m_nameEdits[2];
	CNumberCtrl m_scaleEdits[2];
	CButton     m_flippedButtons[2];
	CNumberCtrl m_offsetEdits[2];

	string      m_names[2];
	float       m_scales[2];
	bool        m_flippeds[2];
	float       m_offsets[2];
	bool        m_enableds[2];
};
