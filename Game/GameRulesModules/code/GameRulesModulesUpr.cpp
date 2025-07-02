// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 

	-------------------------------------------------------------------------
	История:
	- 02:09:2009  : Created by Colin Gulliver

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRulesModulesUpr.h>
#include <drx3D/Game/GameRules.h>

#define GAMERULES_DEFINITIONS_XML_PATH		"Scripts/GameRules/GameModes.xml"

//------------------------------------------------------------------------
CGameRulesModulesUpr* CGameRulesModulesUpr::s_pInstance = NULL;

//------------------------------------------------------------------------
CGameRulesModulesUpr * CGameRulesModulesUpr::GetInstance( bool create /*= true*/ )
{
	if (create && !s_pInstance)
	{
		s_pInstance = new CGameRulesModulesUpr();
	}
	return s_pInstance;
}

//------------------------------------------------------------------------
CGameRulesModulesUpr::CGameRulesModulesUpr()
{
	assert(!s_pInstance);
	s_pInstance = this;
}

//------------------------------------------------------------------------
CGameRulesModulesUpr::~CGameRulesModulesUpr()
{
	assert(s_pInstance == this);
	s_pInstance = NULL;
}

// Implement register and create functions for each module type
#define GAMERULES_MODULE_LIST_FUNC(type, name, lowerCase, useInEditor)	\
	void CGameRulesModulesUpr::Register##name##Module( tukk moduleName, type *(*func)(), bool isAI )	\
	{	\
		m_##name##Classes.insert(TModuleClassMap_##name::value_type(moduleName, func));	\
	}	\
	type *CGameRulesModulesUpr::Create##name##Module(tukk moduleName)	\
	{	\
		TModuleClassMap_##name::iterator ite = m_##name##Classes.find(moduleName);	\
		if (ite == m_##name##Classes.end())	\
		{	\
			GameWarning("Unknown GameRules module: <%s>", moduleName);	\
			return NULL;	\
		}	\
		return (*ite->second)();	\
	}

GAMERULES_MODULE_TYPES_LIST

#undef GAMERULES_MODULE_LIST_FUNC

//------------------------------------------------------------------------
void CGameRulesModulesUpr::Init()
{
	XmlNodeRef root = gEnv->pSystem->LoadXmlFromFile( GAMERULES_DEFINITIONS_XML_PATH );
	if (root)
	{
		if (!stricmp(root->getTag(), "Modes"))
		{
			IGameRulesSystem *pGameRulesSystem = g_pGame->GetIGameFramework()->GetIGameRulesSystem();

			i32 numModes = root->getChildCount();

			for (i32 i = 0; i < numModes; ++ i)
			{
				XmlNodeRef modeXml = root->getChild(i);

				if (!stricmp(modeXml->getTag(), "GameMode"))
				{
					tukk modeName;

					if (modeXml->getAttr("name", &modeName))
					{
						pGameRulesSystem->RegisterGameRules(modeName, "GameRules");

						SGameRulesData gameRulesData;

						i32 numModeChildren = modeXml->getChildCount();
						for (i32 j = 0; j < numModeChildren; ++ j)
						{
							XmlNodeRef modeChildXml = modeXml->getChild(j);

							tukk nodeTag = modeChildXml->getTag();
							if (!stricmp(nodeTag, "Alias"))
							{
								tukk alias;
								if (modeChildXml->getAttr("name", &alias))
								{
									pGameRulesSystem->AddGameRulesAlias(modeName, alias);
								}
							}
							else if (!stricmp(nodeTag, "LevelLocation"))
							{
								tukk path;
								if (modeChildXml->getAttr("path", &path))
								{
									pGameRulesSystem->AddGameRulesLevelLocation(modeName, path);
								}
							}
							else if (!stricmp(nodeTag, "Rules"))
							{
								tukk path;
								if (modeChildXml->getAttr("path", &path))
								{
									gameRulesData.m_rulesXMLPath = path;
								}
							}
							else if( !stricmp(nodeTag, "DefaultHudState"))
							{
								tukk name;
								if (modeChildXml->getAttr("name", &name))
								{
									gameRulesData.m_defaultHud = name;
								}
							}
						}

						// Check if we're a team game
						i32 teamBased = 0;
						modeXml->getAttr("teamBased", teamBased);
						gameRulesData.m_bIsTeamGame = (teamBased != 0);

						// Check if this mode uses the gamelobby for team balancing
						i32 useLobbyTeamBalancing = 0;
						modeXml->getAttr("useLobbyTeamBalancing", useLobbyTeamBalancing);
						gameRulesData.m_bUseLobbyTeamBalancing = (useLobbyTeamBalancing != 0);

						// Check if this mode uses the player team visualization to change player models
						i32 usePlayerTeamVisualization = 1;
						modeXml->getAttr("usePlayerTeamVisualization", usePlayerTeamVisualization);
						gameRulesData.m_bUsePlayerTeamVisualization = (usePlayerTeamVisualization != 0);

						// Insert gamerule specific data
						m_rulesData.insert(TDataMap::value_type(modeName, gameRulesData));
					}
					else
					{
						DrxLogAlways("CGameRulesModulesUpr::Init(), invalid 'GameMode' node, requires 'name' attribute");
					}
				}
				else
				{
					DrxLogAlways("CGameRulesModulesUpr::Init(), invalid xml, expected 'GameMode' node, got '%s'", modeXml->getTag());
				}
			}
		}
		else
		{
			DrxLogAlways("CGameRulesModulesUpr::Init(), invalid xml, expected 'Modes' node, got '%s'", root->getTag());
		}
	}
}

//------------------------------------------------------------------------
tukk  CGameRulesModulesUpr::GetXmlPath( tukk gameRulesName ) const
{
	TDataMap::const_iterator it = m_rulesData.find(gameRulesName);
	if (it == m_rulesData.end())
	{
		return NULL;
	}
	return it->second.m_rulesXMLPath.c_str();
}

//------------------------------------------------------------------------
tukk  CGameRulesModulesUpr::GetDefaultHud( tukk gameRulesName ) const
{
	TDataMap::const_iterator it = m_rulesData.find(gameRulesName);
	if (it == m_rulesData.end())
	{
		return NULL;
	}
	return it->second.m_defaultHud.c_str();
}

//------------------------------------------------------------------------
i32 CGameRulesModulesUpr::GetRulesCount()
{
	return m_rulesData.size();
}

//------------------------------------------------------------------------
tukk CGameRulesModulesUpr::GetRules(i32 index)
{
	assert (index >= 0 && index < (i32)m_rulesData.size());
	TDataMap::const_iterator iter = m_rulesData.begin();
	std::advance(iter, index);
	return iter->first.c_str();
}

//------------------------------------------------------------------------
bool CGameRulesModulesUpr::IsTeamGame( tukk gameRulesName) const
{
	TDataMap::const_iterator it = m_rulesData.find(gameRulesName);
	if (it == m_rulesData.end())
	{
		return false;
	}
	return it->second.m_bIsTeamGame;
}

//------------------------------------------------------------------------
bool CGameRulesModulesUpr::UsesLobbyTeamBalancing( tukk gameRulesName ) const
{
	TDataMap::const_iterator it = m_rulesData.find(gameRulesName);
	if (it == m_rulesData.end())
	{
		return false;
	}
	return it->second.m_bUseLobbyTeamBalancing;
}

//------------------------------------------------------------------------
bool CGameRulesModulesUpr::UsesPlayerTeamVisualization( tukk gameRulesName ) const
{
	TDataMap::const_iterator it = m_rulesData.find(gameRulesName);
	if (it == m_rulesData.end())
	{
		return false;
	}
	return it->second.m_bUsePlayerTeamVisualization;
}

//------------------------------------------------------------------------
bool CGameRulesModulesUpr::IsValidGameRules(tukk gameRulesName) const
{
	TDataMap::const_iterator it = m_rulesData.find(gameRulesName);
	if (it == m_rulesData.end())
	{
		return false;
	}
	return true;
}
