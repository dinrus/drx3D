// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptBase.h>

#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/Decorators/ActionButton.h>
#include <drx3D/Schema/ICore.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvClass.h>
#include <drx3D/Schema/IEnvComponent.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/IScriptClass.h>
#include <drx3D/Schema/IScriptComponentInstance.h>
#include <drx3D/Schema/IScriptVariable.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/DrxLinkUtils.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema
{
// #SchematycTODO : Need to set NotCopyable flag on everything we create from base!!!

CScriptBase::CScriptBase()
	: CScriptElementBase({ EScriptElementFlags::NotCopyable, EScriptElementFlags::FixedName })
{}

CScriptBase::CScriptBase(const DrxGUID& guid, const SElementId& classId)
	: CScriptElementBase(guid, nullptr, { EScriptElementFlags::NotCopyable, EScriptElementFlags::FixedName })
	, m_classId(classId)
{
}

void CScriptBase::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const
{
	SXEMA_CORE_ASSERT(enumerator);
	if (enumerator)
	{
		if (m_classId.domain == EDomain::Script)
		{
			enumerator(m_classId.guid);
		}
	}
}

void CScriptBase::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	if (m_classId.domain == EDomain::Script)
	{
		m_classId.guid = guidRemapper.Remap(m_classId.guid);
	}
}

void CScriptBase::ProcessEvent(const SScriptEvent& event)
{
	RefreshFlags refreshFlags;
	switch (event.id)
	{
	case EScriptEventId::FileReload:
		{
			refreshFlags.Add(ERefreshFlags::All);
			break;
		}
	case EScriptEventId::EditorAdd:
		{
			refreshFlags.Add(ERefreshFlags::All);
			break;
		}
	case EScriptEventId::EditorFixUp:
		{
			refreshFlags.Add({ ERefreshFlags::Variables, ERefreshFlags::ComponentInstances });
			break;
		}
	case EScriptEventId::EditorPaste:
	case EScriptEventId::EditorDependencyModified:
		{
			// TODO : How do we track when new elements are added to base?
			refreshFlags.Add(RefreshFlags({ ERefreshFlags::Name, ERefreshFlags::Variables, ERefreshFlags::ComponentInstances }));
			// ~TODO
			break;
		}
	}
	if (!refreshFlags.IsEmpty())
	{
		Refresh(refreshFlags);
	}
}

void CScriptBase::Serialize(Serialization::IArchive& archive)
{
	// #SchematycTODO : Shouldn't this be handled by CScriptElementBase itself?
	CScriptElementBase::Serialize(archive);
	CMultiPassSerializer::Serialize(archive);
	CScriptElementBase::SerializeExtensions(archive);
}

SElementId CScriptBase::GetClassId() const
{
	return m_classId;
}

CAnyConstPtr CScriptBase::GetEnvClassProperties() const
{
	return m_pEnvClassProperties.get();
}

void CScriptBase::LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_classId, "classId");
}

void CScriptBase::Load(Serialization::IArchive& archive, const ISerializationContext& context)
{
	Refresh({ ERefreshFlags::Name, ERefreshFlags::EnvClassProperties, ERefreshFlags::Variables });
	if (m_pEnvClassProperties)
	{
		archive(*m_pEnvClassProperties, "envClassProperties");
	}
}

void CScriptBase::Save(Serialization::IArchive& archive, const ISerializationContext& context)
{
	archive(m_classId, "classId");
	if (m_pEnvClassProperties)
	{
		archive(*m_pEnvClassProperties, "envClassProperties");
	}
}

void CScriptBase::Edit(Serialization::IArchive& archive, const ISerializationContext& context)
{
	if (m_pEnvClassProperties)
	{
		archive(*m_pEnvClassProperties, "envClassProperties", "Properties");
	}

	if (m_classId.domain == EDomain::Script)
	{
		archive(Serialization::ActionButton(functor(*this, &CScriptBase::GoToClass)), "goToClass", "^Go To Class");
	}
}

