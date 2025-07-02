// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IEntityComponent.h>

struct IEntityObserverComponent : public IEntityComponent
{
	typedef std::function<bool(const VisionID& observerId, const ObserverParams& observerParams, const VisionID& observableId, const ObservableParams& observableParams)> UserConditionCallback;
	
	virtual bool CanSee(const EntityId entityId) const = 0;

	virtual void SetUserConditionCallback(const UserConditionCallback callback) = 0;
	
	virtual void SetTypeMask(u32k typeMask) = 0;
	virtual void SetTypesToObserveMask(u32k typesToObserverMask) = 0;
	virtual void SetFactionsToObserveMask(u32k factionsToObserveMask) = 0;
	virtual void SetFOV(const float fovInRad) = 0;
	virtual void SetRange(const float range) = 0;
	virtual void SetPivotOffset(const Vec3& offsetFromPivot) = 0;
	virtual void SetBoneOffset(const Vec3& offsetFromBone, tukk szBoneName) = 0;

	virtual u32 GetTypeMask() const = 0;
	virtual u32 GetTypesToObserveMask() const = 0;
	virtual u32 GetFactionsToObserveMask() const = 0;
	virtual float GetFOV() const = 0;
	virtual float GetRange() const = 0;
	
	static void ReflectType(sxema::CTypeDesc<IEntityObserverComponent>& desc)
	{
		desc.SetGUID("EC4E135E-BB29-48DB-B143-CB9206F4B2E6"_drx_guid);
	}
};