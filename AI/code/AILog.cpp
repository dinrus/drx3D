// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   AILog.cpp
   $Id$
   Описание:

   -------------------------------------------------------------------------
   История:
   - ?
   - 2 Mar 2009	: Evgeny Adamenkov: Replaced IRenderer with CDebugDrawContext

 *********************************************************************/
#include <drx3D/AI/StdAfx.h>

#ifdef DRXAISYS_VERBOSITY

	#include <drx3D/AI/AILog.h>
	#include <drx3D/AI/CAISystem.h>
	#include <drx3D/AI/DrxAISys.h>

	#include <drx3D/Sys/ISystem.h>
	#include <drx3D/Sys/ITimer.h>
	#include <drx3D/Sys/IValidator.h>
	#include <drx3D/Sys/IConsole.h>
	#include <drx3D/AI/DebugDrawContext.h>

// these should all be in sync - so testing one for 0 should be the same for all
ISystem* pSystem = 0;

static const char outputPrefix[] = "AI: ";
static const unsigned outputPrefixLen = sizeof(outputPrefix) - 1;

	#define DECL_OUTPUT_BUF char outputBufferLog[MAX_WARNING_LENGTH + outputPrefixLen];

static i32k maxSavedMsgs = 5;
static i32k maxSavedMsgLength = MAX_WARNING_LENGTH + outputPrefixLen + 1;
enum ESavedMsgType {SMT_WARNING, SMT_ERROR};
struct SSavedMsg
{
	ESavedMsgType savedMsgType;
	char          savedMsg[maxSavedMsgLength];
	CTimeValue    time;
};
static SSavedMsg savedMsgs[maxSavedMsgs];
static i32 savedMsgIndex = 0;

//====================================================================
// DebugDrawLabel
//====================================================================
static void DebugDrawLabel(ESavedMsgType type, float timeFrac, i32 col, i32 row, tukk szText)
{
	float ColumnSize = 11;
	float RowSize = 11;
	float baseY = 10;
	ColorB colorWarning(0, 255, 255);
	ColorB colorError(255, 255, 0);
	ColorB& color = (type == SMT_ERROR) ? colorError : colorWarning;
	CDebugDrawContext dc;

	float alpha = 1.0f;
	static float fadeFrac = 0.5f;
	if (timeFrac < fadeFrac && fadeFrac > 0.0f)
		alpha = timeFrac / fadeFrac;
	color.a = static_cast<u8>(255 * alpha);

	float actualCol = ColumnSize * static_cast<float>(col);
	float actualRow;
	if (row >= 0)
		actualRow = baseY + RowSize * static_cast<float>(row);
	else
		actualRow = dc->GetHeight() - (baseY + RowSize * static_cast<float>(-row));

	dc->Draw2dLabel(actualCol, actualRow, 1.2f, color, false, "%s", szText);
}

//====================================================================
// DisplaySavedMsgs
//====================================================================
void AILogDisplaySavedMsgs()
{
	float savedMsgDuration = gAIEnv.CVars.OverlayMessageDuration;
	if (savedMsgDuration < 0.01f)
		return;
	static i32 col = 1;

	i32 row = -1;
	CTimeValue currentTime = gEnv->pTimer->GetFrameStartTime();
	CTimeValue time = currentTime - CTimeValue(savedMsgDuration);
	for (i32 i = 0; i < maxSavedMsgs; ++i)
	{
		i32 index = (maxSavedMsgs + savedMsgIndex - i) % maxSavedMsgs;
		if (savedMsgs[index].time < time)
			return;
		// get rid of msgs from the future - can happen during load/save
		if (savedMsgs[index].time > currentTime)
			savedMsgs[index].time = time;
		//      savedMsgIndex = (maxSavedMsgs + savedMsgIndex - 1) % maxSavedMsgs;

		float timeFrac = (savedMsgs[index].time - time).GetSeconds() / savedMsgDuration;
		DebugDrawLabel(savedMsgs[index].savedMsgType, timeFrac, col, row, savedMsgs[index].savedMsg);
		--row;
	}
}

