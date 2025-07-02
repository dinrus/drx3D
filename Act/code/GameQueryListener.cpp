// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  This listener looks for running servers in the LAN and
   is keeping a list of all available servers.
   -------------------------------------------------------------------------
   История:
   - 19/06/2006   : Implemented by Jan Müller
*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameQueryListener.h>
#include <drx3D/Act/DinrusAction.h>

CGameQueryListener::CGameQueryListener()
	: m_pNetListener(nullptr)
	, m_iServers(50)
{
	for (i32 iServer = 0; iServer < m_iServers; iServer++)
	{
		m_astrServers[iServer] = new char[256];
	}
}

CGameQueryListener::~CGameQueryListener()
{
	for (i32 iServer = 0; iServer < m_iServers; iServer++)
	{
		delete[] m_astrServers[iServer];
	}
}

void CGameQueryListener::Update()
{
	CleanUpServers();
}

void CGameQueryListener::AddServer(tukk description, tukk target, tukk additionalText, u32 ping)
{
	SGameServer server(target, description, additionalText, ping);
	//compare messages in showcase and increment if fitting
	std::vector<SGameServer>::iterator sIT;
	for (sIT = m_servers.begin(); sIT != m_servers.end(); ++sIT)
	{
		if ((*sIT).Compare(server))
		{
			(*sIT).Inc(server);
			return;
		}
	}

	//add server to list
	m_servers.push_back(server);

	//ping the servers when they are added ...
	ILanQueryListener* pLQL = static_cast<ILanQueryListener*>(m_pNetListener);
	pLQL->SendPingTo(server.m_target.c_str());
}

void CGameQueryListener::RemoveServer(string address)
{
	std::vector<SGameServer>::iterator it = m_servers.begin();
	for (; it != m_servers.end(); ++it)
	{
		if ((*it).m_target.compare(address) == 0)
		{
			m_servers.erase(it);
			return;
		}
	}
}

void CGameQueryListener::OnReceiveGameState(tukk fromAddress, XmlNodeRef xmlData)
{
	DRX_ASSERT(m_pNetListener);
	DrxLogAlways("Game running at: %s", fromAddress);
	DrxLogAlways("%s", xmlData->getXML().c_str());
}

void CGameQueryListener::Release()
{
	delete this;
}

void CGameQueryListener::SetNetListener(INetQueryListener* pListener)
{
	DRX_ASSERT(!m_pNetListener);
	m_pNetListener = pListener;
	DRX_ASSERT(m_pNetListener);
}

void CGameQueryListener::Complete()
{
	DRX_ASSERT(m_pNetListener);
	m_pNetListener->DeleteNetQueryListener();
}

void CGameQueryListener::AddPong(string address, u32 ping)
{
	std::vector<SGameServer>::iterator it = m_servers.begin();
	for (; it != m_servers.end(); ++it)
	{
		if ((*it).m_target.compare(address) == 0)
		{
			(*it).IncPing(ping);
			break;
		}
	}
}

void CGameQueryListener::GetServer(i32 number, tuk* server, tuk* data, i32& ping)
{
	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);
	if (number >= m_servers.size())
	{
		server = NULL;
		return;
	}

	//number = m_servers.size()-1 - number;	//list is slowest to fastest ...

	*server = (tuk)(m_servers[number].m_target.c_str());
	*data = (tuk)(m_servers[number].m_data.c_str());
	ping = m_servers[number].m_ping;
}

void CGameQueryListener::GetCurrentServers(tuk** pastrServers, i32& o_amount)
{
	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);
	if (m_servers.size() == 0)
	{
		o_amount = 0;
		return;
	}
	o_amount = m_servers.size();

	if (o_amount > m_iServers)// || o_amount < 0.1f*m_iServers)
	{
		for (i32 iServer = 0; iServer < m_iServers; iServer++)
		{
			delete[] m_astrServers[iServer];
		}
		m_iServers = std::max(o_amount, i32(120));
		for (i32 iServer = 0; iServer < m_iServers; iServer++)
		{
			m_astrServers[iServer] = new char[256];
		}
	}

	//servers = new tuk[m_servers.size()];
	*pastrServers = m_astrServers;

	std::vector<SGameServer>::iterator it = m_servers.begin();
	for (i32 s = 0; it != m_servers.end(); ++it, ++s)    //copy server address to return-vector
	{
		strcpy(m_astrServers[s], (*it).m_target.c_str());
	}
}

void CGameQueryListener::CleanUpServers()
{
	if (m_servers.size() == 0)
		return;

	CTimeValue now = gEnv->pTimer->GetFrameStartTime();

	std::vector<SGameServer>::iterator it;
	//kill old servers
	for (i32 i = 0; i < m_servers.size(); ++i)
	{
		u32 seconds = (u32)(now.GetSeconds() - m_servers[i].m_lastTime.GetSeconds());
		if (!seconds)
			continue;
		if (seconds > 10)
		{
			it = m_servers.begin();
			for (i32 inc = 0; inc < i; ++inc)
				++it;
			m_servers.erase(it);
			--i;
		}
	}
}

