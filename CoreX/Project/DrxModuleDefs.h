// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#define eDrxM_3DEngine 1
#define eDrxM_GameFramework 2
#define eDrxM_AISystem 3
#define eDrxM_Animation 4
#define eDrxM_DynamicResponseSystem 5
#define eDrxM_EntitySystem 6
#define eDrxM_Font 7
#define eDrxM_Input 8
#define eDrxM_Movie 9
#define eDrxM_Network 10
// DrxLobby is now an engine plug-in
//#define eDrxM_Lobby 11
#define eDrxM_Physics 12
#define eDrxM_ScriptSystem 13
#define eDrxM_AudioSystem 14
#define eDrxM_System 15
#define eDrxM_LegacyGameDLL 16
#define eDrxM_Render 17
#define eDrxM_Launcher 18
#define eDrxM_Editor 19
#define eDrxM_LiveCreate 20
// DrxOnline has been removed from the engine build for now
//#define eDrxM_Online 21
#define eDrxM_AudioImplementation 22
#define eDrxM_MonoBridge 23
#define eDrxM_ScaleformHelper 24
#define eDrxM_FlowGraph 25
//! Legacy module, does not need a specific name and is only kept for backwards compatibility
#define eDrxM_Legacy 26

#define eDrxM_EnginePlugin 27
#define eDrxM_EditorPlugin 28
#define eDrxM_Schematyc2 29

#define eDrxM_Num 30
#define eDrxM_Game 31

static const wchar_t* g_moduleNames[] =
{
	L"",
	L"drx3D_Eng3D",
	L"drx3D_Act",
	L"drx3D_AI",
	L"drx3D_Animation",
	L"drx3D_DynRespSys",
	L"drx3D_Entity",
	L"drx3D_Font",
	L"drx3D_Input",
	L"drx3D_Movie",
	L"drx3D_Network",
	L"drx3D_Lobby",
	L"drx3D_Phys",
	L"drx3D_Script",
	L"drx3D_Audio",
	L"drx3D_Sys",
	L"drx3D_Game",
	L"drx3D_Renderer",
	L"Launcher",
	L"Sandbox",
	L"drx3D_LiveCreate",
	L"drx3D_Online",
	L"drx3D_AudioImplementation",
	L"drx3D_MonoBridge",
	L"drx3D_ScaleformHelper",
	L"drx3D_FlowGraph",
	L"Legacy Module",
	L"Engine Plug-ins",
	L"Editor Plug-ins",
	L"Sxema2"
};