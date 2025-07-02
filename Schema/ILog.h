// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/ICore.h>
#include <drx3D/Schema/LogMetaData.h>
#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/PreprocessorUtils.h>
#include <drx3D/Schema/Signal.h>

#ifndef SXEMA_LOGGING_ENABLED
    #ifdef _RELEASE
        #define SXEMA_LOGGING_ENABLED 0
    #else
        #define SXEMA_LOGGING_ENABLED 1
    #endif
#endif

#if SXEMA_LOGGING_ENABLED

    #define SXEMA_LOG_SCOPE(metaData)        const sxema::SLogScope schematycLogScope(metaData)
    #define SXEMA_COMMENT(streamId, ...)     gEnv->pSchematyc->GetLog().Comment(streamId, __VA_ARGS__)
    #define SXEMA_WARNING(streamId, ...)     gEnv->pSchematyc->GetLog().Warning(streamId, __VA_ARGS__)
    #define SXEMA_ERROR(streamId, ...)       gEnv->pSchematyc->GetLog().Error(streamId, __VA_ARGS__)
    #define SXEMA_FATAL_ERROR(streamId, ...) gEnv->pSchematyc->GetLog().FatalError(streamId, __VA_ARGS__); DrxFatalError(__VA_ARGS__)

    #define SXEMA_CRITICAL_ERROR(streamId, ...)                                \
      {                                                                            \
        static bool bIgnore = false;                                               \
        if (!bIgnore)                                                              \
        {                                                                          \
          switch (gEnv->pSchematyc->GetLog().CriticalError(streamId, __VA_ARGS__)) \
          {                                                                        \
          case sxema::ECriticalErrorStatus::Break:                             \
            {                                                                      \
              SXEMA_DEBUG_BREAK;                                               \
              break;                                                               \
            }                                                                      \
          case sxema::ECriticalErrorStatus::Ignore:                            \
            {                                                                      \
              bIgnore = true;                                                      \
              break;                                                               \
            }                                                                      \
          case sxema::ECriticalErrorStatus::Continue:                          \
             {                                                                     \
                        bIgnore = false;                                           \
              break;                                                               \
             }                                                                     \
          }                                                                        \
        }                                                                          \
      }

    #define SXEMA_CORE_COMMENT(...)            SXEMA_COMMENT(sxema::LogStreamId::Core, __VA_ARGS__)
    #define SXEMA_CORE_WARNING(...)            SXEMA_WARNING(sxema::LogStreamId::Core, __VA_ARGS__)
    #define SXEMA_CORE_ERROR(...)              SXEMA_ERROR(sxema::LogStreamId::Core, __VA_ARGS__)
    #define SXEMA_CORE_CRITICAL_ERROR(...)     SXEMA_CRITICAL_ERROR(sxema::LogStreamId::Core, __VA_ARGS__)
    #define SXEMA_CORE_FATAL_ERROR(...)        SXEMA_FATAL_ERROR(sxema::LogStreamId::Core, __VA_ARGS__)

    #define SXEMA_COMPILER_COMMENT(...)        SXEMA_COMMENT(sxema::LogStreamId::Compiler, __VA_ARGS__)
    #define SXEMA_COMPILER_WARNING(...)        SXEMA_WARNING(sxema::LogStreamId::Compiler, __VA_ARGS__)
    #define SXEMA_COMPILER_ERROR(...)          SXEMA_ERROR(sxema::LogStreamId::Compiler, __VA_ARGS__)
    #define SXEMA_COMPILER_CRITICAL_ERROR(...) SXEMA_CRITICAL_ERROR(sxema::LogStreamId::Compiler, __VA_ARGS__)
    #define SXEMA_COMPILER_FATAL_ERROR(...)    SXEMA_FATAL_ERROR(sxema::LogStreamId::Compiler, __VA_ARGS__)

    #define SXEMA_EDITOR_COMMENT(...)          SXEMA_COMMENT(sxema::LogStreamId::Editor, __VA_ARGS__)
    #define SXEMA_EDITOR_WARNING(...)          SXEMA_WARNING(sxema::LogStreamId::Editor, __VA_ARGS__)
    #define SXEMA_EDITOR_ERROR(...)            SXEMA_ERROR(sxema::LogStreamId::Editor, __VA_ARGS__)
    #define SXEMA_EDITOR_CRITICAL_ERROR(...)   SXEMA_CRITICAL_ERROR(sxema::LogStreamId::Editor, __VA_ARGS__)
    #define SXEMA_EDITOR_FATAL_ERROR(...)      SXEMA_FATAL_ERROR(sxema::LogStreamId::Editor, __VA_ARGS__)

    #define SXEMA_ENV_COMMENT(...)             SXEMA_COMMENT(sxema::LogStreamId::Env, __VA_ARGS__)
    #define SXEMA_ENV_WARNING(...)             SXEMA_WARNING(sxema::LogStreamId::Env, __VA_ARGS__)
    #define SXEMA_ENV_ERROR(...)               SXEMA_ERROR(sxema::LogStreamId::Env, __VA_ARGS__)
    #define SXEMA_ENV_CRITICAL_ERROR(...)      SXEMA_CRITICAL_ERROR(sxema::LogStreamId::Env, __VA_ARGS__)
    #define SXEMA_ENV_FATAL_ERROR(...)         SXEMA_FATAL_ERROR(sxema::LogStreamId::Env, __VA_ARGS__)

