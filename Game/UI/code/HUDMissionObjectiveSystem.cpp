// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/HUDMissionObjectiveSystem.h>

#include <drx3D/Network/ISerialize.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/HUDEventDispatcher.h>

#include <drx3D/Game/GameRules.h>

void CHUDMissionObjective::SetStatus(HUDMissionStatus status)
{
	if (status == m_eStatus)
		return;

	if(m_eStatus == COMPLETED && status == FAILED)
	{
		DrxLog("Changing mission objective status from COMPLETED to FAILED; This change will be ignored.");
		return;
	}

	m_eStatus = status;

	NotifyStatusChange();
}

void CHUDMissionObjective::NotifyStatusChange()
{
	assert (m_pMOS != 0);

	SHUDEvent objectiveEvent(eHUDEvent_OnObjectiveChanged);
	objectiveEvent.AddData(SHUDEventData((uk )this));
	objectiveEvent.AddData(SHUDEventData(IsSilent()));
	CHUDEventDispatcher::CallEvent(objectiveEvent);

	SendRadarEvent();

	m_lastTimeChanged = gEnv->pTimer->GetFrameStartTime().GetSeconds();
}

void CHUDMissionObjective::SendRadarEvent()
{
	EntityId trackedEntityId = GetTrackedEntity();
	if (trackedEntityId)
	{
		SHUDEvent radarEvent;
		if(m_eStatus == ACTIVATED)
			radarEvent.eventType = eHUDEvent_AddEntity;
		else
			radarEvent.eventType = eHUDEvent_RemoveEntity;
		radarEvent.AddData(SHUDEventData((i32)trackedEntityId));
		CHUDEventDispatcher::CallEvent(radarEvent);
	}
}

CHUDMissionObjectiveSystem::CHUDMissionObjectiveSystem()
: m_bLoadedObjectives(false)
, m_currentAnalysis("")
{
	if (gEnv->pEntitySystem)
		gEnv->pEntitySystem->AddSink(this, IEntitySystem::OnReused, 0);
}

CHUDMissionObjectiveSystem::~CHUDMissionObjectiveSystem()
{
	if (gEnv->pEntitySystem)
		gEnv->pEntitySystem->RemoveSink(this);
}

void CHUDMissionObjectiveSystem::Reset()
{
	stl::free_container(m_currentMissionObjectives);
	ClearObjectiveAnalysisData();
	m_bLoadedObjectives = false;
}

void CHUDMissionObjectiveSystem::OnReused( IEntity *pEntity, SEntitySpawnParams &params )
{
	// Find any objectives set to track this entity, to handle special logic for them
	TObjectives::iterator itObjective = m_currentMissionObjectives.begin();
	TObjectives::iterator itObjectiveEnd = m_currentMissionObjectives.end();
	for (; itObjective != itObjectiveEnd; ++itObjective)
	{
		CHUDMissionObjective &objective = *itObjective;
		if (objective.GetTrackedEntity() == params.id)
		{
			// Send radar event again
			objective.SendRadarEvent();
		}
	}
}

void CHUDMissionObjectiveSystem::LoadLevelObjectives(tukk levelpath, bool forceReloading)
{
	if(!m_bLoadedObjectives || forceReloading)
	{
		m_currentMissionObjectives.clear();
		m_currentMissionObjectives.reserve(256);

		//ClearObjectiveAnalysisData();

		LoadLevelObjectivesInternal(NULL);
		//LoadObjectiveAnalysisData(NULL);
		if(levelpath)
		{
			LoadLevelObjectivesInternal(levelpath);
			//LoadObjectiveAnalysisData(levelpath);
		}

		m_bLoadedObjectives = true;

		TObjectives(m_currentMissionObjectives).swap(m_currentMissionObjectives);
	}
}

