// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_STDAFX_H__81DAABA0_0054_42BF_8696_D99BA6832D03__INCLUDED_)
#define AFX_STDAFX_H__81DAABA0_0054_42BF_8696_D99BA6832D03__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule   eDrxM_AISystem
#define RWI_NAME_TAG "RayWorldIntersection(AI)"
#define PWI_NAME_TAG "PrimitiveWorldIntersection(AI)"

#define DRXAISYS_EXPORTS

#include <drx3D/CoreX/Platform/platform.h>

#if !defined(_RELEASE)
	#define DRXAISYS_DEBUG
	#define DRXAISYS_VERBOSITY
	#define COMPILE_WITH_MOVEMENT_SYSTEM_DEBUG
#endif

#include <stdio.h>

#include <memory>
#include <limits>
#include <vector>
#include <map>
#include <numeric>
#include <algorithm>
#include <list>
#include <set>
#include <deque>

// Reference additional interface headers your program requires here (not local headers)

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Camera.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/CoreX/Math/Drx_XOptimise.h> // required by AMD64 compiler
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Network/ISerialize.h>
#include <drx3D/FlowGraph/IFlowSystem.h>
#include <drx3D/AI/IAIAction.h>
#include <drx3D/Phys/IPhysics.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/AI/IAgent.h>
#include <drx3D/AI/IAIActorProxy.h>
#include <drx3D/AI/IAIGroupProxy.h>
#include <drx3D/Entity/IEntity.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Sys/DrxFile.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/Network/ISerialize.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/CoreX/Math/Random.h>
#include <drx3D/Sys/ICodeCheckpointMgr.h>

#include <drx3D/AI/NavigationIdTypes.h>

#include <drx3D/AI/XMLUtils.h>

//sxema includes
#include <drx3D/Schema/CoreAPI.h>

#include <drx3D/AI/Environment.h>

// Hijack the old CCCPOINT definition (and add a semi-colon to allow compilation)
#define CCCPOINT(x) CODECHECKPOINT(x);

#include <drx3D/Sys/FrameProfiler_JobSystem.h>  // to be removed

#define AI_STRIP_OUT_LEGACY_LOOK_TARGET_CODE

/// This frees the memory allocation for a vector (or similar), rather than just erasing the contents
template<typename T>
void ClearVectorMemory(T& container)
{
	T().swap(container);
}

// adding some headers here can improve build times... but at the cost of the compiler not registering
// changes to these headers if you compile files individually.
#include <drx3D/AI/AILog.h>
#include <drx3D/AI/CAISystem.h>
#include <drx3D/AI/ActorLookUp.h>
// NOTE: (MATT) Reduced this list to the minimum. {2007/07/18:16:24:59}
//////////////////////////////////////////////////////////////////////////

class CAISystem;
ILINE CAISystem* GetAISystem()
{
	extern CAISystem* g_pAISystem;

	return g_pAISystem;
}

//====================================================================
// SetAABBCornerPoints
//====================================================================
inline void SetAABBCornerPoints(const AABB& b, Vec3* pts)
{
	pts[0].Set(b.min.x, b.min.y, b.min.z);
	pts[1].Set(b.max.x, b.min.y, b.min.z);
	pts[2].Set(b.max.x, b.max.y, b.min.z);
	pts[3].Set(b.min.x, b.max.y, b.min.z);

	pts[4].Set(b.min.x, b.min.y, b.max.z);
	pts[5].Set(b.max.x, b.min.y, b.max.z);
	pts[6].Set(b.max.x, b.max.y, b.max.z);
	pts[7].Set(b.min.x, b.max.y, b.max.z);
}

ILINE float LinStep(float a, float b, float x)
{
	float w = (b - a);
	if (w != 0.0f)
	{
		x = (x - a) / w;
		return min(1.0f, max(x, 0.0f));
	}
	return 0.0f;
}

//===================================================================
// HasPointInRange
// (To be replaced)
//===================================================================
ILINE bool HasPointInRange(const std::vector<Vec3>& list, const Vec3& pos, float range)
{
	float r(sqr(range));
	for (u32 i = 0; i < list.size(); ++i)
		if (Distance::Point_PointSq(list[i], pos) < r)
			return true;
	return false;
}

#ifndef OUT
	#define OUT
#endif

#ifndef IN
	#define IN
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__81DAABA0_0054_42BF_8696_D99BA6832D03__INCLUDED_)
