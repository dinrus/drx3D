// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>

#define eDrxModule eDrxM_LiveCreate
#define DRXLIVECREATE_EXPORTS

#include <drx3D/CoreX/Platform/platform.h>

#if defined(_DEBUG) && DRX_PLATFORM_WINAPI
	#include <crtdbg.h>
#endif

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>

#include <vector>
#include <set>
#include <map>

#if (defined(DEDICATED_SERVER) || defined(_RELEASE)) && !defined(NO_LIVECREATE)
	#define NO_LIVECREATE
#endif

#include <drx3D/LiveCreate/ILiveCreateCommon.h>
#include <drx3D/LiveCreate/ILiveCreatePlatform.h>
#include <drx3D/LiveCreate/ILiveCreateUpr.h>
#include <drx3D/LiveCreate/ILiveCreateHost.h>
