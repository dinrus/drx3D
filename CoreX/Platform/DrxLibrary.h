// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef DRXLIBRARY_H__
#define DRXLIBRARY_H__

/*!
   DrxLibrary

   Convenience-Macros which abstract the use of DLLs/shared libraries in a platform independent way.
   A short explanation of the different macros follows:

   DrxSharedLibrarySupported:
    This macro can be used to test if the current active platform supports shared library calls. The default
    value is false. This gets redefined if a certain platform (Windows or Linux) is desired.

   DrxSharedLibraryPrefix:
    The default prefix which will get prepended to library names in calls to DrxLoadLibraryDefName
    (see below).

   DrxSharedLibraryExtension:
    The default extension which will get appended to library names in calls to DrxLoadLibraryDefName
    (see below).

   DrxLoadLibrary(libName):
    Loads a shared library.

   DrxLoadLibraryDefName(libName):
    Loads a shared library. The platform-specific default library prefix and extension are appended to the libName.
    This allows writing of somewhat platform-independent library loading code and is therefore the function
    which should be used most of the time, unless some special extensions are used (e.g. for plugins).

   DrxGetProcAddress(libHandle, procName):
    Import function from the library presented by libHandle.

   DrxFreeLibrary(libHandle):
    Unload the library presented by libHandle.

   ИСТОРИЯ:
    03.03.2004 MarcoK
      - initial version
      - added to DrxPlatform
 */

#include <stdio.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO
	#if DRX_PLATFORM_WINDOWS
		#define DrxLoadLibrary(libName) ::LoadLibraryA(libName)
	#elif DRX_PLATFORM_DURANGO
		#define DrxLoadLibrary(libName) ::LoadLibraryExA(libName, 0, 0)
	#endif
	#define DrxGetCurrentModule() ::GetModuleHandle(nullptr)
	#define DrxSharedLibrarySupported true
	#define DrxSharedLibraryPrefix    ""
	#define DrxSharedLibraryExtension ".dll"
	#define DrxGetProcAddress(libHandle, procName) ::GetProcAddress((HMODULE)(libHandle), procName)
	#define DrxFreeLibrary(libHandle)              ::FreeLibrary((HMODULE)(libHandle))
#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#include <dlfcn.h>
	#include <stdlib.h>
	#include "platform.h"

// for compatibility with code written for windows
	#define DrxSharedLibrarySupported   true
	#define DrxSharedLibraryPrefix      "lib"
	#if DRX_PLATFORM_APPLE
		#define DrxSharedLibraryExtension ".dylib"
	#else
		#define DrxSharedLibraryExtension ".so.1.0"
	#endif

	#define DrxGetProcAddress(libHandle, procName) ::dlsym(libHandle, procName)
	#define DrxFreeLibrary(libHandle)              ::dlclose(libHandle)
	#define DrxGetCurrentModule()                  ::dlopen(NULL, RTLD_LAZY)
	#define HMODULE uk 
static tukk gEnvName("MODULE_PATH");

static tukk GetModulePath()
{
	return getenv(gEnvName);
}

static void SetModulePath(tukk pModulePath)
{
	setenv(gEnvName, pModulePath ? pModulePath : "./", true);
}

static HMODULE DrxLoadLibrary(tukk libName, bool bLazy = false, bool bInModulePath = true)
{
	char finalPath[_MAX_PATH] = {};
	DRX_ASSERT(strlen(libName) > DRX_ARRAY_COUNT(DrxSharedLibraryPrefix));
	DRX_ASSERT(strlen(libName) > DRX_ARRAY_COUNT(DrxSharedLibraryExtension));

#if DRX_PLATFORM_ANDROID
	tukk libPath = bInModulePath ? (DrxGetSharedLibraryStoragePath() ? DrxGetSharedLibraryStoragePath() : ".") : "";
#else
	tukk libPath = bInModulePath ? (GetModulePath() ? GetModulePath() : ".") : "";
#endif

	tukk filePre = strncmp(libName, DrxSharedLibraryPrefix, DRX_ARRAY_COUNT(DrxSharedLibraryPrefix) - 1) != 0 ? DrxSharedLibraryPrefix : "";
	tukk fileExt = strcmp(libName + strlen(libName) - (DRX_ARRAY_COUNT(DrxSharedLibraryExtension) - 1), DrxSharedLibraryExtension) != 0 ? DrxSharedLibraryExtension : "";

	drx_sprintf(finalPath, "%s%s%s%s%s", libPath, libPath ? "/" : "", filePre, libName, fileExt);

	#if DRX_PLATFORM_LINUX
	return ::dlopen(finalPath, (bLazy ? RTLD_LAZY : RTLD_NOW) | RTLD_DEEPBIND);
	#else
	return ::dlopen(finalPath, bLazy ? RTLD_LAZY : RTLD_NOW);
	#endif
}
#else
	#define DrxSharedLibrarySupported false
	#define DrxSharedLibraryPrefix    ""
	#define DrxSharedLibraryExtension ""
	#define DrxLoadLibrary(libName)                NULL
	#define DrxGetProcAddress(libHandle, procName) NULL
	#define DrxFreeLibrary(libHandle)
	#define GetModuleHandle(x)                     0
	#define DrxGetCurrentModule()                  NULL

#endif

#define DrxLibraryDefName(libName)               DrxSharedLibraryPrefix libName DrxSharedLibraryExtension
#define DrxLoadLibraryDefName(libName)           DrxLoadLibrary(DrxLibraryDefName(libName))

// RAII helper to load a dynamic library and free it at the end of the scope.
class CDrxLibrary
{
public:
	CDrxLibrary(tukk szLibraryPath)
		: m_hModule(nullptr)
	{
		if (szLibraryPath != nullptr)
		{
			m_hModule = DrxLoadLibrary(szLibraryPath);
		}
	}

	CDrxLibrary(const CDrxLibrary& other) = delete;

	CDrxLibrary(CDrxLibrary&& other)
		: m_hModule(std::move(other.m_hModule))
	{
		other.m_hModule = nullptr;
	}

	~CDrxLibrary()
	{
		Free();
	}

	void Free()
	{
		if (m_hModule != nullptr)
		{
			DrxFreeLibrary(m_hModule);
			m_hModule = nullptr;
		}
	}

	void ReleaseOwnership() { m_hModule = nullptr; }
	bool IsLoaded() const { return m_hModule != nullptr; }

	template<typename TProcedure>
	TProcedure GetProcedureAddress(tukk szName)
	{
		return (TProcedure)DrxGetProcAddress(m_hModule, szName);
	}

	void Set(tukk szLibraryPath)
	{
		if (m_hModule != nullptr)
		{
			DrxFreeLibrary(m_hModule);
			m_hModule = nullptr;
		}

		if (szLibraryPath != nullptr)
		{
			m_hModule = DrxLoadLibrary(szLibraryPath);
		}
	}

	HMODULE m_hModule;
};

#endif //DRXLIBRARY_H__
