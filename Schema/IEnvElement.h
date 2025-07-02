// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/IEnvElement.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/EnumFlags.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{

// Forward declare interfaces.
struct IEnvElement;

enum class EEnvElementType
{
	Root,
	Module,
	DataType,
	Signal,
	Function,
	Class,
	Interface,
	InterfaceFunction,
	Component,
	Action
};

enum class EEnvElementFlags
{
	None       = 0,
	Deprecated = BIT(1)
};

typedef CEnumFlags<EEnvElementFlags> EnvElementFlags;

typedef std::function<EVisitStatus(const IEnvElement&)> EnvElementConstVisitor;

// Environment element interface.
// N.B. Do not inherit from this class directly but instead use IEnvElementBase e.g. struct IEnvFunction : public IEnvElementBase<EEnvElementType::Function>.
struct IEnvElement
{
	virtual ~IEnvElement() {}

	virtual EEnvElementType    GetType() const = 0;
	virtual EnvElementFlags    GetFlags() const = 0;

	virtual DrxGUID            GetGUID() const = 0;
	virtual tukk        GetName() const = 0;
	virtual SSourceFileInfo    GetSourceFileInfo() const = 0;
	virtual tukk        GetDescription() const = 0;

	virtual bool               IsValidScope(IEnvElement& scope) const = 0;

	virtual bool               AttachChild(IEnvElement& child) = 0;
	virtual void               DetachChild(IEnvElement& child) = 0;
	virtual void               SetParent(IEnvElement* pParent) = 0;
	virtual void               SetPrevSibling(IEnvElement* pPrevSibling) = 0;
	virtual void               SetNextSibling(IEnvElement* pNextSibling) = 0;

	virtual IEnvElement*       GetParent() = 0;
	virtual const IEnvElement* GetParent() const = 0;
	virtual IEnvElement*       GetFirstChild() = 0;
	virtual const IEnvElement* GetFirstChild() const = 0;
	virtual IEnvElement*       GetLastChild() = 0;
	virtual const IEnvElement* GetLastChild() const = 0;
	virtual IEnvElement*       GetPrevSibling() = 0;
	virtual const IEnvElement* GetPrevSibling() const = 0;
	virtual IEnvElement*       GetNextSibling() = 0;
	virtual const IEnvElement* GetNextSibling() const = 0;

	virtual EVisitStatus       VisitChildren(const EnvElementConstVisitor& visitor) const = 0;
};

template<EEnvElementType ELEMENT_TYPE> struct IEnvElementBase : public IEnvElement
{
	static const EEnvElementType ElementType = ELEMENT_TYPE;

	virtual EEnvElementType GetType() const override
	{
		return ElementType;
	}
};

template<typename TYPE> inline TYPE& DynamicCast(IEnvElement& envElement)
{
	SXEMA_CORE_ASSERT(envElement.GetType() == TYPE::ElementType);
	return static_cast<TYPE&>(envElement);
}

template<typename TYPE> inline const TYPE& DynamicCast(const IEnvElement& envElement)
{
	SXEMA_CORE_ASSERT(envElement.GetType() == TYPE::ElementType);
	return static_cast<const TYPE&>(envElement);
}

template<typename TYPE> inline TYPE* DynamicCast(IEnvElement* pEnvElement)
{
	return pEnvElement && (pEnvElement->GetType() == TYPE::ElementType) ? static_cast<TYPE*>(pEnvElement) : nullptr;
}

template<typename TYPE> inline const TYPE* DynamicCast(const IEnvElement* pEnvElement)
{
	return pEnvElement && (pEnvElement->GetType() == TYPE::ElementType) ? static_cast<const TYPE*>(pEnvElement) : nullptr;
}

} // sxema
