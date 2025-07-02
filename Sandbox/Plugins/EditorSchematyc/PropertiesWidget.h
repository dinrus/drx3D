// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DetailWidget.h"

#include "PreviewWidget.h"

#include <NodeGraph/IDrxGraphEditor.h>
#include <drx3D/CoreX/Serialization/IArchive.h>

#include <QWidget>

class QAdvancedPropertyTree;

namespace DrxSchematycEditor {

class CComponentItem;
class CAbstractObjectStructureModelItem;
class CAbstractVariablesModelItem;
class CMainWindow;

class CPropertiesWidget : public QWidget
{
	Q_OBJECT

public:
	CPropertiesWidget(CComponentItem& item, CMainWindow* pEditor);
	CPropertiesWidget(CAbstractObjectStructureModelItem& item, CMainWindow* pEditor);
	CPropertiesWidget(CAbstractVariablesModelItem& item, CMainWindow* pEditor);
	CPropertiesWidget(DrxGraphEditor::GraphItemSet& items, CMainWindow* pEditor);

	// TEMP
	CPropertiesWidget(IDetailItem& item, CMainWindow* pEditor, Schematyc::CPreviewWidget* pPreview = nullptr);
	// ~TEMP

	virtual ~CPropertiesWidget() override;

	virtual void showEvent(QShowEvent* pEvent) override;

	void         OnContentDeleted(DrxGraphEditor::CAbstractNodeGraphViewModelItem* deletedItem);

Q_SIGNALS:
	void SignalPropertyChanged();

protected:
	void SetupTree();
	void OnPropertiesChanged();
	void OnPreviewChanged();
	void OnPushUndo();

protected:
	QAdvancedPropertyTree*       m_pPropertyTree;
	Serialization::SStructs      m_structs;
	Serialization::CContextList* m_pContextList;

	bool                         m_isPushingUndo;

	// TEMP
	CMainWindow*                 m_pEditor;
	Schematyc::CPreviewWidget*   m_pPreview;
	std::unique_ptr<IDetailItem> m_pDetailItem;
	//~TEMP
};

}

