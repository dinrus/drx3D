// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Controls/EditorDialog.h>

namespace ACE
{
class CCreateFolderDialog final : public CEditorDialog
{
	Q_OBJECT

public:

	explicit CCreateFolderDialog(QWidget* const pParent);

	CCreateFolderDialog() = delete;

signals:

	void SignalSetFolderName(QString const& folderName);

private:

	void OnAccept();

	QString m_folderName;
};
} //endns ACE
