// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <IEditor.h>
#include <IPlugin.h>

#include <drx3D/Sandbox/Editor/Plugin/QtViewPane.h>
#include "EditorEnvironmentWindow.h"

class CEditorEnvironment : public IPlugin
{
public:
	CEditorEnvironment()
	{
	}

	i32       GetPluginVersion() override                          { return 1; }
	tukk GetPluginName() override                             { return "Environment Editor"; }
	tukk GetPluginDescription() override						 { return ""; }
};

REGISTER_PLUGIN(CEditorEnvironment);

