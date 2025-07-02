// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AudioEvent.h"
#include "AudioObject.h"

namespace DrxAudio
{
namespace Impl
{
namespace Fmod
{
class CListener final : public IListener
{
public:

	explicit CListener(i32 const id)
		: m_id(id)
	{
		ZeroStruct(m_attributes);
	}

	virtual ~CListener() override = default;

	CListener(CListener const&) = delete;
	CListener(CListener&&) = delete;
	CListener&                operator=(CListener const&) = delete;
	CListener&                operator=(CListener&&) = delete;

	ILINE i32                 GetId() const     { return m_id; }
	ILINE FMOD_3D_ATTRIBUTES& Get3DAttributes() { return m_attributes; }

	// DrxAudio::Impl::IListener
	virtual ERequestStatus Set3DAttributes(SObject3DAttributes const& attributes) override
	{
		FillFmodObjectPosition(attributes, m_attributes);
		FMOD_RESULT const fmodResult = s_pSystem->setListenerAttributes(m_id, &m_attributes);
		ASSERT_FMOD_OK;
		return ERequestStatus::Success;
	}
	// ~DrxAudio::Impl::IListener

	static FMOD::Studio::System* s_pSystem;

private:

	i32                m_id;
	FMOD_3D_ATTRIBUTES m_attributes;
};

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
	  u32 const id,
	  EEventType const eventType,
	  FMOD::Studio::EventDescription* const pEventDescription,
	  FMOD_GUID const guid)
		: m_id(id)
		, m_eventType(eventType)
		, m_pEventDescription(pEventDescription)
		, m_guid(guid)
	{}

	virtual ~CTrigger() override = default;

	CTrigger(CTrigger const&) = delete;
	CTrigger(CTrigger&&) = delete;
	CTrigger& operator=(CTrigger const&) = delete;
	CTrigger& operator=(CTrigger&&) = delete;

	// DrxAudio::Impl::ITrigger
	virtual ERequestStatus Load()  const override                            { return ERequestStatus::Success; }
	virtual ERequestStatus Unload() const override                           { return ERequestStatus::Success; }
	virtual ERequestStatus LoadAsync(IEvent* const pIEvent) const override   { return ERequestStatus::Success; }
	virtual ERequestStatus UnloadAsync(IEvent* const pIEvent) const override { return ERequestStatus::Success; }
	// ~DrxAudio::Impl::ITrigger

	u32                          GetId() const               { return m_id; }
	EEventType                      GetEventType() const        { return m_eventType; }
	FMOD::Studio::EventDescription* GetEventDescription() const { return m_pEventDescription; }
	FMOD_GUID                       GetGuid() const             { return m_guid; }

private:

	u32 const                          m_id;
	EEventType const                      m_eventType;
	FMOD::Studio::EventDescription* const m_pEventDescription;
	FMOD_GUID const                       m_guid;
};

enum class EParameterType : EnumFlagsType
{
	None,
	Parameter,
	VCA,
};

class CParameter : public IParameter
{
public:

	explicit CParameter(
	  u32 const id,
	  float const multiplier,
	  float const shift,
	  char const* const szName,
	  EParameterType const type)
		: m_id(id)
		, m_multiplier(multiplier)
		, m_shift(shift)
		, m_name(szName)
		, m_type(type)
	{}

	virtual ~CParameter() override = default;

	CParameter(CParameter const&) = delete;
	CParameter(CParameter&&) = delete;
	CParameter&                                            operator=(CParameter const&) = delete;
	CParameter&                                            operator=(CParameter&&) = delete;

	u32                                                 GetId() const              { return m_id; }
	float                                                  GetValueMultiplier() const { return m_multiplier; }
	float                                                  GetValueShift() const      { return m_shift; }
	EParameterType                                         GetType() const            { return m_type; }
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const& GetName() const            { return m_name; }

private:

	u32 const         m_id;
	float const          m_multiplier;
	float const          m_shift;
	EParameterType const m_type;
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const m_name;
};

class CVcaParameter final : public CParameter
{
public:

	explicit CVcaParameter(
	  u32 const id,
	  float const multiplier,
	  float const shift,
	  char const* const szName,
	  FMOD::Studio::VCA* const vca)
		: CParameter(id, multiplier, shift, szName, EParameterType::VCA)
		, m_vca(vca)
	{}

	FMOD::Studio::VCA* GetVca() const { return m_vca; }

private:

	FMOD::Studio::VCA* const m_vca;
};

enum class EStateType : EnumFlagsType
{
	None,
	State,
	VCA,
};

class CSwitchState : public ISwitchState
{
public:

	explicit CSwitchState(
	  u32 const id,
	  float const value,
	  char const* const szName,
	  EStateType const type)
		: m_id(id)
		, m_value(value)
		, m_name(szName)
		, m_type(type)
	{}

	virtual ~CSwitchState() override = default;

	CSwitchState(CSwitchState const&) = delete;
	CSwitchState(CSwitchState&&) = delete;
	CSwitchState&                                          operator=(CSwitchState const&) = delete;
	CSwitchState&                                          operator=(CSwitchState&&) = delete;

	u32                                                 GetId() const    { return m_id; }
	float                                                  GetValue() const { return m_value; }
	EStateType                                             GetType() const  { return m_type; }
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const& GetName() const  { return m_name; }

private:

	u32 const     m_id;
	float const      m_value;
	EStateType const m_type;
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const m_name;
};

class CVcaState final : public CSwitchState
{
public:

	explicit CVcaState(
	  u32 const id,
	  float const value,
	  char const* const szName,
	  FMOD::Studio::VCA* const vca)
		: CSwitchState(id, value, szName, EStateType::VCA)
		, m_vca(vca)
	{}

