// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание:
Network-syncs the actor entity ID being targeted by a player
**************************************************************************/

#ifndef __PLAYERPLUGIN_CURRENTLYTARGETTING_H__
#define __PLAYERPLUGIN_CURRENTLYTARGETTING_H__

#include <drx3D/Game/PlayerPlugin.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>

class CPlayerPlugin_CurrentlyTargetting : public CPlayerPlugin
{
	public:
		SET_PLAYER_PLUGIN_NAME(CPlayerPlugin_CurrentlyTargetting);

		CPlayerPlugin_CurrentlyTargetting();

		ILINE EntityId GetCurrentTargetEntityId() const
		{
			return m_currentTarget;
		}

		ILINE const float GetCurrentTargetTime() const
		{
			return m_currentTargetTime;
		}

		virtual void NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags);
	private:
		virtual void Update(const float dt);
		virtual void HandleEvent(EPlayerPlugInEvent theEvent, uk  data);
		virtual void Enter(CPlayerPluginEventDistributor* pEventDist);
		virtual void Leave();

		CAudioSignalPlayer m_targetedSignal;
		EntityId m_currentTarget;
		float m_currentTargetTime;
		bool m_bTargetingLocalPlayer;
};

#endif // __PLAYERPLUGIN_CURRENTLYTARGETTING_H__

