// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/ILog.h>
#include <drx3D/Schema/PreprocessorUtils.h>

#ifndef SXEMA_ASSERTS_ENABLED
	#ifdef _RELEASE
		#define SXEMA_ASSERTS_ENABLED 0
	#else
		#define SXEMA_ASSERTS_ENABLED 1
	#endif
#endif

#if SXEMA_ASSERTS_ENABLED

	#define SXEMA_ASSERT(streamId, expression)                    DRX_ASSERT((expression)); if (!((expression))) { SXEMA_CRITICAL_ERROR(streamId, ("ASSERT" # expression)); }
	#define SXEMA_ASSERT_MESSAGE(streamId, expression, ...)       DRX_ASSERT_MESSAGE((expression),__VA_ARGS__); if (!((expression))) { SXEMA_CRITICAL_ERROR(streamId, __VA_ARGS__); }
	#define SXEMA_ASSERT_FATAL(streamId, expression)              DRX_ASSERT((expression)); if (!((expression))) { SXEMA_FATAL_ERROR(streamId, ("ASSERT" # expression)); }
	#define SXEMA_ASSERT_FATAL_MESSAGE(streamId, expression, ...) DRX_ASSERT_MESSAGE((expression),__VA_ARGS__); if (!((expression))) { SXEMA_FATAL_ERROR(streamId, __VA_ARGS__); }

	#define SXEMA_CORE_ASSERT(expression)                         SXEMA_ASSERT(sxema::LogStreamId::Core, (expression))
	#define SXEMA_CORE_ASSERT_MESSAGE(expression, ...)            SXEMA_ASSERT_MESSAGE(sxema::LogStreamId::Core, (expression), __VA_ARGS__)
	#define SXEMA_CORE_ASSERT_FATAL(expression)                   SXEMA_ASSERT_FATAL(sxema::LogStreamId::Core, (expression))
	#define SXEMA_CORE_ASSERT_FATAL_MESSAGE(expression, ...)      SXEMA_CORE_ASSERT_FATAL_MESSAGE(sxema::LogStreamId::Core,, (expression), __VA_ARGS__)

	#define SXEMA_COMPILER_ASSERT(expression)                     SXEMA_ASSERT(sxema::LogStreamId::Compiler, (expression))
	#define SXEMA_COMPILER_ASSERT_MESSAGE(expression, ...)        SXEMA_ASSERT_MESSAGE(sxema::LogStreamId::Compiler, (expression), __VA_ARGS__)
	#define SXEMA_COMPILER_ASSERT_FATAL(expression)               SXEMA_ASSERT_FATAL(sxema::LogStreamId::Compiler, (expression))
	#define SXEMA_COMPILER_ASSERT_FATAL_MESSAGE(expression, ...)  SXEMA_CORE_ASSERT_FATAL_MESSAGE(sxema::LogStreamId::Compiler, (expression), __VA_ARGS__)

	#define SXEMA_EDITOR_ASSERT(expression)                       SXEMA_ASSERT(sxema::LogStreamId::Editor, (expression))
	#define SXEMA_EDITOR_ASSERT_MESSAGE(expression, ...)          SXEMA_ASSERT_MESSAGE(sxema::LogStreamId::Editor, (expression), __VA_ARGS__)
	#define SXEMA_EDITOR_ASSERT_FATAL(expression)                 SXEMA_ASSERT_FATAL(sxema::LogStreamId::Editor, (expression))
	#define SXEMA_EDITOR_ASSERT_FATAL_MESSAGE(expression, ...)    SXEMA_CORE_ASSERT_FATAL_MESSAGE(sxema::LogStreamId::Editor, (expression), __VA_ARGS__)

	#define SXEMA_ENV_ASSERT(expression)                          SXEMA_ASSERT(sxema::LogStreamId::Env, (expression))
	#define SXEMA_ENV_ASSERT_MESSAGE(expression, ...)             SXEMA_ASSERT_MESSAGE(sxema::LogStreamId::Env, (expression), __VA_ARGS__)
	#define SXEMA_ENV_ASSERT_FATAL(expression)                    SXEMA_ASSERT_FATAL(sxema::LogStreamId::Env, (expression))
	#define SXEMA_ENV_ASSERT_FATAL_MESSAGE(expression, ...)       SXEMA_CORE_ASSERT_FATAL_MESSAGE(sxema::LogStreamId::Env, (expression), __VA_ARGS__)

#else

	#define SXEMA_ASSERT(streamId, expression)                    SXEMA_NOP;
	#define SXEMA_ASSERT_MESSAGE(streamId, expression, ...)       SXEMA_NOP;
	#define SXEMA_ASSERT_FATAL(streamId, expression)              SXEMA_NOP;
	#define SXEMA_ASSERT_FATAL_MESSAGE(streamId, expression, ...) SXEMA_NOP;

	#define SXEMA_CORE_ASSERT(expression)                         SXEMA_NOP;
	#define SXEMA_CORE_ASSERT_MESSAGE(expression, ...)            SXEMA_NOP;
	#define SXEMA_CORE_ASSERT_FATAL(expression)                   SXEMA_NOP;
	#define SXEMA_CORE_ASSERT_FATAL_MESSAGE(expression, ...)      SXEMA_NOP;

	#define SXEMA_COMPILER_ASSERT(expression)                     SXEMA_NOP;
	#define SXEMA_COMPILER_ASSERT_MESSAGE(expression, ...)        SXEMA_NOP;
	#define SXEMA_COMPILER_ASSERT_FATAL(expression)               SXEMA_NOP;
	#define SXEMA_COMPILER_ASSERT_FATAL_MESSAGE(expression, ...)  SXEMA_NOP;

	#define SXEMA_EDITOR_ASSERT(expression)                       SXEMA_NOP;
	#define SXEMA_EDITOR_ASSERT_MESSAGE(expression, ...)          SXEMA_NOP;
	#define SXEMA_EDITOR_ASSERT_FATAL(expression)                 SXEMA_NOP;
	#define SXEMA_EDITOR_ASSERT_FATAL_MESSAGE(expression, ...)    SXEMA_NOP;

	#define SXEMA_ENV_ASSERT(expression)                          SXEMA_NOP;
	#define SXEMA_ENV_ASSERT_MESSAGE(expression, ...)             SXEMA_NOP;
	#define SXEMA_ENV_ASSERT_FATAL(expression)                    SXEMA_NOP;
	#define SXEMA_ENV_ASSERT_FATAL_MESSAGE(expression, ...)       SXEMA_NOP;

#endif
