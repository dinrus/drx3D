// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   OrbisPadWin.h
   Описание: Pad for Orbis in Windows
   ---------------------------------------------------------------------
   История:
   - 16.06.2014 : Added by Matthijs van der Meide
   - 20.05.2015 : Updated to Pad for Windows PC Games (v1.4)

 *********************************************************************/
#ifndef __ORBISPADWIN_H__
#define __ORBISPADWIN_H__
#pragma once

// If you don't want this feature at all, just:
//#undef WANT_ORBISPAD_WIN

#if _MSC_VER > 1900 && defined(WANT_ORBISPAD_WIN)
// Missing OrbisPad libs for Visual Studio 2015
	#undef WANT_ORBISPAD_WIN
	#pragma message("Unable to use OrbisPad on Windows with MSVC newer than 2015 due to missing libs.")
#endif

#if defined(USE_DXINPUT) && (DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT) && defined(WANT_ORBISPAD_WIN)
	#ifndef USE_ORBISPAD_WIN
		#define USE_ORBISPAD_WIN
	#endif
#else
	#undef USE_ORBISPAD_WIN
#endif

#ifdef USE_ORBISPAD_WIN
	#include "DinrusXInput.h"
	#include "InputDevice.h"

// Construct a new COrbisPadWin instance
extern CInputDevice* CreateOrbisPadWin(ISystem* pSystem, CBaseInput& input, i32 deviceNo);

// Destroy an existing COrbisPadWin instance
extern void DestroyOrbisPadWin(CInputDevice* pDevice);

#endif //#ifdef USE_ORBISPAD_WIN
#endif //#ifdef __ORBISPADWIN_H__
