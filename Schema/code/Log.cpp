// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/Log.h>

#include <drx3D/CoreX/Platform/DrxWindows.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/CoreX/String/DrxStringUtils.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/ScopedConnection.h>

#include <drx3D/Schema/CVars.h>
#include <drx3D/Schema/Core.h>

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, ELogMessageType, "DrxSchematyc Log Message Type")
SERIALIZATION_ENUM(sxema::ELogMessageType::Comment, "Comment", "Comment")
SERIALIZATION_ENUM(sxema::ELogMessageType::Warning, "Warning", "Warning")
SERIALIZATION_ENUM(sxema::ELogMessageType::Error, "Error", "Error")
SERIALIZATION_ENUM(sxema::ELogMessageType::CriticalError, "CriticalError", "Critical Error")
SERIALIZATION_ENUM_END()

namespace sxema
{
namespace LogUtils
{
template<u32 SIZE> inline void FormatMessage(char (&messageBuffer)[SIZE], tukk szFormat, va_list va_args)
{
	u32k pos = strlen(messageBuffer);
	drx_vsprintf(messageBuffer + pos, sizeof(messageBuffer) - pos - 1, szFormat, va_args);
}

inline ECriticalErrorStatus DisplayCriticalErrorMessage(tukk szMessage)
{
#if defined(DRX_PLATFORM_WINDOWS)
	string message = szMessage;
	message.append("\r\n\r\n");
	message.append("Would you like to continue?\r\n\r\nClick 'Yes' to continue, 'No' to break or 'Cancel' to continue and ignore this error.");
	switch (DrxMessageBox(message.c_str(), "DrxSchematyc - Critical Error!", eMB_YesNoCancel))
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
#else
	DrxMessageBox(szMessage, "DrxSchematyc - Critical Error!", eMB_Error);
#endif
	return ECriticalErrorStatus::Break;
}

void CriticalErrorCommand(IConsoleCmdArgs* pArgs)
{
	tukk szMessage = "";
	if (pArgs->GetArgCount() == 2)
	{
		szMessage = pArgs->GetArg(1);
	}
	SXEMA_CORE_CRITICAL_ERROR(szMessage);
}

void FatalErrorCommand(IConsoleCmdArgs* pArgs)
{
	tukk szMessage = "";
	if (pArgs->GetArgCount() == 2)
	{
		szMessage = pArgs->GetArg(1);
	}
	SXEMA_CORE_FATAL_ERROR(szMessage);
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
		, m_fileName(szFileName)
		, m_bErrorObserverRegistered(false)
	{
		Initialize();
		messageSignal.GetSlots().Connect(SXEMA_MEMBER_DELEGATE(&CFileOutput::OnLogMessage, *this), m_connectionScope);
	}

	~CFileOutput()
	{
		FlushAndClose();
	}

	// ILogOutput

	virtual void EnableStream(LogStreamId streamId) override
	{
		stl::push_back_unique(m_streamIds, streamId);
	}

	virtual void DisableAllStreams() override
	{
		m_streamIds.clear();
	}

	virtual void EnableMessageType(ELogMessageType messageType) override
	{
		stl::push_back_unique(m_messageTypes, messageType);
	}

	virtual void DisableAllMessageTypes() override
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
		if (std::find(m_streamIds.begin(), m_streamIds.end(), logMessageData.streamId) != m_streamIds.end())
		{
			if (std::find(m_messageTypes.begin(), m_messageTypes.end(), logMessageData.messageType) != m_messageTypes.end())
			{
				EnqueueMessage(logMessageData);
			}
		}
	}

	void FlushMessages()
	{
		DRX_PROFILE_FUNCTION(PROFILE_GAME);

		u32k valueSize = sizeof(MessageString::CharTraits::value_type);
		DrxAutoCriticalSection lock(m_criticalSection);
		if (m_pFile && !m_messagQueue.empty())
		{
			for (const MessageString& message : m_messagQueue)
			{
				fwrite(message.c_str(), valueSize, message.size(), m_pFile);
			}
			m_messagQueue.clear();
			fflush(m_pFile);
		}
	}

