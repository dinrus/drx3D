// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Add support for indentation/blocks.
// #SchematycTODO : Move ILogRecorder to separate header?
// #SchematycTODO : Rename 'Game' stream to 'Env'?

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Sys/IDrxLink.h>
#include <drx3D/Schema2/TemplateUtils_Signalv2.h>

#include <drx3D/Schema2/GUID.h>
#include <drx3D/Schema2/IFramework.h>

#define SXEMA2_LOG_METAINFO(drxLinkCommand, ...)       sxema2::CLogMessageMetaInfo((drxLinkCommand), sxema2::SLogMetaFunction(__FUNCTION__), __VA_ARGS__)
#define SXEMA2_LOG_METAINFO_DEFAULT(drxLinkCommand)    sxema2::CLogMessageMetaInfo((drxLinkCommand), sxema2::SLogMetaFunction(__FUNCTION__))

#define SXEMA2_COMMENT(streamId, ...)                  gEnv->pSchematyc2->GetLog().Comment(streamId, SXEMA2_LOG_METAINFO_DEFAULT(sxema2::EDrxLinkCommand::None), __VA_ARGS__)
#define SXEMA2_ENTITY_COMMENT(streamId, entityId, ...) gEnv->pSchematyc2->GetLog().Comment(streamId, SXEMA2_LOG_METAINFO(sxema2::EDrxLinkCommand::None, sxema2::SLogMetaEntityId(entityId)), __VA_ARGS__)
#define SXEMA2_WARNING(streamId, ...)                  gEnv->pSchematyc2->GetLog().Warning(streamId, SXEMA2_LOG_METAINFO_DEFAULT(sxema2::EDrxLinkCommand::None), __VA_ARGS__)
#define SXEMA2_ENTITY_WARNING(streamId, entityId, ...) gEnv->pSchematyc2->GetLog().Warning(streamId, SXEMA2_LOG_METAINFO(sxema2::EDrxLinkCommand::None, sxema2::SLogMetaEntityId(entityId)), __VA_ARGS__)
#define SXEMA2_ERROR(streamId, ...)                    gEnv->pSchematyc2->GetLog().Error(streamId, SXEMA2_LOG_METAINFO_DEFAULT(sxema2::EDrxLinkCommand::None), __VA_ARGS__)
#define SXEMA2_ENTITY_ERROR(streamId, entityId, ...)   gEnv->pSchematyc2->GetLog().Error(streamId, SXEMA2_LOG_METAINFO(sxema2::EDrxLinkCommand::None, sxema2::SLogMetaEntityId(entityId)), __VA_ARGS__)
#define SXEMA2_FATAL_ERROR(streamId, ...)              gEnv->pSchematyc2->GetLog().FatalError(streamId, SXEMA2_LOG_METAINFO_DEFAULT(sxema2::EDrxLinkCommand::None), __VA_ARGS__); DrxFatalError(__VA_ARGS__)

#define SXEMA2_CRITICAL_ERROR(streamId, ...)                                                                                                    \
  {                                                                                                                                                 \
    static bool bIgnore = false;                                                                                                                    \
    if (!bIgnore)                                                                                                                                   \
    {                                                                                                                                               \
      switch (gEnv->pSchematyc2->GetLog().CriticalError(streamId, SXEMA2_LOG_METAINFO_DEFAULT(sxema2::EDrxLinkCommand::None), __VA_ARGS__)) \
      {                                                                                                                                             \
      case sxema2::ECriticalErrorStatus::Break:                                                                                                 \
        {                                                                                                                                           \
          SXEMA2_DEBUG_BREAK;                                                                                                                   \
          break;                                                                                                                                    \
        }                                                                                                                                           \
      case sxema2::ECriticalErrorStatus::Ignore:                                                                                                \
        {                                                                                                                                           \
          bIgnore = true;                                                                                                                           \
          break;                                                                                                                                    \
        }                                                                                                                                           \
      }                                                                                                                                             \
    }                                                                                                                                               \
  }

