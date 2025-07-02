// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Input implementation for Linux using SDL
   -------------------------------------------------------------------------
   История:
   - Jun 09,2006:	Created by Sascha Demetrio

*************************************************************************/
#ifndef __LINUXINPUT_H__
#define __LINUXINPUT_H__
#pragma once

#ifdef USE_LINUXINPUT

	#include "BaseInput.h"
	#include "InputDevice.h"

class CSDLPadUpr;
class CSDLMouse;
struct ILog;

	#if !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_IOS
		#define SDL_USE_HAPTIC_FEEDBACK
	#endif

class CLinuxInput : public CBaseInput
{
public:
	CLinuxInput(ISystem* pSystem);

	virtual bool Init() override;
	virtual void ShutDown() override;
	virtual void Update(bool focus) override;
	virtual bool GrabInput(bool bGrab) override;
	virtual i32  ShowCursor(const bool bShow) override;

private:
	ISystem*        m_pSystem;
	ILog*           m_pLog;
	CSDLPadUpr* m_pPadUpr;
	CSDLMouse*      m_pMouse;
};

class CLinuxInputDevice : public CInputDevice
{
public:
	CLinuxInputDevice(CLinuxInput& input, tukk deviceName);
	virtual ~CLinuxInputDevice();

	CLinuxInput& GetLinuxInput() const;
protected:
	void         PostEvent(SInputSymbol* pSymbol, unsigned keyMod = ~0);
private:
	CLinuxInput& m_linuxInput;

};

struct ILog;
struct ICVar;

#endif

#endif

// vim:ts=2:sw=2:tw=78
