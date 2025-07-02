// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Optimize! Right now we're storing a full copy of the log's scope stack plus two strings per message. Maybe we could serialize to one log string buffer instead?

#pragma once

#include <drx3D/Entity/IEntity.h>
#include <drx3D/Schema/ILog.h>
#include <drx3D/Schema/ILogRecorder.h>
#include <drx3D/Schema/DrxLinkUtils.h>

namespace sxema
{
class CLogRecorder : public ILogRecorder
{
private:

	struct SRecordedMessage
	{
		explicit SRecordedMessage(const SLogMessageData& logMessageData);

		CLogMetaData           metaData;
		ELogMessageType        messageType;
		LogStreamId            streamId;
		DrxLinkUtils::ECommand linkCommand;
		DrxGUID                  elementGUID;
		DrxGUID                  detailGUID;
		EntityId               entityId;
		string                 function;
		string                 message;
	};

	typedef std::vector<SRecordedMessage> RecordedMessages;

public:

	~CLogRecorder();

	// ILogRecorder
	virtual void Begin() override;
	virtual void End() override;
	virtual void VisitMessages(const LogMessageVisitor& visitor) override;
	virtual void Clear() override;
	// ~ILogRecorder

private:

	void OnLogMessage(const SLogMessageData& logMessageData);

private:

	RecordedMessages m_recordedMessages;
	CConnectionScope m_connectionScope;
};
}
