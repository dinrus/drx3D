// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <QAbstractProxyModel>
#include <QFileInfo>

//! This will display a flat lists of selected entries. Despite the name the entries can be directories or files indiscriminately
//! Assumes the parent model is a CAdvancedFileSystemModel (or QFileSystemModel)
//! Drag&dropped 
class EDITOR_COMMON_API CFileCollectionModel : public QAbstractProxyModel
{
public:
	CFileCollectionModel();
	~CFileCollectionModel();

	void AddEntry(const QString& file);
	void AddEntries(const QStringList& entries);
	void InsertEntry(const QString& file, uint index);
	void InsertEntries(const QStringList& entries, uint index);
	void RemoveEntry(const QString& file);
	void RemoveEntry(const QModelIndex& index);
	void Clear();

	i32 EntriesCount() const;

	bool HasEntry(const QString& file) const;

	QString GetEntry(i32 index) const;
	QStringList GetEntries() const;

	//QAbstractItemModel interface
	virtual QVariant headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
	virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
	virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
	virtual i32 columnCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual i32 rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual QModelIndex index(i32 row, i32 column, const QModelIndex &parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex &child) const override;

	virtual QVariant data(const QModelIndex &index, i32 role = Qt::DisplayRole) const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, i32 row, i32 column, const QModelIndex &parent) const override;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, i32 row, i32 column, const QModelIndex &parent) override;

private:

	void ReorderEntry(const QString& entry, uint newIndex);
	QModelIndex GetSourceIndex(const QString &path, i32 column = 0) const;

	struct Entry
	{
		QFileInfo m_fileInfo;
		QPersistentModelIndex m_sourceIndex;
	};

	QVector<Entry> m_entries;
};
