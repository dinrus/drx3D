
#include <drx3D/Plugins/b3PluginManager.h>
#include <drx3D/Common/b3HashMap.h>
#include <drx3D/Common/b3ResizablePool.h>
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/SharedMemory/PhysicsDirect.h>
#include <drx3D/Plugins/PluginContext.h>
#include <drx3D/Common/DefaultFileIO.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

typedef HMODULE D3_DYNLIB_HANDLE;

#define D3_DYNLIB_OPEN LoadLibraryA
#define D3_DYNLIB_CLOSE FreeLibrary
#define D3_DYNLIB_IMPORT GetProcAddress
#else
#include <dlfcn.h>

typedef uk D3_DYNLIB_HANDLE;

#ifdef D3_USE_DLMOPEN
#define D3_DYNLIB_OPEN(path) dlmopen(LM_ID_NEWLM, path, RTLD_LAZY)
#else
#define D3_DYNLIB_OPEN(path) dlopen(path, RTLD_NOW | RTLD_GLOBAL)
#endif
#define D3_DYNLIB_CLOSE dlclose
#define D3_DYNLIB_IMPORT dlsym
#endif

struct b3Plugin
{
	D3_DYNLIB_HANDLE m_pluginHandle;
	bool m_ownsPluginHandle;
	bool m_isInitialized;
	STxt m_pluginPath;
	STxt m_pluginPostFix;
	i32 m_pluginUniqueId;
	PFN_INIT m_initFunc;
	PFN_EXIT m_exitFunc;
	PFN_EXECUTE m_executeCommandFunc;

	PFN_TICK m_preTickFunc;
	PFN_TICK m_postTickFunc;
	PFN_TICK m_processNotificationsFunc;
	PFN_TICK m_processClientCommandsFunc;

	PFN_GET_RENDER_INTERFACE m_getRendererFunc;
	PFN_GET_COLLISION_INTERFACE m_getCollisionFunc;
	PFN_GET_FILEIO_INTERFACE m_getFileIOFunc;

	uk m_userPointer;
	b3UserDataValue* m_returnData;

	b3Plugin()
		: m_pluginHandle(0),
		  m_ownsPluginHandle(false),
		  m_isInitialized(false),
		  m_pluginUniqueId(-1),
		  m_initFunc(0),
		  m_exitFunc(0),
		  m_executeCommandFunc(0),
		  m_preTickFunc(0),
		  m_postTickFunc(0),
		  m_processNotificationsFunc(0),
		  m_processClientCommandsFunc(0),
		  m_getRendererFunc(0),
		  m_getCollisionFunc(0),
		  m_getFileIOFunc(0),
		  m_userPointer(0),
		  m_returnData(0)
	{
	}
	void clear()
	{
		if (m_ownsPluginHandle)
		{
			D3_DYNLIB_CLOSE(m_pluginHandle);
		}
		m_pluginHandle = 0;
		m_initFunc = 0;
		m_exitFunc = 0;
		m_executeCommandFunc = 0;
		m_preTickFunc = 0;
		m_postTickFunc = 0;
		m_processNotificationsFunc = 0;
		m_processClientCommandsFunc = 0;
		m_getRendererFunc = 0;
		m_getCollisionFunc = 0;
		m_getFileIOFunc = 0;
		m_userPointer = 0;
		m_returnData = 0;
		m_isInitialized = false;
	}

	tukk GetMapKey() const
	{
		return GetMapKey(m_pluginPath.c_str(), m_pluginPostFix.c_str());
	}

	static tukk GetMapKey(tukk path, tukk postFix)
	{
		if (path != 0 && strlen(path) > 0)
		{
			return path;
		}
		else if (postFix != 0 && strlen(postFix) > 0)
		{
			return postFix;
		}
		return "";
	}
};

typedef b3PoolBodyHandle<b3Plugin> b3PluginHandle;

