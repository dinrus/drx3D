// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

	Plays Announcements based upon AreaBox triggers placed in levels

История:
- 25:02:2010		Created by Ben Parbury
*************************************************************************/

#ifndef __AREAANNOUNCER_H__
#define __AREAANNOUNCER_H__

#include <drx3D/Game/GameRulesModules/IGameRulesRevivedListener.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>

#define AREA_ANNOUNCERS 2

class CAreaAnnouncer : IGameRulesRevivedListener
{
public:
	CAreaAnnouncer();
	~CAreaAnnouncer();

	void Init();
	void Reset();

	void Update(const float dt);

	virtual void EntityRevived(EntityId entityId);

#if !defined(_RELEASE)
	static void CmdPlay(IConsoleCmdArgs* pCmdArgs);
	static void CmdReload(IConsoleCmdArgs* pCmdArgs);
#endif

protected:

	struct SAnnouncementArea
	{
#if !defined(_RELEASE)
		const static i32 k_maxNameLength = 64;
		char m_name[k_maxNameLength];
#endif
		EntityId m_areaProxyId;
		TAudioSignalID m_signal[AREA_ANNOUNCERS];
	};

	bool AnnouncerRequired();
	void LoadAnnouncementArea(const IEntity* pEntity, tukk areaName);

	TAudioSignalID BuildAnnouncement(const EntityId clientId);
	TAudioSignalID GenerateAnnouncement(i32k* actorCount, i32k k_areaCount, const EntityId clientId);
	i32 GetAreaAnnouncerTeamIndex(const EntityId clientId);

	const static i32 k_maxAnnouncementAreas = 16;
	DrxFixedArray<SAnnouncementArea, k_maxAnnouncementAreas> m_areaList;
};

#endif // __AREAANNOUNCER_H__