// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <EditorFramework/Editor.h>
#include "IEditorImpl.h"

#include "FileDialogs/FileDialogsCommon.h"

class QWidget;
class QVBoxLayout;
class QStackedLayout;
class QGridLayout;
class QPreviewWidget;
class QToolButton;

class CCreateObjectButtons : public QWidget
{
	Q_OBJECT
public:
	CCreateObjectButtons(QWidget* pParent = nullptr);
	~CCreateObjectButtons();

	void AddButton(tukk type, const std::function<void()>& onClicked);
private:

	QGridLayout* m_layout;
};

class CObjectCreateToolPanel : public CDockableEditor
{
	Q_OBJECT
public:
	CObjectCreateToolPanel(QWidget* pParent = nullptr);
	~CObjectCreateToolPanel();

	virtual tukk GetEditorName() const { return "Create Object"; }

protected:
	void OnBackButtonPressed();
	virtual void mousePressEvent(QMouseEvent* pMouseEvent) override;
	virtual bool eventFilter(QObject* pWatched, QEvent* pEvent) override;

private:
	void         OnObjectTypeSelected(tukk type);
	void         OnObjectSubTypeSelected(tukk type, QWidget* parent);
	void         Animate(QWidget* pIn, QWidget* pOut, bool bReverse = false);

	QWidget*     CreateWidgetOrStartCreate(tukk type);
	QWidget*     CreateWidgetContainer(QWidget* pChild, tukk type);

	void         StartCreation(CObjectClassDesc* cls, tukk file);
	void         AbortCreateTool();

	void         UpdatePreviewWidget(QString path, QPreviewWidget* previewWidget, QToolButton* showPreviewButton);

	QStackedLayout*       m_stacked;
	CCreateObjectButtons* m_pTypeButtonPanel;
	std::vector<string>   m_typeToStackIndex;
};

