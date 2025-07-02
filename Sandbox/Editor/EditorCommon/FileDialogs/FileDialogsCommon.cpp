// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "FileDialogsCommon.h"
#include <drx3D/Sys/File/IDrxPak.h>

#include <QHeaderView>
#include <QDir>
#include <QDrag>
#include <QMenu>
#include <QAction>

namespace FileDialogs
{
FilesTreeView::FilesTreeView(QAbstractItemModel* model, QWidget* parent)
{
	header()->setSectionResizeMode(QHeaderView::Interactive);
	setSortingEnabled(true);
	setUniformRowHeights(true);
	setAllColumnsShowFocus(true);
	header()->setStretchLastSection(false);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	sortByColumn(0, Qt::AscendingOrder);
	setModel(model);

	header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header(), &QHeaderView::customContextMenuRequested, this, &FilesTreeView::OnContextMenu);
}

void FilesTreeView::startDrag(Qt::DropActions supportedActions)
{
	QModelIndexList indexes = selectionModel()->selectedIndexes();
	onDragFilter(indexes);
	// If there are no valid indices to drag then just return
	if (indexes.empty())
		return;

	QDrag* pDrag = new QDrag(this);
	QMimeData* pMimeData = model()->mimeData(indexes);
	pDrag->setMimeData(pMimeData);
	pDrag->exec(supportedActions);
}

void FilesTreeView::OnContextMenu(const QPoint& point)
{
	QAbstractItemModel* pModel = model();
	if (pModel)
	{
		QMenu menu;
		i32 columnCount = pModel->columnCount();
		for (i32 i = 0; i < columnCount; ++i)
		{
			QAction* pAction = menu.addAction(pModel->headerData(i, Qt::Horizontal).toString());
			pAction->setCheckable(true);
			pAction->setChecked(!header()->isSectionHidden(i));
			pAction->setData(QVariant(i));

			connect(pAction, &QAction::toggled, [=](bool bChecked)
				{
					i32 column = pAction->data().toInt();
					if (bChecked)
						header()->showSection(i);
					else
						header()->hideSection(i);
			  });
		}
		menu.exec(QCursor::pos());
	}
}

}

