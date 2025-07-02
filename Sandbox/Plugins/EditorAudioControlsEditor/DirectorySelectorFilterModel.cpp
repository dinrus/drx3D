// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "DirectorySelectorFilterModel.h"

#include <FileSystem/OsFileSystemModels/AdvancedFileSystemModel.h>

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
CDirectorySelectorFilterModel::CDirectorySelectorFilterModel(QString const& assetpath, QObject* const pParent)
	: QDeepFilterProxyModel(QDeepFilterProxyModel::Behavior::AcceptIfChildMatches, pParent)
	, m_assetPath(assetpath)
{
}

//////////////////////////////////////////////////////////////////////////
bool CDirectorySelectorFilterModel::rowMatchesFilter(i32 sourceRow, QModelIndex const& sourceParent) const
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
