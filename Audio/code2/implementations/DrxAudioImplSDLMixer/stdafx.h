// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_AudioImplementation
#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/Project/ProjectDefines.h>
#include <drx3D/Sys/ISystem.h>

#if !defined(_RELEASE)
	#define INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE
#endif // _RELEASE
