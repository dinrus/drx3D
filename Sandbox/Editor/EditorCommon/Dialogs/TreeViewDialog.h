// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include "Controls/EditorDialog.h"

class QAbstractItemModel;
class QAdvancedTreeView;
class QSearchBox;

class EDITOR_COMMON_API CTreeViewDialog : public CEditorDialog
{
public:
	CTreeViewDialog::CTreeViewDialog(QWidget* pParent)
		: CEditorDialog("TreeViewDialog", pParent)
		, m_pSearchBox(nullptr)
	{}

	void        Initialize(QAbstractItemModel* pModel, i32 filterColumn, const std::vector<i32>& visibleColumns = std::vector<i32>(0));
	void        SetSelectedValue(const QModelIndex& index, bool bExpand);

	QModelIndex GetSelected();

protected:
	void OnOk();

	virtual void showEvent(QShowEvent* event) override;
private:
	QSearchBox*		   m_pSearchBox;
	QAdvancedTreeView* m_pTreeView;
};

