// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptRegistry.h>

#include <drx3D/CoreX/DrxCrc32.h>
#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Schema/IEnvRegistry.h>
#include <drx3D/Schema/IEnvComponent.h>
#include <drx3D/Schema/ILog.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/StackString.h>
#include <drx3D/Schema/StringUtils.h>
#include <drx3D/Schema/ICompiler.h>

#include <drx3D/Schema/CVars.h>
#include <drx3D/Schema/Script.h>
#include <drx3D/Schema/ScriptSerializers.h>
#include <drx3D/Schema/ScriptActionInstance.h>
#include <drx3D/Schema/ScriptBase.h>
#include <drx3D/Schema/ScriptClass.h>
#include <drx3D/Schema/ScriptComponentInstance.h>
#include <drx3D/Schema/ScriptConstructor.h>
#include <drx3D/Schema/ScriptEnum.h>
#include <drx3D/Schema/ScriptFunction.h>
#include <drx3D/Schema/ScriptInterface.h>
#include <drx3D/Schema/ScriptInterfaceFunction.h>
#include <drx3D/Schema/ScriptInterfaceImpl.h>
#include <drx3D/Schema/ScriptInterfaceTask.h>
#include <drx3D/Schema/ScriptModule.h>
#include <drx3D/Schema/ScriptRoot.h>
#include <drx3D/Schema/ScriptSignal.h>
#include <drx3D/Schema/ScriptSignalReceiver.h>
#include <drx3D/Schema/ScriptState.h>
#include <drx3D/Schema/ScriptStateMachine.h>
#include <drx3D/Schema/ScriptStruct.h>
#include <drx3D/Schema/ScriptTimer.h>
#include <drx3D/Schema/ScriptVariable.h>
#include <drx3D/Schema/SerializationContext.h>
#include <drx3D/Schema/FileUtils.h>
#include <drx3D/Schema/GUIDRemapper.h>
#include <drx3D/Schema/Core.h>

