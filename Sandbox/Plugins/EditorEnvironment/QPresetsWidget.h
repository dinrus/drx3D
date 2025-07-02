// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>
#include "PresetsModel.h"

class QAdvancedTreeView;
class QAction;
class QItemSelection;

class QPresetsWidget : public QWidget
{
	Q_OBJECT
public:
	QPresetsWidget();
	~QPresetsWidget();

	void               OnNewScene();
	void               SaveAllPresets() const;
	void               RefreshPresetList();

	CPresetsModelNode* GetRoot() const { return m_presetsViewRoot.get(); }
	void               SelectByNode(CPresetsModelNode* pEntryNode);

signals:
	void SignalCurrentPresetChanged();

	void SignalBeginAddEntryNode(CPresetsModelNode* pEntryNode);
	void SignalEndAddEntryNode(CPresetsModelNode* pEntryNode);
	void SignalBeginDeleteEntryNode(CPresetsModelNode* pEntryNode);
	void SignalEndDeleteEntryNode();
	void SignalBeginResetModel();
	void SignalEndResetModel();
	void SignalEntryNodeDataChanged(CPresetsModelNode* pEntryNode) const;

protected:
	class CPresetChangedUndoCommand;
	class CPresetRemovedUndoCommand;
	class CPresetResetUndoCommand;
	class CPresetAddUndoCommand;
	class CPresetAddExistingUndoCommand;

	void        CreateUi();
	void        CreateActions();

	void        AddNewPreset();
	void        AddExistingPreset();
	void        ResetPreset(tukk szName);
	void        RemovePreset(tukk szName);
	void        OpenPreset(tukk szName);
	void        SavePresetAs(tukk szName);

	void        OnSignalTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void        OnContextMenu(const QPoint& point);

	tukk GetSelectedItem() const;

protected:
	QAdvancedTreeView*   m_presetsView;
	CPresetsModelNodePtr m_presetsViewRoot;
	CPresetsModel*       m_presetsModel;
	CPresetsModelNode*   m_currentlySelectedNode;

	QAction*             m_addNewPresetAction;
	QAction*             m_addExistingPresetAction;
};

