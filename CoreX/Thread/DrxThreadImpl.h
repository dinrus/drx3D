// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Thread/DrxThread.h>

// Include architecture specific code.
#if DRX_PLATFORM_WINAPI
	#include <drx3D/CoreX/Thread/DrxAtomics_impl_win32.h>
	#include <drx3D/CoreX/Thread/DrxThreadImpl_win32.h>
#elif DRX_PLATFORM_ORBIS
	#include <drx3D/CoreX/Thread/DrxThreadImpl_sce.h>
#elif DRX_PLATFORM_POSIX
	#include <drx3D/CoreX/Thread/DrxThreadImpl_posix.h>
#else
// Put other platform specific includes here!
#endif

// vim:ts=2
