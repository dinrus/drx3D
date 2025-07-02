// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_Font

#define DRXFONT_EXPORTS

#include <drx3D/CoreX/Platform/platform.h>

#include <drx3D/Font/IFont.h>

#include <drx3D/Sys/ILog.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>

// USE_NULLFONT should be defined for all platforms running as a pure dedicated server
#if DRX_PLATFORM_DESKTOP
	#ifndef USE_NULLFONT
		#define USE_NULLFONT
	#endif
#endif
