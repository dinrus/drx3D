// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QMetaObject>
#include <QObject>
#include <QAbstractItemModel>

struct CAbstractMultiProxyModelHelper
{
	typedef QAbstractItemModel                   SourceModel;
	typedef QAbstractItemModel                   ProxyModel;
	typedef QVector<QMetaObject::Connection>     Connections;
	typedef QAbstractItemModel::LayoutChangeHint LayoutChangeHint;

	typedef QModelIndex                          SourceIndex;
	typedef QModelIndex                          ProxyIndex;
	typedef QList<QPersistentModelIndex>         PersistentModelIndexList;

	CAbstractMultiProxyModelHelper() : m_storedMovedSituation(-1) {}
	virtual ~CAbstractMultiProxyModelHelper() {}

	// return true if the sourceIndex has a mapped proxy index
	// - this is used to split move into a insert/remove or real move on the proxy model
	virtual bool IsSourceIndexMapped(const SourceModel*, const SourceIndex&) const = 0;

	// for data changed you can get the source model of the indices (they are required to be valid)
	virtual void onSourceDataChanged(const SourceIndex& topLeft, const SourceIndex& bottomRight, const QVector<i32>& roles) = 0;
	// all other handlers get the source model
	virtual void onSourceAboutToBeReset(const SourceModel*) = 0;
	virtual void onSourceReset(const SourceModel*) = 0;
	virtual void onSourceLayoutAboutToBeChanged(const SourceModel*, const PersistentModelIndexList& sourceParents, LayoutChangeHint hint) = 0;
	virtual void onSourceLayoutChanged(const SourceModel*, const PersistentModelIndexList& sourceParents, LayoutChangeHint hint) = 0;
	virtual void onSourceRowsAboutToBeInserted(const SourceModel*, const SourceIndex& sourceParent, i32 first, i32 last) = 0;
	virtual void onSourceRowsInserted(const SourceModel*, const SourceIndex& sourceParent, i32 first, i32 last) = 0;
	virtual void onSourceRowsAboutToBeRemoved(const SourceModel*, const SourceIndex& sourceParent, i32 first, i32 last) = 0;
	virtual void onSourceRowsRemoved(const SourceModel*, const SourceIndex& sourceParent, i32 first, i32 last) = 0;
	virtual void onSourceRowsAboutToBeMoved(const SourceModel*, const SourceIndex& sourceParent, i32 sourceStart, i32 sourceEnd, const QModelIndex& destinationParent, i32 destinationRow) = 0;
	virtual void onSourceRowsMoved(const SourceModel*, const SourceIndex& sourceParent, i32 sourceStart, i32 sourceEnd, const QModelIndex& destinationParent, i32 destinationRow) = 0;

	// TODO: add support for columns!

	Connections ConnectSourceModel(const SourceModel*, QObject*);
	void        Disconnect(const Connections&);

private:
	i32 m_storedMovedSituation;
};

