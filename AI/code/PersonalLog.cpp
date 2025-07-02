// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/PersonalLog.h>

void PersonalLog::AddMessage(const EntityId entityId, tukk message)
{
	if (m_messages.size() + 1 > 20)
		m_messages.pop_front();

	m_messages.push_back(message);

	if (gAIEnv.CVars.OutputPersonalLogToConsole)
	{
		tukk name = "(null)";

		if (IEntity* entity = gEnv->pEntitySystem->GetEntity(entityId))
			name = entity->GetName();

		gEnv->pLog->Log("Personal Log [%s] %s", name, message);
	}

#ifdef DRXAISYS_DEBUG
	if (IEntity* entity = gEnv->pEntitySystem->GetEntity(entityId))
	{
		if (IAIObject* ai = entity->GetAI())
		{
			IAIRecordable::RecorderEventData recorderEventData(message);
			ai->RecordEvent(IAIRecordable::E_PERSONALLOG, &recorderEventData);
		}
	}
#endif
}
