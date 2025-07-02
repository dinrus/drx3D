// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/Network/ISerialize.h> // <> required for Interfuscator
#include <drx3D/Network/DrxSocks.h>

struct IServerBrowser;
struct IServerListener;
struct INetworkChat;
struct IChatListener;
struct ITestInterface;
struct IServerReport;
struct IServerReportListener;
struct IStatsTrack;
struct IStatsTrackListener;
struct INatNeg;
struct INatNegListener;
struct IFileDownloader;
struct IHTTPGateway;
struct SFileDownloadParameters;
struct IPatchCheck;
struct INetworkProfile;
struct INetworkProfileListener;
struct INetNub;
struct INetProfileTokens;

struct IContextViewExtension;
class CContextView;

struct IContextViewExtensionAdder
{
	virtual ~IContextViewExtensionAdder(){}
	virtual void AddExtension(IContextViewExtension*) = 0;
};

enum ENetworkServiceState
{

	eNSS_Initializing,  //!< Service is opened, but has not finished initializing.
	eNSS_Ok,            //!< Service is initialized okay.
	eNSS_Failed,        //!< Service has failed initializing in some way.
	eNSS_Closed         //!< Service has been closed.
};

struct INetworkService : public CMultiThreadRefCount
{
	// <interfuscator:shuffle>
	virtual ENetworkServiceState GetState() = 0;
	virtual void                 Close() = 0;
	virtual void                 CreateContextViewExtensions(bool server, IContextViewExtensionAdder* adder) = 0;
	virtual IServerBrowser*      GetServerBrowser() = 0;
	virtual INetworkChat*        GetNetworkChat() = 0;
	virtual IServerReport*       GetServerReport() = 0;
	virtual IStatsTrack*         GetStatsTrack(i32 version) = 0;
	virtual INatNeg*             GetNatNeg(INatNegListener* const) = 0;
	virtual INetworkProfile*     GetNetworkProfile() = 0;
	virtual ITestInterface*      GetTestInterface() = 0;
	virtual IFileDownloader*     GetFileDownloader() = 0;
	virtual void                 GetMemoryStatistics(IDrxSizer* pSizer) = 0;
	virtual IPatchCheck*         GetPatchCheck() = 0;
	virtual IHTTPGateway*        GetHTTPGateway() = 0;
	virtual INetProfileTokens*   GetNetProfileTokens() = 0;
	//etc
	virtual ~INetworkService(){}
	// </interfuscator:shuffle>
};

struct INetProfileTokens
{
	// <interfuscator:shuffle>
	virtual ~INetProfileTokens(){}
	virtual void AddToken(u32 profile, u32 token) = 0;
	virtual bool IsValid(u32 profile, u32 token) = 0;
	virtual void Init() = 0;
	virtual void Lock() = 0;
	virtual void Unlock() = 0;
	// </interfuscator:shuffle>
};

struct INetworkInterface
{
	// <interfuscator:shuffle>
	virtual ~INetworkInterface(){}
	virtual bool IsAvailable() const = 0;
	// </interfuscator:shuffle>
};

struct IGameQueryListener;

struct ITestInterface
{
	// <interfuscator:shuffle>
	virtual ~ITestInterface(){}
	virtual void TestBrowser(IGameQueryListener* GQListener) {}
	virtual void TestReport()                                {}
	virtual void TestChat()                                  {}
	virtual void TestStats()                                 {}
	// </interfuscator:shuffle>
};

struct SBasicServerInfo
{
	i32         m_numPlayers;
	i32         m_maxPlayers;
	bool        m_anticheat;
	bool        m_official;
	bool        m_private;
	bool        m_dx11;
	bool        m_voicecomm;
	bool        m_friendlyfire;
	bool        m_dedicated;
	bool        m_gamepadsonly;
	u16      m_hostPort;
	u16      m_publicPort;
	u32      m_publicIP;
	u32      m_privateIP;
	tukk m_hostName;
	tukk m_mapName;
	tukk m_gameVersion;
	tukk m_gameType;
	tukk m_country;
};

