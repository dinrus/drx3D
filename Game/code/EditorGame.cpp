// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$

 -------------------------------------------------------------------------
  История:
  - 30:8:2004   11:19 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/EditorGame.h>
#include <drx3D/Game/GameStartup.h>
#include <drx3D/Game/IActionMapUpr.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Network/INetwork.h>
#include <drx3D/AI/IAgent.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Game/IMovementController.h>
#include <drx3D/Act/IItemSystem.h>
//#include <drx3D/Entity/IEntityPoolUpr.h>
#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Game/EquipmentSystemInterface.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/GameParameters.h>
#include <drx3D/Game/ItemResourceCache.h>
#include <drx3D/AI/ICommunicationUpr.h>
#include <drx3D/Game/Turret/Turret/Turret.h>
#include <drx3D/Game/Environment/DoorPanel.h>

#include <drx3D/Act/IForceFeedbackSystem.h>

#include <drx3D/Game/Environment/LedgeUpr.h>

#define EDITOR_SERVER_PORT 0xed17

ICVar * CEditorGame::s_pEditorGameMode;
CEditorGame * CEditorGame::s_pEditorGame = NULL;
struct IGameStartup;

//------------------------------------------------------------------------
CEditorGame::CEditorGame()
{
	InitMembers(nullptr);
}

CEditorGame::CEditorGame(tukk binariesDir)
{
	InitMembers(binariesDir);
}

void CEditorGame::InitMembers(tukk binariesDir)
{
	m_pGame = 0;
	m_pGameStartup = 0;
	m_pEquipmentSystemInterface = 0;
	m_bEnabled = false;
	m_bGameMode = false;
	m_bPlayer = false;
	m_bUsingMultiplayerGameRules = false;
	s_pEditorGame = this;
	s_pEditorGameMode = 0;
	m_binariesDir = binariesDir;
	m_pGTE = NULL;
}

void CEditorGame::ResetClient(IConsoleCmdArgs*)
{
	g_pGame->ReloadPlayerParamFiles();

	bool value = s_pEditorGame->m_bPlayer;
	s_pEditorGame->EnablePlayer(false);

	IEntityClass *pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Player");
	if (pClass)	pClass->LoadScript(true);

	if (value)
	{
		s_pEditorGame->ConfigureNetContext(true);
		tukk pGameRulesName = GetGameRulesName();
		s_pEditorGame->m_pGame->GetIGameFramework()->GetIGameRulesSystem()->CreateGameRules(pGameRulesName);
	}
	s_pEditorGame->EnablePlayer(value);
	s_pEditorGame->HidePlayer(true);

	CActor *pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pActor)
	{
		SEntityEvent event;
		event.event = ENTITY_EVENT_RELOAD_SCRIPT;
		pActor->GetEntity()->SendEvent(event);
	}
}

//------------------------------------------------------------------------
void CEditorGame::ToggleMultiplayerGameRules()
{
	m_bUsingMultiplayerGameRules = !m_bUsingMultiplayerGameRules;

	bool value = s_pEditorGame->m_bPlayer;
	s_pEditorGame->EnablePlayer(false);

	s_pEditorGame->ConfigureNetContext(true);

	IEntityClass *pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Player");
	if (pClass)
	{
		pClass->LoadScript(true);
	}

	tukk pGameRulesName = GetGameRulesName();
	s_pEditorGame->m_pGame->GetIGameFramework()->GetIGameRulesSystem()->CreateGameRules(pGameRulesName);

	s_pEditorGame->EnablePlayer(value);
	s_pEditorGame->HidePlayer(true);

	gEnv->pConsole->ShowConsole(false);
}

//------------------------------------------------------------------------
CEditorGame::~CEditorGame()
{
	SAFE_DELETE(m_pEquipmentSystemInterface);
	s_pEditorGame = NULL;

	SAFE_RELEASE(s_pEditorGameMode);
}

//------------------------------------------------------------------------
void CEditorGame::OnChangeEditorMode( ICVar * pVar )
{
	assert( pVar == s_pEditorGameMode );
	if (s_pEditorGame)
	{
		s_pEditorGame->SetGameMode( s_pEditorGame->m_bGameMode );
	}
}

//------------------------------------------------------------------------

extern "C"
{
	GAME_API IGameStartup *CreateGameStartup();
};

bool CEditorGame::Init(ISystem *pSystem,IGameToEditorInterface *pGameToEditorInterface)
{
	assert(pSystem);

	SSysInitParams startupParams;
	FillSystemInitParams(startupParams, pSystem);

	m_pGameStartup = CreateGameStartup();

	m_pGame = m_pGameStartup->Init(startupParams);

	if (!m_pGame)
	{
		return false;
	}

	InitUIEnums(pGameToEditorInterface);

	m_pGame->GetIGameFramework()->InitEditor(pGameToEditorInterface);

	m_pEquipmentSystemInterface = new CEquipmentSystemInterface(this, pGameToEditorInterface);

	gEnv->bServer = true;
	gEnv->bMultiplayer = false;

#if DRX_PLATFORM_DESKTOP
	gEnv->SetIsClient(true);
#endif

	m_bUsingMultiplayerGameRules = (g_pGameCVars->g_multiplayerDefault != 0);

	if (IConsole* pConsole = gEnv->pConsole)
		s_pEditorGameMode = REGISTER_INT_CB("net_gamemode", 0, VF_NULL, "Should editor connect a new client?", &OnChangeEditorMode);

	SetGameMode(false);

	REGISTER_COMMAND( "net_reseteditorclient", ResetClient, VF_NULL, "Resets player and gamerules!" );

	ConfigureNetContext(true);

	g_pGame->OnEditorGameInitComplete();

	return true;
}

//------------------------------------------------------------------------
i32 CEditorGame::Update(bool haveFocus, u32 updateFlags)
{
	return m_pGameStartup->Update(haveFocus, updateFlags);
}

//------------------------------------------------------------------------
void CEditorGame::Shutdown()
{
	gEnv->pConsole->RemoveCommand("net_reseteditorclient");

	EnablePlayer(false);
	SetGameMode(false);
	m_pGameStartup->Shutdown();
}

