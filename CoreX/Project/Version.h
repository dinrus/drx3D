// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   version.h
//  Version:     v1.00
//  Created:     16/09/2013 by Leander Beernaert
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   All
//  Описание: Contains the build version of the Engine
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __version_h__
#define __version_h__

// The macros below define the engine version. Their names are self-
// -explanatory. These macros can be overriden by defines in two ways:
// 1) Hard coded - Define each of the DRXENGINE_VERSION_* in DrxCommon/Version_override.h
// 2) Config tool - Define eacho of the DRXENGINE_CONFIG_VERSION_*
//
// If there are no defines, all the values will default to 0
//

//#include "Version_override.h"

#ifndef DRXENGINE_VERSION_MAJOR
	#ifndef DRXENGINE_CONFIG_VERSION_MAJOR
		#define DRXENGINE_VERSION_MAJOR 0
	#else
		#define DRXENGINE_VERSION_MAJOR DRXENGINE_CONFIG_VERSION_MAJOR
	#endif
#endif

#ifndef DRXENGINE_VERSION_MINOR
	#ifndef DRXENGINE_CONFIG_VERSION_MINOR
		#define DRXENGINE_VERSION_MINOR 0
	#else
		#define DRXENGINE_VERSION_MINOR DRXENGINE_CONFIG_VERSION_MINOR
	#endif
#endif

#ifndef DRXENGINE_VERSION_REVISION
	#ifndef DRXENGINE_CONFIG_VERSION_REVISION
		#define DRXENGINE_VERSION_REVISION 0
	#else
		#define DRXENGINE_VERSION_REVISION DRXENGINE_CONFIG_VERSION_REVISION
	#endif
#endif

#ifndef DRXENGINE_VERSION_BUILDNUM
	#ifndef DRXENGINE_CONFIG_VERSION_BUILDNUM
		#define DRXENGINE_VERSION_BUILDNUM 0
	#else
		#define DRXENGINE_VERSION_BUILDNUM DRXENGINE_CONFIG_VERSION_BUILDNUM
	#endif
#endif

#endif // __version_h__
