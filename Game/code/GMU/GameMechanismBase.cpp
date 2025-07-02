// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameMechanismBase.h>
#include <drx3D/Game/GameMechanismUpr.h>

CGameMechanismBase::CGameMechanismBase(tukk  className)
{
	memset (& m_linkedListPointers, 0, sizeof(m_linkedListPointers));

#if INCLUDE_NAME_IN_GAME_MECHANISMS
	m_className = className;
#endif

	CGameMechanismUpr * manager = CGameMechanismUpr::GetInstance();
	manager->RegisterMechanism(this);
}

CGameMechanismBase::~CGameMechanismBase()
{
	CGameMechanismUpr * manager = CGameMechanismUpr::GetInstance();
	manager->UnregisterMechanism(this);
}
