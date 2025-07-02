// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "../stdafx.h"
#include "../EditorCommonInit.h"
#include "../IPlugin.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <QMetaType>
#include "../ProxyModels/ItemModelAttribute.h"

static IEditor* g_editor = nullptr;

IEditor* GetIEditor() { return g_editor; }

void EDITOR_COMMON_API EditorCommon::SetIEditor(IEditor* editor)
{
	g_editor = editor;
	auto system = GetIEditor()->GetSystem();
	gEnv = system->GetGlobalEnvironment();
}

void EDITOR_COMMON_API EditorCommon::Initialize()
{
	ModuleInitISystem(GetIEditor()->GetSystem(), "EditorCommon");

	RegisterPlugin();

	QMetaType::registerComparators<CItemModelAttribute*>();
}

void EDITOR_COMMON_API EditorCommon::Deinitialize()
{
	UnregisterPlugin();
	g_editor = nullptr;
	gEnv = nullptr;
}