namespace sxema
{
DECLARE_SHARED_POINTERS(CScriptActionInstance)
DECLARE_SHARED_POINTERS(CScriptBase)
DECLARE_SHARED_POINTERS(CScriptClass)
DECLARE_SHARED_POINTERS(CScriptComponentInstance)
DECLARE_SHARED_POINTERS(CScriptConstructor)
DECLARE_SHARED_POINTERS(CScriptEnum)
DECLARE_SHARED_POINTERS(CScriptFunction)
DECLARE_SHARED_POINTERS(CScriptInterface)
DECLARE_SHARED_POINTERS(CScriptInterfaceFunction)
DECLARE_SHARED_POINTERS(CScriptInterfaceImpl)
DECLARE_SHARED_POINTERS(CScriptInterfaceTask)
DECLARE_SHARED_POINTERS(CScriptModule)
DECLARE_SHARED_POINTERS(CScriptSignal)
DECLARE_SHARED_POINTERS(CScriptSignalReceiver)
DECLARE_SHARED_POINTERS(CScriptState)
DECLARE_SHARED_POINTERS(CScriptStateMachine)
DECLARE_SHARED_POINTERS(CScriptStruct)
DECLARE_SHARED_POINTERS(CScriptTimer)
DECLARE_SHARED_POINTERS(CScriptVariable)

namespace
{
void SaveAllScriptFilesCommand(IConsoleCmdArgs* pArgs)
{
	gEnv->pSchematyc->GetScriptRegistry().Save(true);
}

void ProcessEventRecursive(IScriptElement& element, const SScriptEvent& event)
{
	element.ProcessEvent(event);

	for (IScriptElement* pChildElement = element.GetFirstChild(); pChildElement; pChildElement = pChildElement->GetNextSibling())
	{
		ProcessEventRecursive(*pChildElement, event);
	}
}
} // Anonymous

CScriptRegistry::CScriptRegistry()
	: m_changeDepth(0)
{
	m_pRoot = std::make_shared<CScriptRoot>();
	REGISTER_COMMAND("sc_SaveAllScriptFiles", SaveAllScriptFilesCommand, VF_NULL, "Save all sxema script file regardless of whether they have been modified");
}

void CScriptRegistry::ProcessEvent(const SScriptEvent& event)
{
	ProcessEventRecursive(*m_pRoot, event);
}

bool CScriptRegistry::Load()
{
	LOADING_TIME_PROFILE_SECTION;

	// Configure file enumeration flags.
	FileUtils::FileEnumFlags fileEnumFlags = FileUtils::EFileEnumFlags::Recursive;
	if (CVars::sc_IgnoreUnderscoredFolders)
	{
		fileEnumFlags.Add(FileUtils::EFileEnumFlags::IgnoreUnderscoredFolders);
	}

	// Enumerate files and construct new elements.
	ScriptInputBlocks inputBlocks;
	tukk szScriptFolder = gEnv->pSchematyc->GetScriptsFolder();

	auto loadScript = [this, &inputBlocks](tukk szFileName, unsigned attributes)
	{
		LOADING_TIME_PROFILE_SECTION_ARGS(szFileName);
		// TODO: Move this to a separated function.
		SScriptInputBlock inputBlock;
		CScriptLoadSerializer serializer(inputBlock);
		Serialization::LoadXmlFile(serializer, szFileName);

		if (!GUID::IsEmpty(inputBlock.guid) && inputBlock.rootElement.instance)
		{
			CScript* pScript = GetScript(inputBlock.guid);
			if (!pScript)
			{
				DRX_ASSERT_MESSAGE(szFileName && szFileName[0], "Undefined file name!");
				if (szFileName && szFileName[0])
				{
					CScriptPtr pSharedScript = std::make_shared<CScript>(inputBlock.guid, szFileName);
					pScript = pSharedScript.get();
					m_scriptsByGuid.emplace(inputBlock.guid, pSharedScript);
					m_scriptsByFileName.emplace(CCrc32::ComputeLowercase(szFileName), pSharedScript);

					pScript->SetRoot(inputBlock.rootElement.instance.get());
					inputBlock.rootElement.instance->SetScript(pScript);
					inputBlocks.push_back(std::move(inputBlock));
				}
			}
		}
		// ~TODO
	};
	FileUtils::EnumFilesInFolder(szScriptFolder, "*.schematyc_*", loadScript, fileEnumFlags);

	ProcessInputBlocks(inputBlocks, *m_pRoot, EScriptEventId::FileLoad);
	return true;
}

void CScriptRegistry::Save(bool bAlwaysSave)
{
	// Save script files.
	for (ScriptsByGuid::value_type& script : m_scriptsByGuid)
	{
		SaveScript(*script.second);
	}
}

bool CScriptRegistry::IsValidScope(EScriptElementType elementType, IScriptElement* pScope) const
{
	const EScriptElementType scopeElementType = pScope ? pScope->GetType() : EScriptElementType::Root;
	switch (elementType)
	{
	case EScriptElementType::Module:
		{
			return scopeElementType == EScriptElementType::Root;
		}
	case EScriptElementType::Enum:
		{
			return scopeElementType == EScriptElementType::Root || scopeElementType == EScriptElementType::Module || scopeElementType == EScriptElementType::Class;
		}
	case EScriptElementType::Struct:
		{
			return scopeElementType == EScriptElementType::Root || scopeElementType == EScriptElementType::Module || scopeElementType == EScriptElementType::Class;
		}
	case EScriptElementType::Signal:
		{
			return scopeElementType == EScriptElementType::Root || scopeElementType == EScriptElementType::Module || scopeElementType == EScriptElementType::Class || scopeElementType == EScriptElementType::State;
		}
	case EScriptElementType::Constructor:
		{
			return scopeElementType == EScriptElementType::Class;
		}
	case EScriptElementType::Function:
		{
			return scopeElementType == EScriptElementType::Root || scopeElementType == EScriptElementType::Module || scopeElementType == EScriptElementType::Class;
		}
	case EScriptElementType::Interface:
		{
			return scopeElementType == EScriptElementType::Root || scopeElementType == EScriptElementType::Module;
		}
	case EScriptElementType::InterfaceFunction:
		{
			return scopeElementType == EScriptElementType::Interface;
		}
	case EScriptElementType::InterfaceTask:
		{
			return scopeElementType == EScriptElementType::Interface;
		}
	case EScriptElementType::Class:
		{
			return scopeElementType == EScriptElementType::Root;
		}
	case EScriptElementType::Base:
		{
			return scopeElementType == EScriptElementType::Class;
		}
	case EScriptElementType::StateMachine:
		{
			if (scopeElementType == EScriptElementType::Class)
			{
				bool hasStateMachine = false;
				auto visitScriptElement = [&hasStateMachine](IScriptElement& scriptElement) -> EVisitStatus
				{
					if (scriptElement.GetType() == EScriptElementType::StateMachine)
					{
						hasStateMachine = true;
						return EVisitStatus::Stop;
					}

					if (scriptElement.GetType() == EScriptElementType::Base)
					{
						return EVisitStatus::Continue;
					}

					return EVisitStatus::Recurse;
				};
				pScope->VisitChildren(visitScriptElement);
				return !hasStateMachine;
			}

			return false;
		}
	case EScriptElementType::State:
		{
			return scopeElementType == EScriptElementType::StateMachine || scopeElementType == EScriptElementType::State;
		}
	case EScriptElementType::Variable:
		{
			// TODO: Variables in modules aren't supported yet.
			return scopeElementType == EScriptElementType::Class /*|| scopeElementType == EScriptElementType::Module*/;
			// ~TODO
		}
	case EScriptElementType::Timer:
		{
			return scopeElementType == EScriptElementType::Class || scopeElementType == EScriptElementType::State;
		}
	case EScriptElementType::SignalReceiver:
		{
			return scopeElementType == EScriptElementType::Class || scopeElementType == EScriptElementType::State;
		}
	case EScriptElementType::InterfaceImpl:
		{
			return scopeElementType == EScriptElementType::Class;
		}
	case EScriptElementType::ComponentInstance:
		{
			switch (scopeElementType)
			{
			case EScriptElementType::Class:
			case EScriptElementType::Base:
				{
					return true;
				}
			case EScriptElementType::ComponentInstance:
				{
					const IScriptComponentInstance& componentInstance = DynamicCast<IScriptComponentInstance>(*pScope);
					const IEnvComponent* pEnvComponent = gEnv->pSchematyc->GetEnvRegistry().GetComponent(componentInstance.GetTypeGUID());
					if (pEnvComponent)
					{
						if (pEnvComponent->GetDesc().GetComponentFlags().Check(IEntityComponent::EFlags::Socket))
						{
							return true;
						}
					}
					return false;
				}
			default:
				{
					return false;
				}
			}
		}
	case EScriptElementType::ActionInstance:
		{
			return scopeElementType == EScriptElementType::Class || scopeElementType == EScriptElementType::State;
		}
	default:
		{
			return false;
		}
	}
}

bool CScriptRegistry::IsValidName(tukk szName, IScriptElement* pScope, tukk & szErrorMessage) const
{
	if (StringUtils::IsValidElementName(szName, szErrorMessage))
	{
		if (IsUniqueElementName(szName, pScope))
		{
			return true;
		}
		else
		{
			szErrorMessage = "Name must be unique.";
		}
	}
	return false;
}

IScriptModule* CScriptRegistry::AddModule(tukk szName, tukk szFilePath)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Module, m_pRoot.get());
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (IsElementNameUnique(szName, m_pRoot.get()))
			{
				CScriptModulePtr pModule = std::make_shared<CScriptModule>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pModule, *m_pRoot.get(), szFilePath);
				return pModule.get();
			}
		}
	}
	return nullptr;
}

