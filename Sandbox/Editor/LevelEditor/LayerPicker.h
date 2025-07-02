// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "Controls/EditorDialog.h"

class QAdvancedTreeView;
class CObjectLayer;

//TODO : does not allow to select folders.
class CLayerPicker : public CEditorDialog
{
public:
	CLayerPicker();

	void          SetSelectedLayer(CObjectLayer* layer);
	CObjectLayer* GetSelectedLayer() const { return m_selectedLayer; }

protected:
	virtual QSize sizeHint() const override { return QSize(400, 500); }

private:

	void OnOk();
	void SelectLayerIndex(const QModelIndex& index);

	CObjectLayer*      m_selectedLayer;
	QAdvancedTreeView* m_treeView;
};

