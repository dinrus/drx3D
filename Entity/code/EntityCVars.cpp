// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>
#include <drx3D/Entity/EntityCVars.h>
#include <drx3D/Entity/Entity.h>
#include <drx3D/Entity/EntitySystem.h>
#include <drx3D/Entity/AreaUpr.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Entity/IEntityComponent.h>
#include <drx3D/AI/IAISystem.h>

ICVar* CVar::pUpdateScript = NULL;
ICVar* CVar::pUpdateEntities = NULL;
ICVar* CVar::pEntityBBoxes = NULL;
ICVar* CVar::pMinImpulseVel = NULL;
ICVar* CVar::pImpulseScale = NULL;
ICVar* CVar::pMaxImpulseAdjMass = NULL;
ICVar* CVar::pDebrisLifetimeScale = NULL;
ICVar* CVar::pHitCharacters = NULL;
ICVar* CVar::pHitDeadBodies = NULL;
ICVar* CVar::pEnableFullScriptSave = NULL;
ICVar* CVar::pLogCollisions = NULL;
ICVar* CVar::pDrawAreas = NULL;
ICVar* CVar::pDrawAreaGrid = NULL;
ICVar* CVar::pDrawAreaGridCells = NULL;
ICVar* CVar::pDrawAreaDebug = NULL;

ICVar* CVar::pSysSpecLight = NULL;

i32 CVar::es_DebugTimers = 0;
i32 CVar::es_DebugFindEntity = 0;
i32 CVar::es_UsePhysVisibilityChecks = 1;
float CVar::es_MaxPhysDist;
float CVar::es_MaxPhysDistInvisible;
float CVar::es_MaxPhysDistCloth;
float CVar::es_FarPhysTimeout;
i32 CVar::es_DebugEvents = 0;
i32 CVar::es_debugEntityLifetime = 0;
i32 CVar::es_DebugEntityUsage = 0;
tukk CVar::es_DebugEntityUsageFilter = "";
i32 CVar::es_DebugEntityUsageSortMode = 0;
i32 CVar::es_LayerSaveLoadSerialization = 0;
i32 CVar::es_LayerDebugInfo = 0;
i32 CVar::es_SaveLoadUseLUANoSaveFlag = 1;
float CVar::es_EntityUpdatePosDelta = 0.0f;
i32 CVar::es_debugDrawEntityIDs = 0;
i32 CVar::es_MaxJointFx = 8;

i32 CVar::es_profileComponentUpdates = 0;

