// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/Log.h>

#include <drx3D/CoreX/Platform/DrxWindows.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema2/TemplateUtils_ScopedConnection.h>
#include <drx3D/Sys/IDrxPak.h>

#include <drx3D/Schema2/CVars.h>

SERIALIZATION_ENUM_BEGIN_NESTED(sxema2, ELogMessageType, "sxema Log Message Type")
	SERIALIZATION_ENUM(sxema2::ELogMessageType::Comment, "Comment", "Comment")
	SERIALIZATION_ENUM(sxema2::ELogMessageType::Warning, "Warning", "Warning")
	SERIALIZATION_ENUM(sxema2::ELogMessageType::Error, "Error", "Error")
	SERIALIZATION_ENUM(sxema2::ELogMessageType::CriticalError, "CriticalError", "Critical Error")
SERIALIZATION_ENUM_END()

namespace sxema2
{
	namespace LogUtils
	{
		template <size_t SIZE> inline void FormatMessage(char (&messageBuffer)[SIZE], tukk szFormat, va_list va_args)
		{
			const size_t pos = strlen(messageBuffer);
			vsnprintf(messageBuffer + pos, sizeof(messageBuffer) - pos - 1, szFormat, va_args);
		}

		inline ECriticalErrorStatus DisplayCriticalErrorMessage(tukk szMessage)
		{
			string message = szMessage;
			message.append("\r\n\r\n");
			message.append("Would you like to continue?\r\nClick 'Yes' to continue, 'No' to break or 'Cancel' to continue and ignore this error.");

			switch (DrxMessageBox(message.c_str(), "sxema - Critical Error!", eMB_YesNoCancel))
			{
			case eQR_Yes:
				{
					return ECriticalErrorStatus::Continue;
				}
			case eQR_Cancel:
				{
					return ECriticalErrorStatus::Ignore;
				}
			}
			return ECriticalErrorStatus::Break;
		}

		void CriticalErrorCommand(IConsoleCmdArgs* pArgs)
		{
			tukk szMessage = "";
			if(pArgs->GetArgCount() == 2)
			{
				szMessage = pArgs->GetArg(1);
			}
			SXEMA2_SYSTEM_CRITICAL_ERROR(szMessage);
		}

		void FatalErrorCommand(IConsoleCmdArgs* pArgs)
		{
			tukk szMessage = "";
			if(pArgs->GetArgCount() == 2)
			{
				szMessage = pArgs->GetArg(1);
			}
			SXEMA2_SYSTEM_FATAL_ERROR(szMessage);
		}

		class CFileOutput : public ILogOutput, public IErrorObserver
		{
		private:

			typedef DrxStackStringT<char, 512>   MessageString;
			typedef std::vector<MessageString>   MessageQueue;
			typedef std::vector<LogStreamId>     StreamIds;
			typedef std::vector<ELogMessageType> MessageTypes;

		public:

			CFileOutput(LogMessageSignal& messageSignal, tukk szFileName)
				: m_pFile(nullptr)
				, m_bErrorObserverRegistered(false)
				, m_bWriteToFile(true)
				, m_bForwardToStandardLog(false)
			{
				m_fileName = PathUtil::Make(gEnv->pSystem->GetRootFolder(), szFileName);

				Initialize();
				messageSignal.Connect(LogMessageSignal::Delegate::FromMemberFunction<CFileOutput, &CFileOutput::OnLogMessage>(*this), m_connectionScope);
			}

			~CFileOutput()
			{
				FlushAndClose();
			}

			// ILogOutput

			virtual void ConfigureFileOutput(bool bWriteToFile, bool bForwardToStandardLog) override // Hack! Allows game to decide where messages are forwarded.
			{
				m_bWriteToFile = bWriteToFile;
				m_bForwardToStandardLog = bForwardToStandardLog;
			}

			virtual void EnableStream(const LogStreamId& streamId) override
			{
				stl::push_back_unique(m_streamIds, streamId);
			}

