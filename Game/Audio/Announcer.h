// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 21:07:2009		Created by Tim Furnish
*************************************************************************/

#ifndef __ANNOUNCER_H__
#define __ANNOUNCER_H__

#include <drx3D/Game/Audio/AudioTypes.h>
#include <drx3D/Game/UI/HUD/HUDOnScreenMessageDef.h>
#include <drx3D/Game/Utility/SingleAllocTextBlock.h>
#include <drx3D/Game/GameRulesTypes.h>

#define INVALID_ANNOUNCEMENT_ID (-1)

struct SAnnouncementDef
{
	EAnnouncementID					m_announcementID;
	EAnnounceConditions			m_conditions;
	tukk 						m_pName;
	tukk 						m_audio;
	u32									m_audioFlags;
	SOnScreenMessageDef			m_onScreenMessage;
	float										m_noRepeat;

#ifndef _RELEASE
	u32									m_playCount;
#endif

	SAnnouncementDef()
		: m_announcementID(0)
		, m_conditions(0)
		, m_pName("")
		, m_audio("")
		, m_audioFlags(0)
		, m_noRepeat(0.0f)
#ifndef _RELEASE
		, m_playCount(0)
#endif
	{}
};

class CAnnouncer /*: public ISoundEventListener*/
{
	public:
		CAnnouncer(const EGameMode gamemode);
		~CAnnouncer();

		static EAnnouncementID NameToID(tukk announcement);

		enum EAnnounceContext
		{
			eAC_inGame = 0,
			eAC_postGame,
			eAC_always
		};

		void Announce(tukk announcement, EAnnounceContext context);

		//Works out play conditions around entity id e.g self, teammate or enemy
		void Announce(const EntityId entityID, tukk announcement, EAnnounceContext context);
		void Announce(const EntityId entityID, EAnnouncementID announcementID, EAnnounceContext context);

		//FromTeamId works out play conditions, e.g teammate or enemy
		void AnnounceFromTeamId(i32k teamId, EAnnouncementID announcementID, EAnnounceContext context);
		void AnnounceFromTeamId(i32k teamId, tukk announcement, EAnnounceContext context);

		//AsTeam plays with specific teamId
		void AnnounceAsTeam(i32k teamId, EAnnouncementID announcementID, EAnnounceContext context);
		void AnnounceAsTeam(i32k teamId, tukk announcement, EAnnounceContext context);

		//Force announce overrides currently playing
		void ForceAnnounce(tukk announcement, EAnnounceContext context);
		void ForceAnnounce(const EAnnouncementID announcementID, EAnnounceContext context);

		static CAnnouncer* GetInstance();

#ifndef _RELEASE
		static void CmdAnnounce(IConsoleCmdArgs* pArgs);
		static void CmdDump(IConsoleCmdArgs* pArgs);
		static void CmdDumpPlayed(IConsoleCmdArgs *pCmdArgs);
		static void CmdDumpUnPlayed(IConsoleCmdArgs *pCmdArgs);
		static void CmdClearPlayCount(IConsoleCmdArgs *pCmdArgs);
		static void CmdCheckAnnouncements(IConsoleCmdArgs* pArgs);

		i32 GetCount() const;
		tukk GetName(i32 i) const;
		tukk GetAudio(i32 i) const;
#endif

		//void OnSoundEvent( ESoundCallbackEvent event, ISound *pSound );

	private:
		void LoadAnnouncements(const EGameMode gamemode);

		bool AnnounceInternal(EntityId entityID, i32k overrideTeamId, tukk announcement, const EAnnounceConditions conditions, const bool force, EAnnounceContext context);
		bool AnnounceInternal(EntityId entityID, i32k overrideTeamId, const EAnnouncementID announcementID, const EAnnounceConditions conditions, const bool force, EAnnounceContext context);

		bool AllowAnnouncement() const;
		bool AllowNoRepeat(const float noRepeat, const float currentTime);

		tukk GetGamemodeName(EGameMode mode);
		
		// Suppress passedByValue for smart pointers like XmlNodeRef
		// cppcheck-suppress passedByValue
		bool ShouldLoadNode(const XmlNodeRef gamemodeXML, tukk currentGamemode, tukk currentLanguage);
		// cppcheck-suppress passedByValue
		bool LoadAnnouncementOption(tukk optionName, const XmlNodeRef optionXML, const EAnnounceConditions conditions, tukk  storedName, SOnScreenMessageDef &onScreenMessage, const float noRepeat, i32 &numAnnouncementsAdded);

		void AddAnnouncement(tukk  pName, const EAnnounceConditions condition, tukk  audio, const SOnScreenMessageDef & onScreenMessage, const float noRepeat);

		const SAnnouncementDef * FindAnnouncementWithConditions(i32 announcementCRC, EAnnounceConditions conditions) const;

		EAnnounceConditions GetConditionsFromEntityId(const EntityId entityId) const;
		EAnnounceConditions GetConditionsFromTeamId(i32k teamId) const;

		void ConvertToFinalSoundName(i32k overrideTeamId, string& filename);
		i32k GetTeam(i32k overrideTeamId);

		static CAnnouncer* s_announcer_instance;
		std::vector<SAnnouncementDef> m_annoucementList;
		//typedef std::vector<tSoundID> TSoundIDList;
		//TSoundIDList m_soundsBeingListenedTo;

		//tSoundID m_mostRecentSoundId;

		mutable float m_lastPlayedTime;
		mutable float m_canPlayFromTime;
		mutable float m_repeatTime;

		TAudioSignalID m_messageSignalID;
		
		CSingleAllocTextBlock m_singleAllocTextBlock;
};

#endif // __ANNOUNCER_H__