// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Controls/EditorDialog.h"

class QListView;
class QStandardItemModel;

class QFBXImporterDlg : public CEditorDialog
{
public:
	QFBXImporterDlg();

	void AddImportObject(string objectName);
	bool IsObjectSelected(string objectName);

protected:
	void OnConfirmed();

	QListView*          m_pObjectList;
	QStandardItemModel* m_pModel;

	bool                m_bConfirmed;
};

