#ifndef D3_PLUGIN_CONTEXT_H
#define D3_PLUGIN_CONTEXT_H

#include <drx3D/SharedMemory/PhysicsClientC_API.h>

struct PluginContext
{
	b3PhysicsClientHandle m_physClient;

	//plugin can modify the m_userPointer to store persistent object pointer (class or struct instance etc)
	uk m_userPointer;

	//plugin can provide additional return data for executePluginCommand.
	//Lifetime of this m_returnData pointer is minimum of
	//next call to the next executePluginCommand or plugin termination.
	b3UserDataValue* m_returnData;

	const struct b3VRControllerEvent* m_vrControllerEvents;
	i32 m_numVRControllerEvents;
	const struct b3KeyboardEvent* m_keyEvents;
	i32 m_numKeyEvents;
	const struct b3MouseEvent* m_mouseEvents;
	i32 m_numMouseEvents;
	const struct b3Notification* m_notifications;
	i32 m_numNotifications;

	//only used for grpc/processClientCommands
	class PhysicsCommandProcessorInterface* m_rpcCommandProcessorInterface;
};

#endif  //D3_PLUGIN_CONTEXT_H