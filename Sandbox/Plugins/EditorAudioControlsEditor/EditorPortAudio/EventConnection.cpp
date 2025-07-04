// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "EventConnection.h"

#include <drx3D/CoreX/Serialization/Enum.h>

namespace ACE
{
namespace Impl
{
namespace PortAudio
{
//////////////////////////////////////////////////////////////////////////
void CEventConnection::Serialize(Serialization::IArchive& ar)
{
	EActionType const actionType = m_actionType;
	u32 const loopCount = m_loopCount;
	bool const isInfiniteLoop = m_isInfiniteLoop;

	ar(m_actionType, "action", "Action");

	if (m_actionType == EActionType::Start)
	{
		ar(m_isInfiniteLoop, "infinite_loop", "Infinite Loop");

		if (!m_isInfiniteLoop)
		{
			ar(m_loopCount, "loop_count", "Loop Count");
			m_loopCount = std::max<u32>(1, m_loopCount);
		}
	}

	if (ar.isInput())
	{
		if (actionType != m_actionType ||
		    loopCount != m_loopCount ||
		    isInfiniteLoop != m_isInfiniteLoop)
		{
			SignalConnectionChanged();
		}
	}
}

SERIALIZATION_ENUM_BEGIN_NESTED(CEventConnection, EActionType, "Action Type")
SERIALIZATION_ENUM(CEventConnection::EActionType::Start, "start", "Start")
SERIALIZATION_ENUM(CEventConnection::EActionType::Stop, "stop", "Stop")
SERIALIZATION_ENUM_END()
} //endns PortAudio
} //endns Impl
} //endns ACE
