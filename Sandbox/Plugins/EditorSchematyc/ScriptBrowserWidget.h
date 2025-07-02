// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>
#include <QWidget>
#include <drx3D/CoreX/Serialization/Forward.h>
#include <DrxSchematyc/Script/IScriptRegistry.h>
#include <DrxSchematyc/Utils/EnumFlags.h>
#include <DrxSchematyc/Utils/ScopedConnection.h>
#include <DrxSchematyc/Utils/Signal.h>

#include <ProxyModels/ItemModelAttribute.h>

#include "ScriptBrowserUtils.h"

// Forward declare classes.
class QBoxLayout;
class QItemSelection;
class QLineEdit;
class QMenu;
class QPushButton;
class QSplitter;
class QToolBar;
class QSearchBox;
class QAdvancedTreeView;
class CAsset;

namespace DrxSchematycEditor {
class CMainWindow;
}

namespace Schematyc
{
// Forward declare structures.
struct SSourceControlStatus;
// Forward declare classes.
class CScriptElementFilterProxyModel;
class CScriptBrowserItem;
class CSourceControlManager;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(CScriptBrowserItem)

struct EScriptBrowserColumn
{
	enum : i32
	{
		Name = 0,
		Sort,
		Filter,

		COUNT
	};
};

enum class EScriptBrowserItemType
{
	None,
	Root,
	Filter,
	ScriptElement
};

enum class EScriptBrowserItemFlags
{
	None                  = 0,

	Modified              = BIT(0),   // #SchematycTODO : Should this be a flag on the script element itself? Yes!

	FileLocal             = BIT(1),
	FileInPak             = BIT(2),
	FileReadOnly          = BIT(3),
	FileManaged           = BIT(4),
	FileCheckedOut        = BIT(5),
	FileAdd               = BIT(6),

	ContainsWarnings      = BIT(7),
	ContainsErrors        = BIT(8),
	ChildContainsWarnings = BIT(9),
	ChildContainsErrors   = BIT(10),

	FileMask              = FileLocal | FileInPak | FileReadOnly | FileManaged | FileCheckedOut | FileAdd,
	ChildMask             = ChildContainsWarnings | ChildContainsErrors
};

typedef CEnumFlags<EScriptBrowserItemFlags> ScriptBrowserItemFlags;

class CScriptBrowserItem
{
	typedef std::vector<CScriptBrowserItemPtr> Items;

public:
	CScriptBrowserItem(EScriptBrowserItemType type, tukk szName, tukk szIcon);
	CScriptBrowserItem(EScriptBrowserItemType type, tukk szName, EFilterType filterType);
	CScriptBrowserItem(EScriptBrowserItemType type, tukk szName, tukk szIcon, IScriptElement* pScriptElement);

	EScriptBrowserItemType GetType() const;

	void                   SetName(tukk szName);
	tukk            GetName() const;
	tukk            GetPath() const;
	tukk            GetIcon() const;
	tukk            GetTooltip() const;
	tukk            GetSortString() const;
	void                   SetSortString(tukk szSort);

	tukk            GetFilterString() const;
	void                   SetFilterString(tukk szFilter);

	void                   SetFlags(const ScriptBrowserItemFlags& flags);
	ScriptBrowserItemFlags GetFlags() const;
	EFilterType            GetFilterType() const;
	void                   SetFilterType(EFilterType filterType);
	tukk            GetFileText() const;
	tukk            GetFileIcon() const;
	QVariant               GetColor() const;

	IScriptElement*        GetScriptElement() const;
	IScript*               GetScript() const;

	CScriptBrowserItem*    GetParent();
	void                   AddChild(const CScriptBrowserItemPtr& pChild);
	CScriptBrowserItemPtr  RemoveChild(CScriptBrowserItem* pChild);
	i32                    GetChildCount() const;
	i32                    GetChildIdx(CScriptBrowserItem* pChild);
	CScriptBrowserItem*    GetChildByIdx(i32 childIdx);
	CScriptBrowserItem*    GetChildByTypeAndName(EScriptBrowserItemType type, tukk szName);

	void                   Validate();
	void                   RefreshChildFlags();

private:
	EScriptBrowserItemType m_type;
	string                 m_name;
	string                 m_path;
	string                 m_iconName;
	string                 m_tooltip;
	string                 m_sortString;
	string                 m_filterString;
	ScriptBrowserItemFlags m_flags;
	EFilterType            m_filterType;
	IScriptElement*        m_pScriptElement;
	CScriptBrowserItem*    m_pParent;
	Items                  m_children;
};

struct SFilterAttributes;

class CScriptBrowserModel : public QAbstractItemModel
{
	Q_OBJECT

	typedef std::unordered_map<DrxGUID, CScriptBrowserItem*> ItemsByGUID;
	typedef std::map<string, CScriptBrowserItem*>            ItemsByFileName;

public:
	enum ERole : i32
	{
		DisplayRole    = Qt::DisplayRole,
		DecorationRole = Qt::DecorationRole,
		EditRole       = Qt::EditRole,
		ToolTipRole    = Qt::ToolTipRole,
		ForegroundRole = Qt::ForegroundRole,
		SizeHintRole   = Qt::SizeHintRole,
		PointerRole    = Qt::UserRole
	};

public:
	CScriptBrowserModel(DrxSchematycEditor::CMainWindow& editor, CAsset& asset, const DrxGUID& assetGUID);
	~CScriptBrowserModel();

	// QAbstractItemModel
	QModelIndex   index(i32 row, i32 column, const QModelIndex& parent) const override;
	QModelIndex   parent(const QModelIndex& index) const override;
	i32           rowCount(const QModelIndex& index) const override;
	i32           columnCount(const QModelIndex& parent) const override;
	bool          hasChildren(const QModelIndex& parent) const override;
	QVariant      data(const QModelIndex& index, i32 role) const override;
	bool          setData(const QModelIndex& index, const QVariant& value, i32 role) override;
	QVariant      headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	// ~QAbstractItemModel

