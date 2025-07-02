// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include "EditorDialog.h"

class QObjectTreeWidget;
class QLabel;

class EDITOR_COMMON_API QResourceBrowserDialog : public CEditorDialog
{
	Q_OBJECT
public:
	QResourceBrowserDialog();

	void           PreviewResource(tukk szIconPath);
	void           ResourceSelected(tukk szIconPath);
	const QString& GetSelectedResource() const { return m_SelectedResource; }

protected:
	virtual QSize sizeHint() const override { return QSize(480, 480); }

protected:
	QObjectTreeWidget* m_pTreeWidget;
	QString            m_SelectedResource;
	QLabel*            m_pPreview;
};

