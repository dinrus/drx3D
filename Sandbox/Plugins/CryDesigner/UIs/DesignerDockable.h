// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/Plugin/QtViewPane.h>
#include "IEditor.h"

class QWidget;

namespace Designer
{

class DesignerDockable : public CDockableWidget
{
public:
	DesignerDockable(QWidget* pParent = nullptr);
	~DesignerDockable();

	tukk GetPaneTitle() const { return "Designer Tool"; }

private:
	void SetupUI();
};

}