	QVariant                   GetHeaderData(i32 section, Qt::Orientation orientation, i32 role) const;
	CAsset&                    GetAsset() const { return m_asset; }

	void                       Reset();

	QModelIndex                ItemToIndex(CScriptBrowserItem* pItem, i32 column = 0) const;
	CScriptBrowserItem*        ItemFromIndex(const QModelIndex& index) const;

	CScriptBrowserItem*        GetRootItem();
	CScriptBrowserItem*        GetItemByGUID(const DrxGUID& guid);
	CScriptBrowserItem*        GetItemByFileName(tukk szFileName);

	Schematyc::IScriptElement* GetRootElement();

	bool                       HasUnsavedChanged();

private:
	void                  Populate();

	void                  OnScriptRegistryChange(const SScriptRegistryChange& change);
	void                  OnScriptElementAdded(IScriptElement& scriptElement);
	void                  OnScriptElementModified(IScriptElement& scriptElement);
	void                  OnScriptElementRemoved(IScriptElement& scriptElement);
	void                  OnScriptElementSaved(IScriptElement& scriptElement);

	CScriptBrowserItem*   CreateScriptElementItem(IScriptElement& scriptElement, const ScriptBrowserItemFlags& flags, CScriptBrowserItem* pParentItem);
	CScriptBrowserItem*   CreateScriptElementBaseItem(IScriptElement& scriptElement, const ScriptBrowserItemFlags& flags, CScriptBrowserItem* pParentItem, CScriptBrowserItem* pBaseItem);

	CScriptBrowserItem*   AddItem(CScriptBrowserItem& parentItem, const CScriptBrowserItemPtr& pItem);
	CScriptBrowserItemPtr RemoveItem(CScriptBrowserItem& item);

	void                  ValidateItem(CScriptBrowserItem& item);

	CScriptBrowserItem*   GetFilter(tukk szName) const;
	CScriptBrowserItem*   CreateFilter(const SFilterAttributes& filterAttributes);

private:
	static const CItemModelAttribute   s_columnAttributes[EScriptBrowserColumn::COUNT];

	DrxSchematycEditor::CMainWindow&   m_editor;
	CScriptBrowserItemPtr              m_pRootItem;
	ItemsByGUID                        m_itemsByGUID;
	ItemsByFileName                    m_itemsByFileName;

	CConnectionScope                   m_connectionScope;
	const DrxGUID                      m_assetGUID;
	CAsset&                            m_asset;

	std::vector<CScriptBrowserItemPtr> m_filterItems;
};

struct SScriptBrowserSelection
{
	SScriptBrowserSelection(IScriptElement* _pScriptElement);

	IScriptElement* pScriptElement;
};

typedef CSignal<void (const SScriptBrowserSelection&)> ScriptBrowserSelectionSignal;

class CScriptBrowserWidget : public QWidget
{
	Q_OBJECT

	struct SSignals
	{
		ScriptBrowserSelectionSignal selection;
	};

public:
	CScriptBrowserWidget(DrxSchematycEditor::CMainWindow& editor);
	~CScriptBrowserWidget();

	void                                 InitLayout();
	void                                 SelectItem(const DrxGUID& guid);
	DrxGUID                              GetSelectedItemGUID() const;
	bool                                 SetModel(CScriptBrowserModel* pModel);
	void                                 Serialize(Serialization::IArchive& archive);
	ScriptBrowserSelectionSignal::Slots& GetSelectionSignalSlots();

	bool                                 HasScriptUnsavedChanges() const;

public slots:
	void OnFiltered();
	void OnTreeViewClicked(const QModelIndex& index);
	void OnTreeViewCustomContextMenuRequested(const QPoint& position);
	void OnTreeViewKeyPress(QKeyEvent* pKeyEvent, bool& bEventHandled);
	void OnFindReferences();
	void OnAddItem(IScriptElement* pScriptElement, EScriptElementType elementType);
	void OnCopyItem();
	void OnPasteItem();
	void OnRemoveItem();
	void OnRenameItem();
	void OnSave();
	void OnExtract();

Q_SIGNALS:
	void OnScriptElementRemoved(Schematyc::IScriptElement& sriptElement);

protected:
	virtual void keyPressEvent(QKeyEvent* pKeyEvent) override;
	virtual void showEvent(QShowEvent* pEvent) override;

private:
	void        ExpandAll();
	void        PopulateAddMenu(QMenu* pMenu, IScriptElement* pScriptScope);
	void        PopulateFilterMenu(QMenu* pMenu, EFilterType filterType);
	bool        HandleKeyPress(QKeyEvent* pKeyEvent);

	QModelIndex GetTreeViewSelection() const;
	QModelIndex TreeViewToModelIndex(const QModelIndex& index) const;
	QModelIndex TreeViewFromModelIndex(const QModelIndex& index) const;

private:
	DrxSchematycEditor::CMainWindow& m_editor;
	QBoxLayout*                      m_pMainLayout;
	QBoxLayout*                      m_pHeaderLayout;
	QSearchBox*                      m_pFilter;

	QPushButton*                     m_pAddButton;
	QMenu*                           m_pAddMenu;
	QMenu*                           m_pContextMenu;
	QAdvancedTreeView*               m_pTreeView;
	CScriptBrowserModel*             m_pModel;
	CScriptElementFilterProxyModel*  m_pFilterProxy;

	SSignals                         m_signals;
	CAsset*                          m_pAsset;
};
} // Schematyc