enum EServerBrowserError
{
	eSBE_General,
	eSBE_ConnectionFailed,
	eSBE_DuplicateUpdate,
};

struct IServerListener
{
	// <interfuscator:shuffle>
	virtual ~IServerListener(){}
	//! New server reported during update and basic values received.
	virtual void NewServer(i32k id, const SBasicServerInfo*) = 0;
	virtual void UpdateServer(i32k id, const SBasicServerInfo*) = 0;

	//! Remove server from UI server list.
	virtual void RemoveServer(i32k id) = 0;

	//! Server successfully pinged (servers behind NAT cannot be pinged).
	virtual void UpdatePing(i32k id, i32k ping) = 0;

	//! Extended data - can be dependent on game type etc, retrieved only by request via IServerBrowser::UpdateServerInfo.
	virtual void UpdateValue(i32k id, tukk name, tukk value) = 0;
	virtual void UpdatePlayerValue(i32k id, i32k playerNum, tukk name, tukk value) = 0;
	virtual void UpdateTeamValue(i32k id, i32k teamNum, tukk name, tukk value) = 0;

	//! Called on any error.
	virtual void OnError(const EServerBrowserError) = 0;

	//! All servers in the list processed.
	virtual void UpdateComplete(bool cancelled) = 0;

	//! Cannot update/ping server.
	virtual void ServerUpdateFailed(i32k id) = 0;
	virtual void ServerUpdateComplete(i32k id) = 0;

	virtual void ServerDirectConnect(bool needsnat, u32 ip, u16 port) = 0;
	// </interfuscator:shuffle>
};

struct IServerBrowser : public INetworkInterface
{
	// <interfuscator:shuffle>
	virtual ~IServerBrowser(){}
	virtual void Start(bool browseLAN) = 0;
	virtual void SetListener(IServerListener* lst) = 0;
	virtual void Stop() = 0;
	virtual void Update() = 0;
	virtual void UpdateServerInfo(i32 id) = 0;
	virtual void BrowseForServer(u32 ip, u16 port) = 0;
	virtual void BrowseForServer(tukk addr, u16 port) = 0;
	virtual void SendNatCookie(u32 ip, u16 port, i32 cookie) = 0;
	virtual void CheckDirectConnect(i32 id, u16 port) = 0;
	virtual i32  GetServerCount() = 0;
	virtual i32  GetPendingQueryCount() = 0;
	// </interfuscator:shuffle>
};

enum EChatJoinResult
{
	//! Join successful.
	eCJR_Success,

	// Error codes.
	eCJR_ChannelError,
	eCJR_Banned,
};

enum EChatUserStatus
{
	eCUS_inchannel,
	eCUS_joined,
	eCUS_left
};

enum ENetworkChatMessageType
{
	eNCMT_server,
	eNCMT_say,
	eNCMT_data
};

struct IChatListener
{
	// <interfuscator:shuffle>
	virtual ~IChatListener(){}
	virtual void Joined(EChatJoinResult) = 0;
	virtual void Message(tukk from, tukk message, ENetworkChatMessageType type) = 0;
	virtual void ChatUser(tukk nick, EChatUserStatus st) = 0;
	virtual void OnError(i32 err) = 0;
	virtual void OnChatKeys(tukk user, i32 num, tukk* keys, tukk* values) = 0;
	virtual void OnGetKeysFailed(tukk user) = 0;
	// </interfuscator:shuffle>
};

struct INetworkChat : public INetworkInterface
{
	// <interfuscator:shuffle>
	virtual void Join() = 0;
	virtual void Leave() = 0;
	virtual void SetListener(IChatListener* lst) = 0;
	virtual void Say(tukk message) = 0;
	virtual void SendData(tukk nick, tukk message) = 0;
	virtual void SetChatKeys(i32 num, tukk* keys, tukk* value) = 0;
	virtual void GetChatKeys(tukk user, i32 num, tukk* keys) = 0;
	// </interfuscator:shuffle>
};

