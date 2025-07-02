// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <QAbstractItemModel>

#include <functional>
#include <memory>

/**
 * This is a proxy model that supports mounting submodels to rows
 *
 * output:
 *
 * / - root of SourceModel
 * s0/
 *   s1/ - MountingPoint, root of SubModel
 *     k0/
 *       k1
 *
 *
 * Note: This model is not a magic bullet!
 * - if you need other features consider adding them - but try to avoid expensive models!
 * - mount points are only allowed on leafs
 *   * this was not part of my just tests!
 * - each submodel can only be attached once!
 *   * otherwise sort would create overlapping begin-end-events.
 * - modifing columns at runtime is not supported
 *   * no signal handling for this is implemented
 * - nested mounting should be possible
 *   * be aware that the factory receives indices from all models!
 *   * this is untested!
 *
 * Usage of QAbstractProxyModel would be dangerous, it falls back to sourceModel too often.
 */
class EDITOR_COMMON_API CMountingProxyModel
	: public QAbstractItemModel
{
	Q_OBJECT
public:
	typedef QModelIndex        SourceIndex;
	typedef QModelIndex        ProxyIndex;
	typedef QAbstractItemModel SourceModel;

	/// creates a new sourceModel for a valid source index
	typedef std::function<SourceModel*(const SourceIndex&)>                               SourceModelFactory;

	typedef std::function<QVariant(i32 section, Qt::Orientation, i32 role)>               GetHeaderDataCallback;
	typedef std::function<bool (i32 section, Qt::Orientation, const QVariant&, i32 role)> SetHeaderDataCallback;

public:
	explicit CMountingProxyModel(const SourceModelFactory& = SourceModelFactory(), QObject* parent = nullptr);
	~CMountingProxyModel();

signals:
	/// Invoked whenever the sourceindex was changed
	void MountPointDataChanged(const QModelIndex& proxyIndex, const QAbstractItemModel* sourceModel);

public:
	/// \returns the main source model that is mounted to rootIndex
	const SourceModel* GetSourceModel() const;

	/// \returns the source model that is mounted at the given proxy index
	const SourceModel* GetMountSourceModel(const ProxyIndex&) const;

	/// \brief set the main source model
	/// \note a main source model is required before you can use this class
	void SetSourceModel(SourceModel* newSourceModel);

	/// \brief set the header data callbacks and the column count for the final model
	/// \note mergeValueRole is used to map all mounted models to the columns
	void SetHeaderDataCallbacks(i32 columnCount, const GetHeaderDataCallback&, i32 mergeValueRole = Qt::DisplayRole, const SetHeaderDataCallback& = SetHeaderDataCallback());

	/// \returns the source index for a given proxy index
	SourceIndex MapToSource(const ProxyIndex&) const;

	/// \returns the proxy index for a given source index
	ProxyIndex MapFromSource(const SourceIndex&) const;

	/// \returns the proxy index for a given source index or the
	ProxyIndex MapFromSourceModel(const SourceModel*) const;

	// interface QAbstractItemModel
	virtual QModelIndex         index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex         parent(const QModelIndex& child) const override;
	virtual QModelIndex         sibling(i32 row, i32 column, const QModelIndex&) const override;
	virtual i32                 rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual i32                 columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual bool                hasChildren(const QModelIndex& parent) const override;

	virtual Qt::ItemFlags       flags(const QModelIndex& proxyIndex) const override;
	virtual QVariant            data(const QModelIndex& proxyIndex, i32 role = Qt::DisplayRole) const override;
	virtual bool                setData(const QModelIndex& proxyIndex, const QVariant& value, i32 role) override;
	virtual QVariant            headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual bool                setHeaderData(i32 section, Qt::Orientation orientation, const QVariant& value, i32 role = Qt::EditRole) override;
	virtual QMap<i32, QVariant> itemData(const QModelIndex& proxyIndex) const override;
	virtual bool                setItemData(const QModelIndex& proxyIndex, const QMap<i32, QVariant>& roles) override;

	// mime & drag/drop only works on the sourceModel
	virtual QStringList     mimeTypes() const override;
	virtual QMimeData*      mimeData(const QModelIndexList& indexes) const override;
	virtual bool            canDropMimeData(const QMimeData* data, Qt::DropAction action, i32 row, i32 column, const QModelIndex& proxyParent) const override;
	virtual bool            dropMimeData(const QMimeData* data, Qt::DropAction action, i32 row, i32 column, const QModelIndex& proxyParent) override;
	virtual Qt::DropActions supportedDropActions() const override;
	virtual Qt::DropActions supportedDragActions() const override;
	void SetDragCallback(const std::function<QMimeData*(const QModelIndexList&)>& callback);

	virtual void            fetchMore(const QModelIndex& proxyParent) override;
	virtual bool            canFetchMore(const QModelIndex& proxyParent) const override;

	virtual void            sort(i32 column, Qt::SortOrder order = Qt::AscendingOrder) override;
	virtual QModelIndex     buddy(const QModelIndex& proxyIndex) const override;
	virtual QSize           span(const QModelIndex& proxyIndex) const override;

	// header and roleNames are taken from sourceModel
	virtual QHash<i32, QByteArray> roleNames() const override;

public slots:
	// these are moc slots to enable signal passing through threads
	void ColumnsChanged(i32 columnCount);
	void MountToSource(const QPersistentModelIndex& sourceIndex, QAbstractItemModel* sourceModel);
	void Mount(const QModelIndex& proxyIndex, QAbstractItemModel* sourceModel);
	void Unmount(const QModelIndex& proxyIndex);

private:
	struct Implementation;
	std::unique_ptr<Implementation> p;
};

