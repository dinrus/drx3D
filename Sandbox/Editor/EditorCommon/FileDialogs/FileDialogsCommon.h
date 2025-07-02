// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include <QWidget>
#include "QAdvancedTreeView.h"

namespace FileDialogs
{
//Old "tree view" approach similar to the Explorer and older DrxEngine browser designs
struct EDITOR_COMMON_API FilesTreeView : public QAdvancedTreeView
{
	FilesTreeView(QAbstractItemModel* model, QWidget* parent = nullptr);

	CDrxSignal<void(QModelIndexList&)> onDragFilter;

protected:
	virtual void startDrag(Qt::DropActions supportedActions) override;
	void         OnContextMenu(const QPoint& point);
};

}