IScriptEnum* CScriptRegistry::AddEnum(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Enum, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptEnumPtr pEnum = std::make_shared<CScriptEnum>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pEnum, *pScope);
				return pEnum.get();
			}
		}
	}
	return nullptr;
}

IScriptStruct* CScriptRegistry::AddStruct(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Struct, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptStructPtr pStruct = std::make_shared<CScriptStruct>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pStruct, *pScope);
				return pStruct.get();
			}
		}
	}
	return nullptr;
}

IScriptSignal* CScriptRegistry::AddSignal(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Signal, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptSignalPtr pSignal = std::make_shared<CScriptSignal>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pSignal, *pScope);
				return pSignal.get();
			}
		}
	}
	return nullptr;
}

IScriptConstructor* CScriptRegistry::AddConstructor(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Constructor, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptConstructorPtr pConstructor = std::make_shared<CScriptConstructor>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pConstructor, *pScope);
				return pConstructor.get();
			}
		}
	}
	return nullptr;
}

IScriptFunction* CScriptRegistry::AddFunction(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Function, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptFunctionPtr pFunction = std::make_shared<CScriptFunction>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pFunction, *pScope);
				return pFunction.get();
			}
		}
	}
	return nullptr;
}

IScriptInterface* CScriptRegistry::AddInterface(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Interface, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptInterfacePtr pInterface = std::make_shared<CScriptInterface>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pInterface, *pScope);
				return pInterface.get();
			}
		}
	}
	return nullptr;
}

IScriptInterfaceFunction* CScriptRegistry::AddInterfaceFunction(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::InterfaceFunction, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptInterfaceFunctionPtr pInterfaceFunction = std::make_shared<CScriptInterfaceFunction>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pInterfaceFunction, *pScope);
				return pInterfaceFunction.get();
			}
		}
	}
	return nullptr;
}

IScriptInterfaceTask* CScriptRegistry::AddInterfaceTask(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::InterfaceTask, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptInterfaceTaskPtr pInterfaceTask = std::make_shared<CScriptInterfaceTask>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pInterfaceTask, *pScope);
				return pInterfaceTask.get();
			}
		}
	}
	return nullptr;
}

IScriptClass* CScriptRegistry::AddClass(tukk szName, const SElementId& baseId, tukk szFilePath)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	IScriptElement* pScope = &GetRootElement();

	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Class, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptClassPtr pClass = std::make_shared<CScriptClass>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pClass, *pScope, szFilePath);
				AddBase(baseId, pClass.get());
				return pClass.get();
			}
		}
	}
	return nullptr;
}

