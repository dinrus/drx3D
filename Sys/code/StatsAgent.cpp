// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

#include <drx3D/CoreX/Project/ProjectDefines.h>
#include <drx3D/Sys/ICmdLine.h>

#if defined(ENABLE_STATS_AGENT)

	#include <cstdlib>
	#include <cstring>
	#include <atomic>

	#include <drx3D/Sys/StatsAgent.h>
	#include <drx3D/Sys/StatsAgentPipe.h>
	#include <drx3D/CoreX/Thread/MultiThread_Containers.h>

	#include <drx3D/CoreX/Thread/IThreadUpr.h>

class CStatsAgentImpl
{
	/* 10mins - assuming 60fps which the game can run at when it is in the precache phase */
	static i32k DELAY_MESSAGES_TIMEOUT = 60 * 60 * 10;
	static i32k DELAY_MESSAGES_EXIT_DELAY = 10;

	class CThread : public ::IThread
	{
	public:
		CThread(class CStatsAgentImpl& statsAgentImpl)
			: m_StatsAgentImpl(statsAgentImpl)
		{
			if (!gEnv->pThreadUpr->SpawnThread(this, "StatsAgent"))
			{
				DrxFatalError((string().Format("Error spawning \"%s\" thread.", "StatsAgent").c_str()));
			}
		}

		virtual ~CThread()
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
		virtual void ThreadEntry() override;

	private:
		class CStatsAgentImpl& m_StatsAgentImpl;
		std::atomic<bool>      m_bQuit = false;
	};

	class CSystemEventListener : public ::ISystemEventListener
	{
	public:
		CSystemEventListener()
		{
			gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CStatsAgent");
		}

		virtual ~CSystemEventListener()
		{
			gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);
		}
	protected:
		virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override
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

	DrxMT::vector<DrxStringLocal> m_MessageQueue;
	std::unique_ptr<CThread>      m_pStatsThread;
	bool                          m_bDelayMessages = false;
	i32                           m_nDelayMessagesCounter = -1;
	CSystemEventListener          m_listener;

	/* Helper functions */

	//Whether string starts with certain prefix literal.
	template<i32 Length>
	static bool StrStartsWith(const DrxStringLocal& str, const char(&szSubstr)[Length])
	{
		static_assert(Length >= 1, "invalid string literal");     //at least '\0'
		if (Length - 1 > str.GetLength())
			return false;
		return strncmp(str.c_str(), szSubstr, Length - 1) == 0;
	}

	//Returns remaining 0-terminated substring if matching prefix, or nullptr otherwise.
	template<i32 Length>
	static tukk TryParse(const DrxStringLocal& str, const char(&szSubstr)[Length])
	{
		if (StrStartsWith(str, szSubstr))
		{
			return str.c_str() + Length - 1;
		}
		return nullptr;
	}

public:
	CStatsAgentImpl(tukk szPipeName)
	{
		if (CStatsAgentPipe::OpenPipe(szPipeName))
		{
			// Kick off a thread to listen on the pipe
			m_pStatsThread = stl::make_unique<CThread>(*this);
		}
	}

	~CStatsAgentImpl()
	{
		CStatsAgentPipe::ClosePipe();
	}

	void SetDelayTimeout(i32k timeout)
	{
		m_nDelayMessagesCounter = timeout;
	}

	void SetDelayMessages(bool bEnable)
	{
		if (bEnable)
		{
			m_bDelayMessages = true;
			m_nDelayMessagesCounter = -1;
		}
		else if (m_bDelayMessages)
		{
			SetDelayTimeout(DELAY_MESSAGES_EXIT_DELAY);
		}
	}

