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
class CEventConnection final : public CBaseConnection
{
public:

	enum class EActionType
	{
		Start,
		Stop,
		Pause,
		Resume,
	};

	explicit CEventConnection(ControlId const id)
		: CBaseConnection(id)
		, m_actionType(EActionType::Start)
		, m_volume(-14.0f)
		, m_fadeInTime(DrxAudio::Impl::SDL_mixer::s_defaultFadeInTime)
		, m_fadeOutTime(DrxAudio::Impl::SDL_mixer::s_defaultFadeOutTime)
		, m_minAttenuation(DrxAudio::Impl::SDL_mixer::s_defaultMinAttenuationDist)
		, m_maxAttenuation(DrxAudio::Impl::SDL_mixer::s_defaultMaxAttenuationDist)
		, m_isPanningEnabled(true)
		, m_isAttenuationEnabled(true)
		, m_isInfiniteLoop(false)
		, m_loopCount(1)
	{}

	CEventConnection() = delete;

	// CBaseConnection
	virtual void Serialize(Serialization::IArchive& ar) override;
	// ~CBaseConnection

	void        SetActionType(EActionType const type)         { m_actionType = type; }
	EActionType GetActionType() const                         { return m_actionType; }

	void        SetVolume(float const volume)                 { m_volume = volume; }
	float       GetVolume() const                             { return m_volume; }

	void        SetFadeInTime(float const fadeInTime)         { m_fadeInTime = fadeInTime; }
	float       GetFadeInTime() const                         { return m_fadeInTime; }
	void        SetFadeOutTime(float const fadeInTime)        { m_fadeOutTime = fadeInTime; }
	float       GetFadeOutTime() const                        { return m_fadeOutTime; }

	void        SetMinAttenuation(float const minAttenuation) { m_minAttenuation = minAttenuation; }
	float       GetMinAttenuation() const                     { return m_minAttenuation; }
	void        SetMaxAttenuation(float const maxAttenuation) { m_maxAttenuation = maxAttenuation; }
	float       GetMaxAttenuation() const                     { return m_maxAttenuation; }

	void        SetPanningEnabled(bool const isEnabled)       { m_isPanningEnabled = isEnabled; }
	bool        IsPanningEnabled() const                      { return m_isPanningEnabled; }

	void        SetAttenuationEnabled(bool const isEnabled)   { m_isAttenuationEnabled = isEnabled; }
	bool        IsAttenuationEnabled() const                  { return m_isAttenuationEnabled; }

	void        SetInfiniteLoop(bool const isInfinite)        { m_isInfiniteLoop = isInfinite; }
	bool        IsInfiniteLoop() const                        { return m_isInfiniteLoop; }

	void        SetLoopCount(u32 const loopCount)          { m_loopCount = loopCount; }
	u32      GetLoopCount() const                          { return m_loopCount; }

private:

	EActionType m_actionType;
	float       m_volume;
	float       m_fadeInTime;
	float       m_fadeOutTime;
	float       m_minAttenuation;
	float       m_maxAttenuation;
	bool        m_isPanningEnabled;
	bool        m_isAttenuationEnabled;
	bool        m_isInfiniteLoop;
	u32      m_loopCount;
};
} //endns SDLMixer
} //endns Impl
} //endns ACE
