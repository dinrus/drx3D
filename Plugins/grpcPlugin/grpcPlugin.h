#ifndef GRPC_PLUGIN_H
#define GRPC_PLUGIN_H

#include <drx3D/Plugins/PluginAPI.h>

#ifdef __cplusplus
extern "C"
{
#endif

	//the following 3 APIs are required
	DRX3D_SHARED_API i32 initPlugin_grpcPlugin(struct PluginContext* context);
	DRX3D_SHARED_API void exitPlugin_grpcPlugin(struct PluginContext* context);
	DRX3D_SHARED_API i32 executePluginCommand_grpcPlugin(struct PluginContext* context, const struct b3PluginArguments* arguments);

	//all the APIs below are optional
	DRX3D_SHARED_API i32 preTickPluginCallback_grpcPlugin(struct PluginContext* context);
	DRX3D_SHARED_API i32 postTickPluginCallback_grpcPlugin(struct PluginContext* context);

	DRX3D_SHARED_API i32 processClientCommands_grpcPlugin(struct PluginContext* context);

#ifdef __cplusplus
};
#endif

#endif  //#define GRPC_PLUGIN_H
