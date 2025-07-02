// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   StatsAgent.cpp
//  Version:     v1.00
//  Created:     06/10/2009 by Steve Barnett.
//  Описание: This the declaration for CStatsAgent
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Game/StdAfx.h>

#include <drx3D/CoreX/Project/ProjectDefines.h>
#include <drx3D/Sys/ICmdLine.h>

#if defined(ENABLE_STATS_AGENT)

#	include <cstdlib>
#	include <cstring>

#	include "StatsAgent.h>
#	include "StatsAgentPipe.h>
#	include <DrxThreading/MultiThread_Containers.h>

# include <DrxThreading/IThreadUpr.h>
#	include "ITelemetryCollector.h>

namespace
{
	class CStatsAgentThread;

	/* 10mins - assuming 60fps which the game can run at when it is in the precache phase */
	i32k DELAY_MESSAGES_TIMEOUT = 60 * 60 * 10;

	typedef DrxMT::vector<DrxStringLocal> TStatsAgentMessageQueue;
	static TStatsAgentMessageQueue* s_pMessageQueue;

	static CStatsAgentThread* s_pStatsThread = 0;
}

namespace
{
	class CStatsAgentThread : public IThread
	{
	public:
		CStatsAgentThread()
			: m_bQuit(false)
		{
			if (!gEnv->pThreadUpr->SpawnThread(this, "StatsAgent"))
			{
				DrxFatalError((string().Format("Error spawning \"%s\" thread.", "StatsAgent").c_str()));
			}
		}

		virtual ~CStatsAgentThread()
		{
			SignalStopWork();
			gEnv->pThreadUpr->JoinThread(this, eJM_Join);
		}

		// Signals the thread that it should not accept anymore work and exit
		void SignalStopWork()
		{
			m_bQuit = true;
		}

	protected:
		// Start accepting work on thread
		virtual void ThreadEntry()
		{
			//ScopedSwitchToGlobalHeap useGlobalHeap;

			while (!m_bQuit)
			{
				if (CStatsAgentPipe::PipeOpen())
				{
					if (tukk szMessage = CStatsAgentPipe::Receive())
					{
						if (*szMessage)
							s_pMessageQueue->push_back(szMessage);
					}
				}
				DrxSleep(1000);
			}
		}

		 bool	m_bQuit;
	};
}	
struct CSystemEventListener_StatsAgent : public ISystemEventListener
{
public:
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
	{
		switch (event)
		{
			case ESYSTEM_EVENT_LEVEL_LOAD_PREPARE:
			case ESYSTEM_EVENT_LEVEL_LOAD_START_LOADINGSCREEN:
			case ESYSTEM_EVENT_LEVEL_LOAD_START:
			case ESYSTEM_EVENT_LEVEL_UNLOAD:
			{
				DrxLogAlways("CStatsAgent: starting delay of messages");
				CStatsAgent::SetDelayMessages(true);
				break;
			}
			case ESYSTEM_EVENT_LEVEL_LOAD_END:
			case ESYSTEM_EVENT_LEVEL_PRECACHE_START:
			{
				DrxLogAlways("CStatsAgent: starting long timeout of messages");
				CStatsAgent::SetDelayTimeout(DELAY_MESSAGES_TIMEOUT);
				break;
			}
			case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
			case ESYSTEM_EVENT_LEVEL_GAMEPLAY_START:
			case ESYSTEM_EVENT_LEVEL_PRECACHE_END:
			{
				DrxLogAlways("CStatsAgent: stopping delay of messages");
				CStatsAgent::SetDelayMessages(false);
				break;
			}
		}
	}
};
static CSystemEventListener_StatsAgent g_system_event_listener_statsagent;

i32k DELAY_MESSAGES_EXIT_DELAY = 10;

bool CStatsAgent::s_delayMessages = false;
i32 CStatsAgent::s_iDelayMessagesCounter = -1;

void CStatsAgent::CreatePipe(const ICmdLineArg* pPipeName)
{
	if (pPipeName)
	{
		s_delayMessages = false;
		gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_statsagent);

		CStatsAgentPipe::OpenPipe(pPipeName->GetValue());

		if (CStatsAgentPipe::PipeOpen())
		{
			// Kick off a thread to listen on the pipe
			s_pMessageQueue = new TStatsAgentMessageQueue();
			s_pStatsThread = new CStatsAgentThread();		
		}
	}
}

void CStatsAgent::ClosePipe()
{
	if (s_pStatsThread)
	{
		s_pStatsThread->SignalStopWork();
		gEnv->pThreadUpr->JoinThread(s_pStatsThread, eJM_Join);
		SAFE_DELETE(s_pStatsThread);
		SAFE_DELETE(s_pMessageQueue);
		gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(&g_system_event_listener_statsagent);
	}
	CStatsAgentPipe::ClosePipe();
}

