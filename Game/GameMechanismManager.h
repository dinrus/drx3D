// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GAMEMECHANISMMANAGER_H__
#define __GAMEMECHANISMMANAGER_H__

#include <drx3D/Game/GameMechanismEvents.h>

class CGameMechanismBase;

class CGameMechanismUpr
{
	public:
	CGameMechanismUpr();
	~CGameMechanismUpr();
	void Update(float dt);
	void Inform(EGameMechanismEvent gmEvent, const SGameMechanismEventData * data = NULL);
	void RegisterMechanism(CGameMechanismBase * mechanism);
	void UnregisterMechanism(CGameMechanismBase * mechanism);

	static ILINE CGameMechanismUpr * GetInstance()
	{
		assert (s_instance);
		return s_instance;
	}

	private:
	static CGameMechanismUpr * s_instance;
	CGameMechanismBase * m_firstMechanism;

#if !defined(_RELEASE)
	i32 m_cvarWatchEnabled;
	i32 m_cvarLogEnabled;
#endif
};

#endif //__GAMEMECHANISMMANAGER_H__