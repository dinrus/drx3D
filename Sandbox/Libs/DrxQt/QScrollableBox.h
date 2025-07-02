// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "DrxQtAPI.h"
#include "DrxQtCompatibility.h"
#include <QWidget>

class QScrollArea;
class QVBoxLayout;

class DRXQT_API QScrollableBox : public QWidget
{
	Q_OBJECT
public:
	QScrollableBox(QWidget* parent = nullptr);
	virtual ~QScrollableBox() {}
	void addWidget(QWidget*);
	void removeWidget(QWidget*);
	void insertWidget(i32 i, QWidget*);
	void clearWidgets();
	i32 indexOf(QWidget*);

protected:
	QScrollArea* m_scrollArea;
	QVBoxLayout* m_layout;
};

