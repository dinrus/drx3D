// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QAbstractListModel>

#include "FileSystem/FileSystem_FileFilter.h"
#include "FileSystem/FileSystem_Enumerator.h"

#include <memory>

namespace FileSystem
{
class CEnumerator;
}

//! This is meant to display a list of engine files based on a SFileFilter or added manually
class CFileListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum ERole
	{
		eRole_EnginePath = Qt::UserRole + 1
	};

public:
	CFileListModel(FileSystem::CEnumerator& enumerator, const FileSystem::SFileFilter& filter = FileSystem::SFileFilter(), QObject* pParent = nullptr);
	virtual ~CFileListModel();

	void AddEntry(const QString& enginePath);
	void AddEntries(const QStringList& enginePaths);
	void InsertEntry(const QString& enginePath, uint index);
	void InsertEntries(const QStringList& enginePaths, uint index);
	void RemoveEntry(const QString& enginePath);
	void RemoveEntry(const QModelIndex& index);
	void Clear();

	i32 EntriesCount() const;

	bool HasEntry(const QString& enginePath) const;

	QString GetEntry(i32 index) const;
	QStringList GetEntries() const;

	//////////////////////////////////////////////////////////////////////////

	QModelIndex      GetIndexByPath(const QString& enginePath) const;
	QString          GetPathFromIndex(const QModelIndex& index) const;

public:
	virtual i32      rowCount(const QModelIndex& parent) const override;
	virtual QVariant data(const QModelIndex& index, i32 role) const override;
	virtual QVariant headerData(i32 section, Qt::Orientation orientation, i32 role) const override;

	virtual QStringList   mimeTypes() const override;
	virtual QMimeData*    mimeData(const QModelIndexList& indexes) const override;
	virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, i32 row, i32 column, const QModelIndex &parent) const override;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, i32 row, i32 column, const QModelIndex &parent) override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

private:

	void ReorderEntry(const QString& entry, uint newIndex);
	i32 FindEntry(const QString& enginePath) const;

	struct Entry
	{
		QString enginePath;
		FileSystem::DirectoryPtr directoryPtr;
		FileSystem::FilePtr filePtr;

		Entry() : directoryPtr(nullptr), filePtr(nullptr) {}
		bool IsFile() const { return filePtr != nullptr; }
		bool IsDirectory() const { return directoryPtr != nullptr; }
		bool IsValid() const { return directoryPtr != nullptr || filePtr != nullptr; }
	};

	Entry CreateEntry(const QString& enginePath) const;

	FileSystem::CEnumerator&               m_enumerator;
	QVector<Entry> m_entries;
};

