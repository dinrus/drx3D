// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   GameTokenSystem.h
//  Version:     v1.00
//  Created:     20/10/2005 by Craig,Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _GameTokenSystem_h_
#define _GameTokenSystem_h_
#pragma once

#include <drx3D/CoreX/Game/IGameTokens.h>
#include <drx3D/CoreX/StlUtils.h>

class CGameToken;

//////////////////////////////////////////////////////////////////////////
// Game Token Upr implementation class.
//////////////////////////////////////////////////////////////////////////
class CGameTokenSystem : public IGameTokenSystem
{
	friend class CGameTokenIterator;
public:
	CGameTokenSystem();
	~CGameTokenSystem();

	//////////////////////////////////////////////////////////////////////////
	// IGameTokenSystem
	//////////////////////////////////////////////////////////////////////////
	virtual IGameToken*   SetOrCreateToken(tukk sTokenName, const TFlowInputData& defaultValue) override;
	virtual void          DeleteToken(IGameToken* pToken) override;
	virtual IGameToken*   FindToken(tukk sTokenName) override;
	virtual void          RenameToken(IGameToken* pToken, tukk sNewName) override;

	virtual IGameTokenIt* GetGameTokenIterator() override;

	virtual void          RegisterListener(tukk sGameToken, IGameTokenEventListener* pListener, bool bForceCreate, bool bImmediateCallback) override;
	virtual void          UnregisterListener(tukk sGameToken, IGameTokenEventListener* pListener) override;

	virtual void          RegisterListener(IGameTokenEventListener* pListener) override;
	virtual void          UnregisterListener(IGameTokenEventListener* pListener) override;

	virtual void          TriggerTokensAsChanged() override;

	virtual void          Reset() override;
	virtual void          Unload() override;
	virtual void          Serialize(TSerialize ser) override;

	virtual void          DebugDraw() override;

	virtual void          LoadLibs(tukk sFileSpec) override;
	virtual void          RemoveLibrary(tukk sPrefix) override;

	virtual void          SerializeSaveLevelToLevel(tukk* ppGameTokensList, u32 numTokensToSave) override;
	virtual void          SerializeReadLevelToLevel() override;

	virtual void          GetMemoryStatistics(IDrxSizer* s) override;

	virtual void          SetGoingIntoGame(bool bGoingIntoGame) override { m_bGoingIntoGame = bGoingIntoGame; }
	//////////////////////////////////////////////////////////////////////////

	CGameToken* GetToken(tukk sTokenName);
	void        Notify(EGameTokenEvent event, CGameToken* pToken);
	void        DumpAllTokens();

private:
	bool _InternalLoadLibrary(tukk filename, tukk tag);

	// Use Hash map for speed.
	typedef std::unordered_map<tukk , CGameToken*, stl::hash_stricmp<tukk >, stl::hash_stricmp<tukk >> GameTokensMap;

	typedef std::vector<IGameTokenEventListener*>                                                                        Listeners;
	Listeners m_listeners;

	// Loaded libraries.
	std::vector<string>          m_libraries;

	GameTokensMap*               m_pGameTokensMap; // A pointer so it can be fully unloaded on level unload
	class CScriptBind_GameToken* m_pScriptBind;
	XmlNodeRef                   m_levelToLevelSave;
	bool                         m_bGoingIntoGame;

#ifdef _GAMETOKENSDEBUGINFO
	typedef std::set<string> TDebugListMap;
	TDebugListMap m_debugList;
	struct SDebugHistoryEntry
	{
		string     tokenName;
		string     value;
		CTimeValue timeChanged;
	};

	enum { DBG_HISTORYSIZE = 25 };
	u32                          m_historyStart; // start of the circular list in m_debugHistory.
	u32                          m_historyEnd;
	u32                          m_oldNumHistoryLines; // just to control dynamic changes without using a callback
	std::vector<SDebugHistoryEntry> m_debugHistory;       // works like a circular list
	static i32                      m_CVarShowDebugInfo;
	static i32                      m_CVarPosX;
	static i32                      m_CVarPosY;
	static i32                      m_CVarNumHistoricLines;
	static ICVar*                   m_pCVarFilter;

	void        DrawToken(tukk pTokenName, tukk pTokenValue, const CTimeValue& timeChanged, i32 line);
	void        ClearDebugHistory();
	i32         GetHistoryBufferSize() { return min(i32(DBG_HISTORYSIZE), m_CVarNumHistoricLines); }
	tukk GetTokenDebugString(CGameToken* pToken);

public:
	virtual void AddTokenToDebugList(tukk pToken) override;
	virtual void RemoveTokenFromDebugList(tukk pToken) override;

#endif

};

#endif // _GameTokenSystem_h_
