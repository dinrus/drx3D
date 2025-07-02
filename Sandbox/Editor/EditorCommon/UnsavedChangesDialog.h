// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/CoreX/Serialization/DynArray.h>

#include "Controls/EditorDialog.h"
class QListWidget;

//TODO : this is unused, use or remove
// Supposed to be used through
// ConfirmSaveDialog function.
class CUnsavedChangedDialog : public CEditorDialog
{
	Q_OBJECT
public:
	CUnsavedChangedDialog(QWidget* parent);

	bool Exec(DynArray<string>* selectedFiles, const DynArray<string>& files);
private:
	QListWidget* m_list;
	i32          m_result;
};

// Returns true if window should be closed (Yes/No). False for Cancel.
// selectedFiles contains list of files that should be saved (empty in No case).
bool EDITOR_COMMON_API UnsavedChangesDialog(QWidget* parent, DynArray<string>* selectedFiles, const DynArray<string>& files);