enum EServerReportError
{
	eSRE_noerror, //!< Everything is okay.
	eSRE_socket,  //!< Socket problem: creation, bind, etc.
	eSRE_connect, //!< Connection problem, DNS, server unreachable, NAT etc.
	eSRE_noreply, //!< No reply from the other end.
	eSRE_other,   //!< Something happened.
};

struct IServerReportListener
{
	// <interfuscator:shuffle>
	virtual ~IServerReportListener(){}
	virtual void OnError(EServerReportError) = 0;
	virtual void OnPublicIP(u32, unsigned short) = 0;
	// </interfuscator:shuffle>
};

struct  IServerReport : public INetworkInterface
{
	// <interfuscator:shuffle>
	virtual void AuthPlayer(i32 playerid, u32 ip, tukk challenge, tukk responce) = 0;
	virtual void ReAuthPlayer(i32 playerid, tukk responce) = 0;

	virtual void SetReportParams(i32 numplayers, i32 numteams) = 0;

	virtual void SetServerValue(tukk key, tukk ) = 0;
	virtual void SetPlayerValue(i32, tukk key, tukk ) = 0;
	virtual void SetTeamValue(i32, tukk key, tukk ) = 0;

	virtual void Update() = 0;

	//! Start reporting.
	virtual void StartReporting(INetNub*, IServerReportListener*) = 0;

	//! Stop reporting and clear listener.
	virtual void StopReporting() = 0;

	virtual void ProcessQuery(tuk data, i32 len, DRXSOCKADDR_IN* addr) = 0;
	// </interfuscator:shuffle>
};

enum EStatsTrackError
{
	eSTE_noerror, //!< Everything is okay.
	eSTE_socket,  //!< Socket problem: creation, bind, etc.
	eSTE_connect, //!< Connection problem, DNS, server unreachable, NAT etc.
	eSTE_noreply, //!< No reply from the other end.
	eSTE_other,   //!< Something happened.
};

struct IStatsTrackListener
{
	// <interfuscator:shuffle>
	virtual ~IStatsTrackListener(){}
	virtual void OnError(EStatsTrackError) = 0;
	// </interfuscator:shuffle>
};

//! Player/Team id semantic differ from ServerReport, as ServerReport is stateless, and StatsTrack is not
struct  IStatsTrack : public INetworkInterface
{
	// <interfuscator:shuffle>
	virtual void SetListener(IStatsTrackListener*) = 0;
	//! These will return id that should be used in all other calls.
	virtual i32  AddPlayer(i32 id) = 0;   //!< Player.
	virtual i32  AddTeam(i32 id) = 0;     //!< Team.

	virtual void PlayerDisconnected(i32 id) = 0;   //!< User left the game.
	virtual void PlayerConnected(i32 id) = 0;      //!< User returned to game.

	virtual void StartGame() = 0;   //!< Clear data.
	virtual void EndGame() = 0;     //!< Send data off.
	virtual void Reset() = 0;       //!< Reset game - don't send anything.

	virtual void SetServerValue(i32 key, tukk value) = 0;
	virtual void SetPlayerValue(i32, i32 key, tukk value) = 0;
	virtual void SetTeamValue(i32, i32 key, tukk value) = 0;

	virtual void SetServerValue(i32 key, i32 value) = 0;
	virtual void SetPlayerValue(i32, i32 key, i32 value) = 0;
	virtual void SetTeamValue(i32, i32 key, i32 value) = 0;
	// </interfuscator:shuffle>
};

struct INatNegListener
{
	// <interfuscator:shuffle>
	virtual ~INatNegListener(){}

	//! indicates NAT is possible on this site.
	virtual void OnDetected(bool success, bool compatible) = 0;

	virtual void OnCompleted(i32 cookie, bool success, DRXSOCKADDR_IN* addr) = 0;
	// </interfuscator:shuffle>
};

