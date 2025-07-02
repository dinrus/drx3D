// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "Impl.h"

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <DrxSystem/ISystem.h>

ACE::Impl::Fmod::CImpl* g_pFmodInterface;

//------------------------------------------------------------------
extern "C" ACE_API ACE::Impl::IImpl * GetAudioInterface(ISystem * pSystem)
{
	ModuleInitISystem(pSystem, "EditorFmod");

	if (g_pFmodInterface == nullptr)
	{
		g_pFmodInterface = new ACE::Impl::Fmod::CImpl();
	}

	return g_pFmodInterface;
}

//------------------------------------------------------------------
HINSTANCE g_hInstance = 0;
BOOL __stdcall DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstance = hinstDLL;
		break;
	case DLL_PROCESS_DETACH:
		if (g_pFmodInterface != nullptr)
		{
			delete g_pFmodInterface;
			g_pFmodInterface = nullptr;
		}
		break;
	}
	return TRUE;
}