IScriptBase* CScriptRegistry::AddBase(const SElementId& baseId, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor());
	if (gEnv->IsEditor())
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Base, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			CScriptBasePtr pBase = std::make_shared<CScriptBase>(gEnv->pSchematyc->CreateGUID(), baseId);
			AddElement(pBase, *pScope);
			return pBase.get();
		}
	}
	return nullptr;
}

IScriptStateMachine* CScriptRegistry::AddStateMachine(tukk szName, EScriptStateMachineLifetime lifetime, const DrxGUID& contextGUID, const DrxGUID& partnerGUID, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::StateMachine, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptStateMachinePtr pStateMachine = std::make_shared<CScriptStateMachine>(gEnv->pSchematyc->CreateGUID(), szName, lifetime, contextGUID, partnerGUID);
				AddElement(pStateMachine, *pScope);
				return pStateMachine.get();
			}
		}
	}
	return nullptr;
}

IScriptState* CScriptRegistry::AddState(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::State, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptStatePtr pState = std::make_shared<CScriptState>(gEnv->pSchematyc->CreateGUID(), szName);
				AddElement(pState, *pScope);
				return pState.get();
			}
		}
	}
	return nullptr;
}

IScriptVariable* CScriptRegistry::AddVariable(tukk szName, const SElementId& typeId, const DrxGUID& baseGUID, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Variable, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptVariablePtr pVariable = std::make_shared<CScriptVariable>(gEnv->pSchematyc->CreateGUID(), szName, typeId, baseGUID);
				AddElement(pVariable, *pScope);
				return pVariable.get();
			}
		}
	}
	return nullptr;
}

IScriptTimer* CScriptRegistry::AddTimer(tukk szName, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor());
	if (gEnv->IsEditor())
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::Timer, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			CScriptTimerPtr pTimer = std::make_shared<CScriptTimer>(gEnv->pSchematyc->CreateGUID(), szName);
			AddElement(pTimer, *pScope);
			return pTimer.get();
		}
	}
	return nullptr;
}

IScriptSignalReceiver* CScriptRegistry::AddSignalReceiver(tukk szName, EScriptSignalReceiverType type, const DrxGUID& signalGUID, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor());
	if (gEnv->IsEditor())
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::SignalReceiver, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			CScriptSignalReceiverPtr pSignalReceiver = std::make_shared<CScriptSignalReceiver>(gEnv->pSchematyc->CreateGUID(), szName, type, signalGUID);
			AddElement(pSignalReceiver, *pScope);
			return pSignalReceiver.get();
		}
	}
	return nullptr;
}

IScriptInterfaceImpl* CScriptRegistry::AddInterfaceImpl(EDomain domain, const DrxGUID& refGUID, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor());
	if (gEnv->IsEditor())
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::InterfaceImpl, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			CScriptInterfaceImplPtr pInterfaceImpl = std::make_shared<CScriptInterfaceImpl>(gEnv->pSchematyc->CreateGUID(), domain, refGUID);
			AddElement(pInterfaceImpl, *pScope);
			return pInterfaceImpl.get();
		}
	}
	return nullptr;
}

IScriptComponentInstance* CScriptRegistry::AddComponentInstance(tukk szName, const DrxGUID& typeGUID, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::ComponentInstance, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptComponentInstancePtr pComponentInstance = std::make_shared<CScriptComponentInstance>(gEnv->pSchematyc->CreateGUID(), szName, typeGUID);
				AddElement(pComponentInstance, *pScope);
				return pComponentInstance.get();
			}
		}
	}
	return nullptr;
}

IScriptActionInstance* CScriptRegistry::AddActionInstance(tukk szName, const DrxGUID& actionGUID, const DrxGUID& contextGUID, IScriptElement* pScope)
{
	SXEMA_CORE_ASSERT(gEnv->IsEditor() && szName);
	if (gEnv->IsEditor() && szName)
	{
		const bool bIsValidScope = IsValidScope(EScriptElementType::ActionInstance, pScope);
		SXEMA_CORE_ASSERT(bIsValidScope);
		if (bIsValidScope)
		{
			if (!pScope)
			{
				pScope = m_pRoot.get();
			}
			if (IsElementNameUnique(szName, pScope))
			{
				CScriptActionInstancePtr pActionInstance = std::make_shared<CScriptActionInstance>(gEnv->pSchematyc->CreateGUID(), szName, actionGUID, contextGUID);
				AddElement(pActionInstance, *pScope);
				return pActionInstance.get();
			}
		}
	}
	return nullptr;
}

void CScriptRegistry::RemoveElement(const DrxGUID& guid)
{
	Elements::iterator itElement = m_elements.find(guid);
	if (itElement != m_elements.end())
	{
		RemoveElement(*itElement->second);
	}
}