#else

    #define SXEMA_LOG_SCOPE(metaData)           SXEMA_NOP
    #define SXEMA_COMMENT(streamId, ...)        SXEMA_NOP
    #define SXEMA_WARNING(streamId, ...)        SXEMA_NOP
    #define SXEMA_ERROR(streamId, ...)          SXEMA_NOP
    #define SXEMA_FATAL_ERROR(streamId, ...)    SXEMA_NOP

    #define SXEMA_CRITICAL_ERROR(streamId, ...) SXEMA_NOP

    #define SXEMA_CORE_COMMENT(...)             SXEMA_NOP
    #define SXEMA_CORE_WARNING(...)             SXEMA_NOP
    #define SXEMA_CORE_ERROR(...)               SXEMA_NOP
    #define SXEMA_CORE_CRITICAL_ERROR(...)      SXEMA_NOP
    #define SXEMA_CORE_FATAL_ERROR(...)         SXEMA_NOP

    #define SXEMA_COMPILER_COMMENT(...)         SXEMA_NOP
    #define SXEMA_COMPILER_WARNING(...)         SXEMA_NOP
    #define SXEMA_COMPILER_ERROR(...)           SXEMA_NOP
    #define SXEMA_COMPILER_CRITICAL_ERROR(...)  SXEMA_NOP
    #define SXEMA_COMPILER_FATAL_ERROR(...)     SXEMA_NOP

    #define SXEMA_EDITOR_COMMENT(...)           SXEMA_NOP
    #define SXEMA_EDITOR_WARNING(...)           SXEMA_NOP
    #define SXEMA_EDITOR_ERROR(...)             SXEMA_NOP
    #define SXEMA_EDITOR_CRITICAL_ERROR(...)    SXEMA_NOP
    #define SXEMA_EDITOR_FATAL_ERROR(...)       SXEMA_NOP

    #define SXEMA_ENV_COMMENT(...)              SXEMA_NOP
    #define SXEMA_ENV_WARNING(...)              SXEMA_NOP
    #define SXEMA_ENV_ERROR(...)                SXEMA_NOP
    #define SXEMA_ENV_CRITICAL_ERROR(...)       SXEMA_NOP
    #define SXEMA_ENV_FATAL_ERROR(...)          SXEMA_NOP

#endif

namespace sxema
{
// Forward declare structures.
struct SLogScope;

enum class ELogMessageType
{
    Comment,
    Warning,
    Error,
    CriticalError,
    FatalError,
    Invalid
};

enum class ECriticalErrorStatus
{
    Break,
    Continue,
    Ignore
};

enum class LogStreamId : u32
{
    Invalid = 0,
    Default,
    Core,
    Compiler,
    Editor,
    Env,
    Custom
};

inline bool Serialize(Serialization::IArchive& archive, LogStreamId& value, tukk szName, tukk szLabel)
{
    if (!archive.isEdit())
    {
        return archive(static_cast<u32>(value), szName, szLabel);
    }
    return true;
}

struct SLogMessageData
{
    explicit inline SLogMessageData(const CLogMetaData* _pMetaData, ELogMessageType _messageType, LogStreamId _streamId, tukk _szMessage)
        : pMetaData(_pMetaData)
        , messageType(_messageType)
        , streamId(_streamId)
        , szMessage(_szMessage)
    {}

