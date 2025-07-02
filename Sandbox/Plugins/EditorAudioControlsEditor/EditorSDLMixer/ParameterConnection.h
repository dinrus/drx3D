// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "BaseConnection.h"

#include <DrxAudioImplSDLMixer/GlobalData.h>

namespace ACE
{
namespace Impl
{
namespace SDLMixer
{
class CParameterConnection final : public CBaseConnection
{
public:

	explicit CParameterConnection(
	  ControlId const id,
	  float const mult = DrxAudio::Impl::SDL_mixer::s_defaultParamMultiplier,
	  float const shift = DrxAudio::Impl::SDL_mixer::s_defaultParamShift)
		: CBaseConnection(id)
		, m_mult(mult)
		, m_shift(shift)
	{}

	CParameterConnection() = delete;

	// CBaseConnection
	virtual void Serialize(Serialization::IArchive& ar) override;
	// ~CBaseConnection

	float GetMultiplier() const { return m_mult; }
	float GetShift() const      { return m_shift; }

private:

	float m_mult;
	float m_shift;
};
} //endns SDLMixer
} //endns Impl
} //endns ACE