struct b3PluginManagerInternalData
{
	b3ResizablePool<b3PluginHandle> m_plugins;
	b3HashMap<b3HashString, i32> m_pluginMap;
	PhysicsDirect* m_physicsDirect;
	PhysicsCommandProcessorInterface* m_rpcCommandProcessorInterface;
	b3AlignedObjectArray<b3KeyboardEvent> m_keyEvents;
	b3AlignedObjectArray<b3VRControllerEvent> m_vrEvents;
	b3AlignedObjectArray<b3MouseEvent> m_mouseEvents;
	b3AlignedObjectArray<b3Notification> m_notifications[2];
	i32 m_activeNotificationsBufferIndex;
	i32 m_activeRendererPluginUid;
	i32 m_activeCollisionPluginUid;
	i32 m_numNotificationPlugins;
	i32 m_activeFileIOPluginUid;
	DefaultFileIO m_defaultFileIO;

	b3PluginManagerInternalData()
		: m_physicsDirect(0), m_rpcCommandProcessorInterface(0), m_activeNotificationsBufferIndex(0), m_activeRendererPluginUid(-1), m_activeCollisionPluginUid(-1), m_numNotificationPlugins(0), m_activeFileIOPluginUid(-1)
	{
	}
};

b3PluginManager::b3PluginManager(class PhysicsCommandProcessorInterface* physSdk)
{
	m_data = new b3PluginManagerInternalData;
	m_data->m_rpcCommandProcessorInterface = physSdk;
	m_data->m_physicsDirect = new PhysicsDirect(physSdk, false);
}

b3PluginManager::~b3PluginManager()
{
	while (m_data->m_pluginMap.size())
	{
		i32* pluginUidPtr = m_data->m_pluginMap.getAtIndex(0);
		if (pluginUidPtr)
		{
			i32 pluginUid = *pluginUidPtr;
			unloadPlugin(*pluginUidPtr);
		}
	}
	delete m_data->m_physicsDirect;
	m_data->m_pluginMap.clear();
	m_data->m_plugins.exitHandles();
	delete m_data;
}

void b3PluginManager::addEvents(const struct b3VRControllerEvent* vrControllerEvents, i32 numVRControllerEvents, const struct b3KeyboardEvent* keyEvents, i32 numKeyEvents, const struct b3MouseEvent* mouseEvents, i32 numMouseEvents)
{
	for (i32 i = 0; i < numKeyEvents; i++)
	{
		m_data->m_keyEvents.push_back(keyEvents[i]);
	}

	for (i32 i = 0; i < numVRControllerEvents; i++)
	{
		m_data->m_vrEvents.push_back(vrControllerEvents[i]);
	}
	for (i32 i = 0; i < numMouseEvents; i++)
	{
		m_data->m_mouseEvents.push_back(mouseEvents[i]);
	}
}

void b3PluginManager::clearEvents()
{
	m_data->m_keyEvents.resize(0);
	m_data->m_vrEvents.resize(0);
	m_data->m_mouseEvents.resize(0);
}

void b3PluginManager::addNotification(const struct b3Notification& notification)
{
	if (m_data->m_numNotificationPlugins > 0)
	{
		m_data->m_notifications[m_data->m_activeNotificationsBufferIndex].push_back(notification);
	}
}

