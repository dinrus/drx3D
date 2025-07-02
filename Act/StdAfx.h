// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   DrxGame Source File.
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	include file for standard system include files,	or project
                specific include files that are used frequently, but are
                changed infrequently

   -------------------------------------------------------------------------
   История:
   - 20:7:2004   10:51 : Created by Marco Koegler

*************************************************************************/
#if !defined(AFX_STDAFX_H__B36C365D_F0EA_4545_B3BC_1E0EAB3B5E42__INCLUDED_)
#define AFX_STDAFX_H__B36C365D_F0EA_4545_B3BC_1E0EAB3B5E42__INCLUDED_

//#define _CRTDBG_MAP_ALLOC
#pragma once


#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule  eDrxM_GameFramework
#define RWI_NAME_TAG "RayWorldIntersection(Action)"
#define PWI_NAME_TAG "PrimitiveWorldIntersection(Action)"

#define DRXACTION_EXPORTS

// Insert your headers here
#include <drx3D/CoreX/Platform/platform.h>

#include <memory>
#include <vector>
#include <map>
#include <queue>

inline void GameWarning(tukk , ...) PRINTF_PARAMS(1, 2);

#include <drx3D/CoreX/Project/ProjectDefines.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Camera.h>
#include <drx3D/CoreX/Math/Random.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Network/INetwork.h>
#include <drx3D/Input/IInput.h>
#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Network/NetHelpers.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Network/IRemoteControl.h>
#include <drx3D/Network/ISimpleHttpServer.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Act/IAnimatedCharacter.h>
#include <drx3D/CoreX/Game/IGame.h>
#include <drx3D/Act/IItem.h>
#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Act/IViewSystem.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/FlowGraph/IFlowSystem.h>
#include <drx3D/Act/IGameplayRecorder.h>
#include <drx3D/CoreX/Game/GameUtils.h>

#if !defined(_RELEASE)
	#define ENABLE_NETDEBUG 1
#endif

//////////////////////////////////////////////////////////////////////////
//! Reports a Game Warning to validator with WARNING severity.
inline void GameWarning(tukk format, ...)
{
	if (!format)
		return;

	va_list args;
	va_start(args, format);
	GetISystem()->WarningV(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, 0, 0, format, args);
	va_end(args);
}

#if 1
	#define NET_USE_SIMPLE_BREAKAGE 1
#else// deprecated and won't compile
	#define NET_USE_SIMPLE_BREAKAGE 0
#endif

#if !defined(RELEASE)
	#define DRXACTION_AI_VERBOSITY
#endif

#ifdef DRXACTION_AI_VERBOSITY
	#define AIWarningID     gEnv->pAISystem->Warning
	#define AIErrorID       gEnv->pAISystem->Error
	#define AILogProgressID gEnv->pAISystem->LogProgress
	#define AILogEventID    gEnv->pAISystem->LogEvent
	#define AILogCommentID  gEnv->pAISystem->LogComment
#else
	#define AIWarningID     (void)
	#define AIErrorID       (void)
	#define AILogProgressID (void)
	#define AILogEventID    (void)
	#define AILogCommentID  (void)
#endif

inline bool IsClientActor(EntityId id)
{
	IActor* pActor = CDrxAction::GetDrxAction()->GetClientActor();
	if (pActor && pActor->GetEntity()->GetId() == id)
		return true;
	return false;
}

template<typename T> bool inline GetAttr(const XmlNodeRef& node, tukk key, T& val)
{
	return node->getAttr(key, val);
}

bool inline GetTimeAttr(const XmlNodeRef& node, tukk key, time_t& val)
{
	tukk pVal = node->getAttr(key);
	if (!pVal)
		return false;
	val = GameUtils::stringToTime(pVal);
	return true;
}

template<> bool inline GetAttr(const XmlNodeRef& node, tukk key, string& val)
{
	tukk pVal = node->getAttr(key);
	if (!pVal)
		return false;
	val = pVal;
	return true;
}

#endif // !defined(AFX_STDAFX_H__B36C365D_F0EA_4545_B3BC_1E0EAB3B5E42__INCLUDED_)
