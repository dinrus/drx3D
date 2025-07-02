// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/FactionComponent.h>

#include <drx3D/AI/FactionSystemSchematyc.h>

void CEntityAIFactionComponent::SReactionChangedSignal::ReflectType(sxema::CTypeDesc<SReactionChangedSignal>& desc)
{
	desc.SetGUID("2c54532e-7b62-4fde-8c98-eff0e9ed2b6b"_drx_guid);
	desc.SetLabel("ReactionChanged");
	desc.SetDescription("Sent when reaction with other faction has changed for this entity.");
	desc.AddMember(&SReactionChangedSignal::m_otherFaction, 'of', "otherFaction", "Faction", nullptr, SFactionID());
	desc.AddMember(&SReactionChangedSignal::m_reactionType, 'rt', "reactionType", "Reaction Type", nullptr, IFactionMap::ReactionType::Hostile);
}

CEntityAIFactionComponent::CEntityAIFactionComponent()
{
}

CEntityAIFactionComponent::~CEntityAIFactionComponent()
{
}

void CEntityAIFactionComponent::ReflectType(sxema::CTypeDesc<CEntityAIFactionComponent>& desc)
{
	desc.AddBase<IEntityFactionComponent>();
	desc.SetGUID("214527d9-156b-4a50-90a5-3dc4a362d800"_drx_guid);

	desc.SetLabel("AI Faction");
	desc.SetDescription("Faction component");
	desc.SetEditorCategory("AI");
	desc.SetIcon("icons:Navigation/Move_Classic.ico");
	desc.SetComponentFlags({ IEntityComponent::EFlags::Singleton, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach });

	desc.AddMember(&CEntityAIFactionComponent::m_factionId, 'fid', "FactionId", "Faction", nullptr, SFactionID());
}

void CEntityAIFactionComponent::Register(sxema::IEnvRegistrar& registrar)
{
	sxema::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		sxema::CEnvRegistrationScope componentScope = scope.Register(SXEMA_MAKE_ENV_COMPONENT(CEntityAIFactionComponent));

		// Functions
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&CEntityAIFactionComponent::SetFactionIdSchematyc, "504e4eee-1633-4e17-92e8-0967edb4a3ae"_drx_guid, "SetFaction");
			pFunction->SetDescription("Sets new faction for entity");
			pFunction->SetFlags(sxema::EEnvFunctionFlags::Construction);
			pFunction->BindInput(1, 'fid', "FactionId", "New faction", Vec3(ZERO));
			componentScope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&CEntityAIFactionComponent::GetFactionIdSchematyc, "d700b459-bada-4c65-9287-8a9e2feed7ff"_drx_guid, "GetFaction");
			pFunction->SetDescription("Gets entity faction");
			pFunction->SetFlags({ sxema::EEnvFunctionFlags::Construction , sxema::EEnvFunctionFlags::Const });
			pFunction->BindOutput(0, 'fid', "FactionId", "New faction");
			componentScope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&CEntityAIFactionComponent::IsReactionEqual, "aafa3f76-a304-4eb7-b6ad-7d8e52698969"_drx_guid, "CompareReactionWithEntity");
			pFunction->SetDescription("Returns true if reaction to faction of the other entity equals to specified reaction (Enemy, Neutral, Friend)");
			pFunction->BindInput(1, 'eid', "EntityId", "Id of the other entity");
			pFunction->BindInput(2, 'rt', "ReactionType", "Reaction type to compare with");
			pFunction->BindOutput(0, 'rv', "Equals", "True if reaction between current and other entity equals to specified reaction");
			componentScope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&CEntityAIFactionComponent::GetFactionMaskByReaction, "5dcb72d4-ec97-43d4-843d-fd74b9088568"_drx_guid, "GetFactionMaskByReaction");
			pFunction->SetDescription("Gets mask of factions that has specified reaction to this faction");
			pFunction->BindInput(1, 'rt', "ReactionType", "Reaction type to compare with");
			pFunction->BindOutput(0, 'fm', "FactionsMask", "Mask of factions");
			componentScope.Register(pFunction);
		}
		// Signals
		{
			componentScope.Register(SXEMA_MAKE_ENV_SIGNAL(SReactionChangedSignal));
		}
	}
}

void CEntityAIFactionComponent::Initialize()
{
	RegisterFactionId(m_factionId);
}

void CEntityAIFactionComponent::OnShutDown()
{
	RegisterFactionId(SFactionID());
}

u8 CEntityAIFactionComponent::GetFactionId() const
{
	return m_factionId.id;
}

void CEntityAIFactionComponent::SetFactionId(u8k factionId)
{
	m_factionId.id = factionId;
}

IFactionMap::ReactionType CEntityAIFactionComponent::GetReaction(const EntityId otherEntityId) const
{
	CFactionSystem* pFactionSystem = GetAISystem()->GetFactionSystem();
	const SFactionID otherFaction = pFactionSystem->GetEntityFaction(otherEntityId);
	return pFactionSystem->GetFactionMap()->GetReaction(m_factionId.id, otherFaction.id);
}

void CEntityAIFactionComponent::SetReactionChangedCallback(std::function<void(u8k, const IFactionMap::ReactionType)> callbackFunction)
{
	m_reactionChangedCallback = callbackFunction;
}

SFactionID CEntityAIFactionComponent::GetFactionIdSchematyc() const
{
	return m_factionId;
}

void CEntityAIFactionComponent::SetFactionIdSchematyc(const SFactionID& factionId)
{
	if (factionId.id != m_factionId.id)
	{
		m_factionId = factionId;
		RegisterFactionId(factionId);
	}
}

void CEntityAIFactionComponent::RegisterFactionId(const SFactionID& factionId)
{
	GetAISystem()->GetFactionSystem()->SetEntityFaction(
		GetEntity()->GetId(),
		factionId,
		factionId.IsValid() ? functor(*this, &CEntityAIFactionComponent::OnReactionChanged) : CFactionSystem::ReactionChangedCallback());
}

bool CEntityAIFactionComponent::IsReactionEqual(sxema::ExplicitEntityId otherEntity, IFactionMap::ReactionType reactionType) const
{
	CFactionSystem* pFactionSystem = GetAISystem()->GetFactionSystem();
	SFactionID otherFaction = pFactionSystem->GetEntityFaction(static_cast<EntityId>(otherEntity));
	return pFactionSystem->GetFactionMap()->GetReaction(m_factionId.id, otherFaction.id) == reactionType;
}

SFactionFlagsMask CEntityAIFactionComponent::GetFactionMaskByReaction(IFactionMap::ReactionType reactionType) const
{
	return GetAISystem()->GetFactionSystem()->GetFactionMaskByReaction(m_factionId, reactionType);
}

void CEntityAIFactionComponent::OnReactionChanged(u8 factionId, IFactionMap::ReactionType reaction)
{
	if (GetEntity()->GetSchematycObject())
	{
		GetEntity()->GetSchematycObject()->ProcessSignal(SReactionChangedSignal{ factionId, reaction });
	}

	if (m_reactionChangedCallback)
	{
		m_reactionChangedCallback(factionId, reaction);
	}
}

