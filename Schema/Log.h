// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/ILog.h>
#include <drx3D/Schema/ISettingsUpr.h>
#include <drx3D/Schema/ScopedConnection.h>
#include <drx3D/Schema/StackString.h>

namespace sxema
{
// Forward declare interfaces.
struct ILogOutput;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(ILogOutput)

class CLog : public ILog
{
private:

	struct SStream
	{
		SStream(tukk _szName, LogStreamId _id);

		string      name;
		LogStreamId id;
	};

	typedef std::vector<SStream>       Streams;
	typedef std::vector<ILogOutputPtr> Outputs;
	typedef std::vector<SLogScope*>    ScopeStack;

	typedef std::function<void ()>         SettingsModifiedCallback;

	struct SSettings : public ISettings
	{
		typedef std::vector<string> UserStreams;

		SSettings(const SettingsModifiedCallback& _modifiedCallback);

		// ISettings
		virtual void Serialize(Serialization::IArchive& archive) override;
		// ~ISettings

		UserStreams              userStreams;
		SettingsModifiedCallback modifiedCallback;
	};

	DECLARE_SHARED_POINTERS(SSettings)

	struct SSignals
	{
		LogMessageSignal message;
	};

public:

	CLog();

	void Init();
	void Shutdown();

	// ILog
	virtual ELogMessageType          GetMessageType(tukk szMessageType) override;
	virtual tukk              GetMessageTypeName(ELogMessageType messageType) override;
	virtual LogStreamId              CreateStream(tukk szName, LogStreamId staticStreamId = LogStreamId::Invalid) override;
	virtual void                     DestroyStream(LogStreamId streamId) override;
	virtual LogStreamId              GetStreamId(tukk szName) const override;
	virtual tukk              GetStreamName(LogStreamId streamId) const override;
	virtual void                     VisitStreams(const LogStreamVisitor& visitor) const override;
	virtual ILogOutputPtr            CreateFileOutput(tukk szFileName) override;
	virtual void                     PushScope(SLogScope* pScope) override;
	virtual void                     PopScope(SLogScope* pScope) override;
	virtual void                     Comment(LogStreamId streamId, tukk szFormat, va_list va_args) override;
	virtual void                     Warning(LogStreamId streamId, tukk szFormat, va_list va_args) override;
	virtual void                     Error(LogStreamId streamId, tukk szFormat, va_list va_args) override;
	virtual ECriticalErrorStatus     CriticalError(LogStreamId streamId, tukk szFormat, va_list va_args) override;
	virtual void                     FatalError(LogStreamId streamId, tukk szFormat, va_list va_args) override;
	virtual void                     Update() override;
	virtual LogMessageSignal::Slots& GetMessageSignalSlots() override;
	// ~ILog

private:

	void   OnSettingsModified();
	u32 FindStream(tukk szName) const;
	u32 FindStream(LogStreamId streamId) const;
	void   GetUserStreamsFileName(CStackString& fileName) const;

private:

	u32              m_nextDynamicStreamId;
	Streams             m_streams;
	Outputs             m_outputs;
	ScopeStack          m_scopeStack;      // #SchematycTODO : If we want to support multi-threading we will need to allocate one stack per thread.
	SSettingsPtr        m_pSettings;
	SSignals            m_signals;

	static u32k ms_firstDynamicStreamId;
};
} // sxema