	void FlushAndClose()
	{
		DrxAutoCriticalSection lock(m_criticalSection);

		FlushMessages();

		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = nullptr;
		}

		if (m_bErrorObserverRegistered)
		{
			m_bErrorObserverRegistered = false;
			gEnv->pSystem->UnregisterErrorObserver(this);
		}

		m_messagQueue.shrink_to_fit();
	}

private:

	void Initialize()
	{
		if (!m_pFile)
		{
			m_bErrorObserverRegistered = gEnv->pSystem->RegisterErrorObserver(this);

			FILE* pFile = fxopen(m_fileName.c_str(), "rb");
			if (pFile)
			{
				fclose(pFile);

				CStackString backupFileName("LogBackups/");
				backupFileName.append(m_fileName.c_str());
#if DRX_PLATFORM_DURANGO
				DRX_ASSERT_MESSAGE(false, "MoveFileEx not supported on Durango!");
#else
				CopyFile(m_fileName.c_str(), backupFileName.c_str(), true);
#endif
			}

			m_pFile = fxopen(m_fileName.c_str(), "wtc");
			setvbuf(m_pFile, m_writeBuffer, _IOFBF, sizeof(m_writeBuffer));

			m_messagQueue.reserve(100);
		}
	}

	void EnqueueMessage(const SLogMessageData& logMessageData)
	{
		MessageString output;
		if (logMessageData.pMetaData)
		{
			CStackString uri;
			logMessageData.pMetaData->CreateUri(uri);
			if (!uri.empty())
			{
				output.append("[");
				output.append(uri.c_str());
				output.append("]");
			}
		}
		output.append(logMessageData.szMessage);
		output.append("\n");
		{
			DrxAutoCriticalSection lock(m_criticalSection);
			m_messagQueue.push_back(output);
		}
	}

private:

	MessageQueue       m_messagQueue;
	StreamIds          m_streamIds;
	MessageTypes       m_messageTypes;
	FILE*      m_pFile;
	string             m_fileName;
	DrxCriticalSection m_criticalSection;
	char               m_writeBuffer[1024 * 1024];
	bool               m_bErrorObserverRegistered;
	CConnectionScope   m_connectionScope;
};
} // LogUtils

u32k CLog::ms_firstDynamicStreamId = 0xffff0000;

CLog::SStream::SStream(tukk _szName, LogStreamId _id)
	: name(_szName)
	, id(_id)
{}

CLog::CLog()
	: m_nextDynamicStreamId(ms_firstDynamicStreamId)
{
	m_pSettings = SSettingsPtr(new SSettings(SXEMA_MEMBER_DELEGATE(&CLog::OnSettingsModified, *this)));
}

CLog::SSettings::SSettings(const SettingsModifiedCallback& _modifiedCallback)
	: modifiedCallback(_modifiedCallback)
{}

void CLog::SSettings::Serialize(Serialization::IArchive& archive)
{
	archive(userStreams, "userStreams", "User Streams");
	if (archive.isInput() && modifiedCallback)
	{
		modifiedCallback();
	}
}

void CLog::Init()
{
	CreateStream("Default", LogStreamId::Default);
	CreateStream("Core", LogStreamId::Core);
	CreateStream("Compiler", LogStreamId::Compiler);
	CreateStream("Editor", LogStreamId::Editor);
	CreateStream("Environment", LogStreamId::Env);

	CCore::GetInstance().GetSettingsUpr().RegisterSettings("log_settings", m_pSettings);

	REGISTER_COMMAND("sc_CriticalError", LogUtils::CriticalErrorCommand, VF_NULL, "Trigger sxema critical error");
	REGISTER_COMMAND("sc_FatalError", LogUtils::FatalErrorCommand, VF_NULL, "Trigger sxema fatal error");
}

void CLog::Shutdown()
{
	m_outputs.clear();
}

