// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*

GameAISystem Overview
----------------------
The GameAISystem contains two terms; 'module' and 'instance'.

An entity can enter and leave blocks of logic specific to a context.
We refer to these blocks as 'modules'.

As an example, lets look at AI units in the context of pressure:
When an entity wants to run logic specific to a context we say it
enters the module.  When an entity enters a module an instance,
in this case a pressure instance, is created and mapped to the entity.
The instance contains entity specific data/logic for the context.
In the pressure case such data could be current pressure level,
decrease rate etc.  When the entity is no longer interested in the context
it leaves the module.  This way, only logic/data specific to what the
AI unit needs is running/kept in memory.



GameAISystem Tasks
---------------------
- Use helpers for all modules

*/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameAISystem.h>
#include <drx3D/Game/AIBattleFront.h>
#include <drx3D/Game/SearchModule.h>
#include <drx3D/Game/StalkerModule.h>
#include <drx3D/Game/RangeModule.h>
#include <drx3D/Game/AloneDetectorModule.h>
#include <drx3D/Game/HazardModule/HazardModule.h>
#include <drx3D/Game/RadioChatter/RadioChatterModule.h>
#include <drx3D/AI/IBehaviorTree.h>
#include <drx3D/Game/BehaviorTree/BehaviorTreeNodes_Game.h>
#include <drx3D/Game/GameAIEnv.h>
#include <drx3D/AI/IAISystem.h>
#include <algorithm>

#if DRX_PLATFORM_WINDOWS && !defined(_RELEASE)
# define GAME_AI_ASSERTS_ENABLED
#endif

#if defined(GAME_AI_ASSERTS_ENABLED)
	#include <drx3D/CoreX/Platform/DrxWindows.h>
	#include <shellapi.h> // requires <windows.h>

	#define AssertEntityManipulationIsAllowed() if (m_state != Idle) Error(__FUNCTION__);

	bool g_gameAISystemAssertsAllowed = true;

	class CursorShowerWithStack
	{
	public:
		void StoreCurrentAndShow()
		{
			m_numberOfShows = 1;

			while (gEnv->pInput->ShowCursor(true) < 0)
			{
				++m_numberOfShows;
			}
		}

		void RevertToPrevious()
		{
			while (m_numberOfShows > 0)
			{
				gEnv->pInput->ShowCursor(false);
				--m_numberOfShows;
			}
		}

	private:
		i32 m_numberOfShows;
	};

#else // Not Windows

	#define AssertEntityManipulationIsAllowed()

#endif



CGameAISystem::CGameAISystem()
: m_state(Idle)
, m_pDeathUpr(NULL)
, m_pCorpsesUpr(NULL)
{
	i32k modulesReserveSize = 7;

	m_modules.reserve(modulesReserveSize);
	m_modules.push_back(gGameAIEnv.battleFrontModule = new CAIBattleFrontModule);
	m_modules.push_back(gGameAIEnv.searchModule = new SearchModule);
	m_modules.push_back(gGameAIEnv.stalkerModule = new StalkerModule);
	m_modules.push_back(gGameAIEnv.rangeModule = new RangeModule);
	m_modules.push_back(gGameAIEnv.aloneDetectorModule = new AloneDetectorModule);
	m_modules.push_back(gGameAIEnv.hazardModule = new HazardSystem::HazardModule);
	m_modules.push_back(gGameAIEnv.radioChatterModule = new RadioChatterModule);
	
	RegisterGameBehaviorTreeNodes();
	
	Reset(false);
}

CGameAISystem::~CGameAISystem()
{
	Modules::iterator it = m_modules.begin();
	Modules::iterator end = m_modules.end();

	for ( ; it != end; ++it)
	{
		IGameAIModule* module = *it;
		delete module;
	}

	m_modules.clear();
}

IGameAIModule* CGameAISystem::FindModule(tukk moduleName) const
{
	for (u32 i = 0; i < m_modules.size(); ++i)
	{
		if (strcmp(m_modules[i]->GetName(), moduleName) == 0)
		{
			return m_modules[i];
		}
	}

	return NULL;
}

void CGameAISystem::EnterModule(EntityId entityID, tukk moduleName)
{
	AssertEntityManipulationIsAllowed();

	IGameAIModule* module = FindModule(moduleName);
	if (!module)
	{
		IEntity* entity = gEnv->pEntitySystem->GetEntity(entityID);
		string message;
		message.Format("Could not register entity '%s' [%d] with module '%s' - the module doesn't exist.", entity?entity->GetName():"NullEntity", entityID, moduleName);
		gEnv->pLog->LogError("GameAISystem: %s", message.c_str());
		DRX_ASSERT_MESSAGE(0, message.c_str());
		return;
	}

	module->EntityEnter(entityID);
}

