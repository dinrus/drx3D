
//test plugin, can load a URDF file, example usage on a Windows machine:

/*
import pybullet as p
p.connect(p.GUI)
pluginUid = p.loadPlugin("E:/develop/bullet3/bin/pybullet_testplugin_vs2010_x64_debug.dll")
commandUid = 0
argument = "plane.urdf"
p.executePluginCommand(pluginUid,commandUid,argument)
p.unloadPlugin(pluginUid)
*/

#include "testplugin.h"
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Plugins/PluginContext.h>
#include <stdio.h>

struct MyClass
{
	i32 m_testData;

	MyClass()
		: m_testData(42)
	{
	}
	virtual ~MyClass()
	{
	}
};

DRX3D_SHARED_API i32 initPlugin_testPlugin(struct PluginContext* context)
{
	MyClass* obj = new MyClass();
	context->m_userPointer = obj;

	printf("hi!\n");
	return SHARED_MEMORY_MAGIC_NUMBER;
}

DRX3D_SHARED_API i32 preTickPluginCallback_testPlugin(struct PluginContext* context)
{
	return 0;
}

DRX3D_SHARED_API i32 postTickPluginCallback_testPlugin(struct PluginContext* context)
{
	MyClass* obj = (MyClass*)context->m_userPointer;
	obj->m_testData++;
	return 0;
}

DRX3D_SHARED_API i32 executePluginCommand_testPlugin(struct PluginContext* context, const struct b3PluginArguments* arguments)
{
	printf("text argument:%s\n", arguments->m_text);
	printf("i32 args: [");
	for (i32 i = 0; i < arguments->m_numInts; i++)
	{
		printf("%d", arguments->m_ints[i]);
		if ((i + 1) < arguments->m_numInts)
		{
			printf(",");
		}
	}
	printf("]\nfloat args: [");
	for (i32 i = 0; i < arguments->m_numFloats; i++)
	{
		printf("%f", arguments->m_floats[i]);
		if ((i + 1) < arguments->m_numFloats)
		{
			printf(",");
		}
	}
	printf("]\n");

	MyClass* obj = (MyClass*)context->m_userPointer;

	b3SharedMemoryStatusHandle statusHandle;
	i32 statusType = -1;
	i32 bodyUniqueId = -1;

	b3SharedMemoryCommandHandle command =
		b3LoadUrdfCommandInit(context->m_physClient, arguments->m_text);

	statusHandle = b3SubmitClientCommandAndWaitStatus(context->m_physClient, command);
	statusType = b3GetStatusType(statusHandle);
	if (statusType == CMD_URDF_LOADING_COMPLETED)
	{
		bodyUniqueId = b3GetStatusBodyIndex(statusHandle);
	}
	return bodyUniqueId;
}

DRX3D_SHARED_API void exitPlugin_testPlugin(struct PluginContext* context)
{
	MyClass* obj = (MyClass*)context->m_userPointer;
	delete obj;
	context->m_userPointer = 0;

	printf("bye!\n");
}
