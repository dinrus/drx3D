// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "QAdvancedTreeView.h"

class CVegetationTreeView : public QAdvancedTreeView
{
	Q_OBJECT

public:
	explicit CVegetationTreeView(QWidget* pParent = nullptr);

	virtual void startDrag(Qt::DropActions supportedActions) override;

signals:
	void dragStarted(const QModelIndexList& dragRows);
};