//------------------------------------------------------------------------
void CEditorGame::EnablePlayer(bool bPlayer)
{
	bool spawnPlayer = false;
	if (m_bPlayer != bPlayer)
	{
		spawnPlayer = m_bPlayer = bPlayer;
	}
	if (!SetGameMode( m_bGameMode ))
	{
		GameWarning("Failed setting game mode");
	}
	else if (m_bEnabled && spawnPlayer)
	{
		if (!m_pGame->GetIGameFramework()->BlockingSpawnPlayer())
			GameWarning("Failed spawning player");
	}
}

//------------------------------------------------------------------------
bool CEditorGame::SetGameMode(bool bGameMode)
{
	m_bGameMode = bGameMode;
	bool on = bGameMode;
	if (s_pEditorGameMode->GetIVal() == 0)
		on = m_bPlayer;
	bool ok = ConfigureNetContext( on );
	if (ok)
	{
		if(gEnv->IsEditor())
		{
			m_pGame->EditorResetGame(bGameMode);
		}

		IGameFramework * pGameFramework = m_pGame->GetIGameFramework();

		pGameFramework->OnEditorSetGameMode(bGameMode);

		CActor *pActor = static_cast<CActor*>(m_pGame->GetIGameFramework()->GetClientActor());
		if (pActor)
		{
			if (bGameMode)
			{
				// Revive actor in its current location (it will be moved to the editor viewpoint location later)
				const Vec3 pos = pActor->GetEntity()->GetWorldPos();
				const Quat rot = pActor->GetEntity()->GetWorldRotation();
				i32k teamId = g_pGame->GetGameRules()->GetTeam(pActor->GetEntityId());
				pActor->NetReviveAt(pos, rot, teamId, MP_MODEL_INDEX_DEFAULT);
			}
			else
			{
				pActor->Reset(true);
			}
		}
	}
	else
	{
		GameWarning("Failed configuring net context");
	}
	return ok;
}

//------------------------------------------------------------------------
IEntity * CEditorGame::GetPlayer()
{
	IGameFramework * pGameFramework = m_pGame->GetIGameFramework();

	if(!m_pGame)
		return 0;

	IActor * pActor = pGameFramework->GetClientActor();
	return pActor? pActor->GetEntity() : NULL;
}

//------------------------------------------------------------------------
void CEditorGame::SetPlayerPosAng(Vec3 pos,Vec3 viewDir)
{
	IActor * pClActor = m_pGame->GetIGameFramework()->GetClientActor();

	if (pClActor /*&& m_bGameMode*/)
	{
		// pos coming from editor is a camera position, we must convert it into the player position by subtructing eye height.
		IEntity *myPlayer = pClActor->GetEntity();
		if (myPlayer)
		{
			pe_player_dimensions dim;
			dim.heightEye = 0;
			if (myPlayer->GetPhysics())
			{
				myPlayer->GetPhysics()->GetParams( &dim );
				pos.z = pos.z - dim.heightEye;
			}
		}

		pClActor->GetEntity()->SetPosRotScale( pos,Quat::CreateRotationVDir(viewDir),Vec3(1,1,1),ENTITY_XFORM_EDITOR|ENTITY_XFORM_POS|ENTITY_XFORM_ROT|ENTITY_XFORM_SCL);
	}
}

//------------------------------------------------------------------------
void CEditorGame::HidePlayer(bool bHide)
{
	IEntity * pEntity = GetPlayer();
	if (pEntity)
		pEntity->Hide( bHide );
}

//------------------------------------------------------------------------
bool CEditorGame::ConfigureNetContext( bool on )
{
	bool ok = false;

	IGameFramework * pGameFramework = m_pGame->GetIGameFramework();

	if (on == m_bEnabled)
	{
		ok = true;
	}
	else if (on)
	{
		SGameContextParams ctx;

		SGameStartParams gameParams;
		gameParams.flags = eGSF_Server
			| eGSF_NoSpawnPlayer
			| eGSF_Client
			| eGSF_NoLevelLoading
			| eGSF_BlockingClientConnect
			| eGSF_NoGameRules
			| eGSF_NoQueries;

		if (!m_bUsingMultiplayerGameRules)
		{
			gameParams.flags |= eGSF_LocalOnly;
		}

		if (m_bUsingMultiplayerGameRules)
		{
			gameParams.flags |= eGSF_ImmersiveMultiplayer;
		}

		gameParams.connectionString = "";
		gameParams.hostname = "localhost";
		gameParams.port = EDITOR_SERVER_PORT;
		gameParams.pContextParams = &ctx;
		gameParams.maxPlayers = 1;

		if (pGameFramework->StartGameContext( &gameParams ))
			ok = true;
	}
	else
	{
		pGameFramework->EndGameContext();
		gEnv->pNetwork->SyncWithGame(eNGS_Shutdown);
		ok = true;
	}

	m_bEnabled = on && ok;
	return ok;
}

//------------------------------------------------------------------------
void CEditorGame::OnBeforeLevelLoad()
{
	//This must be called before ILevelSystem::OnLoadingStart (good place to free resources in editor, before next level loads)
	g_pGame->OnBeforeEditorLevelLoad();

	EnablePlayer(false);
	ConfigureNetContext(true);
	tukk pGameRulesName = GetGameRulesName();
	m_pGame->GetIGameFramework()->GetIGameRulesSystem()->CreateGameRules(pGameRulesName);
	m_pGame->GetIGameFramework()->GetILevelSystem()->OnLoadingStart(0);
}

//------------------------------------------------------------------------
void CEditorGame::OnAfterLevelInit(tukk levelName, tukk levelFolder)
{
	InitEntityArchetypeEnums(m_pGTE, levelFolder, levelName);
	EnablePlayer(true);
}

