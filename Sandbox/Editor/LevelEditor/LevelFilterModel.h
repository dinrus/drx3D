// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QSortFilterProxyModel>
#include <QHash>

class CLevelFilterModel : public QSortFilterProxyModel
{
	// Used to create a cache for determining whether or
	// not a given path represents a level.
	class CPathIsLevelCache
	{
	public:
		bool IsLevel(const QString& path);
		void Reset();

	private:
		QHash<QString, bool> m_cache;
	};

public:
	CLevelFilterModel(QObject* pParent = nullptr);

	// QSortFilterProxyModel interface
	virtual bool     hasChildren(const QModelIndex& parent = QModelIndex()) const override;
	virtual QVariant data(const QModelIndex& index, i32 role = Qt::DisplayRole) const override;
	virtual bool     filterAcceptsRow(i32 sourceRow, const QModelIndex& sourceParent) const override;

public:
	bool IsIndexLevel(const QModelIndex& index) const;
	bool IsIndexLevelOrContainsLevels(const QModelIndex& index, const QString& path) const;

private:
	mutable CPathIsLevelCache m_cache;
};

