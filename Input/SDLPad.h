// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	GamePad Input implementation for Linux using SDL
   -------------------------------------------------------------------------
   История:
   - Jan 20,2014:	Created by Leander Beernaert

*************************************************************************/

#pragma once

#if defined(USE_LINUXINPUT)

	#include  <SDL2/SDL.h>
	#include <drx3D/Input/LinuxInput.h>
// We need a manager, since all the input for each Game Pad is collected
// in the same queue. If we were to update the game pads seperately
// they would consume each other's events.

class CSDLPad : public CLinuxInputDevice
{
public:

	CSDLPad(CLinuxInput& input, i32 device);

	virtual ~CSDLPad();

	// IInputDevice overrides
	virtual i32  GetDeviceIndex() const { return m_deviceNo; }
	virtual bool Init();
	virtual void Update(bool bFocus);
	virtual void ClearAnalogKeyState(TInputSymbols& clearedSymbols);
	virtual void ClearKeyState();
	virtual bool SetForceFeedback(IFFParams params);
	// ~IInputDevice

	i32          GetInstanceId() const;

	void         HandleAxisEvent(const SDL_JoyAxisEvent& evt);

	void         HandleHatEvent(const SDL_JoyHatEvent& evt);

	void         HandleButtonEvent(const SDL_JoyButtonEvent& evt);

	void         HandleConnectionState(const bool connected);
private:
	static float DeadZoneFilter(i32 input);

	bool         OpenDevice();

	void         CloseDevice();

private:
	SDL_Joystick* m_pSDLDevice;
	SDL_Haptic*   m_pHapticDevice;
	i32           m_curHapticEffect;
	i32           m_deviceNo;
	i32           m_handle;
	bool          m_connected;
	bool          m_supportsFeedback;
	float         m_vibrateTime;
};

class CSDLPadUpr
{
public:

	CSDLPadUpr(CLinuxInput& input);

	~CSDLPadUpr();

	bool Init();

	void Update(bool bFocus);

private:
	bool     AddGamePad(i32 deviceIndex);

	bool     RemovGamePad(i32 instanceId);

	CSDLPad* FindPadByInstanceId(i32 instanceId);
	CSDLPad* FindPadByDeviceIndex(i32 deviceIndex);
private:

	typedef std::vector<CSDLPad*> GamePadVector;

	CLinuxInput&  m_rLinuxInput;
	GamePadVector m_gamePads;
};

#endif
