// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QWidget>
#include "EditToolPanel.h"
#include "TerrainBrushTool.h"

class QTerrainSculptButtons : public QWidget
{
public:
	QTerrainSculptButtons(QWidget* parent = nullptr);

private:
	void AddTool(CRuntimeClass* pRuntimeClass, tukk text);

	i32           m_buttonCount;
	CTerrainBrush mTerrainBrush;
};

class QTerrainSculptPanel : public QEditToolPanel
{
public:
	QTerrainSculptPanel(QWidget* parent = nullptr);

protected:
	virtual bool CanEditTool(CEditTool* pTool);
};

