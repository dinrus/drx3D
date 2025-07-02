// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "StateConnection.h"

#include <drx3D/CoreX/Serialization/Decorators/Range.h>

namespace ACE
{
namespace Impl
{
namespace SDLMixer
{
//////////////////////////////////////////////////////////////////////////
void CStateConnection::Serialize(Serialization::IArchive& ar)
{
	float const value = m_value;

	ar(Serialization::Range(m_value, 0.0f, 1.0f, 0.1f), "value", "Volume (normalized)");
	m_value = crymath::clamp(m_value, 0.0f, 1.0f);

	if (ar.isInput())
	{
		if (fabs(value - m_value) > g_precision)
		{
			SignalConnectionChanged();
		}
	}
}
} //endns SDLMixer
} //endns Impl
} //endns ACE
