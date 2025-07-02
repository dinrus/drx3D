// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DockedWidget.h>
#include <drx3D/CoreX/Serialization/IArchive.h>

class QPropertyTree;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

class CDrsResponseEditorWindow;
class CSpokenLinesWidget;
class CDialogLinesEditorWidget;

class CDrsSerializerBase : public QWidget
{
public:
	CDrsSerializerBase();
	void SetAutoUpdateActive(bool bValue);

protected:
	void TriggerUpdate();

	QPropertyTree* m_pPropertyTree;
	QTimer*        m_pAutoUpdateTimer;

	QVBoxLayout*   m_pVLayout;
	QPushButton*   m_pUpdateButton;
};

class CDrsVariableSerializer : public CDrsSerializerBase
{
public:
	CDrsVariableSerializer();
	void Serialize(Serialization::IArchive& ar);
};

class CDrsVariableChangesSerializer : public CDrsSerializerBase
{
public:
	CDrsVariableChangesSerializer();
	void Serialize(Serialization::IArchive& ar);

protected:
	QHBoxLayout* m_pHLayout;
	QPushButton* m_pClearButton;
	i32          m_SerializationFilter;
};

class CDrsVariableInfos : public QMainWindow
{
public:
	CDrsVariableInfos();
	void SetAutoUpdateActive(bool bValue);

protected:
	DockedWidget<CDrsVariableSerializer>*        m_pVariablesSerializerDockWidget;
	DockedWidget<CDrsVariableChangesSerializer>* m_pVariableChangesSerializerDockWidget;
};

class CDrsEditorMainWindow : public CDockableWindow
{
public:
	CDrsEditorMainWindow();

	//////////////////////////////////////////////////////////
	// CDockableWindow implementation
	virtual tukk                       GetPaneTitle() const override        { return "Dynamic Response System"; };
	virtual IViewPaneClass::EDockingDirection GetDockingDirection() const override { return IViewPaneClass::DOCK_FLOAT; }
	//////////////////////////////////////////////////////////

protected:
	void customEvent(QEvent* event) override;

private:
	DockedWidget<CSpokenLinesWidget>*       m_pSpokenLinesDockWidget;
	DockedWidget<CDrsVariableInfos>*        m_pVariableInfosDockWidget;
	DockedWidget<CDrsResponseEditorWindow>* m_pResponseEditorDockWidget;
	DockedWidget<CDialogLinesEditorWidget>* m_pDialogLinesDockWidget;
};