#define SXEMA2_SYSTEM_COMMENT(...)                           SXEMA2_COMMENT(sxema2::LOG_STREAM_SYSTEM, __VA_ARGS__)
#define SXEMA2_SYSTEM_WARNING(...)                           SXEMA2_WARNING(sxema2::LOG_STREAM_SYSTEM, __VA_ARGS__)
#define SXEMA2_SYSTEM_ERROR(...)                             SXEMA2_ERROR(sxema2::LOG_STREAM_SYSTEM, __VA_ARGS__)
#define SXEMA2_SYSTEM_CRITICAL_ERROR(...)                    SXEMA2_CRITICAL_ERROR(sxema2::LOG_STREAM_SYSTEM, __VA_ARGS__)
#define SXEMA2_SYSTEM_FATAL_ERROR(...)                       SXEMA2_FATAL_ERROR(sxema2::LOG_STREAM_SYSTEM, __VA_ARGS__)

#define SXEMA2_COMPILER_COMMENT(...)                         SXEMA2_COMMENT(sxema2::LOG_STREAM_COMPILER, __VA_ARGS__)
#define SXEMA2_COMPILER_WARNING(...)                         SXEMA2_WARNING(sxema2::LOG_STREAM_COMPILER, __VA_ARGS__)
#define SXEMA2_COMPILER_ERROR(...)                           SXEMA2_ERROR(sxema2::LOG_STREAM_COMPILER, __VA_ARGS__)
#define SXEMA2_COMPILER_CRITICAL_ERROR(...)                  SXEMA2_CRITICAL_ERROR(sxema2::LOG_STREAM_COMPILER, __VA_ARGS__)
#define SXEMA2_COMPILER_FATAL_ERROR(...)                     SXEMA2_FATAL_ERROR(sxema2::LOG_STREAM_COMPILER, __VA_ARGS__)

#define SXEMA2_GAME_COMMENT(...)                             SXEMA2_COMMENT(sxema2::LOG_STREAM_GAME, __VA_ARGS__)
#define SXEMA2_GAME_WARNING(...)                             SXEMA2_WARNING(sxema2::LOG_STREAM_GAME, __VA_ARGS__)
#define SXEMA2_GAME_ERROR(...)                               SXEMA2_ERROR(sxema2::LOG_STREAM_GAME, __VA_ARGS__)
#define SXEMA2_GAME_CRITICAL_ERROR(...)                      SXEMA2_CRITICAL_ERROR(sxema2::LOG_STREAM_GAME, __VA_ARGS__)
#define SXEMA2_GAME_FATAL_ERROR(...)                         SXEMA2_FATAL_ERROR(sxema2::LOG_STREAM_GAME, __VA_ARGS__)

#define SXEMA2_METAINFO_COMMENT(streamId, metaInfo, ...)     gEnv->pSchematyc2->GetLog().Comment(streamId, (metaInfo), __VA_ARGS__)
#define SXEMA2_METAINFO_WARNING(streamId, metaInfo, ...)     gEnv->pSchematyc2->GetLog().Warning(streamId, (metaInfo), __VA_ARGS__)
#define SXEMA2_METAINFO_ERROR(streamId, metaInfo, ...)       gEnv->pSchematyc2->GetLog().Error(streamId, (metaInfo), __VA_ARGS__)
#define SXEMA2_METAINFO_FATAL_ERROR(streamId, metaInfo, ...) gEnv->pSchematyc2->GetLog().FatalError(streamId, (metaInfo), __VA_ARGS__); DrxFatalError(__VA_ARGS__)

#define SXEMA2_METAINFO_CRITICAL_ERROR(streamId, metaInfo, ...)                         \
  {                                                                                         \
    static bool bIgnore = false;                                                            \
    if (!bIgnore)                                                                           \
    {                                                                                       \
      switch (gEnv->pSchematyc2->GetLog().CriticalError(streamId, (metaInfo), __VA_ARGS__)) \
      {                                                                                     \
      case sxema2::ECriticalErrorStatus::Break:                                         \
        {                                                                                   \
          SXEMA2_DEBUG_BREAK;                                                           \
          break;                                                                            \
        }                                                                                   \
      case sxema2::ECriticalErrorStatus::Ignore:                                        \
        {                                                                                   \
          bIgnore = true;                                                                   \
          break;                                                                            \
        }                                                                                   \
      }                                                                                     \
    }                                                                                       \
  }

