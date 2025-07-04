// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/GImeHelper.h>
#if defined(INCLUDE_SCALEFORM_SDK) && defined(USE_GFX_IME)
	#include <drx3D/Sys/System.h>
	#include <drx3D/Sys/ImeUpr.h>
	#include <drx3D/Sys/SharedResources.h>
	#include <drx3D/Sys/SharedStates.h>

inline IImeUpr* GetSystemImeUpr(ISystem* pSystem)
{
	DRX_ASSERT(pSystem);
	return pSystem->GetImeUpr();
}

	#if DRX_PLATFORM_WINDOWS
inline HWND GetSystemHWND(ISystem* pSystem)
{
	DRX_ASSERT(pSystem);
	return static_cast<HWND>(pSystem->GetHWND());
}
	#endif

// Constructor
GImeHelper::GImeHelper() : GFxImeUprBase(GetSystemHWND(gEnv->pSystem))
{
	m_pCurrentMovie = NULL;
}

// Destructor
GImeHelper::~GImeHelper()
{
	if (gEnv && gEnv->pSystem)
	{
		IImeUpr* pImeUpr = GetSystemImeUpr(gEnv->pSystem);
		if (pImeUpr)
		{
			pImeUpr->SetScaleformHandler(NULL);
		}
	}
}

// Apply this IME helper to the Scaleform loader
bool GImeHelper::ApplyToLoader(GFxLoader* pLoader)
{
	ISystem* pSystem = gEnv->pSystem;

	if (!GetSystemImeUpr(pSystem)->SetScaleformHandler(this))
	{
		// This IME should not be used
		return false;
	}

	// Note: Renamed to .dat file because .xml files can be transformed to binary XML which can't be read by Scaleform
	string gameFolder = pSystem->GetIPak()->GetGameFolder();
	string imeXmlFile = gameFolder + "/Libs/UI/ime.dat";
	if (pLoader && Init(&DrxGFxLog::GetAccess(), &DrxGFxFileOpener::GetAccess(), imeXmlFile))
	{
		SetIMEMoviePath("IME.gfx"); // Note: Will be looked for in the Libs/UI folder
		pLoader->SetIMEUpr(this);
	}
	return true;
}

// Forward the event to the current Scaleform movie
bool GImeHelper::ForwardEvent(GFxIMEEvent& event)
{
	return m_pCurrentMovie && (m_pCurrentMovie->HandleEvent(event) & GFxMovieView::HE_NoDefaultAction) == 0;
}

void GImeHelper::SetImeFocus(GFxMovieView* pMovie, bool bSet)
{
	if (bSet && pMovie)
	{
		m_pCurrentMovie = pMovie;
		m_pCurrentMovie->HandleEvent(GFxEvent::SetFocus);
	}
	else if (!bSet && pMovie == m_pCurrentMovie)
	{
		m_pCurrentMovie = NULL;
	}
}

	#if DRX_PLATFORM_WINDOWS
// Implement IWindowMessageHandler::PreprocessMessage
void GImeHelper::PreprocessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || ImmIsUIMessageW(NULL, uMsg, wParam, lParam) == TRUE)
	{
		GFxIMEWin32Event event(GFxIMEEvent::IME_PreProcessKeyboard, (UPInt)hWnd, uMsg, wParam, lParam);
		ForwardEvent(event);
	}
}

// Implement IWindowMessageHandler::HandleMessage
bool GImeHelper::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// Find out if this needs IME handling
	switch (uMsg)
	{
	case WM_IME_SETCONTEXT:
		// This is a special case, Scaleform wants to set lParam to 0 for this, and then forward it to the OS
		// We manually forward, and then indicate that the message is handled already, since we can't modify lParam here
		DefWindowProcW(hWnd, uMsg, wParam, 0);
		*pResult = 0;
		return true;
	case WM_IME_STARTCOMPOSITION:
	case WM_IME_KEYDOWN:
	case WM_IME_COMPOSITION:
	case WM_IME_ENDCOMPOSITION:
	case WM_IME_NOTIFY:
	case WM_IME_CHAR:
	case WM_IME_CONTROL:
	case WM_IME_COMPOSITIONFULL:
	case WM_IME_SELECT:
	case WM_IME_KEYUP:
		break;
	default:
		return false;
	}

	// IME event
	GFxIMEWin32Event event(GFxIMEEvent::IME_Default, (UPInt)hWnd, uMsg, wParam, lParam);
	if (!ForwardEvent(event))
	{
		// This is handled by Scaleform
		*pResult = 0;
		return true;
	}

	// This should be forwarded to the OS
	return false;
}
	#endif // DRX_PLATFORM_WINDOWS
#endif   //defined(INCLUDE_SCALEFORM_SDK) && defined(USE_GFX_IME)