void CHUDMissionObjectiveSystem::LoadLevelObjectivesInternal(tukk levelpath)
{
	DrxFixedStringT<128> filename;
	if (levelpath==NULL)
	{
		// there is no Objectives_global.xml, but there is a file with the previous standard naming
		// load the file with old name for backwards compatibility
		if (!gEnv->pDrxPak->IsFileExist("Libs/UI/Objectives_global.xml")
			&& gEnv->pDrxPak->IsFileExist("Libs/UI/Objectives_new.xml"))
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "File 'Objectives_new.xml' is deprecated and should be renamed to 'Objectives_global.xml'");
			filename = "Libs/UI/Objectives_new.xml";
		}
		else
		{
			filename = "Libs/UI/Objectives_global.xml";
		}
	}
	else
	{
		filename.Format("%s/leveldata/Objectives.xml", levelpath);
	}
	/*if(gEnv->bMultiplayer)
	{
		CGameRules *pGameRules = g_pGame->GetGameRules();
		if(stricmp (pGameRules->GetEntity()->GetClass()->GetName(), "Coop"))
			filename = "Libs/UI/MP_Objectives.xml";
	}*/

	XmlNodeRef missionObjectives = GetISystem()->LoadXmlFromFile(filename.c_str());
	if (missionObjectives == 0)
		return;

	for(i32 tag = 0; tag < missionObjectives->getChildCount(); ++tag)
	{
		XmlNodeRef mission = missionObjectives->getChild(tag);
		tukk attrib;
		tukk objective;
		tukk text;
		tukk optional;

		tukk levelName;
		if (!mission->getAttr("name", &levelName))
		{
			levelName = mission->getTag();
		}

		for(i32 obj = 0; obj < mission->getChildCount(); ++obj)
		{
			XmlNodeRef objectiveNode = mission->getChild(obj);
			string id(levelName);
			id.append(".");
			id.append(objectiveNode->getTag());
			if(objectiveNode->getAttributeByIndex(0, &attrib, &objective) && objectiveNode->getAttributeByIndex(1, &attrib, &text))
			{
				bool secondaryObjective = false;
				i32 attribs = objectiveNode->getNumAttributes();
				for(i32 attribIndex = 2; attribIndex < attribs; ++attribIndex)
				{
					if(objectiveNode->getAttributeByIndex(attribIndex, &attrib, &optional))
					{
						if(attrib)
						{
							if(!stricmp(attrib, "Secondary"))
							{
								if(!stricmp(optional, "true"))
									secondaryObjective = true;
							}
						}
					}
				}
				m_currentMissionObjectives.push_back(CHUDMissionObjective(this, id.c_str(), objective, text, secondaryObjective));
			}
			else
				GameWarning("Error reading mission objectives.");
		}
	}
}

void CHUDMissionObjectiveSystem::LoadObjectiveAnalysisData(tukk levelpath)
{
	DrxFixedStringT<128> filename;
	if(levelpath==NULL)
	{
		filename = "Libs/UI/ObjectivesAnalysis.xml";
	}
	else
	{
		filename.Format("%s/leveldata/ObjectivesAnalysis.xml", levelpath);
	}

	XmlNodeRef root = GetISystem()->LoadXmlFromFile(filename.c_str());
	if (root == 0)
		return;

	XmlNodeRef analysisDataContainerNode = root->findChild("AnalysisData");		
	SObjectiveAnalysisData analysisData;
	SSingleObjectiveAnalysisData singleAnalysisData;
	if (analysisDataContainerNode != 0)
	{
		for(i32 i = 0; i < analysisDataContainerNode->getChildCount(); ++i)
		{
			XmlNodeRef analysisDataNode = analysisDataContainerNode->getChild(i);
			tukk szOuterId = analysisDataNode->getAttr("Id");
			if (szOuterId == NULL || szOuterId[0] == '\0')
			{
				GameWarning("CHUDMissionObjectiveSystem::LoadObjectiveAnalysisData: Failed to load analysisdata, invalid id");
				continue;
			}
			
			analysisData.m_singleObjectiveData.clear();
			i32k iNumSingleAnalysis = analysisDataNode->getChildCount();
			bool bFailedAddedSingleAnalysis = false;
			for (i32 j = 0; j < iNumSingleAnalysis; j++)
			{
				XmlNodeRef singleAnalysisDataNode = analysisDataNode->getChild(j);

				tukk szSingleId;
				i32 iUseOuterId;
				// UseOuterId saves needing to duplicate text to the outer Id
				bool bResult = singleAnalysisDataNode->getAttr("UseOuterId", iUseOuterId);
				if (bResult && (iUseOuterId == 1))
				{
					szSingleId = szOuterId;
				}
				else
				{
					szSingleId = analysisDataNode->getAttr("Id");
				}

				if (szSingleId == NULL || szSingleId[0] == '\0')
				{
					GameWarning("CHUDMissionObjectiveSystem::LoadObjectiveAnalysisData: Failed to load single analysisdata section for analysis: %s, invalid id", szOuterId);
					bFailedAddedSingleAnalysis = true;
					break;
				}

				singleAnalysisData.m_id = szSingleId;
				singleAnalysisDataNode->getAttr("Percent", singleAnalysisData.m_fPercent);
				analysisData.m_singleObjectiveData.push_back(singleAnalysisData);
			}

			if (bFailedAddedSingleAnalysis || analysisData.m_singleObjectiveData.size() < 1)
			{
				GameWarning("CHUDMissionObjectiveSystem::LoadObjectiveAnalysisData: Failed to load analysisdata: %s, problem with section data", szOuterId);
				continue;
			}

			analysisDataNode->getAttr("OverrideSectionID",analysisData.m_iOverrideIndex);
			analysisDataNode->getAttr("AttachInWorld", analysisData.m_bAttachInWorld);
			m_objectiveAnalysisData[szOuterId] = analysisData;
		}
	}
}

