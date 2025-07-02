// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/ListenerComponent.h>

#include <drx3D/AI/FactionComponent.h>
#include <drx3D/AI/FactionSystemSchematyc.h>

struct SListenerUserConditionSignal
{
	static void ReflectType(sxema::CTypeDesc<SListenerUserConditionSignal>& desc)
	{
		desc.SetGUID("B78D6EB0-FA2C-422F-ABE9-48481894FAF0"_drx_guid);
		desc.SetLabel("ListenerCustomFilter");

		desc.AddMember(&SListenerUserConditionSignal::position, 'pos', "Position", "Position", nullptr, ZERO);
		desc.AddMember(&SListenerUserConditionSignal::sourceEntityId, 'seid', "SenderEntity", "SenderEntity", nullptr, sxema::ExplicitEntityId::Invalid);
		desc.AddMember(&SListenerUserConditionSignal::faction, 'fac', "Faction", "Faction", nullptr, SFactionID());
	}
	Vec3 position;
	sxema::ExplicitEntityId sourceEntityId;
	SFactionID faction;
};

struct SSoundHeardSignal
{
	static void ReflectType(sxema::CTypeDesc<SSoundHeardSignal>& desc)
	{
		desc.SetGUID("e59c9627-d739-4a7a-860e-ee5f9a62540f"_drx_guid);
		desc.SetLabel("SoundHeard");

		desc.AddMember(&SSoundHeardSignal::position, 'pos', "Position", "Position", nullptr, ZERO);
		desc.AddMember(&SSoundHeardSignal::sourceEntityId, 'seid', "SenderEntity", "SenderEntity", nullptr, sxema::ExplicitEntityId::Invalid);
		desc.AddMember(&SSoundHeardSignal::faction, 'fac', "Faction", "Faction", nullptr, SFactionID());
	}

	Vec3 position;
	sxema::ExplicitEntityId sourceEntityId;
	SFactionID faction;
};

CEntityAIListenerComponent::CEntityAIListenerComponent()
	: m_entityEventMask(ENTITY_EVENT_BIT(ENTITY_EVENT_RESET) | ENTITY_EVENT_BIT(ENTITY_EVENT_START_GAME))
{

}

CEntityAIListenerComponent::~CEntityAIListenerComponent()
{

}

void CEntityAIListenerComponent::ReflectType(sxema::CTypeDesc<CEntityAIListenerComponent>& desc)
{
	desc.AddBase<IEntityListenerComponent>();
	desc.SetGUID("B8A61764-31CC-4B76-A6CF-05F754E62024"_drx_guid);

	desc.SetLabel("AI Listener");
	desc.SetDescription("Listener component");
	desc.SetEditorCategory("AI");
	desc.SetIcon("icons:Navigation/Move_Classic.ico");
	desc.SetComponentFlags({ IEntityComponent::EFlags::Transform, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach });
	desc.AddComponentInteraction(SEntityComponentRequirements::EType::SoftDependency, IEntity::SComponentType<CEntityAIFactionComponent>::GetGUID());

	desc.AddMember(&CEntityAIListenerComponent::m_factionsToListen, 'ftl', "factionsToListen", "Factions To Listen To",
		"Only sound stimuli whose source entities belong to these factions will be processed.", SFactionFlagsMask());
	desc.AddMember(&CEntityAIListenerComponent::m_distanceScale, 'dsc', "listeningDistanceScale", "Listening Distance Scale",
		"An optional scale that can be applied to increase or reduce the listening distance.", 1.0f);
	desc.AddMember(&CEntityAIListenerComponent::m_earLocations, 'eloc', "earLocations", "Ear Locations", 
		"A collection of locations that will be determined if the entity is within hearing range of sound stimuli. \
		Note: can also be used to cast rays to these locations to determine sound obstruction fall-offs and such.", Perception::ComponentHelpers::SLocationsArray());
	desc.AddMember(&CEntityAIListenerComponent::m_bUserCustomCondition, 'cfc', "useCustomFilter", "Use Custom Filter", 
		"Flag for enabling custom condition filter.", false);
}

void CEntityAIListenerComponent::Register(sxema::IEnvRegistrar& registrar)
{
	sxema::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		sxema::CEnvRegistrationScope componentScope = scope.Register(SXEMA_MAKE_ENV_COMPONENT(CEntityAIListenerComponent));

		// Functions
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&CEntityAIListenerComponent::ListenerUserConditionResult, "554468AD-C27D-4D58-8145-33060E6B1929"_drx_guid, "ReturnUserConditionResult");
			pFunction->SetDescription("Return true if the listener should continue checking if it can hear the sound stimulus.");
			pFunction->BindInput(1, 'res', "IsAudible", "Is Audible?", false);
			componentScope.Register(pFunction);
		}

		// Signals
		{
			componentScope.Register(SXEMA_MAKE_ENV_SIGNAL(SSoundHeardSignal));
			componentScope.Register(SXEMA_MAKE_ENV_SIGNAL(SListenerUserConditionSignal));
		}
	}
}

void CEntityAIListenerComponent::OnShutDown()
{
	Reset(EEntitySimulationMode::Idle);
}

void CEntityAIListenerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_UPDATE:
	case ENTITY_EVENT_XFORM:
		Update();
		break;
	case ENTITY_EVENT_RESET:
		Reset(GetEntity()->GetSimulationMode());
		break;
	case ENTITY_EVENT_START_GAME:
	{
		m_entityEventMask |= IsUsingBones() ? ENTITY_EVENT_BIT(ENTITY_EVENT_UPDATE) : ENTITY_EVENT_BIT(ENTITY_EVENT_XFORM);
		GetEntity()->UpdateComponentEventMask(this);
		break;
	}
	}
}

void CEntityAIListenerComponent::Reset(EEntitySimulationMode simulationMode)
{
	if (simulationMode == EEntitySimulationMode::Game)
	{
		RegisterToAuditionMap();
	}
	else
	{
		UnregisterFromAuditionMap();
		m_entityEventMask = ENTITY_EVENT_BIT(ENTITY_EVENT_RESET) | ENTITY_EVENT_BIT(ENTITY_EVENT_START_GAME);
	}
}

void CEntityAIListenerComponent::RegisterToAuditionMap()
{
	IAuditionMap* pAuditionMap = gEnv->pAISystem->GetAuditionMap();
	if (!pAuditionMap)
		return;
	
	IEntity* pEntity = GetEntity();

	m_params.listeningDistanceScale = m_distanceScale;
	m_params.factionsToListenMask = m_factionsToListen.mask;
	m_params.onSoundHeardCallback = functor(*this, &CEntityAIListenerComponent::OnSoundHeard);

	if (m_bUserCustomCondition || m_userConditionCallback)
	{
		m_params.userConditionCallback = functor_ret(*this, &CEntityAIListenerComponent::OnListenerUserCondition);
	}

	for (const Perception::ComponentHelpers::SLocation& location : m_earLocations.locations)
	{
		location.Validate(pEntity, "AI Listener Component");
	}
	SyncWithEntity();

	pAuditionMap->RegisterListener(pEntity->GetId(), m_params);

	m_changeHintFlags = Perception::ListenerParamsChangeOptions::Value();
	m_registeredEntityId = pEntity->GetId();
}

void CEntityAIListenerComponent::UnregisterFromAuditionMap()
{
	if (IsRegistered())
	{
		gEnv->pAISystem->GetAuditionMap()->UnregisterListner(m_registeredEntityId);
		m_registeredEntityId = INVALID_ENTITYID;
	}
}

void CEntityAIListenerComponent::Update()
{
	if (IsRegistered())
	{
		SyncWithEntity();
		UpdateChange();
	}
}

bool CEntityAIListenerComponent::IsUsingBones() const
{
	for (const Perception::ComponentHelpers::SLocation& location : m_earLocations.locations)
	{
		if (location.type == Perception::ComponentHelpers::SLocation::EType::Bone)
			return true;
	}
	return false;
}

void CEntityAIListenerComponent::ListenerUserConditionResult(bool bResult)
{
	m_userConditionResult = bResult;
}

bool CEntityAIListenerComponent::OnListenerUserCondition(const Perception::SSoundStimulusParams& soundParams)
{
	bool userConditionResult = false;
	if (m_bUserCustomCondition && GetEntity()->GetSchematycObject())
	{
		SListenerUserConditionSignal signal;
		signal.position = soundParams.position;
		signal.faction = SFactionID(soundParams.faction);
		signal.sourceEntityId = sxema::ExplicitEntityId(soundParams.sourceEntityId);

		GetEntity()->GetSchematycObject()->ProcessSignal(signal, GetGUID());

		userConditionResult |= m_userConditionResult;
	}
	if (m_userConditionCallback)
	{
		userConditionResult |= m_userConditionCallback(soundParams);
	}
	return userConditionResult;
}

void CEntityAIListenerComponent::OnSoundHeard(const Perception::SSoundStimulusParams& soundParams)
{
	if (GetEntity()->GetSchematycObject())
	{
		SSoundHeardSignal signal;
		signal.position = soundParams.position;
		signal.faction = SFactionID(soundParams.faction);
		signal.sourceEntityId = sxema::ExplicitEntityId(soundParams.sourceEntityId);

		GetEntity()->GetSchematycObject()->ProcessSignal(signal, GetGUID());
	}

	if (m_soundHeardCallback)
	{
		m_soundHeardCallback(soundParams);
	}
}

void CEntityAIListenerComponent::SyncWithEntity()
{
	m_params.ears.clear();
	size_t earsCount = m_earLocations.locations.size();
	if (earsCount > Perception::SListenerParams::MaxEarsCount)
	{
		earsCount = Perception::SListenerParams::MaxEarsCount;
	}

	Vec3 earPosition;
	for (size_t earIdx = 0; earIdx < earsCount; ++earIdx)
	{
		m_earLocations.locations[earIdx].GetPositionAndOrientation(GetEntity(), &earPosition, nullptr);
		m_params.ears.push_back(Perception::SListenerEarParams(earPosition));
	}
	m_changeHintFlags = Perception::ListenerParamsChangeOptions::ChangedEars;
}

void CEntityAIListenerComponent::UpdateChange()
{
	gEnv->pAISystem->GetAuditionMap()->ListenerChanged(m_registeredEntityId, m_params, m_changeHintFlags);
	m_changeHintFlags = Perception::ListenerParamsChangeOptions::Value();
}
