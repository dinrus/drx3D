// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2001-2012.
// -------------------------------------------------------------------------
//  Description: The plugin manager, central point for loading/unloading plugins
//
////////////////////////////////////////////////////////////////////////////

struct IPlugin;
struct IUIEvent;

struct SPluginEntry
{
	HMODULE  hLibrary;
	IPlugin* pPlugin;
};

typedef std::list<SPluginEntry> TPluginList;

// Event IDs associated with event handlers
typedef std::map<i32, IUIEvent*>   TEventHandlerMap;
typedef TEventHandlerMap::iterator TEventHandlerIt;

// Plugins associated with ID / handler maps
typedef std::map<IPlugin*, TEventHandlerMap> TPluginEventMap;
typedef TPluginEventMap::iterator            TPluginEventIt;

// UI IDs associated with plugin pointers. When a plugin UI element is
// activated, the ID is used to determine which plugin should handle
// the event
typedef std::map<u8, IPlugin*> TUIIDPluginMap;
typedef TUIIDPluginMap::iterator  TUIIDPluginIt;

class CPluginManager
{
public:
	CPluginManager();
	virtual ~CPluginManager();

	bool               LoadPlugins(tukk pPathWithMask);
	void               UnloadAllPlugins();

	void               RegisterPlugin(HMODULE dllHandle, IPlugin* pPlugin);
	const TPluginList& GetPluginList() const { return m_plugins; }

protected:
	void CleanUp();

	TPluginList     m_plugins;
};