struct INatNeg : public INetworkInterface
{
	// <interfuscator:shuffle>
	virtual void StartNegotiation(i32 cookie, bool server, i32 socket) = 0;
	virtual void CancelNegotiation(i32 cookie) = 0;
	virtual void OnPacket(tuk data, i32 len, DRXSOCKADDR_IN* fromaddr) = 0;
	// </interfuscator:shuffle>
};

struct IDownloadStream
{
	// <interfuscator:shuffle>
	virtual ~IDownloadStream(){}
	virtual void GotData(u8k* pData, u32 length) = 0;
	virtual void Complete(bool success) = 0;
	// </interfuscator:shuffle>
};

struct SFileDownloadParameters
{
	virtual ~SFileDownloadParameters(){}
	SFileDownloadParameters()
	{
		sourceFilename.clear();
		destFilename.clear();
		destPath.clear();
		for (i32 i = 0; i < 16; ++i)
			md5[i] = 0;
		fileSize = 0;
		pStream = 0;
	}

	string           sourceFilename; //!< e.g. "http://< www.server.com/file.ext".
	string           destFilename;   //!< e.g. "file.ext".
	string           destPath;       //!< e.g. "Game\Levels\Multiplayer\IA\LevelName\".
	u8    md5[16];        //!< MD5 checksum of the file.
	i32              fileSize;

	IDownloadStream* pStream;  //!< Replaces destFilename if set - direct to memory downloads.

	virtual void     SerializeWith(TSerialize ser)
	{
		ser.Value("SourceFilename", sourceFilename);
		ser.Value("DestFilename", destFilename);
		ser.Value("DestPath", destPath);
		ser.Value("FileSize", fileSize);

		for (i32 i = 0; i < 16; ++i)
			ser.Value("MD5Checksum", md5[i]);
	}
};

struct IFileDownload
{
	// <interfuscator:shuffle>
	virtual ~IFileDownload(){}
	virtual bool  Finished() const = 0;
	virtual float GetProgress() const = 0;
	// </interfuscator:shuffle>
};

struct IFileDownloader : public INetworkInterface
{
	// <interfuscator:shuffle>
	//! Start downloading from http server and save the file locally.
	virtual void DownloadFile(SFileDownloadParameters& dl) = 0;

	//! Set throttling parameters (so as not to slow down games in progress).
	//! Probably want to turn off throttling if we're at the end of level, etc.
	//! Set both parameters to zero to disable throttling.
	//! \param datasize How much data to request.
	//! \param timedelay How often to request it (in ms).
	virtual void SetThrottleParameters(i32 datasize, i32 timedelay) = 0;

	//! Is a download in progress?
	virtual bool IsDownloading() const = 0;

	//! Get progress of current download (0.0 - 1.0).
	virtual float GetDownloadProgress() const = 0;

	//! Get md5 checksum of downloaded file (only valid when IsDownloading()==false).
	virtual u8k* GetFileMD5() const = 0;

	//! Stop downloads (also called when no more files to download).
	virtual void Stop() = 0;
	// </interfuscator:shuffle>
};

struct IPatchCheck : public INetworkInterface
{
	// <interfuscator:shuffle>
	//! Fire off a query to a server somewhere to figure out if there's an update available for download.
	virtual bool CheckForUpdate() = 0;

	//! Is the query still pending?
	virtual bool IsUpdateCheckPending() const = 0;

	//! Is there an update?
	virtual bool IsUpdateAvailable() const = 0;

	//! Is it a required update?
	virtual bool IsUpdateMandatory() const = 0;

	//! Get the URL of the patch file.
	virtual tukk GetPatchURL() const = 0;

	//! Get display name of new version.
	virtual tukk GetPatchName() const = 0;

	//! Trigger install of patch on exit game.
	virtual void SetInstallOnExit(bool install) = 0;
	virtual bool GetInstallOnExit() const = 0;

