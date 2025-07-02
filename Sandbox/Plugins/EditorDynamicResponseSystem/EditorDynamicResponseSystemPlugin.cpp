// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"

#include "EditorDynamicResponseSystemPlugin.h"
#include "DrsEditorMainWindow.h"

#include <IResourceSelectorHost.h>

#include <drx3D/CoreX/Serialization/IArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>

#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <IEditor.h>
#include <DrxSystem/ISystem.h>
#include <DrxDynamicResponseSystem/IDynamicResponseSystem.h>

REGISTER_PLUGIN(CEditorDynamicResponseSystemPlugin);

CEditorDynamicResponseSystemPlugin::CEditorDynamicResponseSystemPlugin()
{
	DRX_ASSERT(gEnv->pDynamicResponseSystem->GetResponseManager());
}
