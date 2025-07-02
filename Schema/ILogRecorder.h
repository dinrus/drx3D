// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/ILog.h>
#include <drx3D/Schema/Delegate.h>

namespace sxema
{
typedef std::function<EVisitStatus (const SLogMessageData&)> LogMessageVisitor;

struct ILogRecorder
{
	virtual ~ILogRecorder() {}

	virtual void Begin() = 0;
	virtual void End() = 0;
	virtual void VisitMessages(const LogMessageVisitor& visitor) = 0;
	virtual void Clear() = 0;
};
} // sxema