i32 b3PluginManager::loadPlugin(tukk pluginPath, tukk postFixStr)
{
	i32 pluginUniqueId = -1;

	i32* pluginUidPtr = m_data->m_pluginMap.find(b3Plugin::GetMapKey(pluginPath, postFixStr));
	if (pluginUidPtr)
	{
		//already loaded
		pluginUniqueId = *pluginUidPtr;
		b3PluginHandle* plugin = m_data->m_plugins.getHandle(pluginUniqueId);
		if (!plugin->m_isInitialized)
		{
			PluginContext context = {0};
			context.m_userPointer = 0;
			context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;
			context.m_rpcCommandProcessorInterface = m_data->m_rpcCommandProcessorInterface;
			i32 result = plugin->m_initFunc(&context);
			plugin->m_isInitialized = true;
			plugin->m_userPointer = context.m_userPointer;
		}
	}
	else
	{
		pluginUniqueId = m_data->m_plugins.allocHandle();
		b3PluginHandle* plugin = m_data->m_plugins.getHandle(pluginUniqueId);
		plugin->m_pluginUniqueId = pluginUniqueId;
		D3_DYNLIB_HANDLE pluginHandle = D3_DYNLIB_OPEN(pluginPath);
		bool ok = false;
		if (pluginHandle)
		{
			STxt postFix = postFixStr;
			STxt initStr = STxt("initPlugin") + postFix;
			STxt exitStr = STxt("exitPlugin") + postFix;
			STxt executePluginCommandStr = STxt("executePluginCommand") + postFix;
			STxt preTickPluginCallbackStr = STxt("preTickPluginCallback") + postFix;
			STxt postTickPluginCallback = STxt("postTickPluginCallback") + postFix;
			STxt processNotificationsStr = STxt("processNotifications") + postFix;
			STxt processClientCommandsStr = STxt("processClientCommands") + postFix;
			STxt getRendererStr = STxt("getRenderInterface") + postFix;
			STxt getCollisionStr = STxt("getCollisionInterface") + postFix;
			STxt getFileIOStr = STxt("getFileIOInterface") + postFix;

			plugin->m_initFunc = (PFN_INIT)D3_DYNLIB_IMPORT(pluginHandle, initStr.c_str());
			plugin->m_exitFunc = (PFN_EXIT)D3_DYNLIB_IMPORT(pluginHandle, exitStr.c_str());
			plugin->m_executeCommandFunc = (PFN_EXECUTE)D3_DYNLIB_IMPORT(pluginHandle, executePluginCommandStr.c_str());
			plugin->m_preTickFunc = (PFN_TICK)D3_DYNLIB_IMPORT(pluginHandle, preTickPluginCallbackStr.c_str());
			plugin->m_postTickFunc = (PFN_TICK)D3_DYNLIB_IMPORT(pluginHandle, postTickPluginCallback.c_str());
			plugin->m_processNotificationsFunc = (PFN_TICK)D3_DYNLIB_IMPORT(pluginHandle, processNotificationsStr.c_str());

			if (plugin->m_processNotificationsFunc)
			{
				m_data->m_numNotificationPlugins++;
			}
			plugin->m_processClientCommandsFunc = (PFN_TICK)D3_DYNLIB_IMPORT(pluginHandle, processClientCommandsStr.c_str());

			plugin->m_getRendererFunc = (PFN_GET_RENDER_INTERFACE)D3_DYNLIB_IMPORT(pluginHandle, getRendererStr.c_str());
			plugin->m_getCollisionFunc = (PFN_GET_COLLISION_INTERFACE)D3_DYNLIB_IMPORT(pluginHandle, getCollisionStr.c_str());
			plugin->m_getFileIOFunc = (PFN_GET_FILEIO_INTERFACE)D3_DYNLIB_IMPORT(pluginHandle, getFileIOStr.c_str());


			if (plugin->m_initFunc && plugin->m_exitFunc && plugin->m_executeCommandFunc)
			{
				PluginContext context;
				context.m_userPointer = plugin->m_userPointer;
				context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;
				context.m_rpcCommandProcessorInterface = m_data->m_rpcCommandProcessorInterface;
				i32 version = plugin->m_initFunc(&context);
				plugin->m_isInitialized = true;
				//keep the user pointer persistent
				plugin->m_userPointer = context.m_userPointer;
				if (version == SHARED_MEMORY_MAGIC_NUMBER)
				{
					ok = true;
					plugin->m_ownsPluginHandle = true;
					plugin->m_pluginHandle = pluginHandle;
					plugin->m_pluginPath = pluginPath;
					plugin->m_pluginPostFix = postFixStr;
					m_data->m_pluginMap.insert(plugin->GetMapKey(), pluginUniqueId);
				}
				else
				{
					i32 expect = SHARED_MEMORY_MAGIC_NUMBER;
					drx3DWarning("Warning: plugin is wrong version: expected %d, got %d\n", expect, version);
				}
			}
			else
			{
				drx3DWarning("Loaded plugin but couldn't bind functions");
			}

			if (!ok)
			{
				D3_DYNLIB_CLOSE(pluginHandle);
			}
		}
		else
		{
			drx3DWarning("Warning: couldn't load plugin %s\n", pluginPath);
#ifdef _WIN32
#else
			drx3DWarning("Ошибка: %s\n", dlerror());
#endif
		}
		if (!ok)
		{
			m_data->m_plugins.freeHandle(pluginUniqueId);
			pluginUniqueId = -1;
		}
	}

	//for now, automatically select the loaded plugin as active renderer.
	if (pluginUniqueId >= 0)
	{
		b3PluginHandle* plugin = m_data->m_plugins.getHandle(pluginUniqueId);
		if (plugin && plugin->m_getRendererFunc)
		{
			selectPluginRenderer(pluginUniqueId);
		}
	}

	//for now, automatically select the loaded plugin as active collision plugin.
	if (pluginUniqueId >= 0)
	{
		b3PluginHandle* plugin = m_data->m_plugins.getHandle(pluginUniqueId);
		if (plugin && plugin->m_getCollisionFunc)
		{
			selectCollisionPlugin(pluginUniqueId);
		}
	}
	//for now, automatically select the loaded plugin as active fileIO plugin.
	if (pluginUniqueId >= 0)
	{
		b3PluginHandle* plugin = m_data->m_plugins.getHandle(pluginUniqueId);
		if (plugin && plugin->m_getFileIOFunc)
		{
			selectFileIOPlugin(pluginUniqueId);
		}
	}

	return pluginUniqueId;
}

