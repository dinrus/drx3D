// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/CoreX/Lobby/CommonIDrxLobby.h>
#include <drx3D/CoreX/Lobby/IDrxStats.h>

#define DRXLOBBY_USER_PACKET_START 128
#define DRXLOBBY_USER_PACKET_MAX   255

typedef u16 DrxPing;
#define DRXLOBBY_INVALID_PING  (DrxPing(~0))

#define DRXSESSIONID_STRINGLEN 48

//! When a call to set Rich Presence is made, the SDrxLobbyUserData is just passed back to the game code, so.
//! It's the game code's responsibility to correctly turn all the bits and bobs of SDrxLobbyUserData into a single UTF-8 string.
//! [in] = maximum size of the buffer pre-allocated [SCE_NP_BASIC_PRESENCE_EXTENDED_STATUS_SIZE_MAX].
//! [out] = modify the value to contain the actual length of the string in bytes.
struct SDrxLobbyPresenceConverter
{
	SDrxLobbyUserData* m_pData;                //!< Pointer of the SDrxLobbyUserData passed into DrxLobbyUI::SetRichPresence. [in].
	u32             m_numData;              //!< Number of SDrxLobbyUserData in array pointed to by m_pData. [in].
	u8*             m_pStringBuffer;        //!< Buffer to write the result string in to. This buffer is pre-allocated internally, no need to create your own. [in/out].
	u32             m_sizeOfStringBuffer;   //!< Size of the string buffer in bytes, not characters. [in/out].
	DrxSessionID       m_sessionId;            //!< Joinable session ID, to allow friends to join your game if desired. [out].
};

//! Used for creating a session and updating the session details.
//! \note In SDK 1.500 the linkdata (m_pData) could not be updated once created. Feature is due to be added in 1.700.
//! \note Updating localised session name/status will overwrite/replace all previous name/status values. Values not updated are discarded.
//! \note Is to NOT possible to update the number of slots, private or editable flags after creation.
struct SDrxLobbySessionAdvertisement
{
	uk  m_pJPGImage;
	u32 m_sizeofJPGImage;
	u8* m_pData;
	u32 m_sizeofData;
	tuk* m_pLocalisedLanguages;
	tuk* m_pLocalisedSessionNames;
	tuk* m_pLocalisedSessionStatus;
	u32 m_numLanguages;
	u32 m_numSlots;
	bool   m_bIsPrivate;
	bool   m_bIsEditableByAll;
};

//! A request to fill in an XMB string requires a buffer allocated by the calling function, and a value defining the size of the buffer.
//! It's the game's responsibility to fill the buffer with a UTF-8 string, and not to overflow the buffer.
struct SDrxLobbyXMBString
{
	u8* m_pStringBuffer;                           //! Pointer to a UTF-8 compatible string buffer.
	u32 m_sizeOfStringBuffer;                      //!< Maximum sizeof the buffer.
};

//! A request to fill supply age restriction settings for different regions.
struct SDrxLobbyAgeRestrictionSetup
{
	struct SCountry
	{
		char id[3];
		int8 age;
	};
	i32           m_numCountries;                   //!< Number of specified countries.
	const SCountry* m_pCountries;                     //!< Array of m_numCountries SDrxLobbyAgeRestrictionSetup::SCountry entries.
};

// Privilege bits returned from IDrxLobbyService::GetUserPrivileges in the DrxLobbyPrivilegeCallback.
u32k CLPF_BlockMultiplayerSessons = BIT(0); //!< If set the user is not allowed to participate in multiplayer game sessions.
u32k CLPF_NotOnline = BIT(1);               //!< If set the user is not connected to PSN.

struct SDrxSystemTime
{
	u16 m_Year;
	u16 m_Milliseconds;
	u8  m_Month;
	u8  m_DayOfWeek;
	u8  m_Day;
	u8  m_Hour;
	u8  m_Minute;
	u8  m_Second;
};

//! \param taskID    Task ID allocated when the function was executed.
//! \param error     Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param privilege A bitfield of CLPF_* specifying the users privileges. Bits get set to indicate the user should be blocked from an action if their privilege level isn't high enough.
//! \param pArg      Pointer to application-specified data.
typedef void(*DrxLobbyPrivilegeCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, u32 privilege, uk pArg);

