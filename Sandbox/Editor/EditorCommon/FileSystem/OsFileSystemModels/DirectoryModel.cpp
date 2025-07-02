// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "DirectoryModel.h"

#include "AdvancedFileSystemModel.h"

CDirectoriesOnlyProxyModel::CDirectoriesOnlyProxyModel(QObject* parent)
	: QSortFilterProxyModel(parent)
{}

bool CDirectoriesOnlyProxyModel::filterAcceptsRow(i32 sourceRow, const QModelIndex& sourceParent) const
{
	auto fileSystem = static_cast<CAdvancedFileSystemModel*>(sourceModel());
	auto sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
	return fileSystem->isDir(sourceIndex);
}

void CDirectoriesOnlyProxyModel::setSourceModel(CAdvancedFileSystemModel* sourceFileSystem)
{
	QSortFilterProxyModel::setSourceModel(sourceFileSystem);
}

