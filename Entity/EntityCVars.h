// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//////////////////////////////////////////////////////////////////////////
// Console variables local to EntitySystem.
//////////////////////////////////////////////////////////////////////////
struct SEntityWithCharacterInstanceAutoComplete : public IConsoleArgumentAutoComplete
{
	SEntityWithCharacterInstanceAutoComplete();

	virtual i32         GetCount() const override;
	virtual tukk GetValue(i32 index) const override;
};

struct CVar
{
	// General entity CVars
	static ICVar*      pUpdateScript;
	static ICVar*      pUpdateEntities;
	static ICVar*      pEntityBBoxes;

	static i32         es_DebugTimers;
	static i32         es_DebugFindEntity;
	static i32         es_DebugEvents;
	static i32         es_debugEntityLifetime;
	static i32         es_DebugEntityUsage;
	static tukk es_DebugEntityUsageFilter;
	static i32         es_DebugEntityUsageSortMode;
	static i32         es_debugDrawEntityIDs;

	static i32         es_LayerSaveLoadSerialization;
	static i32         es_SaveLoadUseLUANoSaveFlag;
	static i32         es_LayerDebugInfo;

	static i32         es_profileComponentUpdates;

	// Physics CVars
	static ICVar* pMinImpulseVel;
	static ICVar* pImpulseScale;
	static ICVar* pMaxImpulseAdjMass;
	static ICVar* pDebrisLifetimeScale;
	static ICVar* pHitCharacters;
	static ICVar* pHitDeadBodies;
	static ICVar* pLogCollisions;

	static i32    es_UsePhysVisibilityChecks;

	static float  es_MaxPhysDist;
	static float  es_MaxPhysDistInvisible;
	static float  es_MaxPhysDistCloth;
	static float  es_FarPhysTimeout;

	static i32    es_MaxJointFx;

	// Area manager CVars
	static ICVar* pDrawAreas;
	static ICVar* pDrawAreaGrid;
	static ICVar* pDrawAreaGridCells;
	static ICVar* pDrawAreaDebug;

	static float  es_EntityUpdatePosDelta;

	// Script CVars
	static ICVar* pEnableFullScriptSave;
	static ICVar* pSysSpecLight;

	// Initialize console variables.
	static void Init();

	// Dump Entities
	static void DumpEntities(IConsoleCmdArgs*);
	static void DumpEntityClassesInUse(IConsoleCmdArgs*);

	// Recompile area grid
	static void CompileAreaGrid(IConsoleCmdArgs*);

	static void EnableDebugAnimText(IConsoleCmdArgs* args);
	static void SetDebugAnimText(CEntity* entity, const bool enable);

	// Console commands to enable/disable layers
	static void ConsoleCommandToggleLayer(IConsoleCmdArgs* pArgs);
};
