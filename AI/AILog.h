// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   AILog.h
   $Id$
   Описание:

   -------------------------------------------------------------------------
   История:
   - ?
   - 2 Mar 2009			 : Evgeny Adamenkov: Removed IRenderer

 *********************************************************************/
#ifndef AILOG_H
#define AILOG_H

#if _MSC_VER > 1000
	#pragma once
#endif

#ifndef _RELEASE
	#define ENABLE_AI_ASSERT
#endif

/// Asserts that a condition is true. Can be enabled/disabled at compile time (will be enabled
/// for testing). Can also be enabled/disabled at run time (if enabled at compile time), so
/// code below this should handle the condition = false case. It should be used to trap
/// logical/programming errors, NOT data/script errors - i.e. once the code has been tested and
/// debugged, it should never get hit, whatever changes are made to data/script
#ifdef ENABLE_AI_ASSERT
	#define AIAssert(exp) DRX_ASSERT(exp)
#else
	#define AIAssert(exp) ((void)0)
#endif

/// Default message verbosity levels based on type
enum AI_LOG_VERBOSITY
{
	AI_LOG_OFF      = 0,
	AI_LOG_ERROR    = 1,
	AI_LOG_WARNING  = 1,
	AI_LOG_PROGRESS = 1,
	AI_LOG_EVENT    = 2,
	AI_LOG_COMMENT  = 3,
};

#ifndef DRXAISYS_VERBOSITY
	#define AIInitLog       (void)
	#define AIWarning       (void)
	#define AIError         (void)
	#define AILogProgress   (void)
	#define AILogEvent      (void)
	#define AILogComment    (void)
	#define AILogAlways     (void)
	#define AILogLoading    (void)
	#define AIWarningID     (void)
	#define AIErrorID       (void)
	#define AILogProgressID (void)
	#define AILogEventID    (void)
	#define AILogCommentID  (void)
#else
	#define AIWarningID     GetAISystem()->Warning
	#define AIErrorID       GetAISystem()->Error
	#define AILogProgressID GetAISystem()->LogProgress
	#define AILogEventID    GetAISystem()->LogEvent
	#define AILogCommentID  GetAISystem()->LogComment
#endif

#ifdef DRXAISYS_VERBOSITY

/// Sets up console variables etc
void AIInitLog(struct ISystem* system);

/// Return the verbosity for the console output
i32 AIGetLogConsoleVerbosity();

/// Return the verbosity for the file output
i32 AIGetLogFileVerbosity();

/// Check the verbosity level for file/console output
bool AICheckLogVerbosity(const AI_LOG_VERBOSITY CheckVerbosity);

/// Indicates if AI warnings/errors are enabled (e.g. for validation code you
/// don't want to run if there would be no output)
bool AIGetWarningErrorsEnabled();

/// Reports an AI Warning. This should be used when AI gets passed data from outside that
/// is "bad", but we can handle it and it shouldn't cause serious problems.
void AIWarning(tukk format, ...) PRINTF_PARAMS(1, 2);

/// Reports an AI Error. This should be used when AI either gets passed data that is so
/// bad that we can't handle it, or we want to detect/check against a logical/programming
/// error and be sure this check isn't ever compiled out (like an assert might be). Unless
/// in performance critical code, prefer a manual test with AIError to using AIAssert.
/// Note that in the editor this will return (so problems can be fixed). In game it will cause
/// execution to halt.
void AIError(tukk format, ...) PRINTF_PARAMS(1, 2);

/// Used to log progress points - e.g. startup/shutdown/loading of AI. With the default (for testers etc)
/// verbosity settings this will write to the log file and the console.
void AILogProgress(tukk format, ...) PRINTF_PARAMS(1, 2);

/// Used to log significant events like AI state changes. With the default (for testers etc)
/// verbosity settings this will write to the log file, but not the console.

void AILogEvent(tukk format, ...) PRINTF_PARAMS(1, 2);

/// Used to log events like that AI people might be interested in during their private testing.
/// With the default (for testers etc) verbosity settings this will not write to the log file
/// or the console.
void AILogComment(tukk format, ...) PRINTF_PARAMS(1, 2);

/// Should probably never be used since it bypasses verbosity checks
void AILogAlways(tukk format, ...) PRINTF_PARAMS(1, 2);

/// Displays messages during loading - also gives an opportunity for system/display to update
void AILogLoading(tukk format, ...) PRINTF_PARAMS(1, 2);

/// Overlays the saved messages - called from CAISystem::DebugDraw
void AILogDisplaySavedMsgs();

#endif // DRXAISYS_NO_VERBOSITY

#endif // file included