			virtual void ClearStreams() override
			{
				m_streamIds.clear();
			}

			virtual void EnableMessageType(ELogMessageType messageType) override
			{
				stl::push_back_unique(m_messageTypes, messageType);
			}

			virtual void ClearMessageTypes() override
			{
				m_messageTypes.clear();
			}

			virtual void Update() override
			{
				FlushMessages();
			}

			// ~ILogOutput

			// IErrorObserver

			virtual void OnAssert(tukk szCondition, tukk szMessage, tukk szFileName, u32 fileLineNumber) override
			{
				FlushMessages();
			}

			virtual void OnFatalError(tukk szMessage) override
			{
				FlushMessages();
			}

			// ~IErrorObserver
		
			void OnLogMessage(const SLogMessageData& logMessageData)
			{
				if(m_streamIds.empty() || std::find(m_streamIds.begin(), m_streamIds.end(), logMessageData.streamId) != m_streamIds.end())
				{
					if(m_messageTypes.empty() || std::find(m_messageTypes.begin(), m_messageTypes.end(), logMessageData.messageType) != m_messageTypes.end())
					{
						EnqueueMessage(logMessageData);
					}
				}
			}

			void FlushMessages()
			{
				DRX_PROFILE_FUNCTION(PROFILE_GAME);

				bool bFlush = false;
				{
					DrxAutoCriticalSection lock(m_criticalSection);
					if (m_pFile && !m_messagQueue.empty())
					{
						const size_t valueSize = sizeof(MessageString::CharTraits::value_type);
						for (const MessageString& message : m_messagQueue)
						{
							fwrite(message.c_str(), valueSize, message.size(), m_pFile);
						}
						m_messagQueue.clear();
						bFlush = true;
					}
				}
				if (bFlush)
				{
					fflush(m_pFile);
				}
			}

			void FlushAndClose()
			{
				DrxAutoCriticalSection lock(m_criticalSection);

				FlushMessages();

				if(m_pFile)
				{
					fclose(m_pFile);
					m_pFile = nullptr;
				}

				if(m_bErrorObserverRegistered)
				{
					m_bErrorObserverRegistered = false;
					gEnv->pSystem->UnregisterErrorObserver(this);
				}

				m_messagQueue.shrink_to_fit();
			}

		private:

			void Initialize()
			{
				if(!m_pFile)
				{
					m_bErrorObserverRegistered = gEnv->pSystem->RegisterErrorObserver(this);

					FILE *pFile = fxopen(m_fileName.c_str(), "rb");
					if(pFile)
					{
						fclose(pFile);

						stack_string backupFileName("LogBackups/");
						backupFileName.append(m_fileName.c_str());
#if DRX_PLATFORM_DURANGO
						DRX_ASSERT_MESSAGE(false, "MoveFileEx not supported on Durango!");
#else
						CopyFile(m_fileName.c_str(), backupFileName.c_str(), true);
#endif
					}

					if (m_pFile = fxopen(m_fileName.c_str(), "wtc"))
					{
						setvbuf(m_pFile, m_writeBuffer, _IOFBF, s_writeBufferSize);
					}
					else
					{
						DrxWarning(VALIDATOR_MODULE_UNKNOWN, VALIDATOR_WARNING, "Failed to write sxema2 log to disk!");
					}

					m_messagQueue.reserve(100);
				}
			}

			void EnqueueMessage(const SLogMessageData& logMessageData)
			{
				MessageString output;

				{
					LogUtils::TimeStringBuffer stringBuffer = "";
					output.append("<");
					output.append(LogUtils::FormatTime(stringBuffer));
					output.append("> ");
				}

				const string drxLinkUri = logMessageData.metaInfo.GetUri();
				if(!drxLinkUri.empty())
				{
					output.append("[");
					output.append(logMessageData.metaInfo.GetUri().c_str());
					output.append("]");
				}

				output.append(logMessageData.szMessage);
				output.append("\n");

				if(m_bWriteToFile)
				{
					DrxAutoCriticalSection lock(m_criticalSection);
					m_messagQueue.push_back(output);
				}

				if (m_bForwardToStandardLog)
				{
					DrxLog("SCHEMATYC: %s", output.c_str());
				}
			}

