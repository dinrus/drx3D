// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "FBXPlugin.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>

#include "FBXExporter.h"

REGISTER_PLUGIN(CFBXPlugin);

namespace PluginInfo
{
tukk kName = "FBX Exporter";
tukk kDesc = "FBX Exporter plugin";
i32k kVersion = 1;
}

CFBXPlugin::CFBXPlugin()
{
	IExportManager* pExportManager = GetIEditor()->GetExportManager();
	if (pExportManager)
	{
		pExportManager->RegisterExporter(new CFBXExporter());
	}
}

CFBXPlugin::~CFBXPlugin()
{
	DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "CFBXPlugin cannot be unloaded as FBX exporter cannot be unregistered");
}

i32 CFBXPlugin::GetPluginVersion()
{
	return PluginInfo::kVersion;
}

tukk CFBXPlugin::GetPluginName()
{
	return PluginInfo::kName;
}

tukk CFBXPlugin::GetPluginDescription()
{
	return PluginInfo::kDesc;
}

