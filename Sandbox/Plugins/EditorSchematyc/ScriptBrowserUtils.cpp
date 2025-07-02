// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ScriptBrowserUtils.h"

#include <DrxSchematyc/FundamentalTypes.h>
#include <DrxSchematyc/Env/Elements/IEnvAction.h>
#include <DrxSchematyc/Env/Elements/IEnvClass.h>
#include <DrxSchematyc/Env/Elements/IEnvComponent.h>
#include <DrxSchematyc/Env/Elements/IEnvDataType.h>
#include <DrxSchematyc/Env/Elements/IEnvSignal.h>
#include <DrxSchematyc/Script/IScript.h>
#include <DrxSchematyc/Script/IScriptRegistry.h>
#include <DrxSchematyc/Script/IScriptView.h>
#include <DrxSchematyc/Script/Elements/IScriptActionInstance.h>
#include <DrxSchematyc/Script/Elements/IScriptClass.h>
#include <DrxSchematyc/Script/Elements/IScriptEnum.h>
#include <DrxSchematyc/Script/Elements/IScriptInterface.h>
#include <DrxSchematyc/Script/Elements/IScriptInterfaceFunction.h>
#include <DrxSchematyc/Script/Elements/IScriptInterfaceImpl.h>
#include <DrxSchematyc/Script/Elements/IScriptInterfaceTask.h>
#include <DrxSchematyc/Script/Elements/IScriptModule.h>
#include <DrxSchematyc/Script/Elements/IScriptSignal.h>
#include <DrxSchematyc/Script/Elements/IScriptSignalReceiver.h>
#include <DrxSchematyc/Script/Elements/IScriptState.h>
#include <DrxSchematyc/Script/Elements/IScriptStateMachine.h>
#include <DrxSchematyc/Script/Elements/IScriptStruct.h>
#include <DrxSchematyc/Script/Elements/IScriptTimer.h>
#include <DrxSchematyc/Utils/StackString.h>

#include <Controls/QuestionDialog.h>
#include "PluginUtils.h"
#include "ReportWidget.h"

#include "TypesDictionary.h"
#include <Controls/DictionaryWidget.h>
#include <Controls/QPopupWidget.h>

#include <QCursor>
#include <QtUtil.h>

#include "ComponentsDictionaryModel.h"
#include "InterfacesDictionaryModel.h"

namespace Schematyc
{
namespace ScriptBrowserUtils
{
// #SchematycTODO : Move icon declarations to separate file?

static tukk g_szScriptModuleIcon = "icons:schematyc/element_folder.ico";
static tukk g_szScriptEnumIcon = "icons:schematyc/element_enum.ico";
static tukk g_szScriptStructIcon = "icons:schematyc/element_struct.ico";
static tukk g_szScriptSignalIcon = "icons:schematyc/element_signal.ico";
static tukk g_szScriptConstructorIcon = "icons:schematyc/element_constructor.ico";
static tukk g_szScriptFunctionIcon = "icons:schematyc/element_function.ico";
static tukk g_szScriptInterfaceIcon = "";
static tukk g_szScriptInterfaceFunctionIcon = "";
static tukk g_szScriptInterfaceTaskIcon = "";
static tukk g_szScriptElementIcon = "icons:schematyc/element_class.ico";
static tukk g_szScriptStateMachineIcon = "icons:schematyc/element_statemachine.ico";
static tukk g_szScriptStateIcon = "icons:schematyc/element_state.ico";
static tukk g_szScriptVariableIcon = "icons:schematyc/element_variable.ico";
static tukk g_szScriptTimerIcon = "icons:schematyc/element_timer.ico";
static tukk g_szScriptSignalReceiverIcon = "icons:schematyc/element_signalreceiver.ico";
static tukk g_szScriptComponentIcon = "icons:schematyc/element_componentinstance.ico";
static tukk g_szScriptActionIcon = "icons:schematyc/element_actioninstance.ico";

struct SBase
{
	inline SBase() {}

	inline SBase(tukk _szName, tukk _szDescription, const SElementId& _id)
		: name(_szName)
		, description(_szDescription)
		, id(_id)
	{}

	string     name;
	string     description;
	SElementId id;
};

typedef std::vector<SBase> Bases;

struct SSignalReceiver
{
	inline SSignalReceiver(EScriptSignalReceiverType _type, const DrxGUID& _guid, tukk _szLabel, tukk szDescription)
		: type(_type)
		, guid(_guid)
		, label(_szLabel)
		, description(szDescription)
	{}

