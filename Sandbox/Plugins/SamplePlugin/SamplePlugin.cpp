// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#include "StdAfx.h"
#include "SamplePlugin.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <ICommandManager.h>

namespace Private_SamplePlugin
{
	void SampleCommand()
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Sample Command executed");
	}
}

REGISTER_EDITOR_COMMAND(Private_SamplePlugin::SampleCommand, sample, sample_command, CCommandDescription(""));

REGISTER_PLUGIN(CSamplePlugin);