void CHUDMissionObjectiveSystem::ClearObjectiveAnalysisData()
{
	m_objectiveAnalysisData.clear();
}

void CHUDMissionObjectiveSystem::DeactivateObjectives(bool leaveOutRecentlyChanged)
{
	for(i32 i = 0; i < i32(m_currentMissionObjectives.size()); ++i)
	{
		if(!leaveOutRecentlyChanged ||
			(m_currentMissionObjectives[i].m_lastTimeChanged - gEnv->pTimer->GetFrameStartTime().GetSeconds() > 0.2f))
		{
			bool isSilent = m_currentMissionObjectives[i].IsSilent();
			m_currentMissionObjectives[i].SetSilent(true);
			m_currentMissionObjectives[i].SetStatus(CHUDMissionObjective::DEACTIVATED);
			m_currentMissionObjectives[i].SetSilent(isSilent);
		}
	}
}
CHUDMissionObjective* CHUDMissionObjectiveSystem::GetMissionObjective(tukk id)
{
	TObjectives::iterator it;
	for(it = m_currentMissionObjectives.begin(); it != m_currentMissionObjectives.end(); ++it)
	{
		if(!strcmp( (*it).GetID(), id))
			return &(*it);
	}

	return NULL;
}


CHUDMissionObjective* CHUDMissionObjectiveSystem::GetMissionObjectiveByEntityId(EntityId id)
{
	for(TObjectives::iterator it = m_currentMissionObjectives.begin(), end = m_currentMissionObjectives.end(); it != end; ++it)
	{
		if((*it).GetTrackedEntity() == id)
			return &(*it);
	}

	return NULL;
}

void CHUDMissionObjectiveSystem::Serialize(TSerialize ser)
{
	u32 iNumObjectives = m_currentMissionObjectives.size();
	ser.Value("iNumObjectives", iNumObjectives);

	if(ser.IsReading() && m_currentMissionObjectives.empty())
	{
		LoadLevelObjectives(g_pGame && g_pGame->GetIGameFramework() ? g_pGame->GetIGameFramework()->GetLevelName() : NULL);
	}

	assert(iNumObjectives == m_currentMissionObjectives.size());
	for(size_t i = 0; i < iNumObjectives; ++i)
	{
		ser.BeginGroup("Obj");
		m_currentMissionObjectives[i].Serialize(ser);
		ser.EndGroup();
	}
}

void CHUDMissionObjectiveSystem::PostSerialize()
{
	// Visual hud data is cleared, and all the objectives have been serialized, so now make sure hud knows about them again
	TObjectives::iterator itObjective = m_currentMissionObjectives.begin();
	TObjectives::iterator itObjectiveEnd = m_currentMissionObjectives.end();
	for (; itObjective != itObjectiveEnd; ++itObjective)
	{
		CHUDMissionObjective &objective = *itObjective;
		if (objective.GetStatus() != CHUDMissionObjective::DEACTIVATED)
		{
			objective.NotifyStatusChange();
		}
	}
}

