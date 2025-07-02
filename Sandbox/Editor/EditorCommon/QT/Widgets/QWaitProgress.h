// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

////////////////////////////////////////////////////////////////////////////
//
//  DrxEngine Source File.
//  Copyright (C), DinrusPro 3D, 2015.
// -------------------------------------------------------------------------
//  File name:   CWaitProgress.h
//  Created:     12/01/2015 by Timur.
//  Description: QWaitProgress class adds information about lengthy process
//		Usage:
//		QWaitProgress wait;
//		wait.SetText("Long");
//		wait.SetProgress(35); // 35 percent.
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

class QProgressBar;

// DEPRECATED DO NOT USE!
// Will be replaced by the notification and task ui system
class EDITOR_COMMON_API CWaitProgress
{
public:
	CWaitProgress(tukk text, bool bStart = true);
	~CWaitProgress();

	void          Start();
	void          Stop();
	//! @return true to continue, false to abort lengthy operation.
	bool          Step(i32 nPercentage = -1, bool bProcessEvents = false);

	void          SetText(tukk text);
	const string& GetProcessName() { return m_strText;  }

protected:

	string      m_strText;
	string      m_processName;
	bool        m_bStarted;
	bool        m_bIgnore;
	i32         m_percent;
	static bool s_bInProgressNow;
};

