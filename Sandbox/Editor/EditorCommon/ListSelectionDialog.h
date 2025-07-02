// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include "Controls/EditorDialog.h"

class QDeepFilterProxyModel;
class QLineEdit;
class QModelIndex;
class QStandardItemModel;
class QStandardItem;
class QString;
class QAdvancedTreeView;
class QWidget;

//This is mostly used by the character tool, evaluate usability before reusing
class EDITOR_COMMON_API ListSelectionDialog : public CEditorDialog
{
	Q_OBJECT
public:
	ListSelectionDialog(const QString& dialogNameId, QWidget* parent);
	void        SetColumnText(i32 column, tukk text);
	void        SetColumnWidth(i32 column, i32 width);

	void        AddRow(tukk firstColumnValue);
	void        AddRow(tukk firstColumnValue, QIcon& icon);
	void        AddRowColumn(tukk value);

	tukk ChooseItem(tukk currentValue);

	QSize       sizeHint() const override;
protected slots:
	void        onActivated(const QModelIndex& index);
	void        onFilterChanged(const QString&);
protected:
	bool        eventFilter(QObject* obj, QEvent* event);
private:
	struct less_str : public std::binary_function<STxt, STxt, bool>
	{
		bool operator()(const STxt& left, const STxt& right) const { return _stricmp(left.c_str(), right.c_str()) < 0; }
	};

	QAdvancedTreeView*     m_tree;
	QStandardItemModel*    m_model;
	QDeepFilterProxyModel* m_filterModel;
	typedef std::map<STxt, QStandardItem*, less_str> StringToItem;
	StringToItem           m_firstColumnToItem;
	QLineEdit*             m_filterEdit;
	STxt            m_chosenItem;
	i32                    m_currentColumn;
};

