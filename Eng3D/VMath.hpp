// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Version:     v1.00
//  Created:     Michael Kopietz
//  Описание: unified vector math lib
// -------------------------------------------------------------------------
//  История:		- created 1999  for Katmai and K3
//							-	...
//							-	integrated into drxengine
//
////////////////////////////////////////////////////////////////////////////
#ifndef __VMATH__
#define __VMATH__

#define VEC4_SSE
#if DRX_PLATFORM_DURANGO
	#define VEC4_SSE4
#endif

#include <drx3D/CoreX/Math/Drx_Math.h>

#if DRX_PLATFORM_NEON
	#include "VMath_NEON.hpp"
#elif DRX_PLATFORM_SSE2 || DRX_PLATFORM_SSE4
	#include "VMath_SSE.hpp"
#else
	#include "VMath_C.hpp"
#endif

#endif