#define SXEMA2_SYSTEM_METAINFO_COMMENT(metaInfo, ...)          SXEMA2_METAINFO_COMMENT(sxema2::LOG_STREAM_SYSTEM, (metaInfo), __VA_ARGS__)
#define SXEMA2_SYSTEM_METAINFO_WARNING(metaInfo, ...)          SXEMA2_METAINFO_WARNING(sxema2::LOG_STREAM_SYSTEM, (metaInfo), __VA_ARGS__)
#define SXEMA2_SYSTEM_METAINFO_ERROR(metaInfo, ...)            SXEMA2_METAINFO_ERROR(sxema2::LOG_STREAM_SYSTEM, (metaInfo), __VA_ARGS__)
#define SXEMA2_SYSTEM_METAINFO_CRITICAL_ERROR(metaInfo, ...)   SXEMA2_METAINFO_CRITICAL_ERROR(sxema2::LOG_STREAM_SYSTEM, (metaInfo), __VA_ARGS__)
#define SXEMA2_SYSTEM_METAINFO_FATAL_ERROR(metaInfo, ...)      SXEMA2_METAINFO_FATAL_ERROR(sxema2::LOG_STREAM_SYSTEM, (metaInfo), __VA_ARGS__)

#define SXEMA2_COMPILER_METAINFO_COMMENT(metaInfo, ...)        SXEMA2_METAINFO_COMMENT(sxema2::LOG_STREAM_COMPILER, (metaInfo), __VA_ARGS__)
#define SXEMA2_COMPILER_METAINFO_WARNING(metaInfo, ...)        SXEMA2_METAINFO_WARNING(sxema2::LOG_STREAM_COMPILER, __VA_ARGS__)
#define SXEMA2_COMPILER_METAINFO_ERROR(metaInfo, ...)          SXEMA2_METAINFO_ERROR(sxema2::LOG_STREAM_COMPILER, (metaInfo), __VA_ARGS__)
#define SXEMA2_COMPILER_METAINFO_CRITICAL_ERROR(metaInfo, ...) SXEMA2_METAINFO_CRITICAL_ERROR(sxema2::LOG_STREAM_COMPILER, (metaInfo), __VA_ARGS__)
#define SXEMA2_COMPILER_METAINFO_FATAL_ERROR(metaInfo, ...)    SXEMA2_METAINFO_FATAL_ERROR(sxema2::LOG_STREAM_COMPILER, (metaInfo), __VA_ARGS__)

#define SXEMA2_GAME_METAINFO_COMMENT(metaInfo, ...)            SXEMA2_METAINFO_COMMENT(sxema2::LOG_STREAM_GAME, (metaInfo), __VA_ARGS__)
#define SXEMA2_GAME_METAINFO_WARNING_(metaInfo, ...)           SXEMA2_METAINFO_WARNING(sxema2::LOG_STREAM_GAME, (metaInfo), __VA_ARGS__)
#define SXEMA2_GAME_METAINFO_ERROR(metaInfo, ...)              SXEMA2_METAINFO_ERROR(sxema2::LOG_STREAM_GAME, (metaInfo), __VA_ARGS__)
#define SXEMA2_GAME_METAINFO_CRITICAL_ERROR(metaInfo, ...)     SXEMA2_METAINFO_CRITICAL_ERROR(sxema2::LOG_STREAM_GAME, (metaInfo), __VA_ARGS__)
#define SXEMA2_GAME_METAINFO_FATAL_ERROR(metaInfo, ...)        SXEMA2_METAINFO_FATAL_ERROR(sxema2::LOG_STREAM_GAME, (metaInfo), __VA_ARGS__)

