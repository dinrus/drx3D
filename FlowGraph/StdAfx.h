// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define eDrxModule eDrxM_FlowGraph

#include <drx3D/CoreX/Platform/platform.h>

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Script/IScriptSystem.h>

#include <drx3D/Entity/IEntity.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Entity/IEntityComponent.h>

#include <drx3D/Phys/IPhysics.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/CoreX/Game/IGameTokens.h>
#include <drx3D/Input/IInput.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/Game/IGame.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/FlowGraph/IFlowSystem.h>

#include <drx3D/AI/IAISystem.h>

// Include DinrusAction specific headers
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Act/IViewSystem.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/IForceFeedbackSystem.h>
#include <drx3D/Act/IAnimatedCharacter.h>
#include <drx3D/Act/IItemSystem.h>

#include "HelperFunctions.h"