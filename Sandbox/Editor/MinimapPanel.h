// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QWidget>
#include "EditToolPanel.h"

class QMinimapButtons : public QWidget
{
public:
	QMinimapButtons(QWidget* parent = nullptr);

private:
	void AddTool(CRuntimeClass* pRuntimeClass, tukk text);
};

class QMinimapPanel : public QEditToolPanel
{
public:
	QMinimapPanel(QWidget* parent = nullptr);

protected:
	virtual bool CanEditTool(CEditTool* pTool);
};

