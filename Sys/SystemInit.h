// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/System.h>
#if DRX_PLATFORM_ANDROID && defined(DEDICATED_SERVER)
	#include <drx3D/Sys/AndroidConsole.h>
#endif

#include <drx3D/Sys/UnixConsole.h>
#ifdef USE_UNIXCONSOLE
extern __attribute__((visibility("default"))) CUNIXConsole* pUnixConsole;
#endif
