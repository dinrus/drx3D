// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SYNERGYMOUSE_H__
#define __SYNERGYMOUSE_H__

#ifdef USE_SYNERGY_INPUT

	#include <drx3D/Input/InputDevice.h>

struct  IInput;
class CSynergyContext;

enum
{
	SYNERGY_MOUSE_X,
	SYNERGY_MOUSE_Y,
	SYNERGY_MOUSE_Z,
	SYNERGY_BUTTON_L,
	SYNERGY_BUTTON_M,
	SYNERGY_BUTTON_R,
	SYNERGY_MOUSE_X_ABS,
	SYNERGY_MOUSE_Y_ABS,
	MAX_SYNERGY_MOUSE_SYMBOLS
};

class CSynergyMouse : public CInputDevice
{
public:
	CSynergyMouse(IInput& input, CSynergyContext* pContext);

	// IInputDevice overrides
	virtual i32  GetDeviceIndex() const { return eIDT_Mouse; } //Assume only one device of this type
	virtual bool Init();
	virtual void Update(bool bFocus);
	virtual bool SetExclusiveMode(bool value);
	// ~IInputDevice

private:
	u16                      m_lastX, m_lastY, m_lastZ;
	bool                        m_lastButtonL, m_lastButtonM, m_lastButtonR;
	_smart_ptr<CSynergyContext> m_pContext;

	static SInputSymbol*        Symbol[MAX_SYNERGY_MOUSE_SYMBOLS];
};

#endif // USE_SYNERGY_INPUT

#endif // __SYNERGYMOUSE_H__
