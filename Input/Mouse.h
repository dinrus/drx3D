// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Mouse for Windows/DirectX
   -------------------------------------------------------------------------
   История:
   - Dec 05,2005:	Major rewrite by Marco Koegler

*************************************************************************/

#ifndef __MOUSE_H__
#define __MOUSE_H__
#pragma once

#ifdef USE_DXINPUT

	#include "DXInputDevice.h"

struct  ITimer;
struct  ILog;
struct  ICVar;
class CDXInput;

class CMouse : public CDXInputDevice
{
public:
	CMouse(CDXInput& input);

	// IInputDevice overrides
	virtual i32  GetDeviceIndex() const { return 0; } //Assume only one device of this type
	virtual bool Init();
	virtual void Update(bool bFocus);
	virtual bool SetExclusiveMode(bool value);
	// ~IInputDevice

private:
	void PostEvent(SInputSymbol* pSymbol);
	void PostOnlyIfChanged(SInputSymbol* pSymbol, EInputState newState);

	//smooth movement & mouse accel
	void CapDeltas(float cap);
	void SmoothDeltas(float accel, float decel = 0.0f);

	Vec2                 m_deltas;
	Vec2                 m_oldDeltas;
	Vec2                 m_deltasInertia;
	float                m_mouseWheel;

	const static i32     MAX_MOUSE_SYMBOLS = eKI_MouseLast - KI_MOUSE_BASE;
	static SInputSymbol* Symbol[MAX_MOUSE_SYMBOLS];
};

#endif //USE_DXINPUT

#endif // __MOUSE_H__