//------------------------------------------------------------------------
void CEditorGame::OnAfterLevelLoad(tukk levelName, tukk levelFolder)
{
	m_pGame->GetIGameFramework()->GetILevelSystem()->Rescan("levels", ILevelSystem::TAG_MAIN);

	ILevelInfo* pLevel = m_pGame->GetIGameFramework()->GetILevelSystem()->SetEditorLoadedLevel(levelName);
	m_pGame->GetIGameFramework()->GetILevelSystem()->OnLoadingComplete(pLevel);
}

//------------------------------------------------------------------------
void CEditorGame::OnCloseLevel()
{
	IItemSystem* pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();
	if (pItemSystem)
	{
		pItemSystem->ClearGeometryCache();
		pItemSystem->ClearSoundCache();
		pItemSystem->Reset();
	}
	g_pGame->GetGameSharedParametersStorage()->GetItemResourceCache().FlushCaches();
}

//------------------------------------------------------------------------
IFlowSystem * CEditorGame::GetIFlowSystem()
{
	return m_pGame->GetIGameFramework()->GetIFlowSystem();
}

//------------------------------------------------------------------------
IGameTokenSystem* CEditorGame::GetIGameTokenSystem()
{
	return m_pGame->GetIGameFramework()->GetIGameTokenSystem();
}

void CEditorGame::RegisterTelemetryTimelineRenderers(Telemetry::ITelemetryRepository* pRepository)
{
	// pRepository->RegisterTimelineRenderer("shot", RenderShot);
}

void CEditorGame::OnDisplayRenderUpdated( bool displayHelpers )
{
	CLedgeUprEdit* pLedgeUprEdit = g_pGame->GetLedgeUpr()->GetEditorUpr();
	if(pLedgeUprEdit != NULL)
	{
		pLedgeUprEdit->OnDisplayHelpersChanged( displayHelpers );
	}

	g_pGame->OnEditorDisplayRenderUpdated( displayHelpers );
}

//------------------------------------------------------------------------
void CEditorGame::InitUIEnums(IGameToEditorInterface* pGTE)
{
	m_pGTE = pGTE;

	InitGlobalFileEnums(pGTE);
	InitEntityClassesEnums(pGTE);
	InitLevelTypesEnums(pGTE);
	InitEntityArchetypeEnums(pGTE);
	InitForceFeedbackEnums(pGTE);
	InitActionInputEnums(pGTE);
	InitReadabilityEnums(pGTE);
	InitLedgeTypeEnums(pGTE);
	InitSmartMineTypeEnums(pGTE);
	InitDamageTypeEnums(pGTE);
	InitDialogBuffersEnum(pGTE);
	InitTurretEnum(pGTE);
	InitDoorPanelEnum(pGTE);
	InitModularBehaviorTreeEnum(pGTE);
}

void CEditorGame::InitGlobalFileEnums(IGameToEditorInterface* pGTE)
{
	// Read in enums stored offline XML. Format is
	// <GlobalEnums>
	//   <EnumName>
	//     <entry enum="someName=someValue" />  <!-- displayed name != value -->
	// 	   <entry enum="someNameValue" />       <!-- displayed name == value -->
	//   </EnumName>
	// </GlobalEnums>
	//
	XmlNodeRef rootNode = GetISystem()->LoadXmlFromFile("Libs/UI/GlobalEnums.xml");
	if (!rootNode || !rootNode->getTag() || stricmp(rootNode->getTag(), "GlobalEnums") != 0)
	{
		// GameWarning("CEditorGame::InitUIEnums: File 'Libs/UI/GlobalEnums.xml' is not a GlobalEnums file");
		return;
	}
	for (i32 i = 0; i < rootNode->getChildCount(); ++i)
	{
		XmlNodeRef enumNameNode = rootNode->getChild(i);
		tukk enumId = enumNameNode->getTag();
		if (enumId == 0 || *enumId=='\0')
			continue;
		i32 maxChilds = enumNameNode->getChildCount();
		if (maxChilds > 0)
		{
			// allocate enough space to hold all strings
			tukk* nameValueStrings = new tukk [maxChilds];
			i32 curEntryIndex = 0;
			for (i32 j = 0; j < maxChilds; ++j)
			{
				XmlNodeRef enumNode = enumNameNode->getChild(j);
				tukk nameValue = enumNode->getAttr("enum");
				if (nameValue != 0 && *nameValue!='\0')
				{
					// put in the nameValue pair
					nameValueStrings[curEntryIndex++] = nameValue;
				}
			}
			// if we found some entries inform CUIDataBase about it
			if (curEntryIndex > 0)
				pGTE->SetUIEnums(enumId, nameValueStrings, curEntryIndex);

			// be nice and free our array
			delete[] nameValueStrings;
		}
	}
}

void CEditorGame::InitEntityClassesEnums(IGameToEditorInterface* pGTE)
{
	// init ActionFilter enums
	gEnv->pEntitySystem->GetClassRegistry()->IteratorMoveFirst();
	IEntityClass* entClass = gEnv->pEntitySystem->GetClassRegistry()->IteratorNext();
	tukk* allEntities = new tukk [gEnv->pEntitySystem->GetClassRegistry()->GetClassCount()+2];

	allEntities[0] = "AllClasses";
	allEntities[1] = "CustomClasses";

	u32 numEnts = 2;
	while (entClass)
	{
		tukk classname = entClass->GetName();
		allEntities[numEnts++] = classname;
		entClass = gEnv->pEntitySystem->GetClassRegistry()->IteratorNext();
	}

	pGTE->SetUIEnums("entity_classes", allEntities, numEnts);
	delete[] allEntities;
}

void CEditorGame::InitLevelTypesEnums(IGameToEditorInterface* pGTE)
{
	DynArray<string>* levelTypes;
	levelTypes = g_pGame->GetIGameFramework()->GetILevelSystem()->GetLevelTypeList();

	tukk* allLevelTypes = new tukk [levelTypes->size()];
	for (i32 i = 0; i < levelTypes->size(); i++)
	{
		PREFAST_ASSUME(i > 0 && i < levelTypes->size());
		allLevelTypes[i] = (*levelTypes)[i];
	}

	pGTE->SetUIEnums("level_types", allLevelTypes, levelTypes->size());
	delete[] allLevelTypes;
}

