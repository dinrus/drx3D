// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   stdafx.h
//  Version:     v1.00
//  Created:     30/9/2002 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Precompiled Header.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __stdafx_h__
#define __stdafx_h__

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_System

#define DRXSYSTEM_EXPORTS

#include <drx3D/CoreX/Platform/platform.h>

//////////////////////////////////////////////////////////////////////////
// CRT
//////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_APPLE && !DRX_PLATFORM_ORBIS
	#include <memory.h>
	#include <malloc.h>
#endif
#include <fcntl.h>

#if !DRX_PLATFORM_ORBIS && !DRX_PLATFORM_APPLE && !DRX_PLATFORM_ANDROID
	#if DRX_PLATFORM_LINUX
		#include <sys/io.h>
	#else
		#include <io.h>
	#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// DRX Stuff ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Camera.h>
#include <drx3D/CoreX/Math/Random.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/Math/Range.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/StlUtils.h>

static inline i32 RoundToClosestMB(size_t memSize)
{
	// add half a MB and shift down to get closest MB
	return((i32) ((memSize + (1 << 19)) >> 20));
}

//////////////////////////////////////////////////////////////////////////
// For faster compilation
//////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Sys/DrxFile.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/CoreX/Audio/IAudioSystem.h>
#include <drx3D/Phys/IPhysics.h>
#include <drx3D/AI/IAISystem.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/Sys/ICmdLine.h>
#include <drx3D/Input/IInput.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Sys/ILog.h>

#include <drx3D/Sys/FrameProfiler_JobSystem.h>  // to be removed

/////////////////////////////////////////////////////////////////////////////
//forward declarations for common Interfaces.
/////////////////////////////////////////////////////////////////////////////
class ITexture;
struct IRenderer;
struct ISystem;
struct IScriptSystem;
struct ITimer;
struct IFFont;
struct IInput;
struct IKeyboard;
struct ICVar;
struct IConsole;
struct IEntitySystem;
struct IProcess;
struct IDrxPak;
struct IDrxFont;
struct I3DEngine;
struct IMovieSystem;
struct IPhysicalWorld;

/////////////////////////////////////////////////////////////////////////////
// HMD libraries
/////////////////////////////////////////////////////////////////////////////
#include<drx3D/Sys/IHMDDevice.h>

#endif // __stdafx_h__
