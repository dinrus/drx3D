// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <QtCore/QAbstractItemModel>
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

namespace Explorer
{

struct ExplorerEntry;
struct ExplorerEntryModifyEvent;
class ExplorerData;

class EDITOR_COMMON_API ExplorerModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	ExplorerModel(ExplorerData* explorerData, QObject* parent);

	void                  SetRootByIndex(i32 index);
	i32                   GetRootIndex() const;
	ExplorerEntry*        GetActiveRoot() const;

	static ExplorerEntry* GetEntry(const QModelIndex& index);

	// QAbstractItemModel
	QModelIndex index(i32 row, i32 column, const QModelIndex& parent) const override;
	i32         rowCount(const QModelIndex& parent) const override;
	i32         columnCount(const QModelIndex& parent) const override;

	QVariant    headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	bool        hasChildren(const QModelIndex& parent) const override;
	QVariant    data(const QModelIndex& index, i32 role) const override;
	QModelIndex ModelIndexFromEntry(ExplorerEntry* entry, i32 column) const;
	QModelIndex NewModelIndexFromEntry(ExplorerEntry* entry, i32 column) const;
	QModelIndex parent(const QModelIndex& index) const override;

public slots:
	void OnEntryModified(ExplorerEntryModifyEvent& ev);
	void OnBeginAddEntry(ExplorerEntry* entry);
	void OnEndAddEntry();
	void OnBeginRemoveEntry(ExplorerEntry* entry);
	void OnEndRemoveEntry();
protected:

	bool          m_addWithinActiveRoot;
	bool          m_removeWithinActiveRoot;
	i32           m_rootIndex;
	i32           m_rootSubtree;
	ExplorerData* m_explorerData;
};

}