void CEditorGame::InitDialogBuffersEnum( IGameToEditorInterface* pGTE )
{
	static tukk BUFFERS_FILENAME = "Libs/FlowNodes/DialogFlowNodeBuffers.xml";

	XmlNodeRef xmlNodeRoot = gEnv->pSystem->LoadXmlFromFile( BUFFERS_FILENAME );
	if (xmlNodeRoot==NULL)
	{
		DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CEditorGame::InitDialogBuffersEnum() - Failed to load '%s'. flownode dialog buffers drop down list will be empty.", BUFFERS_FILENAME);
		return;
	}

	u32 count = xmlNodeRoot->getChildCount();

	tukk* bufferNames = new tukk [count];
	bool bOk = true;

	for (u32 i=0; i<count; ++i)
	{
		XmlNodeRef xmlNode = xmlNodeRoot->getChild( i );
		bufferNames[i] = xmlNode->getAttr("name");
		if (!bufferNames[i])
		{
			DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CEditorGame::InitDialogBuffersEnum() - file: '%s' child: %d is missing the 'name' field. flownode dialog buffers drop down list will be empty", BUFFERS_FILENAME, i);
			bOk = false;
		}
	}
	if (bOk)
		pGTE->SetUIEnums("dialogBuffers", bufferNames, count);
	delete[] bufferNames;
}

void GetArchetypesFromLib(XmlNodeRef root, string& name, std::vector<string>* archetypeNames)
{
	if (!root)
		return;

	i32k iChildCount = root->getChildCount();
	for (i32 iChild = 0; iChild < iChildCount; ++iChild)
	{
		XmlNodeRef pChild = root->getChild(iChild);
		if (!pChild || stricmp(pChild->getTag(), "EntityPrototype"))
			continue;

		XmlString sChildName;
		pChild->getAttr("Name", sChildName);

		string sFullName;
		sFullName.Format("%s.%s", name.c_str(), sChildName.c_str());
		archetypeNames->push_back(sFullName);
	}
}

void GetArchetypesFromLevelLib(XmlNodeRef root, std::vector<string>* archetypeNames)
{
	if (!root)
		return;

	string sRootName;
	sRootName = root->getTag();

	XmlNodeRef pLevelNode = root->findChild("EntityPrototypesLibs");

	if (!pLevelNode)
		return;

	XmlNodeRef pLevelLibrary = pLevelNode->findChild("LevelLibrary");

	if (!pLevelLibrary)
		return;

	GetArchetypesFromLib(pLevelLibrary, sRootName, archetypeNames);
}

void CEditorGame::InitEntityArchetypeEnums(IGameToEditorInterface* pGTE, tukk levelFolder /*= NULL*/, tukk levelName /*= NULL*/)
{
	DRX_ASSERT(pGTE);

	// Look in all the archetype files
	IDrxPak* pDrxPak = gEnv->pDrxPak;
	DRX_ASSERT(pDrxPak);

	std::vector<string> vecArchetypeNames;

	if (levelFolder && levelName)
	{
		string levelPath = string(levelFolder) + "/" + string(levelName) + ".cry";

		if (pDrxPak && pDrxPak->OpenPack(levelPath))
		{
			string editorXML = string(levelFolder) + "/Level.editor_xml";
			XmlNodeRef pRoot = gEnv->pSystem->LoadXmlFromFile(editorXML);

			if (pRoot)
				GetArchetypesFromLevelLib(pRoot, &vecArchetypeNames);

			pDrxPak->ClosePack(levelPath);
		}
	}

	_finddata_t fd;
	string sSearchPath = PathUtil::Make("Libs\\EntityArchetypes", "*", "xml");
	intptr_t handle = pDrxPak->FindFirst(sSearchPath, &fd);
	if (handle >= 0)
	{
		do
		{
			string sFilePath = PathUtil::Make("Libs\\EntityArchetypes", fd.name, "xml");

			XmlNodeRef pRoot = gEnv->pSystem->LoadXmlFromFile(sFilePath.c_str());
			if (!pRoot || stricmp(pRoot->getTag(), "EntityPrototypeLibrary"))
				continue;

				XmlString sRootName;
				pRoot->getAttr("Name", sRootName);

				GetArchetypesFromLib(pRoot, sRootName, &vecArchetypeNames);
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);
		pDrxPak->FindClose(handle);
	}

	if (!vecArchetypeNames.empty())
	{
		size_t numFilters = 0;
		i32k allArchetypeCount = vecArchetypeNames.size()+1;
		tukk* allArchetypeNames = new tukk [allArchetypeCount];
		allArchetypeNames[numFilters++] = ""; // Blank entry at top
		std::vector<string>::const_iterator iter = vecArchetypeNames.begin();
		std::vector<string>::const_iterator iterEnd = vecArchetypeNames.end();
		while (iter != iterEnd)
		{
			assert(numFilters > 0 && numFilters < allArchetypeCount);
			PREFAST_ASSUME(numFilters > 0 && numFilters < allArchetypeCount);
			allArchetypeNames[numFilters++] = iter->c_str();
			++iter;
		}
		pGTE->SetUIEnums("entity_archetypes", allArchetypeNames, numFilters);
		delete[] allArchetypeNames;
	}
}

IEquipmentSystemInterface* CEditorGame::GetIEquipmentSystemInterface()
{
	return m_pEquipmentSystemInterface;
}

void CEditorGame::FillSystemInitParams(SSysInitParams &startupParams, ISystem* system)
{
	startupParams.bEditor = true;
	startupParams.pSystem = system;
	startupParams.bExecuteCommandLine=false;		// in editor we do it later - after other things are initialized
	if (m_binariesDir && m_binariesDir[0])
	{
		drx_strcpy(startupParams.szBinariesDir, m_binariesDir);
	}
}

tukk  CEditorGame::GetGameRulesName()
{
	tukk pGameRulesName = "SinglePlayer";
	if (s_pEditorGame->m_bUsingMultiplayerGameRules)
	{
		pGameRulesName = "TeamInstantAction";
	}
	return pGameRulesName;
}