void CStatsAgent::Update()
{
	if (CStatsAgentPipe::PipeOpen())
	{
		if (s_delayMessages)
		{
			const size_t numMessages = s_pMessageQueue->size();
			DrxLogAlways("CStatsAgent: Delaying processing of %d messages delayCounter:%d", numMessages, s_iDelayMessagesCounter);
			for( size_t i = 0; i < numMessages; i++)
			{
				DrxLogAlways("CStatsAgent: Message[%d] '%s'", i, (*s_pMessageQueue)[i].c_str());
			}
			if (s_iDelayMessagesCounter >= 0)
			{
				--s_iDelayMessagesCounter;
				if (s_iDelayMessagesCounter <= 0)
				{
					s_iDelayMessagesCounter = 0;
					s_delayMessages = false;
				}
			}
		}
		DrxStringLocal message;
		std::vector<DrxStringLocal> unprocessedMessages;
		bool saveMessages = false;
		while (s_pMessageQueue->try_pop_front(message))
		{
			// Can't process the message - have to save it and put it back after the queue has been drained
			if (saveMessages == true)
			{
				unprocessedMessages.push_back(message);
				continue;
			}
			// Just return the dump filename
			if (!strncmp(message.c_str(), "getdumpfilename", 15))
			{
				DrxReplayInfo info;
				DrxGetIMemReplay()->GetInfo(info);
				tukk pFilename = info.filename;
				if (!pFilename) { pFilename = "<unknown>"; }

				CStatsAgentPipe::Send(pFilename, "dumpfile", message.c_str());
			}
			// Get the value of a CVAR
			else if (!strncmp(message.c_str(), "getcvarvalue ", 13))
			{
				tukk const pCVARName = message.c_str() + 13;
				if (ICVar* pValue = gEnv->pConsole->GetCVar(pCVARName))
				{
					CStatsAgentPipe::Send(pValue->GetString(), "cvarvalue", message.c_str());
				}
				else
				{
					DrxFixedStringT<64> response = "Unknown CVAR '";
					response += pCVARName;
					response += "'";
					CStatsAgentPipe::Send(response.c_str(), "error", message.c_str());
				}
			}
			// Create a new CVAR
			else if (!strncmp(message.c_str(), "createcvar ", 11))
			{
				tukk const pCVARName = message.c_str() + 11;
				REGISTER_STRING(pCVARName, "", 0, "Amble CVAR");

				CStatsAgentPipe::Send("finished", NULL, message.c_str());
			}
			// Get telemetry session ID
			else if (!strncmp(message.c_str(), "getsessionid", 12))
			{
				ITelemetryCollector* tc = NULL;
				if (g_pGame)
				{
					tc = g_pGame->GetITelemetryCollector();
				}
				if (tc)
				{
					CStatsAgentPipe::Send(tc->GetSessionId().c_str(), "sessionid", message.c_str());
				}
				else
				{
					CStatsAgentPipe::Send("", "sessionid", message.c_str());
				}
			}
			// Execute the command
			else if (!strncmp(message.c_str(), "exec ", 5))
			{
				if (s_delayMessages == false)
				{
					// Execute the rest of the string
					if (gEnv && gEnv->pConsole)
					{
						tukk pCommand = message.c_str() + 5;
						gEnv->pConsole->ExecuteString(pCommand);

						CStatsAgentPipe::Send("finished", NULL, message.c_str());
					}
				}
				else
				{
					// Stop processing the message queue - store the queue and restore it
					saveMessages = true;
					unprocessedMessages.push_back(message);
				}
			}
			// Unknown command
			else
			{
				DrxFixedStringT<64> response = "Unknown command '";
				response += message.c_str();
				response += "'";
				CStatsAgentPipe::Send(response.c_str(), "error", message.c_str());
			}
		}
		for (size_t i = 0; i < unprocessedMessages.size(); i++)
		{
			message = unprocessedMessages[i];
			s_pMessageQueue->push_back(message);
		}
	}
}

void CStatsAgent::SetDelayTimeout(i32k timeout)
{
	DrxLogAlways("CStatsAgent: SetDelayTimeout %d", timeout);
	s_iDelayMessagesCounter = timeout;
}

void CStatsAgent::SetDelayMessages(bool enable)
{
	if (enable)
	{
		s_delayMessages = true;
		s_iDelayMessagesCounter = -1;
	}
	else if (s_delayMessages)
	{
		SetDelayTimeout(DELAY_MESSAGES_EXIT_DELAY);
	}
}

#endif	// defined(ENABLE_STATS_AGENT)
