// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#include <drx3D/CoreX/Platform/platform.h>

#define DRX_USE_MFC
#define DRX_SUPPRESS_CRYENGINE_WINDOWS_FUNCTION_RENAMING
#define DRX_USE_XT
#include <drx3D/CoreX/Platform/DrxAtlMfc.h>

#include "Resource.h"

#include <drx3D/CoreX/functor.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <DrxSystem/ISystem.h>
#include <Drx3DEngine/I3DEngine.h>
#include <DrxEntitySystem/IEntity.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/CoreX/Extension/DrxGUID.h>

// STL headers.
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>

#include "IEditor.h"
#include "LogFile.h"

#include "EditorCommon.h"


