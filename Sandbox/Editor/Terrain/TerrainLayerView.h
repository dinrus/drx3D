// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "QAdvancedTreeView.h"

class QTerrainLayerView : public QAdvancedTreeView
{
	Q_OBJECT
public:
	QTerrainLayerView(CTerrainManager* pTerrainManager);
	virtual ~QTerrainLayerView() override;

protected:
	void         selectRow(i32 row);
	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
	virtual void customEvent(QEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;

private:
	void SelectedLayerChanged(class CLayer* pLayer);
	void LayersChanged();

	class QAbstractTableModel* m_pModel;
	class CTerrainManager*     m_pTerrainManager;
	bool                       m_selecting;
};


