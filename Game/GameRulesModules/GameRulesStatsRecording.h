// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: 
Game rules module to allow a game mode based on module to use the IGameStatistics
interface
-------------------------------------------------------------------------
История:
- 19:10:2009  : Created by Mark Tully

*************************************************************************/

#ifndef __GAMERULESSTATSRECORDING_H__
#define __GAMERULESSTATSRECORDING_H__

#include <drx3D/Game/IGameRulesStatsRecording.h>

struct IGameStatistics;
struct IActor;
class CStatsRecordingMgr;

class CGameRulesStatsRecording : public IGameRulesStatsRecording
{
	protected:
		IGameStatistics		*m_gameStatistics;
		IActorSystem		*m_actorSystem;
		CStatsRecordingMgr	*m_statsMgr;

	public:
							CGameRulesStatsRecording() :
								m_gameStatistics(NULL),
								m_actorSystem(NULL),
								m_statsMgr(NULL)
							{
							}

		virtual void		Init(
								XmlNodeRef		xml);
		virtual void		OnInGameBegin();
		virtual void		OnPostGameBegin();
		virtual void		OnGameEnd();
		virtual void		OnPlayerRevived(
								IActor			*inActor);
		virtual void		OnPlayerKilled(
								IActor			*inActor);

#if USE_PC_PREMATCH
		virtual void OnPrematchEnd(
								IActor			*inActor);
#endif
};

#endif // __GAMERULESSTATSRECORDING_H__