void b3PluginManager::unloadPlugin(i32 pluginUniqueId)
{
	b3PluginHandle* plugin = m_data->m_plugins.getHandle(pluginUniqueId);
	if (plugin)
	{
		if (plugin->m_processNotificationsFunc)
		{
			m_data->m_numNotificationPlugins--;
		}
		PluginContext context = {0};
		context.m_userPointer = plugin->m_userPointer;
		context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;

		if (plugin->m_isInitialized)
		{
			plugin->m_exitFunc(&context);
			plugin->m_userPointer = 0;
			plugin->m_returnData = 0;
			plugin->m_isInitialized = false;
		}
		m_data->m_pluginMap.remove(plugin->GetMapKey());
		m_data->m_plugins.freeHandle(pluginUniqueId);
	}
}

void b3PluginManager::tickPlugins(double timeStep, b3PluginManagerTickMode tickMode)
{
	for (i32 i = 0; i < m_data->m_pluginMap.size(); i++)
	{
		i32* pluginUidPtr = m_data->m_pluginMap.getAtIndex(i);
		b3PluginHandle* plugin = 0;

		if (pluginUidPtr)
		{
			i32 pluginUid = *pluginUidPtr;
			plugin = m_data->m_plugins.getHandle(pluginUid);
		}
		else
		{
			continue;
		}

		PFN_TICK tick = 0;
		switch (tickMode)
		{
			case D3_PRE_TICK_MODE:
			{
				tick = plugin->m_preTickFunc;
				break;
			}
			case D3_POST_TICK_MODE:
			{
				tick = plugin->m_postTickFunc;
				break;
			}
			case D3_PROCESS_CLIENT_COMMANDS_TICK:
			{
				tick = plugin->m_processClientCommandsFunc;
				break;
			}
			default:
			{
			}
		}

		if (tick)
		{
			PluginContext context = {0};
			context.m_userPointer = plugin->m_userPointer;
			context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;
			context.m_numMouseEvents = m_data->m_mouseEvents.size();
			context.m_mouseEvents = m_data->m_mouseEvents.size() ? &m_data->m_mouseEvents[0] : 0;
			context.m_numKeyEvents = m_data->m_keyEvents.size();
			context.m_keyEvents = m_data->m_keyEvents.size() ? &m_data->m_keyEvents[0] : 0;
			context.m_numVRControllerEvents = m_data->m_vrEvents.size();
			context.m_vrControllerEvents = m_data->m_vrEvents.size() ? &m_data->m_vrEvents[0] : 0;
			if (tickMode == D3_PROCESS_CLIENT_COMMANDS_TICK)
			{
				context.m_rpcCommandProcessorInterface = m_data->m_rpcCommandProcessorInterface;
			}
			i32 result = tick(&context);
			plugin->m_userPointer = context.m_userPointer;
		}
	}
}

