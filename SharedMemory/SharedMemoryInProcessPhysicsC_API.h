
#ifndef IN_PROCESS_PHYSICS_C_API_H
#define IN_PROCESS_PHYSICS_C_API_H

#include <drx3D/SharedMemory/PhysicsClientC_API.h>

#ifdef __cplusplus
extern "C"
{
#endif

	///think more about naming. The b3ConnectPhysicsLoopback
	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerAndConnect(i32 argc, tuk argv[]);
	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerAndConnectMainThread(i32 argc, tuk argv[]);

	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerAndConnectSharedMemory(i32 argc, tuk argv[]);
	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerAndConnectMainThreadSharedMemory(i32 argc, tuk argv[]);

	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessGraphicsServerAndConnectSharedMemory(i32 port);
	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessGraphicsServerAndConnectMainThreadSharedMemory(i32 port);
	

	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect(uk guiHelperPtr);
	//create a shared memory physics server, with a DummyGUIHelper (no graphics)
	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect2(uk guiHelperPtr);
	//create a shared memory physics server, with a DummyGUIHelper (no graphics) and allow to set shared memory key
	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect3(uk guiHelperPtr, i32 sharedMemoryKey);
	//create a shared memory physics server, with a RemoteGUIHelper (connect to remote graphics server) and allow to set shared memory key
	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect4(uk guiHelperPtr, i32 sharedMemoryKey);
//#ifdef DRX3D_ENABLE_CLSOCKET
	DRX3D_SHARED_API b3PhysicsClientHandle b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnectTCP(tukk hostName, i32 port);
//#endif
	///ignore the following APIs, they are for internal use for example browser
	void b3InProcessRenderSceneInternal(b3PhysicsClientHandle clientHandle);
	void b3InProcessDebugDrawInternal(b3PhysicsClientHandle clientHandle, i32 debugDrawMode);
	i32 b3InProcessMouseMoveCallback(b3PhysicsClientHandle clientHandle, float x, float y);
	i32 b3InProcessMouseButtonCallback(b3PhysicsClientHandle clientHandle, i32 button, i32 state, float x, float y);

#ifdef __cplusplus
}
#endif

#endif  //IN_PROCESS_PHYSICS_C_API_H