void CEditorGame::InitForceFeedbackEnums( IGameToEditorInterface* pGTE )
{
	DRX_ASSERT(pGTE);

	IForceFeedbackSystem* pForceFeedback = g_pGame->GetIGameFramework()->GetIForceFeedbackSystem();
	if (pForceFeedback)
	{
		struct SEffectList : public IFFSPopulateCallBack
		{
			explicit SEffectList(i32 effectCount)
				: m_maxNumNames(effectCount)
				, m_nameCount(0)
			{
				m_allEffectNames = new tukk [effectCount];
			}

			~SEffectList()
			{
				SAFE_DELETE_ARRAY(m_allEffectNames);
			}

			//IFFSPopulateCallBack
			virtual void AddFFSEffectName( tukk const pName )
			{
				assert(m_nameCount < m_maxNumNames);

				if (m_nameCount < m_maxNumNames)
				{
					m_allEffectNames[m_nameCount] = pName;
					m_nameCount++;
				}
			}
			//~IFFSPopulateCallBack

			tukk* m_allEffectNames;
			i32 m_maxNumNames;
			i32 m_nameCount;
		};

		i32 effectCount = pForceFeedback->GetEffectNamesCount();
		if (effectCount > 0)
		{
			SEffectList effectList(effectCount);

			pForceFeedback->EnumerateEffects(&effectList);

			pGTE->SetUIEnums("forceFeedback", effectList.m_allEffectNames, effectList.m_nameCount);
		}
	}
}

void CEditorGame::InitActionInputEnums( IGameToEditorInterface* pGTE )
{
	DRX_ASSERT(pGTE);

	IActionMapUpr* pActionMapUpr = g_pGame->GetIGameFramework()->GetIActionMapUpr();
	if (pActionMapUpr)
	{
		struct SActionList : public IActionMapPopulateCallBack
		{
			explicit SActionList(i32 actionCount)
				: m_maxNumNames(actionCount)
				, m_nameCount(0)
			{
				m_allActionNames = new tukk [actionCount];
			}

			~SActionList()
			{
				SAFE_DELETE_ARRAY(m_allActionNames);
			}

			//IActionMapPopulateCallBack
			virtual void AddActionName( tukk const pName )
			{
				assert(m_nameCount < m_maxNumNames);

				if (m_nameCount < m_maxNumNames)
				{
					m_allActionNames[m_nameCount] = pName;
					m_nameCount++;
				}
			}
			//~IActionMapPopulateCallBack

			tukk* m_allActionNames;
			i32 m_maxNumNames;
			i32 m_nameCount;
		};

		i32 actionCount = pActionMapUpr->GetActionsCount();
		if (actionCount > 0)
		{
			SActionList actionList(actionCount);

			pActionMapUpr->EnumerateActions(&actionList);

			pGTE->SetUIEnums("input_actions", actionList.m_allActionNames, actionList.m_nameCount);
		}
	}
}

void CEditorGame::InitReadabilityEnums( IGameToEditorInterface* pGTE )
{
	DRX_ASSERT(pGTE);

	ICommunicationUpr* pCommunicationUpr = gEnv->pAISystem ? gEnv->pAISystem->GetCommunicationUpr() : NULL;
	if (pCommunicationUpr)
	{
		struct SCommunicationNameList
		{
			typedef std::vector<tukk > TNamesVec;
			TNamesVec m_vecNames;

			void AddName(tukk name)
			{
				m_vecNames.push_back(name);
			}

			void CreateEnum(IGameToEditorInterface *pGTEInterface, tukk enumName) const
			{
				assert(pGTEInterface);
				assert(enumName && enumName[0]);

				if (!m_vecNames.empty())
				{
					const size_t numNames = m_vecNames.size();
					tukk* pNameArray = new tukk [numNames];

					TNamesVec::const_iterator itName = m_vecNames.begin();
					TNamesVec::const_iterator itNameEnd = m_vecNames.end();
					for (size_t index = 0; itName != itNameEnd; ++itName, ++index)
					{
						assert(index >= 0 && index < numNames);
						PREFAST_ASSUME(index >= 0 && index < numNames);
						pNameArray[index] = *itName;
					}

					pGTEInterface->SetUIEnums(enumName, pNameArray, numNames);

					delete [] pNameArray;
				}
			}
		};

		u32k configCount = pCommunicationUpr->GetConfigCount();
		if (configCount > 0)
		{
			SCommunicationNameList globalNameList;

			for (u32 configIndex = 0; configIndex < configCount; ++configIndex)
			{
				CommConfigID commConfigId = pCommunicationUpr->GetConfigIDByIndex(configIndex);
				u32k communicationCount = pCommunicationUpr->GetConfigCommunicationCount(commConfigId);
				if (communicationCount > 0)
				{
					SCommunicationNameList configNameList;

					for (u32 communicationIndex = 0; communicationIndex < communicationCount; ++communicationIndex)
					{
						tukk szName = pCommunicationUpr->GetConfigCommunicationNameByIndex(commConfigId, communicationIndex);
						globalNameList.AddName(szName);
						configNameList.AddName(szName);
					}

					stack_string uiName;
					uiName.Format("communications_%s", pCommunicationUpr->GetConfigName(configIndex));
					configNameList.CreateEnum(pGTE, uiName.c_str());
				}
			}

			globalNameList.CreateEnum(pGTE, "communications");
		}
	}
}

void CEditorGame::InitLedgeTypeEnums (IGameToEditorInterface* pGTE )
{
	i32k ledgeTypeCount = 3;
	tukk ledgeTypeNames[ledgeTypeCount] =
	{
		"Normal",
		"Vault",
		"HighVault"
	};

	pGTE->SetUIEnums( "LedgeType", ledgeTypeNames, ledgeTypeCount );
}

void CEditorGame::InitSmartMineTypeEnums(IGameToEditorInterface* pGTE )
{
	i32k smartMineTypeCount = 1;
	tukk smartMineTypeNames[smartMineTypeCount] =
	{
		"Cell",
	};

	pGTE->SetUIEnums( "SmartMineType", smartMineTypeNames, smartMineTypeCount );
}

