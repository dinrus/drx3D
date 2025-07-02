// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "Plugin.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>

#include "IResourceSelectorHost.h"

#include <DrxSchematyc/Compiler/ICompiler.h>
#include <DrxSchematyc/Script/IScriptRegistry.h>
#include <DrxSchematyc/SerializationUtils/SerializationEnums.inl>
#include <DrxSchematyc/Utils/GUID.h>

#include "DrxLinkCommands.h"

i32k g_pluginVersion = 1;
tukk g_szPluginName = "Schematyc Plugin";
tukk g_szPluginDesc = "Schematyc Editor integration";

DrxGUID GenerateGUID()
{
	DrxGUID guid;
#if DRX_PLATFORM_WINDOWS
	GUID winGuid;
	::CoCreateGuid(&winGuid);
	static_assert(sizeof(winGuid)==sizeof(guid),"GUID and DrxGUID sizes should match.");
	memcpy(&guid,&winGuid,sizeof(guid));
#else
	guid = DrxGUID::Create();
#endif
	return guid;
}

//REGISTER_PLUGIN(CSchematycPlugin);
	
// TODO: Temp solution for the case experimental Schematyc plugin was not loaded.
static IEditor* g_pEditor = nullptr;                                                    
IEditor* GetIEditor() { return g_pEditor; }                                             
                                                                                        
DLL_EXPORT IPlugin* CreatePluginInstance(PLUGIN_INIT_PARAM* pInitParam)                 
{ 
	if (pInitParam->pluginVersion != SANDBOX_PLUGIN_SYSTEM_VERSION)                     
	{                                                                                   
		pInitParam->outErrorCode = IPlugin::eError_VersionMismatch;                     
		return nullptr;
	}                                                                                   
                                                                                        
	g_pEditor = pInitParam->pIEditor;													
	ModuleInitISystem(g_pEditor->GetSystem(), "CSchematycPlugin");
	if (gEnv->pSchematyc == nullptr)
	{
		return nullptr;
	}
	CSchematycPlugin* pPlugin = new CSchematycPlugin();
                                                                                        
	RegisterPlugin();                                                                   
                                                                                        
	return pPlugin;                                                                     
}                                                                                       
                                                                                        
DLL_EXPORT void DeletePluginInstance(IPlugin* pPlugin)                                  
{                                                                                       
	UnregisterPlugin();                                                                 
	delete pPlugin;                                                                     
}
// ~TODO

CSchematycPlugin::CSchematycPlugin()
{
	// In case the Schematyc Core module hasn't been loaded we would crash here without this check.
	// This condition can be removed once Editor plugins are properly handled by the Plugin Manager.
	if (gEnv->pSchematyc != nullptr)
	{
		Schematyc::CDrxLinkCommands::GetInstance().Register(g_pEditor->GetSystem()->GetIConsole());

		// Hook up GUID generator then fix-up script files and resolve broken/deprecated dependencies.
		DrxLogAlways("[SchematycEditor]: Initializing...");
		gEnv->pSchematyc->SetGUIDGenerator(SCHEMATYC_DELEGATE(&GenerateGUID));
		DrxLogAlways("[SchematycEditor]: Fixing up script files");
		gEnv->pSchematyc->GetScriptRegistry().ProcessEvent(Schematyc::SScriptEvent(Schematyc::EScriptEventId::EditorFixUp));
		DrxLogAlways("[SchematycEditor]: Compiling script files");
		gEnv->pSchematyc->GetCompiler().CompileAll();
		DrxLogAlways("[SchematycEditor]: Initialization complete");
	}
}

i32 CSchematycPlugin::GetPluginVersion()
{
	return g_pluginVersion;
}

tukk CSchematycPlugin::GetPluginName()
{
	return g_szPluginName;
}

tukk CSchematycPlugin::GetPluginDescription()
{
	return g_szPluginDesc;
}