#if SXEMA2_ASSERTS_ENABLED

	#define SXEMA2_ASSERT(streamId, expression)                    if (!((expression))) { SXEMA2_CRITICAL_ERROR(streamId, ("ASSERT" # expression)); }
	#define SXEMA2_ASSERT_MESSAGE(streamId, expression, ...)       if (!((expression))) { SXEMA2_CRITICAL_ERROR(streamId, __VA_ARGS__); }
	#define SXEMA2_ASSERT_FATAL(streamId, expression)              if (!((expression))) { SXEMA2_FATAL_ERROR(streamId, ("ASSERT" # expression)); }
	#define SXEMA2_ASSERT_FATAL_MESSAGE(streamId, expression, ...) if (!((expression))) { SXEMA2_FATAL_ERROR(streamId, __VA_ARGS__); }

	#define SXEMA2_SYSTEM_ASSERT(expression)                       SXEMA2_ASSERT(sxema2::LOG_STREAM_SYSTEM, (expression))
	#define SXEMA2_SYSTEM_ASSERT_MESSAGE(expression, ...)          SXEMA2_ASSERT_MESSAGE(sxema2::LOG_STREAM_SYSTEM, (expression), __VA_ARGS__)
	#define SXEMA2_SYSTEM_ASSERT_FATAL(expression)                 SXEMA2_ASSERT_FATAL(sxema2::LOG_STREAM_SYSTEM, (expression))
	#define SXEMA2_SYSTEM_ASSERT_FATAL_MESSAGE(expression, ...)    SXEMA2_ASSERT_FATAL_MESSAGE(sxema2::LOG_STREAM_SYSTEM, (expression), __VA_ARGS__)

	#define SXEMA2_COMPILER_ASSERT(expression)                     SXEMA2_ASSERT(sxema2::LOG_STREAM_COMPILER, (expression))
	#define SXEMA2_COMPILER_ASSERT_MESSAGE(expression, ...)        SXEMA2_ASSERT_MESSAGE(sxema2::LOG_STREAM_COMPILER, (expression), __VA_ARGS__)
	#define SXEMA2_COMPILER_ASSERT_FATAL(expression)               SXEMA2_ASSERT_FATAL(sxema2::LOG_STREAM_COMPILER, (expression))
	#define SXEMA2_COMPILER_ASSERT_FATAL_MESSAGE(expression, ...)  SXEMA2_ASSERT_FATAL_MESSAGE(sxema2::LOG_STREAM_COMPILER, (expression), __VA_ARGS__)

	#define SXEMA2_GAME_ASSERT(expression)                         SXEMA2_ASSERT(sxema2::LOG_STREAM_GAME, (expression))
	#define SXEMA2_GAME_ASSERT_MESSAGE(expression, ...)            SXEMA2_ASSERT_MESSAGE(sxema2::LOG_STREAM_GAME, (expression), __VA_ARGS__)
	#define SXEMA2_GAME_ASSERT_FATAL(expression)                   SXEMA2_ASSERT_FATAL(sxema2::LOG_STREAM_GAME, (expression))
	#define SXEMA2_GAME_ASSERT_FATAL_MESSAGE(expression, ...)      SXEMA2_ASSERT_FATAL_MESSAGE(sxema2::LOG_STREAM_GAME, (expression), __VA_ARGS__)

#else

	#define SXEMA2_ASSERT(streamId, expression)                    ((void)0)
	#define SXEMA2_ASSERT_MESSAGE(streamId, expression, ...)       ((void)0)
	#define SXEMA2_ASSERT_FATAL(streamId, expression)              ((void)0)
	#define SXEMA2_ASSERT_FATAL_MESSAGE(streamId, expression, ...) ((void)0)

	#define SXEMA2_SYSTEM_ASSERT(expression)                       ((void)0)
	#define SXEMA2_SYSTEM_ASSERT_MESSAGE(expression, ...)          ((void)0)
	#define SXEMA2_SYSTEM_ASSERT_FATAL(expression)                 ((void)0)
	#define SXEMA2_SYSTEM_ASSERT_FATAL_MESSAGE(expression, ...)    ((void)0)

	#define SXEMA2_COMPILER_ASSERT(expression)                     ((void)0)
	#define SXEMA2_COMPILER_ASSERT_MESSAGE(expression, ...)        ((void)0)
	#define SXEMA2_COMPILER_ASSERT_FATAL(expression)               ((void)0)
	#define SXEMA2_COMPILER_ASSERT_FATAL_MESSAGE(expression, ...)  ((void)0)

	#define SXEMA2_GAME_ASSERT(expression)                         ((void)0)
	#define SXEMA2_GAME_ASSERT_MESSAGE(expression, ...)            ((void)0)
	#define SXEMA2_GAME_ASSERT_FATAL(expression)                   ((void)0)
	#define SXEMA2_GAME_ASSERT_FATAL_MESSAGE(expression, ...)      ((void)0)

#endif