	//! Store and retrieve the place the patch was downloaded to.
	virtual void        SetPatchFileName(tukk filename) = 0;
	virtual tukk GetPatchFileName() const = 0;

	virtual void        TrackUsage() = 0;
	// </interfuscator:shuffle>
};

struct IHTTPGateway
{
	// <interfuscator:shuffle>
	virtual ~IHTTPGateway(){}
	virtual i32  GetURL(tukk url, IDownloadStream* stream) = 0;
	virtual i32  PostURL(tukk url, tukk params, IDownloadStream* stream) = 0;
	virtual i32  PostFileToURL(tukk url, tukk params, tukk name, u8k* data, u32 size, tukk mime, IDownloadStream* stream) = 0;
	virtual void CancelRequest(i32) = 0;
	// </interfuscator:shuffle>
};

i32k MAX_PROFILE_STRING_LEN = 31;

enum EUserStatus
{
	eUS_offline = 0,
	eUS_online,
	eUS_playing,
	eUS_staging,
	eUS_chatting,
	eUS_away
};

struct SNetworkProfileUserStatus
{
	char        m_locationString[MAX_PROFILE_STRING_LEN + 1];
	char        m_statusString[MAX_PROFILE_STRING_LEN + 1];
	EUserStatus m_status;
};

enum ENetworkProfileError
{
	eNPE_ok,

	eNPE_connectFailed,         //!< Cannot connect to server.
	eNPE_disconnected,          //!< Disconnected from GP.

	eNPE_loginFailed,           //!< Check your account/password.
	eNPE_loginTimeout,          //!< Timeout.
	eNPE_anotherLogin,          //!< Another login.

	eNPE_actionFailed,          //!< Generic action failed.

	eNPE_nickTaken,             //!< Nick already take.
	eNPE_registerAccountError,  //!< Mail/pass not match.
	eNPE_registerGeneric,       //!< Register failed.

	eNPE_nickTooLong,         //!< No longer than 20 chars.
	eNPE_nickFirstNumber,     //!< No first digits.
	eNPE_nickSlash,           //!< No slash in nicks.
	eNPE_nickFirstSpecial,    //!< No first specials @+#:.
	eNPE_nickNoSpaces,        //!< No spaces in nick.
	eNPE_nickEmpty,           //!< Empty nicks.

	eNPE_profileEmpty,          //!< Empty profile name.

	eNPE_passTooLong,           //!< No longer than 30 chars.
	eNPE_passEmpty,             //!< Empty password.

	eNPE_mailTooLong,           //!< No longer than 50 chars.
	eNPE_mailEmpty,             //!< No longer than 50 chars.
};

struct INetworkProfileListener
{
	// <interfuscator:shuffle>
	virtual ~INetworkProfileListener(){}
	virtual void AddNick(tukk nick) = 0;
	virtual void UpdateFriend(i32 id, tukk nick, EUserStatus, tukk status, bool foreign) = 0;
	virtual void RemoveFriend(i32 id) = 0;
	virtual void OnFriendRequest(i32 id, tukk message) = 0;
	virtual void OnMessage(i32 id, tukk message) = 0;
	virtual void LoginResult(ENetworkProfileError res, tukk descr, i32 id, tukk nick) = 0;
	virtual void OnError(ENetworkProfileError res, tukk descr) = 0;
	virtual void OnProfileInfo(i32 id, tukk key, tukk value) = 0;

	//! Finished updating profile info.
	virtual void OnProfileComplete(i32 id) = 0;

	virtual void OnSearchResult(i32 id, tukk nick) = 0;
	virtual void OnSearchComplete() = 0;
	virtual void OnUserId(tukk nick, i32 id) = 0;
	virtual void OnUserNick(i32 id, tukk nick, bool foreign_name) = 0;
	virtual void RetrievePasswordResult(bool ok) = 0;
	// </interfuscator:shuffle>
};

