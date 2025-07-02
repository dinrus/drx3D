// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   AppleGPUInfoUtils.h
//  Version:     v1.00
//  Created:     06/02/2014 by Leander Beernaert.
//  Описание: Utitilities to access GPU info on Mac OS X and iOS
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __APPLEGPUINFOUTILS__
#define __APPLEGPUINFOUTILS__

// Return -1 on failure and availabe VRAM otherwise
long GetVRAMForDisplay(i32k dspNum);

#endif