	EScriptSignalReceiverType type;
	DrxGUID                   guid;
	string                    label;
	string                    description;
};

struct SInterface
{
	inline SInterface() {}

	inline SInterface(EDomain _domain, const DrxGUID& _guid, tukk _szName, tukk _szFullName, tukk szDescription)
		: domain(_domain)
		, guid(_guid)
		, name(_szName)
		, fullName(_szFullName)
		, description(szDescription)
	{}

	EDomain domain;
	DrxGUID guid;
	string  name;
	string  fullName;
	string  description;
};

struct SComponent
{
	inline SComponent() {}

	inline SComponent(const DrxGUID& _typeGUID, tukk _szName, tukk _szFullName, tukk szDescription)
		: typeGUID(_typeGUID)
		, name(_szName)
		, fullName(_szFullName)
		, description(szDescription)
	{}

	DrxGUID typeGUID;
	string  name;
	string  fullName;
	string  description;
};

struct SAction
{
	inline SAction() {}

	inline SAction(const DrxGUID& _guid, const DrxGUID& _componentInstanceGUID, tukk _szName, tukk _szFullName, tukk _szDescription)
		: guid(_guid)
		, componentInstanceGUID(_componentInstanceGUID)
		, name(_szName)
		, fullName(_szFullName)
		, description(_szDescription)
	{}

