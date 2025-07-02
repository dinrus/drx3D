// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "Impl.h"

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <DrxSystem/ISystem.h>

ACE::Impl::Wwise::CImpl* g_pWwiseInterface;

//------------------------------------------------------------------
extern "C" ACE_API ACE::Impl::IImpl * GetAudioInterface(ISystem * pSystem)
{
	ModuleInitISystem(pSystem, "EditorWwise");

	if (g_pWwiseInterface == nullptr)
	{
		g_pWwiseInterface = new ACE::Impl::Wwise::CImpl();
	}

	return g_pWwiseInterface;
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
		if (g_pWwiseInterface != nullptr)
		{
			delete g_pWwiseInterface;
			g_pWwiseInterface = nullptr;
		}
		break;
	}
	return TRUE;
}

