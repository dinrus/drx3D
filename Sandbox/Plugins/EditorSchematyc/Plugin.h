// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <IPlugin.h>

class CSchematycPlugin : public IPlugin
{
public:

	CSchematycPlugin();
	~CSchematycPlugin();

	// IPlugin
	i32       GetPluginVersion() override;
	tukk GetPluginName() override;
	tukk GetPluginDescription() override;
	// ~IPlugin
};

