// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QWidget>
#include <QVariant>

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

class QLineEdit;

//! Replacement for QComboBox. Internally uses popup QMenu.
//! Meant to provide a better widget to subclass than QComboBox due to its incoherent behavior
//! Handles multi-selection, empty entries, and can be extended for more (editable, submenus...)
class EDITOR_COMMON_API QMenuComboBox : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(QString currentText READ GetText WRITE SetText USER true)
	//! The icon size and margin used to place the icon of the line edit action
	//! is hard coded in the Qt line edit code and cannot be retrieved from the line
	//! edit. Therefore this property is used to implement the additional offset
	//! that is needed to determine the sizeHint of the widget which consists of
	//! the text size of the menu entries and the icon size + icon margin
	Q_PROPERTY(i32 widthOffset READ GetWidthOffset WRITE SetWidthOffset)

public:
	QMenuComboBox(QWidget* parent = nullptr);
	~QMenuComboBox();

	void ShowPopup();

	//Content
	void AddItem(const QString& str, const QVariant& userData = QVariant());
	void AddItems(const QStringList& items);

	void RemoveItem(i32 index);

	//! Removes all items and resets to default state
	void     Clear();

	i32      GetItemCount() const { return m_data.length(); }

	QVariant GetData(i32 index) const;
	QString  GetItem(i32 index) const;

	//! Returns the text that can be seen in the Edit part of the ComboBox
	QString GetText() const;

	//! Single-select: Returns the userData of the selected item, Multi-Select: Returns empty QVariant
	QVariant GetCurrentData() const;

	//! Returns the width offset applied to the combobox overall width
	i32 GetWidthOffset() const;

	//! Single-select : must match one of the items, Multi-select: values must be separated by the separator, default is ", "
	void SetText(const QString& text, tukk separator = ", ");

	//Checked state
	void SetChecked(i32 index, bool checked = true);

	//! Checks the item that matches the string. Only the first match will be selected
	void SetChecked(const QString& str, bool checked = true);

	//! Checks the item that matches the user data. Only the first match will be selected
	void SetCheckedByData(const QVariant& userData, bool checked = true);

	//! Single-select: can only handle one element in the list, Multi-select: only checks entries present in the list. Entries not found are ignored, signals are not fired
	void SetChecked(const QStringList& items);

	//! Sets the width offset applied to the combobox overall width
	void SetWidthOffset(i32 widthOffset);

	//! Sets the item tool tip
	void SetItemToolTip(i32 index, const QString& toolTip);

	//! Sets the item text
	void SetItemText(i32 index, const QString& text);

	//! Checks if first item matching the string is selected
	bool        IsChecked(const QString& str);

	bool        IsChecked(i32 index) const;

	QList<i32>  GetCheckedIndices() const;
	QStringList GetCheckedItems() const;

	//! Single-select: returns currently selected item or -1, Multi-select: returns -1
	i32  GetCheckedItem() const;

	void UncheckAllItems();

	//Internal state

	//! Use before adding items, Default is false
	void SetMultiSelect(bool multiSelect) { m_multiSelect = multiSelect;  }

	//! Use before adding items, Default is false
	void SetCanHaveEmptySelection(bool emptySelect) { m_emptySelect = emptySelect; }

	//! Only called for single-select combo boxes when the currently selected item changes
	CDrxSignal<void(i32)> signalCurrentIndexChanged;

	//! Only called for multi-select combo boxes when the check state of an item changes
	CDrxSignal<void(i32, bool)> signalItemChecked;

	//! QWidget interface
protected:
	virtual QSize sizeHint() const override;
	virtual QSize minimumSizeHint() const override;
	//! paintEvent is needed to make custom widget styleable. Otherwise
	//! only properties provided by QWidget would be styleable.
	//! http://doc.qt.io/qt-5/stylesheet-reference.html#list-of-stylable-widgets
	virtual void paintEvent(QPaintEvent*) override;
	virtual void changeEvent(QEvent*) override;
	virtual void focusOutEvent(QFocusEvent*) override;
	virtual void focusInEvent(QFocusEvent*) override;
	virtual bool event(QEvent*) override;

private:
	class Popup;
	struct MenuAction;

	struct ItemData
	{
		QString     text;
		QVariant    userData;
		MenuAction* action;
		QString     toolTip;
	};

	bool eventFilter(QObject* object, QEvent* event) override;

	void InternalSetChecked(i32 index, bool checked = true);
	void HidePopup();
	void UpdateText();
	void ApplyDelta(i32 delta);

	void OnToggled(i32 index, bool checked);

	void UpdateSizeHint(const QString& entry);
	void RecalculateSizeHint();

	QVector<ItemData> m_data;
	QLineEdit*        m_lineEdit;
	Popup*            m_popup;
	i32               m_lastSelected;
	bool              m_multiSelect;
	bool              m_emptySelect;
	QSize             m_sizeHint;
	i32               m_widthOffset;
};

