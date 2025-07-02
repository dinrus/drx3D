// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

// DrxEngine headers.

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_Legacy

#include <drx3D/CoreX/Platform/platform.h>

// MFC & XTToolkit Pro headers.
#define DRX_USE_MFC
#define DRX_USE_ATL
#define DRX_USE_XT
#include <drx3D/CoreX/Platform/DrxAtlMfc.h>

// STL headers.

#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>

// Additional DrxEngine headers.
#include <DrxSystem/ISystem.h>
#include <drx3D/CoreX/functor.h>
#include <drx3D/CoreX/SmartPtr.h>
#include <DrxSystem/File/IDrxPak.h>

#include "IEditor.h"
#include "Util/EditorUtils.h"
#include "Util/Variable.h"

#include <QMetaType>

// Schematyc headers and declarations.

#if defined(SCHEMATYC_PLUGIN_EXPORTS)
	#define SCHEMATYC_PLUGIN_API __declspec(dllexport)
#else
	#define SCHEMATYC_PLUGIN_API __declspec(dllimport)
#endif

#include <DrxSchematyc/CoreAPI.h>


Q_DECLARE_METATYPE(DrxGUID);

