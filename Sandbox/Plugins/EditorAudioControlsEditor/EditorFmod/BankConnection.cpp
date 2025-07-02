// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "BankConnection.h"

#include "Impl.h"

namespace ACE
{
namespace Impl
{
namespace Fmod
{
//////////////////////////////////////////////////////////////////////////
void CBankConnection::Serialize(Serialization::IArchive& ar)
{
	PlatformIndexType const configurationsMask = m_configurationsMask;
	size_t const numPlatforms = s_platforms.size();

	for (size_t i = 0; i < numPlatforms; ++i)
	{
		auto const platformIndex = static_cast<PlatformIndexType>(i);
		bool isEnabled = IsPlatformEnabled(platformIndex);
		ar(isEnabled, s_platforms[i], s_platforms[i]);
		SetPlatformEnabled(platformIndex, isEnabled);
	}

	if (ar.isInput())
	{
		if (configurationsMask != m_configurationsMask)
		{
			SignalConnectionChanged();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBankConnection::SetPlatformEnabled(PlatformIndexType const platformIndex, bool const isEnabled)
{
	if (isEnabled)
	{
		m_configurationsMask |= 1 << platformIndex;
	}
	else
	{
		m_configurationsMask &= ~(1 << platformIndex);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CBankConnection::IsPlatformEnabled(PlatformIndexType const platformIndex) const
{
	return (m_configurationsMask & (1 << platformIndex)) != 0;
}

//////////////////////////////////////////////////////////////////////////
void CBankConnection::ClearPlatforms()
{
	m_configurationsMask = 0;
}
} //endns Fmod
} //endns Impl
} //endns ACE
