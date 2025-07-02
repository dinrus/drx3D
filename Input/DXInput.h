// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	CDXInput is now an IInput implementation which is DirectInput
              specific. This removes all these ugly #ifdefs from this code,
              which is a good thing.
   -------------------------------------------------------------------------
   История:
   - Jan 31,2001:	Created by Marco Corbetta
   - Dec 01,2005:	Major rewrite by Marco Koegler
   - Dec 05,2005:	Rename CInput to CDXInput ... now platform-specific
   - Dec 18,2005:	Many refinements and abstraction

*************************************************************************/
#ifndef __DXINPUT_H__
#define __DXINPUT_H__
#pragma once

#include "BaseInput.h"
#include <drx3D/Sys/IWindowMessageHandler.h>
#include <map>
#include <queue>

#ifdef USE_DXINPUT

	#define DIRECTINPUT_VERSION 0x0800
	#include <dinput.h>

struct  ILog;
struct  ISystem;
class CKeyboard;

class CDXInput : public CBaseInput, public IWindowMessageHandler
{
public:
	CDXInput(ISystem* pSystem, HWND hwnd);
	virtual ~CDXInput();

	// IInput overrides
	virtual bool Init() override;
	virtual void Update(bool bFocus) override;
	virtual void ShutDown() override;
	virtual void ClearKeyState() override;
	virtual void SetExclusiveMode(EInputDeviceType deviceType, bool exclusive, uk pUser) override;
	virtual i32  ShowCursor(const bool bShow) override;
	// ~IInput

	HWND           GetHWnd() const        { return m_hwnd;  }
	LPDIRECTINPUT8 GetDirectInput() const { return m_pDI; }
	bool           IsImeComposing() const { return m_bImeComposing; }

private:
	// IWindowMessageHandler
	bool HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;

	// very platform specific params
	HWND             m_hwnd;
	LPDIRECTINPUT8   m_pDI;
	u16           m_highSurrogate;
	bool             m_bImeComposing;
	CKeyboard*       m_pKeyboard;
	HKL              m_lastLayout;
	static CDXInput* This;
};

#endif //USE_DXINPUT

#endif // __DXINPUT_H__
