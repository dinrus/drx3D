// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <IPlugin.h>

class SandboxPythonBridgePlugin : public IPlugin
{
public:
	SandboxPythonBridgePlugin();
	~SandboxPythonBridgePlugin();

	virtual i32 GetPluginVersion() override;
	virtual tukk GetPluginName() override;
	virtual tukk GetPluginDescription() override;
};

