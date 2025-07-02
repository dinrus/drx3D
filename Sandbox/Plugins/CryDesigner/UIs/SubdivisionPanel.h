// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "UICommon.h"

class QSlider;
class QTreeWidget;
class QTreeWidgetItem;

namespace Designer
{
class SubdivisionTool;
class SubdivisionPanel : public QWidget, public IBasePanel
{
public:
	SubdivisionPanel(SubdivisionTool* pTool);

	void     Update() override;
	QWidget* GetWidget() override { return this; }

protected:
	void OnApply();
	void OnDelete();
	void OnDeleteUnused();
	void OnClear();
	void OnItemDoubleClicked(QTreeWidgetItem* item, i32 column);
	void OnItemChanged(QTreeWidgetItem* item, i32 column);
	void OnCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

	void AddSemiSharpGroup();
	void RefreshEdgeGroupList();

	QSlider*         m_pSubdivisionLevelSlider;
	QTreeWidget*     m_pSemiSharpCreaseList;
	SubdivisionTool* m_pSubdivisionTool;
	QString          m_ItemNameBeforeEdit;
};
}

