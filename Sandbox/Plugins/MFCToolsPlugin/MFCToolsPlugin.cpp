// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#include "StdAfx.h"
#include "MFCToolsPlugin.h"

#include "IPlugin.h"
#include <drx3D/CoreX/Platform/platform_impl.inl>

static IEditor* g_editor = nullptr;
IEditor* GetIEditor() { return g_editor; }

PLUGIN_API void MFCToolsPlugin::SetEditor(IEditor* editor)
{
	g_editor = editor;
	auto system = GetIEditor()->GetSystem();
	gEnv = system->GetGlobalEnvironment();
}

PLUGIN_API void MFCToolsPlugin::Initialize()
{
	ModuleInitISystem(GetIEditor()->GetSystem(), "MFCToolsPlugin");
	RegisterPlugin();
}

PLUGIN_API void MFCToolsPlugin::Deinitialize()
{
	UnregisterPlugin();
	g_editor = nullptr;
	gEnv = nullptr;
}

