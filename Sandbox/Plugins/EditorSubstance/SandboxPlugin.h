// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
// Sandbox plugin wrapper.
#pragma once
#include "IEditor.h"
#include "IPlugin.h"

// Base class for plugin
class CSubstancePlugin : public IPlugin
{
public:
	static CSubstancePlugin* GetInstance();
	explicit CSubstancePlugin();
	~CSubstancePlugin();
	virtual tukk GetPluginName() override;
	virtual tukk GetPluginDescription() override;
	virtual i32       GetPluginVersion() override;
private:

};

