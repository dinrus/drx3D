// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>

class QPropertyGroupBox : public QWidget
{
	Q_OBJECT
public:
	explicit QPropertyGroupBox(QWidget* parent = 0)
		: QWidget(parent)
	{
	}

	virtual ~QPropertyGroupBox() override {}
};

class QPropertyGroupHeader : public QWidget
{
	Q_OBJECT
public:
	explicit QPropertyGroupHeader(QWidget* parent = 0)
		: QWidget(parent)
	{
	}

	virtual ~QPropertyGroupHeader() override {}
};


