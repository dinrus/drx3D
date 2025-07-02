// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "PerforcePlugin.h"
#include "PerforceSourceControl.h"
#include "ISourceControl.h"
#include "IEditorClassFactory.h"

REGISTER_PLUGIN(CPerforcePlugin)

CPerforceSourceControl* g_pPerforceControl;

namespace PluginInfo
{
tukk kName = "Perforce Client";
i32k kVersion = 1;
}

CPerforcePlugin::CPerforcePlugin()
{
	g_pPerforceControl = new CPerforceSourceControl();
	GetIEditor()->GetClassFactory()->RegisterClass(g_pPerforceControl);
}

CPerforcePlugin::~CPerforcePlugin()
{
	delete g_pPerforceControl;
	g_pPerforceControl = nullptr;
}

i32 CPerforcePlugin::GetPluginVersion()
{
	return PluginInfo::kVersion;
}

tukk CPerforcePlugin::GetPluginName()
{
	return PluginInfo::kName;
}

void CPerforcePlugin::OnEditorNotifyEvent(EEditorNotifyEvent aEventId)
{
	if (eNotify_OnInit == aEventId)
	{
		g_pPerforceControl->Init();
	}
}

