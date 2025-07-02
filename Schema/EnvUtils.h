// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvElement.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/Delegate.h>

namespace sxema
{
namespace EnvUtils
{

// #SchematycTODO : Do we still need IEnvElement::VisitChildren?
inline void VisitChildren(const IEnvElement& envElement, const std::function<void(const IEnvElement&)>& visitor)
{
	SXEMA_CORE_ASSERT(visitor);
	if (visitor)
	{
		for (const IEnvElement* pEnvChildElement = envElement.GetFirstChild(); pEnvChildElement; pEnvChildElement = pEnvChildElement->GetNextSibling())
		{
			visitor(*pEnvChildElement);
		}
	}
}

template<typename TYPE> inline void VisitChildren(const IEnvElement& envElement, const std::function<void(const TYPE&)>& visitor)
{
	SXEMA_CORE_ASSERT(visitor);
	if (visitor)
	{
		for (const IEnvElement* pEnvChildElement = envElement.GetFirstChild(); pEnvChildElement; pEnvChildElement = pEnvChildElement->GetNextSibling())
		{
			const TYPE* pEnvChildElementImpl = DynamicCast<TYPE>(pEnvChildElement);
			if (pEnvChildElementImpl)
			{
				visitor(*pEnvChildElementImpl);
			}
		}
	}
}

} // EnvUtils
} // sxema
