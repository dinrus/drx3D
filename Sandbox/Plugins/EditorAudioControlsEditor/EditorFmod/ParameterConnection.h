// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "BaseConnection.h"

#include <DrxAudioImplFmod/GlobalData.h>

namespace ACE
{
namespace Impl
{
namespace Fmod
{
class CParameterConnection final : public CBaseConnection
{
public:

	explicit CParameterConnection(
	  ControlId const id,
	  float const mult = DrxAudio::Impl::Fmod::s_defaultParamMultiplier,
	  float const shift = DrxAudio::Impl::Fmod::s_defaultParamShift)
		: CBaseConnection(id)
		, m_mult(mult)
		, m_shift(shift)
	{}

	CParameterConnection() = delete;

	// CBaseConnection
	virtual bool HasProperties() const override { return true; }
	virtual void Serialize(Serialization::IArchive& ar) override;
	// ~CBaseConnection

	float GetMultiplier() const { return m_mult; }
	float GetShift() const      { return m_shift; }

private:

	float m_mult;
	float m_shift;
};
} //endns Fmod
} //endns Impl
} //endns ACE
