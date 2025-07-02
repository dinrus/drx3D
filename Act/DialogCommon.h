// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DialogCommon.h
//  Version:     v1.00
//  Created:     07/07/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Dialog Common Include file
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DIALOGCOMMON_H__
#define __DIALOGCOMMON_H__

#pragma once

#include <drx3D/Sys/ISystem.h>
#include "DialogSystem.h"

namespace DiaLOG
{
enum ELevel
{
	eAlways = 1,
	eDebugA = 2,
	eDebugB = 3,
	eDebugC = 4
};

inline void Log(DiaLOG::ELevel level, tukk , ...) PRINTF_PARAMS(2, 3);

//////////////////////////////////////////////////////////////////////////
//! Reports a Game Warning to validator with WARNING severity.
inline void Log(DiaLOG::ELevel level, tukk format, ...)
{
	if (gEnv->pSystem && (i32)level <= CDialogSystem::sDiaLOGLevel)
	{
		va_list args;
		va_start(args, format);
		gEnv->pLog->LogV(ILog::eMessage, format, args);
		va_end(args);
	}
}

};

#endif
