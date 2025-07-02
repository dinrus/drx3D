// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QWidget>

#include "EditorFramework/Editor.h"
#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>

#include <memory>

class CVegetationEditor : public CDockableEditor
{
public:
	explicit CVegetationEditor(QWidget* parent = nullptr);
	~CVegetationEditor();

	virtual IViewPaneClass::EDockingDirection GetDockingDirection() const override { return IViewPaneClass::DOCK_FLOAT; }
	virtual QRect                             GetPaneRect() override               { return QRect(0, 0, 800, 500); }

	virtual tukk                       GetEditorName() const override       { return "Vegetation Editor"; };

protected:
	virtual bool OnNew() override;
	virtual bool OnDelete() override;
	virtual bool OnDuplicate() override;
	virtual bool OnSelectAll() override;

private:
	struct SImplementation;
	std::unique_ptr<SImplementation> p;
};

