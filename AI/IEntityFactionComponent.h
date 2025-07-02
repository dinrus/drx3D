// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IEntityComponent.h>

struct IEntityFactionComponent : public IEntityComponent
{
	virtual u8 GetFactionId() const = 0;
	virtual void SetFactionId(u8k factionId) = 0;
	virtual IFactionMap::ReactionType GetReaction(const EntityId otherEntityId) const = 0;
	virtual void SetReactionChangedCallback(std::function<void(u8k, const IFactionMap::ReactionType)> callbackFunction) = 0;

	static void ReflectType(sxema::CTypeDesc<IEntityFactionComponent>& desc)
	{
		desc.SetGUID("C0B0554B-FBA5-402B-905A-EAE1D5B1A527"_drx_guid);
	}
};