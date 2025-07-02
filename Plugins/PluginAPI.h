#ifndef D3_PLUGIN_API_H
#define D3_PLUGIN_API_H

#include <drxtypes.h>

#ifdef _WIN32
#define DRX3D_SHARED_API __declspec(dllexport)
#elif defined(__GNUC__)
#define DRX3D_SHARED_API __attribute__((visibility("default")))
#else
#define DRX3D_SHARED_API
#endif

#if defined(_WIN32)
#define D3_API_ENTRY
#define D3_API_CALL __cdecl
#define D3_CALLBACK __cdecl
#else
#define D3_API_ENTRY
#define D3_API_CALL
#define D3_CALLBACK
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	/* Plugin API */
	typedef D3_API_ENTRY i32(D3_API_CALL* PFN_INIT)(struct PluginContext* context);
	typedef D3_API_ENTRY void(D3_API_CALL* PFN_EXIT)(struct PluginContext* context);
	typedef D3_API_ENTRY i32(D3_API_CALL* PFN_EXECUTE)(struct PluginContext* context, const struct b3PluginArguments* arguments);
	typedef D3_API_ENTRY i32(D3_API_CALL* PFN_TICK)(struct PluginContext* context);

	typedef D3_API_ENTRY struct UrdfRenderingInterface*(D3_API_CALL* PFN_GET_RENDER_INTERFACE)(struct PluginContext* context);
	typedef D3_API_ENTRY struct b3PluginCollisionInterface*(D3_API_CALL* PFN_GET_COLLISION_INTERFACE)(struct PluginContext* context);
	typedef D3_API_ENTRY struct CommonFileIOInterface*(D3_API_CALL* PFN_GET_FILEIO_INTERFACE)(struct PluginContext* context);
#ifdef __cplusplus
}
#endif

#endif  //D3_PLUGIN_API_H
