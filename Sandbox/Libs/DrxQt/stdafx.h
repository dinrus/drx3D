// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if _MSC_VER == 1900
// Disabling warnings related to member variables of classes that are exported with SANDBOX_API not being exported as well.
	#pragma warning(disable: 4251)
// Disabling warnings related to base classes of classes that are exported with SANDBOX_API not being exported as well.
	#pragma warning(disable: 4275)
#endif

#if defined(DrxQt_EXPORTS) && !defined(QToolWindowManager_EXPORTS)
	#define QToolWindowManager_EXPORTS
#endif

