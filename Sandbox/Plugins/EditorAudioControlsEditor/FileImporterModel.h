// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>
#include <QDir>
#include <FileImportInfo.h>

namespace ACE
{
class CFileImporterModel final : public QAbstractItemModel
{
	Q_OBJECT

public:

	enum class EColumns
	{
		Source,
		Target,
		Import,
		Count,
	};

	explicit CFileImporterModel(FileImportInfos& fileImportInfos, QString const& assetFolderPath, QString const& targetPath, QObject* const pParent);

	CFileImporterModel() = delete;

	static QString const s_newAction;
	static QString const s_replaceAction;
	static QString const s_unsupportedAction;
	static QString const s_sameFileAction;

	void             Reset();
	SFileImportInfo& ItemFromIndex(QModelIndex const& index);

signals:

	void SignalActionChanged(Qt::CheckState const isChecked);

protected:
	// QAbstractItemModel
	virtual i32             rowCount(QModelIndex const& parent) const override;
	virtual i32             columnCount(QModelIndex const& parent) const override;
	virtual QVariant        data(QModelIndex const& index, i32 role) const override;
	virtual bool            setData(QModelIndex const& index, QVariant const& value, i32 role) override;
	virtual QVariant        headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual Qt::ItemFlags   flags(QModelIndex const& index) const override;
	virtual QModelIndex     index(i32 row, i32 column, QModelIndex const& parent = QModelIndex()) const override;
	virtual QModelIndex     parent(QModelIndex const& index) const override;
	virtual bool            canDropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) const override;
	virtual bool            dropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) override;
	virtual Qt::DropActions supportedDragActions() const override;
	// ~QAbstractItemModel

private:

	FileImportInfos& m_fileImportInfos;
	QString const    m_targetPath;
	QDir const       m_assetFolder;
};
} //endns ACE
