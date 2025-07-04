// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QWidget>

#pragma once

class QMenuComboBox;
class QLineEdit;

//TODO : Unify with QMenuComboBox by adding editable functionnality there
class EDITOR_COMMON_API QEditableComboBox : public QWidget
{
	Q_OBJECT
public:
	QEditableComboBox(QWidget* pParent = nullptr);

	QString GetCurrentText() const;

	void    ClearItems();
	void    AddItem(const QString& itemName);
	void    AddItems(const QStringList& itemNames);
	void    RemoveCurrentItem();

	void    SetCurrentItem(const QString& itemName);

public Q_SLOTS:
	void OnBeginEditing();
	void OnEditingFinished();
	void OnEditingCancelled();

Q_SIGNALS:
	void ItemRenamed(const QString& before, const QString& after);
	void TextChanged();
	void OnCurrentIndexChanged(i32 index);

protected:
	bool eventFilter(QObject* obj, QEvent* event);

protected:
	QMenuComboBox* m_pComboBox;
	QLineEdit* m_pLineEdit;
};