	FMOD::Studio::VCA* GetVca() const { return m_vca; }

private:

	FMOD::Studio::VCA* const m_vca;
};

enum class EEnvironmentType : EnumFlagsType
{
	None,
	Bus,
	Parameter,
};

class CEnvironment : public IEnvironment
{
public:

	explicit CEnvironment(EEnvironmentType const type)
		: m_type(type)
	{}

	virtual ~CEnvironment() override = default;

	CEnvironment(CEnvironment const&) = delete;
	CEnvironment(CEnvironment&&) = delete;
	CEnvironment&    operator=(CEnvironment const&) = delete;
	CEnvironment&    operator=(CEnvironment&&) = delete;

	EEnvironmentType GetType() const { return m_type; }

private:

	EEnvironmentType const m_type;
};

class CEnvironmentBus final : public CEnvironment
{
public:

	explicit CEnvironmentBus(
	  FMOD::Studio::EventDescription* const pEventDescription,
	  FMOD::Studio::Bus* const pBus)
		: CEnvironment(EEnvironmentType::Bus)
		, m_pEventDescription(pEventDescription)
		, m_pBus(pBus)
	{}

	virtual ~CEnvironmentBus() override = default;

	FMOD::Studio::EventDescription* GetEventDescription() const { return m_pEventDescription; }
	FMOD::Studio::Bus*              GetBus() const              { return m_pBus; }

private:

	FMOD::Studio::EventDescription* const m_pEventDescription;
	FMOD::Studio::Bus* const              m_pBus;
};

class CEnvironmentParameter final : public CEnvironment
{
public:

	explicit CEnvironmentParameter(
	  u32 const id,
	  float const multiplier,
	  float const shift,
	  char const* const szName)
		: CEnvironment(EEnvironmentType::Parameter)
		, m_id(id)
		, m_multiplier(multiplier)
		, m_shift(shift)
		, m_name(szName)
	{}

	virtual ~CEnvironmentParameter() override = default;

	u32                                                 GetId() const              { return m_id; }
	float                                                  GetValueMultiplier() const { return m_multiplier; }
	float                                                  GetValueShift() const      { return m_shift; }
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const& GetName() const            { return m_name; }

private:

	u32 const m_id;
	float const  m_multiplier;
	float const  m_shift;
	DrxFixedStringT<DrxAudio::MaxControlNameLength> const m_name;
};

class CFile final : public IFile
{
public:

	CFile() = default;
	virtual ~CFile() override = default;

	CFile(CFile const&) = delete;
	CFile(CFile&&) = delete;
	CFile& operator=(CFile const&) = delete;
	CFile& operator=(CFile&&) = delete;

	FMOD::Studio::Bank*                pBank = nullptr;
	FMOD::Studio::Bank*                pStreamsBank = nullptr;

	DrxFixedStringT<MaxFilePathLength> m_streamsBankPath;
};

class CStandaloneFileBase : public IStandaloneFile
{
public:

	explicit CStandaloneFileBase(char const* const szFile, CATLStandaloneFile& atlStandaloneFile);
	virtual ~CStandaloneFileBase() override;

	virtual void StartLoading() = 0;
	virtual bool IsReady() = 0;
	virtual void Play(FMOD_3D_ATTRIBUTES const& attributes) = 0;
	virtual void Set3DAttributes(FMOD_3D_ATTRIBUTES const& attributes) = 0;
	virtual void Stop() = 0;

	void         ReportFileStarted();
	void         ReportFileFinished();

	CATLStandaloneFile&                          m_atlStandaloneFile;
	DrxFixedStringT<DrxAudio::MaxFilePathLength> m_fileName;
	CObjectBase* m_pObject = nullptr;
	static FMOD::System*                         s_pLowLevelSystem;

};

class CStandaloneFile final : public CStandaloneFileBase
{
public:

	explicit CStandaloneFile(char const* const szFile, CATLStandaloneFile& atlStandaloneFile)
		: CStandaloneFileBase(szFile, atlStandaloneFile)
	{}

	// CStandaloneFileBase
	virtual void StartLoading() override;
	virtual bool IsReady() override;
	virtual void Play(FMOD_3D_ATTRIBUTES const& attributes) override;
	virtual void Set3DAttributes(FMOD_3D_ATTRIBUTES const& attributes) override;
	virtual void Stop() override;
	// ~CStandaloneFileBase

	FMOD::Sound*   m_pLowLevelSound = nullptr;
	FMOD::Channel* m_pChannel = nullptr;

};

class CProgrammerSoundFile final : public CStandaloneFileBase
{
public:

	explicit CProgrammerSoundFile(char const* const szFile, FMOD_GUID const eventGuid, CATLStandaloneFile& atlStandaloneFile)
		: CStandaloneFileBase(szFile, atlStandaloneFile)
		, m_eventGuid(eventGuid)
	{}

	// CStandaloneFileBase
	virtual void StartLoading() override;
	virtual bool IsReady() override;
	virtual void Play(FMOD_3D_ATTRIBUTES const& attributes) override;
	virtual void Set3DAttributes(FMOD_3D_ATTRIBUTES const& attributes) override;
	virtual void Stop() override;
	// ~CStandaloneFileBase

private:

	FMOD_GUID const              m_eventGuid;
	FMOD::Studio::EventInstance* m_pEventInstance = nullptr;

};
class CObjectBase;

using Objects = std::vector<CObjectBase*>;
using Events = std::vector<CEvent*>;
using StandaloneFiles = std::vector<CStandaloneFile*>;
using ParameterIdToIndex = std::map<u32, i32>;
using TriggerToParameterIndexes = std::map<CTrigger const* const, ParameterIdToIndex>;
} //endns Fmod
} //endns Impl
} //endns DrxAudio
