// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ParameterToStateConnection.h"

namespace ACE
{
namespace Impl
{
namespace Wwise
{
//////////////////////////////////////////////////////////////////////////
void CParameterToStateConnection::Serialize(Serialization::IArchive& ar)
{
	float const value = m_value;

	ar(m_value, "value", "Value");

	if (ar.isInput())
	{
		if (fabs(value - m_value) > g_precision)
		{
			SignalConnectionChanged();
		}
	}
}
} //endns Wwise
} //endns Impl
} //endns ACE
