// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "Controls/EditorDialog.h"

#include <QLabel>

class CAboutDialog : public CEditorDialog
{
public:
	CAboutDialog();
	~CAboutDialog();

	void SetVersion(const Version& version);

protected:
	QLabel* m_versionText;
	QLabel* m_miscInfoLabel;

private:
	Version m_version;
};