//! \param taskID    Task ID allocated when the function was executed.
//! \param error     Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pArg      Pointer to application-specified data.
typedef void(*DrxLobbyOnlineStateCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pString
//! \param isProfanity true if the string contained a profanity, otherwise false.
//! \param pArg        Pointer to application-specified data.
typedef void(*DrxLobbyCheckProfanityCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, tukk pString, bool isProfanity, uk pArg);

#include "IDrxTCPService.h"
#include "IDrxLobbyEvent.h"

struct IDrxSignIn;

struct IDrxLobbyService
{
public:
	// <interfuscator:shuffle>
	virtual ~IDrxLobbyService() {}
	virtual IDrxMatchMaking*       GetMatchMaking() = 0;
	virtual IDrxVoice*             GetVoice() = 0;
	virtual IDrxStats*             GetStats() = 0;
	virtual IDrxLobbyUI*           GetLobbyUI() = 0;
	virtual IDrxFriends*           GetFriends() = 0;
	virtual IDrxFriendsManagement* GetFriendsManagement() = 0;
	virtual IDrxReward*            GetReward() = 0;
	virtual IDrxOnlineStorage*     GetOnlineStorage() = 0;
	virtual IDrxSignIn*            GetSignIn() = 0;

	//! Cancel the given task. The task will still be running in the background but the callback will not be called when it finishes.
	//! \param TaskID			- The task to cancel.
	virtual void CancelTask(DrxLobbyTaskID lTaskID) = 0;

	//! Get an IDrxTCPService.
	//! \param pService Service.
	//! \return Pointer to IDrxTCPService for given server & port.
	virtual IDrxTCPServicePtr GetTCPService(tukk pService) = 0;

	//! Get an IDrxTCPService.
	//! \param pServer server
	//! \param port port
	//! \param pUrlPrefix URL prefix
	//! \return Pointer to IDrxTCPService for given server & port.
	virtual IDrxTCPServicePtr GetTCPService(tukk pServer, u16 port, tukk pUrlPrefix) = 0;

	//! Get the user id of a user signed in to this service locally.
	//! \param user	The pad number of the local user.
	//! \return DrxUserID of the user.
	virtual DrxUserID GetUserID(u32 user) = 0;

	//! Get the given users privileges.
	//! \param user      The pad number of the local user.
	//! \param pTaskID   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCB       Callback function that is called when function completes.
	//! \param pCbArg    Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError GetUserPrivileges(u32 user, DrxLobbyTaskID* pTaskID, DrxLobbyPrivilegeCallback pCB, uk pCBArg) = 0;

	//! Check a string for profanity.
	//! \param pString   String to check.
	//! \param pTaskID   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb       Callback function that is called when function completes.
	//! \param pCbArg    Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError CheckProfanity(tukk const pString, DrxLobbyTaskID* pTaskID, DrxLobbyCheckProfanityCallback pCb, uk pCbArg) = 0;

	//! Returns the current time from the online time server (must be signed in).
	//! \param pSystemTime Pointer to the DrxSystemTime structure to contain the time. Only year, month, day, hours, minutes and seconds will be populated.
	//! \return eCLE_Success if the function was successful, or an error code if not.
	virtual EDrxLobbyError GetSystemTime(u32 user, SDrxSystemTime* pSystemTime) = 0;

	// </interfuscator:shuffle>
};

// The callback below is fired whenever the lobby system needs access to data that is unavailable through the standard API.
//Its usually used to work around problems with specific platform lobbies requiring data not exposed through standard functionality.

//Basically whenever this callback is fired, you are required to fill in the requestedParams that are asked for.
//At present specifications are that the callback can fire multiple times.
//
// 'XSvc'		u32	Service Id for XLSP servers
// 'XPor'		u16	Port for XLSP server to communicate with Telemetry
// 'XSNm'		uk 		ptr to a static const string containging the name of the required XLSP service. is used to filter the returned server list from Live. return NULL for no filtering
// 'LUnm'		uk 		ptr to user name for local user - used by LAN (due to lack of guid) (is copied internally - DO NOT PLACE ON STACK)
//
// PS3 ONLY
// 'PCom'		uk 		ptr to static SceNpCommunitcationId					(not copied - DO NOT PLACE ON STACK!)
// 'PPas'		uk 		ptr to static SceNpCommunicationPassphrase	(not copied - DO NOT PLACE ON STACK!)
// 'PSig'		uk 		ptr to static SceNpCommunicationSignature		(not copied - DO NOT PLACE ON STACK!)
// 'PInM'		tuk		ptr to string used for the custom XMB button for sending invites to friends.
// 'PInS'		tuk		ptr to string used for the XMB game invite message subject text.
// 'PInB'		tuk		ptr to string used for the XMB game invite message body text.
// 'PFrS'		tuk		ptr to string used for the XMB friend request message subject text.
// 'PFrB'		tuk		ptr to string used for the XMB friend request message body text.
// 'PAge'		i32		Age limit of game (set to 0 for now, probably wants to come from some kind of configuration file)
// 'PSto'		tuk		Store ID. AKA NP Commerce ServiceID and also top level category Id.
// 'PDlc'		int8		Input is DLC pack index to test is installed, output is boolean result.
//
// PS4 ONLY
// PTit and PSec are the PS4 replacements for the PS3 PCom, PPas and PSig.
// 'PTit'		uk 		ptr to static SceNpTitleId									(not copied - DO NOT PLACE ON STACK!)
// 'PSec'		uk 		ptr to static SceNpTitleSecret							(not copied - DO NOT PLACE ON STACK!)
// 'PSAd'		SDrxLobbySessionAdvertisement*	in/out structure to request extra session advertising info from the game
// 'PAge'		SDrxLobbyAgeRestrictionSetup*		Age limit settings per region
// 'PPlu'		i32		Age limit of game (set to 0 for now, probably wants to come from some kind of configuration file)
//
// 'CSgs'		u32	EDrxLobbyLoginGUIState constant to indicate whether other data requested is available
// 'CPre'		SDrxLobbyPresenceConverter* in/out structure for converting a list of SDrxLobbyUserData into a single string for presense
// 'Mspl'		Matchmaking session password length (not including null terminator)

#define CLCC_LAN_USER_NAME                           'LUnm'
#define CLCC_LIVE_TITLE_ID                           'XTtl'
#define CLCC_XLSP_SERVICE_ID                         'XSvc'
#define CLCC_LIVE_SERVICE_CONFIG_ID                  'XSCf'
#define CLCC_XLSP_SERVICE_PORT                       'XPor'
#define CLCC_XLSP_SERVICE_NAME                       'XSNm'
#define CLCC_PSN_COMMUNICATION_ID                    'PCom'
#define CLCC_PSN_COMMUNICATION_PASSPHRASE            'PPas'
#define CLCC_PSN_COMMUNICATION_SIGNATURE             'PSig'
#define CLCC_PSN_CUSTOM_MENU_GAME_INVITE_STRING      'PInM'
#define CLCC_PSN_CUSTOM_MENU_GAME_JOIN_STRING        'PJoM'
#define CLCC_PSN_INVITE_SUBJECT_STRING               'PInS'
#define CLCC_PSN_INVITE_BODY_STRING                  'PInB'
#define CLCC_PSN_FRIEND_REQUEST_SUBJECT_STRING       'PFrS'
#define CLCC_PSN_FRIEND_REQUEST_BODY_STRING          'PFrB'
#define CLCC_PSN_AGE_LIMIT                           'PAge'
#define CLCC_PSN_PLUS_TEST_REQUIRED                  'PPlu'
#define CLCC_PSN_STORE_ID                            'PSto'
#define CLCC_PSN_TICKETING_ID                        'PTik'
#define CLCC_PSN_IS_DLC_INSTALLED                    'PDlc'
#define CLCC_PSN_TITLE_ID                            'PTit'
#define CLCC_PSN_TITLE_SECRET                        'PSec'
#define CLCC_PSN_TICKETING_ID                        'PTik'
#define CLCC_PSN_CREATE_SESSION_ADVERTISEMENT        'PCSA'
#define CLCC_PSN_UPDATE_SESSION_ADVERTISEMENT        'PUSA'
#define CLCC_DRXLOBBY_EXPORTGAMESTATE                'CEgs'
#define CLCC_DRXLOBBY_IMPORTGAMESTATE                'CIgs'
#define CLCC_DRXLOBBY_LOBBYDATANAME                  'CLdn'
#define CLCC_DRXLOBBY_LOBBYDATAUSAGE                 'CLdu'
#define CLCC_DRXLOBBY_PRESENCE_CONVERTER             'CPre' // Used by PSN for converting presence info into a string form
#define CLCC_DRXLOBBY_QUERYTRANSLATEHASH             'CQth'
#define CLCC_DRXLOBBY_TRANSLATEHASH                  'CTrh'

#define CLCC_DRXSTATS_ENCRYPTION_KEY                 'CEnc' // Used for all platforms to encrypt UserData buffers

#define CLCC_MATCHMAKING_SESSION_PASSWORD_MAX_LENGTH 'Mspl' // Used to determine the maximum length of the session password (not including NULL character)

#define CLCC_STEAM_APPID                             'SaID' // Request for the Steam application id (only required during development)

struct SConfigurationParams
{
	u32 m_fourCCID;
	union
	{
		uint64 m_64;
		uk  m_pData;
		u32 m_32;
		u16 m_16;
		u8  m_8;
	};
};
