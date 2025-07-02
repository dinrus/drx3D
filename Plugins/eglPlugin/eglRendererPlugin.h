#ifndef EGL_RENDERER_PLUGIN_H
#define EGL_RENDERER_PLUGIN_H

#include <drx3D/Plugins/PluginAPI.h>

#ifdef __cplusplus
extern "C"
{
#endif

	//initPlugin, exitPlugin and executePluginCommand are required, otherwise plugin won't load
	DRX3D_SHARED_API i32 initPlugin_eglRendererPlugin(struct PluginContext* context);
	DRX3D_SHARED_API void exitPlugin_eglRendererPlugin(struct PluginContext* context);
	DRX3D_SHARED_API i32 executePluginCommand_eglRendererPlugin(struct PluginContext* context, const struct b3PluginArguments* arguments);

	//all the APIs below are optional
	DRX3D_SHARED_API struct UrdfRenderingInterface* getRenderInterface_eglRendererPlugin(struct PluginContext* context);

#ifdef __cplusplus
};
#endif

#endif  //#define EGL_RENDERER_PLUGIN_H
