// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "BaseConnection.h"

#include <DrxAudioImplWwise/GlobalData.h>

namespace ACE
{
namespace Impl
{
namespace Wwise
{
class CParameterToStateConnection final : public CBaseConnection
{
public:

	explicit CParameterToStateConnection(ControlId const id, float const value = DrxAudio::Impl::Wwise::s_defaultStateValue)
		: CBaseConnection(id)
		, m_value(value)
	{}

	CParameterToStateConnection() = delete;

	// CBaseConnection
	virtual bool HasProperties() const override { return true; }
	virtual void Serialize(Serialization::IArchive& ar) override;
	// ~CBaseConnection

	float GetValue() const { return m_value; }

private:

	float m_value;
};
} //endns Wwise
} //endns Impl
} //endns ACE
