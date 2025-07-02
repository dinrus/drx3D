// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/Schema/IInterfaceMap.h>

namespace sxema
{
template<typename BASE_INTERFACE> class CInterfaceMap : public IInterfaceMap<BASE_INTERFACE>
{
private:

	typedef VectorMap<CTypeName, BASE_INTERFACE*> Interfaces;

private:

	// IInterfaceMap

	virtual bool Add(const CTypeName& typeName, BASE_INTERFACE* pInterface) override
	{
		auto pair = std::pair<CTypeName, BASE_INTERFACE*>(typeName, pInterface);
		return m_interfaces.insert(pair).second;
	}

	virtual BASE_INTERFACE* Query(const CTypeName& typeName) const override
	{
		auto itInterface = m_interfaces.find(typeName);
		return itInterface != m_interfaces.end() ? itInterface->second : nullptr;
	}

	// ~IInterfaceMap

private:

	Interfaces m_interfaces;
};
} // sxema