namespace sxema2
{
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

WRAP_TYPE(u32, LogStreamId, 0)

typedef DynArray<LogStreamId> LogStreamIdArray;

static const LogStreamId LOG_STREAM_DEFAULT = LogStreamId(1);
static const LogStreamId LOG_STREAM_SYSTEM = LogStreamId(2);
static const LogStreamId LOG_STREAM_COMPILER = LogStreamId(3);
static const LogStreamId LOG_STREAM_GAME = LogStreamId(4);
static const LogStreamId LOG_STREAM_CUSTOM = LogStreamId(5);

enum class EDrxLinkCommand
{
	None = 0,
	Show,
	Goto
};

struct SLogMetaFunction
{
	inline SLogMetaFunction(tukk _szName = "")
		: szName(_szName)
	{}

	tukk szName;
};

struct SLogMetaChildGUID
{
	inline SLogMetaChildGUID() {}

	inline SLogMetaChildGUID(const SGUID& _guid)
		: guid(_guid)
	{}

	SGUID guid;
};

struct SLogMetaItemGUID
{
	inline SLogMetaItemGUID() {}

	inline SLogMetaItemGUID(const SGUID& _guid)
		: guid(_guid)
	{}

	SGUID guid;
};

struct SLogMetaEntityId
{
	inline SLogMetaEntityId()
		: entityId(INVALID_ENTITYID)
	{}

	inline SLogMetaEntityId(EntityId _entityId)
		: entityId(_entityId)
	{}

	EntityId entityId;
};

// TODO: We should think about how we want to handle string information
//       because this class gets passed through library boundaries.
class CLogMessageMetaInfo
{
public:

	CLogMessageMetaInfo()
	{}

	template<typename T1>
	CLogMessageMetaInfo(EDrxLinkCommand command, const T1& value1)
		: m_command(command)
	{
		Set<const T1&>(value1);
	}

	template<typename T1, typename T2>
	CLogMessageMetaInfo(EDrxLinkCommand command, const T1& value1, const T2& value2)
		: m_command(command)
	{
		Set<const T1&>(value1);
		Set<const T2&>(value2);
	}

	template<typename T1, typename T2, typename T3>
	CLogMessageMetaInfo(EDrxLinkCommand command, const T1& value1, const T2& value2, const T3& value3)
		: m_command(command)
	{
		Set<const T1&>(value1);
		Set<const T2&>(value2);
		Set<const T3&>(value3);
	}

	template<typename T1, typename T2, typename T3, typename T4>
	CLogMessageMetaInfo(EDrxLinkCommand command, const T1& value1, const T2& value2, const T3& value3, const T4& value4)
		: m_command(command)
	{
		Set<const T1&>(value1);
		Set<const T2&>(value2);
		Set<const T3&>(value3);
		Set<const T4&>(value4);
	}

	EntityId GetEntityId() const
	{
		return m_entityId.entityId;
	}

	const SGUID& GetItemGUID() const
	{
		return m_itemGuid.guid;
	}

	const SGUID& GetChildGUID() const
	{
		return m_childGuid.guid;
	}

	tukk GetFunction() const
	{
		return m_function.szName;
	}

	string GetUri() const
	{
		// TODO: Move this into CSchematycDrxLink.
		switch (m_command)
		{
		case EDrxLinkCommand::Show:
			{
				char itemGuid[StringUtils::s_guidStringBufferSize] = { 0 };
				StringUtils::SysGUIDToString(m_itemGuid.guid.sysGUID, itemGuid);
				char childGuid[StringUtils::s_guidStringBufferSize] = { 0 };
				StringUtils::SysGUIDToString(m_childGuid.guid.sysGUID, childGuid);

				stack_string command;
				DrxLinkService::CDrxLinkUriFactory drxLinkFactory("editor", DrxLinkService::ELinkType::Commands);

				command.Format("sc_rpcShowLegacy %s %s", itemGuid, childGuid);
				drxLinkFactory.AddCommand(command.c_str(), command.length());

				return drxLinkFactory.GetUri();
			}
		case EDrxLinkCommand::Goto:
			{
				char itemGuid[StringUtils::s_guidStringBufferSize] = { 0 };
				StringUtils::SysGUIDToString(m_itemGuid.guid.sysGUID, itemGuid);
				char itemBackGuid[StringUtils::s_guidStringBufferSize] = { 0 };
				StringUtils::SysGUIDToString(m_childGuid.guid.sysGUID, itemBackGuid);

				stack_string command;
				DrxLinkService::CDrxLinkUriFactory drxLinkFactory("editor", DrxLinkService::ELinkType::Commands);

				command.Format("sc_rpcGotoLegacy %s %s", itemGuid, itemBackGuid);
				drxLinkFactory.AddCommand(command.c_str(), command.length());

				return drxLinkFactory.GetUri();
			}
		case EDrxLinkCommand::None:
		default:
			return "";
		}
	}

private:
	template<typename T> void Set(T);

private:
	SLogMetaFunction  m_function;
	SLogMetaItemGUID  m_itemGuid;
	SLogMetaChildGUID m_childGuid;
	SLogMetaEntityId  m_entityId;
	EDrxLinkCommand   m_command;
};

template<> inline void CLogMessageMetaInfo::Set(const SLogMetaEntityId& entityId)
{
	m_entityId = entityId;
}

template<> inline void CLogMessageMetaInfo::Set(const SLogMetaItemGUID& guid)
{
	m_itemGuid = guid;
}

template<> inline void CLogMessageMetaInfo::Set(const SLogMetaChildGUID& guid)
{
	m_childGuid = guid;
}

template<> inline void CLogMessageMetaInfo::Set(const SLogMetaFunction& function)
{
	m_function = function;
}

struct SLogMessageData
{
	explicit inline SLogMessageData(const LogStreamId& _streamId, ELogMessageType _messageType, CLogMessageMetaInfo& _metaInfo, tukk _szMessage)
		: streamId(_streamId)
		, szMessage(_szMessage)
		, metaInfo(_metaInfo)
		, messageType(_messageType)
	{}

