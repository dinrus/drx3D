// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/PerceptionComponentHelpers.h>
#include <drx3D/AI/IEntityListenerComponent.h>

namespace sxema
{
	struct IEnvRegistrar;
}

class CEntityAIListenerComponent : public IEntityListenerComponent
{
public:
	static void ReflectType(sxema::CTypeDesc<CEntityAIListenerComponent>& desc);
	static void Register(sxema::IEnvRegistrar& registrar);

	CEntityAIListenerComponent();
	virtual ~CEntityAIListenerComponent();

	// IEntityComponent
	virtual void OnShutDown() override;

	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual uint64 GetEventMask() const override { return m_entityEventMask; };
	// ~IEntityComponent

	// IEntityListenerComponent
	virtual void SetUserConditionCallback(const UserConditionCallback& callback) override { m_userConditionCallback = callback; }
	virtual void SetSoundHeardCallback(const SoundHeardCallback& callback) override { m_soundHeardCallback = callback; }
	// ~IEntityListenerComponent

private:
	void Update();
	void Reset(EEntitySimulationMode simulationMode);

	void RegisterToAuditionMap();
	void UnregisterFromAuditionMap();
	bool IsRegistered() const { return m_registeredEntityId != INVALID_ENTITYID; }

	void SyncWithEntity();
	void UpdateChange();

	void ListenerUserConditionResult(bool bResult);
	bool OnListenerUserCondition(const Perception::SSoundStimulusParams& soundParams);
	void OnSoundHeard(const Perception::SSoundStimulusParams& soundParams);

	bool IsUsingBones() const;

	SFactionFlagsMask      m_factionsToListen = { 0 };
	float                  m_distanceScale = 1.0f;
	Perception::ComponentHelpers::SLocationsArray m_earLocations;
	bool m_bUserCustomCondition = false;

	Perception::SListenerParams m_params;
	Perception::ListenerParamsChangeOptions::Value m_changeHintFlags;
	uint64 m_entityEventMask;

	EntityId m_registeredEntityId = INVALID_ENTITYID;
	bool     m_userConditionResult = false;

	IEntityListenerComponent::UserConditionCallback m_userConditionCallback;
	IEntityListenerComponent::SoundHeardCallback m_soundHeardCallback;
};
