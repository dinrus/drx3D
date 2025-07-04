// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

#include <drx3D/Schema/IScriptElement.h>
#include <drx3D/Schema/IScriptBase.h>
#include <drx3D/Schema/IScriptClass.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/GUID.h>
#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/IString.h>

namespace sxema
{
namespace ScriptUtils
{

// #SchematycTODO : Do we still need IScriptElement::VisitChildren?
inline void VisitChildren(const IScriptElement& scriptElement, const std::function<void(const IScriptElement&)>& visitor)
{
	SXEMA_CORE_ASSERT(visitor);
	if (visitor)
	{
		for (const IScriptElement* pScriptChildElement = scriptElement.GetFirstChild(); pScriptChildElement; pScriptChildElement = pScriptChildElement->GetNextSibling())
		{
			visitor(*pScriptChildElement);
		}
	}
}

template<typename TYPE> inline void VisitChildren(const IScriptElement& scriptElement, const std::function<void(const TYPE&)>& visitor)
{
	SXEMA_CORE_ASSERT(visitor);
	if (visitor)
	{
		for (const IScriptElement* pScriptChildElement = scriptElement.GetFirstChild(); pScriptChildElement; pScriptChildElement = pScriptChildElement->GetNextSibling())
		{
			const TYPE* pScriptChildElementImpl = DynamicCast<TYPE>(pScriptChildElement);
			if (pScriptChildElementImpl)
			{
				visitor(*pScriptChildElementImpl);
			}
		}
	}
}

inline const IScriptClass* GetClass(const IScriptElement& scriptElement)
{
	for (const IScriptElement* pScriptElement = &scriptElement; pScriptElement; pScriptElement = pScriptElement->GetParent())
	{
		const IScriptClass* pScriptClass = DynamicCast<IScriptClass>(pScriptElement);
		if (pScriptClass)
		{
			return pScriptClass;
		}
	}
	return nullptr;
}

inline const IScriptBase* GetBase(const IScriptClass& scriptClass)
{
	for (const IScriptElement* pScriptElement = scriptClass.GetFirstChild(); pScriptElement; pScriptElement = pScriptElement->GetNextSibling())
	{
		const IScriptBase* pScriptBase = DynamicCast<IScriptBase>(pScriptElement);
		if (pScriptBase)
		{
			return pScriptBase;
		}
	}
	return nullptr;
}

inline void QualifyName(IString& output, const IScriptElement& element, EScriptElementType scopeType = EScriptElementType::Root)
{
	output.clear();
	for (const IScriptElement* pScope = &element; pScope; pScope = pScope->GetParent())
	{
		if (pScope->GetType() == scopeType)
		{
			break;
		}
		else
		{
			if (!output.empty())
			{
				output.insert(0, "::");
			}
			output.insert(0, pScope->GetName());
		}
	}
}

} // ScriptUtils
} // sxema
