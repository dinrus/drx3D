// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/PerceptionComponentHelpers.h>
#include <drx3D/AI/IEntityObservableComponent.h>

namespace sxema
{
	struct IEnvRegistrar;
}

class CEntityAIObservableComponent : public IEntityObservableComponent
{
public:
	static void ReflectType(sxema::CTypeDesc<CEntityAIObservableComponent>& desc);
	static void Register(sxema::IEnvRegistrar& registrar);

	CEntityAIObservableComponent();
	virtual ~CEntityAIObservableComponent();

protected:

	// IEntityComponent
	virtual void OnShutDown() override;

	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual uint64 GetEventMask() const override { return m_entityEventMask; };
	// ~IEntityComponent

	// IEntityObservableComponent
	virtual void SetTypeMask(u32k typeMask) override;
	virtual void AddObservableLocationOffsetFromPivot(const Vec3& offsetFromPivot) override;
	virtual void AddObservableLocationOffsetFromBone(const Vec3& offsetFromBone, tukk szBoneName) override;
	virtual void SetObservableLocationOffsetFromPivot(const size_t index, const Vec3& offsetFromPivot) override;
	virtual void SetObservableLocationOffsetFromBone(const size_t index, const Vec3& offsetFromBone, tukk szBoneName) override;
	// ~IEntityObservableComponent

private:
	void Update();
	void Reset(EEntitySimulationMode simulationMode);

	void RegisterToVisionMap();
	void UnregisterFromVisionMap();
	bool IsRegistered() const { return m_observableId != 0; }

	void SyncWithEntity();
	void UpdateChange();
	void UpdateChange(u32k changeHintFlags);

	void OnObservableVisionChanged(const VisionID& observerId, const ObserverParams& observerParams, const VisionID& observableId, const ObservableParams& observableParams, bool visible);

	bool IsUsingBones() const
	{
		for (const Perception::ComponentHelpers::SLocation& location : m_observableLocations.locations)
		{
			if (location.type == Perception::ComponentHelpers::SLocation::EType::Bone)
				return true;
		}
		return false;
	}

	uint64 m_entityEventMask;

	ObservableID m_observableId;
	ObservableParams m_params;
	EChangeHint m_changeHintFlags;

	// Properties

	Perception::ComponentHelpers::SVisionMapType m_visionMapType;
	Perception::ComponentHelpers::SLocationsArray m_observableLocations;
};