void CEditorGame::InitDamageTypeEnums(IGameToEditorInterface *pGTE)
{
	i32k damageTypeCount = CGameRules::EHitType::Unreserved;
	pGTE->SetUIEnums( "DamageType", CGameRules::s_reservedHitTypes, damageTypeCount);
}

void CEditorGame::InitTurretEnum(IGameToEditorInterface* pGTE)
{
	pGTE->SetUIEnums( "TurretState", TurretBehaviorStateNames::GetNames(), eTurretBehaviorState_Count );
}

void CEditorGame::InitDoorPanelEnum(IGameToEditorInterface* pGTE)
{
	pGTE->SetUIEnums( "DoorPanelState", DoorPanelBehaviorStateNames::GetNames(), eDoorPanelBehaviorState_Count );
}

void CEditorGame::ScanBehaviorTrees(const string& folderName, std::vector<string>& behaviorTrees)
{
	_finddata_t fd;
	intptr_t handle = 0;
	IDrxPak *pPak = gEnv->pDrxPak;

	string searchString = folderName;
	searchString += "*.xml";

	handle = pPak->FindFirst(searchString.c_str(), &fd);

	if (handle > -1)
	{
		do
		{
			if (fd.attrib & _A_HIDDEN)
				continue;

			string szFileName = fd.name;
			PathUtil::RemoveExtension(szFileName);
			behaviorTrees.push_back(szFileName);

		} while (pPak->FindNext(handle, &fd) >= 0);

		pPak->FindClose(handle);
	}
}

void CEditorGame::InitModularBehaviorTreeEnum(IGameToEditorInterface* pGTE)
{
	std::vector<string> behaviorTrees;
	// no behavior tree
	behaviorTrees.push_back("");
	ScanBehaviorTrees("Scripts/AI/BehaviorTrees/", behaviorTrees);

	size_t numTrees = 0;
	i32k behaviorTreeCount = behaviorTrees.size();
	tukk* allTrees = new tukk [behaviorTreeCount];
	std::vector<string>::const_iterator iter = behaviorTrees.begin();
	std::vector<string>::const_iterator iterEnd = behaviorTrees.end();

	while (iter != iterEnd)
	{
		assert(numTrees >= 0 && numTrees < behaviorTreeCount);
		PREFAST_ASSUME(numTrees >= 0 && numTrees < behaviorTreeCount);
		allTrees[numTrees++] = iter->c_str();
		++iter;
	}

	pGTE->SetUIEnums("ModularBehaviorTree", allTrees, numTrees);

	SAFE_DELETE_ARRAY(allTrees);
}

