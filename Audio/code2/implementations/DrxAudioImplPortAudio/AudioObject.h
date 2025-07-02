// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ATLEntities.h"
#include <drx3D/Audio/PoolObject.h>
#include <drx3D/CoreX/Audio/IObject.h>

namespace DrxAudio
{
namespace Impl
{
namespace PortAudio
{
	
class CEvent;

//using IObject = struct DrxAudio::Impl::IObject;

struct IObject : public DrxAudio::Impl::IObject
{
public:
	IObject() = default;
	virtual ~IObject() = default;
	virtual ERequestStatus Update() = 0;
	virtual ERequestStatus Set3DAttributes(SObject3DAttributes const& attributes) = 0;
	virtual ERequestStatus SetEnvironment(IEnvironment const* const pIEnvironment, float const amount) = 0;
	virtual ERequestStatus SetParameter(IParameter const* const pIParameter, float const value) = 0;
	virtual ERequestStatus SetSwitchState(ISwitchState const* const pISwitchState) = 0;
	virtual ERequestStatus SetObstructionOcclusion(float const obstruction, float const occlusion) = 0;
	virtual ERequestStatus ExecuteTrigger(ITrigger const* const pITrigger, IEvent* const pIEvent) = 0;

	virtual ERequestStatus StopAllTriggers() = 0;
	virtual ERequestStatus PlayFile(IStandaloneFile* const pIStandaloneFile) = 0;
	virtual ERequestStatus StopFile(IStandaloneFile* const pIStandaloneFile) = 0;
	virtual ERequestStatus SetName(char const* const szName) = 0;
	virtual IObject*            ConstructGlobalObject() = 0;
	virtual IObject*            ConstructObject(char const* const szName = nullptr) = 0;
	virtual void                DestructObject(IObject const* const pIObject) = 0;
};


class CObject final : public IObject, public CPoolObject<CObject, stl::PSyncNone>
{
public:

	CObject() = default;
	virtual ~CObject() override = default;

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

	void StopEvent(u32 const pathId);
	void RegisterEvent(CEvent* const pEvent);
	void UnregisterEvent(CEvent* const pEvent);

private:

	std::vector<CEvent*> m_activeEvents;
};
} //endns PortAudio
} //endns Impl
} //endns DrxAudio
