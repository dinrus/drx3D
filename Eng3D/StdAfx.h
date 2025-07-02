// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   stdafx.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_STDAFX_H__8B93AD4E_EE86_4127_9BED_37AC6D0F978B__INCLUDED_3DENGINE)
#define AFX_STDAFX_H__8B93AD4E_EE86_4127_9BED_37AC6D0F978B__INCLUDED_3DENGINE

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule   eDrxM_3DEngine
#define RWI_NAME_TAG "RayWorldIntersection(3dEngine)"
#define PWI_NAME_TAG "PrimitiveWorldIntersection(3dEngine)"
#include <drx3D/CoreX/Platform/platform.h>

#define DRX3DENGINE_EXPORTS

i32k nThreadsNum = 3;

//#define DEFINE_MODULE_NAME "DinrusX3dEng"

//////////////////////////////////////////////////////////////////////////////////////////////
// Highlevel defines

// deferred cull queue handling - currently disabled
// #define USE_CULL_QUEUE

// Drxsis3 as it's dx11 only can simply use the zbufferculler everywhere
// Older CCoverageBuffer currently does not compile
#if 1
	#define OCCLUSIONCULLER CZBufferCuller
#endif

// Compilation (Export to Engine) not needed on consoles
#if DRX_PLATFORM_DESKTOP
	#define ENGINE_ENABLE_COMPILATION 1
#else
	#define ENGINE_ENABLE_COMPILATION 0
#endif

#if !defined(_RELEASE) && DRX_PLATFORM_WINDOWS
	#define ENABLE_CONSOLE_MTL_VIZ
#endif

#pragma warning( error: 4018 )

#include <stdio.h>

#define MAX_PATH_LENGTH 512

#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/IProcess.h>
#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Camera.h>
#include <drx3D/CoreX/Math/Drx_XOptimise.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Phys/IPhysics.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Eng3D/IRenderNode.h>
#include <drx3D/CoreX/Containers/StackContainer.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Sys/DrxFile.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/Eng3D/DrxHeaders.h>
#include <drx3D/Eng3D/DinrusX3dEngBase.h>
#include <float.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include "cvars.h"
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/StlUtils.h>
#include "Array2d.h"
#include "Material.h"
#include <drx3D/Eng3D/3dEngine.h>
#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/Vegetation.h>
#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/ObjectsTree.h>

#include <drx3D/Sys/FrameProfiler_JobSystem.h>  // to be removed

template<class T>
void AddToPtr(byte*& pPtr, T& rObj, EEndian eEndian)
{
	PREFAST_SUPPRESS_WARNING(6326) static_assert((sizeof(T) % 4) == 0, "Invalid type size!");
	assert(!((INT_PTR)pPtr & 3));
	memcpy(pPtr, &rObj, sizeof(rObj));
	SwapEndian(*(T*)pPtr, eEndian);
	pPtr += sizeof(rObj);
	assert(!((INT_PTR)pPtr & 3));
}

template<class T>
void AddToPtr(byte*& pPtr, i32& nDataSize, T& rObj, EEndian eEndian)
{
	PREFAST_SUPPRESS_WARNING(6326) static_assert((sizeof(T) % 4) == 0, "Invalid type size!");
	assert(!((INT_PTR)pPtr & 3));
	memcpy(pPtr, &rObj, sizeof(rObj));
	SwapEndian(*(T*)pPtr, eEndian);
	pPtr += sizeof(rObj);
	nDataSize -= sizeof(rObj);
	assert(nDataSize >= 0);
	assert(!((INT_PTR)pPtr & 3));
}

inline void FixAlignment(byte*& pPtr, i32& nDataSize)
{
	while ((UINT_PTR)pPtr & 3)
	{
		*pPtr = 222;
		pPtr++;
		nDataSize--;
	}
}

inline void FixAlignment(byte*& pPtr)
{
	while ((UINT_PTR)pPtr & 3)
	{
		*pPtr = 222;
		pPtr++;
	}
}

template<class T>
void AddToPtr(byte*& pPtr, i32& nDataSize, const T* pArray, i32 nElemNum, EEndian eEndian, bool bFixAlignment = false)
{
	assert(!((INT_PTR)pPtr & 3));
	memcpy(pPtr, pArray, nElemNum * sizeof(T));
	SwapEndian((T*)pPtr, nElemNum, eEndian);
	pPtr += nElemNum * sizeof(T);
	nDataSize -= nElemNum * sizeof(T);
	assert(nDataSize >= 0);

	if (bFixAlignment)
		FixAlignment(pPtr, nDataSize);
	else
		assert(!((INT_PTR)pPtr & 3));
}

template<class T>
void AddToPtr(byte*& pPtr, const T* pArray, i32 nElemNum, EEndian eEndian, bool bFixAlignment = false)
{
	assert(!((INT_PTR)pPtr & 3));
	memcpy(pPtr, pArray, nElemNum * sizeof(T));
	SwapEndian((T*)pPtr, nElemNum, eEndian);
	pPtr += nElemNum * sizeof(T);

	if (bFixAlignment)
		FixAlignment(pPtr);
	else
		assert(!((INT_PTR)pPtr & 3));
}

struct TriangleIndex
{
	TriangleIndex() { ZeroStruct(*this); }
	u16&       operator[](i32k& n)       { assert(n >= 0 && n < 3); return idx[n]; }
	u16k& operator[](i32k& n) const { assert(n >= 0 && n < 3); return idx[n]; }
	u16        idx[3];
	u16        nCull;
};

#define FUNCTION_PROFILER_3DENGINE DRX_PROFILE_FUNCTION(PROFILE_3DENGINE)

#if DRX_PLATFORM_DESKTOP
	#define INCLUDE_SAVECGF
#endif

#define ENABLE_TYPE_INFO_NAMES 1

#endif // !defined(AFX_STDAFX_H__8B93AD4E_EE86_4127_9BED_37AC6D0F978B__INCLUDED_3DENGINE)
