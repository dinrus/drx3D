// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class ICmdLineArg;

//! Provides utilities for inter-process communication using pipe
class CStatsAgent
{
public:
	static void CreatePipe(const ICmdLineArg* pPipeName);
	static void ClosePipe(void);
	static void Update(void);

	static void SetDelayMessages(bool enable);
	static void SetDelayTimeout(i32k timeout);

	CStatsAgent() = delete;
};
