// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <map>

typedef std::map<u16, class CGameServerChannel*> TServerChannelMap;
class CGameContext;

class CGameServerNub final :
	public IGameServerNub
{
	typedef std::map<INetChannel*, u16> TNetServerChannelMap;
	typedef struct SHoldChannel
	{
		SHoldChannel() : channelId(0), time(0.0f) {};
		SHoldChannel(u16 chanId, const CTimeValue& tv) : channelId(chanId), time(tv) {};

		u16     channelId;
		CTimeValue time;
	} SHoldChannel;

	typedef std::map<i32, SHoldChannel> THoldChannelMap;

	struct SBannedPlayer
	{
		SBannedPlayer() : profileId(0), time(0.0f) {}
		SBannedPlayer(i32 profId, u32 addr, const CTimeValue& tv) : profileId(profId), ip(addr), time(tv) {}

		i32      profileId; //profile id
		u32     ip;        //in LAN we use ip
		CTimeValue time;
	};
	typedef std::vector<SBannedPlayer> TBannedVector;
public:
	CGameServerNub();
	virtual ~CGameServerNub();

	// IGameNub
	virtual void                 Release();
	virtual SCreateChannelResult CreateChannel(INetChannel* pChannel, tukk pRequest);
	virtual void                 FailedActiveConnect(EDisconnectionCause cause, tukk description);
	// ~IGameNub

	// IGameServerNub
	virtual void AddSendableToRemoteClients(INetSendablePtr pMsg, i32 numAfterHandle, const SSendableHandle* afterHandle, SSendableHandle* handle);
	// ~IGameServerNub

	void                SetGameContext(CGameContext* pGameContext) { m_pGameContext = pGameContext; };
	TServerChannelMap*  GetServerChannelMap()                      { return &m_channels; };

	void                Update();

	void                GetMemoryUsage(IDrxSizer* s) const;

	void                SetMaxPlayers(i32 maxPlayers);
	i32                 GetMaxPlayers() const  { return m_maxPlayers; }
	i32                 GetPlayerCount() const { return i32(m_channels.size()); }

	CGameServerChannel* GetChannel(u16 channelId);
	CGameServerChannel* GetChannel(INetChannel* pNetChannel);
	INetChannel*        GetLocalChannel();
	u16              GetChannelId(INetChannel* pNetChannel) const;
	void                RemoveChannel(u16 channelId);

	bool                IsPreordered(u16 channelId);
	bool                PutChannelOnHold(CGameServerChannel* pServerChannel);
	void                RemoveOnHoldChannel(CGameServerChannel* pServerChannel, bool renewed);
	CGameServerChannel* GetOnHoldChannelFor(INetChannel* pNetChannel);
	void                ResetOnHoldChannels();//called when onhold channels loose any sense - game reset, new level etc.

	void                BanPlayer(u16 channelId, tukk reason);
	bool                CheckBanned(INetChannel* pNetChannel);
	void                BannedStatus();
	void                UnbanPlayer(i32 profileId);

private:
	TServerChannelMap    m_channels;
	TNetServerChannelMap m_netchannels;
	THoldChannelMap      m_onhold;
	TBannedVector        m_banned;
	CGameContext*        m_pGameContext;
	u16               m_genId;
	i32                  m_maxPlayers;

	static ICVar*        sv_timeout_disconnect;
	bool                 m_allowRemoveChannel;
};