// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <queue>

#if !defined(_RELEASE) && DRX_PLATFORM_WINDOWS
	#define AI_COMPILE_WITH_PERSONAL_LOG
#endif

// An actor keeps a personal log where messages are stored.
// This text can be rendered to the screen and will be recorded
// in the AI Recorded for later inspection.
class PersonalLog
{
public:
	typedef std::deque<string> Messages;

	void            AddMessage(const EntityId entityId, tukk message);
	const Messages& GetMessages() const { return m_messages; }
	void            Clear()             { m_messages.clear(); }

private:
	Messages m_messages;
};
