// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 

	-------------------------------------------------------------------------
	История:
	- 02:09:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GameRulesModulesUpr_h_
#define _GameRulesModulesUpr_h_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameRulesModulesUpr.h>

class CGameRulesModulesUpr : public IGameRulesModulesUpr
{
private:
	static CGameRulesModulesUpr *s_pInstance;

	typedef DrxFixedStringT<32>	TFixedString_32;
	typedef DrxFixedStringT<64>	TFixedString_64;

// Create class map and register/create functions for each module type
#define GAMERULES_MODULE_LIST_FUNC(type, name, lowerCase, useInEditor)	\
	private:	\
		typedef std::map<TFixedString_32, type*(*)()> TModuleClassMap_##name;	\
		TModuleClassMap_##name m_##name##Classes;	\
	public:		\
		void RegisterFactory(tukk moduleName, type*(*func)(), bool isAI) { Register##name##Module(moduleName, func, isAI); };	\
		void Register##name##Module(tukk moduleName, type*(*func)(), bool isAI);	\
		type *Create##name##Module(tukk moduleName);

	GAMERULES_MODULE_TYPES_LIST

#undef GAMERULES_MODULE_LIST_FUNC

	struct SGameRulesData{
		TFixedString_64 m_rulesXMLPath;
		TFixedString_64 m_defaultHud;
		bool m_bIsTeamGame;
		bool m_bUseLobbyTeamBalancing;
		bool m_bUsePlayerTeamVisualization;


		//ctor
		SGameRulesData() : m_rulesXMLPath(""), m_defaultHud(""), m_bIsTeamGame(false), m_bUseLobbyTeamBalancing(false), m_bUsePlayerTeamVisualization(true) {}
	};

	typedef std::map<TFixedString_32, SGameRulesData> TDataMap;
	TDataMap m_rulesData;

public:
	static CGameRulesModulesUpr *GetInstance(bool create = true);

	CGameRulesModulesUpr();
	virtual ~CGameRulesModulesUpr();

	void Init();

	// Summary
	//	 Returns the path for the gamerules XML description.
	tukk GetXmlPath(tukk gameRulesName) const;

	// Summary
	//	 Returns the default HUDState name path for the given gamerules.
	tukk GetDefaultHud(tukk gameRulesName) const;

	// Summary
	//	Returns the number of game rules
	i32 GetRulesCount();

	// Summary
	//	Returns the name of the nth GameRules
	tukk GetRules(i32 index);

	// Summary
	//	Determines if the specified gameRules is a team game
	bool IsTeamGame(tukk gameRulesName) const;

	// Summary
	//	Determines if the specified gameRules uses team balancing
	bool UsesLobbyTeamBalancing(tukk gameRulesName) const;

	// Summary
	//	Determines if the specified gameRules uses team visualization
	bool UsesPlayerTeamVisualization(tukk gameRulesName) const;

	// Summary
	// Determines if game rules are valid
	bool IsValidGameRules(tukk gameRulesName) const;
};

#endif // _GameRulesModulesUpr_h_
