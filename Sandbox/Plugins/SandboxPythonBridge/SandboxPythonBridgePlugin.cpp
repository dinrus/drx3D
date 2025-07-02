// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SandboxPythonBridgePlugin.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>

REGISTER_PLUGIN(SandboxPythonBridgePlugin)

SandboxPythonBridgePlugin* g_pPlugin = nullptr;

SandboxPythonBridgePlugin* GetSandboxPythonBridge()
{
	return g_pPlugin;
}

namespace PluginInfo
{
	tukk kName = "Sandbox Python Bridge";
	tukk kDesc = "Enables usage of Python plugins";
	i32k kVersion = 0;
}

SandboxPythonBridgePlugin::SandboxPythonBridgePlugin()
{
}


SandboxPythonBridgePlugin::~SandboxPythonBridgePlugin()
{
}

i32 SandboxPythonBridgePlugin::GetPluginVersion()
{
	return PluginInfo::kVersion;
}

tukk SandboxPythonBridgePlugin::GetPluginName()
{
	return PluginInfo::kName;
}

tukk SandboxPythonBridgePlugin::GetPluginDescription()
{
	return PluginInfo::kDesc;
}