void CScriptBase::Validate(Serialization::IArchive& archive, const ISerializationContext& context)
{
	switch (m_classId.domain)
	{
	case EDomain::Env:
		{
			if (!gEnv->pSchematyc->GetEnvRegistry().GetClass(m_classId.guid))
			{
				archive.error(*this, "Unable to retrieve base class!");
			}
			break;
		}
	case EDomain::Script:
		{
			if (!DynamicCast<IScriptClass>(gEnv->pSchematyc->GetScriptRegistry().GetElement(m_classId.guid)))
			{
				archive.error(*this, "Unable to retrieve base class!");
			}
			break;
		}
	}
}

void CScriptBase::Refresh(const RefreshFlags& flags)
{
	switch (m_classId.domain)
	{
	case EDomain::Env:
		{
			const IEnvClass* pEnvClass = gEnv->pSchematyc->GetEnvRegistry().GetClass(m_classId.guid);
			if (pEnvClass)
			{
				if (flags.Check(ERefreshFlags::Name))
				{
					CStackString name = "Base [";
					name.append(pEnvClass->GetName());
					name.append("]");
					CScriptElementBase::SetName(name.c_str());
				}

				if (flags.Check(ERefreshFlags::EnvClassProperties))
				{
					CAnyConstPtr pEnvClassProperties = pEnvClass->GetProperties();
					if (pEnvClassProperties)
					{
						m_pEnvClassProperties = CAnyValue::CloneShared(*pEnvClassProperties);
					}
				}

				if (flags.Check(ERefreshFlags::ComponentInstances))
				{
					RefreshComponentInstances(*pEnvClass);
				}
			}
			break;
		}
	case EDomain::Script:
		{
			const IScriptClass* pScriptClass = DynamicCast<const IScriptClass>(gEnv->pSchematyc->GetScriptRegistry().GetElement(m_classId.guid));
			if (pScriptClass)
			{
				if (flags.Check(ERefreshFlags::Name))
				{
					CStackString name = "Base [";
					name.append(pScriptClass->GetName());
					name.append("]");
					CScriptElementBase::SetName(name.c_str());
				}

				// TODO: Not fully working yet.
				/*if (flags.Check(ERefreshFlags::Variables))
				   {
				   RefreshVariables(*pScriptClass);
				   }*/
				// ~TODO
			}
			break;
		}
	}
}

void CScriptBase::RefreshVariables(const IScriptClass& scriptClass)
{
	typedef std::unordered_map<DrxGUID, const IScriptVariable*> BaseVariables;
	typedef std::unordered_map<DrxGUID, IScriptVariable*>       DerivedVariables;

	// Collect base variables.
	BaseVariables baseVariables;
	auto collectBaseVariables = [&baseVariables](const IScriptElement& scriptElement) -> EVisitStatus
	{
		switch (scriptElement.GetType())
		{
		case EScriptElementType::Variable: // #SchematycTODO : Make sure variable is public/protected?
			{
				const IScriptVariable& baseVariable = DynamicCast<IScriptVariable>(scriptElement);
				baseVariables.insert(BaseVariables::value_type(baseVariable.GetGUID(), &baseVariable));
				return EVisitStatus::Continue;
			}
		case EScriptElementType::ComponentInstance:
			{
				// TODO: Not implemented yet!
			}
		default:
			{
				return EVisitStatus::Continue;
			}
		}
		return EVisitStatus::Continue;
	};
	scriptClass.VisitChildren(collectBaseVariables);

	// Collect derived variables.
	DerivedVariables derivedVariables;
	auto collectDerivedVariables = [&derivedVariables](IScriptElement& scriptElement) -> EVisitStatus
	{
		switch (scriptElement.GetType())
		{
		case EScriptElementType::Variable: // #SchematycTODO : Make sure variable is public/protected?
			{
				IScriptVariable& derivedVariable = DynamicCast<IScriptVariable>(scriptElement);
				derivedVariables.insert(DerivedVariables::value_type(derivedVariable.GetBaseGUID(), &derivedVariable));
				return EVisitStatus::Continue;
			}
		default:
			{
				return EVisitStatus::Continue;
			}
		}
	};
	CScriptElementBase::VisitChildren(collectDerivedVariables);

	IScriptRegistry& scriptRegistry = gEnv->pSchematyc->GetScriptRegistry();

	// Remove all derived variables that are either finalized of no longer part of base class.
	for (DerivedVariables::value_type& derivedVariable : derivedVariables)
	{
		BaseVariables::iterator itBaseVariable = baseVariables.find(derivedVariable.first);
		if ((itBaseVariable == baseVariables.end()) || (itBaseVariable->second->GetOverridePolicy() == EOverridePolicy::Final))
		{
			scriptRegistry.RemoveElement(derivedVariable.second->GetGUID());
		}
	}

	// Create or update derived variables.
	for (const BaseVariables::value_type& baseVariable : baseVariables)
	{
		if (baseVariable.second->GetOverridePolicy() != EOverridePolicy::Final)
		{
			DerivedVariables::iterator itDerivedVariable = derivedVariables.find(baseVariable.first);
			if (itDerivedVariable != derivedVariables.end())
			{
				itDerivedVariable->second->SetName(baseVariable.second->GetName());
			}
			else
			{
				scriptRegistry.AddVariable(baseVariable.second->GetName(), baseVariable.second->GetTypeId(), baseVariable.first, this);
			}
		}
	}
}

