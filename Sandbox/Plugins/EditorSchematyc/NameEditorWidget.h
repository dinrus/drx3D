// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QLineEdit>

namespace DrxSchematycEditor {

template<typename TItem>
class CNameEditor : public QLineEdit
{
public:
	CNameEditor(TItem& item, QWidget* pParent = nullptr)
		: QLineEdit(pParent)
		, m_item(item)
	{
		setFrame(true);
	}

	virtual void focusInEvent(QFocusEvent* pEvent) override
	{
		setText(m_item.GetName());
		selectAll();
		QLineEdit::focusInEvent(pEvent);
	}

private:
	TItem& m_item;
};

}

