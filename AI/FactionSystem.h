// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/FactionMap.h>

struct SFactionID
{
	SFactionID(u8k id = IFactionMap::InvalidFactionID)
		: id(id)
	{}

	bool IsValid() const { return id != IFactionMap::InvalidFactionID; }

	inline bool operator==(const SFactionID& other) const
	{
		if (id == IFactionMap::InvalidFactionID)
			return false;

		return id == other.id;
	}

	u8 id;
	string name;
};

struct SFactionFlagsMask
{
	SFactionFlagsMask() : mask(0) {}
	SFactionFlagsMask(u32 mask) : mask(mask) {}
	
	inline bool operator==(const SFactionFlagsMask& other) const { return mask == other.mask; }
	
	u32 mask;
};

class CFactionSystem
{
public:
	typedef Functor2 <u8 /*factionID*/, IFactionMap::ReactionType /*reaction*/> ReactionChangedCallback;
	
	CFactionSystem();
	~CFactionSystem();

	CFactionMap* GetFactionMap() { return &m_factionMap; }

	void SetEntityFaction(EntityId entityId, const SFactionID& newFactionId, const ReactionChangedCallback& reactionChangedCallback);
	SFactionID GetEntityFaction(EntityId entityId) const;

	void OnFactionReactionChanged(u8k factionOne, u8k factionTwo, const IFactionMap::ReactionType reactionType);

	SFactionFlagsMask GetFactionMaskByReaction(const SFactionID& factionId, const IFactionMap::ReactionType reactionType) const;
	
private:
	typedef std::set<std::pair<EntityId, ReactionChangedCallback>> EntitiesWithCallbackSet;

	std::unordered_map<u8, EntitiesWithCallbackSet> m_entitiesInFactionsMap;
	std::unordered_map<EntityId, u8> m_factionForEntitiesMap;

	CFactionMap m_factionMap;
};