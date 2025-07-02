// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "IPlugin.h"

class CSmartObjectEditorPlugin : public IPlugin
{
public:
	CSmartObjectEditorPlugin();
	~CSmartObjectEditorPlugin();

	i32       GetPluginVersion() { return 1; };
	tukk GetPluginName() { return "Sample Plugin"; };
	tukk GetPluginDescription() { return "Plugin used as a code sample to demonstrate Sandbox's plugin system"; };

private:

	bool OnEditDeprecatedProperty(i32 type, const string& oldValue, string& newValue) const;
};
