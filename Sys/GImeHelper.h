// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <drx3D/Sys/ConfigScaleform.h>
#if defined(INCLUDE_SCALEFORM_SDK) && defined(USE_GFX_IME)
	#include "GFxIMEUpr.h"
	#include <drx3D/Sys/IWindowMessageHandler.h>

	#if DRX_PLATFORM_WINDOWS
// Note: This file is part of the IME plug-in for Scaleform, and requires linking against the GFxIME.lib
		#include "GFxIMEUprWin32.h"
typedef GFxIMEUprWin32 GFxImeUprBase;
	#else
		#error No IME implementation on this platform
	#endif

// Helper for Scaleform IME
class GImeHelper : public GFxImeUprBase, public IWindowMessageHandler
{
public:
	GImeHelper();
	~GImeHelper();
	bool ApplyToLoader(GFxLoader* pLoader);
	bool ForwardEvent(GFxIMEEvent& event);
	void SetImeFocus(GFxMovieView* pMovie, bool bSet);

	#if DRX_PLATFORM_WINDOWS
	// IWindowMessageHandler
	void PreprocessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	// ~IWindowMessageHandler
	#endif

private:
	// Get the current movie
	GFxMovieView* m_pCurrentMovie;
};

#endif //defined(INCLUDE_SCALEFORM_SDK) && defined(USE_GFX_IME)