ELogMessageType CLog::GetMessageType(tukk szMessageType)
{
	SXEMA_CORE_ASSERT(szMessageType);
	if (szMessageType)
	{
		const Serialization::EnumDescription& enumDescription = Serialization::getEnumDescription<ELogMessageType>();
		for (i32 logMessageTypeIdx = 0, logMessageTypeCount = enumDescription.count(); logMessageTypeIdx < logMessageTypeCount; ++logMessageTypeIdx)
		{
			if (stricmp(enumDescription.labelByIndex(logMessageTypeIdx), szMessageType) == 0)
			{
				return static_cast<ELogMessageType>(enumDescription.valueByIndex(logMessageTypeIdx));
			}
		}
	}
	return ELogMessageType::Invalid;
}

tukk CLog::GetMessageTypeName(ELogMessageType messageType)
{
	const Serialization::EnumDescription& enumDescription = Serialization::getEnumDescription<ELogMessageType>();
	return enumDescription.labelByIndex(enumDescription.indexByValue(static_cast<i32>(messageType)));
}

LogStreamId CLog::CreateStream(tukk szName, LogStreamId staticStreamId)
{
	SXEMA_CORE_ASSERT(szName);
	if (szName && (FindStream(szName) == InvalidIdx))
	{
		LogStreamId streamId;
		if (staticStreamId == LogStreamId::Invalid)
		{
			streamId = static_cast<LogStreamId>(m_nextDynamicStreamId++);
		}
		else if (static_cast<u32>(staticStreamId) < ms_firstDynamicStreamId)
		{
			if (FindStream(staticStreamId) == InvalidIdx)
			{
				streamId = staticStreamId;
			}
		}
		SXEMA_CORE_ASSERT(streamId != LogStreamId::Invalid);
		if (streamId != LogStreamId::Invalid)
		{
			m_streams.push_back(SStream(szName, streamId));
			return streamId;
		}
	}
	return LogStreamId::Invalid;
}

void CLog::DestroyStream(LogStreamId streamId)
{
	for (Streams::iterator itStream = m_streams.begin(), itEndStream = m_streams.end(); itStream != itEndStream; ++itStream)
	{
		if (itStream->id == streamId)
		{
			m_streams.erase(itStream);
			break;
		}
	}
}

LogStreamId CLog::GetStreamId(tukk szName) const
{
	u32k streamIdx = FindStream(szName);
	return streamIdx != InvalidIdx ? m_streams[streamIdx].id : LogStreamId::Invalid;
}

tukk CLog::GetStreamName(LogStreamId streamId) const
{
	u32k streamIdx = FindStream(streamId);
	return streamIdx != InvalidIdx ? m_streams[streamIdx].name.c_str() : "";
}

void CLog::VisitStreams(const LogStreamVisitor& visitor) const
{
	SXEMA_CORE_ASSERT(visitor);
	if (visitor)
	{
		for (const SStream& stream : m_streams)
		{
			visitor(stream.name.c_str(), stream.id);
		}
	}
}

ILogOutputPtr CLog::CreateFileOutput(tukk szFileName)
{
	ILogOutputPtr pFileOutput(new LogUtils::CFileOutput(m_signals.message, szFileName));
	m_outputs.push_back(pFileOutput);
	return pFileOutput;
}

void CLog::PushScope(SLogScope* pScope)
{
	SXEMA_CORE_ASSERT(pScope);
	if (pScope)
	{
		m_scopeStack.push_back(pScope);
	}
}

void CLog::PopScope(SLogScope* pScope)
{
	SXEMA_CORE_ASSERT(!m_scopeStack.empty() && (pScope == m_scopeStack.back()));
	m_scopeStack.pop_back();
}

void CLog::Comment(LogStreamId streamId, tukk szFormat, va_list va_args)
{
	DRX_PROFILE_FUNCTION(PROFILE_GAME);

	char messageBuffer[1024] = "";
	LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
	m_signals.message.Send(SLogMessageData(!m_scopeStack.empty() ? &m_scopeStack.back()->metaData : nullptr, ELogMessageType::Comment, streamId, messageBuffer));
}

