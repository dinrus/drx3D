// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SYNERGYKEYBOARD_H__
#define __SYNERGYKEYBOARD_H__

#ifdef USE_SYNERGY_INPUT

	#include <drx3D/Input/InputDevice.h>

struct IInput;
class CSynergyContext;

class CSynergyKeyboard : public CInputDevice
{
public:
	CSynergyKeyboard(IInput& input, CSynergyContext* pContext);
	virtual ~CSynergyKeyboard();

	// IInputDevice overrides
	virtual i32    GetDeviceIndex() const { return eIDT_Keyboard; }
	virtual bool   Init();
	virtual void   Update(bool bFocus);
	virtual u32 GetInputCharUnicode(const SInputEvent& event);
	// ~IInputDevice

private:
	_smart_ptr<CSynergyContext> m_pContext;
	void   SetupKeys();
	void   ProcessKey(u32 key, bool bPressed, bool bRepeat, u32 modificators);
	u32 PackModificators(u32 modificators);
	void   TypeASCIIString(tukk pString);
};

#endif // USE_SYNERGY_INPUT

#endif //__SYNERGYKEYBOARD_H__
