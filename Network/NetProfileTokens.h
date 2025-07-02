// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: keep list of profile and its token ids
             profile provides its token id during connection
             server uses this server to validate token per profile
   -------------------------------------------------------------------------
   История:
   - 29/09/2009 : Created by Sergii Rustamov
*************************************************************************/

#ifndef __NET_PROFILE_TOKENS_H__
#define __NET_PROFILE_TOKENS_H__

#pragma once

#include <drx3D/Network/INetwork.h>
#include <drx3D/Network/INetworkService.h>

class CNetProfileTokens : public INetworkService, public INetProfileTokens
{
private:
	typedef std::map<u32, u32> TTokenEntities;

public:
	CNetProfileTokens();

	// INetworkService
	virtual ENetworkServiceState GetState()                                                                  { return m_state; }
	virtual void                 Close();
	virtual void                 CreateContextViewExtensions(bool server, IContextViewExtensionAdder* adder) {};
	virtual IServerBrowser*      GetServerBrowser()                                                          { return 0; }
	virtual INetworkChat*        GetNetworkChat()                                                            { return 0; }
	virtual IServerReport*       GetServerReport()                                                           { return 0; }
	virtual IStatsTrack*         GetStatsTrack(i32 version)                                                  { return 0; }
	virtual INatNeg*             GetNatNeg(INatNegListener* const)                                           { return 0; }
	virtual INetworkProfile*     GetNetworkProfile()                                                         { return 0; }
	virtual ITestInterface*      GetTestInterface()                                                          { return 0; }
	virtual IFileDownloader*     GetFileDownloader()                                                         { return 0; }
	virtual void                 GetMemoryStatistics(IDrxSizer* pSizer)                                      {}
	virtual IPatchCheck*         GetPatchCheck()                                                             { return 0; }
	virtual IHTTPGateway*        GetHTTPGateway()                                                            { return 0; }
	virtual INetProfileTokens*   GetNetProfileTokens()                                                       { return static_cast<INetProfileTokens*>(this); }

	// INetProfileTokens
	virtual void AddToken(u32 profile, u32 token);
	virtual bool IsValid(u32 profile, u32 token);
	virtual void Init();
	virtual void Lock();
	virtual void Unlock();

private:
	ENetworkServiceState m_state;
	TTokenEntities       m_tokens;
	DrxCriticalSection   m_lockTokens;
};

#endif // __NET_PROFILE_TOKENS_H__