void CLog::Warning(LogStreamId streamId, tukk szFormat, va_list va_args)
{
	DRX_PROFILE_FUNCTION(PROFILE_GAME);

	char messageBuffer[1024] = "";
	LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
	m_signals.message.Send(SLogMessageData(!m_scopeStack.empty() ? &m_scopeStack.back()->metaData : nullptr, ELogMessageType::Warning, streamId, messageBuffer));

	for (SLogScope* pScope : m_scopeStack)
	{
		++pScope->warningCount;
	}
}

void CLog::Error(LogStreamId streamId, tukk szFormat, va_list va_args)
{
	DRX_PROFILE_FUNCTION(PROFILE_GAME);

	char messageBuffer[1024] = "";
	LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
	m_signals.message.Send(SLogMessageData(!m_scopeStack.empty() ? &m_scopeStack.back()->metaData : nullptr, ELogMessageType::Error, streamId, messageBuffer));

	for (SLogScope* pScope : m_scopeStack)
	{
		++pScope->errorCount;
	}
}

ECriticalErrorStatus CLog::CriticalError(LogStreamId streamId, tukk szFormat, va_list va_args)
{
	char messageBuffer[1024] = "";
	LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
	m_signals.message.Send(SLogMessageData(!m_scopeStack.empty() ? &m_scopeStack.back()->metaData : nullptr, ELogMessageType::CriticalError, streamId, messageBuffer));

	for (SLogScope* pScope : m_scopeStack)
	{
		++pScope->errorCount;
	}

	if (CVars::sc_DisplayCriticalErrors)
	{
		return LogUtils::DisplayCriticalErrorMessage(messageBuffer);
	}
	else
	{
		return ECriticalErrorStatus::Continue;
	}
}

void CLog::FatalError(LogStreamId streamId, tukk szFormat, va_list va_args)
{
	char messageBuffer[1024] = "";
	LogUtils::FormatMessage(messageBuffer, szFormat, va_args);
	m_signals.message.Send(SLogMessageData(!m_scopeStack.empty() ? &m_scopeStack.back()->metaData : nullptr, ELogMessageType::FatalError, streamId, messageBuffer));

	for (SLogScope* pScope : m_scopeStack)
	{
		++pScope->errorCount;
	}
}

void CLog::Update()
{
	for (ILogOutputPtr& pOutput : m_outputs)
	{
		pOutput->Update();
	}
}

LogMessageSignal::Slots& CLog::GetMessageSignalSlots()
{
	return m_signals.message.GetSlots();
}

void CLog::OnSettingsModified()
{
	for (const string& userStream : m_pSettings->userStreams)
	{
		tukk szStreamName = userStream.c_str();
		if (szStreamName[0] && (FindStream(szStreamName) == InvalidIdx))
		{
			CreateStream(szStreamName);
		}
	}
}

u32 CLog::FindStream(tukk szName) const
{
	SXEMA_CORE_ASSERT(szName);
	if (szName)
	{
		for (u32 streamIdx = 0, steamCount = m_streams.size(); streamIdx < steamCount; ++streamIdx)
		{
			if (stricmp(m_streams[streamIdx].name.c_str(), szName) == 0)
			{
				return streamIdx;
			}
		}
	}
	return InvalidIdx;
}

u32 CLog::FindStream(LogStreamId streamId) const
{
	for (u32 streamIdx = 0, steamCount = m_streams.size(); streamIdx < steamCount; ++streamIdx)
	{
		if (m_streams[streamIdx].id == streamId)
		{
			return streamIdx;
		}
	}
	return InvalidIdx;
}

void CLog::GetUserStreamsFileName(CStackString& fileName) const
{
	fileName = gEnv->pSystem->GetIPak()->GetGameFolder();
	fileName.append("/");
	fileName.append(CVars::sc_RootFolder->GetString());
	fileName.append("/");
	fileName.append("log_user_streams.xml");
}
} // sxema
