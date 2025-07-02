// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "IPlugin.h"

class CViconClient;
class CFacialEditorPlugin : public IPlugin, public IAutoEditorNotifyListener
{
public:
	CFacialEditorPlugin();
	~CFacialEditorPlugin() { /* exit point of the plugin, perform cleanup */ }

	i32       GetPluginVersion() { return 1; };
	tukk GetPluginName() { return "Sample Plugin"; };
	tukk GetPluginDescription() { return "Plugin used as a code sample to demonstrate Sandbox's plugin system"; };

	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event) override;

	static CFacialEditorPlugin* GetInstance();
private:
	static CFacialEditorPlugin* s_instance;

#ifndef DISABLE_VICON
public:
	CViconClient* GetViconClient() { return m_ViconClient; }

private:
	float             m_fViconScale;
	float             m_fViconBend;
	CViconClient*     m_ViconClient;
#endif
};