void b3PluginManager::reportNotifications()
{
	b3AlignedObjectArray<b3Notification>& notifications = m_data->m_notifications[m_data->m_activeNotificationsBufferIndex];
	if (notifications.size() == 0)
	{
		return;
	}

	// Swap notification buffers.
	m_data->m_activeNotificationsBufferIndex = 1 - m_data->m_activeNotificationsBufferIndex;

	for (i32 i = 0; i < m_data->m_pluginMap.size(); i++)
	{
		i32* pluginUidPtr = m_data->m_pluginMap.getAtIndex(i);
		b3PluginHandle* plugin = 0;

		if (pluginUidPtr)
		{
			i32 pluginUid = *pluginUidPtr;
			plugin = m_data->m_plugins.getHandle(pluginUid);
		}
		else
		{
			continue;
		}

		if (plugin->m_processNotificationsFunc)
		{
			PluginContext context = {0};
			context.m_userPointer = plugin->m_userPointer;
			context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;
			context.m_numNotifications = notifications.size();
			context.m_notifications = notifications.size() ? &notifications[0] : 0;
			plugin->m_processNotificationsFunc(&context);
		}
	}
	notifications.resize(0);
}

i32 b3PluginManager::executePluginCommand(i32 pluginUniqueId, const b3PluginArguments* arguments)
{
	i32 result = -1;

	b3PluginHandle* plugin = m_data->m_plugins.getHandle(pluginUniqueId);
	if (plugin)
	{
		PluginContext context = {0};
		context.m_userPointer = plugin->m_userPointer;
		context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;
		context.m_rpcCommandProcessorInterface = m_data->m_rpcCommandProcessorInterface;
		result = plugin->m_executeCommandFunc(&context, arguments);
		plugin->m_userPointer = context.m_userPointer;
		plugin->m_returnData = context.m_returnData;
	}
	return result;
}



i32 b3PluginManager::registerStaticLinkedPlugin(tukk pluginPath,  b3PluginFunctions& functions, bool initPlugin)
{
	b3Plugin orgPlugin;

	i32 pluginUniqueId = m_data->m_plugins.allocHandle();
	b3PluginHandle* pluginHandle = m_data->m_plugins.getHandle(pluginUniqueId);
	pluginHandle->m_pluginHandle = 0;
	pluginHandle->m_ownsPluginHandle = false;
	pluginHandle->m_pluginUniqueId = pluginUniqueId;
	pluginHandle->m_executeCommandFunc = functions.m_executeCommandFunc;
	pluginHandle->m_exitFunc = functions.m_exitFunc;
	pluginHandle->m_initFunc = functions.m_initFunc;
	pluginHandle->m_preTickFunc = functions.m_preTickFunc;
	pluginHandle->m_postTickFunc = functions.m_postTickFunc;
	pluginHandle->m_getRendererFunc = functions.m_getRendererFunc;
	pluginHandle->m_getCollisionFunc = functions.m_getCollisionFunc;
	pluginHandle->m_processClientCommandsFunc = functions.m_processClientCommandsFunc;
	pluginHandle->m_getFileIOFunc = functions.m_fileIoFunc;
	pluginHandle->m_pluginHandle = 0;
	pluginHandle->m_pluginPath = pluginPath;
	pluginHandle->m_pluginPostFix = "";
	pluginHandle->m_userPointer = 0;
	pluginHandle->m_returnData = 0;

	if (pluginHandle->m_processNotificationsFunc)
	{
		m_data->m_numNotificationPlugins++;
	}

	m_data->m_pluginMap.insert(pluginHandle->GetMapKey(), pluginUniqueId);

	if (initPlugin)
	{
		PluginContext context = {0};
		context.m_userPointer = 0;
		context.m_returnData = 0;
		context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;
		context.m_rpcCommandProcessorInterface = m_data->m_rpcCommandProcessorInterface;
		i32 result = pluginHandle->m_initFunc(&context);
		pluginHandle->m_isInitialized = true;
		pluginHandle->m_userPointer = context.m_userPointer;
		pluginHandle->m_returnData = 0;
	}
	return pluginUniqueId;
}