IScriptElement& CScriptRegistry::GetRootElement()
{
	return *m_pRoot;
}

const IScriptElement& CScriptRegistry::GetRootElement() const
{
	return *m_pRoot;
}

IScriptElement* CScriptRegistry::GetElement(const DrxGUID& guid)
{
	Elements::iterator itElement = m_elements.find(guid);
	return itElement != m_elements.end() ? itElement->second.get() : nullptr;
}

const IScriptElement* CScriptRegistry::GetElement(const DrxGUID& guid) const
{
	Elements::const_iterator itElement = m_elements.find(guid);
	return itElement != m_elements.end() ? itElement->second.get() : nullptr;
}

bool CScriptRegistry::CopyElementsToXml(XmlNodeRef& output, IScriptElement& scope) const
{
	// #SchematycTODO : Make sure elements don't have NotCopyable flag!!!
	output = Serialization::SaveXmlNode(CScriptCopySerializer(scope), "DrxSchematycScript");
	return !!output;
}

bool CScriptRegistry::PasteElementsFromXml(const XmlNodeRef& input, IScriptElement* pScope)
{
	ScriptInputBlocks inputBlocks(1);
	CScriptPasteSerializer serializer(inputBlocks.back());
	if (Serialization::LoadXmlNode(serializer, input))
	{
		ProcessInputBlocks(inputBlocks, pScope ? *pScope : *m_pRoot, EScriptEventId::EditorPaste);
	}
	return true;
}

bool CScriptRegistry::SaveUndo(XmlNodeRef& output, IScriptElement& scope) const
{
	output = Serialization::SaveXmlNode(CScriptSaveSerializer(static_cast<CScript&>(*scope.GetScript())), "DrxSchematycScript");
	return !!output;
}

IScriptElement* CScriptRegistry::RestoreUndo(const XmlNodeRef& input, IScriptElement* pScope)
{
	CScriptPtr pSharedScript = m_scriptsByGuid.find(pScope->GetScript()->GetGUID())->second;
	RemoveElement(*pScope);

	ScriptInputBlocks inputBlocks(1);
	SScriptInputBlock& inputBlock = inputBlocks.back();
	CScriptLoadSerializer serializer(inputBlock);
	if (Serialization::LoadXmlNode(serializer, input))
	{
		if (!GUID::IsEmpty(inputBlock.guid) && inputBlock.rootElement.instance)
		{
			m_scriptsByGuid.emplace(inputBlock.guid, pSharedScript);
			m_scriptsByFileName.emplace(CCrc32::ComputeLowercase(pSharedScript->GetFilePath()), pSharedScript);

			pSharedScript.get()->SetRoot(inputBlock.rootElement.instance.get());
			inputBlock.rootElement.instance->SetScript(pSharedScript.get());

			ProcessInputBlocks(inputBlocks, *m_pRoot, EScriptEventId::FileReload);
			CCore::GetInstance().GetCompiler().CompileDependencies(pSharedScript->GetRoot()->GetGUID());

			return pSharedScript->GetRoot();
		}
	}

	return nullptr;
}

