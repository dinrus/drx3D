// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

class QListView;

#include "ProxyModels/ItemModelAttribute.h"
#include <EditorFramework/StateSerializable.h>
#include "QAdvancedTreeView.h"

//! Extends and specializes a QListView to behave as a thumbnail view (see windows explorer for inspiration)
//! All items have the same size, default is 64x64 pixels. Set ItemSize and Spacing, the rest should work out of the box
//! For convenience use the default Thumbnail attribute to create a specific Thumbnail column
//! Will use s_ThumbnailRole in priority, or Qt::DecorationRole as backup for the icon.
class EDITOR_COMMON_API QThumbnailsView : public QWidget, public IStateSerializable
{
	Q_OBJECT;
public:
	//! Pass an internal view to be used as the list view or let it be set as a regular QListView();
	QThumbnailsView(QListView* pInternalView = nullptr, bool showSizeButtons = true, QWidget* parent = nullptr);
	~QThumbnailsView();

	bool         SetItemSize(const QSize& size);
	inline QSize GetItemSize() const { return m_itemSize; }

	void         SetItemSizeBounds(const QSize& min, const QSize& max);
	void         GetItemSizeBounds(QSize& minSizeOut, QSize& maxSizeOut) const { minSizeOut = m_minItemSize; maxSizeOut = m_maxItemSize; }

	void         SetSpacing(i32 space);
	i32          GetSpacing() const;

	void         SetRootIndex(const QModelIndex& index);
	QModelIndex  GetRootIndex() const;

	//! column that will be used to query data to the model
	void SetDataColumn(i32 column);
	//! Returns false if attribute not found
	bool SetDataAttribute(const CItemModelAttribute* attribute = & Attributes::s_thumbnailAttribute, i32 attributeRole = Attributes::s_getAttributeRole);

	//! When set, the selection will be restored after model reset, based on persistent model indices and unique id of each index from the original model. Default is true.
	void SetRestoreSelectionAfterReset(bool bRestore) { m_restoreSelection = bRestore; }

	void SetModel(QAbstractItemModel* model);

	//! Helper method for scrolling as the ListView method has non-obvious constraints
	void ScrollToRow(const QModelIndex& indexInRow, QAbstractItemView::ScrollHint scrollHint = QAbstractItemView::EnsureVisible);

	//! Use to configure the internal view if necessary
	QAbstractItemView* GetInternalView() { return m_listView; }

	void AppendPreviewSizeActions(CAbstractMenu& menu);

	CDrxSignal<void()> signalShowContextMenu;

	//! This is the role used to return thumbnail status:
	//! Return false to signify the thumbnail is loading.
	//! Return QIcon for the actual thumbnail once it is loaded.
	static constexpr i32 s_ThumbnailRole = Qt::UserRole + 2562;

	virtual QVariantMap GetState() const override;
	virtual void        SetState(const QVariantMap& state) override;

private:

	class ListView;
	class Delegate;

	bool eventFilter(QObject* pObject, QEvent* pEvent) override;
	void customEvent(QEvent* pEvent) override;

	void OnModelAboutToBeReset();
	void OnModelReset();
	void OnPreviewSizeButtonClicked(i32 value);
	void StartUpdateTimer();

	void AddSizingButton(i32 size, i32 iconNumber);

	// Save persistent personalization properties
	void SaveState() const;

	Delegate*         m_pDelegate;
	QSize             m_minItemSize;
	QSize             m_maxItemSize;
	QSize             m_itemSize;
	QTimer            m_timer;
	QListView*        m_listView;
	QPrecisionSlider* m_slider;
	QButtonGroup*     m_previewSizeButtons;
	i32k         m_numOfPreviewSizes = 4;
	bool              m_timerStarted;
	bool			  m_restoreSelection;
	std::vector<CAdvancedPersistentModelIndex> m_selectedBackup;
};