void CGameAISystem::LeaveModule(EntityId entityID, tukk moduleName)
{
	AssertEntityManipulationIsAllowed();

	IGameAIModule* module = FindModule(moduleName);
	if (!module)
	{
		gEnv->pLog->LogError("GameAISystem: Could not unregister entity [%d] with module '%s' - the module doesn't exist.", entityID, moduleName);
		return;
	}

	module->EntityLeave(entityID);
}

void CGameAISystem::LeaveAllModules(EntityId entityID)
{
	AssertEntityManipulationIsAllowed();

	for (u32 i = 0; i < m_modules.size(); ++i)
	{
		IGameAIModule* module = m_modules[i];
		module->EntityLeave(entityID);
	}
}

void CGameAISystem::PauseModule(EntityId entityID, tukk moduleName)
{
	AssertEntityManipulationIsAllowed();

	IGameAIModule* module = FindModule(moduleName);
	if (!module)
	{
		gEnv->pLog->LogError("GameAISystem: Could not pause entity [%d] with module '%s' - the module doesn't exist.", entityID, moduleName);
		return;
	}

	module->EntityPause(entityID);
}

void CGameAISystem::PauseAllModules(EntityId entityID)
{
	AssertEntityManipulationIsAllowed();

	for (u32 i = 0; i < m_modules.size(); ++i)
	{
		IGameAIModule* module = m_modules[i];
		module->EntityPause(entityID);
	}
}

void CGameAISystem::ResumeModule(EntityId entityID, tukk moduleName)
{
	AssertEntityManipulationIsAllowed();

	IGameAIModule* module = FindModule(moduleName);
	if (!module)
	{
		gEnv->pLog->LogError("GameAISystem: Could not resume entity [%d] with module '%s' - the module doesn't exist.", entityID, moduleName);
		return;
	}

	module->EntityResume(entityID);
}

void CGameAISystem::ResumeAllModules(EntityId entityID)
{
	AssertEntityManipulationIsAllowed();

	for (u32 i = 0; i < m_modules.size(); ++i)
	{
		IGameAIModule* module = m_modules[i];
		assert(module);
		if (module)
			module->EntityResume(entityID);
	}
}

void CGameAISystem::Update(float frameTime)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_AI);

	UpdateModules(frameTime);
	UpdateSubSystems(frameTime);

	if(m_pCorpsesUpr)
	{
		m_pCorpsesUpr->Update( frameTime );
	}
}

void CGameAISystem::ResetModules(bool bUnload)
{
	AssertEntityManipulationIsAllowed();

	for (u32 i = 0; i < m_modules.size(); ++i)
	{
		IGameAIModule* module = m_modules[i];
		module->Reset(bUnload);
	}
}

void CGameAISystem::Reset(bool bUnload)
{
	ResetModules(bUnload);
	ResetSubSystems(bUnload);
}

void CGameAISystem::Serialize(TSerialize ser)
{
	m_AIAwarenessToPlayerHelper.Serialize( ser );
	m_AICounters.Serialize( ser );
	if (ser.IsReading())
		Reset(false);

	m_state = UpdatingModules;

	Modules::iterator it = m_modules.begin();
	Modules::iterator end = m_modules.end();

	for ( ; it != end; ++it)
	{
		IGameAIModule& module = *(*it);
		module.Serialize(ser);
	}

	m_state = Idle;
}

void CGameAISystem::PostSerialize()
{
	m_state = UpdatingModules;

	Modules::iterator it = m_modules.begin();
	Modules::iterator end = m_modules.end();

	for ( ; it != end; ++it)
	{
		IGameAIModule& module = *(*it);
		module.PostSerialize();
	}

	m_state = Idle;
}

void CGameAISystem::UpdateModules(float frameTime)
{
	m_state = UpdatingModules;

	Modules::iterator it = m_modules.begin();
	Modules::iterator end = m_modules.end();

	for ( ; it != end; ++it)
	{
		IGameAIModule& module = *(*it);
		module.Update(frameTime);
	}

	m_state = Idle;
}

void CGameAISystem::UpdateSubSystems(float frameTime)
{
	if (m_pDeathUpr)
		m_pDeathUpr->Update();
	m_visibleObjectsHelper.Update();
	m_AIAwarenessToPlayerHelper.Update( frameTime );
	m_AICounters.Update( frameTime );
	m_AISquadUpr.Update(frameTime);
	m_environmentDisturbanceUpr.Update();
	m_advantagePointOccupancyControl.Update();
}

void CGameAISystem::ResetSubSystems(bool bUnload)
{
	m_advantagePointOccupancyControl.Reset();
	m_visibleObjectsHelper.Reset();
	m_AIAwarenessToPlayerHelper.Reset();
	m_AICounters.Reset( bUnload );
	m_AISquadUpr.Reset();
	m_environmentDisturbanceUpr.Reset();

	if (bUnload)
	{
		SAFE_DELETE(m_pDeathUpr);
		SAFE_DELETE(m_pCorpsesUpr);
	}
	else
	{
		if (!m_pDeathUpr)
			m_pDeathUpr = new GameAI::DeathUpr();

		if (!m_pCorpsesUpr)
		{
			if(gEnv->bMultiplayer == false)
			{
				m_pCorpsesUpr = new CAICorpseUpr();
			}
		}

		if (m_pCorpsesUpr)
		{
			m_pCorpsesUpr->Reset();
		}
	}
}

