// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/BaseTypes.h>
#include <drx3D/CoreX/String/DrxString.h>
#include <drx3D/Entity/IEntity.h>

struct IEntityLayer
{
	IEntityLayer() {}
	virtual ~IEntityLayer() {}

	virtual void          SetParentName(tukk szParent) = 0;
	virtual void          AddChild(IEntityLayer* pLayer) = 0;
	virtual i32           GetNumChildren() const = 0;
	virtual IEntityLayer* GetChild(i32 idx) const = 0;
	virtual void          AddObject(EntityId id) = 0;
	virtual void          RemoveObject(EntityId id) = 0;
	virtual void          Enable(bool bEnable, bool bSerialize = true, bool bAllowRecursive = true) = 0;
	virtual bool          IsEnabled() const = 0;
	virtual bool          IsEnabledBrush() const = 0;
	virtual bool          IsSerialized() const = 0;
	virtual bool          IsDefaultLoaded() const = 0;
	virtual bool          IncludesEntity(EntityId id) const = 0;
	virtual tukk   GetName() const = 0;
	virtual tukk   GetParentName() const = 0;
	virtual u16k  GetId() const = 0;
	virtual bool          IsSkippedBySpec() const = 0;
};