void CScriptBase::RefreshComponentInstances(const IEnvClass& envClass)
{
	typedef std::unordered_map<DrxGUID, const IEnvComponent*>      BaseComponentInstances;
	typedef std::unordered_map<DrxGUID, IScriptComponentInstance*> DerivedComponentInstances;

	IEnvRegistry& envRegistry = gEnv->pSchematyc->GetEnvRegistry();

	// Collect base component instances.
	u32k baseComponentCount = envClass.GetComponentCount();
	BaseComponentInstances baseComponentInstances;
	baseComponentInstances.reserve(baseComponentCount);
	for (u32 baseComponentIdx = 0; baseComponentIdx < baseComponentCount; ++baseComponentIdx)
	{
		const DrxGUID baseComponentTypeGUID = envClass.GetComponentTypeGUID(baseComponentIdx);
		const IEnvComponent* pEnvComponent = envRegistry.GetComponent(baseComponentTypeGUID);
		SXEMA_CORE_ASSERT(pEnvComponent);
		if (pEnvComponent)
		{
			baseComponentInstances.insert(BaseComponentInstances::value_type(baseComponentTypeGUID, pEnvComponent));
		}
	}

	// Collect derived component instances.
	DerivedComponentInstances derivedComponentInstances;
	auto collectDerivedComponentInstances = [&derivedComponentInstances](IScriptElement& scriptElement) -> EVisitStatus
	{
		switch (scriptElement.GetType())
		{
		case EScriptElementType::ComponentInstance:
			{
				IScriptComponentInstance& derivedComponentInstance = DynamicCast<IScriptComponentInstance>(scriptElement);
				derivedComponentInstances.insert(DerivedComponentInstances::value_type(derivedComponentInstance.GetTypeGUID(), &derivedComponentInstance));
				return EVisitStatus::Continue;
			}
		default:
			{
				return EVisitStatus::Continue;
			}
		}
	};
	CScriptElementBase::VisitChildren(collectDerivedComponentInstances);

	IScriptRegistry& scriptRegistry = gEnv->pSchematyc->GetScriptRegistry();

	// Remove all derived component instances that are either finalized of no longer part of base class.
	for (DerivedComponentInstances::value_type& derivedComponent : derivedComponentInstances)
	{
		BaseComponentInstances::iterator itBaseComponent = baseComponentInstances.find(derivedComponent.first);
		if (itBaseComponent == baseComponentInstances.end())
		{
			scriptRegistry.RemoveElement(derivedComponent.second->GetGUID());
		}
	}

	// Create or update derived component instances.
	for (const BaseComponentInstances::value_type& baseComponent : baseComponentInstances)
	{
		DerivedComponentInstances::iterator itDerivedComponent = derivedComponentInstances.find(baseComponent.first);
		if (itDerivedComponent != derivedComponentInstances.end())
		{
			itDerivedComponent->second->SetName(baseComponent.second->GetName());
		}
		else
		{
			scriptRegistry.AddComponentInstance(baseComponent.second->GetName(), baseComponent.first, this);
		}
	}
}

void CScriptBase::GoToClass()
{
	DrxLinkUtils::ExecuteCommand(DrxLinkUtils::ECommand::Show, m_classId.guid);
}
} // sxema
