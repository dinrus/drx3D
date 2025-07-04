// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/BehaviorTreeComponent.h>
#include <drx3D/AI/BehaviorTreeUpr.h>
#include <drx3D/AI/Environment.h>

// *INDENT-OFF* - <hard to read code and declarations due to inconsistent indentation>


void CEntityAIBehaviorTreeComponent::CBehaviorTreeName::ReflectType(sxema::CTypeDesc<CBehaviorTreeName>& desc)
{
	desc.SetGUID("97fb53c6-01ab-4c5f-8429-1d07b4ccfed2"_drx_guid);
	desc.SetLabel("BehaviorTreeName");
	desc.SetDescription("Behavior Tree Name");
}

static void _EnumerateBehaviorTreeFilesOnDisk(tukk szDirectory, std::vector<string>& outFileList)
{
	const DrxPathString folder = PathUtil::AddSlash(szDirectory);
	const DrxPathString search = PathUtil::Make(folder, DrxPathString("*.xml"));

	IDrxPak* pPak = gEnv->pDrxPak;

	_finddata_t fd;
	intptr_t handle = pPak->FindFirst(search.c_str(), &fd);

	if (handle == -1)
		return;

	do
	{
		if (!strcmp(fd.name, ".") || !strcmp(fd.name, ".."))
			continue;

		const DrxPathString fdName(fd.name);

		if (fd.attrib & _A_SUBDIR)
		{
			const DrxPathString subName = PathUtil::Make(folder, PathUtil::AddSlash(fdName));
			_EnumerateBehaviorTreeFilesOnDisk(subName.c_str(), outFileList);
		}
		else
		{
			const DrxPathString fileName = PathUtil::Make(folder, fdName);
			outFileList.push_back(fileName.c_str());
		}

	} while (pPak->FindNext(handle, &fd) >= 0);
}

static void EnumerateBehaviorTreeFilesOnDisk(tukk szDirectory, std::vector<string>& outFileList)
{
	_EnumerateBehaviorTreeFilesOnDisk(szDirectory, outFileList);

	const size_t dirLen = strlen(szDirectory);

	// strip leading directory and file extension
	for (string& fn : outFileList)
	{
		fn = fn.substr(dirLen);
		fn = PathUtil::RemoveExtension(fn.c_str());
	}
}

bool CEntityAIBehaviorTreeComponent::CBehaviorTreeName::Serialize(Serialization::IArchive& archive)
{
	tukk szName = "btname", *szLabel = "Name";

	if (archive.isEdit())
	{
		std::vector<string> filePaths;
		EnumerateBehaviorTreeFilesOnDisk("Scripts/AI/BehaviorTrees/", filePaths);

		Serialization::StringList fileNamesList;
		fileNamesList.reserve(filePaths.size() + 1);
		fileNamesList.push_back("");

		for (u32 i = 0; i < filePaths.size(); ++i)
		{
			fileNamesList.push_back(filePaths[i].c_str());
		}

		bool bResult = false;
		if (archive.isInput())
		{
			Serialization::StringListValue temp(fileNamesList, 0);
			bResult = archive(temp, szName, szLabel);
			m_filePathWithoutExtension = temp.c_str();
		}
		else if (archive.isOutput())
		{
			i32k pos = fileNamesList.find(m_filePathWithoutExtension.c_str());
			bResult = archive(Serialization::StringListValue(fileNamesList, pos), szName, szLabel);
		}
		return bResult;
	}
	else
	{
		return archive(m_filePathWithoutExtension, szName, szLabel);
	}
}

CEntityAIBehaviorTreeComponent::CEntityAIBehaviorTreeComponent()
	: m_bBehaviorTreeShouldBeRunning(false)
	, m_bBehaviorTreeIsRunning(false)
{
	// nothing
}

void CEntityAIBehaviorTreeComponent::OnShutDown()
{
	EnsureBehaviorTreeIsStopped();
}

uint64 CEntityAIBehaviorTreeComponent::GetEventMask() const
{
	return
		ENTITY_EVENT_BIT(ENTITY_EVENT_COMPONENT_PROPERTY_CHANGED) |
		ENTITY_EVENT_BIT(ENTITY_EVENT_RESET) |
		ENTITY_EVENT_BIT(ENTITY_EVENT_START_GAME);
}

void CEntityAIBehaviorTreeComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_COMPONENT_PROPERTY_CHANGED:
		// Unfortunately, the promised IEntityComponent pointer and member-ID are not always set in the event, so we cannot reliably react when only the
		// behavior tree has changed. Consequently we simply *always* restart the behavior tree (when it was running before).
		if (m_bBehaviorTreeShouldBeRunning)
		{
			// re-start with the changed behavior tree
			EnsureBehaviorTreeIsStopped();
			EnsureBehaviorTreeIsRunning();
		}
		break;

	case ENTITY_EVENT_RESET:
		// entering game-mode?
		if (event.nParam[0])
		{
			// yup
			EnsureBehaviorTreeIsRunning();
		}
		else
		{
			// leaving game-mode
			EnsureBehaviorTreeIsStopped();
		}
		break;

	case ENTITY_EVENT_START_GAME:
		EnsureBehaviorTreeIsRunning();
		break;
	}
}

