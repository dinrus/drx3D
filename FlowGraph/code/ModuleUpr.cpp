// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/FlowGraph/StdAfx.h>
#include <drx3D/FlowGraph/ModuleUpr.h>
#include <drx3D/FlowGraph/Module.h>
#include <drx3D/FlowGraph/FlowModuleNodes.h>
#include <drx3D/FlowGraph/IFlowBaseNode.h>


#if !defined (_RELEASE)
void RenderModuleDebugInfo();
#endif

AllocateConstIntCVar(CFlowGraphModuleUpr, CV_fg_debugmodules);

IFlowGraphModule* CFlowGraphModuleUpr::CModuleIterator::Next()
{
	if (m_cur == m_pModuleUpr->m_Modules.end())
		return nullptr;
	IFlowGraphModule* pCur = m_cur->second;
	++m_cur;
	return pCur;
}

//////////////////////////////////////////////////////////////////////////

CFlowGraphModuleUpr::CFlowGraphModuleUpr()
	: m_listeners(1)
{
	m_nextModuleId = 0;
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this,"CFlowGraphModuleUpr");

	DefineConstIntCVarName("fg_debugmodules", CV_fg_debugmodules, 0, VF_NULL, "Display Flowgraph Modules debug information.\n" \
	  "0=Disabled"                                                                                           \
	  "1=Modules only"                                                                                       \
	  "2=Modules + Module Instances");
	fg_debugmodules_filter = REGISTER_STRING("fg_debugmodules_filter", "", VF_NULL,
	                                         "List of module names to display with the CVar 'fg_debugmodules'. Partial names can be supplied, but not regular expressions.");
}

CFlowGraphModuleUpr::~CFlowGraphModuleUpr()
{
	Shutdown();
	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	gEnv->pConsole->UnregisterVariable("fg_debugmodules", true);
	gEnv->pConsole->UnregisterVariable("fg_debugmodules_filter", true);

#if !defined (_RELEASE)
	if (gEnv->pGameFramework)
	{
		gEnv->pGameFramework->UnregisterListener(this);
	}
#endif
}

void CFlowGraphModuleUpr::Shutdown()
{
	ClearModules();
	m_listeners.Clear();
}

void CFlowGraphModuleUpr::DestroyModule(TModuleMap::iterator& itModule)
{
	LOADING_TIME_PROFILE_SECTION;
	IF_UNLIKELY(!itModule->second) return;

	for (CListenerSet<IFlowGraphModuleListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnModuleDestroyed(itModule->second);
	}

	SAFE_DELETE(itModule->second); // Calls module->Destroy() , sets the pointer in the map to null
}

void CFlowGraphModuleUpr::ClearModules()
{
	LOADING_TIME_PROFILE_SECTION;
	for (TModuleMap::iterator i = m_Modules.begin(), end = m_Modules.end(); i != end; ++i)
	{
		DestroyModule(i);
	}
	m_Modules.clear();
	m_ModuleIds.clear();
	m_ModulesPathInfo.clear();
	m_nextModuleId = 0;
}

void CFlowGraphModuleUpr::ClearLevelModules()
{
	LOADING_TIME_PROFILE_SECTION;
	for (TModuleMap::iterator it = m_Modules.begin(); it != m_Modules.end();)
	{
		CFlowGraphModule* pModule = it->second;
		if(pModule)
		{
			if (pModule->GetType() == IFlowGraphModule::eT_Level)
			{
				m_ModulesPathInfo.erase(pModule->m_fileName);
				m_ModuleIds.erase(pModule->m_fileName);
				DestroyModule(it);
				it = m_Modules.erase(it);
			}
			else
			{
				++it;
			}
		}
		else
		{
			assert(pModule); // should have been here. remove it and carry on
			it = m_Modules.erase(it);
		}
	}

	if (m_Modules.empty())
		m_nextModuleId = 0;
}

/* Serialization */

