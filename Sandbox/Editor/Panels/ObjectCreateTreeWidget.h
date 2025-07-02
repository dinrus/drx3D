// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "Controls/QObjectTreeWidget.h"

class CObjectClassDesc;

class QToolButton;
class QPreviewWidget;
class QStandardItem;

class CObjectCreateTreeWidget : public QObjectTreeWidget
{
public:
	enum Role
	{
		eRole_Path = QObjectTreeView::Roles::Id + 1,
		eRole_Material
	};

public:
	explicit CObjectCreateTreeWidget(CObjectClassDesc* pClassDesc, tukk szRegExp = "[/\\\\.]", QWidget* pParent = nullptr);

private:
	void UpdatePreviewWidget();

private:
	QToolButton*      m_pShowPreviewButton;
	QPreviewWidget*   m_pPreviewWidget;
	CObjectClassDesc* m_pClassDesc;
};

