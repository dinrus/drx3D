// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_Editor
#undef  RWI_NAME_TAG
#define RWI_NAME_TAG "RayWorldIntersection(Editor)"
#undef  PWI_NAME_TAG
#define PWI_NAME_TAG "PrimitiveWorldIntersection(Editor)"
#include <drx3D/CoreX/Platform/platform.h>

//#define DRX_USE_XT
//#define DRX_USE_ATL
//#define DRX_SUPPRESS_CRYENGINE_WINDOWS_FUNCTION_RENAMING // Because we depend on having wrappers for GetObjectA/W and LoadLibraryA/W variants.
//#include <drx3D/CoreX/Platform/DrxAtlMfc.h>

#include <drx3D/CoreX/Project/ProjectDefines.h>
// Shell Extensions.
//#include <Shlwapi.h>

// Resource includes
//#include "Resource.h"

//////////////////////////////////////////////////////////////////////////
// Main Editor include.
//////////////////////////////////////////////////////////////////////////
#include "Plugin/IPlugin.h"
#include "EditorDefs.h"

//#include "DrxEdit.h"
#include "EditTool.h"
#include "PluginManager.h"

#include "Util/Variable.h"

#include <drx3D/CoreX/Math/ISplines.h>
#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

#include "EditorCommon.h"
#include "IEditorClassFactory.h"

