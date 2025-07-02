// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/ScriptDependencyEnumerator.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/EnumFlags.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{

// Forward declare interfaces.
struct IGUIDRemapper;
struct IScript;
struct IScriptElement;
struct IScriptExtensionMap;
struct IScriptFile;

enum class EScriptEventId
{
	Invalid,
	FileLoad,                 // Sent immediately after loading elements from file.
	FileReload,               // Sent immediately after reloading elements from file.

	EditorFixUp,              // Editor only. Sent after editor plug-in has been initialized in order to fix-up broken/outdated elements.
	EditorAdd,                // Editor only. Sent immediately after element is added to registry.
	EditorPaste,              // Editor only. Sent immediately after element is pasted to registry.
	EditorEnvModified,        // Editor only. Sent when environment has been modified.
	EditorDependencyModified, // Editor only. Sent when a dependency has been modified.
	EditorDependencyRemoved,  // Editor only. Sent when a dependency has been removed.
	EditorRefresh,            // Editor only. Sent when an element is selected/modified and needs to be refreshed.

	InternalRefresh           // May be sent by an element to itself in order to refresh certain 'internal' aspects.
};

struct SScriptEvent
{
	explicit inline SScriptEvent(EScriptEventId _id, const DrxGUID& _guid = DrxGUID())
		: id(_id)
		, guid(_guid)
	{}

	EScriptEventId id;
	DrxGUID          guid;
};

enum class EScriptElementType
{
	None,
	Root,
	Module,
	Enum,
	Struct,
	Signal,
	Constructor,
	Function,
	Interface,
	InterfaceFunction,
	InterfaceTask,
	Class,
	Base,
	StateMachine,
	State,
	Variable,
	Timer,
	SignalReceiver,
	InterfaceImpl,
	ComponentInstance,
	ActionInstance
};

enum class EScriptElementAccessor // N.B. It is important that the order of this enumeration does not change since we often use comparison operators to filter accessibility.
{
	Public,
	Protected,
	Private
};

enum class EScriptElementFlags
{
	None          = 0,
	System        = BIT(0), // Element was created by system and therefore can't be removed by users.
	FromFile      = BIT(1), // Element was loaded from file.
	CanOwnScript  = BIT(2), // Element can own it's own script.
	MustOwnScript = BIT(3), // Element must own it's own script.
	NotCopyable   = BIT(4), // Element can not be copied.
	FixedName     = BIT(5)  // Element cannot be renamed.
	                        // #SchematycTODO : Use flags to specify whether element can be renamed, copied, removed etc?
	                        // #SchematycTODO : Separate flags which can be modified from those that are fixed?
};

typedef CEnumFlags<EScriptElementFlags>                ScriptElementFlags;

typedef std::function<EVisitStatus(IScriptElement&)>       ScriptElementVisitor;
typedef std::function<EVisitStatus(const IScriptElement&)> ScriptElementConstVisitor;

// Script element interface.
// N.B. Do not inherit from this class directly but instead use IScriptElementBase.
struct IScriptElement
{
	virtual ~IScriptElement() {}

	virtual EScriptElementType         GetType() const = 0;
	virtual DrxGUID                      GetGUID() const = 0;

	virtual void                       SetName(tukk szName) = 0;
	virtual tukk                GetName() const = 0;
	virtual EScriptElementAccessor     GetAccessor() const = 0;
	virtual void                       SetFlags(const ScriptElementFlags& flags) = 0;
	virtual ScriptElementFlags         GetFlags() const = 0;
	virtual void                       SetScript(IScript* pScript) = 0;
	virtual IScript*                   GetScript() = 0;
	virtual const IScript*             GetScript() const = 0;

	virtual void                       AttachChild(IScriptElement& child) = 0;
	virtual void                       DetachChild(IScriptElement& child) = 0;
	virtual void                       SetParent(IScriptElement* pParent) = 0;
	virtual void                       SetPrevSibling(IScriptElement* pPrevSibling) = 0;
	virtual void                       SetNextSibling(IScriptElement* pNextSibling) = 0;

	virtual IScriptElement*            GetParent() = 0;
	virtual const IScriptElement*      GetParent() const = 0;
	virtual IScriptElement*            GetFirstChild() = 0;
	virtual const IScriptElement*      GetFirstChild() const = 0;
	virtual IScriptElement*            GetLastChild() = 0;
	virtual const IScriptElement*      GetLastChild() const = 0;
	virtual IScriptElement*            GetPrevSibling() = 0;
	virtual const IScriptElement*      GetPrevSibling() const = 0;
	virtual IScriptElement*            GetNextSibling() = 0;
	virtual const IScriptElement*      GetNextSibling() const = 0;
	virtual EVisitStatus               VisitChildren(const ScriptElementVisitor& visitor) = 0;
	virtual EVisitStatus               VisitChildren(const ScriptElementConstVisitor& visitor) const = 0;

	virtual IScriptExtensionMap&       GetExtensions() = 0;
	virtual const IScriptExtensionMap& GetExtensions() const = 0;

	virtual void                       EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const = 0;
	virtual void                       RemapDependencies(IGUIDRemapper& guidRemapper) = 0;
	virtual void                       ProcessEvent(const SScriptEvent& event) = 0;
	virtual void                       Serialize(Serialization::IArchive& archive) = 0;
};

DECLARE_SHARED_POINTERS(IScriptElement)

template<EScriptElementType ELEMENT_TYPE> struct IScriptElementBase : public IScriptElement
{
	static const EScriptElementType ElementType = ELEMENT_TYPE;

	virtual EScriptElementType GetType() const override
	{
		return ElementType;
	}
};

template<typename TYPE> inline TYPE& DynamicCast(IScriptElement& scriptElement)
{
	SXEMA_CORE_ASSERT(scriptElement.GetType() == TYPE::ElementType);
	return static_cast<TYPE&>(scriptElement);
}

template<typename TYPE> inline const TYPE& DynamicCast(const IScriptElement& scriptElement)
{
	SXEMA_CORE_ASSERT(scriptElement.GetType() == TYPE::ElementType);
	return static_cast<const TYPE&>(scriptElement);
}

template<typename TYPE> inline TYPE* DynamicCast(IScriptElement* pScriptElement)
{
	return pScriptElement && (pScriptElement->GetType() == TYPE::ElementType) ? static_cast<TYPE*>(pScriptElement) : nullptr;
}

template<typename TYPE> inline const TYPE* DynamicCast(const IScriptElement* pScriptElement)
{
	return pScriptElement && (pScriptElement->GetType() == TYPE::ElementType) ? static_cast<const TYPE*>(pScriptElement) : nullptr;
}

} // sxema
