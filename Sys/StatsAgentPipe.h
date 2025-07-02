// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//! Wrapper around windows pipe. XB1 (Durango) in theory is supported, but this needs verification.
class CStatsAgentPipe
{
public:
	static bool OpenPipe(tukk szPipeName);
	static void ClosePipe();

	static bool Send(tukk szMessage, tukk szPrefix = 0, tukk szDebugTag = 0);
	static tukk Receive();

	static bool IsPipeOpen();

private:
	CStatsAgentPipe(); // prevent instantiation

	static bool CreatePipe(tukk szName);
};
