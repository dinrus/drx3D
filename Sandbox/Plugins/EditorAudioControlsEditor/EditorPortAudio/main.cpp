// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "Impl.h"

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <DrxSystem/ISystem.h>

ACE::Impl::PortAudio::CImpl* g_pPortAudioInterface;

//------------------------------------------------------------------
extern "C" ACE_API ACE::Impl::IImpl * GetAudioInterface(ISystem * pSystem)
{
	ModuleInitISystem(pSystem, "EditorPortAudio");

	if (g_pPortAudioInterface == nullptr)
	{
		g_pPortAudioInterface = new ACE::Impl::PortAudio::CImpl();
	}

	return g_pPortAudioInterface;
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
		if (g_pPortAudioInterface != nullptr)
		{
			delete g_pPortAudioInterface;
			g_pPortAudioInterface = nullptr;
		}
		break;
	}
	return TRUE;
}

