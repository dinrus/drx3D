#ifndef PID_CONTROL_PLUGIN_H
#define PID_CONTROL_PLUGIN_H

#include <drx3D/Plugins/PluginAPI.h>

#ifdef __cplusplus
extern "C"
{
#endif

	//the following 3 APIs are required
	DRX3D_SHARED_API i32 initPlugin_pdControlPlugin(struct PluginContext* context);
	DRX3D_SHARED_API void exitPlugin_pdControlPlugin(struct PluginContext* context);
	DRX3D_SHARED_API i32 executePluginCommand_pdControlPlugin(struct PluginContext* context, const struct b3PluginArguments* arguments);

	///
	enum PDControlCommandEnum
	{
		eSetPDControl = 1,
		eRemovePDControl = 2,
	};

	//all the APIs below are optional
	DRX3D_SHARED_API i32 preTickPluginCallback_pdControlPlugin(struct PluginContext* context);

#ifdef __cplusplus
};
#endif

#endif  //#define PID_CONTROL_PLUGIN_H
