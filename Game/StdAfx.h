// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
 -------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание:	include file for standard system include files,	or project
								specific include files that are used frequently, but are
								changed infrequently

 -------------------------------------------------------------------------
	История:
	- 20:7:2004   10:51 : Created by Marco Koegler
	- 3:8:2004		15:00 : Taken-over by M�rcio Martins

*************************************************************************/
#if !defined(AFX_STDAFX_H__B36C365D_F0EA_4545_B3BC_1E0EAB3B5E43__INCLUDED_)
#define AFX_STDAFX_H__B36C365D_F0EA_4545_B3BC_1E0EAB3B5E43__INCLUDED_


//#define _CRTDBG_MAP_ALLOC

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_Game
#define RWI_NAME_TAG "RayWorldIntersection(Game)"
#define PWI_NAME_TAG "PrimitiveWorldIntersection(Game)"

// Insert your headers here
#include <drx3D/CoreX/Platform/platform.h>
#include <algorithm>
#include <vector>
#include <memory>
#include <list>
#include <map>
#include <functional>
#include <limits>


#include <drx3D/CoreX/smartptr.h>

#include <drx3D/CoreX/Containers/VectorSet.h>
#include <drx3D/CoreX/StlUtils.h>

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Camera.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/CoreX/ParticleSys/IParticles.h>
#include <drx3D/Input/IInput.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Act/IGameplayRecorder.h>
#include <drx3D/Network/ISerialize.h>

#include <drx3D/Game/DrxMacros.h>

#include <drx3D/CoreX/Game/GameUtils.h>

#include <drx3D/Sys/FrameProfiler_JobSystem.h>	// to be removed

#ifndef GAMEDLL_EXPORTS
#define GAMEDLL_EXPORTS
#endif

#ifdef GAMEDLL_EXPORTS
#define GAME_API DLL_EXPORT
#else
#define GAME_API
#endif

# ifdef _DEBUG
#  ifndef DEBUG
#    define DEBUG
#  endif
#endif

#define MAX_PLAYER_LIMIT 16

#include <drx3D/Game/Game.h>

//////////////////////////////////////////////////////////////////////////
//! Reports a Game Warning to validator with WARNING severity.
inline void GameWarning( tukk format,... ) PRINTF_PARAMS(1, 2);
inline void GameWarning( tukk format,... )
{
	if (!format)
		return;
	va_list args;
	va_start(args, format);
	GetISystem()->WarningV( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,0,NULL,format,args );
	va_end(args);
}

extern struct SCVars *g_pGameCVars;

//---------------------------------------------------------------------
inline float LinePointDistanceSqr(const Line& line, const Vec3& point, float zScale = 1.0f)
{
	Vec3 x0=point;
	Vec3 x1=line.pointonline;
	Vec3 x2=line.pointonline+line.direction;

	x0.z*=zScale;
	x1.z*=zScale;
	x2.z*=zScale;

	return ((x2-x1).Cross(x1-x0)).GetLengthSquared()/(x2-x1).GetLengthSquared();
}
/*
inline IEntityProxy* GetOrMakeProxy(IEntity *pEntity, EEntityProxy proxyType)
{
	IEntityProxy* pProxy = pEntity->GetProxy(proxyType);
	if (!pProxy)
	{
		if (pEntity->CreateProxy(proxyType))
			pProxy = pEntity->GetProxy(proxyType);
	}
	return pProxy;
}
*/
#if !defined(_RELEASE)
#	define ENABLE_DEBUG_FLASH_PLAYBACK
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B36C365D_F0EA_4545_B3BC_1E0EAB3B5E43__INCLUDED_)

