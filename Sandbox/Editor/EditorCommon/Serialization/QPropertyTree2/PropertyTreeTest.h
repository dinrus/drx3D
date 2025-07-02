// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

//TODO: Create a specific property tree testing dockable to remain available instead of relying on the Playground Dockable.
// This will also allow using these tests without exporting the symbols

namespace PropertyTreeTest
{
	EDITOR_COMMON_API QWidget* CreatePropertyTreeTestWidget();
	EDITOR_COMMON_API QWidget* CreateLegacyPTWidget();

	EDITOR_COMMON_API QWidget* CreatePropertyTreeTestWidget_MultiEdit();
	EDITOR_COMMON_API QWidget* CreatePropertyTreeTestWidget_MultiEdit2();
	EDITOR_COMMON_API QWidget* CreateLegacyPTWidget_MultiEdit();
}