struct IStatsAccessor
{
	// <interfuscator:shuffle>
	virtual ~IStatsAccessor(){}
	//input
	virtual tukk GetTableName() = 0;
	virtual void        End(bool success) = 0;
	// </interfuscator:shuffle>
};

struct IStatsReader : public IStatsAccessor
{
	// <interfuscator:shuffle>
	virtual i32         GetFieldsNum() = 0;
	virtual tukk GetFieldName(i32 i) = 0;
	//output
	virtual void        NextRecord(i32 id) = 0;
	virtual void        OnValue(i32 field, tukk val) = 0;
	virtual void        OnValue(i32 field, i32 val) = 0;
	virtual void        OnValue(i32 field, float val) = 0;
	// </interfuscator:shuffle>
};

struct IStatsWriter : public IStatsAccessor
{
	// <interfuscator:shuffle>
	virtual i32         GetFieldsNum() = 0;
	virtual tukk GetFieldName(i32 i) = 0;
	//input
	virtual i32         GetRecordsNum() = 0;
	virtual i32         NextRecord() = 0;
	virtual bool        GetValue(i32 field, tukk & val) = 0;
	virtual bool        GetValue(i32 field, i32& val) = 0;
	virtual bool        GetValue(i32 field, float& val) = 0;
	//output
	virtual void        OnResult(i32 idx, i32 id, bool success) = 0;
	// </interfuscator:shuffle>
};

struct IStatsDeleter : public IStatsAccessor
{
	// <interfuscator:shuffle>
	virtual i32  GetRecordsNum() = 0;
	virtual i32  NextRecord() = 0;
	virtual void OnResult(i32 idx, i32 id, bool success) = 0;
	virtual void End(bool success) = 0;
	// </interfuscator:shuffle>
};

struct SRegisterDayOfBirth
{
	SRegisterDayOfBirth(){}
	SRegisterDayOfBirth(u8 d, u8 m, u16 y) : day(d), month(m), year(y){}

	//hm...
	SRegisterDayOfBirth(u32 i) : day(i & 0xFF), month((i >> 8) & 0xFF), year(i >> 16){}
	operator u32() const { return (u32(year) << 16) + (u32(month) << 8) + day; }

	u8  day;
	u8  month;
	u16 year;
};

struct INetworkProfile : public INetworkInterface
{
	// <interfuscator:shuffle>
	virtual void AddListener(INetworkProfileListener*) = 0;
	virtual void RemoveListener(INetworkProfileListener*) = 0;

	virtual void Register(tukk nick, tukk email, tukk password, tukk country, SRegisterDayOfBirth dob) = 0;
	virtual void Login(tukk nick, tukk password) = 0;
	virtual void LoginProfile(tukk email, tukk password, tukk profile) = 0;
	virtual void Logoff() = 0;
	virtual void SetStatus(EUserStatus status, tukk location) = 0;
	virtual void EnumUserNicks(tukk email, tukk password) = 0;
	virtual void AuthFriend(i32 id, bool auth) = 0;
	virtual void RemoveFriend(i32 id, bool ignore) = 0;
	virtual void AddFriend(i32 id, tukk reason) = 0;
	virtual void SendFriendMessage(i32 id, tukk message) = 0;
	virtual void GetProfileInfo(i32 id) = 0;
	virtual bool IsLoggedIn() = 0;
	virtual void UpdateBuddies() = 0;
	virtual void SearchFriends(tukk request) = 0;
	virtual void GetUserId(tukk nick) = 0;
	virtual void GetUserNick(i32 id) = 0;

	//! Player Stats.
	virtual void ReadStats(IStatsReader* reader) = 0;
	virtual void WriteStats(IStatsWriter* writer) = 0;
	virtual void DeleteStats(IStatsDeleter* deleter) = 0;

	//! Other Player's Stats.
	virtual void ReadStats(i32 id, IStatsReader* reader) = 0;
	virtual void RetrievePassword(tukk email) = 0;
	// </interfuscator:shuffle>
};

//! \endcond