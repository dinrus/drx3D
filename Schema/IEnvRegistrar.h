// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Replace IEnvElementPtr with std::unique_ptr?

#pragma once

#include <drx3D/Schema/IEnvElement.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{

// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(IEnvElement)

struct IEnvElementRegistrar
{
	virtual ~IEnvElementRegistrar() {}

	virtual void Register(const IEnvElementPtr& pElement, const DrxGUID& scopeGUID) = 0;
};

class CEnvRegistrationScope
{
public:

	inline CEnvRegistrationScope(IEnvElementRegistrar& elementRegistrar, const DrxGUID& scopeGUID)
		: m_elementRegistrar(elementRegistrar)
		, m_scopeGUID(scopeGUID)
	{}

	inline CEnvRegistrationScope Register(const IEnvElementPtr& pElement)
	{
		SXEMA_CORE_ASSERT(pElement);
		if (pElement)
		{
			m_elementRegistrar.Register(pElement, m_scopeGUID);
			return CEnvRegistrationScope(m_elementRegistrar, pElement->GetGUID());
		}
		return *this;
	}

private:

	IEnvElementRegistrar& m_elementRegistrar;
	DrxGUID                 m_scopeGUID;
};

struct IEnvRegistrar
{
	virtual ~IEnvRegistrar() {}

	virtual CEnvRegistrationScope RootScope() = 0;
	virtual CEnvRegistrationScope Scope(const DrxGUID& scopeGUID) = 0;
};

} // sxema