//////////////////////////////////////////////////////////////////////////
//
// Allows the game code to write game-specific data into the minimap xml
// file on level export. Currently used to export data to StatsTool
//
//////////////////////////////////////////////////////////////////////////
bool CEditorGame::GetAdditionalMinimapData(XmlNodeRef output)
{
	string classes = g_pGameCVars->g_telemetryEntityClassesToExport;
	if(!classes.empty())
	{
		// additional data relating to StatsTool
		XmlNodeRef statsNode = output->findChild("StatsTool");
		if(!statsNode)
		{
			statsNode = GetISystem()->CreateXmlNode("StatsTool");
			output->addChild(statsNode);
		}
		else
		{
			statsNode->removeAllChilds();
		}

		// first build a list of entity classes from the cvar
		std::vector<IEntityClass*> interestingClasses;
		i32 curPos = 0;
		string currentClass = classes.Tokenize(",",curPos);
		IEntitySystem* pES = GetISystem()->GetIEntitySystem();
		if(IEntityClassRegistry* pClassReg = pES->GetClassRegistry())
		{
			while (!currentClass.empty())
			{
				if(IEntityClass* pClass = pClassReg->FindClass(currentClass.c_str()))
				{
					interestingClasses.push_back(pClass);
				}

				currentClass = classes.Tokenize(",",curPos);
			}
		}

		// now iterate through all entities and save the ones which match the classes
		if(interestingClasses.size() > 0)
		{
			IEntityItPtr it = pES->GetEntityIterator();
			while(IEntity* pEntity = it->Next())
			{
				if(stl::find(interestingClasses, pEntity->GetClass()))
				{
					XmlNodeRef entityNode = GetISystem()->CreateXmlNode("Entity");
					statsNode->addChild(entityNode);

					entityNode->setAttr("class", pEntity->GetClass()->GetName());
					Vec3 pos = pEntity->GetWorldPos();
					entityNode->setAttr("x", pos.x);
					entityNode->setAttr("y", pos.y);
					entityNode->setAttr("z", pos.z);
				}
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//	Additional export code: create a list of entities which will
//	need to be serialized by the game - speeds up saving at runtime
//
//////////////////////////////////////////////////////////////////////////

#define SERIALIZATION_EXPORT_DEBUG 0	// set this to 1 for a lot of debug info about which entities will be saved

enum ESerializeType
{
	eST_NotSerialized				= 0x00,		// don't serialize this entity
	eST_FlowGraphContainer	= 0x01,		// entity contains a flowgraph
	eST_FlowGraph						= 0x02,		// entity is referenced in a flowgraph
	eST_Class								= 0x04,		// entity class means we always save it
	eST_TrackView						= 0x08,		// entity is referenced in a trackview sequence
	eST_Child								= 0x10,		// entity is a child of a serialized entity
	eST_Parent							= 0x20,		// entity is a parent of a serialized entity
	eST_Linked							= 0x40,		// entity is linked from/to a serialized entity
	eST_ConstraintNeighbour	= 0x80,		// entity is near a constraint entity
};

typedef std::map<EntityId, u32> TSerializationData;

bool ShouldSaveEntityClass(IEntity* pEntity)
{
	typedef std::vector<string> TClassesToNotSave;
	static TClassesToNotSave classList;
	if(classList.empty())
	{
		classList.reserve(12);
		classList.push_back("AIAnchor");
		classList.push_back("AmbientVolume");
		classList.push_back("AreaBox");
		classList.push_back("DecalPlacer");
		classList.push_back("GrabableLedge");
		classList.push_back("Light");
		classList.push_back("MissionObjective");
		classList.push_back("ParticleEffect");
		classList.push_back("RigidBodyEx");
		classList.push_back("SmartObject");
		classList.push_back("TacticalEntity");
		classList.push_back("TagPoint");
	}

	if(stl::find(classList, CONST_TEMP_STRING(pEntity->GetClass()->GetName())))
		return false;

	// default to saving for now
	return true;
}

bool ShouldSaveTrackViewEntityClass(IEntity* pEntity)
{
	if(!strcmp(pEntity->GetClass()->GetName(), "ParticleEffect"))
		return true;
	return false;
}

void MarkEntityForSerialize(TSerializationData& data, IEntity* pEntity, i32 reason, bool markParent = true)
{
	assert(pEntity);
	if(!pEntity)
		return;

	// only need to save unremoveable entities.
	//	(dynamic entities, including the player, are added to the list at runtime)
	u32 flags = pEntity->GetFlags();
	if(!(flags & ENTITY_FLAG_UNREMOVABLE) || (flags & ENTITY_FLAG_LOCAL_PLAYER))
	{
		return;
	}

	// skip if no-save flag is set on the entity itself, or if the entity is due to be pooled
	if(!gEnv->pEntitySystem->ShouldSerializedEntity(pEntity) || gEnv->pEntitySystem->GetIEntityPoolUpr()->IsEntityBookmarked(pEntity->GetId()))
	{
		return;
	}

	// first mark the entity itself
	data[pEntity->GetId()] |= reason;

	// then the parent
	if(markParent && pEntity->GetParent())
	{
		data[pEntity->GetParent()->GetId()] |= (reason | eST_Parent);
	}

	// repeat for all children
	i32 count = pEntity->GetChildCount();
	for(i32 i=0; i<count; ++i)
	{
		IEntity* pChild = pEntity->GetChild(i);
		assert(pChild);

		MarkEntityForSerialize(data, pChild, (reason | eST_Child), false);
	}
}

void CEditorGame::OnSaveLevel()
{
	ILevelInfo* pLevelInfo = m_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel();

	if (pLevelInfo)
	{
		string levelPath = PathUtil::GetGameFolder() + "/" + pLevelInfo->GetPath();
		InitEntityArchetypeEnums(m_pGTE, levelPath, PathUtil::GetFile(pLevelInfo->GetName()));
	}
}

bool CEditorGame::BuildEntitySerializationList(XmlNodeRef output)
{
	if(!output)
		return false;

	typedef std::vector<IFlowGraph*> TFGVector;

	TSerializationData entityData;	// all entities
	TFGVector flowGraphs;						// all flowgraphs

	// build the all-entity list, and keep a record of those entities
	//	which have a flowgraph attached
	IEntityIt* pIt = gEnv->pEntitySystem->GetEntityIterator();
	while(IEntity* pEntity = pIt->Next())
	{
		IEntityFlowGraphProxy* pFGProxy = static_cast<IEntityFlowGraphProxy*>(pEntity->GetProxy(ENTITY_PROXY_FLOWGRAPH));
		if(pFGProxy)
		{
			flowGraphs.push_back(pFGProxy->GetFlowGraph());
			MarkEntityForSerialize(entityData, pEntity, eST_FlowGraphContainer);
		}

		if (!stricmp(pEntity->GetClass()->GetName(), "Constraint"))
		{
			MarkEntityForSerialize(entityData, pEntity, eST_Class);

			float constraintRadius = -FLT_MAX;

			if (IScriptTable* entityTable = pEntity->GetScriptTable())
			{
				SmartScriptTable props;
				if (entityTable->GetValue("Properties", props))
					props->GetValue("radius", constraintRadius);
			}

			if (constraintRadius > 0.0f)
			{
				const Vec3 boxMin = pEntity->GetWorldPos() - Vec3(constraintRadius + 0.05f);
				const Vec3 boxMax = pEntity->GetWorldPos() + Vec3(constraintRadius + 0.05f);

				IPhysicalEntity** nearbyEntities = 0;

				if (size_t entityCount = gEnv->pPhysicalWorld->GetEntitiesInBox(boxMin, boxMax, nearbyEntities, ent_all))
				{
					for (size_t i = 0; i < entityCount; ++i)
					{
						if (IEntity* nearbyEntity = gEnv->pEntitySystem->GetEntityFromPhysics(nearbyEntities[i]))
							MarkEntityForSerialize(entityData, nearbyEntity, eST_ConstraintNeighbour);
					}
				}
			}
			else
			{
				DrxLogAlways("Failed to find a value radius property for constraint '%s'. "
					"Serialization for this constraint might not work.", pEntity->GetName());
			}
		}
		else if(ShouldSaveEntityClass(pEntity)) // some classes should always be saved
			MarkEntityForSerialize(entityData, pEntity, eST_Class);
	}

	// for each FG, mark all entities referenced as needing saving
	for(TFGVector::const_iterator it = flowGraphs.begin(); it != flowGraphs.end(); ++it)
	{
		if(!*it)
			continue;

		IFlowNodeIteratorPtr nodeIt = (*it)->CreateNodeIterator();
		TFlowNodeId nodeId = 0;
		while(IFlowNodeData* pNode = nodeIt->Next(nodeId))
		{
			EntityId nodeEntity = (*it)->GetEntityId(nodeId);
			if(nodeEntity != 0)
			{
				IEntity* pEntity = gEnv->pEntitySystem->GetEntity(nodeEntity);
				MarkEntityForSerialize(entityData, pEntity, eST_FlowGraph);
			}
		}
	}


	// now do the same for entities moved by trackview
	for(i32 i=0; i<gEnv->pMovieSystem->GetNumSequences(); ++i)
	{
		IAnimSequence* pSeq = gEnv->pMovieSystem->GetSequence(i);
		i32 nodeCount = pSeq->GetNodeCount();
		for(i32 j=0; j<nodeCount; ++j)
		{
			IAnimNode* pNode = pSeq->GetNode(j);
			IAnimEntityNode* pEntityNode = pNode ? pNode->QueryEntityNodeInterface() : nullptr;
			if(pEntityNode)
			{
				IEntity* pEntity = pEntityNode->GetEntity();
				if(pEntity && ShouldSaveTrackViewEntityClass(pEntity))
				{
					MarkEntityForSerialize(entityData, pEntity, eST_TrackView);
				}

				if(EntityGUID* pGuid = pEntityNode->GetEntityGuid())
				{
					EntityId id = gEnv->pEntitySystem->FindEntityByGuid(*pGuid);
					if(id != 0)
					{
						IEntity* pEntity2 = gEnv->pEntitySystem->GetEntity(id);
						MarkEntityForSerialize(entityData, pEntity2, eST_TrackView);
					}
				}
			}
		}
	}

	// now check entity links: any entity linked to or from a serialized
	//	entity must also be serialized
	for(TSerializationData::iterator it = entityData.begin(), end = entityData.end(); it != end; ++it)
	{
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(it->first);
		assert(pEntity);

		IEntityLink* pLink = pEntity->GetEntityLinks();
		while(pLink)
		{
			IEntity* pLinkedEntity = gEnv->pEntitySystem->GetEntity(pLink->entityId);
			assert(pLinkedEntity);

			MarkEntityForSerialize(entityData, pLinkedEntity, eST_Linked);

			pLink = pLink->next;
		}
	}

	// output the final file, plus debug info

#if SERIALIZATION_EXPORT_DEBUG
	i32 saveCount = 0;
	i32 totalCount = 0;

	i32 fgCount = 0;
	i32 fgRefCount = 0;
	i32 classCount = 0;
	i32 tvCount = 0;
	i32 childCount = 0;
	i32 parentCount = 0;
	i32 linkCount = 0;

	i32 fgUnique = 0;
	i32 fgRefUnique = 0;
	i32 classUnique = 0;
	i32 tvUnique = 0;
	i32 linkUnique = 0;

	typedef std::map<string, i32> TClassSaveInfo;
	TClassSaveInfo classesNotSaved;
	TClassSaveInfo classesSaved;
#endif

	output->setTag("EntitySerialization");
	for(TSerializationData::const_iterator it = entityData.begin(); it != entityData.end(); ++it)
	{
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(it->first);

#if SERIALIZATION_EXPORT_DEBUG
		++totalCount;
		string reasons = "Saving: ";
#endif

		if(it->second != eST_NotSerialized)
		{
			XmlNodeRef child = output->createNode("Entity");
			child->setAttr("id", it->first);
			//child->setAttr("class", pEntity->GetClass()->GetName());	// debug check
			//child->setAttr("name", pEntity->GetName());								// debug check
			output->addChild(child);

#if SERIALIZATION_EXPORT_DEBUG
			classesSaved[pEntity->GetClass()->GetName()]++;

			++saveCount;
			if(it->second & eST_FlowGraphContainer)
			{
				reasons += "FG Container; ";
				++fgCount;
			}
			if(it->second & eST_FlowGraph)
			{
				reasons += "FG reference; ";
				++fgRefCount;
			}
			if(it->second & eST_Class)
			{
				reasons += "Class; ";
				++classCount;
			}
			if(it->second & eST_TrackView)
			{
				reasons += "Trackview; ";
				++tvCount;
			}
			if(it->second & eST_Child)
			{
				reasons += "Child; ";
				++childCount;
			}
			if(it->second & eST_Parent)
			{
				reasons += "Parent; ";
				++parentCount;
			}
			if(it->second & eST_Linked)
			{
				reasons += "Linked; ";
				++linkCount;
			}

			// unique counts (things added as a result of only one condition)
			switch(it->second)
			{
			case eST_FlowGraph:
				++fgRefUnique;
				break;
			case eST_FlowGraphContainer:
				++fgUnique;
				break;
			case eST_Class:
				++classUnique;
				break;
			case eST_TrackView:
				++tvUnique;
				break;
			case eST_Linked:
				++linkUnique;
				break;
			default:
				break;
			}
		}
		else
		{
			reasons = "Not saving";
			classesNotSaved[pEntity->GetClass()->GetName()]++;
		}

		DrxLogAlways("Entity %s (%d, class %s) : %s", pEntity->GetName(), it->first, pEntity->GetClass()->GetName(), reasons.c_str());
#endif
		}
	}

#if SERIALIZATION_EXPORT_DEBUG
	DrxLogAlways("========================");
	DrxLogAlways("Total %d entities, of which %d will be serialized", totalCount, saveCount);
	DrxLogAlways("FG Container count: %d (%d unique)", fgCount, fgUnique);
	DrxLogAlways("FG Reference count: %d (%d unique)", fgRefCount, fgRefUnique);
	DrxLogAlways("Class count: %d (%d unique)", classCount, classUnique);
	DrxLogAlways("Trackview reference count: %d (%d unique)", tvCount, tvUnique);
	DrxLogAlways("Child entity count: %d", childCount);
	DrxLogAlways("Parent entity count: %d", parentCount);
	DrxLogAlways("Linked entity count: %d (%d unique)", linkCount, linkUnique);
	DrxLogAlways("========================");
	DrxLogAlways("Serialized entity classes:");
	for(TClassSaveInfo::const_iterator it = classesSaved.begin(), end = classesSaved.end(); it != end; ++it)
	{
		DrxLogAlways("%s (count %d)", it->first.c_str(), it->second);
	}
	DrxLogAlways("========================");
	DrxLogAlways("Not-serialized entity classes:");
	for(TClassSaveInfo::const_iterator it = classesNotSaved.begin(), end = classesNotSaved.end(); it != end; ++it)
	{
		DrxLogAlways("%s (count %d)", it->first.c_str(), it->second);
	}
	DrxLogAlways("========================");

	assert(output->getChildCount() == saveCount);
#endif

	return true;
}

#undef SERIALIZATION_EXPORT_DEBUG
