// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "FolderSelectorFilterModel.h"

#include <FileSystem/OsFileSystemModels/AdvancedFileSystemModel.h>

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
CFolderSelectorFilterModel::CFolderSelectorFilterModel(QString const& assetpath, QObject* const pParent)
	: QDeepFilterProxyModel(QDeepFilterProxyModel::Behavior::AcceptIfChildMatches, pParent)
	, m_assetPath(assetpath)
{
}

//////////////////////////////////////////////////////////////////////////
bool CFolderSelectorFilterModel::rowMatchesFilter(i32 sourceRow, QModelIndex const& sourceParent) const
{
	bool matchesFilter = false;

	if (QDeepFilterProxyModel::rowMatchesFilter(sourceRow, sourceParent))
	{
		QModelIndex const& index = sourceModel()->index(sourceRow, 0, sourceParent);

		if (index.isValid())
		{
			QString const& filePath = index.data(QFileSystemModel::Roles::FilePathRole).toString();

			if (filePath.startsWith(m_assetPath + "/", Qt::CaseInsensitive) || (filePath.compare(m_assetPath, Qt::CaseInsensitive) == 0))
			{
				matchesFilter = true;
			}
		}
	}

	return matchesFilter;
}
} //endns ACE
