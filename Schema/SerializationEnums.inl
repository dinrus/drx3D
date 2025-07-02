// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// This header should be included once (and only once) per module.

#include  <drx3D/Schema/FundamentalTypes.h>
#include  <drx3D/Schema/ITimerSystem.h>
#include  <drx3D/Schema/IScriptElement.h>
#include  <drx3D/Schema/IScriptSignalReceiver.h>
#include  <drx3D/Schema/UniqueId.h>

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, EDomain, "DrxSchematyc Domain")
SERIALIZATION_ENUM(sxema::EDomain::Env, "Env", "Environment")
SERIALIZATION_ENUM(sxema::EDomain::Script, "Script", "Script")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, EOverridePolicy, "DrxSchematyc Override Policy")
SERIALIZATION_ENUM(sxema::EOverridePolicy::Default, "Default", "Default")
SERIALIZATION_ENUM(sxema::EOverridePolicy::Override, "Override", "Override")
SERIALIZATION_ENUM(sxema::EOverridePolicy::Final, "Final", "Final")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, ETimerUnits, "DrxSchematyc Timer Units")
SERIALIZATION_ENUM(sxema::ETimerUnits::Empty, "Empty", "Empty")
SERIALIZATION_ENUM(sxema::ETimerUnits::Frames, "Frames", "Frames")
SERIALIZATION_ENUM(sxema::ETimerUnits::Seconds, "Seconds", "Seconds")
SERIALIZATION_ENUM(sxema::ETimerUnits::Random, "Random", "Random")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, ETimerFlags, "DrxSchematyc Timer Flags")
SERIALIZATION_ENUM(sxema::ETimerFlags::AutoStart, "AutoStart", "Auto Start")
SERIALIZATION_ENUM(sxema::ETimerFlags::Repeat, "Repeat", "Repeat")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED2(sxema, CUniqueId, EType, "DrxSchematyc Unique Port Id Type")
#if SXEMA_PATCH_UNIQUE_IDS
SERIALIZATION_ENUM(sxema::CUniqueId::EType::UInt32, "UniqueId", "UniqueId")
#endif
SERIALIZATION_ENUM(sxema::CUniqueId::EType::Idx, "Idx", "Idx")
SERIALIZATION_ENUM(sxema::CUniqueId::EType::UInt32, "UInt32", "UInt32")
SERIALIZATION_ENUM(sxema::CUniqueId::EType::StringHash, "StringHash", "StringHash")
SERIALIZATION_ENUM(sxema::CUniqueId::EType::GUID, "GUID", "GUID")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, EScriptElementType, "DrxSchematyc Script Element Type")
SERIALIZATION_ENUM(sxema::EScriptElementType::Root, "Root", "Root")
SERIALIZATION_ENUM(sxema::EScriptElementType::Module, "Module", "Module")
SERIALIZATION_ENUM(sxema::EScriptElementType::Enum, "Enum", "Enumeration")
SERIALIZATION_ENUM(sxema::EScriptElementType::Struct, "Struct", "Structure")
SERIALIZATION_ENUM(sxema::EScriptElementType::Signal, "Signal", "Signal")
SERIALIZATION_ENUM(sxema::EScriptElementType::Constructor, "Constructor", "Constructor")
SERIALIZATION_ENUM(sxema::EScriptElementType::Function, "Function", "Function")
SERIALIZATION_ENUM(sxema::EScriptElementType::Interface, "Interface", "Interface")
SERIALIZATION_ENUM(sxema::EScriptElementType::InterfaceFunction, "InterfaceFunction", "Interface Function")
SERIALIZATION_ENUM(sxema::EScriptElementType::InterfaceTask, "InterfaceTask", "Interface Task")
SERIALIZATION_ENUM(sxema::EScriptElementType::Class, "Class", "Class")
SERIALIZATION_ENUM(sxema::EScriptElementType::Base, "Base", "Base")
SERIALIZATION_ENUM(sxema::EScriptElementType::StateMachine, "StateMachine", "State Machine")
SERIALIZATION_ENUM(sxema::EScriptElementType::State, "State", "State")
SERIALIZATION_ENUM(sxema::EScriptElementType::Variable, "Variable", "Variable")
SERIALIZATION_ENUM(sxema::EScriptElementType::Timer, "Timer", "Timer")
SERIALIZATION_ENUM(sxema::EScriptElementType::SignalReceiver, "SignalReceiver", "Signal Receiver")
SERIALIZATION_ENUM(sxema::EScriptElementType::InterfaceImpl, "InterfaceImpl", "Interface Implementation")
SERIALIZATION_ENUM(sxema::EScriptElementType::ComponentInstance, "ComponentInstance", "Component Instance")
SERIALIZATION_ENUM(sxema::EScriptElementType::ActionInstance, "ActionInstance", "Action Instance")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, EScriptElementAccessor, "DrxSchematyc Script Element Accessor")
SERIALIZATION_ENUM(sxema::EScriptElementAccessor::Public, "Public", "Public")
SERIALIZATION_ENUM(sxema::EScriptElementAccessor::Protected, "Protected", "Protected")
SERIALIZATION_ENUM(sxema::EScriptElementAccessor::Private, "Private", "Private")
SERIALIZATION_ENUM_END()

SERIALIZATION_ENUM_BEGIN_NESTED(sxema, EScriptSignalReceiverType, "DrxSchematyc Script Signal Receiver Type")
SERIALIZATION_ENUM(sxema::EScriptSignalReceiverType::EnvSignal, "EnvSignal", "Environment Signal")
SERIALIZATION_ENUM(sxema::EScriptSignalReceiverType::ScriptSignal, "ScriptSignal", "Script Signal")
SERIALIZATION_ENUM(sxema::EScriptSignalReceiverType::ScriptTimer, "ScriptTimer", "Script Timer")
SERIALIZATION_ENUM(sxema::EScriptSignalReceiverType::Universal, "Universal", "Universal")
SERIALIZATION_ENUM_END()
