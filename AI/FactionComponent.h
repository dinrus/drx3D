// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/IEntityFactionComponent.h>
#include <drx3D/AI/FactionSystem.h>

namespace sxema
{
// Forward declare interfaces.
struct IEnvRegistrar;
// Forward declare structures.
struct SUpdateContext;
}

class CEntityAIFactionComponent final : public IEntityFactionComponent
{
public:
	struct SReactionChangedSignal
	{
		static void ReflectType(sxema::CTypeDesc<SReactionChangedSignal>& typeInfo);
		SFactionID m_otherFaction;
		IFactionMap::ReactionType m_reactionType;
	};

	static void ReflectType(sxema::CTypeDesc<CEntityAIFactionComponent>& desc);
	static void Register(sxema::IEnvRegistrar& registrar);

	CEntityAIFactionComponent();
	virtual ~CEntityAIFactionComponent();

	// IEntityComponent
	virtual void Initialize() override;
	virtual void OnShutDown() override;
	// ~IEntityComponent

	// IEntityComponent
	virtual u8 GetFactionId() const override;
	virtual void SetFactionId(u8k factionId) override;
	virtual IFactionMap::ReactionType GetReaction(const EntityId otherEntityId) const override;
	virtual void SetReactionChangedCallback(std::function<void(u8k, const IFactionMap::ReactionType)> callbackFunction) override;
	// IEntityComponent

private:
	SFactionID GetFactionIdSchematyc() const;
	void SetFactionIdSchematyc(const SFactionID& factionId);
	bool IsReactionEqual(sxema::ExplicitEntityId otherEntity, IFactionMap::ReactionType reaction) const;
	SFactionFlagsMask GetFactionMaskByReaction(IFactionMap::ReactionType reactionType) const;

	void OnReactionChanged(u8 factionId, IFactionMap::ReactionType reaction);

	void RegisterFactionId(const SFactionID& factionId);

	SFactionID m_factionId;
	std::function<void(u8k, const IFactionMap::ReactionType)> m_reactionChangedCallback;
};