	DrxGUID guid;
	DrxGUID componentInstanceGUID;
	string  name;
	string  fullName;
	string  description;
};

inline void MoveRenameScriptsRecursive(IScriptElement& element)
{
	// TODO: Can we remove this function?
	/*IScript* pScript = element.GetScript();
	   if (pScript)
	   {
	   const CStackString prevFileName = pScript->GetName();
	   const CStackString fileName = pScript->SetNameFromRoot();

	   CStackString folder = fileName.substr(0, fileName.rfind('/'));
	   if (!folder.empty())
	   {
	    gEnv->pDrxPak->MakeDir(folder.c_str());
	   }

	   MoveFile(prevFileName.c_str(), fileName.c_str());

	   // #SchematycTODO : Use RemoveDirectory() to remove empty directories?
	   // #SchematycTODO : Update source control!
	   }
	   for (IScriptElement* pChild = element.GetFirstChild(); pChild; pChild = pChild->GetNextSibling())
	   {
	   MoveRenameScriptsRecursive(*pChild);
	   }*/
	// ~TODO
}

// #SchematycTODO : Move logic to Schematyc core (add IsValidScope() function to IStriptElement).
bool CanAddScriptElement(EScriptElementType elementType, IScriptElement* pScope)
{
	return gEnv->pSchematyc->GetScriptRegistry().IsValidScope(elementType, pScope);
}

bool CanRemoveScriptElement(const IScriptElement& element)
{
	const EScriptElementType elementType = element.GetType();
	switch (elementType)
	{
	case EScriptElementType::Root:
	case EScriptElementType::Base:
	case EScriptElementType::Constructor:
		{
			return false;
		}
		/*case EScriptElementType::Graph:
		   {
		    switch(static_cast<const IDocGraph&>(element).GetType())
		    {
		    case EScriptGraphType::InterfaceFunction:
		    case EScriptGraphType::Transition_DEPRECATED:
		      {
		        return false;
		      }
		    }
		    break;
		   }*/
	}
	return true;
}

bool CanRenameScriptElement(const IScriptElement& element)
{
	// #SchematycTODO : Looks like it's not crashing on an item we removed!!!

	return !element.GetFlags().Check(EScriptElementFlags::FixedName);
}

bool CanCopyScriptElement(const IScriptElement& element)
{
	return !element.GetFlags().Check(EScriptElementFlags::NotCopyable);
}

tukk GetScriptElementTypeName(EScriptElementType scriptElementType)
{
	switch (scriptElementType)
	{
	case EScriptElementType::Enum:
		{
			return "Enumeration";
		}
	case EScriptElementType::Struct:
		{
			return "Structure";
		}
	case EScriptElementType::Signal:
		{
			return "Signal";
		}
	case EScriptElementType::Function:
		{
			return "Function";
		}
	case EScriptElementType::SignalReceiver:
		{
			return "Signal Receiver";
		}
	case EScriptElementType::Interface:
		{
			return "Interface";
		}
	case EScriptElementType::Class:
		{
			return "Class";
		}
	case EScriptElementType::StateMachine:
		{
			return "State Machine";
		}
	case EScriptElementType::Variable:
		{
			return "Variable";
		}
	case EScriptElementType::Timer:
		{
			return "Timer";
		}
	case EScriptElementType::InterfaceImpl:
		{
			return "Interface";
		}
	case EScriptElementType::ComponentInstance:
		{
			return "Component";
		}
	case EScriptElementType::ActionInstance:
		{
			return "Action";
		}
	}
	return nullptr;
}

SFilterAttributes GetScriptElementFilterAttributes(EScriptElementType scriptElementType)
{
	SFilterAttributes attribute;
	switch (scriptElementType)
	{
	case EScriptElementType::Base:
		{
			attribute.szOrder = "A";
			attribute.filterType = EFilterType::Base;
			break;
		}
	case EScriptElementType::Enum:
	case EScriptElementType::Struct:
		{
			attribute.szName = "Types";
			attribute.szOrder = "C";
			attribute.filterType = EFilterType::Types;
			break;
		}
	case EScriptElementType::Signal:
		{
			attribute.szName = "Signals";
			attribute.szOrder = "E";
			attribute.filterType = EFilterType::Signals;
			break;
		}
	case EScriptElementType::Function:
	case EScriptElementType::Constructor:
	case EScriptElementType::SignalReceiver:
		{
			attribute.szName = "Graphs";
			attribute.szOrder = "F";
			attribute.filterType = EFilterType::Graphs;
			break;
		}
	case EScriptElementType::InterfaceImpl:
	case EScriptElementType::Interface:
		{
			attribute.szName = "Interfaces";
			attribute.szOrder = "H";
			attribute.filterType = EFilterType::Interfaces;
			break;
		}
	case EScriptElementType::StateMachine:
		{
			attribute.szName = "State Machines";
			attribute.szOrder = "G";
			attribute.filterType = EFilterType::StateMachine;
			break;
		}
	case EScriptElementType::Variable:
	case EScriptElementType::Timer:
		{
			attribute.szName = "Variables";
			attribute.szOrder = "D";
			attribute.filterType = EFilterType::Variables;
			break;
		}
	case EScriptElementType::ComponentInstance:
		{
			attribute.szName = "Components";
			attribute.szOrder = "B";
			attribute.filterType = EFilterType::Components;
			break;
		}
	case EScriptElementType::InterfaceFunction:
	case EScriptElementType::State:
	case EScriptElementType::Class:
		{
			break;
		}
	default:
		{
			DRX_ASSERT_MESSAGE(false, "Invalid or deprecated element type.");
			break;
		}
	}
	return attribute;
}

void AppendFilterTags(EScriptElementType scriptElementType, stack_string& filter)
{
	switch (scriptElementType)
	{
	case EScriptElementType::Base:
		{
			filter.append("Base ");
			break;
		}
	case EScriptElementType::Enum:
		{
			filter.append("Type Enum");
			break;
		}
	case EScriptElementType::Struct:
		{
			filter.append("Type Struct");
			break;
		}
	case EScriptElementType::Signal:
		{
			filter.append("Signal ");
			break;
		}
	case EScriptElementType::Function:
		{
			filter.append("Graph Function");
			break;
		}
	case EScriptElementType::Constructor:
		{
			filter.append("Graph Constructor");
			break;
		}
	case EScriptElementType::SignalReceiver:
		{
			filter.append("Graph Signal Receiver");
			break;
		}
	case EScriptElementType::InterfaceImpl:
	case EScriptElementType::InterfaceFunction:
	case EScriptElementType::Interface:
		{
			filter.append("Interface ");
			break;
		}
	case EScriptElementType::StateMachine:
		{
			filter.append("State Machine ");
			break;
		}
	case EScriptElementType::State:
		{
			filter.append("State  ");
			break;
		}
	case EScriptElementType::Variable:
		{
			filter.append("Variable  ");
			break;
		}
	case EScriptElementType::Timer:
		{
			filter.append("Variable Timer  ");
			break;
		}
	case EScriptElementType::ComponentInstance:
		{
			filter.append("Component  ");
			break;
		}
	default:
		{
			DRX_ASSERT_MESSAGE(false, "Invalid or deprecated element type.");
			break;
		}
	}
}

tukk GetScriptElementIcon(const IScriptElement& scriptElement)
{
	switch (scriptElement.GetType())
	{
	case EScriptElementType::Module:
		{
			return g_szScriptModuleIcon;
		}
	case EScriptElementType::Enum:
		{
			return g_szScriptEnumIcon;
		}
	case EScriptElementType::Struct:
		{
			return g_szScriptStructIcon;
		}
	case EScriptElementType::Signal:
		{
			return g_szScriptSignalIcon;
		}
	case EScriptElementType::Constructor:
		{
			return g_szScriptConstructorIcon;
		}
	case EScriptElementType::Function:
		{
			return g_szScriptFunctionIcon;
		}
	case EScriptElementType::Interface:
		{
			return g_szScriptInterfaceIcon;
		}
	case EScriptElementType::InterfaceFunction:
		{
			return g_szScriptInterfaceFunctionIcon;
		}
	case EScriptElementType::InterfaceTask:
		{
			return g_szScriptInterfaceTaskIcon;
		}
	case EScriptElementType::Class:
		{
			return g_szScriptElementIcon;
		}
	case EScriptElementType::Base:
		{
			return g_szScriptElementIcon;
		}
	case EScriptElementType::StateMachine:
		{
			return g_szScriptStateMachineIcon;
		}
	case EScriptElementType::State:
		{
			return g_szScriptStateIcon;
		}
	case EScriptElementType::Variable:
		{
			return g_szScriptVariableIcon;
		}
	case EScriptElementType::Timer:
		{
			return g_szScriptTimerIcon;
		}
	case EScriptElementType::SignalReceiver:
		{
			return g_szScriptSignalReceiverIcon;
		}
	case EScriptElementType::InterfaceImpl:
		{
			return g_szScriptInterfaceIcon;
		}
	case EScriptElementType::ComponentInstance:
		{
			const DrxGUID guid = DynamicCast<IScriptComponentInstance>(scriptElement).GetTypeGUID();
			const IEnvComponent* pEnvComponent = gEnv->pSchematyc->GetEnvRegistry().GetComponent(guid);
			if (pEnvComponent)
			{
				tukk szIcon = pEnvComponent->GetDesc().GetIcon();
				if (szIcon && szIcon[0])
				{
					return szIcon;
				}
			}
			return g_szScriptComponentIcon;
		}
	case EScriptElementType::ActionInstance:
		{
			const DrxGUID guid = DynamicCast<IScriptActionInstance>(scriptElement).GetActionTypeGUID();
			const IEnvAction* pEnvAction = gEnv->pSchematyc->GetEnvRegistry().GetAction(guid);
			if (pEnvAction)
			{
				tukk szIcon = pEnvAction->GetDesc().GetIcon();
				if (szIcon && szIcon[0])
				{
					return szIcon;
				}
			}
			return g_szScriptActionIcon;
		}
	default:
		{
			return "";
		}
	}
}

void MakeScriptElementNameUnique(CStackString& name, IScriptElement* pScope)
{
	IScriptRegistry& scriptRegistry = gEnv->pSchematyc->GetScriptRegistry();
	if (!scriptRegistry.IsElementNameUnique(name.c_str(), pScope))
	{
		CStackString::size_type counterPos = name.find(".");
		if (counterPos == CStackString::npos)
		{
			counterPos = name.length();
		}

		char stringBuffer[16] = "";
		CStackString::size_type counterLength = 0;
		u32 counter = 1;
		do
		{
			ltoa(counter, stringBuffer, 10);
			name.replace(counterPos, counterLength, stringBuffer);
			counterLength = strlen(stringBuffer);
			++counter;
		}
		while (!scriptRegistry.IsElementNameUnique(name.c_str(), pScope));
	}
}

void FindReferences(const IScriptElement& element)
{
	const DrxGUID& guid = element.GetGUID();
	const IScriptElement* pCurrentScriptElement = nullptr;
	std::vector<const IScriptElement*> references;
	references.reserve(100);

	auto enumerateDependencies = [&guid, &pCurrentScriptElement, &references](const DrxGUID& referenceGUID)
	{
		if (referenceGUID == guid)
		{
			references.push_back(pCurrentScriptElement);
		}
	};

	auto visitScriptElement = [&pCurrentScriptElement, &enumerateDependencies](const IScriptElement& scriptElement) -> EVisitStatus
	{
		pCurrentScriptElement = &scriptElement;
		scriptElement.EnumerateDependencies(enumerateDependencies, EScriptDependencyType::Reference);
		return EVisitStatus::Recurse;
	};

	gEnv->pSchematyc->GetScriptRegistry().GetRootElement().VisitChildren(visitScriptElement);

	IScriptViewPtr pScriptView = gEnv->pSchematyc->CreateScriptView(element.GetGUID());

	CReportWidget* pReportWidget = new CReportWidget(nullptr);
	pReportWidget->setAttribute(Qt::WA_DeleteOnClose);
	pReportWidget->setWindowTitle("References");

	for (const IScriptElement* pReference : references)
	{
		CStackString uri;
		DrxLinkUtils::CreateUri(uri, DrxLinkUtils::ECommand::Show, pReference->GetGUID());

		CStackString name;
		pScriptView->QualifyName(*pReference, EDomainQualifier::Global, name);

		pReportWidget->WriteUri(uri.c_str(), name.c_str());
	}

	pReportWidget->show();
}

IScriptEnum* AddScriptEnum(IScriptElement* pScope)
{
	CStackString name = "NewEnumeration";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddEnum(name.c_str(), pScope);
}

IScriptStruct* AddScriptStruct(IScriptElement* pScope)
{
	CStackString name = "NewStructure";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddStruct(name.c_str(), pScope);
}

IScriptSignal* AddScriptSignal(IScriptElement* pScope)
{
	CStackString name = "NewSignal";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddSignal(name.c_str(), pScope);
}

IScriptFunction* AddScriptFunction(IScriptElement* pScope)
{
	CStackString name = "NewFunction";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddFunction(name.c_str(), pScope);
}

IScriptInterface* AddScriptInterface(IScriptElement* pScope)
{
	CStackString name = "NewInterface";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddInterface(name.c_str(), pScope);
}

IScriptInterfaceFunction* AddScriptInterfaceFunction(IScriptElement* pScope)
{
	CStackString name = "NewFunction";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddInterfaceFunction(name.c_str(), pScope);
}

IScriptInterfaceTask* AddScriptInterfaceTask(IScriptElement* pScope)
{
	CStackString name = "NewTask";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddInterfaceTask(name.c_str(), pScope);
}

IScriptStateMachine* AddScriptStateMachine(IScriptElement* pScope)
{
	CStackString name = "NewStateMachine";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddStateMachine(name.c_str(), EScriptStateMachineLifetime::Persistent, DrxGUID(), DrxGUID(), pScope);
}

IScriptState* AddScriptState(IScriptElement* pScope)
{
	CStackString name = "NewState";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddState(name.c_str(), pScope);
}

IScriptVariable* AddScriptVariable(IScriptElement* pScope)
{
	CStackString name = "NewVariable";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddVariable(name.c_str(), SElementId(), DrxGUID(), pScope);
}

IScriptTimer* AddScriptTimer(IScriptElement* pScope)
{
	CStackString name = "NewTimer";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddTimer(name.c_str(), pScope);
}

IScriptSignalReceiver* AddScriptSignalReceiver(IScriptElement* pScope)
{
	CStackString name = "NewSignalReceiver";
	MakeScriptElementNameUnique(name, pScope);
	return gEnv->pSchematyc->GetScriptRegistry().AddSignalReceiver(name.c_str(), EScriptSignalReceiverType::Universal, DrxGUID(), pScope);
}

IScriptInterfaceImpl* AddScriptInterfaceImpl(IScriptElement* pScope, const QPoint* pPosition)
{
	DrxSchematycEditor::CInterfacesDictionary dict(&gEnv->pSchematyc->GetScriptRegistry().GetRootElement());
	CModalPopupDictionary dictionary("Schematyc::AddInterfaceImplementation", dict);

	QPoint pos;
	if (pPosition)
	{
		pos = *pPosition;
	}
	else
	{
		pos = QCursor::pos();
	}

	dictionary.ExecAt(pos, QPopupWidget::TopLeft);

	DrxSchematycEditor::CInterfaceDictionaryEntry* pEntry = static_cast<DrxSchematycEditor::CInterfaceDictionaryEntry*>(dictionary.GetResult());
	if (pEntry)
	{
		return gEnv->pSchematyc->GetScriptRegistry().AddInterfaceImpl(pEntry->GetDomain(), pEntry->GetInterfaceGUID(), pScope);
	}

	return nullptr;
}

IScriptComponentInstance* AddScriptComponentInstance(IScriptElement* pScope, const QPoint* pPosition)
{
	DrxSchematycEditor::CComponentsDictionary dict(pScope);
	CModalPopupDictionary dictionary("Schematyc::AddComponent", dict);

	QPoint pos;
	if (pPosition)
	{
		pos = *pPosition;
	}
	else
	{
		pos = QCursor::pos();
	}

	dictionary.ExecAt(pos, QPopupWidget::TopLeft);

	DrxSchematycEditor::CComponentDictionaryEntry* pEntry = static_cast<DrxSchematycEditor::CComponentDictionaryEntry*>(dictionary.GetResult());
	if (pEntry)
	{
		CStackString name = QtUtil::ToString(pEntry->GetName()).c_str();
		MakeScriptElementNameUnique(name, pScope);
		return gEnv->pSchematyc->GetScriptRegistry().AddComponentInstance(name.c_str(), pEntry->GetTypeGUID(), pScope);
	}

	return nullptr;
}

bool RenameScriptElement(IScriptElement& scriptElement, tukk szName)
{
	SCHEMATYC_EDITOR_ASSERT(szName);
	if (szName)
	{
		if (CanRenameScriptElement(scriptElement))
		{
			tukk szPrevName = scriptElement.GetName();
			if (strcmp(szName, szPrevName) != 0)
			{
				tukk szErrorMessage = nullptr;
				if (!gEnv->pSchematyc->GetScriptRegistry().IsValidName(szName, scriptElement.GetParent(), szErrorMessage))
				{
					stack_string message;
					message.Format("Invalid name '%s'. %s", szName, szErrorMessage);

					CQuestionDialog dialog;
					dialog.SetupQuestion("Invalid Name", message.c_str(), QDialogButtonBox::Ok);

					dialog.Execute();

					return false;
				}

				bool bRenameScriptElement = true;
				bool bMoveRenameScriptFiles = false;

				// TODO: This shouldn't be needed anymore.
				const IScript* pScript = scriptElement.GetScript();
				if (pScript)
				{
					tukk szScriptName = pScript->GetFilePath();
					if (szScriptName[0] != '\0')
					{
						CStackString fileName;
						DrxSchematycEditor::Utils::ConstructAbsolutePath(fileName, szScriptName);
						if (gEnv->pDrxPak->IsFileExist(fileName.c_str()))
						{
							CStackString question;
							question.Format("Renaming '%s' to '%s'.\nWould you like to move/rename script files to match?", szPrevName, szName);

							CQuestionDialog dialog;
							dialog.SetupQuestion("Move/Rename Script Files", question.c_str(), QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);

							QDialogButtonBox::StandardButton result = dialog.Execute();
							bRenameScriptElement = (result != QDialogButtonBox::Cancel);
							bMoveRenameScriptFiles = (result == QDialogButtonBox::Yes);
						}
					}
				}
				// ~TODO

				if (bRenameScriptElement)
				{
					scriptElement.SetName(szName);
					gEnv->pSchematyc->GetScriptRegistry().ElementModified(scriptElement); // #SchematycTODO : Call from script element?
					if (bMoveRenameScriptFiles)
					{
						MoveRenameScriptsRecursive(scriptElement);
					}
				}

				return true;
			}
		}
	}
	return false;
}

bool RemoveScriptElement(const IScriptElement& element)
{
	if (CanRemoveScriptElement(element))
	{
		CStackString question = "Are you sure you want to remove '";
		question.append(element.GetName());
		question.append("'?");

		CQuestionDialog dialog;
		dialog.SetupQuestion("Remove", question.c_str(), QDialogButtonBox::Yes | QDialogButtonBox::No);

		QDialogButtonBox::StandardButton result = dialog.Execute();

		if (result == QDialogButtonBox::Yes)
		{
			gEnv->pSchematyc->GetScriptRegistry().RemoveElement(element.GetGUID());
			return true;
		}
	}

	return false;
}
} // ScriptBrowserUtils
} // Schematyc