//====================================================================
// AIInitLog
//====================================================================
void AIInitLog(ISystem* system)
{
	if (pSystem)
		AIWarning("Re-registering AI Logging");

	AIAssert(system);
	if (!system)
		return;
	IConsole* console = gEnv->pConsole;
	#ifdef _DEBUG
	i32 isDebug = 1;
	#else
	i32 isDebug = 0;
	#endif

	if (console)
		pSystem = system;

	for (i32 i = 0; i < maxSavedMsgs; ++i)
	{
		savedMsgs[i].savedMsg[0] = '\0';
		savedMsgs[i].savedMsgType = SMT_WARNING;
		savedMsgs[i].time = CTimeValue(0.0f);
		savedMsgIndex = 0;
	}
}

//====================================================================
// AIGetLogConsoleVerbosity
//====================================================================
i32 AIGetLogConsoleVerbosity()
{
	return gAIEnv.CVars.LogConsoleVerbosity;
}

//====================================================================
// AIGetLogFileVerbosity
//====================================================================
i32 AIGetLogFileVerbosity()
{
	return gAIEnv.CVars.LogFileVerbosity;
}

//====================================================================
// AICheckLogVerbosity
//====================================================================
bool AICheckLogVerbosity(const AI_LOG_VERBOSITY CheckVerbosity)
{
	bool bResult = false;

	i32k iAILogVerbosity = AIGetLogFileVerbosity();
	i32k iAIConsoleVerbosity = AIGetLogConsoleVerbosity();

	if (iAILogVerbosity >= CheckVerbosity || iAIConsoleVerbosity >= CheckVerbosity)
	{
		// Check against actual log system
		i32k nVerbosity = gEnv->pLog->GetVerbosityLevel();
		bResult = (nVerbosity >= CheckVerbosity);
	}

	return bResult;
}

//===================================================================
// AIGetWarningErrorsEnabled
//===================================================================
bool AIGetWarningErrorsEnabled()
{
	return gAIEnv.CVars.EnableWarningsErrors != 0;
}

//====================================================================
// AIError
//====================================================================
void AIError(tukk format, ...)
{
	if (!pSystem || !AIGetWarningErrorsEnabled() || !AICheckLogVerbosity(AI_LOG_ERROR))
		return;

	DECL_OUTPUT_BUF;

	va_list args;
	va_start(args, format);
	drx_vsprintf(outputBufferLog, format, args);
	va_end(args);

	if (gEnv->IsEditor())
	{
		pSystem->Warning(VALIDATOR_MODULE_AI, VALIDATOR_ERROR, VALIDATOR_FLAG_AI, 0, "!AI: Error: %s", outputBufferLog);
	}
	else
	{
		gEnv->pLog->LogError("%s", outputBufferLog);
	}

	savedMsgIndex = (savedMsgIndex + 1) % maxSavedMsgs;
	savedMsgs[savedMsgIndex].savedMsgType = SMT_ERROR;
	drx_strcpy(savedMsgs[savedMsgIndex].savedMsg, outputBufferLog);
	savedMsgs[savedMsgIndex].time = gEnv->pTimer->GetFrameStartTime();
}

//====================================================================
// AIWarning
//====================================================================
void AIWarning(tukk format, ...)
{
	if (!pSystem || !AIGetWarningErrorsEnabled() || !AICheckLogVerbosity(AI_LOG_WARNING))
		return;

	DECL_OUTPUT_BUF;

	va_list args;
	va_start(args, format);
	drx_vsprintf(outputBufferLog, format, args);
	va_end(args);
	pSystem->Warning(VALIDATOR_MODULE_AI, VALIDATOR_WARNING, VALIDATOR_FLAG_AI, 0, "AI: %s", outputBufferLog);

	savedMsgIndex = (savedMsgIndex + 1) % maxSavedMsgs;
	savedMsgs[savedMsgIndex].savedMsgType = SMT_WARNING;
	drx_strcpy(savedMsgs[savedMsgIndex].savedMsg, outputBufferLog);
	savedMsgs[savedMsgIndex].time = gEnv->pTimer->GetFrameStartTime();
}

