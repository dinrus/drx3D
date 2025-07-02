// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PerforcePlugin_h__
#define __PerforcePlugin_h__

#pragma once

#include <IPlugin.h>

class CPerforcePlugin : public IPlugin, public IAutoEditorNotifyListener
{
public:
	CPerforcePlugin();
	~CPerforcePlugin();
	
	i32       GetPluginVersion() override;
	tukk GetPluginName() override;
	tukk GetPluginDescription() override { return "Perforce source control integration"; }
	void        OnEditorNotifyEvent(EEditorNotifyEvent aEventId) override;
};

#endif //__PerforcePlugin_h__