void string_replace(string& str, const string& find_what, const string& replace_with)
{
	string::size_type pos = 0;

	while ((pos = str.find(find_what, pos)) != string::npos)
	{
		str.erase(pos, find_what.length());
		str.insert(pos, replace_with);
		pos += replace_with.length();
	}
}

//void CrashOnPurpose()
//{
//	gEnv->pLog->LogError("Crashing on purpose...");
//	i32* badPointer = 0;
//	*badPointer = 666;
//}

void CGameAISystem::Error(tukk functionName) const
{
	string logMessage;
	logMessage.Format(
		"[%s]: An entity entered or left a module while the modules were being updated.\n"
		"This is bad and we'd appreciate it if you'd help us by informing Jonas, Mario and Marcio.\n",
		functionName);

	gEnv->pLog->LogError("-------------------------- Error in GameAISystem ---------------------------------");
	gEnv->pLog->LogError("%s", logMessage.c_str());

#if defined(GAME_AI_ASSERTS_ENABLED)

	if (ICVar* sys_no_crash_dialog = gEnv->pConsole->GetCVar("sys_no_crash_dialog"))
	{
		const bool dialogsAreAllowed = sys_no_crash_dialog->GetIVal() == 0;
		if (dialogsAreAllowed)
		{
			InformContentCreatorOfError(logMessage);
		}
	}

#else

	gEnv->pLog->LogError("Generating callstack... please wait...");
	string callstack;
	GetCallStack(callstack);
	gEnv->pLog->LogError("Callstack:\n%s", callstack.c_str());
	//CrashOnPurpose();

#endif

}

void CGameAISystem::GetCallStack(string& callstack) const
{
	tukk functionNames[32];
	i32 functionCount = 32;
	gEnv->pSystem->debug_GetCallStack(functionNames, functionCount);

	for (i32 i = 1; i < functionCount; ++i)
	{
		callstack += string().Format("    %02d) %s\n", i, functionNames[i]);
	}
}

void CGameAISystem::InformContentCreatorOfError(string logMessage) const
{
#if defined(GAME_AI_ASSERTS_ENABLED)

	if (g_gameAISystemAssertsAllowed)
	{
		CursorShowerWithStack cursorShowerWithStack;
		cursorShowerWithStack.StoreCurrentAndShow();

		string dialogMessage = logMessage +
			"\n"
			"Do you want to help?\n"
			"Yes\t- Help out and generate callstack (takes around 30 seconds)\n"
			"No\t- Not this time\n"
			"Cancel\t- Ignore similar errors for the rest of this session";

		tukk dialogCaption = "Error in GameAISystem";
		i32 messageBoxResult = DrxMessageBox(dialogMessage.c_str(), dialogCaption, MB_ICONERROR | MB_YESNOCANCEL);

		switch (messageBoxResult)
		{
		case IDYES:
			{
				string callstack;
				GetCallStack(callstack);

				gEnv->pLog->LogError("Callstack:\n%s", callstack.c_str());

				messageBoxResult = DrxMessageBox(
					"The callstack has been generated.\n"
					"\n"
					"Do you want me to prepare an email for you, describing the issue?",
					dialogCaption, MB_ICONINFORMATION | MB_YESNO);

				if (messageBoxResult == IDYES)
				{
					string_replace(callstack, "\n", "%%0A");

					string command = string().Format(
						"mailto:jonas@Dinrus.com;mario@Dinrus.com;marcio@Dinrus.com"
						"?subject=Error in GameAISystem"
						"&body="
						"[Auto-generated e-mail]%%0A"
						"%%0A"
						"Hi,%%0A"
						"%%0A"
						"This is an auto-generated email.%%0A"
						"An error occurred in the GameAISystem: An entity entered or left a module while the modules were being updated.%%0A"
						"%%0A"
						"Here is the callstack:%%0A"
						+ callstack);

					::ShellExecute(0, "open", command.c_str(), "", "", SW_NORMAL);
					DrxMessageBox("Thanks! The email is being created for you.\nHave a nice day! :)", dialogCaption, MB_ICONINFORMATION | MB_OK);
				}
				else
				{
					DrxMessageBox("Ok! The callstack has been written to the log file anyway.\nHave a nice day! :)", dialogCaption, MB_ICONINFORMATION | MB_OK);
				}

				break;
			}

		case IDNO:
			break;

		case IDCANCEL:
			g_gameAISystemAssertsAllowed = false;
			break;
		}

		cursorShowerWithStack.RevertToPrevious();
	}

#endif
}
