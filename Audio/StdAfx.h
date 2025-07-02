// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_AudioSystem
#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/Project/ProjectDefines.h>
#include <drx3D/Sys/ITimer.h>

#if !defined(_RELEASE)
	#define INCLUDE_AUDIO_PRODUCTION_CODE
	#define ENABLE_AUDIO_LOGGING
#endif // _RELEASE
