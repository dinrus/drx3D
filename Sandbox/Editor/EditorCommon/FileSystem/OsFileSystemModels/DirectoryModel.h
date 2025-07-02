// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QSortFilterProxyModel>

class CAdvancedFileSystemModel;

/**
 * \brief specialized filter that filters out all files
 * \note used for the directory column
 */
class CDirectoriesOnlyProxyModel
	: public QSortFilterProxyModel
{
	// as we do not need registered QObject behavior it's not registered as such
public:
	explicit CDirectoriesOnlyProxyModel(QObject* parent = nullptr);

	// QSortFilterProxyModel interface
protected:
	virtual bool filterAcceptsRow(i32 sourceRow, const QModelIndex& sourceParent) const;

public:
	virtual void setSourceModel(CAdvancedFileSystemModel* sourceFileSystem);
};