CFlowGraphModule* CFlowGraphModuleUpr::PreLoadModuleFile(tukk moduleName, tukk fileName, bool bGlobal)
{
	// NB: the module name passed in might be a best guess based on the filename. The actual name comes from within the module xml.

	// first check for existing module
	CFlowGraphModule* pModule = static_cast<CFlowGraphModule*>(GetModule(moduleName));
	if (pModule)
	{
		for (CListenerSet<IFlowGraphModuleListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		{
			notifier->OnModuleDestroyed(pModule);
		}
		// exists, reload
		pModule->Destroy();
		pModule->PreLoadModule(fileName);
	}
	else
	{
		// not found, create
		TModuleId id = m_nextModuleId++;
		pModule = new CFlowGraphModule(id);
		pModule->SetType(bGlobal ? IFlowGraphModule::eT_Global : IFlowGraphModule::eT_Level);

		m_Modules[id] = pModule;

		pModule->PreLoadModule(fileName); // constructs, loads name and registers the special module nodes

		m_ModuleIds[pModule->GetName()] = id;
		// important: the module should be added using its internal name rather than the filename
		m_ModulesPathInfo.emplace(pModule->GetName(), fileName);
	}

	return pModule;
}

void CFlowGraphModuleUpr::LoadModuleGraph(tukk moduleName, tukk fileName, IFlowGraphModuleListener::ERootGraphChangeReason rootGraphChangeReason)
{
	// Load actual graph with nodes and edges. The module should already be constructed and its nodes (call/start/end) registered
	LOADING_TIME_PROFILE_SECTION_ARGS(fileName);

	// first check for existing module - must exist by this point
	CFlowGraphModule* pModule = static_cast<CFlowGraphModule*>(GetModule(moduleName));
	assert(pModule);

	if (pModule && pModule->LoadModuleGraph(moduleName, fileName))
	{
		for (CListenerSet<IFlowGraphModuleListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		{
			notifier->OnRootGraphChanged(pModule, rootGraphChangeReason);
		}
	}
}

IFlowGraphModule* CFlowGraphModuleUpr::LoadModuleFile(tukk moduleName, tukk fileName, bool bGlobal)
{
	// first check for existing module
	CFlowGraphModule* pModule = static_cast<CFlowGraphModule*>(GetModule(moduleName));
	if (pModule)
	{
		for (CListenerSet<IFlowGraphModuleListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		{
			notifier->OnModuleDestroyed(pModule);
		}

		pModule->Destroy();
	}

	pModule = PreLoadModuleFile(moduleName, fileName, bGlobal);
	LoadModuleGraph(moduleName, fileName, IFlowGraphModuleListener::ERootGraphChangeReason::LoadModuleFile);

	return pModule;
}

bool CFlowGraphModuleUpr::DeleteModuleXML(tukk moduleName)
{
	// Deletes the module XML file from disk and cleans up internal structures
	TModulesPathInfo::iterator modulePathEntry = m_ModulesPathInfo.find(moduleName);
	if (m_ModulesPathInfo.end() != modulePathEntry)
	{
		if (remove(modulePathEntry->second) == 0)
		{
			m_ModulesPathInfo.erase(moduleName);

			TModuleIdMap::iterator idIt = m_ModuleIds.find(moduleName);
			assert(idIt != m_ModuleIds.end());
			TModuleId id = idIt->second;

			TModuleMap::iterator modIt = m_Modules.find(id);
			assert(modIt != m_Modules.end());

			if (modIt != m_Modules.end() && idIt != m_ModuleIds.end()) // prevent crash, should have asserted
			{
				DestroyModule(modIt);

				m_ModuleIds.erase(idIt);
				m_Modules.erase(modIt);

				if (m_Modules.empty())
					m_nextModuleId = 0;
			}

			return true;
		}
	}

	return false;
}

bool CFlowGraphModuleUpr::RenameModuleXML(tukk moduleName, tukk newName)
{
	// Renames the module XML file in disk and in the internal structures
	TModulesPathInfo::iterator modulePathEntry = m_ModulesPathInfo.find(moduleName);
	if (m_ModulesPathInfo.end() != modulePathEntry)
	{
		string newNameWithPath = PathUtil::GetPathWithoutFilename(modulePathEntry->second);
		newNameWithPath += newName;
		newNameWithPath += ".xml";

		if (rename(modulePathEntry->second, newNameWithPath.c_str()) == 0)
		{
			m_ModulesPathInfo.emplace(PathUtil::GetFileName(newNameWithPath), newNameWithPath);
			m_ModulesPathInfo.erase(moduleName);
			return true;
		}
	}

	return false;
}

void CFlowGraphModuleUpr::ScanFolder(const string& folderName, bool bGlobal)
{
	// Recursively scan module folders. Calls PreLoadModuleFile for each found xml

	_finddata_t fd;
	intptr_t handle = 0;
	IDrxPak* pPak = gEnv->pDrxPak;

	DrxPathString searchString = folderName.c_str();
	searchString.append("*.*");

	handle = pPak->FindFirst(searchString.c_str(), &fd);

	DrxPathString moduleName("");
	string newFolder("");

	if (handle > -1)
	{
		do
		{
			if (!strcmp(fd.name, ".") || !strcmp(fd.name, "..") || (fd.attrib & _A_HIDDEN))
				continue;

			if (fd.attrib & _A_SUBDIR)
			{
				newFolder.Format("%s%s/", folderName.c_str(), fd.name);
				ScanFolder(newFolder, bGlobal);
			}
			else
			{
				moduleName = fd.name;
				if (strcmpi(PathUtil::GetExt(moduleName.c_str()), "xml") == 0)
				{
					PathUtil::RemoveExtension(moduleName);
					PathUtil::MakeGamePath(folderName);

					// initial load: creates module, registers nodes
					CFlowGraphModule* pModule = PreLoadModuleFile(moduleName.c_str(), PathUtil::GetPathWithoutFilename(folderName) + fd.name, bGlobal);
				}
			}

		}
		while (pPak->FindNext(handle, &fd) >= 0);

		pPak->FindClose(handle);
	}
}

void CFlowGraphModuleUpr::RescanModuleNames(bool bGlobal, tukk szLoadedLevelName)
{
	LOADING_TIME_PROFILE_SECTION_ARGS(bGlobal ? "Global Modules" : "Level Modules");

	DrxFixedStringT<512> path = "";

	if (bGlobal)
	{
		path.Format("%s/%s", PathUtil::GetGameFolder().c_str(), GetGlobalModulesPath());
	}
	else
	{
		if (gEnv->IsEditor())
		{
			tuk levelName;
			tuk levelPath;
			gEnv->pGameFramework->GetEditorLevel(&levelName, &levelPath);
			path = levelPath;
		}
		else if (gEnv->pGameFramework->GetILevelSystem())
		{
			ILevelInfo* pLevel = gEnv->pGameFramework->GetILevelSystem()->GetLevelInfo(szLoadedLevelName);
			if (pLevel)
			{
				path = pLevel->GetPath();
				path.append("/");
			}
		}

		if (path.empty())
		{
			return;
		}

		path.append(GetLevelModulesPath());
	}

	ScanFolder(PathUtil::AddSlash(path), bGlobal);
}

void CFlowGraphModuleUpr::ScanAndReloadModules(bool bScanGlobalModules, bool bScanLevelModules, tukk szLoadedLevelName)
{
	LOADING_TIME_PROFILE_SECTION;

	// First pass: RescanModuleNames will "preload" the modules.
	// The module will be constructed ant its nodes (call,start,end) registered in the flowsystem. The actual graph is not loaded.
	// This ensures that module graphs can have call nodes for other graphs, because when the graphs are loaded, the special module nodes will already exist.
	if (bScanGlobalModules)
	{
		ClearModules();
		RescanModuleNames(true, szLoadedLevelName);  // global
	}

	if (bScanLevelModules)
	{
		ClearLevelModules();
		RescanModuleNames(false, szLoadedLevelName); // level
	}

	// Second pass: loading the actual graphs with their nodes and edges
	for (TModuleMap::const_iterator it = m_Modules.begin(), end = m_Modules.end(); it != end; ++it)
	{
		CFlowGraphModule* pModule = it->second;
		if ( pModule && (
				   (bScanGlobalModules && pModule->GetType() == IFlowGraphModule::eT_Global)
				|| (bScanLevelModules && pModule->GetType() == IFlowGraphModule::eT_Level) ))
		{
			LoadModuleGraph(pModule->GetName(), pModule->m_fileName, IFlowGraphModuleListener::ERootGraphChangeReason::ScanningForModules);
		}
	}

	// send modules reloaded event (Editor will create the corresponding graphs)
	for (CListenerSet<IFlowGraphModuleListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnModulesScannedAndReloaded();
	}
}

bool CFlowGraphModuleUpr::SaveModule(tukk moduleName, XmlNodeRef saveTo)
{
	TModuleId moduleId = stl::find_in_map(m_ModuleIds, moduleName, MODULEID_INVALID);
	if (moduleId != MODULEID_INVALID)
	{
		CFlowGraphModule* pModule = stl::find_in_map(m_Modules, moduleId, nullptr);
		if (pModule)
		{
			return pModule->SaveModuleXml(saveTo);
		}
	}

	return false;
}


/* Iterator */
IModuleIteratorPtr CFlowGraphModuleUpr::CreateModuleIterator()
{
	if (m_iteratorPool.empty())
	{
		return new CModuleIterator(this);
	}
	else
	{
		CModuleIterator* pIter = m_iteratorPool.back();
		m_iteratorPool.pop_back();
		new(pIter) CModuleIterator(this);
		return pIter;
	}
}

/* Getters/Setters */

IFlowGraphModule* CFlowGraphModuleUpr::GetModule(TModuleId moduleId) const
{
	return stl::find_in_map(m_Modules, moduleId, NULL);
}

IFlowGraphModule* CFlowGraphModuleUpr::GetModule(tukk moduleName) const
{
	TModuleId id = stl::find_in_map(m_ModuleIds, moduleName, MODULEID_INVALID);
	return GetModule(id);
}

IFlowGraphModule* CFlowGraphModuleUpr::GetModule(IFlowGraphPtr pFlowgraph) const
{
	for (TModuleMap::const_iterator it = m_Modules.begin(), end = m_Modules.end(); it != end; ++it)
	{
		CFlowGraphModule* pModule = it->second;

		if (pModule && pModule->HasInstanceGraph(pFlowgraph))
		{
			return pModule;
		}
	}

	return NULL;
}

tukk CFlowGraphModuleUpr::GetModulePath(tukk name)
{
	TModulesPathInfo::iterator modulePathEntry = m_ModulesPathInfo.find(name);
	if (m_ModulesPathInfo.end() != modulePathEntry)
	{
		return modulePathEntry->second;
	}

	return nullptr;
}


tukk CFlowGraphModuleUpr::GetStartNodeName(tukk moduleName) const
{
	static DrxFixedStringT<64> temp;
	temp.Format("Module:Start_%s", moduleName);
	return temp.c_str();
}

tukk CFlowGraphModuleUpr::GetReturnNodeName(tukk moduleName) const
{
	static DrxFixedStringT<64> temp;
	temp.Format("Module:End_%s", moduleName);
	return temp.c_str();
}

tukk CFlowGraphModuleUpr::GetCallerNodeName(tukk moduleName) const
{
	static DrxFixedStringT<64> temp;
	temp.Format("Module:Call_%s", moduleName);
	return temp.c_str();
}

/* Create Module Nodes */
bool CFlowGraphModuleUpr::CreateModuleNodes(tukk moduleName, TModuleId moduleId)
{
	assert(moduleId != MODULEID_INVALID);

	CFlowGraphModule* pModule = static_cast<CFlowGraphModule*>(GetModule(moduleId));
	if (pModule)
	{
		pModule->RegisterNodes();
	}

	return true;
}

/* Module handling */
void CFlowGraphModuleUpr::RemoveCompletedModuleInstances()
{
	for (TModuleMap::iterator it = m_Modules.begin(), end = m_Modules.end(); it != end; ++it)
{
		it->second->RemoveCompletedInstances();
}
}

void CFlowGraphModuleUpr::DestroyActiveModuleInstances()
{
	for (TModuleMap::const_iterator it = m_Modules.begin(), end = m_Modules.end(); it != end; ++it)
	{
		if (CFlowGraphModule* pModule = it->second)
		{
			pModule->RemoveAllInstances();
			pModule->RemoveCompletedInstances();
			pModule->ClearCallNodesForInstances();
			pModule->ClearGlobalControlNodes();
		}
	}
}

void CFlowGraphModuleUpr::ClearModuleRequestedInstances()
{
	for (TModuleMap::const_iterator it = m_Modules.begin(), end = m_Modules.end(); it != end; ++it)
{
		if (CFlowGraphModule* pModule = it->second)
	{
			pModule->ClearCallNodesForInstances();
		}
	}
}

bool CFlowGraphModuleUpr::RegisterListener(IFlowGraphModuleListener* pListener, tukk name)
{
	return m_listeners.Add(pListener, name);
}

void CFlowGraphModuleUpr::UnregisterListener(IFlowGraphModuleListener* pListener)
{
	m_listeners.Remove(pListener);
}

void CFlowGraphModuleUpr::BroadcastModuleInstanceStarted(IFlowGraphModule* module, TModuleInstanceId instanceID)
{
	for (CListenerSet<IFlowGraphModuleListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnModuleInstanceCreated(module, instanceID);
	}
}

void CFlowGraphModuleUpr::BroadcastModuleInstanceFinished(IFlowGraphModule* module, TModuleInstanceId instanceID)
{
	for (CListenerSet<IFlowGraphModuleListener*>::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnModuleInstanceDestroyed(module, instanceID);
	}
}

/* ISystemEventListener */
void CFlowGraphModuleUpr::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_GAME_POST_INIT:
		{
#if !defined (_RELEASE)
			DRX_ASSERT_MESSAGE(gEnv->pGameFramework, "Unable to register as Framework listener!");
			if (gEnv->pGameFramework)
			{
				gEnv->pGameFramework->RegisterListener(this, "FlowGraphModuleUpr", FRAMEWORKLISTENERPRIORITY_GAME);
			}
#endif
			tukk szLevelName = reinterpret_cast<tukk >(wparam);
			ScanAndReloadModules(true, false, szLevelName); // load global modules only
		}
		break;
	case ESYSTEM_EVENT_LEVEL_LOAD_START:
		{
			tukk szLevelName = reinterpret_cast<tukk >(wparam);
			ScanAndReloadModules(false, true, szLevelName); // load level modules
		}
		break;
	case ESYSTEM_EVENT_LEVEL_UNLOAD:
		{
			if(gEnv->pGameFramework->GetILevelSystem() && gEnv->pGameFramework->GetILevelSystem()->IsLevelLoaded())
			{
				ClearLevelModules();

				//Clear all instances in the level.
				DestroyActiveModuleInstances();
			}
		}
		break;
	case ESYSTEM_EVENT_EDITOR_GAME_MODE_CHANGED:
	case ESYSTEM_EVENT_EDITOR_SIMULATION_MODE_CHANGED:
		{
			if (wparam == 0)
			{
				DestroyActiveModuleInstances();
			}
		}
		break;
	}
}

/* IGameFrameworkListener */
void CFlowGraphModuleUpr::OnPostUpdate(float fDeltaTime)
{
#if !defined (_RELEASE)
	if (CV_fg_debugmodules > 0)
	{
		RenderModuleDebugInfo();
	}
#endif
}

/* Debug Draw */
#if !defined (_RELEASE)
void inline DrawModule2dLabel(float x, float y, float fontSize, const float* pColor, tukk pText)
{
	IRenderAuxText::DrawText(Vec3(x, y, 0.5f), fontSize, IRenderAuxText::AColor(pColor),
		eDrawText_2D | eDrawText_800x600 | eDrawText_FixedSize | eDrawText_Monospace, pText);
}

void DrawModuleTextLabel(float x, float y, const float* pColor, tukk pFormat, ...)
{
	char buffer[512];

	va_list args;
	va_start(args, pFormat);
	drx_vsprintf(buffer, pFormat, args);
	va_end(args);

	DrawModule2dLabel(x, y, 1.4f, pColor, buffer);
}

void RenderModuleDebugInfo()
{
	DRX_ASSERT_MESSAGE(gEnv->pFlowSystem, "No Flowsystem available!");

	if (!gEnv->pFlowSystem || !gEnv->pFlowSystem->GetIModuleUpr())
		return;

	IModuleIteratorPtr pIter = gEnv->pFlowSystem->GetIModuleUpr()->CreateModuleIterator();
	if (!pIter)
		return;

	i32k count = pIter->Count();

	if (count == 0)
	{
		static const float colorRed[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		DrawModuleTextLabel(10, 20, colorRed, "NO FLOWGRAPH MODULES AVAILABLE!");
		return;
	}

	if (IFlowGraphModule* pModule = pIter->Next())
	{
		float py = 15;
		const float dy = 15;
		const float dy_space = 5;
		static const float colorWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		static const float colorBlue[4] = { 0.3f, 0.8f, 1.0f, 1.0f };
		static const float colorGray[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
		static const float colorGreen[4] = { 0.3f, 1.0f, 0.8f, 1.0f };

		const float col1 = 10;
		const float col4 = col1 + 30;
		const float col5 = col4 + 350;

		DrawModuleTextLabel(col1, py, colorWhite, "Instances");
		DrawModuleTextLabel(col4 + 70, py, colorWhite, "Module");

		py += dy + dy_space;

		for (i32 i = 0; i < count; ++i)
		{
			// skip this module if the filter is being used and it's not there
			string filter = gEnv->pConsole->GetCVar("fg_debugmodules_filter")->GetString();
			string moduleName = pModule->GetName();
			moduleName.MakeLower();
			filter.MakeLower();
			if (!filter.empty() && moduleName.find(filter) == string::npos)
			{
				pModule = pIter->Next();
				continue;
			}

			// module details
			i32k runningInstances = pModule->GetRunningInstancesCount();

			DrawModuleTextLabel(col1, py, runningInstances ? colorGreen : colorGray, "%d", runningInstances);
			DrawModuleTextLabel(col4, py, runningInstances ? colorBlue : colorGray, "%s", pModule->GetName());

			// instances' details
			if (CFlowGraphModuleUpr::CV_fg_debugmodules == 2 && runningInstances > 0)
			{
				IModuleInstanceIteratorPtr instanceIter = pModule->CreateInstanceIterator();
				if (!instanceIter)
				{
					continue;
				}

				DrawModuleTextLabel(col5, py, colorWhite, "IDs:");
				stack_string s_idList("");
				for (IModuleInstance* pInstance = instanceIter->Next(); pInstance; pInstance = instanceIter->Next())
					{
					if (!pInstance->m_bUsed)
						{
						continue;
						}
					EntityId entityId = static_cast<SModuleInstance*>(pInstance)->m_entityId;
					stack_string s_inst;
					if (entityId != INVALID_ENTITYID)
					{
						s_inst.Format(", %d (Entity %u)", (pInstance->m_instanceId != MODULEINSTANCE_INVALID ? pInstance->m_instanceId : -1), entityId);
					}
				else
				{
						s_inst.Format(", %u", pInstance->m_instanceId);
					}
					s_idList.append(s_inst.c_str());
				}
				DrawModuleTextLabel(col5 + 40, py, colorGreen, "%s", s_idList.size() > 2 ? s_idList.c_str() + 2 : "-");
			}

			py += dy;
			pModule = pIter->Next();
		}
	}
}
#endif
