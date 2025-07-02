// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <limits>

#include <drx3D/Entity/IEntity.h>
#include <drx3D/Network/INetwork.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>
#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/Math.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema2/TemplateUtils_ArrayView.h>
#include <drx3D/Schema2/TemplateUtils_PreprocessorUtils.h>
#include <drx3D/Schema2/TemplateUtils_ScopedConnection.h>
#include <drx3D/Schema2/TemplateUtils_TypeWrapper.h>
#include <drx3D/Schema2/TemplateUtils_Signal.h>
#include <drx3D/Schema2/GUID.h>

#ifdef _RELEASE
#define SXEMA2_DEBUG_BREAK
#else
#define SXEMA2_DEBUG_BREAK DrxDebugBreak();
#endif

#define SXEMA2_ASSERTS_ENABLED 1

#define SXEMA2_FILE_NAME __FILE__

#if defined(SXEMA2_EXPORTS)
#define SXEMA2_API __declspec(dllexport)
#else
#define SXEMA2_API __declspec(dllimport)
#endif

// Copied from DrxTpeInfo.cpp.
// #SchematycTODO : Find a better home for this?
#if defined(LINUX) || defined(flagLINUX) || defined(APPLE) || defined(ORBIS)

inline tuk SXEMA2_I64TOA(int64 value, tuk szOutput, i32 radix)
{
	if(radix == 10)
	{
		sprintf(szOutput, "%llu", static_cast<zu64>(value));
	}
	else
	{
		sprintf(szOutput, "%llx", static_cast<zu64>(value));
	}
	return szOutput;
}

inline tuk SXEMA2_ULTOA(u32 value, tuk szOutput, i32 radix)
{
	if(radix == 10)
	{
		sprintf(szOutput, "%.d", value);
	}
	else
	{
		sprintf(szOutput, "%.x", value);
	}
	return szOutput;
}

#else

#define SXEMA2_I64TOA _i64toa
#define SXEMA2_ULTOA  ultoa

#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif
