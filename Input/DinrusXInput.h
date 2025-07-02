// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Here the actual input implementation gets chosen for the
              different platforms.
   -------------------------------------------------------------------------
   История:
   - Dec 05,2005: Created by Marco Koegler
   - Sep 09,2012: Updated by Dario Sancho (added support for Durango)

*************************************************************************/
#ifndef __DRXINPUT_H__
#define __DRXINPUT_H__
#pragma once

#if !defined(DEDICATED_SERVER)
	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
// Linux client (not dedicated server)
		#define USE_LINUXINPUT
		#include <drx3D/Input/LinuxInput.h>
	#elif DRX_PLATFORM_ORBIS
		#define USE_ORBIS_INPUT
		#include <drx3D/Input/OrbisInput.h>
	#elif DRX_PLATFORM_DURANGO
		#define USE_DURANGOINPUT
		#include <drx3D/Input/DurangoInput.h>
	#else
// Win32
		#define USE_DXINPUT
		#include <drx3D/Input/DXInput.h>
	#endif
	#if !defined(_RELEASE) && !DRX_PLATFORM_WINDOWS
		#define USE_SYNERGY_INPUT
	#endif
	#include "InputCVars.h"
#endif

#endif //__DRXINPUT_H__
