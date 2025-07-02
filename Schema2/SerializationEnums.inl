// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// This header should be included once (and only once) per module.

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/BasicTypes.h>
#include <drx3D/Schema2/IScriptElement.h>
#include <drx3D/Schema2/IScriptGraph.h>

SERIALIZATION_ENUM_BEGIN_NESTED(sxema2, EDomain, "sxema Domain")
	SERIALIZATION_ENUM(sxema2::EDomain::Env, "env", "Environment")
	SERIALIZATION_ENUM(sxema2::EDomain::Script, "doc", "Script") // #SchematycTODO : Update name and patch files!
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema2, EAccessor, "sxema Accessor")
	SERIALIZATION_ENUM(sxema2::EAccessor::Public, "Public", "Public")
	SERIALIZATION_ENUM(sxema2::EAccessor::Protected, "Protected", "Protected")
	SERIALIZATION_ENUM(sxema2::EAccessor::Private, "Private", "Private")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema2, EOverridePolicy, "sxema Override Policy")
	SERIALIZATION_ENUM(sxema2::EOverridePolicy::Default, "Default", "Default")
	SERIALIZATION_ENUM(sxema2::EOverridePolicy::Override, "Override", "Override")
	SERIALIZATION_ENUM(sxema2::EOverridePolicy::Finalize, "Finalize", "Finalize")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema2, EScriptElementType, "sxema Script Element Type")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::None, "", "None") // Workaround to avoid triggering assert error when serializing empty value.
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Root, "Root", "Root")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Module, "Module", "Module")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Include, "Include", "Include")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Group, "Group", "Group")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Enumeration, "Enumeration", "Enumeration")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Structure, "Structure", "Structure")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Signal, "Signal", "Signal")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Function, "Function", "Function")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::AbstractInterface, "AbstractInterface", "AbstractInterface")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::AbstractInterfaceFunction, "AbstractInterfaceFunction", "AbstractInterfaceFunction")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::AbstractInterfaceTask, "AbstractInterfaceTask", "AbstractInterfaceTask")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Class, "Class", "Class")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::ClassBase, "ClassBase", "ClassBase")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::StateMachine, "StateMachine", "StateMachine")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::State, "State", "State")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Variable, "Variable", "Variable")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Property, "Property", "Property")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Container, "Container", "Container")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Timer, "Timer", "Timer")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::AbstractInterfaceImplementation, "AbstractInterfaceImplementation", "AbstractInterfaceImplementation")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::ComponentInstance, "ComponentInstance", "ComponentInstance")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::ActionInstance, "ActionInstance", "ActionInstance")
	SERIALIZATION_ENUM(sxema2::EScriptElementType::Graph, "Graph", "Graph")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema2, EScriptGraphType, "sxema Script Graph Type")
	SERIALIZATION_ENUM(sxema2::EScriptGraphType::Unknown, "unknown", "Unknown")
	SERIALIZATION_ENUM(sxema2::EScriptGraphType::AbstractInterfaceFunction, "abstract_interface_function", "AbstractInterfaceFunction")
	SERIALIZATION_ENUM(sxema2::EScriptGraphType::Function, "function", "Function")
	SERIALIZATION_ENUM(sxema2::EScriptGraphType::Condition, "condition", "Condition")
	SERIALIZATION_ENUM(sxema2::EScriptGraphType::Constructor, "constructor", "Constructor")
	SERIALIZATION_ENUM(sxema2::EScriptGraphType::Destructor, "destructor", "Destructor")
	SERIALIZATION_ENUM(sxema2::EScriptGraphType::SignalReceiver, "signal_receiver", "SignalReceiver")
	SERIALIZATION_ENUM(sxema2::EScriptGraphType::Transition, "transition", "Transition")
	SERIALIZATION_ENUM(sxema2::EScriptGraphType::Property, "Property", "Property")
SERIALIZATION_ENUM_END()
