// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IEntityComponent.h>

struct IEntityObservableComponent : public IEntityComponent
{
	virtual void SetTypeMask(u32k typeMask) = 0;
	virtual void AddObservableLocationOffsetFromPivot(const Vec3& offsetFromPivot) = 0;
	virtual void AddObservableLocationOffsetFromBone(const Vec3& offsetFromBone, tukk szBoneName) = 0;
	virtual void SetObservableLocationOffsetFromPivot(const size_t index, const Vec3& offsetFromPivot) = 0;
	virtual void SetObservableLocationOffsetFromBone(const size_t index, const Vec3& offsetFromBone, tukk szBoneName) = 0;
	
	static void ReflectType(sxema::CTypeDesc<IEntityObservableComponent>& desc)
	{
		desc.SetGUID("A2406F9B-F650-4207-BA05-EC0649D1F166"_drx_guid);
	}
};