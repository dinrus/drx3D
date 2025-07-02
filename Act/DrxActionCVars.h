// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------

   Rip off NetworkCVars, general DinrusAction CVars should be here...

   -------------------------------------------------------------------------
   История:
   - 6:6:2007   Created by Benito G.R.

*************************************************************************/
#ifndef __DRXACTIONCVARS_H__
#define __DRXACTIONCVARS_H__

#pragma once

#include <drx3D/Sys/IConsole.h>

class CDrxActionCVars
{
public:

	float playerInteractorRadius;  //Controls CInteractor action radius
	i32   debugItemMemStats;       //Displays item mem stats

	i32   g_debug_stats;
	i32   g_statisticsMode;
	i32   useCurrentUserNameAsDefault;

#if !defined(_RELEASE)
	i32 g_userNeverAutoSignsIn;
#endif

#ifdef AI_LOG_SIGNALS
	i32   aiLogSignals;
	float aiMaxSignalDuration;
#endif
	i32   aiFlowNodeAlertnessCheck;

	i32 g_gameplayAnalyst;
	i32 g_multiplayerEnableVehicles;

	// Cooperative Animation System
	i32 co_coopAnimDebug;
	i32 co_usenewcoopanimsystem;
	i32 co_slideWhileStreaming;
	// ~Cooperative Animation System

	i32 g_syncClassRegistry;

	i32 g_allowSaveLoadInEditor;
	i32 g_saveLoadBasicEntityOptimization;
	i32 g_debugSaveLoadMemory;
	i32 g_saveLoadUseExportedEntityList;
	i32 g_useXMLCPBinForSaveLoad;
	i32 g_XMLCPBGenerateXmlDebugFiles;
	i32 g_XMLCPBAddExtraDebugInfoToXmlDebugFiles;
	i32 g_XMLCPBSizeReportThreshold;
	i32 g_XMLCPBUseExtraZLibCompression;
	i32 g_XMLCPBBlockQueueLimit;
	i32 g_saveLoadExtendedLog;

	i32 g_debugDialogBuffers;

	i32 g_allowDisconnectIfUpdateFails;

	i32 g_useSinglePosition;
	i32 g_handleEvents;

	i32 g_enableMergedMeshRuntimeAreas;

	// AI stances
	ICVar* ag_defaultAIStance;

	i32    sw_gridSize;
	i32    sw_debugInfo;

	static ILINE CDrxActionCVars& Get()
	{
		DRX_ASSERT(s_pThis);
		return *s_pThis;
	}

private:
	friend class CDrxAction; // Our only creator

	CDrxActionCVars(); // singleton stuff
	~CDrxActionCVars();

	static CDrxActionCVars* s_pThis;

	static void DumpEntitySerializationData(IConsoleCmdArgs* pArgs);
	static void DumpClassRegistry(IConsoleCmdArgs* pArgs);
};

#endif // __DRXACTIONCVARS_H__