	void Update()
	{
		if (CStatsAgentPipe::IsPipeOpen())
		{
			if (m_bDelayMessages)
			{
				auto numMessages = m_MessageQueue.size();
				DrxLogAlways("CStatsAgent: Delaying processing of %d messages delayCounter:%d", numMessages, m_nDelayMessagesCounter);
				for (auto i = 0; i < numMessages; i++)
				{
					DrxLogAlways("CStatsAgent: Message[%d] '%s'", i, m_MessageQueue[i].c_str());
				}
				if (m_nDelayMessagesCounter >= 0)
				{
					--m_nDelayMessagesCounter;
					if (m_nDelayMessagesCounter <= 0)
					{
						m_nDelayMessagesCounter = 0;
						m_bDelayMessages = false;
					}
				}
			}
			DrxStringLocal message;
			std::vector<DrxStringLocal> unprocessedMessages;
			bool bSaveMessages = false;
			while (m_MessageQueue.try_pop_front(message))
			{
				// Can't process the message - have to save it and put it back after the queue has been drained
				if (bSaveMessages)
				{
					unprocessedMessages.push_back(message);
					continue;
				}
				// Just return the dump filename
				if (StrStartsWith(message, "getdumpfilename"))
				{
					DrxReplayInfo info {};
					DrxGetIMemReplay()->GetInfo(info);
					tukk pFilename = info.filename;
					if (!pFilename) { pFilename = "<unknown>"; }

					CStatsAgentPipe::Send(pFilename, "dumpfile", message.c_str());
				}
				// Get the value of a CVAR
				else if (tukk pCVARName = TryParse(message, "getcvarvalue "))
				{
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
				else if (tukk pCVARName = TryParse(message, "createcvar "))
				{
					REGISTER_STRING(pCVARName, "", 0, "Amble CVAR");

					CStatsAgentPipe::Send("finished", nullptr, message.c_str());
				}
				// Execute the command
				else if (tukk pCommand = TryParse(message, "exec "))
				{
					if (!m_bDelayMessages)
					{
						// Execute the rest of the string
						if (gEnv && gEnv->pConsole)
						{
							gEnv->pConsole->ExecuteString(pCommand);

							CStatsAgentPipe::Send("finished", nullptr, message.c_str());
						}
					}
					else
					{
						// Stop processing the message queue - store the queue and restore it
						bSaveMessages = true;
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
			m_MessageQueue.append(unprocessedMessages.begin(), unprocessedMessages.end());
		}
	}

	void ThreadReceiveMessage()
	{
		if (CStatsAgentPipe::IsPipeOpen())
		{
			tukk szMessage = CStatsAgentPipe::Receive();
			if (szMessage != nullptr && strlen(szMessage) != 0)
			{
				m_MessageQueue.push_back(szMessage);
			}
		}
	}
};

static CStatsAgentImpl* g_pStatsAgentImpl = nullptr;

void CStatsAgentImpl::CThread::ThreadEntry()
{
	while (!m_bQuit)
	{
		m_StatsAgentImpl.ThreadReceiveMessage();
		DrxSleep(1000);
	}
}

void CStatsAgent::CreatePipe(const ICmdLineArg* pPipeName)
{
	DRX_ASSERT(pPipeName != nullptr);
	DRX_ASSERT(g_pStatsAgentImpl == nullptr);

	DrxLogAlways("CStatsAgent::CreatePipe: pPipeName = %s", pPipeName->GetValue());
	string sPipeName = pPipeName->GetValue();
	sPipeName.Trim();//Trimming is essential as command line arg value may contain spaces at the end
	g_pStatsAgentImpl = new CStatsAgentImpl(sPipeName.c_str());
}

void CStatsAgent::ClosePipe()
{
	if (g_pStatsAgentImpl != nullptr)
	{
		DrxLogAlways("CStatsAgent: ClosePipe");
		delete g_pStatsAgentImpl;
		g_pStatsAgentImpl = nullptr;
	}
}

void CStatsAgent::Update()
{
	if (g_pStatsAgentImpl != nullptr)
		g_pStatsAgentImpl->Update();
}

void CStatsAgent::SetDelayTimeout(i32k timeout)
{
	DrxLogAlways("CStatsAgent: SetDelayTimeout %d", timeout);
	if (g_pStatsAgentImpl != nullptr)
		g_pStatsAgentImpl->SetDelayTimeout(timeout);
}

void CStatsAgent::SetDelayMessages(bool enable)
{
	DrxLogAlways("CStatsAgent: SetDelayMessages %d", enable);
	if (g_pStatsAgentImpl != nullptr)
		g_pStatsAgentImpl->SetDelayMessages(enable);
}

#endif  // defined(ENABLE_STATS_AGENT)