// for editor only
static void OnSysSpecLightChange(ICVar* pVar)
{
	IEntityItPtr it = GetIEntitySystem()->GetEntityIterator();
	it->MoveFirst();

	while (CEntity* pEntity = static_cast<CEntity*>(it->Next()))
	{
		IScriptTable* pScriptTable = pEntity->GetScriptTable();
		if (pScriptTable && pScriptTable->HaveValue("OnSysSpecLightChanged"))
		{
			Script::CallMethod(pScriptTable, "OnSysSpecLightChanged");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
SEntityWithCharacterInstanceAutoComplete::SEntityWithCharacterInstanceAutoComplete()
{
}

//////////////////////////////////////////////////////////////////////////
i32 SEntityWithCharacterInstanceAutoComplete::GetCount() const
{
	u32 count = 0;
	auto itEntity = GetIEntitySystem()->GetEntityIterator();
	while (!itEntity->IsEnd())
	{
		if (CEntity* pEnt = static_cast<CEntity*>(itEntity->Next()))
		{
			u32k numSlots = pEnt->GetSlotCount();
			for (u32 i = 0; i < numSlots; i++)
			{
				ICharacterInstance* pCharInst = pEnt->GetCharacter(i);
				if (pCharInst)
				{
					count++;
				}
			}
		}
	}

	return count;
}

//////////////////////////////////////////////////////////////////////////
tukk SEntityWithCharacterInstanceAutoComplete::GetValue(i32 index) const
{
	auto itEntity = GetIEntitySystem()->GetEntityIterator();
	u32 count = 0;
	while (!itEntity->IsEnd())
	{
		if (CEntity* pEnt = static_cast<CEntity*>(itEntity->Next()))
		{
			u32k numSlots = pEnt->GetSlotCount();
			for (u32 i = 0; i < numSlots; i++)
			{
				ICharacterInstance* pCharInst = pEnt->GetCharacter(i);
				if (pCharInst)
				{
					if (count == index)
					{
						return pEnt->GetName();
					}
					count++;
				}
			}
		}
	}

	return "";
}

static SEntityWithCharacterInstanceAutoComplete s_entityWithCharacterInstanceAutoComplete;

//////////////////////////////////////////////////////////////////////////
void CVar::Init()
{
	assert(gEnv->pConsole);
	PREFAST_ASSUME(gEnv->pConsole);

	REGISTER_COMMAND("es_dump_entities", (ConsoleCommandFunc)DumpEntities, 0, "Dumps current entities and their states!");
	REGISTER_COMMAND("es_dump_entity_classes_in_use", (ConsoleCommandFunc)DumpEntityClassesInUse, 0, "Dumps all used entity classes");
	REGISTER_COMMAND("es_compile_area_grid", (ConsoleCommandFunc)CompileAreaGrid, 0, "Trigger a recompile of the area grid");

	pEntityBBoxes = REGISTER_INT("es_bboxes", 0, VF_CHEAT,
	                             "Toggles entity bounding boxes.\n"
	                             "Usage: es_bboxes [0/1]\n"
	                             "Default is 0 (off). Set to 1 to display bounding boxes.");
	pUpdateScript = REGISTER_INT("es_UpdateScript", 1, VF_CHEAT,
	                             "Usage: es_UpdateScript [0/1]\n"
	                             "Default is 1 (on).");
	pUpdateEntities = REGISTER_INT("es_UpdateEntities", 1, VF_CHEAT,
	                               "Toggles entity updating.\n"
	                               "Usage: es_UpdateEntities [0/1]\n"
	                               "Default is 1 (on). Set to 0 to prevent all entities from updating.");
	pMinImpulseVel = REGISTER_FLOAT("es_MinImpulseVel", 0.0f, VF_CHEAT,
	                                "Usage: es_MinImpulseVel 0.0");
	pImpulseScale = REGISTER_FLOAT("es_ImpulseScale", 0.0f, VF_CHEAT,
	                               "Usage: es_ImpulseScale 0.0");
	pMaxImpulseAdjMass = REGISTER_FLOAT("es_MaxImpulseAdjMass", 2000.0f, VF_CHEAT,
	                                    "Usage: es_MaxImpulseAdjMass 2000.0");
	pDebrisLifetimeScale = REGISTER_FLOAT("es_DebrisLifetimeScale", 1.0f, 0,
	                                      "Usage: es_DebrisLifetimeScale 1.0");
	pHitCharacters = REGISTER_INT("es_HitCharacters", 1, 0,
	                              "specifies whether alive characters are affected by bullet hits (0 or 1)");
	pHitDeadBodies = REGISTER_INT("es_HitDeadBodies", 1, 0,
	                              "specifies whether dead bodies are affected by bullet hits (0 or 1)");

	pEnableFullScriptSave = REGISTER_INT("es_enable_full_script_save", 0,
	                                     VF_DUMPTODISK, "Enable (experimental) full script save functionality");

	pLogCollisions = REGISTER_INT("es_log_collisions", 0, 0, "Enables collision events logging");
	REGISTER_CVAR(es_DebugTimers, 0, VF_CHEAT,
	              "This is for profiling and debugging (for game coders and level designer)\n"
	              "By enabling this you get a lot of console printouts that show all entities that receive OnTimer\n"
	              "events - it's good to minimize the call count. Certain entities might require this feature and\n"
	              "using less active entities can often be defined by the level designer.\n"
	              "Usage: es_DebugTimers 0/1");
	REGISTER_CVAR(es_DebugFindEntity, 0, VF_CHEAT, "");
	REGISTER_CVAR(es_DebugEvents, 0, VF_CHEAT, "Enables logging of entity events");

	REGISTER_CVAR(es_DebugEntityUsage, 0, 0,
	              "Draws information to the screen to show how entities are being used, per class, including total, active and hidden counts and memory usage"
	              "\nUsage: es_DebugEntityUsage update_rate"
	              "\nupdate_rate - Time in ms to refresh memory usage calculation or 0 to disable");
	REGISTER_CVAR(es_DebugEntityUsageFilter, "", 0, "Filter entity usage debugging to classes which have this string in their name");
	REGISTER_CVAR(es_DebugEntityUsageSortMode, 0, 0, "Determines how es_DebugEntityUsage sorts the visual output\n0 = unsorted\n1 = sort by number of active instances\n2 = sort by memory usage");

	REGISTER_CVAR(es_LayerSaveLoadSerialization, 0, VF_CHEAT,
	              "Switches layer entity serialization: \n"
	              "0 - serialize all \n"
	              "1 - automatically ignore entities on disabled layers \n"
	              "2 - only ignore entities on non-save layers.");
	REGISTER_CVAR(es_LayerDebugInfo, 0, VF_CHEAT,
	              "Render debug info on active layers: \n"
	              "0 - inactive \n"
	              "1 - active brush layers \n"
	              "2 - all layer info \n"
	              "3 - all layer and all layer pak info");
	REGISTER_CVAR(es_SaveLoadUseLUANoSaveFlag, 0, VF_CHEAT, "Save&Load optimization : use lua flag to not serialize entities, for example rigid bodies.");

	pDrawAreas = REGISTER_INT("es_DrawAreas", 0, VF_CHEAT, "Enables drawing of Areas");
	pDrawAreaGrid = REGISTER_INT("es_DrawAreaGrid", 0, VF_CHEAT, "Enables drawing of Area Grid");
	pDrawAreaGridCells = REGISTER_INT("es_DrawAreaGridCells", 0, VF_CHEAT, "Enables drawing of Area Grid Cells' number and coordinates. Requires \"es_DrawAreaGrid\" to be enabled!");
	pDrawAreaDebug = REGISTER_INT("es_DrawAreaDebug", 0, VF_CHEAT, "Enables debug drawing of Areas, set 2 for log details");

	REGISTER_CVAR(es_UsePhysVisibilityChecks, 1, 0,
	              "Activates physics quality degradation and forceful sleeping for invisible and faraway entities");
	REGISTER_CVAR(es_MaxPhysDist, 300.0f, 0,
	              "Physical entities farther from the camera than this are forcefully deactivated");
	REGISTER_CVAR(es_MaxPhysDistCloth, 300.0f, 0,
	              "Cloth entities farther from the camera than this are forcefully deactivated");
	REGISTER_CVAR(es_MaxPhysDistInvisible, 40.0f, 0,
	              "Invisible physical entities farther from the camera than this are forcefully deactivated");
	REGISTER_CVAR(es_FarPhysTimeout, 4.0f, 0,
	              "Timeout for faraway physics forceful deactivation");

	pSysSpecLight = gEnv->pConsole->GetCVar("sys_spec_light");
	if (pSysSpecLight && gEnv->IsEditor())
		pSysSpecLight->SetOnChangeCallback(OnSysSpecLightChange);

	REGISTER_CVAR(es_debugEntityLifetime, 0, 0,
	              "Debug entities creation and deletion time");

	REGISTER_COMMAND("es_debugAnim", (ConsoleCommandFunc)EnableDebugAnimText, 0, "Debug entity animation (toggle on off)");
	gEnv->pConsole->RegisterAutoComplete("es_debugAnim", &s_entityWithCharacterInstanceAutoComplete);

	REGISTER_COMMAND("es_togglelayer", &ConsoleCommandToggleLayer, VF_DEV_ONLY, "Toggles a layer (on/off)\n Usage: es_togglelayer LAYER_NAME\nPlease bear in mind that layer names are case-sensitive");

	REGISTER_CVAR(es_EntityUpdatePosDelta, 0.1f, 0,
	              "Indicates the position delta by which an entity must move before the AreaUpr updates position relevant data.\n"
	              "Default: 0.1 (10 cm)");

	REGISTER_CVAR(es_debugDrawEntityIDs, 0, VF_CHEAT,
	              "Displays the EntityId of all entities.\n"
	              "Default is 0 (off), any other number enables it.\n"
	              "Note: es_debug must be set to 1 also (or else the EntityId won't be displayed)");

	REGISTER_CVAR(es_MaxJointFx, 8, 0, "Sets the maximum number of joint break fx per frame");

	REGISTER_CVAR(es_profileComponentUpdates, 0, 0, "Enables profiling of components that are updated per frame.\n"
	                                                "Default: 0 (off)\n"
	                                                "1 - Simple profiling, shows cost of all components per frame\n"
	                                                "2 - Component type cost braekdown, shows cost of each component type per frame");
}

void CVar::DumpEntities(IConsoleCmdArgs* args)
{
	GetIEntitySystem()->DumpEntities();
}

void CVar::DumpEntityClassesInUse(IConsoleCmdArgs* args)
{
	IEntityItPtr it = GetIEntitySystem()->GetEntityIterator();
	it->MoveFirst();

	std::map<string, i32> classes;
	while (CEntity* pEntity = static_cast<CEntity*>(it->Next()))
	{
		classes[pEntity->GetClass()->GetName()]++;
	}

	DrxLogAlways("--------------------------------------------------------------------------------");
	for (std::map<string, i32>::iterator iter = classes.begin(); iter != classes.end(); ++iter)
		DrxLogAlways("%s: %d instances", iter->first.c_str(), iter->second);
}

void CVar::CompileAreaGrid(IConsoleCmdArgs*)
{
	CEntitySystem* pEntitySystem = GetIEntitySystem();
	CAreaUpr* pAreaUpr = (pEntitySystem ? static_cast<CAreaUpr*>(pEntitySystem->GetAreaUpr()) : 0);
	if (pAreaUpr)
		pAreaUpr->SetAreasDirty();
}

void CVar::SetDebugAnimText(CEntity* pEntity, const bool bEnable)
{
	CEntitySystem* pEntitySystem = GetIEntitySystem();

	if (pEntity)
	{
		u32 numSlots = pEntity->GetSlotCount();
		for (u32 i = 0; i < numSlots; i++)
		{
			ICharacterInstance* pCharInst = pEntity->GetCharacter(i);
			if (pCharInst)
			{
				pCharInst->GetISkeletonAnim()->SetDebugging(bEnable);
				IAttachmentUpr* pAttachmentUpr = pCharInst->GetIAttachmentUpr();
				for (i32 attachmentIndex = 0; attachmentIndex < pAttachmentUpr->GetAttachmentCount(); ++attachmentIndex)
				{
					IAttachment* pAttachment = pAttachmentUpr->GetInterfaceByIndex(attachmentIndex);
					assert(pAttachment);

					IAttachmentObject* pObject = pAttachment->GetIAttachmentObject();
					if (pObject)
					{
						ICharacterInstance* pObjectCharInst = pObject->GetICharacterInstance();
						if (pObjectCharInst)
						{
							pObjectCharInst->GetISkeletonAnim()->SetDebugging(bEnable);
						}
						if (pObject->GetAttachmentType() == IAttachmentObject::eAttachment_Entity)
						{
							CEntity* pAttachmentEntity = pEntitySystem->GetEntityFromID(static_cast<CEntityAttachment*>(pObject)->GetEntityId());
							if (pAttachmentEntity)
							{
								SetDebugAnimText(pAttachmentEntity, bEnable);
							}
						}
					}
				}
			}
		}
	}
}

void CVar::EnableDebugAnimText(IConsoleCmdArgs* args)
{
	if (args && args->GetArgCount() > 1)
	{
		tukk szFilterName = args->GetArg(1);
		bool bEnable = true;
		if (args->GetArgCount() > 2)
		{
			bEnable = (strcmp(args->GetArg(2), "0") != 0);
		}

		CEntitySystem* pEntitySystem = GetIEntitySystem();
		CEntity* pEntity = static_cast<CEntity*>(pEntitySystem->FindEntityByName(szFilterName));

		SetDebugAnimText(pEntity, bEnable);
	}
}

void CVar::ConsoleCommandToggleLayer(IConsoleCmdArgs* pArgs)
{
	// Note: based on Flow Node Engine:LayerSwitch
	if (pArgs && pArgs->GetArgCount() > 1 && g_pIEntitySystem)
	{
		tukk szLayerName = pArgs->GetArg(1);
		const bool bSerialize = false;
		const bool bShouldBeEnabled = !g_pIEntitySystem->IsLayerEnabled(szLayerName, false);

		DrxLogAlways("[Info][Layers] Toggling EntitySystemLayer %s to: %s", szLayerName, bShouldBeEnabled ? "Enabled" : "Disabled");
		g_pIEntitySystem->EnableLayer(szLayerName, bShouldBeEnabled, bSerialize);

		if (bShouldBeEnabled && gEnv->pAISystem)
		{
			DrxLogAlways("[Info][Layers] Toggling AISystemLayer %s to: %s", szLayerName, bShouldBeEnabled ? "Enabled" : "Disabled");
			gEnv->pAISystem->LayerEnabled(szLayerName, bShouldBeEnabled, bSerialize);
		}
	}
}
