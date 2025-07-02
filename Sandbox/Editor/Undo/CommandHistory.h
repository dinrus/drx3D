// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>

class CHistoryPanel : public CDockableWidget
{
public:
	CHistoryPanel();
	~CHistoryPanel();

	virtual IViewPaneClass::EDockingDirection GetDockingDirection() const override { return IViewPaneClass::DOCK_RIGHT; }
	virtual tukk                       GetPaneTitle() const override        { return "Undo History"; };
	virtual QRect                             GetPaneRect() override               { return QRect(0, 0, 800, 500); }
};

