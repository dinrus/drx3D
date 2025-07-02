// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QLayout>
#include <QRect>

#include "PropertyTreeModel.h"
#include "PropertyTree2.h"

namespace PropertyTree2
{
	//This widget container is in charge of laying out the widgets in the tree and drawing the extra elements
	//This also handles interaction with the tree's UI elements (expanding/collapsing, slider...)
	class CFormWidget : public QWidget
	{
		Q_OBJECT

		//Styling properties
		Q_PROPERTY(i32 nesting READ GetNesting)
		Q_PROPERTY(i32 spacing READ GetSpacing WRITE SetSpacing DESIGNABLE true)
		Q_PROPERTY(i32 minRowHeight READ GetMinRowHeight WRITE SetMinRowHeight DESIGNABLE true)
		Q_PROPERTY(i32 iconSize READ GetIconSize WRITE SetIconSize DESIGNABLE true)
		Q_PROPERTY(i32 nestingIndent READ GetNestingIndent WRITE SetNestingIndent DESIGNABLE true)
		Q_PROPERTY(i32 groupBorderWidth READ GetGroupBorderWidth WRITE SetGroupBorderWidth DESIGNABLE true)
		Q_PROPERTY(QColor groupBorderColor READ GetGroupBorderColor WRITE SetGroupBorderColor DESIGNABLE true)
		Q_PROPERTY(QRect margins READ GetMargins WRITE SetMargins DESIGNABLE true)
		
	public:
		CFormWidget(QPropertyTree2* parentTree, const CRowModel* row, i32 nesting);
		~CFormWidget();

		//Styling
		i32 GetNesting() const { return m_nesting; }

		i32 GetNestingIndent() const { return m_nestingIndent; }
		void SetNestingIndent(i32 value);

		i32 GetSpacing() const { return m_spacing; }
		void SetSpacing(i32 value);

		i32 GetMinRowHeight() const { return m_minRowHeight; }
		void SetMinRowHeight(i32 value);

		i32 GetIconSize() const { return m_iconSize; }
		void SetIconSize(i32 value);

		i32 GetGroupBorderWidth() const { return m_groupBorderWidth; }
		void SetGroupBorderWidth(i32 value);

		QColor GetGroupBorderColor() const { return m_groupBorderColor; }
		void SetGroupBorderColor(const QColor& color);

		const CRowModel* GetRowModel() const { return m_rowModel; }
		void OnActiveRowChanged(const CRowModel* newActiveRow);

		QRect GetMargins() const;
		void SetMargins(const QRect& margins);

		void OnSplitterPosChanged();

		//! Expands a row in this form's scope. Returns the formWidget child of the row
		CFormWidget* ExpandRow(const CRowModel* row, bool expand);

		//! Recursively expands to the specified row. Returns the formWidget that contains the row
		CFormWidget* ExpandToRow(const CRowModel* row);

		//! Scrolls the parent scroll area to make this row visible
		void ScrollToRow(const CRowModel* row);

		//Updates the tree if the model is marked dirty
		void UpdateTree();

	private:
		void paintEvent(QPaintEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		bool event(QEvent* ev) override;
		void mousePressEvent(QMouseEvent* ev) override;
		void mouseReleaseEvent(QMouseEvent* ev) override;
		void mouseMoveEvent(QMouseEvent* ev) override;
		void enterEvent(QEvent* ev) override;
		void leaveEvent(QEvent* ev) override;
		void contextMenuEvent(QContextMenuEvent* event) override;
		void dragEnterEvent(QDragEnterEvent* ev) override;
		void dragMoveEvent(QDragMoveEvent* ev) override;
		void dragLeaveEvent(QDragLeaveEvent* ev) override;
		void dropEvent(QDropEvent* ev) override;
		void keyPressEvent(QKeyEvent* ev) override;

		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;
		bool focusNextPrevChild(bool next) override;

		void SetupWidgets();
		void ReleaseWidgets();

		void UpdateMinimumSize();
		void DoLayout(const QSize& rect);
		void DoFullLayoutUpdate();

		void OnCopy();
		void OnPaste();

		struct FormRow
		{
			FormRow(const CRowModel* rowModel);
			FormRow(FormRow&& other);
			FormRow(const FormRow& other) = delete;
			~FormRow();

			FormRow& operator=(FormRow&& other);
			FormRow& operator=(const FormRow& other) = delete;

			_smart_ptr<const CRowModel> model;
			QWidget* widget; //Widget that is actually displayed on this row, may differ from model's widget due to inlining behavior
			CFormWidget* childContainer;

			//Cached rowRect for paint event (paint labels, hover highlight and so on)
			QRect rowRect;

			bool IsExpanded() const;
			QWidget* GetQWidget();
			IPropertyTreeWidget* GetPropertyTreeWidget();
			bool HasChildren() const;
		};

		i32 GetSplitterPosition() const;
		FormRow* ModelToFormRow(const CRowModel* row);
		FormRow* RowAtPos(const QPoint& position);
		i32 InsertionIndexAtPos(const QPoint& position, i32 tolerancePx = 5);
		i32 RowIndex(const FormRow* row);

		void UpdateActiveRow(const QPoint& cursorPos);
		void SetActiveRow(FormRow* row);
		QRect GetExpandIconRect(const FormRow& row) const;
		QRect GetDragHandleIconRect(const FormRow& row) const;

		void ToggleExpand(FormRow& row, bool skipLayout = false);

		//Invariants
		_smart_ptr<const CRowModel> m_rowModel;
		QPropertyTree2* m_parentTree;
		i32k m_nesting;

		//Internals
		std::vector<FormRow> m_rows;
		FormRow* m_activeRow;
		QPoint m_lastCursorPos; //Last position tested for hovering. Used to avoid unnecessary computing.
		QPoint m_mouseDownPos;
		bool m_allowReorderChildren;
		i32 m_dragInsertIndex; //-2 = not dragging at all, -1 = dragging but invalid index, >0 = valid index

		//Style props
		i32 m_spacing;
		i32 m_minRowHeight;
		i32 m_iconSize;
		i32 m_nestingIndent;
		i32 m_groupBorderWidth;
		QColor m_groupBorderColor;
	};

	class CInlineWidgetBox : public QWidget
	{
		Q_OBJECT
	public:
		CInlineWidgetBox(CFormWidget* parent);

		void AddWidget(QWidget* widget, QPropertyTree2* pTree);
		void ReleaseWidgets(QPropertyTree2* pTree);
	};
}