//====================================================================
// AILogAlways
//====================================================================
void AILogAlways(tukk format, ...)
{
	if (!pSystem)
		return;

	DECL_OUTPUT_BUF;

	memcpy(outputBufferLog, outputPrefix, outputPrefixLen);
	va_list args;
	va_start(args, format);
	drx_vsprintf(outputBufferLog + outputPrefixLen, sizeof(outputBufferLog) - outputPrefixLen, format, args);
	va_end(args);

	gEnv->pLog->Log("%s", outputBufferLog);
}

void AILogLoading(tukk format, ...)
{
	if (!pSystem)
		return;

	const char outputPrefix2[] = "--- AI: ";
	const unsigned outputPrefixLen2 = sizeof(outputPrefix2) - 1;

	DECL_OUTPUT_BUF;

	memcpy(outputBufferLog, outputPrefix2, outputPrefixLen2);
	va_list args;
	va_start(args, format);
	drx_vsprintf(outputBufferLog + outputPrefixLen2, sizeof(outputBufferLog) - outputPrefixLen2, format, args);
	va_end(args);

	gEnv->pLog->UpdateLoadingScreen("%s", outputBufferLog);
}

//====================================================================
// AIHandleLogMessage
//====================================================================
void AIHandleLogMessage(tukk outputBufferLog)
{
	i32k cV = AIGetLogConsoleVerbosity();
	i32k fV = AIGetLogFileVerbosity();

	if ((cV >= AI_LOG_PROGRESS) && (fV >= AI_LOG_PROGRESS))
		gEnv->pLog->Log("%s", outputBufferLog);
	else if (cV >= AI_LOG_PROGRESS)
		gEnv->pLog->LogToConsole("%s", outputBufferLog);
	else if (fV >= AI_LOG_PROGRESS)
		gEnv->pLog->LogToFile("%s", outputBufferLog);
}

//====================================================================
// AILogProgress
//====================================================================
void AILogProgress(tukk format, ...)
{
	if (!pSystem || !AICheckLogVerbosity(AI_LOG_PROGRESS))
		return;

	DECL_OUTPUT_BUF;

	memcpy(outputBufferLog, outputPrefix, outputPrefixLen);
	va_list args;
	va_start(args, format);
	drx_vsprintf(outputBufferLog + outputPrefixLen, sizeof(outputBufferLog) - outputPrefixLen, format, args);
	va_end(args);

	AIHandleLogMessage(outputBufferLog);
}

//====================================================================
// AILogEvent
//====================================================================
void AILogEvent(tukk format, ...)
{
	if (!pSystem || !AICheckLogVerbosity(AI_LOG_EVENT))
		return;

	DECL_OUTPUT_BUF;

	memcpy(outputBufferLog, outputPrefix, outputPrefixLen);
	va_list args;
	va_start(args, format);
	drx_vsprintf(outputBufferLog + outputPrefixLen, sizeof(outputBufferLog) - outputPrefixLen, format, args);
	va_end(args);

	AIHandleLogMessage(outputBufferLog);
}

//====================================================================
// AILogComment
//====================================================================
void AILogComment(tukk format, ...)
{
	if (!pSystem || !AICheckLogVerbosity(AI_LOG_COMMENT))
		return;

	DECL_OUTPUT_BUF;

	memcpy(outputBufferLog, outputPrefix, outputPrefixLen);
	va_list args;
	va_start(args, format);
	drx_vsprintf(outputBufferLog + outputPrefixLen, sizeof(outputBufferLog) - outputPrefixLen, format, args);
	va_end(args);

	AIHandleLogMessage(outputBufferLog);
}

#endif // DRXAISYS_VERBOSITY
