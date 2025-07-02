// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>
#include <QAdvancedTreeView.h>
#include <QWidget>

// Forward declare classes.
class QBoxLayout;
class QItemSelection;
class QLineEdit;
class QMenu;
class QPushButton;
class QSplitter;

namespace Schematyc
{
	// Forward declare classes.
	class CEnvBrowserFilter;

	struct EEnvBrowserColumn
	{
		enum : i32
		{
			Name = 0,
			Count
		};
	};

	class CEnvBrowserItem;

	DECLARE_SHARED_POINTERS(CEnvBrowserItem)

	typedef std::vector<CEnvBrowserItemPtr> EnvBrowserItems;

	class CEnvBrowserItem
	{
	public:

		CEnvBrowserItem(const DrxGUID& guid, tukk szName, tukk szIcon);

		DrxGUID GetGUID() const;
		tukk GetName() const;
		tukk GetIcon() const;

		CEnvBrowserItem* GetParent();
		void AddChild(const CEnvBrowserItemPtr& pChild);
		void RemoveChild(CEnvBrowserItem* pChild);
		i32 GetChildCount() const;
		i32 GetChildIdx(CEnvBrowserItem* pChild);
		CEnvBrowserItem* GetChildByIdx(i32 childIdx);

	private:

		DrxGUID            m_guid;
		string           m_name;
		string           m_iconName;
		CEnvBrowserItem* m_pParent;
		EnvBrowserItems  m_children;
	};

	class CEnvBrowserModel : public QAbstractItemModel
	{
		Q_OBJECT

	private:

		typedef std::unordered_map<DrxGUID, CEnvBrowserItem*> ItemsByGUID;

	public:

		CEnvBrowserModel(QObject* pParent);

		// QAbstractItemModel
		QModelIndex index(i32 row, i32 column, const QModelIndex& parent) const override;
		QModelIndex parent(const QModelIndex& index) const override;
		i32 rowCount(const QModelIndex& index) const override;
		i32 columnCount(const QModelIndex& parent) const override;
		bool hasChildren(const QModelIndex &parent) const override;
		QVariant data(const QModelIndex& index, i32 role) const override;
		bool setData(const QModelIndex &index, const QVariant &value, i32 role) override;
		QVariant headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
		Qt::ItemFlags flags(const QModelIndex& index ) const override;
		// ~QAbstractItemModel

		QModelIndex ItemToIndex(CEnvBrowserItem* pItem, i32 column = 0) const;
		CEnvBrowserItem* ItemFromIndex(const QModelIndex& index) const;
		CEnvBrowserItem* ItemFromGUID(const DrxGUID& guid) const;

	public slots:

	private:

		void Populate();

	private:

		EnvBrowserItems m_items;
		ItemsByGUID     m_itemsByGUID;
	};

	class CEnvBrowserWidget : public QWidget
	{
		Q_OBJECT

	public:

		CEnvBrowserWidget(QWidget* pParent);

		~CEnvBrowserWidget();

		void InitLayout();

	public slots:

		void OnSearchFilterChanged(const QString& text);
		void OnTreeViewCustomContextMenuRequested(const QPoint& position);

	private:

		void ExpandAll();

	private:

		QBoxLayout*        m_pMainLayout;
		QBoxLayout*        m_pFilterLayout;
		QLineEdit*         m_pSearchFilter;
		QAdvancedTreeView* m_pTreeView;
		CEnvBrowserModel*  m_pModel;
		CEnvBrowserFilter* m_pFilter;
	};
}

