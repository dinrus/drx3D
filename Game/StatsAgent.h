// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   StatsAgent.h
//  Version:     v1.00
//  Created:     06/10/2009 by Steve Barnett.
//  Описание: This the declaration for CStatsAgent
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __STATS_AGENT_H__
#define __STATS_AGENT_H__

#pragma once

class ICmdLineArg;

class CStatsAgent
{
public:
	static void CreatePipe( const ICmdLineArg* pPipeName );
	static void ClosePipe( void );
	static void Update( void );

	static void SetDelayMessages(bool enable);
	static void SetDelayTimeout(i32k timeout);
private:
	static bool s_delayMessages;
	static i32 s_iDelayMessagesCounter;
	CStatsAgent( void ) {} // Prevent instantiation
};

#endif