void b3PluginManager::selectPluginRenderer(i32 pluginUniqueId)
{
	m_data->m_activeRendererPluginUid = pluginUniqueId;
}

UrdfRenderingInterface* b3PluginManager::getRenderInterface()
{
	UrdfRenderingInterface* renderer = 0;

	if (m_data->m_activeRendererPluginUid >= 0)
	{
		b3PluginHandle* plugin = m_data->m_plugins.getHandle(m_data->m_activeRendererPluginUid);
		if (plugin && plugin->m_getRendererFunc)
		{
			PluginContext context = {0};
			context.m_userPointer = plugin->m_userPointer;
			context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;
			renderer = plugin->m_getRendererFunc(&context);
		}
	}
	return renderer;
}

void b3PluginManager::selectFileIOPlugin(i32 pluginUniqueId)
{
	m_data->m_activeFileIOPluginUid = pluginUniqueId;
}

struct CommonFileIOInterface* b3PluginManager::getFileIOInterface()
{
	CommonFileIOInterface* fileIOInterface = 0;
	if (m_data->m_activeFileIOPluginUid >= 0)
	{
		b3PluginHandle* plugin = m_data->m_plugins.getHandle(m_data->m_activeFileIOPluginUid);
		if (plugin && plugin->m_getFileIOFunc)
		{
			PluginContext context = {0};
			context.m_userPointer = plugin->m_userPointer;
			context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;
			fileIOInterface = plugin->m_getFileIOFunc(&context);
		}
	}
	if (fileIOInterface==0)
	{
		return &m_data->m_defaultFileIO;
	}
	return fileIOInterface;
}

void b3PluginManager::selectCollisionPlugin(i32 pluginUniqueId)
{
	m_data->m_activeCollisionPluginUid = pluginUniqueId;
}

struct b3PluginCollisionInterface* b3PluginManager::getCollisionInterface()
{
	b3PluginCollisionInterface* collisionInterface = 0;
	if (m_data->m_activeCollisionPluginUid >= 0)
	{
		b3PluginHandle* plugin = m_data->m_plugins.getHandle(m_data->m_activeCollisionPluginUid);
		if (plugin && plugin->m_getCollisionFunc)
		{
			PluginContext context = {0};
			context.m_userPointer = plugin->m_userPointer;
			context.m_physClient = (b3PhysicsClientHandle)m_data->m_physicsDirect;
			collisionInterface = plugin->m_getCollisionFunc(&context);
		}
	}
	return collisionInterface;
}

const struct b3UserDataValue* b3PluginManager::getReturnData(i32 pluginUniqueId)
{
	b3PluginHandle* plugin = m_data->m_plugins.getHandle(pluginUniqueId);
	if (plugin)
	{
		return plugin->m_returnData;
	}
	return 0;
}