void CEntityAIBehaviorTreeComponent::ReflectType(sxema::CTypeDesc<CEntityAIBehaviorTreeComponent>& desc)
{
	desc.AddBase<IEntityBehaviorTreeComponent>();
	desc.SetGUID("c66c0266-b634-4dc4-9e69-899f69f9aabb"_drx_guid);
	desc.SetLabel("AI Behavior Tree");
	desc.SetDescription("Behavior Tree Component");
	desc.SetEditorCategory("AI");
	desc.SetIcon("icons:Navigation/Move_Classic.ico");
	desc.SetComponentFlags({ EEntityComponentFlags::Singleton });
	desc.AddMember(&CEntityAIBehaviorTreeComponent::m_behaviorTreeName, 'btna', "behaviorTreeName", "Behavior Tree Name", nullptr, CBehaviorTreeName());
}

void CEntityAIBehaviorTreeComponent::Register(sxema::IEnvRegistrar& registrar)
{
	sxema::CEnvRegistrationScope entityScope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		sxema::CEnvRegistrationScope componentScope = entityScope.Register(SXEMA_MAKE_ENV_COMPONENT(CEntityAIBehaviorTreeComponent));
		{
			//
			// Functions
			//

			{
				auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&CEntityAIBehaviorTreeComponent::SchematycFunction_SendEvent, "4fd411d7-4af7-45b3-8a33-992e12ac5f53"_drx_guid, "SendEvent");
				pFunction->SetDescription("Sends an arbitrary event (in the form of a string) to the Behavior Tree.");
				pFunction->BindInput(1, 'evnt', "Event", "Name of the event to send to the Behavior Tree", sxema::CSharedString(""));
				componentScope.Register(pFunction);
			}

			{
				auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&CEntityAIBehaviorTreeComponent::SchematycFunction_SetBBKeyValue<Vec3>, "ea9a42e2-5c48-48e3-8678-4513f9794e19"_drx_guid, "SetBBVec3");
				pFunction->SetDescription("Writes a Vec3 to the Behavior Tree's blackboard.");
				pFunction->BindInput(1, 'key ', "Key", "Name of the entry in the blackboard under which to store the value.");
				pFunction->BindInput(2, 'valu', "Value", "Value of the entry to write to the blackboard.");
				componentScope.Register(pFunction);
			}
			// TODO: register more SchematycFunction_SetBBKeyValue<>() functions as needed

		}
	}
}

void CEntityAIBehaviorTreeComponent::EnsureBehaviorTreeIsRunning()
{
	m_bBehaviorTreeShouldBeRunning = true;

	if (!m_bBehaviorTreeIsRunning)
	{
		m_bBehaviorTreeIsRunning = gAIEnv.pBehaviorTreeUpr->StartModularBehaviorTree(GetEntityId(), m_behaviorTreeName.GetFilePathWithoutExtension());
	}
}

void CEntityAIBehaviorTreeComponent::EnsureBehaviorTreeIsStopped()
{
	m_bBehaviorTreeShouldBeRunning = false;

	if (m_bBehaviorTreeIsRunning)
	{
		gAIEnv.pBehaviorTreeUpr->StopModularBehaviorTree(GetEntityId());
		m_bBehaviorTreeIsRunning = false;
	}
}

void CEntityAIBehaviorTreeComponent::SendEvent(tukk szEventName)
{
	if (m_bBehaviorTreeIsRunning)
	{
		BehaviorTree::Event ev(szEventName);
		gAIEnv.pBehaviorTreeUpr->HandleEvent(GetEntityId(), ev);
	}
}

void CEntityAIBehaviorTreeComponent::SchematycFunction_SendEvent(const sxema::CSharedString& eventName)
{
	if (m_bBehaviorTreeIsRunning)
	{
		BehaviorTree::Event ev(eventName.c_str());
		gAIEnv.pBehaviorTreeUpr->HandleEvent(GetEntityId(), ev);
	}
}

template <class TValue>
void CEntityAIBehaviorTreeComponent::SchematycFunction_SetBBKeyValue(const sxema::CSharedString& key, const TValue& value)
{
	if (m_bBehaviorTreeIsRunning)
	{
		// TODO #1: cache a pointer to the BT blackboard instance
		// TODO #2: but then we'd need to be able to get notified when the BehaviorTreeUpr resets (e. g. due to AISystem reset) and kills all BT instances
		if (BehaviorTree::Blackboard* pBlackboard = gAIEnv.pBehaviorTreeUpr->GetBehaviorTreeBlackboard(GetEntityId()))
		{
			const BehaviorTree::BlackboardVariableId id(key.c_str());
			const bool bSuccess = pBlackboard->SetVariable<TValue>(id, value);
			if (!bSuccess)
			{
				SXEMA_ENV_WARNING("CEntityAIBehaviorTreeComponent::SchematycFunction_SetBBKeyValue: type clash of variable '%s' in the blackboard", key.c_str());
			}
		}
	}
}

