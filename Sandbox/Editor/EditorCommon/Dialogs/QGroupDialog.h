// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "Dialogs/QStringDialog.h"

class QLineEdit;

//! DO NOT USE THIS!!!!

// getting string via modal dialog is a bad pattern we are trying to avoid, it's here because of legacy reasons
// if you want to get a string, you should use some inline spawning control, etc.
class EDITOR_COMMON_API QGroupDialog : public QStringDialog
{
	Q_OBJECT

public:
	QGroupDialog(const QString& title = "", QWidget* pParent = NULL);

	void SetGroup(tukk str);
	const string GetGroup();

private:
	QLineEdit* m_pGroup;
};


