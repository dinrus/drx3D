// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <drx3D/Sys/IImeUpr.h>
#include <drx3D/Sys/IWindowMessageHandler.h>

// IME manager utility
// This class is responsible for handling IME specific window messages
class CImeUpr : public IImeUpr, public IWindowMessageHandler
{
	// No copy/assign
	CImeUpr(const CImeUpr&);
	void operator=(const CImeUpr&);

public:
	CImeUpr();
	virtual ~CImeUpr();

	// Check if IME is supported
	virtual bool IsImeSupported() { return m_pScaleformMessageHandler != NULL; }

	// This is called by Scaleform in the case that IME support is compiled in
	// Returns false if IME should not be used
	virtual bool SetScaleformHandler(IWindowMessageHandler* pHandler);

#if DRX_PLATFORM_WINDOWS
	// IWindowMessageHandler
	virtual void PreprocessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool HandleMessage(HWND hWnd, UINT UMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	// ~IWindowMessageHandler
#endif

private:
	IWindowMessageHandler* m_pScaleformMessageHandler; // If Scaleform IME is enabled, this is not NULL
};
