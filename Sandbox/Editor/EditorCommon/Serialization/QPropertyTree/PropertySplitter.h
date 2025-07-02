// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>

class QPropertySplitter : public QWidget
{
	Q_OBJECT
public:
	explicit QPropertySplitter(QWidget* parent = 0)
		: QWidget(parent)
	{
	}

	virtual ~QPropertySplitter() override {}
};


