// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SnapshotConnection.h"

#include <drx3D/CoreX/Serialization/Enum.h>

namespace ACE
{
namespace Impl
{
namespace Fmod
{
//////////////////////////////////////////////////////////////////////////
void CSnapshotConnection::Serialize(Serialization::IArchive& ar)
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

SERIALIZATION_ENUM_BEGIN_NESTED(CSnapshotConnection, EActionType, "Action Type")
SERIALIZATION_ENUM(CSnapshotConnection::EActionType::Start, "start", "Start")
SERIALIZATION_ENUM(CSnapshotConnection::EActionType::Stop, "stop", "Stop")
SERIALIZATION_ENUM_END()
} //endns Fmod
} //endns Impl
} //endns ACE
