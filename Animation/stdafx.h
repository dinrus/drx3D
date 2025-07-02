// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_Animation

#define DRXANIMATION_EXPORTS
//#define NOT_USE_DRX_MEMORY_MANAGER
//#define DEFINE_MODULE_NAME "DinrusXAnimation"

//! Include standard headers.
#include <drx3D/CoreX/Platform/platform.h>

#if defined(USER_ivof) || defined(USER_ivo) || defined(USER_sven)
	#define ASSERT_ON_ASSET_CHECKS
#endif

#ifdef ASSERT_ON_ASSET_CHECKS
	#define ANIM_ASSET_ASSERT_TRACE(condition, parenthese_message) DRX_ASSERT_TRACE(condition, parenthese_message)
	#define ANIM_ASSET_ASSERT(condition)                           DRX_ASSERT(condition)
#else
	#define ANIM_ASSET_ASSERT_TRACE(condition, parenthese_message)
	#define ANIM_ASSET_ASSERT(condition)
#endif

#define LOG_ASSET_PROBLEM(condition, parenthese_message)                \
  do                                                                    \
  {                                                                     \
    gEnv->pLog->LogWarning("Anim asset error: %s failed", # condition); \
    gEnv->pLog->LogWarning parenthese_message;                          \
  } while (0)

#define ANIM_ASSET_CHECK_TRACE(condition, parenthese_message)     \
  do                                                              \
  {                                                               \
    bool anim_asset_ok = (condition);                             \
    if (!anim_asset_ok)                                           \
    {                                                             \
      ANIM_ASSET_ASSERT_TRACE(anim_asset_ok, parenthese_message); \
      LOG_ASSET_PROBLEM(condition, parenthese_message);           \
    }                                                             \
  } while (0)

#define ANIM_ASSET_CHECK_MESSAGE(condition, message) ANIM_ASSET_CHECK_TRACE(condition, (message))
#define ANIM_ASSET_CHECK(condition)                  ANIM_ASSET_CHECK_MESSAGE(condition, NULL)

//! Include main interfaces.
#include <drx3D/CoreX/Math/Drx_Math.h>

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/CoreX/StlUtils.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/AnimationBase.h>
#include <drx3D/Sys/FrameProfiler_JobSystem.h>  // to be removed
