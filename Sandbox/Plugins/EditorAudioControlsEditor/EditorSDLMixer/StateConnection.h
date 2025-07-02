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
class CStateConnection final : public CBaseConnection
{
public:

	explicit CStateConnection(ControlId const id, float const value = DrxAudio::Impl::SDL_mixer::s_defaultStateValue)
		: CBaseConnection(id)
		, m_value(value)
	{}

	CStateConnection() = delete;

	// CBaseConnection
	virtual void Serialize(Serialization::IArchive& ar) override;
	// ~CBaseConnection

	float GetValue() const { return m_value; }

private:

	float m_value;
};
} //endns SDLMixer
} //endns Impl
} //endns ACE
