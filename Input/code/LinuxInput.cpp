// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Input implementation for Linux using SDL
   -------------------------------------------------------------------------
   История:
   - Jun 09,2006:	Created by Sascha Demetrio

*************************************************************************/
#include <drx3D/Input/StdAfx.h>

#ifdef USE_LINUXINPUT

	#include <drx3D/Input/LinuxInput.h>

	#include <math.h>

	#include <drx3D/Sys/IConsole.h>
	#include <drx3D/Sys/ILog.h>
	#include <drx3D/Sys/ISystem.h>
	#include <drx3D/CoreX/Renderer/IRenderer.h>

	#include <SDL2/SDL.h>

	#include <drx3D/Input/SDLKeyboard.h>
	#include <drx3D/Input/SDLMouse.h>
	#include <drx3D/Input/SDLPad.h>

CLinuxInput::CLinuxInput(ISystem* pSystem) : CBaseInput()
{
	m_pSystem = pSystem;
	m_pLog = pSystem->GetILog();
};

bool CLinuxInput::Init()
{
	m_pLog->Log("Initializing LinuxInput/SDL");

	u32 flags = SDL_INIT_EVENTS | SDL_INIT_JOYSTICK;
	#if defined(SDL_USE_HAPTIC_FEEDBACK)
	flags |= SDL_INIT_HAPTIC;
	#endif

	if (SDL_InitSubSystem(flags) != 0)
	{
		m_pLog->Log("Error: Initializing SDL Subsystems:%s", SDL_GetError());
		return false;
	}

	if (!CBaseInput::Init())
	{
		m_pLog->Log("Error: CBaseInput::Init failed");
		return false;
	}
	CSDLMouse* pMouse = new CSDLMouse(*this);
	if (AddInputDevice(pMouse))
	{
		m_pMouse = pMouse;
	}
	else
	{
		m_pLog->Log("Error: Initializing SDL Mouse");
		delete pMouse;
		return false;
	}
	CSDLKeyboard* pKeyboard = new CSDLKeyboard(*this);
	if (!AddInputDevice(pKeyboard))
	{
		delete pKeyboard;
		m_pLog->Log("Error: Initializing SDL Keyboard");
		return false;
	}

	m_pPadUpr = new CSDLPadUpr(*this);
	if (!m_pPadUpr->Init())
	{
		delete m_pPadUpr;
		m_pLog->Log("Error: Initializing SDL GamePad Upr");
		return false;
	}

	return true;
}

void CLinuxInput::ShutDown()
{

	m_pLog->Log("LinuxInput/SDL Shutdown");
	if (m_pPadUpr) delete m_pPadUpr;
	CBaseInput::ShutDown();
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
	delete this;
}

CLinuxInputDevice::CLinuxInputDevice(
  CLinuxInput& input,
  tukk deviceName) :
	CInputDevice(input, deviceName),
	m_linuxInput(input)
{}

CLinuxInputDevice::~CLinuxInputDevice()
{}

CLinuxInput& CLinuxInputDevice::GetLinuxInput() const
{
	return m_linuxInput;
}

void CLinuxInputDevice::PostEvent(SInputSymbol* pSymbol, unsigned keyMod)
{
	SInputEvent event;
	event.keyName = pSymbol->name;
	event.state = pSymbol->state;
	event.deviceType = pSymbol->deviceType;
	event.modifiers = CSDLKeyboard::ConvertModifiers(keyMod);
	event.value = pSymbol->value;
	event.keyId = pSymbol->keyId;
	GetLinuxInput().PostInputEvent(event);
}

void CLinuxInput::Update(bool bFocus)
{

	SDL_PumpEvents();
	m_pPadUpr->Update(bFocus);
	CBaseInput::Update(bFocus);
	SDL_Event eventList[32];
	i32 nEvents;
	nEvents = SDL_PeepEvents(eventList, 32, SDL_GETEVENT, SDL_QUIT, SDL_QUIT);
	if (nEvents == -1)
	{
		gEnv->pLog->LogError("SDL_GETEVENT error: %s", SDL_GetError());
		return;
	}
	for (i32 i = 0; i < nEvents; ++i)
	{
		if (eventList[i].type == SDL_QUIT)
		{
			gEnv->pSystem->Quit();
			return;
		}
		else
		{
			// Unexpected event type.
			abort();
		}
	}
}

bool CLinuxInput::GrabInput(bool bGrab)
{
	bool bSuccess = false;
	if (m_pMouse)
	{
		bSuccess = true;
		if (bGrab)
			m_pMouse->GrabInput();
		else
			m_pMouse->UngrabInput();
	}
	return bSuccess;
}

i32 CLinuxInput::ShowCursor(const bool bShow)
{
	static i32 displayCounter = 0;
	if (bShow)
		++displayCounter;
	else
		--displayCounter;

	SDL_ShowCursor(displayCounter >= 0 ? SDL_ENABLE : SDL_DISABLE);

	return displayCounter;
}

#endif

// vim:ts=2:sw=2:tw=78