bool CScriptRegistry::IsElementNameUnique(tukk szName, IScriptElement* pScope) const
{
	SXEMA_CORE_ASSERT(szName);
	if (szName)
	{
		if (!pScope)
		{
			pScope = m_pRoot.get();
		}
		for (const IScriptElement* pElement = pScope->GetFirstChild(); pElement; pElement = pElement->GetNextSibling())
		{
			if (strcmp(pElement->GetName(), szName) == 0)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void CScriptRegistry::MakeElementNameUnique(IString& name, IScriptElement* pScope) const
{
	CStackString uniqueName = name.c_str();
	if (!IsElementNameUnique(uniqueName.c_str(), pScope))
	{
		const CStackString::size_type length = uniqueName.length();
		char stringBuffer[32] = "";
		u32 postfix = 1;
		do
		{
			uniqueName.resize(length);
			ltoa(postfix, stringBuffer, 10);
			uniqueName.append(stringBuffer);
			++postfix;
		}
		while (!IsElementNameUnique(uniqueName.c_str(), pScope));
		name.assign(uniqueName.c_str());
	}
}

void CScriptRegistry::ElementModified(IScriptElement& element)
{
	gEnv->pSchematyc->GetCompiler().CompileDependencies(element.GetGUID());

	ProcessChange(SScriptRegistryChange(EScriptRegistryChangeType::ElementModified, element));
	ProcessChangeDependencies(EScriptRegistryChangeType::ElementModified, element.GetGUID());
}

ScriptRegistryChangeSignal::Slots& CScriptRegistry::GetChangeSignalSlots()
{
	return m_signals.change.GetSlots();
}

IScript* CScriptRegistry::GetScriptByGuid(const DrxGUID& guid) const
{
	ScriptsByGuid::const_iterator itScript = m_scriptsByGuid.find(guid);
	return itScript != m_scriptsByGuid.end() ? static_cast<IScript*>(itScript->second.get()) : nullptr;
}

IScript* CScriptRegistry::GetScriptByFileName(tukk szFilePath) const
{
	u32k fileNameHash = CCrc32::ComputeLowercase(szFilePath);
	ScriptsByFileName::const_iterator itScript = m_scriptsByFileName.find(fileNameHash);
	return itScript != m_scriptsByFileName.end() ? static_cast<IScript*>(itScript->second.get()) : nullptr;
}

IScript* CScriptRegistry::LoadScript(tukk szFilePath)
{
	LOADING_TIME_PROFILE_SECTION;

	CScript* pScript = nullptr;

	DRX_ASSERT_MESSAGE(szFilePath && szFilePath[0], "Undefined file name!");
	if (szFilePath && szFilePath[0])
	{
		u32k fileNameCrc = CCrc32::ComputeLowercase(szFilePath);
		auto result = m_scriptsByFileName.find(fileNameCrc);
		if (result != m_scriptsByFileName.end())
		{
			pScript = result->second.get();
			if (pScript->GetRoot())
			{
				RemoveElement(*pScript->GetRoot());
			}
		}

		{
			ScriptInputBlocks inputBlocks(1);
			SScriptInputBlock& inputBlock = inputBlocks.back();
			CScriptLoadSerializer serializer(inputBlock);
			if (Serialization::LoadXmlFile(serializer, szFilePath))
			{
				if (!GUID::IsEmpty(inputBlock.guid) && inputBlock.rootElement.instance)
				{
					CScriptPtr pSharedScript = std::make_shared<CScript>(inputBlock.guid, szFilePath);
					pScript = pSharedScript.get();
					m_scriptsByGuid.emplace(inputBlock.guid, pSharedScript);
					m_scriptsByFileName.emplace(CCrc32::ComputeLowercase(szFilePath), pSharedScript);

					pScript->SetRoot(inputBlock.rootElement.instance.get());
					inputBlock.rootElement.instance->SetScript(pScript);

					ProcessInputBlocks(inputBlocks, *m_pRoot, EScriptEventId::FileReload);
					return pSharedScript.get();
				}
			}
		}
	}
	return nullptr;
}

void CScriptRegistry::SaveScript(IScript& script)
{
	SaveScript(static_cast<CScript&>(script));
}

void CScriptRegistry::OnScriptRenamed(IScript& script, tukk szFilePath)
{
	u32 filePathHash = CCrc32::ComputeLowercase(script.GetFilePath());
	ScriptsByFileName::const_iterator itScript = m_scriptsByFileName.find(filePathHash);
	if (itScript != m_scriptsByFileName.end())
	{
		CScriptPtr pScript = itScript->second;
		pScript->SetFilePath(szFilePath);
		m_scriptsByFileName.erase(itScript);

		filePathHash = CCrc32::ComputeLowercase(szFilePath);
		m_scriptsByFileName.emplace(filePathHash, pScript);

		gEnv->pEntitySystem->GetClassRegistry()->UnregisterStdClass(pScript->GetRoot()->GetGUID());

		CCore::GetInstance().GetCompiler().CompileDependencies(pScript->GetRoot()->GetGUID());
	}
}

CScript* CScriptRegistry::CreateScript(tukk szFilePath, const IScriptElementPtr& pRoot)
{
	if (szFilePath)
	{
		const DrxGUID guid = gEnv->pSchematyc->CreateGUID();
		SXEMA_CORE_ASSERT(!GUID::IsEmpty(guid));
		if (!GUID::IsEmpty(guid))
		{
			CScriptPtr pScript = std::make_shared<CScript>(guid, nullptr);
			pScript->SetRoot(pRoot.get());
			pRoot->SetScript(pScript.get());

			tukk szFileName = pScript->SetFilePath(szFilePath);
			u32k fileNameHash = CCrc32::ComputeLowercase(szFileName);
			m_scriptsByFileName.emplace(fileNameHash, pScript);
			m_scriptsByGuid.emplace(guid, pScript);

			return pScript.get();
		}
	}
	return nullptr;
}

CScript* CScriptRegistry::GetScript(const DrxGUID& guid)
{
	ScriptsByGuid::iterator itScript = m_scriptsByGuid.find(guid);
	return itScript != m_scriptsByGuid.end() ? itScript->second.get() : nullptr;
}

void CScriptRegistry::ProcessInputBlocks(ScriptInputBlocks& inputBlocks, IScriptElement& scope, EScriptEventId eventId)
{
	// Unroll elements.
	ScriptInputElementPtrs elements;
	elements.reserve(100);
	for (SScriptInputBlock& inputBlock : inputBlocks)
	{
		UnrollScriptInputElementsRecursive(elements, inputBlock.rootElement);
	}

	// Pre-load elements.
	for (SScriptInputElement* pElement : elements)
	{
		CScriptInputElementSerializer elementSerializer(*pElement->instance, ESerializationPass::LoadDependencies);
		Serialization::LoadBlackBox(elementSerializer, pElement->blackBox);
	}

	if (eventId == EScriptEventId::EditorPaste)
	{
		// Make sure root elements have unique names.
		for (SScriptInputBlock& inputBlock : inputBlocks)
		{
			CStackString elementName = inputBlock.rootElement.instance->GetName();
			MakeElementNameUnique(elementName, &scope);
			inputBlock.rootElement.instance->SetName(elementName.c_str());
		}

		// Re-map element dependencies.
		CGUIDRemapper guidRemapper;
		for (SScriptInputElement* pElement : elements)
		{
			guidRemapper.Bind(pElement->instance->GetGUID(), gEnv->pSchematyc->CreateGUID());
		}
		for (SScriptInputElement* pElement : elements)
		{
			pElement->instance->RemapDependencies(guidRemapper);
		}
	}

	// Sort elements in order of dependency.
	if (SortScriptInputElementsByDependency(elements))
	{
		// Load elements.
		for (SScriptInputElement* pElement : elements)
		{
			CScriptInputElementSerializer elementSerializer(*pElement->instance, ESerializationPass::Load);
			Serialization::LoadBlackBox(elementSerializer, pElement->blackBox);
			m_elements.insert(Elements::value_type(pElement->instance->GetGUID(), pElement->instance));
		}

		// Attach elements.
		for (SScriptInputBlock& inputBlock : inputBlocks)
		{
			IScriptElement* pScope = !GUID::IsEmpty(inputBlock.scopeGUID) ? GetElement(inputBlock.scopeGUID) : &scope;
			IScriptElement& element = *inputBlock.rootElement.instance;
			if (pScope)
			{
				pScope->AttachChild(element);
			}
			else
			{
				SXEMA_CORE_CRITICAL_ERROR("Invalid scope for element %s!", element.GetName());
			}
		}

		// Post-load elements.
		for (SScriptInputElement* pElement : elements)
		{
			CScriptInputElementSerializer elementSerializer(*pElement->instance, ESerializationPass::PostLoad);
			Serialization::LoadBlackBox(elementSerializer, pElement->blackBox);

			if (eventId == EScriptEventId::EditorPaste)
			{
				pElement->instance->ProcessEvent(SScriptEvent(EScriptEventId::EditorPaste));
			}
			ProcessChange(SScriptRegistryChange(EScriptRegistryChangeType::ElementAdded, *pElement->instance));
		}

		// Broadcast event to all elements.
		for (SScriptInputElement* pElement : elements)
		{
			pElement->instance->ProcessEvent(SScriptEvent(eventId));
		}
	}
}

void CScriptRegistry::AddElement(const IScriptElementPtr& pElement, IScriptElement& scope, tukk szFilePath)
{
	BeginChange();

	bool bCreateScript = false;
	if (CVars::sc_EnableScriptPartitioning)
	{
		bCreateScript = pElement->GetFlags().Check(EScriptElementFlags::CanOwnScript);
	}
	else
	{
		if (pElement->GetFlags().Check(EScriptElementFlags::MustOwnScript))
		{
			bCreateScript = true;
		}
		else if (pElement->GetFlags().Check(EScriptElementFlags::CanOwnScript))
		{
			bCreateScript = true;
			for (const IScriptElement* pScope = &scope; pScope; pScope = pScope->GetParent())
			{
				if (pScope->GetScript())
				{
					bCreateScript = false;
				}
			}
		}
	}

	if (bCreateScript)
	{
		DRX_ASSERT_MESSAGE(szFilePath, "Script file path must be not null!");
		// #SchematycTODO : We should do this when patching up script elements!!!
		if (szFilePath)
		{
			CScript* pScript = CreateScript(szFilePath, pElement);
		}
	}

	scope.AttachChild(*pElement);

	m_elements.insert(Elements::value_type(pElement->GetGUID(), pElement));

	ProcessChange(SScriptRegistryChange(EScriptRegistryChangeType::ElementAdded, *pElement));

	pElement->ProcessEvent(SScriptEvent(EScriptEventId::EditorAdd));

	EndChange();

	CCore::GetInstance().GetCompiler().CompileDependencies(scope.GetGUID());
}

void CScriptRegistry::RemoveElement(IScriptElement& element)
{
	while (IScriptElement* pChildElement = element.GetFirstChild())
	{
		RemoveElement(*pChildElement);
	}

	ProcessChange(SScriptRegistryChange(EScriptRegistryChangeType::ElementRemoved, element));

	IScriptElement* pParent = element.GetParent();
	CScript* pScript = static_cast<CScript*>(element.GetScript());
	if (pScript)
	{
		pParent->DetachChild(element);
		gEnv->pEntitySystem->GetClassRegistry()->UnregisterStdClass(pScript->GetRoot()->GetGUID());
	}

	const DrxGUID parentGuid = pParent ? pParent->GetGUID() : m_pRoot->GetGUID();
	const DrxGUID guid = element.GetGUID();
	m_elements.erase(guid);

	if (pScript)
	{
		m_scriptsByGuid.erase(pScript->GetGUID());
		u32k fileNameHash = CCrc32::ComputeLowercase(pScript->GetFilePath());
		m_scriptsByFileName.erase(fileNameHash);
	}

	CCore::GetInstance().GetCompiler().CompileDependencies(parentGuid);
}

void CScriptRegistry::SaveScript(CScript& script)
{
	const CStackString fileName = script.GetFilePath();
	const bool hasFileName = !fileName.empty();

	DRX_ASSERT_MESSAGE(hasFileName, "Trying to save sxema script without a defined file name.");
	if (hasFileName)
	{
		CStackString folder = fileName.substr(0, fileName.rfind('/'));

		auto elementSerializeCallback = [this](IScriptElement& element)
		{
			CScriptRegistry& scriptRegistry = static_cast<CScriptRegistry&>(gEnv->pSchematyc->GetScriptRegistry());
			scriptRegistry.ProcessChange(SScriptRegistryChange(EScriptRegistryChangeType::ElementSaved, element));
		};

		const bool bError = !Serialization::SaveXmlFile(fileName.c_str(), CScriptSaveSerializer(script, elementSerializeCallback), "DrxSchematyc");
		if (bError)
		{
			SXEMA_CORE_ERROR("Failed to save file '%s'!", fileName.c_str());
		}
	}
}

void CScriptRegistry::BeginChange()
{
	++m_changeDepth;
}

void CScriptRegistry::EndChange()
{
	--m_changeDepth;
	if (m_changeDepth == 0)
	{
		for (const SScriptRegistryChange& change : m_changeQueue)
		{
			m_signals.change.Send(change);
		}
		m_changeQueue.clear();
	}
}

void CScriptRegistry::ProcessChange(const SScriptRegistryChange& change)
{
	if (m_changeDepth)
	{
		m_changeQueue.push_back(change);
	}
	else
	{
		m_signals.change.Send(change);
	}
}

void CScriptRegistry::ProcessChangeDependencies(EScriptRegistryChangeType changeType, const DrxGUID& elementGUID)
{
	EScriptEventId dependencyEventId = EScriptEventId::Invalid;
	EScriptRegistryChangeType dependencyChangeType = EScriptRegistryChangeType::Invalid;
	switch (changeType)
	{
	case EScriptRegistryChangeType::ElementModified:
		{
			dependencyEventId = EScriptEventId::EditorDependencyModified;
			dependencyChangeType = EScriptRegistryChangeType::ElementDependencyModified;
			break;
		}
	case EScriptRegistryChangeType::ElementRemoved:
		{
			dependencyEventId = EScriptEventId::EditorDependencyRemoved;
			dependencyChangeType = EScriptRegistryChangeType::ElementDependencyRemoved;
			break;
		}
	default:
		{
			return;
		}
	}

	const SScriptEvent event(dependencyEventId, elementGUID);
	for (Elements::value_type& dependencyElement : m_elements)
	{
		bool bIsDependency = false;
		auto enumerateDependency = [&elementGUID, &bIsDependency](const DrxGUID& guid)
		{
			if (guid == elementGUID)
			{
				bIsDependency = true;
			}
		};
		dependencyElement.second->EnumerateDependencies(enumerateDependency, EScriptDependencyType::Event);   // #SchematycTODO : Can we cache dependencies after every change?

		if (bIsDependency)
		{
			dependencyElement.second->ProcessEvent(event);

			m_signals.change.Send(SScriptRegistryChange(dependencyChangeType, *dependencyElement.second));   // #SchematycTODO : Queue these changes and process immediately after processing events?
		}
	}
}

bool CScriptRegistry::IsUniqueElementName(tukk szName, IScriptElement* pScope) const
{
	if (!pScope)
	{
		pScope = m_pRoot.get();
	}
	for (IScriptElement* pElement = pScope->GetFirstChild(); pElement; pElement = pElement->GetNextSibling())
	{
		if (strcmp(szName, pElement->GetName()) == 0)
		{
			return false;
		}
	}
	return true;
}
} // sxema
