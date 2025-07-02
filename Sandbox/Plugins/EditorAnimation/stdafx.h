// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CRY Stuff ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#include <drx3D/CoreX/Platform/platform.h>

#define DRX_USE_MFC
#include <drx3D/CoreX/Platform/DrxAtlMfc.h>

#include <stdlib.h>
#define INCLUDE_SAVECGF

/////////////////////////////////////////////////////////////////////////////
// STL
/////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <memory>

namespace physics_editor {
using std::vector;
using std::pair;
using std::unique_ptr;
};

/////////////////////////////////////////////////////////////////////////////
// CRY Stuff ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#define TSmartPtr _smart_ptr
#define SMARTPTR_TYPEDEF(Class) typedef _smart_ptr<Class> Class ## Ptr

#include <DrxSystem/ISystem.h>
#include "Util/EditorUtils.h"
#include "IEditor.h"
#include "EditorCommon.h"

IEditor* GetIEditor();

void     Log(tukk format, ...);

// all these are needed just for CGFLoader.cpp
#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/Renderer/VertexFormats.h>

#include <qt_windows.h>