tukk CGameQueryListener::GetServerData(tukk server, i32& o_ping)
{
	o_ping = -1;

	std::vector<SGameServer>::iterator it = m_servers.begin();
	for (; it != m_servers.end(); ++it)
	{
		if ((*it).m_target.find(string(server)) != string::npos)
		{
			o_ping = (*it).m_ping;
			return (*it).m_data.c_str();
		}
	}
	return NULL;
}

void CGameQueryListener::RefreshPings()
{
	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);

	std::vector<SGameServer>::iterator it = m_servers.begin();
	ILanQueryListener* pLQL = static_cast<ILanQueryListener*>(m_pNetListener);
	for (; it != m_servers.end(); ++it)
		pLQL->SendPingTo((*it).m_target.c_str());
}

CGameQueryListener::SGameServer* CGameQueryListener::FindServer(tukk server)
{
	std::vector<SGameServer>::iterator it = m_servers.begin();
	for (; it != m_servers.end(); ++it)
		if (!strcmp((*it).m_target, server))
			return &(*it);
	return NULL;
}

void CGameQueryListener::ConnectToServer(tukk server)
{
	//first check the version of the server ...
	char myVersion[32];
	GetISystem()->GetProductVersion().ToShortString(myVersion);
	string version(myVersion);

	SGameServer* targetServer = FindServer(server);
	if (!targetServer)
	{
		DrxWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, "Selected server not found in list!");
		return;
	}

	if (version.compare(targetServer->GetServerGameVersion()) != 0)
	{
		DrxWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, "Game versions differ - not connecting!");
		return;
	}

	string addr(server);
	string port;
	size_t pos = addr.find(":");
	if (pos != string::npos) //space for port
	{
		port = addr.substr(pos + 1, addr.size() - pos);
		addr.erase(pos, addr.size() - pos);
	}

	IConsole* pConsole = gEnv->pConsole;

	pConsole->GetCVar("cl_serveraddr")->Set(addr.c_str());
	if (port.size() > 0)
		pConsole->GetCVar("cl_serverport")->Set(port.c_str());

	string tempHost = pConsole->GetCVar("cl_serveraddr")->GetString();

	SGameStartParams params;  //this would connect to a server
	params.flags = eGSF_Client;
	params.hostname = tempHost.c_str();
	params.pContextParams = NULL;
	params.port = pConsole->GetCVar("cl_serverport")->GetIVal();

	CDrxAction* action = (CDrxAction*) (gEnv->pGameFramework);
	if (action)
	{
		gEnv->pConsole->ExecuteString("net_lanbrowser 0");
		action->StartGameContext(&params);
	}
}

void CGameQueryListener::GetValuesFromData(tuk strData, SServerData* pServerData)
{
	string strSTD = strData;

	// Map
	i32 iFirst = strSTD.find_first_of("\"") + 1;
	i32 iSecond = strSTD.find_first_of("\"", iFirst);
	pServerData->strMap = strSTD.substr(iFirst, iSecond - iFirst);

	// Num players
	iFirst = strSTD.find_first_of("\"", iSecond + 1) + 1;
	iSecond = strSTD.find_first_of("\"", iFirst);
	pServerData->iNumPlayers = atoi(strSTD.substr(iFirst, iSecond - iFirst));

	// GameMode
	iFirst = strSTD.find_first_of("\"", iSecond + 1) + 1;
	iSecond = strSTD.find_first_of("\"", iFirst);
	pServerData->strMode = strSTD.substr(iFirst, iSecond - iFirst);

	// Version
	iFirst = strSTD.find_first_of("\"", iSecond + 1) + 1;
	iSecond = strSTD.find_first_of("\"", iFirst);
	pServerData->strVersion = strSTD.substr(iFirst, iSecond - iFirst);

	// Max players
	iFirst = strSTD.find_first_of("\"", iSecond + 1) + 1;
	iSecond = strSTD.find_first_of("\"", iFirst);
	pServerData->iMaxPlayers = atoi(strSTD.substr(iFirst, iSecond - iFirst));
}

void CGameQueryListener::GetMemoryStatistics(IDrxSizer* s)
{
	s->Add(*this);
	s->AddContainer(m_servers);
	for (size_t i = 0; i < m_servers.size(); i++)
	{
		s->Add(m_servers[i].m_target);
		s->Add(m_servers[i].m_description);
		s->Add(m_servers[i].m_data);
	}
	s->AddObject(m_astrServers, m_iServers * 256);
}
