// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GAMEMECHANISMEVENTS_H__
#define __GAMEMECHANISMEVENTS_H__

#include <drx3D/Game/AutoEnum.h>

#define GameMechanismEventList(f)   \
	f(kGMEvent_GameRulesInit)         \
	f(kGMEvent_GameRulesRestart)      \
	f(kGMEvent_GameRulesDestroyed)    \
	f(kGMEvent_LocalPlayerInit)       \
	f(kGMEvent_LocalPlayerDeinit)     \
	f(kGMEvent_LoadGame)              \
	f(kGMEvent_SaveGame)              \

struct SGameMechanismEventData
{
	union
	{
		struct { ILoadGame * m_interface; } m_data_LoadGame;
		struct { ISaveGame * m_interface; } m_data_SaveGame;
	};
};

AUTOENUM_BUILDENUMWITHTYPE(EGameMechanismEvent, GameMechanismEventList);

#endif //__GAMEMECHANISMEVENTS_H__