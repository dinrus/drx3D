// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/PerceptionComponentHelpers.h>

#include <drx3D/CoreX/Serialization/Decorators/BitFlags.h>
#include <drx3D/AI/VisionMapTypes.h>
#include <drx3D/AI/IEntityObserverComponent.h>

namespace sxema
{
struct IEnvRegistrar;
}

namespace ObserverProperties
{
struct SVisionBlockingProperties
{
	static void ReflectType(sxema::CTypeDesc<SVisionBlockingProperties>& typeInfo)
	{
		typeInfo.SetGUID("37c2c87d-97ba-42d2-90a6-bcdc8217bd0a"_drx_guid);
	}

	void Serialize(Serialization::IArchive& archive)
	{
		archive(bBlockedBySolids, "blockedBySolids", "Blocked By Solids");
		archive.doc("If enabled, vision ray-casts cannot pass through colliders that are part of solid objects (sometimes also denoted as 'hard cover').");
		archive(bBlockedBySoftCover, "blockedBySoftCover", "Blocked By Soft Cover");
		archive.doc(
		  "If enabled, vision ray-casts cannot pass through colliders marked as 'soft cover'. "
		  "These are colliders that only block vision but you can still sit 'inside' them (examples are: bushes, tall grass, etc). ");
	}

	bool operator==(const SVisionBlockingProperties& other) const
	{
		return bBlockedBySolids == other.bBlockedBySolids && bBlockedBySoftCover == other.bBlockedBySoftCover;
	}

	bool bBlockedBySolids = true;
	bool bBlockedBySoftCover = true;
};

struct SVisionProperties
{
	static void ReflectType(sxema::CTypeDesc<SVisionProperties>& typeInfo)
	{
		typeInfo.SetGUID("aa29f1ae-d479-432a-b638-c2d420b0a6e6"_drx_guid);
	}

	void Serialize(Serialization::IArchive& archive)
	{
		archive(fov, "fov", "Field of View");
		archive.doc("Field of view in degrees");

		archive(range, "range", "Sight Range");
		archive.doc("The maximum sight distance from the 'eye' point to an observable location on an entity.");

		archive(location, "location", "Location");
		archive.doc("The location of the sensor.");

		archive(blockingProperties, "blockingProps", "Vision Blocking");
		archive.doc("Vision blocking properties.");
	}

	bool operator==(const SVisionProperties& other) const
	{
		return location == other.location
		       && fov == other.fov
		       && range == other.range
		       && blockingProperties == other.blockingProperties;
	}

	Perception::ComponentHelpers::SLocation location;
	DrxTransform::CClampedAngle<0, 360>     fov = 60.0_degrees;
	sxema::Range<0, 1000, 0, 1000>      range = 20.0f;
	SVisionBlockingProperties               blockingProperties;
};

}

class CEntityAIObserverComponent : public IEntityObserverComponent
{
public:
	static void ReflectType(sxema::CTypeDesc<CEntityAIObserverComponent>& desc);
	static void Register(sxema::IEnvRegistrar& registrar);

	CEntityAIObserverComponent();
	virtual ~CEntityAIObserverComponent();

protected:

	// IEntityComponent
	virtual void   OnShutDown() override;

	virtual void   ProcessEvent(const SEntityEvent& event) override;
	virtual uint64 GetEventMask() const override { return m_entityEventMask; };
	// ~IEntityComponent

	// IEntityObserverComponent
	virtual bool CanSee(const EntityId entityId) const override;

	virtual void SetUserConditionCallback(const UserConditionCallback callback) override;
	virtual void SetTypeMask(u32k typeMask) override;
	virtual void SetTypesToObserveMask(u32k typesToObserverMask) override;
	virtual void SetFactionsToObserveMask(u32k factionsToObserveMask) override;
	virtual void SetFOV(const float fovInRad) override;
	virtual void SetRange(const float range) override;
	virtual void SetPivotOffset(const Vec3& offsetFromPivot) override;
	virtual void SetBoneOffset(const Vec3& offsetFromBone, tukk szBoneName) override;

	virtual u32 GetTypeMask() const override;
	virtual u32 GetTypesToObserveMask() const override;
	virtual u32 GetFactionsToObserveMask() const override;
	virtual float GetFOV() const override;
	virtual float GetRange() const override;
	// ~IEntityObserverComponent

private:
	void   Update();
	void   Reset(EEntitySimulationMode simulationMode);

	void   RegisterToVisionMap();
	void   UnregisterFromVisionMap();
	bool   IsRegistered() const { return m_observerId != 0; }

	void   SyncWithEntity();
	void   UpdateChange();
	void   UpdateChange(u32 changeHintFlags);

	bool   CanSeeSchematyc(sxema::ExplicitEntityId entityId) const;

	bool   OnObserverUserCondition(const VisionID& observerId, const ObserverParams& observerParams, const VisionID& observableId, const ObservableParams& observableParams);
	void   ObserverUserConditionResult(bool bResult);

	void   OnObserverVisionChanged(const VisionID& observerId, const ObserverParams& observerParams, const VisionID& observableId, const ObservableParams& observableParams, bool visible);

	u32 GetRaycastFlags() const;

	ObserverID                   m_observerId;
	ObserverParams               m_params;
	EChangeHint                  m_changeHintFlags;
	bool                         m_bUserConditionResult = false;

	std::unordered_set<EntityId> m_visibleEntitiesSet;
	uint64                       m_entityEventMask;

	// Properties

	Perception::ComponentHelpers::SVisionMapType m_visionMapType;
	ObserverProperties::SVisionProperties        m_visionProperties;
	Perception::ComponentHelpers::SVisionMapType m_typesToObserve;
	SFactionFlagsMask                            m_factionsToObserve;
	UserConditionCallback                        m_userConditionCallback;
	bool                                         m_bUseUserCustomCondition = false;
};
