// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/IScriptSignalReceiver.h> // Wrap EScriptSignalReceiverType in a SScriptSignalReceiverParams structure so that we can forward declare?
#include <drx3D/Schema/IScriptStateMachine.h>   // Wrap EScriptStateMachineLifetime in a SScriptStateMachineParams structure so that we can forward declare?
#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/Signal.h>

namespace sxema
{
// Forward declare interfaces.
struct IString;
struct IScriptActionInstance;
struct IScriptBase;
struct IScriptClass;
struct IScriptComponentInstance;
struct IScriptConstructor;
struct IScriptEnum;
struct IScriptFunction;
struct IScriptInterface;
struct IScriptInterfaceFunction;
struct IScriptInterfaceImpl;
struct IScriptInterfaceTask;
struct IScriptModule;
struct IScriptSignal;
struct IScriptState;
struct IScriptStateMachine;
struct IScriptStruct;
struct IScriptTimer;
struct IScriptVariable;
// Forward declare structures.
struct SElementId;

typedef std::function<EVisitStatus(IScript&)> ScriptVisitor;

enum class EScriptRegistryChangeType
{
	Invalid,
	ElementAdded,              // Sent after element has been added.
	ElementModified,           // Sent after element has been modified.
	ElementRemoved,            // Sent before element is removed.
	ElementSaved,              // Sent after element has been saved.
	ElementDependencyModified, // Sent after dependency of element has been modified.
	ElementDependencyRemoved   // Sent after dependency of element has been removed.
};

struct SScriptRegistryChange
{
	inline SScriptRegistryChange(EScriptRegistryChangeType _type, IScriptElement& _element)
		: type(_type)
		, element(_element)
	{}

	EScriptRegistryChangeType type;
	IScriptElement&           element;
};

typedef CSignal<void (const SScriptRegistryChange&)> ScriptRegistryChangeSignal;

struct IScriptRegistry
{
	virtual ~IScriptRegistry() {}

	// Compatibility interface.
	//////////////////////////////////////////////////

	virtual void ProcessEvent(const SScriptEvent& event) = 0;
	virtual bool Load() = 0;
	virtual void Save(bool bAlwaysSave = false) = 0;

	// New interface.
	//////////////////////////////////////////////////

	virtual bool                               IsValidScope(EScriptElementType elementType, IScriptElement* pScope) const = 0;
	virtual bool                               IsValidName(tukk szName, IScriptElement* pScope, tukk & szErrorMessage) const = 0;

	virtual IScriptModule*                     AddModule(tukk szName, tukk szFilePath) = 0;
	virtual IScriptEnum*                       AddEnum(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptStruct*                     AddStruct(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptSignal*                     AddSignal(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptConstructor*                AddConstructor(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptFunction*                   AddFunction(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptInterface*                  AddInterface(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptInterfaceFunction*          AddInterfaceFunction(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptInterfaceTask*              AddInterfaceTask(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptClass*                      AddClass(tukk szName, const SElementId& baseId, tukk szFilePath) = 0;
	virtual IScriptBase*                       AddBase(const SElementId& baseId, IScriptElement* pScope) = 0;
	virtual IScriptStateMachine*               AddStateMachine(tukk szName, EScriptStateMachineLifetime lifetime, const DrxGUID& contextGUID, const DrxGUID& partnerGUID, IScriptElement* pScope) = 0;
	virtual IScriptState*                      AddState(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptVariable*                   AddVariable(tukk szName, const SElementId& typeId, const DrxGUID& baseGUID, IScriptElement* pScope) = 0;
	virtual IScriptTimer*                      AddTimer(tukk szName, IScriptElement* pScope) = 0;
	virtual IScriptSignalReceiver*             AddSignalReceiver(tukk szName, EScriptSignalReceiverType type, const DrxGUID& signalGUID, IScriptElement* pScope) = 0;
	virtual IScriptInterfaceImpl*              AddInterfaceImpl(EDomain domain, const DrxGUID& refGUID, IScriptElement* pScope) = 0;
	virtual IScriptComponentInstance*          AddComponentInstance(tukk szName, const DrxGUID& typeGUID, IScriptElement* pScope) = 0;
	virtual IScriptActionInstance*             AddActionInstance(tukk szName, const DrxGUID& actionGUID, const DrxGUID& contextGUID, IScriptElement* pScope) = 0;

	virtual void                               RemoveElement(const DrxGUID& guid) = 0;

	virtual IScriptElement&                    GetRootElement() = 0;
	virtual const IScriptElement&              GetRootElement() const = 0;
	virtual IScriptElement*                    GetElement(const DrxGUID& guid) = 0;
	virtual const IScriptElement*              GetElement(const DrxGUID& guid) const = 0;

	virtual bool                               CopyElementsToXml(XmlNodeRef& output, IScriptElement& scope) const = 0;
	virtual bool                               PasteElementsFromXml(const XmlNodeRef& input, IScriptElement* pScope) = 0;

	virtual bool                               SaveUndo(XmlNodeRef& output, IScriptElement& scope) const = 0;
	virtual IScriptElement*                    RestoreUndo(const XmlNodeRef& input, IScriptElement* pScope) = 0;

	virtual bool                               IsElementNameUnique(tukk szName, IScriptElement* pScope) const = 0;
	virtual void                               MakeElementNameUnique(IString& name, IScriptElement* pScope) const = 0;    // Make element name unique within specified scope.
	virtual void                               ElementModified(IScriptElement& element) = 0;                              // Notify registry that element has been modified.

	virtual ScriptRegistryChangeSignal::Slots& GetChangeSignalSlots() = 0;

	virtual IScript*                           GetScriptByGuid(const DrxGUID& guid) const = 0;
	virtual IScript*                           GetScriptByFileName(tukk szFilePath) const = 0;

	virtual IScript*                           LoadScript(tukk szFilePath) = 0;
	virtual void                               SaveScript(IScript& script) = 0;

	virtual void                               OnScriptRenamed(IScript& script, tukk szFilePath) = 0;
};
} // sxema