    const CLogMetaData* pMetaData;
    ELogMessageType     messageType;
    LogStreamId         streamId;
    tukk         szMessage;
};

typedef std::function<EVisitStatus(tukk , LogStreamId)> LogStreamVisitor;

struct ILogOutput
{
    virtual ~ILogOutput() {}

    virtual void EnableStream(LogStreamId streamId) = 0;
    virtual void DisableAllStreams() = 0;
    virtual void EnableMessageType(ELogMessageType messageType) = 0;
    virtual void DisableAllMessageTypes() = 0;
    virtual void Update() = 0;
};

DECLARE_SHARED_POINTERS(ILogOutput)

typedef CSignal<void (const SLogMessageData&)> LogMessageSignal;

struct ILog
{
    virtual ~ILog() {}

    virtual ELogMessageType          GetMessageType(tukk szMessageType) = 0;
    virtual tukk              GetMessageTypeName(ELogMessageType messageType) = 0;

    virtual LogStreamId              CreateStream(tukk szName, LogStreamId staticStreamId = LogStreamId::Invalid) = 0;
    virtual void                     DestroyStream(LogStreamId streamId) = 0;
    virtual LogStreamId              GetStreamId(tukk szName) const = 0;
    virtual tukk              GetStreamName(LogStreamId streamId) const = 0;
    virtual void                     VisitStreams(const LogStreamVisitor& visitor) const = 0;

    virtual ILogOutputPtr            CreateFileOutput(tukk szFileName) = 0;

    virtual void                     PushScope(SLogScope* pScope) = 0;
    virtual void                     PopScope(SLogScope* pScope) = 0;

    virtual void                     Comment(LogStreamId streamId, tukk szFormat, va_list va_args) = 0;
    virtual void                     Warning(LogStreamId streamId, tukk szFormat, va_list va_args) = 0;
    virtual void                     Error(LogStreamId streamId, tukk szFormat, va_list va_args) = 0;
    virtual ECriticalErrorStatus     CriticalError(LogStreamId streamId, tukk szFormat, va_list va_args) = 0;
    virtual void                     FatalError(LogStreamId streamId, tukk szFormat, va_list va_args) = 0;

    virtual void                     Update() = 0;
    virtual LogMessageSignal::Slots& GetMessageSignalSlots() = 0;

    inline void                      Comment(LogStreamId streamId, tukk szFormat, ...)
    {
        va_list va_args;
        va_start(va_args, szFormat);
        Comment(streamId, szFormat, va_args);
        va_end(va_args);
    }

    inline void Warning(LogStreamId streamId, tukk szFormat, ...)
    {
        va_list va_args;
        va_start(va_args, szFormat);
        Warning(streamId, szFormat, va_args);
        va_end(va_args);
    }

    inline void Error(LogStreamId streamId, tukk szFormat, ...)
    {
        va_list va_args;
        va_start(va_args, szFormat);
        Error(streamId, szFormat, va_args);
        va_end(va_args);
    }

    inline ECriticalErrorStatus CriticalError(LogStreamId streamId, tukk szFormat, ...)
    {
        va_list va_args;
        va_start(va_args, szFormat);
        const ECriticalErrorStatus result = CriticalError(streamId, szFormat, va_args);
        va_end(va_args);
        return result;
    }

    inline void FatalError(LogStreamId streamId, tukk szFormat, ...)
    {
        va_list va_args;
        va_start(va_args, szFormat);
        FatalError(streamId, szFormat, va_args);
        va_end(va_args);
    }
};

struct SLogScope
{
public:

    inline SLogScope(const CLogMetaData& _metaData)
        : metaData(_metaData)
        , warningCount(0)
        , errorCount(0)
    {
        gEnv->pSchematyc->GetLog().PushScope(this);
    }

    inline ~SLogScope()
    {
        gEnv->pSchematyc->GetLog().PopScope(this);
    }

    const CLogMetaData& metaData;
    u32              warningCount;
    u32              errorCount;
};
} // sxema
