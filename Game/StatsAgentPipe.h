// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   StatsAgentPipe.h
//  Version:     v1.00
//  Created:     20/10/2011 by Sandy MacPherson
//  Описание: Wrapper around platform-dependent pipe comms
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __STATS_AGENT_PIPES_H__
#define __STATS_AGENT_PIPES_H__

#pragma once

class CStatsAgentPipe
{
public:
	static void OpenPipe(tukk szPipeName);
	static void ClosePipe();

	static bool Send(tukk szMessage, tukk szPrefix = 0, tukk szDebugTag = 0);
	static tukk Receive();

	static bool PipeOpen();

private:
	CStatsAgentPipe(); // prevent instantiation

	static bool CreatePipe(tukk szName);
};

#endif // __STATS_AGENT_PIPES_H__
