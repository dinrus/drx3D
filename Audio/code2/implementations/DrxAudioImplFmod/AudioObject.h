// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/ATLEntityData.h>
#include <drx3D/Audio/SharedAudioData.h>
#include <drx3D/Audio/PoolObject.h>

namespace DrxAudio
{
namespace Impl
{
namespace Fmod
{
class CEvent;
class CParameter;
class CSwitchState;
class CEnvironment;
class CStandaloneFileBase;

class CObjectBase : public IObject
{
public:

	CObjectBase();
	virtual ~CObjectBase() override;

	void RemoveEvent(CEvent* const pEvent);
	void RemoveParameter(CParameter const* const pParameter);
	void RemoveSwitch(CSwitchState const* const pSwitch);
	void RemoveEnvironment(CEnvironment const* const pEnvironment);
	void RemoveFile(CStandaloneFileBase const* const pStandaloneFile);

	// DrxAudio::Impl::IObject
	virtual ERequestStatus Update() override;
	virtual ERequestStatus Set3DAttributes(SObject3DAttributes const& attributes) override;
	virtual ERequestStatus ExecuteTrigger(ITrigger const* const pITrigger, IEvent* const pIEvent) override;
	virtual ERequestStatus StopAllTriggers() override;
	virtual ERequestStatus PlayFile(IStandaloneFile* const pIStandaloneFile) override;
	virtual ERequestStatus StopFile(IStandaloneFile* const pIStandaloneFile) override;
	virtual ERequestStatus SetName(char const* const szName) override;
	// ~DrxAudio::Impl::IObject

protected:

	void StopEvent(u32 const id);
	bool SetEvent(CEvent* const pEvent);

	std::vector<CEvent*>                        m_events;
	std::vector<CStandaloneFileBase*>           m_files;
	std::map<CParameter const* const, float>    m_parameters;
	std::map<u32 const, CSwitchState const*> m_switches;
	std::map<CEnvironment const* const, float>  m_environments;

	std::vector<CEvent*>                        m_pendingEvents;
	std::vector<CStandaloneFileBase*>           m_pendingFiles;

	FMOD_3D_ATTRIBUTES                          m_attributes;
	float m_obstruction = 0.0f;
	float m_occlusion = 0.0f;

public:

	static FMOD::Studio::System* s_pSystem;
};

using Objects = std::vector<CObjectBase*>;

class CObject final : public CObjectBase, public CPoolObject<CObject, stl::PSyncNone>
{
public:

	// DrxAudio::Impl::IObject
	virtual ERequestStatus SetEnvironment(IEnvironment const* const pIEnvironment, float const amount) override;
	virtual ERequestStatus SetParameter(IParameter const* const pIParameter, float const value) override;
	virtual ERequestStatus SetSwitchState(ISwitchState const* const pISwitchState) override;
	virtual ERequestStatus SetObstructionOcclusion(float const obstruction, float const occlusion) override;
	// ~DrxAudio::Impl::IObject
};

class CGlobalObject final : public CObjectBase
{
public:

	CGlobalObject(Objects const& objects)
		: m_objects(objects)
	{}

	// DrxAudio::Impl::IObject
	virtual ERequestStatus SetEnvironment(IEnvironment const* const pIEnvironment, float const amount) override;
	virtual ERequestStatus SetParameter(IParameter const* const pIParameter, float const value) override;
	virtual ERequestStatus SetSwitchState(ISwitchState const* const pISwitchState) override;
	virtual ERequestStatus SetObstructionOcclusion(float const obstruction, float const occlusion) override;
	// ~DrxAudio::Impl::IObject

	Objects const& m_objects;
};
} //endns Fmod
} //endns Impl
} //endns DrxAudio
