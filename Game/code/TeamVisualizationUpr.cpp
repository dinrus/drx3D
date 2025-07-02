// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/TeamVisualizationUpr.h>
#include <drx3D/Game/Utility/DesignerWarning.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Game/RecordingSystem.h>

#define PLAYER_TEAM_VISUALIZATION_XML	"Scripts/PlayerTeamVisualization.xml"

CTeamVisualizationUpr::CTeamVisualizationUpr()
{

}

CTeamVisualizationUpr::~CTeamVisualizationUpr()
{
}

void CTeamVisualizationUpr::Init()
{
	XmlNodeRef xml = GetISystem()->LoadXmlFromFile( PLAYER_TEAM_VISUALIZATION_XML );
	if(xml && g_pGame->GetGameLobby())
	{
		if (g_pGame->GetGameLobby()->UsePlayerTeamVisualization())
		{
			InitTeamVisualizationData(xml);
		}
	}
}

void CTeamVisualizationUpr::InitTeamVisualizationData( XmlNodeRef xmlNode )
{
	if(m_teamVisualizationPartsMap.empty())
	{
		IMaterialUpr *pMaterialUpr = gEnv->p3DEngine->GetMaterialUpr();

		// Parse Team vis data and add to m_teamVisualizationPartsMap; 
		XmlNodeRef pPlayerTeamVisualization = xmlNode->findChild("PlayerTeamVisualization");
		DesignerWarning(pPlayerTeamVisualization, "expected to find <PlayerTeamVisualization> </PlayerTeamVisualization>, not found");
		if(pPlayerTeamVisualization)
		{
			// Grab each model setup node
			i32k modelCount = pPlayerTeamVisualization->getChildCount();
			for(i32 i = 0; i < modelCount; ++i)
			{
				XmlNodeRef pModelSetup = pPlayerTeamVisualization->getChild(i);
				if(pModelSetup)
				{
					// Friendly 
					XmlNodeRef friendlyNode = pModelSetup->findChild("Friendly");
					DesignerWarning(friendlyNode, "missing <Friendly> </Friendly> tags in model setup <%d> - PlayerTeamVisualization.xml", i);
					if(friendlyNode)
					{
						// Hostile
						XmlNodeRef hostileNode = pModelSetup->findChild("Hostile");
						DesignerWarning(hostileNode, "missing <Hostile> </Hostile> tags in model setup <%d> - PlayerTeamVisualization.xml", i);
						if(hostileNode)
						{

							XmlNodeRef attachmentsNode = pModelSetup->findChild("BodyAttachments");
							i32k numAttachments = attachmentsNode->getChildCount();
							DesignerWarning(attachmentsNode && numAttachments > 0, "missing <BodyAttachments> </bodyAttachments> tags in model setup <%d> or no child <BodyAttachment> elements - PlayerTeamVisualization.xml", i);
							if(attachmentsNode && numAttachments > 0)
							{
								tukk pModelName = pModelSetup->getAttr("name");
								DesignerWarning(pModelName && pModelName[0], "missing <Model> tag - or <Model name=""> attribute invalid - in model setup <%d> - PlayerTeamVisualization.xml", i);
								if(pModelName && pModelName[0])
								{
									// Add new + Fill in details
									TModelNameCRC modelNameCRC = CCrc32::ComputeLowercase(pModelName); 
									DRX_ASSERT(m_teamVisualizationPartsMap.find(modelNameCRC) == m_teamVisualizationPartsMap.end());
									m_teamVisualizationPartsMap[modelNameCRC] = SModelMaterialSetup(); 
									SModelMaterialSetup& newConfig = m_teamVisualizationPartsMap[modelNameCRC];

									// Get materials
									newConfig.SetMaterial(eMI_AliveFriendly, pMaterialUpr->LoadMaterial(friendlyNode->getAttr("MaterialName")));
									newConfig.SetMaterial(eMI_AliveHostile, pMaterialUpr->LoadMaterial(hostileNode->getAttr("MaterialName")));

									// Hostile
									XmlNodeRef deadFriendlyNode = pModelSetup->findChild("DeadFriendly");
									DesignerWarning(deadFriendlyNode, "missing <DeadFriendly> </DeadFriendly> tags in model setup <%d> - PlayerTeamVisualization.xml", i);
									if(deadFriendlyNode)
									{
										newConfig.SetMaterial(eMI_DeadFriendly, pMaterialUpr->LoadMaterial(deadFriendlyNode->getAttr("MaterialName")));
									}

									XmlNodeRef deadHostileNode = pModelSetup->findChild("DeadHostile");
									DesignerWarning(deadHostileNode, "missing <deadHostileNode> </deadHostileNode> tags in model setup <%d> - PlayerTeamVisualization.xml", i);
									if(deadHostileNode)
									{
										newConfig.SetMaterial(eMI_DeadHostile, pMaterialUpr->LoadMaterial(deadHostileNode->getAttr("MaterialName")));
									}

									// Attachments
									newConfig.m_attachments.reserve(numAttachments);
									for(i32 j = 0; j < numAttachments; ++j)
									{
										XmlNodeRef attachmentNode = attachmentsNode->getChild(j);
										newConfig.m_attachments.push_back(CCrc32::ComputeLowercase(attachmentNode->getAttr("name")));
									}	
									continue;
								}	
							}
						}
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CTeamVisualizationUpr::RefreshPlayerTeamMaterial( const EntityId playerId ) const
{
	if(IEntity* pEntity = gEnv->pEntitySystem->GetEntity(playerId))
	{
		CGameRules* pGameRules = g_pGame->GetGameRules();
		CGameRules::eThreatRating threatRating = pGameRules->GetThreatRating(g_pGame->GetClientActorId(), playerId);
		RefreshTeamMaterial( pEntity, true, threatRating==CGameRules::eFriendly );
	}
}

void CTeamVisualizationUpr::RefreshTeamMaterial( IEntity* pEntity, const bool isAlive, const bool isFriendly ) const
{
	if(ICharacterInstance* pCharInstance =  pEntity->GetCharacter(0))
	{
		tukk pPlayerModelName = pCharInstance->GetFilePath();
		if(pPlayerModelName && pPlayerModelName[0])
		{
			TModelNameCRC modelNameCRC = CCrc32::ComputeLowercase(pPlayerModelName); 
			TModelPartsMap::const_iterator iter = m_teamVisualizationPartsMap.find(modelNameCRC);
			if(iter != m_teamVisualizationPartsMap.end())
			{
				iter->second.ApplyMaterial(pCharInstance, GetMaterialIndex(isAlive, isFriendly));
			}
		}
	}
}

//------------------------------------------------------------------------
void CTeamVisualizationUpr::ProcessTeamChangeVisualization(EntityId entityId) const
{
	if(!m_teamVisualizationPartsMap.empty())
	{
		if(entityId == g_pGame->GetClientActorId())
		{
			// If local player has changed team, refresh team materials for *all* players in game
			CGameRules* pGameRules = g_pGame->GetGameRules();
			CGameRules::TPlayers players;
			pGameRules->GetPlayers(players);

			CGameRules::TPlayers::const_iterator iter = players.begin();
			CGameRules::TPlayers::const_iterator end = players.end();
			while(iter != end)
			{
				RefreshPlayerTeamMaterial(*iter); 
				++iter;
			}
		}
		else // If remote player has changed team, just refresh that player.
		{
			RefreshPlayerTeamMaterial(entityId); 
		}
	}
}

void CTeamVisualizationUpr::OnPlayerTeamChange( const EntityId playerId ) const
{
	ProcessTeamChangeVisualization(playerId);
	// Look at updating corpses here too if player changing team is the local player?
}

void CTeamVisualizationUpr::SModelMaterialSetup::ApplyMaterial( ICharacterInstance* pCharInst, const EMaterialIndex materialIdx ) const
{
	// Run through and set material on desired attachments
	IMaterial* pMaterial = m_materials[materialIdx];
	if(pCharInst && pMaterial)
	{
		if(IAttachmentUpr* pAttachmentUpr = pCharInst->GetIAttachmentUpr())
		{
			i32k count = m_attachments.size();
			for(i32 i=0; i<count; i++)
			{
				if(IAttachment* pAttachment = pAttachmentUpr->GetInterfaceByNameCRC(m_attachments[i]))
				{
					if(IAttachmentObject* pAttachmentObj = pAttachment->GetIAttachmentObject())
					{
						pAttachmentObj->SetReplacementMaterial(pMaterial);
					}
				}
			}
		}
	}
}

CTeamVisualizationUpr::SModelMaterialSetup::SModelMaterialSetup()
{
	for(i32 i=0; i<eMI_Total; i++)
	{
		m_materials[i] = NULL;
	}
}

CTeamVisualizationUpr::SModelMaterialSetup::~SModelMaterialSetup()
{
	for(i32 i=0; i<eMI_Total; i++)
	{
		SAFE_RELEASE(m_materials[i]);
	}
}

void CTeamVisualizationUpr::SModelMaterialSetup::SetMaterial( const EMaterialIndex idx, IMaterial* pMaterial )
{
	if(m_materials[idx]!=pMaterial)
	{
		SAFE_RELEASE(m_materials[idx]);
		m_materials[idx] = pMaterial;
		if(m_materials[idx])
		{
			m_materials[idx]->AddRef();
		}
	}
}
