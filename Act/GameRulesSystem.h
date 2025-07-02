// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 15:9:2004   10:30 : Created by Mathieu Pinard
   - 20:10:2004   10:30 : Added IGameRulesSystem interface (Craig Tiller)

*************************************************************************/
#ifndef __GAMERULESYSTEM_H__
#define __GAMERULESYSTEM_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "IGameRulesSystem.h"

class CGameServerNub;

class CGameRulesSystem
	: public IGameRulesSystem
	  , public INetworkedClientListener
{
	typedef struct SGameRulesDef
	{
		string              extension;
		std::vector<string> aliases;
		std::vector<string> maplocs;

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(extension);
			pSizer->AddObject(aliases);
			pSizer->AddObject(maplocs);
		}
	}SGameRulesDef;

	typedef std::map<string, SGameRulesDef> TGameRulesMap;
public:
	CGameRulesSystem(ISystem* pSystem, IGameFramework* pGameFW);
	~CGameRulesSystem();

	void Release() { delete this; };

	// IGameRulesSystem
	virtual bool        RegisterGameRules(tukk rulesName, tukk extensionName, bool bUseScript) override;
	virtual bool        CreateGameRules(tukk rulesName) override;
	virtual bool        DestroyGameRules() override;

	virtual void        AddGameRulesAlias(tukk gamerules, tukk alias) override;
	virtual void        AddGameRulesLevelLocation(tukk gamerules, tukk mapLocation) override;
	virtual tukk GetGameRulesLevelLocation(tukk gamerules, i32 i) override;

	virtual tukk GetGameRulesName(tukk alias) const override;

	virtual bool        HaveGameRules(tukk rulesName) override;

	virtual void        SetCurrentGameRules(IGameRules* pGameRules) override;
	virtual IGameRules* GetCurrentGameRules() const override;
	// ~IGameRulesSystem

	// INetworkedClientListener
	virtual void OnLocalClientDisconnected(EDisconnectionCause cause, tukk description) override;

	virtual bool OnClientConnectionReceived(i32 channelId, bool bIsReset) override;
	virtual bool OnClientReadyForGameplay(i32 channelId, bool bIsReset) override;
	virtual void OnClientDisconnected(i32 channelId, EDisconnectionCause cause, tukk description, bool bKeepClient) override;
	virtual bool OnClientTimingOut(i32 channelId, EDisconnectionCause cause, tukk description) override;
	// ~INetworkedClientListener

	void        GetMemoryStatistics(IDrxSizer* s);

private:
	SGameRulesDef*           GetGameRulesDef(tukk name);
	static IEntityComponent* CreateGameObject(
	  IEntity* pEntity, SEntitySpawnParams& params, uk pUserData);

	IGameFramework* m_pGameFW;

	EntityId        m_currentGameRules;
	IGameRules*     m_pGameRules;

	TGameRulesMap   m_GameRules;
};

#endif // __GAMERULESYSTEM_H__
