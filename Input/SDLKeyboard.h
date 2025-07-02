// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Keyboard Input implementation for Linux using SDL
   -------------------------------------------------------------------------
   История:
   - Aug 02,2013:	Created by Leander Beernaert

*************************************************************************/

#ifndef __SDLKEYBOARD_H__
#define __SDLKEYBOARD_H__

#include <drx3D/Input/LinuxInput.h>

class CSDLMouse;

class CSDLKeyboard : public CLinuxInputDevice
{
	friend class CLinuxInputDevice;
public:

	CSDLKeyboard(CLinuxInput& input);

	virtual ~CSDLKeyboard();

	virtual i32  GetDeviceIndex() const { return 0; }   //Assume only one keyboard

	virtual bool Init();

	virtual void Update(bool focus);

	virtual char GetInputCharAscii(const SInputEvent& event);

protected:
	static i32 ConvertModifiers(unsigned);

private:
	u8 Event2ASCII(const SInputEvent& event);
	void          SetupKeyNames();

private:
	unsigned m_lastKeySym;
	i32      m_lastMod;
	//unsigned m_lastUNICODE;

};

#endif
