// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "IPlugin.h"

class CVehicleEditorPlugin : public IPlugin
{
public:
	CVehicleEditorPlugin() { /* entry point of the plugin, perform initializations */ }
	~CVehicleEditorPlugin() { /* exit point of the plugin, perform cleanup */ }

	i32       GetPluginVersion() { return 1; };
	tukk GetPluginName() { return "Sample Plugin"; };
	tukk GetPluginDescription() { return "Plugin used as a code sample to demonstrate Sandbox's plugin system"; };

private:
};
