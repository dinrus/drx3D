// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

#if defined(ENABLE_STATS_AGENT)

#include <drx3D/CoreX/Project/ProjectDefines.h>
#include <drx3D/Sys/StatsAgentPipe.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

namespace
{
	i32k BUFFER_SIZE = 1024;

	DrxFixedStringT<BUFFER_SIZE> s_pCommandBuffer;
	bool s_pipeOpen = false;

#if DRX_PLATFORM_DURANGO //FIXME ?
	constexpr char kPipeBaseName[] = "";
	 bool s_commandWaiting = false;
#elif DRX_PLATFORM_WINDOWS
	constexpr char kPipeBaseName[] = R"(\\.\pipe\DrxsisTargetComms)";
	HANDLE s_pipe = INVALID_HANDLE_VALUE;
#endif
};

static i32 statsagent_debug = 0;

bool CStatsAgentPipe::IsPipeOpen()
{
	return s_pipeOpen;
}

bool CStatsAgentPipe::OpenPipe(tukk szPipeName)
{
	REGISTER_CVAR(statsagent_debug, 0, 0, "Enable/Disable StatsAgent debug messages");

	// Construct the pipe name
	string sPipeFullName;
	sPipeFullName.Format("%s%s.pipe", kPipeBaseName, szPipeName);

	CreatePipe(sPipeFullName.c_str());

	if (statsagent_debug && s_pipeOpen)
	{
		DrxLogAlways("CStatsAgent: Pipe connection \"%s\" is open", sPipeFullName.c_str());
	}

	if (s_pipeOpen)
	{
		if (!Send("connected"))
			ClosePipe();
	}
	else
	{
		if (statsagent_debug)
			DrxLogAlways("CStatsAgent: Unable to connect pipe %s", sPipeFullName.c_str());
	}
	return s_pipeOpen;
}

bool CStatsAgentPipe::CreatePipe(tukk szName)
{
#if DRX_PLATFORM_WINDOWS
	s_pipe = ::CreateFile(szName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	DWORD dwMode = PIPE_NOWAIT;
	if (s_pipe != INVALID_HANDLE_VALUE)
	{
		DrxLogAlways("Pipe %s created", szName);
		s_pipeOpen = ::SetNamedPipeHandleState(s_pipe, &dwMode, NULL, NULL) == TRUE;
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed to create pipe %s, error: 0x%X", szName, GetLastError());
	}
#endif

	return s_pipeOpen;
}

void CStatsAgentPipe::ClosePipe()
{
	if (s_pipeOpen)
	{
#if DRX_PLATFORM_WINDOWS 
		::CloseHandle(s_pipe);
		s_pipe = INVALID_HANDLE_VALUE;
#endif
		s_pipeOpen = false;
	}
}

bool CStatsAgentPipe::Send(tukk szMessage, tukk szPrefix, tukk szDebugTag)
{
	DrxFixedStringT<BUFFER_SIZE> pBuffer;
	if (szPrefix)
	{
		pBuffer = szPrefix;
		pBuffer.Trim();
		pBuffer += " ";
	}
	pBuffer += szMessage;

	bool bSuccess = true;
	u32 nBytes = pBuffer.size() + 1;

	if (statsagent_debug)
	{
		if (szDebugTag)
		{
			DrxLogAlways("CStatsAgent: Sending message \"%s\" [%s]", pBuffer.c_str(), szDebugTag);
		}
		else
		{
			DrxLogAlways("CStatsAgent: Sending message \"%s\"", pBuffer.c_str());
		}
	}

#if DRX_PLATFORM_WINDOWS
	DWORD tx;
	bSuccess = ::WriteFile(s_pipe, pBuffer.c_str(), nBytes, &tx, 0) == TRUE;
#endif

	if (statsagent_debug && !bSuccess)
		DrxLogAlways("CStatsAgent: Unable to write to pipe");

	return bSuccess;
}

tukk CStatsAgentPipe::Receive()
{
	tukk szResult = NULL;

#if DRX_PLATFORM_WINDOWS
	DWORD size;
	if (::ReadFile(s_pipe, s_pCommandBuffer.m_strBuf, BUFFER_SIZE - 1, &size, 0) && size > 0)
	{
		s_pCommandBuffer.m_strBuf[size] = '\0';
		s_pCommandBuffer.TrimRight('\n');
		szResult = s_pCommandBuffer.c_str();
	}
#endif

	if (statsagent_debug && szResult)
		DrxLogAlways("CStatsAgent: Received message \"%s\"", szResult);

	return szResult;
}

#endif	// defined(ENABLE_STATS_AGENT)