bool CHUDMissionObjectiveSystem::StartObjectiveAnalysis(tukk szKey, const EObjectiveAnalysisMode analysisMode)
{
	const SObjectiveAnalysisData* pObjectiveAnalysisData = GetObjectiveAnalysisData(szKey);
	if (pObjectiveAnalysisData)
	{
		if (m_currentAnalysis.empty())
		{
			m_currentAnalysis = szKey;
			SHUDEvent hudEvent(eHUDEvent_OnObjectiveAnalysis);
			hudEvent.AddData(SHUDEventData(eOAE_Start));
			hudEvent.AddData(SHUDEventData(szKey));
			hudEvent.AddData(SHUDEventData((ukk )pObjectiveAnalysisData));
			hudEvent.AddData(SHUDEventData((i32)analysisMode));
			CHUDEventDispatcher::CallEvent(hudEvent);
			return true;
		}
		else
		{
			GameWarning("CHUDMissionObjectiveSystem::StartObjectiveAnalysis: Can't start analysis: %s, when analysis: %s is already in progress", m_currentAnalysis.c_str(), szKey);
			return false;
		}
	}
	else
	{
		GameWarning("CHUDMissionObjectiveSystem::StartObjectiveAnalysis: Failed to find objective analysis data: %s", szKey);
		return false;
	}
}

void CHUDMissionObjectiveSystem::StopObjectiveAnalysis(const bool bSignalResultingObjective)
{
	if (!m_currentAnalysis.empty())
	{
		CHUDMissionObjective* pMissionObjective = GetMissionObjective(m_currentAnalysis); // May not be one if only showing UI screen anims without actual objectives for world anims
		if (pMissionObjective)
		{
			pMissionObjective->SetStatus(CHUDMissionObjective::ACTIVATED);
		}

		SHUDEvent hudEvent(eHUDEvent_OnObjectiveAnalysis);
		hudEvent.AddData(SHUDEventData(eOAE_Stop));
		CHUDEventDispatcher::CallEvent(hudEvent);

		m_currentAnalysis = "";
	}
}

const SObjectiveAnalysisData* CHUDMissionObjectiveSystem::GetObjectiveAnalysisData(tukk szKey) const
{
	const TObjectiveAnalysisData::const_iterator iter = m_objectiveAnalysisData.find(szKey);
	if (iter != m_objectiveAnalysisData.end())
	{
		return &iter->second;
	}

	return NULL;
}

void CHUDMissionObjectiveSystem::GetMemoryUsage(IDrxSizer * s) const
{
	s->Add(*this);
	s->AddContainer(m_currentMissionObjectives);	
}

void CHUDMissionObjective::SetTrackedEntity(EntityId entityId)
{
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(entityId);
	if (pEntity)
	{
		m_trackedEntity = pEntity->GetName();
		m_trackedEntityId = entityId;
	}
}

void CHUDMissionObjective::SetTrackedEntity(tukk szEntityName)
{
	m_trackedEntity = szEntityName;
	m_trackedEntityId = 0;

	if (!m_trackedEntity.empty())
	{
		IEntity *pEntity = gEnv->pEntitySystem->FindEntityByName(m_trackedEntity.c_str());

		if (pEntity)
		{
			m_trackedEntityId = pEntity->GetId();
		}
	}
}

void CHUDMissionObjective::ClearTrackedEntity()
{
	m_trackedEntity.clear();
	m_trackedEntityId = 0;
}

void CHUDMissionObjective::Serialize(TSerialize ser)
{
	//ser.Value("m_shortMessage", m_shortMessage);
	//ser.Value("m_screenMessage", m_screenMessage);
	//ser.Value("m_id", m_id);

	ser.Value("m_trackedEntity", m_trackedEntity);
	ser.EnumValue("m_eStatus", m_eStatus, FIRST, LAST);
	ser.Value("m_silent", m_silent);
	ser.Value("m_secondary", m_secondary);

	if(ser.IsReading())
	{
		if(m_eStatus != DEACTIVATED)
		{
			m_lastTimeChanged = gEnv->pTimer->GetFrameStartTime().GetSeconds();
		}

		m_pMOS = g_pGame->GetMOSystem();

		SetTrackedEntity(m_trackedEntity.c_str());
	}
}
