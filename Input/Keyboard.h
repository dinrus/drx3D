// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Keyboard for Windows/DirectX
   -------------------------------------------------------------------------
   История:
   - Dec 05,2005:	Major rewrite by Marco Koegler

*************************************************************************/

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#pragma once

#ifdef USE_DXINPUT

	#include "DXInputDevice.h"

class CDXInput;
struct  SInputSymbol;

class CKeyboard : public CDXInputDevice
{
	struct SScanCode
	{
		u32 lc;    // lowercase
		u32 uc;    // uppercase
		u32 ac;    // alt gr
		u32 cl;    // caps lock (differs slightly from uppercase)
	};
public:
	CKeyboard(CDXInput& input);

	// IInputDevice overrides
	virtual i32         GetDeviceIndex() const { return 0; } //Assume only one keyboard
	virtual bool        Init();
	virtual void        Update(bool bFocus);
	virtual bool        SetExclusiveMode(bool value);
	virtual void        ClearKeyState();
	virtual u32      GetInputCharUnicode(const SInputEvent& event);
	virtual tukk GetOSKeyName(const SInputEvent& event);
	virtual void        OnLanguageChange();
	// ~IInputDevice

public:
	u32 Event2Unicode(const SInputEvent& event);
	void   RebuildScanCodeTable(HKL layout);

protected:
	void SetupKeyNames();
	void ProcessKey(u32 devSpecId, bool pressed);

private:
	static SScanCode     m_scanCodes[256];
	static SInputSymbol* Symbol[256];
	DWORD                m_baseflags;
	static i32           s_disableWinKeys;
	static CKeyboard*    s_instance;

	void        ChangeDisableWinKeys(ICVar* pVar);
	static void ChangeDisableWinKeysCallback(ICVar* pVar);
	DWORD       GetDeviceFlags() const;

};

#endif // USE_DXINPUT

#endif // __KEYBOARD_H__
