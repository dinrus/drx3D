// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  This listener looks for running servers in the LAN and
   is keeping a list of all available servers.
   There is a callback to get this list (ordered by ping) as well as to connect to one ...
   -------------------------------------------------------------------------
   История:
   - 19/06/2006   : Implemented by Jan Müller
*************************************************************************/

#ifndef __GAMEQUERYLISTENER_H__
#define __GAMEQUERYLISTENER_H__

#pragma once

class CGameQueryListener : public IGameQueryListener
{

	struct SGameServer
	{
		string     m_target;
		string     m_description;
		string     m_data;
		u32     m_ping;
		CTimeValue m_lastTime;

		SGameServer(tukk target, tukk description, tukk data, u32k ping) :
			m_target(target), m_description(description), m_data(data), m_ping(ping)
		{
			m_lastTime = gEnv->pTimer->GetFrameStartTime();
		}

		~SGameServer()
		{

		}

		bool Compare(SGameServer& server)
		{
			if (server.m_target.compare(m_target) == 0 && server.m_description.compare(m_data) == 0)
				return true;
			return false;
		}

		bool operator<(const SGameServer& server) const
		{
			if (m_ping < server.m_ping)
				return true;
			return false;
		}

		void Inc(SGameServer& server)
		{
			if (m_data.compare(server.m_data) != 0)
				m_data = server.m_data;
			IncPing(server.m_ping);
			m_lastTime = gEnv->pTimer->GetFrameStartTime();
		}

		void IncPing(u32k ping)
		{
			if (m_ping)
				m_ping = (3 * m_ping + ping) / i32(4);
			else
				m_ping = ping;
		}

		string GetServerGameVersion()
		{
			string retVal("version not found in data");
			size_t pos1, pos2;
			pos1 = m_data.find(string("version"));    //this code will be broken if game info changes
			pos2 = m_data.find(string("maxPlayers"));
			if (pos1 != string::npos && pos2 != string::npos)
			{
				pos1 += 9;
				retVal = m_data.substr(pos1, pos2 - pos1 - 2);
			}
			return retVal;
		}
	};

public:
	CGameQueryListener();
	~CGameQueryListener();

	// IGameQueryListener
	virtual void        AddServer(tukk description, tukk target, tukk additionalText, u32 ping);
	virtual void        RemoveServer(string address);
	virtual void        AddPong(string address, u32 ping);
	virtual void        GetCurrentServers(tuk** pastrServers, i32& o_amount);
	virtual void        GetServer(i32 number, tuk* server, tuk* data, i32& ping);
	virtual tukk GetServerData(tukk server, i32& o_ping);
	virtual void        Update();
	virtual void        OnReceiveGameState(tukk fromAddress, XmlNodeRef xmlData);
	virtual void        Release();
	virtual void        RefreshPings();
	virtual void        ConnectToServer(tukk server);
	virtual void        GetValuesFromData(tuk strData, SServerData* pServerData);
	// ~IGameQueryListener

	void GetMemoryStatistics(IDrxSizer* s);

	void SetNetListener(INetQueryListener* pListener);
	void Complete();

private:

	SGameServer* FindServer(tukk target);

	//throw out old servers
	void CleanUpServers();

	INetQueryListener* m_pNetListener;

	//a vector of servers currently running (from NetQueryListener)
	std::vector<SGameServer> m_servers;
	tuk                    m_astrServers[120];
	i32                      m_iServers;
};

#endif
