// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RICHPRESENCE_H__
#define __RICHPRESENCE_H__

#include <drx3D/Game/GameMechanismBase.h>

class CRichPresence : public CGameMechanismBase
{
	public:

		enum ERichPresenceState
		{
			eRPS_None,
			eRPS_Idle,
			eRPS_FrontEnd,
			eRPS_Lobby,
			eRPS_InGame,
		};

		enum ERichPresenceType
		{
			eRPT_String = 0,
			eRPT_Param1,
			eRPT_Param2,
			eRPT_Max,
		};

	public:

		CRichPresence();
		virtual ~CRichPresence();

		// CGameMechanismBase
		virtual void Update(float dt);
		// ~CGameMechanismBase

		void LoadXmlFromFile(tukk path);
		bool SetRichPresence(ERichPresenceState state);
		DrxSessionID GetPendingRichPresenceSessionID() const { return m_pendingSessionID; }
	
	private:

		void OnSetRichPresenceCallback(DrxLobbyTaskID taskID, EDrxLobbyError error);

		static void SetRichPresenceCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);

	private:

		typedef std::map<DrxFixedStringT<128>, i32> TRichPresenceMap;
		TRichPresenceMap m_richPresence;
		
		ERichPresenceState m_currentState;
		ERichPresenceState m_pendingState;
		ERichPresenceState m_desiredState;

		DrxLobbyTaskID m_taskID;

		DrxSessionID m_currentSessionID;
		DrxSessionID m_pendingSessionID;

		float m_updateTimer;
		bool m_refresh;
};

#endif // __RICHPRESENCE_H__