		private:

			static const size_t s_writeBufferSize = 1024 * 1024;
		
			MessageQueue                    m_messagQueue;
			StreamIds                       m_streamIds;
			MessageTypes                    m_messageTypes;
			FILE *                  m_pFile;
			string                          m_fileName;
			DrxCriticalSection              m_criticalSection;
			char                            m_writeBuffer[s_writeBufferSize];
			bool                            m_bErrorObserverRegistered;
			bool                            m_bWriteToFile;
			bool                            m_bForwardToStandardLog;
			TemplateUtils::CConnectionScope m_connectionScope;
		};
	}

	const LogStreamId CLog::FIRST_DYNAMIC_STREAM_ID(0xffff0000);

	//////////////////////////////////////////////////////////////////////////
	CLog::CLog()
		: m_nextDynamicStreamId(FIRST_DYNAMIC_STREAM_ID)
	{
		m_pSettings = SSettingsPtr(new SSettings(SettingsModifiedCallback::FromMemberFunction<CLog, &CLog::OnSettingsModified>(*this)));
	}

	//////////////////////////////////////////////////////////////////////////
	void CLog::Init()
	{
		CreateStream("Default", LOG_STREAM_DEFAULT);
		CreateStream("System", LOG_STREAM_SYSTEM);
		CreateStream("Compiler", LOG_STREAM_COMPILER);
		CreateStream("Game", LOG_STREAM_GAME);
		gEnv->pSchematyc2->GetEnvRegistry().RegisterSettings("log_settings", m_pSettings);
		REGISTER_COMMAND("sc_CriticalError", LogUtils::CriticalErrorCommand, VF_NULL, "Trigger sxema critical error");
		REGISTER_COMMAND("sc_FatalError", LogUtils::FatalErrorCommand, VF_NULL, "Trigger sxema fatal error");
	}

