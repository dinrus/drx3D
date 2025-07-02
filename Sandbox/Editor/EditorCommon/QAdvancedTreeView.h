// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QTreeView>
#include <QModelIndex>
#include <QPersistentModelIndex>

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include "ProxyModels/ItemModelAttribute.h"
#include "EditorFramework/StateSerializable.h"

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

class QAdvancedItemDelegate;

//These will persist through reset as long as your internalId/internalPointers are stable through resets
class EDITOR_COMMON_API CAdvancedPersistentModelIndex
{
public:
	CAdvancedPersistentModelIndex(const QModelIndex& index);

	QModelIndex toIndex() const;
private:

	QPersistentModelIndex     m_persistentIndex;
	const uint                m_column;
	const QAbstractItemModel* m_model;
	std::deque<quintptr>      m_identifiers;
};

//!Extends functionality of the QTreeView to cover common use cases:
//! - Restores expanded state after reset calls
//! - Restores selection after reset calls
//! - Implements the attribute-based features
class EDITOR_COMMON_API QAdvancedTreeView : public QTreeView, public IStateSerializable
{
public:

	enum Behavior
	{
		None                        = 0,
		PreserveExpandedAfterReset  = 0x01,
		PreserveSelectionAfterReset = 0x02,
		UseItemModelAttribute       = 0x04
	};

	Q_DECLARE_FLAGS(BehaviorFlags, Behavior)

	QAdvancedTreeView(BehaviorFlags flags = BehaviorFlags(PreserveExpandedAfterReset | PreserveSelectionAfterReset), QWidget* parent = nullptr);
	~QAdvancedTreeView();

	virtual void setModel(QAbstractItemModel* model) override;
	virtual void reset() override;

	void                ConnectDefaultHeaderContextMenu();
	void                DisconnectDefaultHeaderContextMenu();

	//! Set once, this is not expected to change
	void                SetAttributeRole(i32 role = Attributes::s_getAttributeRole, i32 menuPathRole = Attributes::s_attributeMenuPathRole);

	i32                 GetColumnCount() const;
	bool                IsColumnVisible(i32 i) const;
	void                SetColumnVisible(i32 i, bool visible = true);
	
	virtual QVariantMap GetState() const override;
	virtual void        SetState(const QVariantMap& state) override;

	QAdvancedItemDelegate* GetAdvancedDelegate();

	void                BackupExpanded();
	void                RestoreExpanded();
	void				BackupSelection();
	void				RestoreSelection();

	void                TriggerRefreshHeaderColumns();

	//Utilities that may be useful outside of QAdvancedTreeView
	static QModelIndex ToOriginal(const QModelIndex& index, QAbstractItemModel* viewModel);
	static QModelIndex FromOriginal(const QModelIndex& index, QAbstractItemModel* viewModel);
	static void BackupSelection(std::vector<CAdvancedPersistentModelIndex>& selectedBackupStorage, QAbstractItemModel* viewModel, QItemSelectionModel* viewSelectionModel);
	static void RestoreSelection(std::vector<CAdvancedPersistentModelIndex>& selectedBackupStorage, QAbstractItemModel* viewModel, QItemSelectionModel* viewSelectionModel);

	CDrxSignal<void()> signalVisibleSectionCountChanged;

protected:
	virtual void startDrag(Qt::DropActions supportedActions) override;
	virtual void drawBranches(QPainter *painter,
		const QRect &rect,
		const QModelIndex &index) const override;
	virtual void drawBranchIndicator(QPainter *painter, const QStyleOptionViewItem *opt) const;
	virtual void mousePressEvent(QMouseEvent* pEvent) override;
	bool viewportEvent(QEvent *event);

	static constexpr i32 s_iconColumnWidth = 25;
	static constexpr i32 m_BranchIconSize = 16;

private:

	void        OnExpanded(const QModelIndex& index);
	void        OnCollapsed(const QModelIndex& index);
	void        OnContextMenu(const QPoint& point);

	void        OnModelAboutToBeReset();
	void        OnModelReset();
	void        OnColumnsInserted(const QModelIndex& parent, i32 first, i32 last);
	void        OnColumnsRemoved(const QModelIndex& parent, i32 first, i32 last);
	void        OnColumnsMoved(const QModelIndex& parent, i32 start, i32 end, const QModelIndex& destination, i32 column);
	void        OnHeaderSectionCountChanged(i32 oldCount, i32 newCount);

	void		UpdateColumnVisibility(bool reset = false);

	BehaviorFlags                              m_behavior;
	QSet<QPersistentModelIndex>                m_expanded;
	QPersistentModelIndex                      m_hoveredOver;

	std::vector<CAdvancedPersistentModelIndex> m_expandedBackup;
	std::vector<CAdvancedPersistentModelIndex> m_selectedBackup;
	std::vector<std::pair<QVariant, bool>>     m_columnVisible;

	i32 m_attributeRole;
	i32 m_attributeMenuPathRole;

	// column state merging map, to keep states for columns not present in all models
	QVariantMap m_columnStateMap;
	i32k m_layoutVersion;
};