	const LogStreamId&   streamId;
	tukk          szMessage;
	CLogMessageMetaInfo& metaInfo;
	ELogMessageType      messageType;
};

typedef TemplateUtils::CDelegate<EVisitStatus(const SLogMessageData&)>          LogMessageVisitor;
typedef TemplateUtils::CDelegate<EVisitStatus(tukk , const LogStreamId&)> LogStreamVisitor;

struct ILogOutput
{
	virtual ~ILogOutput() {}

	virtual void ConfigureFileOutput(bool bWriteToFile, bool bForwardToStandardLog) = 0;   // Hack! Allows game to decide where messages are forwarded.
	virtual void EnableStream(const LogStreamId& streamId) = 0;
	virtual void ClearStreams() = 0;
	virtual void EnableMessageType(ELogMessageType messageType) = 0;
	virtual void ClearMessageTypes() = 0;
	virtual void Update() = 0;
};

DECLARE_SHARED_POINTERS(ILogOutput)

typedef TemplateUtils::CSignalv2<void (const SLogMessageData&)> LogMessageSignal;

struct SLogSignals
{
	LogMessageSignal message;
};

struct ILog
{
	virtual ~ILog() {}

	virtual ELogMessageType      GetMessageType(tukk szMessageType) = 0;
	virtual tukk          GetMessageTypeName(ELogMessageType messageType) = 0;
	virtual LogStreamId          CreateStream(tukk szName, const LogStreamId& staticStreamId = LogStreamId::s_invalid) = 0;
	virtual void                 DestroyStream(const LogStreamId& streamId) = 0;
	virtual LogStreamId          GetStreamId(tukk szName) const = 0;
	virtual tukk          GetStreamName(const LogStreamId& streamId) const = 0;
	virtual void                 VisitStreams(const LogStreamVisitor& visitor) const = 0;
	virtual ILogOutputPtr        CreateFileOutput(tukk szFileName) = 0;
	virtual void                 Comment(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...) = 0;
	virtual void                 Warning(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...) = 0;
	virtual void                 Error(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...) = 0;
	virtual ECriticalErrorStatus CriticalError(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...) = 0;
	virtual void                 FatalError(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...) = 0;
	virtual void                 Update() = 0;
	virtual SLogSignals&         Signals() = 0;
};

struct ILogRecorder
{
	virtual ~ILogRecorder() {}

	virtual void Begin() = 0;
	virtual void End() = 0;
	virtual void VisitMessages(const LogMessageVisitor& visitor) = 0;
	virtual void Clear() = 0;
};

namespace LogUtils
{
typedef char TimeStringBuffer[32];

inline tuk FormatTime(TimeStringBuffer& stringBuffer)
{
	time_t timeValue;
	time(&timeValue);

	strftime(stringBuffer, DRX_ARRAY_COUNT(stringBuffer), "%H:%M:%S", localtime(&timeValue));

	return stringBuffer;
}
}
}
