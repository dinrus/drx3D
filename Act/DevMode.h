// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Helper class for CDrxAction implementing developer mode-only
                functionality

   -------------------------------------------------------------------------
   История:
   - 9:2:2005   12:31 : Created by Craig Tiller

*************************************************************************/
#ifndef __DEVMODE_H__
#define __DEVMODE_H__

#pragma once

#include <drx3D/Input/IInput.h>

struct STagFileEntry
{
	Vec3 pos;
	Ang3 ang;
};

class CDevMode : public IInputEventListener, public IRemoteConsoleListener
{
public:
	CDevMode();
	~CDevMode();

	void GotoTagPoint(i32 i);
	void SaveTagPoint(i32 i);

	// IInputEventListener
	bool OnInputEvent(const SInputEvent&);
	// ~IInputEventListener

	// IRemoteConsoleListener
	virtual void OnGameplayCommand(tukk cmd);
	// ~IRemoteConsoleListener

	void GetMemoryStatistics(IDrxSizer* s) { s->Add(*this); }

private:
	bool m_bSlowDownGameSpeed;
	bool m_bHUD;
	std::vector<STagFileEntry> LoadTagFile();
	void                       SaveTagFile(const std::vector<STagFileEntry>&);
	string                     TagFileName();
	void                       SwitchSlowDownGameSpeed();
	void                       SwitchHUD();
	void                       GotoSpecialSpawnPoint(i32 i);
};

#endif
