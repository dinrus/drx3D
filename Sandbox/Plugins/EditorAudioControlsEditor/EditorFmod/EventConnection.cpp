// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "EventConnection.h"

#include <drx3D/CoreX/Serialization/Enum.h>

namespace ACE
{
namespace Impl
{
namespace Fmod
{
//////////////////////////////////////////////////////////////////////////
void CEventConnection::Serialize(Serialization::IArchive& ar)
{
	EActionType const actionType = m_actionType;

	ar(m_actionType, "action", "Action");

	if (ar.isInput())
	{
		if (actionType != m_actionType)
		{
			SignalConnectionChanged();
		}
	}
}

SERIALIZATION_ENUM_BEGIN_NESTED(CEventConnection, EActionType, "Action Type")
SERIALIZATION_ENUM(CEventConnection::EActionType::Start, "start", "Start")
SERIALIZATION_ENUM(CEventConnection::EActionType::Stop, "stop", "Stop")
SERIALIZATION_ENUM(CEventConnection::EActionType::Pause, "pause", "Pause")
SERIALIZATION_ENUM(CEventConnection::EActionType::Resume, "resume", "Resume")
SERIALIZATION_ENUM_END()
} //endns Fmod
} //endns Impl
} //endns ACE
