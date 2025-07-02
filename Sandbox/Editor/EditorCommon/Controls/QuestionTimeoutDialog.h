// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "QuestionDialog.h"

//! This class allows to set a timeout for auto-closing the dialog in addition to the functionality of CQuestionDialog.
class EDITOR_COMMON_API CQuestionTimeoutDialog : public CQuestionDialog
{
	Q_OBJECT
public:
	CQuestionTimeoutDialog();
	~CQuestionTimeoutDialog();

	//! Displays modal dialog.
	//! \param timeoutInSeconds The dialog is closed automatically after a specified amount of time. 
	//! \return User selection or StandardButton::NoButton if the timeout expires.
	QDialogButtonBox::StandardButton Execute(i32 timeoutInSeconds);

protected:
	void timerEvent(QTimerEvent* pEvent);

private:
	void UpdateText();

private:
	QString m_infoLabelText;
	i32 m_timeInSeconds;
	i32 m_timerId;
};

