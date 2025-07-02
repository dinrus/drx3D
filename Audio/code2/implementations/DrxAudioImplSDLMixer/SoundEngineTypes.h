// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "GlobalData.h"

#include <drx3D/Audio/IAudioImpl.h>
#include <drx3D/Audio/PoolObject.h>

namespace DrxAudio
{
namespace Impl
{
namespace SDL_mixer
{
using SampleId = uint;
using ListenerId = uint;
using ChannelList = std::vector<i32>;

enum class EEventType : EnumFlagsType
{
	None,
	Start,
	Stop,
	Pause,
	Resume,
};

class CTrigger final : public ITrigger
{
public:

	explicit CTrigger(
	  EEventType const type,
	  SampleId const sampleId,
	  float const attenuationMinDistance = s_defaultMinAttenuationDist,
	  float const attenuationMaxDistance = s_defaultMaxAttenuationDist,
	  i32 const volume = 128,
	  i32 const numLoops = 1,
	  i32 const fadeInTime = 0,
	  i32 const fadeOutTime = 0,
	  bool const isPanningEnabled = true)
		: m_type(type)
		, m_sampleId(sampleId)
		, m_attenuationMinDistance(attenuationMinDistance)
		, m_attenuationMaxDistance(attenuationMaxDistance)
		, m_volume(volume)
		, m_numLoops(numLoops)
		, m_fadeInTime(fadeInTime)
		, m_fadeOutTime(fadeOutTime)
		, m_isPanningEnabled(isPanningEnabled)
	{}

	CTrigger() = delete;
	CTrigger(CTrigger const&) = delete;
	CTrigger(CTrigger&&) = delete;
	CTrigger& operator=(CTrigger const&) = delete;
	CTrigger& operator=(CTrigger&&) = delete;

	// DrxAudio::Impl::ITrigger
	virtual ERequestStatus Load() const override                             { return ERequestStatus::Success; }
	virtual ERequestStatus Unload() const override                           { return ERequestStatus::Success; }
	virtual ERequestStatus LoadAsync(IEvent* const pIEvent) const override   { return ERequestStatus::Success; }
	virtual ERequestStatus UnloadAsync(IEvent* const pIEvent) const override { return ERequestStatus::Success; }
	// ~DrxAudio::Impl::ITrigger

	EEventType GetType() const                   { return m_type; }
	SampleId   GetSampleId() const               { return m_sampleId; }

	float      GetAttenuationMinDistance() const { return m_attenuationMinDistance; }
	float      GetAttenuationMaxDistance() const { return m_attenuationMaxDistance; }
	i32        GetVolume() const                 { return m_volume; }
	i32        GetNumLoops() const               { return m_numLoops; }
	i32        GetFadeInTime() const             { return m_fadeInTime; }
	i32        GetFadeOutTime() const            { return m_fadeOutTime; }
	bool       IsPanningEnabled() const          { return m_isPanningEnabled; }

private:

	EEventType const m_type;
	SampleId const   m_sampleId;
	float const      m_attenuationMinDistance;
	float const      m_attenuationMaxDistance;
	i32 const        m_volume;
	i32 const        m_numLoops;
	i32 const        m_fadeInTime;
	i32 const        m_fadeOutTime;
	bool const       m_isPanningEnabled;
};

class CParameter final : public IParameter
{
public:

	explicit CParameter(
	  SampleId const sampleId,
	  float const multiplier,
	  float const shift,
	  char const* const szName)
		: m_sampleId(sampleId)
		, m_multiplier(multiplier)
		, m_shift(shift)
		, m_name(szName)
	{}

	virtual ~CParameter() override = default;

	CParameter() = delete;
	CParameter(CParameter const&) = delete;
	CParameter(CParameter&&) = delete;
	CParameter&                                            operator=(CParameter const&) = delete;
	CParameter&                                            operator=(CParameter&&) = delete;

	SampleId                                               GetSampleId() const   { return m_sampleId; }
	float                                                  GetMultiplier() const { return m_multiplier; }
	float                                                  GetShift() const      { return m_shift; }
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const& GetName() const       { return m_name; }

private:

	SampleId const m_sampleId;
	float const    m_multiplier;
	float const    m_shift;
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const m_name;
};

class CSwitchState final : public ISwitchState
{
public:

	explicit CSwitchState(
	  SampleId const sampleId,
	  float const value,
	  char const* const szName)
		: m_sampleId(sampleId)
		, m_value(value)
		, m_name(szName)
	{}

	virtual ~CSwitchState() override = default;

	CSwitchState() = delete;
	CSwitchState(CSwitchState const&) = delete;
	CSwitchState(CSwitchState&&) = delete;
	CSwitchState&                                          operator=(CSwitchState const&) = delete;
	CSwitchState&                                          operator=(CSwitchState&&) = delete;