	//////////////////////////////////////////////////////////////////////////
	ELogMessageType CLog::GetMessageType(tukk szMessageType)
	{
		SXEMA2_SYSTEM_ASSERT(szMessageType);
		if(szMessageType)
		{
			const Serialization::EnumDescription& enumDescription = Serialization::getEnumDescription<ELogMessageType>();
			for(i32 logMessageTypeIdx = 0, logMessageTypeCount = enumDescription.count(); logMessageTypeIdx < logMessageTypeCount; ++ logMessageTypeIdx)
			{
				if(stricmp(enumDescription.labelByIndex(logMessageTypeIdx), szMessageType) == 0)
				{
					return static_cast<ELogMessageType>(enumDescription.valueByIndex(logMessageTypeIdx));
				}
			}
		}
		return ELogMessageType::Invalid;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CLog::GetMessageTypeName(ELogMessageType messageType)
	{
		const Serialization::EnumDescription& enumDescription = Serialization::getEnumDescription<ELogMessageType>();
		return enumDescription.labelByIndex(enumDescription.indexByValue(static_cast<i32>(messageType)));
	}

	//////////////////////////////////////////////////////////////////////////
	LogStreamId CLog::CreateStream(tukk szName, const LogStreamId& staticStreamId)
	{
		SXEMA2_SYSTEM_ASSERT(szName);
		if(szName && (FindStream(szName) == INVALID_INDEX))
		{
			LogStreamId streamId;
			if(staticStreamId == LogStreamId::s_invalid)
			{
				streamId = m_nextDynamicStreamId ++;
			}
			else if(staticStreamId < FIRST_DYNAMIC_STREAM_ID)
			{
				if(FindStream(staticStreamId) == INVALID_INDEX)
				{
					streamId = staticStreamId;
				}
			}
			SXEMA2_SYSTEM_ASSERT(streamId != LogStreamId::s_invalid);
			if(streamId != LogStreamId::s_invalid)
			{
				m_streams.push_back(SStream(szName, streamId));
				return streamId;
			}
		}
		return LogStreamId::s_invalid;
	}

	//////////////////////////////////////////////////////////////////////////
	void CLog::DestroyStream(const LogStreamId& streamId)
	{
		for(Streams::iterator itStream = m_streams.begin(), itEndStream = m_streams.end(); itStream != itEndStream; ++ itStream)
		{
			if(itStream->id == streamId)
			{
				m_streams.erase(itStream);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	LogStreamId CLog::GetStreamId(tukk szName) const
	{
		const size_t streamIdx = FindStream(szName);
		return streamIdx != INVALID_INDEX ? m_streams[streamIdx].id : LogStreamId::s_invalid;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CLog::GetStreamName(const LogStreamId& streamId) const
	{
		const size_t streamIdx = FindStream(streamId);
		return streamIdx != INVALID_INDEX ? m_streams[streamIdx].name.c_str() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	void CLog::VisitStreams(const LogStreamVisitor& visitor) const
	{
		SXEMA2_SYSTEM_ASSERT(visitor);
		if(visitor)
		{
			for(const SStream& stream : m_streams)
			{
				visitor(stream.name.c_str(), stream.id);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ILogOutputPtr CLog::CreateFileOutput(tukk szFileName)
	{
		ILogOutputPtr pFileOutput(new LogUtils::CFileOutput(m_signals.message, szFileName));
		m_outputs.push_back(pFileOutput);
		return pFileOutput;
	}

	//////////////////////////////////////////////////////////////////////////
	void CLog::Comment(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...)
	{
		DRX_PROFILE_FUNCTION(PROFILE_GAME);

		va_list va_args;
		va_start(va_args, szFormat);
		char messageBuffer[1024] = "";
		LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
		va_end(va_args);
		m_signals.message.Send(SLogMessageData(streamId, ELogMessageType::Comment, metaInfo, messageBuffer));
	}

	//////////////////////////////////////////////////////////////////////////
	void CLog::Warning(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...)
	{
		DRX_PROFILE_FUNCTION(PROFILE_GAME);

		va_list va_args;
		va_start(va_args, szFormat);
		char messageBuffer[1024] = "";
		LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
		va_end(va_args);
		m_signals.message.Send(SLogMessageData(streamId, ELogMessageType::Warning, metaInfo, messageBuffer));
	}

	//////////////////////////////////////////////////////////////////////////
	void CLog::Error(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...)
	{
		DRX_PROFILE_FUNCTION(PROFILE_GAME);

		va_list va_args;
		va_start(va_args, szFormat);
		char messageBuffer[1024] = "";
		LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
		va_end(va_args);
		m_signals.message.Send(SLogMessageData(streamId, ELogMessageType::Error, metaInfo, messageBuffer));
	}

	//////////////////////////////////////////////////////////////////////////
	ECriticalErrorStatus CLog::CriticalError(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...)
	{
		va_list va_args;
		va_start(va_args, szFormat);
		char messageBuffer[1024] = "";
		LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
		va_end(va_args);
		m_signals.message.Send(SLogMessageData(streamId, ELogMessageType::CriticalError, metaInfo, messageBuffer));
		if(CVars::sc_DisplayCriticalErrors)
		{
			return LogUtils::DisplayCriticalErrorMessage(messageBuffer);
		}
		else
		{
			return ECriticalErrorStatus::Continue;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CLog::FatalError(const LogStreamId& streamId, CLogMessageMetaInfo metaInfo, tukk szFormat, ...)
	{
		va_list va_args;
		va_start(va_args, szFormat);
		char messageBuffer[1024] = "";
		LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
		va_end(va_args);
		m_signals.message.Send(SLogMessageData(streamId, ELogMessageType::FatalError, metaInfo, messageBuffer));
	}

	//////////////////////////////////////////////////////////////////////////
	void CLog::Update()
	{
		for(ILogOutputPtr& pOutput : m_outputs)
		{
			pOutput->Update();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	SLogSignals& CLog::Signals()
	{
		return m_signals;
	}

	//////////////////////////////////////////////////////////////////////////
	CLog::SStream::SStream(tukk _szName, const LogStreamId& _id)
		: name(_szName)
		, id(_id)
	{}

	//////////////////////////////////////////////////////////////////////////
	void CLog::OnSettingsModified()
	{
		for(const string& userStream : m_pSettings->userStreams)
		{
			tukk szStreamName = userStream.c_str();
			if(szStreamName[0] && (FindStream(szStreamName) == INVALID_INDEX))
			{
				CreateStream(szStreamName);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CLog::FindStream(tukk szName) const
	{
		SXEMA2_SYSTEM_ASSERT(szName);
		if(szName)
		{
			for(size_t streamIdx = 0, steamCount = m_streams.size(); streamIdx < steamCount; ++ streamIdx)
			{
				if(stricmp(m_streams[streamIdx].name.c_str(), szName) == 0)
				{
					return streamIdx;
				}
			}
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CLog::FindStream(const LogStreamId& streamId) const
	{
		for(size_t streamIdx = 0, steamCount = m_streams.size(); streamIdx < steamCount; ++ streamIdx)
		{
			if(m_streams[streamIdx].id == streamId)
			{
				return streamIdx;
			}
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	void CLog::GetUserStreamsFileName(stack_string& fileName) const
	{
		fileName = gEnv->pSystem->GetIPak()->GetGameFolder();
		fileName.append("/");
		fileName.append(CVars::GetStringSafe(CVars::sc_RootFolder));
		fileName.append("/");
		fileName.append("log_user_streams.xml");
	}

	//////////////////////////////////////////////////////////////////////////
	CLog::SSettings::SSettings(const SettingsModifiedCallback& _modifiedCallback)
		: modifiedCallback(_modifiedCallback)
	{}

	//////////////////////////////////////////////////////////////////////////
	void CLog::SSettings::Serialize(Serialization::IArchive& archive)
	{
		archive(userStreams, "userStreams", "User Streams");
		if(archive.isInput() && modifiedCallback)
		{
			modifiedCallback();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	CLogRecorder::~CLogRecorder()
	{
		End();
	}

	//////////////////////////////////////////////////////////////////////////
	void CLogRecorder::Begin()
	{
		m_recordedMessages.reserve(1024);
		gEnv->pSchematyc2->GetLog().Signals().message.Connect(LogMessageSignal::Delegate::FromMemberFunction<CLogRecorder, &CLogRecorder::OnLogMessage>(*this), m_connectionScope);
	}

	//////////////////////////////////////////////////////////////////////////
	void CLogRecorder::End()
	{
		m_connectionScope.Release();
	}

	//////////////////////////////////////////////////////////////////////////
	void CLogRecorder::VisitMessages(const LogMessageVisitor& visitor)
	{
		SXEMA2_SYSTEM_ASSERT(visitor);
		if(visitor)
		{
			for(auto recordedMessage : m_recordedMessages)
			{
				if(visitor(SLogMessageData(recordedMessage.streamId, recordedMessage.messageType, recordedMessage.metaInfo, recordedMessage.message.c_str())) != EVisitStatus::Continue)
				{
					return;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CLogRecorder::Clear()
	{
		m_recordedMessages.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	CLogRecorder::SRecordedMessage::SRecordedMessage(const SLogMessageData& _logMessageData)
		: streamId(_logMessageData.streamId)
		, messageType(_logMessageData.messageType)
		, metaInfo(_logMessageData.metaInfo)
		, message(_logMessageData.szMessage)
	{}

	//////////////////////////////////////////////////////////////////////////
	void CLogRecorder::OnLogMessage(const SLogMessageData& logMessageData)
	{
		m_recordedMessages.push_back(SRecordedMessage(logMessageData));
	}
}
