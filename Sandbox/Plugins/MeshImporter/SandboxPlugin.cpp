// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
// Sandbox plugin wrapper.
#include "StdAfx.h"
#include "SandboxPlugin.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>

#include "MainDialog.h"
#include <drx3D/Sandbox/Editor/Plugin/QtViewPane.h>
#include "IEditor.h"
#include <DrxSystem/ISystem.h>
#include <EditorFramework/PersonalizationManager.h>

// Plugin versioning
static tukk g_szPluginName = "MeshImporter";
static tukk g_szPluginDesc = "FBX Importer tools for meshes, and animations";
static DWORD g_pluginVersion = 1;

// Plugin instance
static CFbxToolPlugin* g_pInstance = nullptr;

// Global lock instance
DrxCriticalSection Detail::g_lock;


REGISTER_PLUGIN(CFbxToolPlugin);

class CEditorImpl;
CEditorImpl* GetIEditorImpl()
{
	return (CEditorImpl*)GetIEditor();
}

CFbxToolPlugin::CFbxToolPlugin()
{
	REGISTER_STRING("mi_defaultMaterial", "%ENGINE%/EngineAssets/TextureMsg/DefaultSolids", 0, "Default material.");
	REGISTER_INT("mi_lazyLodGeneration", 1, 0, "When non-zero, LOD auto-generation is deferred until LODs are actually visible.");
	REGISTER_FLOAT("mi_jointSize", 0.02f, 0, "Joint size");

	CScopedGlobalLock lock;
	assert(g_pInstance == nullptr);
	g_pInstance = this;
}

CFbxToolPlugin::~CFbxToolPlugin()
{
	CScopedGlobalLock lock;
	assert(g_pInstance == this);
	g_pInstance = nullptr;
}

CFbxToolPlugin* CFbxToolPlugin::GetInstance()
{
	return g_pInstance;
}

tukk CFbxToolPlugin::GetPluginName()
{
	return g_szPluginName;
}

tukk CFbxToolPlugin::GetPluginDescription()
{
	return g_szPluginDesc;
}

i32 CFbxToolPlugin::GetPluginVersion()
{
	return g_pluginVersion;
}

static void LogVPrintf(tukk szFormat, va_list args)
{
	string format;
	format.Format("[%s] %s", g_szPluginName, szFormat);
	gEnv->pLog->LogV(IMiniLog::eMessage, format.c_str(), args);
}

void LogPrintf(tukk szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	LogVPrintf(szFormat, args);
	va_end(args);
}

namespace FbxTool
{
namespace Detail
{

void Log(tukk szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	LogVPrintf(szFormat, args);
	va_end(args);
}

} //endns Detail
} //endns FbxTool

void CFbxToolPlugin::SetPersonalizationProperty(const QString& propName, const QVariant& value)
{
	GetIEditor()->GetPersonalizationManager()->SetProperty(MESH_IMPORTER_NAME, propName, value);
}

const QVariant& CFbxToolPlugin::GetPersonalizationProperty(const QString& propName)
{
	return GetIEditor()->GetPersonalizationManager()->GetProperty(MESH_IMPORTER_NAME, propName);
}


