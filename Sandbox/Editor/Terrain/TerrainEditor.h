// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>
#include "EditorFramework/Editor.h"
#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>

class CTerrainEditor : public CDockableEditor
{
public:
	CTerrainEditor(QWidget* parent = nullptr);
	~CTerrainEditor();

	virtual IViewPaneClass::EDockingDirection GetDockingDirection() const override { return IViewPaneClass::DOCK_FLOAT; }
	virtual QRect                             GetPaneRect() override               { return QRect(0, 0, 800, 500); }

	virtual tukk                       GetEditorName() const override       { return "Terrain Editor"; };
	void                                      InitTerrainMenu();

	virtual void                              SetLayout(const QVariantMap& state);
	virtual QVariantMap                       GetLayout() const override;

	static class CTerrainTextureDialog*       GetTextureLayerEditor();
};

