// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Controls/EditorDialog.h>

namespace ACE
{
class CPreferencesDialog final : public CEditorDialog
{
	Q_OBJECT

public:

	explicit CPreferencesDialog(QWidget* const pParent);

	CPreferencesDialog() = delete;

signals:

	void SignalImplementationSettingsAboutToChange();
	void SignalImplementationSettingsChanged();
	void SignalEnableSaveButton(bool);

private:

	QString const m_projectPath;
};
} //endns ACE
