// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "IPlugin.h"

class CMaterialEditorPlugin : public IPlugin
{
public:
	CMaterialEditorPlugin() { /* entry point of the plugin, perform initializations */ }
	~CMaterialEditorPlugin() { /* exit point of the plugin, perform cleanup */ }

	i32       GetPluginVersion() { return 1; };
	tukk GetPluginName() { return "Material Editor Plugin"; };
	tukk GetPluginDescription() { return "New material editor integrated with the asset system"; };

private:
};
