// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETLOG_H__
#define __NETLOG_H__

#pragma once

#include <drx3D/Sys/ILog.h>
#include <drx3D/Network/Config.h>

extern threadID g_primaryThread;

ILINE bool IsPrimaryThread()
{
	return g_primaryThread == DrxGetCurrentThreadId();
}

#if !defined(EXCLUDE_NORMAL_LOG)

void FlushNetLog(bool final);
void InitPrimaryThread();
void NetLog(tukk fmt, ...) PRINTF_PARAMS(1, 2); // DrxLog
void VNetLog(tukk pFmt, va_list args);          // DrxLog
void NetLogHUD(tukk fmt, ...) PRINTF_PARAMS(1, 2);
// only in response to user input!
void NetLogAlways(tukk fmt, ...) PRINTF_PARAMS(1, 2);        // DrxLogAlways
void NetLogAlways_Secret(tukk fmt, ...) PRINTF_PARAMS(1, 2); // DrxLogAlways
void NetError(tukk fmt, ...) PRINTF_PARAMS(1, 2);            // DrxFatalError
void NetWarning(tukk fmt, ...) PRINTF_PARAMS(1, 2);          // DrxWarning
void NetQuickLog(bool toConsole, float timeout, tukk fmt, ...) PRINTF_PARAMS(3, 4);
void NetPerformanceWarning(tukk fmt, ...) PRINTF_PARAMS(1, 2);
void NetComment(tukk fmt, ...) PRINTF_PARAMS(1, 2);    // DrxComment

	#if ENABLE_CORRUPT_PACKET_DUMP
void NetLogPacketDebug(tukk fmt, ...) PRINTF_PARAMS(1, 2);
	#else
		#define NetLogPacketDebug(...)
	#endif

string GetBacklogAsString();
void   ParseBacklogString(const string& backlog, string& base, std::vector<string>& r);

#else

	#define FlushNetLog(...)
	#define InitPrimaryThread(...)
	#define NetLog(...)
	#define VNetLog(...)
	#define NetLogHUD(...)
	#define NetLogAlways(...)
	#define NetLogAlways_Secret(...)
	#define NetError(...)
	#define NetWarning(...)
	#define NetQuickLog(...)
	#define NetPerformanceWarning(...)
	#define NetComment(...)

	#define NetLogPacketDebug(...)

	#define ParseBacklogString(...)

#endif // !defined(EXCLUDE_NORMAL_LOG)

#ifndef _DEBUG
	#define ASSERT_PRIMARY_THREAD
#else
void AssertPrimaryThread();
	#define ASSERT_PRIMARY_THREAD AssertPrimaryThread()
#endif

#endif
