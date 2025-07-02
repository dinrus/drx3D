// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <IEditor.h>
#include <IPlugin.h>

typedef std::vector<string> TSignalList;

class CEditorDynamicResponseSystemPlugin : public IPlugin
{
public:
	CEditorDynamicResponseSystemPlugin();

	virtual i32       GetPluginVersion() override                          { return 1; }
	virtual tukk GetPluginName() override                             { return "Dynamic Response System Editor"; }
	virtual tukk GetPluginDescription() override						 { return ""; }
};