	SampleId                                               GetSampleId() const { return m_sampleId; }
	float                                                  GetValue() const    { return m_value; }
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const& GetName() const     { return m_name; }

private:

	SampleId const m_sampleId;
	float const    m_value;
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const m_name;
};

struct SEnvironment final : public IEnvironment
{
	// Empty implementation so that the engine has something
	// to refer to since environments are not currently supported by
	// the SDL Mixer implementation
	SEnvironment() = default;
	SEnvironment(SEnvironment const&) = delete;
	SEnvironment(SEnvironment&&) = delete;
	SEnvironment& operator=(SEnvironment const&) = delete;
	SEnvironment& operator=(SEnvironment&&) = delete;
};

class CEvent final : public IEvent, public CPoolObject<CEvent, stl::PSyncNone>
{
public:

	explicit CEvent(CATLEvent& event)
		: m_event(event)
	{}

	CEvent() = delete;
	CEvent(CEvent const&) = delete;
	CEvent(CEvent&&) = delete;
	CEvent& operator=(CEvent const&) = delete;
	CEvent& operator=(CEvent&&) = delete;

	// DrxAudio::Impl::IEvent
	virtual ERequestStatus Stop() override;
	// ~DrxAudio::Impl::IEvent

	CATLEvent&      m_event;
	ChannelList     m_channels;
	CTrigger const* m_pTrigger = nullptr;
};

class CStandaloneFile final : public IStandaloneFile
{
public:

	CStandaloneFile(char const* const szName, CATLStandaloneFile& atlStandaloneFile)
		: m_atlFile(atlStandaloneFile)
		, m_name(szName)
	{}

	CStandaloneFile(CStandaloneFile const&) = delete;
	CStandaloneFile(CStandaloneFile&&) = delete;
	CStandaloneFile& operator=(CStandaloneFile const&) = delete;
	CStandaloneFile& operator=(CStandaloneFile&&) = delete;

	CATLStandaloneFile&                m_atlFile;
	SampleId                           m_sampleId = 0; // ID unique to the file, only needed for the 'finished' request
	DrxFixedStringT<MaxFilePathLength> m_name;
	ChannelList                        m_channels;
};

using EventInstanceList = std::vector<CEvent*>;
using StandAloneFileInstanceList = std::vector<CStandaloneFile*>;
using VolumeMultipliers = std::map<SampleId, float>;

class CObject final : public IObject, public CPoolObject<CObject, stl::PSyncNone>
{
public:

	explicit CObject(u32 const id)
		: m_id(id)
		, m_bPositionChanged(false)
	{}

	CObject(CObject const&) = delete;
	CObject(CObject&&) = delete;
	CObject& operator=(CObject const&) = delete;
	CObject& operator=(CObject&&) = delete;

	// DrxAudio::Impl::IObject
	virtual ERequestStatus Update() override;
	virtual ERequestStatus Set3DAttributes(SObject3DAttributes const& attributes) override;
	virtual ERequestStatus SetEnvironment(IEnvironment const* const pIEnvironment, float const amount) override;
	virtual ERequestStatus SetParameter(IParameter const* const pIParameter, float const value) override;
	virtual ERequestStatus SetSwitchState(ISwitchState const* const pISwitchState) override;
	virtual ERequestStatus SetObstructionOcclusion(float const obstruction, float const occlusion) override;
	virtual ERequestStatus ExecuteTrigger(ITrigger const* const pITrigger, IEvent* const pIEvent) override;
	virtual ERequestStatus StopAllTriggers() override;
	virtual ERequestStatus PlayFile(IStandaloneFile* const pIStandaloneFile) override;
	virtual ERequestStatus StopFile(IStandaloneFile* const pIStandaloneFile) override;
	virtual ERequestStatus SetName(char const* const szName) override;
	// ~DrxAudio::Impl::IObject

	u32k               m_id;
	CObjectTransformation      m_transformation;
	EventInstanceList          m_events;
	StandAloneFileInstanceList m_standaloneFiles;
	VolumeMultipliers          m_volumeMultipliers;
	bool                       m_bPositionChanged;
};

class CListener final : public IListener
{
public:

	explicit CListener(const ListenerId id)
		: m_id(id)
	{}

	CListener(CListener const&) = delete;
	CListener(CListener&&) = delete;
	CListener& operator=(CListener const&) = delete;
	CListener& operator=(CListener&&) = delete;

	// DrxAudio::Impl::IListener
	virtual ERequestStatus Set3DAttributes(SObject3DAttributes const& attributes) override;
	// ~DrxAudio::Impl::IListener

	const ListenerId m_id;
};

struct SFile final : public IFile
{
	SFile() = default;
	SFile(SFile const&) = delete;
	SFile(SFile&&) = delete;
	SFile& operator=(SFile const&) = delete;
	SFile& operator=(SFile&&) = delete;

	SampleId sampleId;
};
} //endns SDL_mixer
} //endns Impl
} //endns DrxAudio
