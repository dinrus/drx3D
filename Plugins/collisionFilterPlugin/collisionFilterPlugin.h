#ifndef COLLISION_FILTER_PLUGIN_H
#define COLLISION_FILTER_PLUGIN_H

#include <drx3D/Plugins/PluginAPI.h>

#ifdef __cplusplus
extern "C"
{
#endif

	//the following 3 APIs are required
	DRX3D_SHARED_API i32 initPlugin_collisionFilterPlugin(struct PluginContext* context);
	DRX3D_SHARED_API void exitPlugin_collisionFilterPlugin(struct PluginContext* context);
	DRX3D_SHARED_API i32 executePluginCommand_collisionFilterPlugin(struct PluginContext* context, const struct b3PluginArguments* arguments);

	//all the APIs below are optional
	DRX3D_SHARED_API struct b3PluginCollisionInterface* getCollisionInterface_collisionFilterPlugin(struct PluginContext* context);

#ifdef __cplusplus
};
#endif

#endif  //#define COLLISION_FILTER_PLUGIN_H
