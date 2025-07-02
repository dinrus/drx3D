// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Type/Type.h>
#include <drx3D/Reflection/IDescExtension.h>

#include <drx3D/CoreX/Extension/DrxGUID.h>

#include <vector>

namespace Drx {
namespace Reflection {

// TODO: Probably make 'const CDescExtension*' a shared pointer.
typedef std::vector<const CDescExtension*> ExtensionsByTypeId;
// ~TODO

template<typename INTERFACE_TYPE>
class CExtensibleDesc : public INTERFACE_TYPE
{
public:
	CExtensibleDesc() {}
	~CExtensibleDesc() {}

	// IExtensibleDesc
	bool                  AddExtension(const CDescExtension* pExtension) const;
	const CDescExtension* FindExtensionByTypeId(DrxTypeId typeId) const;
	// ~IExtensibleDesc

protected:
	// Note: We always allow to add extensions even though it's a const object.
	mutable ExtensionsByTypeId m_extensions;
};

template<typename INTERFACE_TYPE>
inline bool CExtensibleDesc<INTERFACE_TYPE >::AddExtension(const CDescExtension* pExtension) const
{
	const DrxTypeId typeId = pExtension->GetTypeId();
	auto condition = [typeId](const CDescExtension* pExtension) -> bool
	{
		return pExtension->GetTypeId() == typeId;
	};

	const auto result = std::find_if(m_extensions.begin(), m_extensions.end(), condition);
	DRX_ASSERT_MESSAGE(result == m_extensions.end(), "Extension of type '%s' is already registered.", pExtension->GetLabel());

	if (result == m_extensions.end())
	{
		m_extensions.emplace_back(pExtension);
		return true;
	}
	return false;
}

template<typename INTERFACE_TYPE>
inline const CDescExtension* CExtensibleDesc<INTERFACE_TYPE >::FindExtensionByTypeId(DrxTypeId typeId) const
{
	auto condition = [typeId](const CDescExtension* pExtension) -> bool
	{
		return pExtension->GetTypeId() == typeId;
	};

	auto result = std::find_if(m_extensions.begin(), m_extensions.end(), condition);
	if (result != m_extensions.end())
	{
		return *result;
	}

	return nullptr;
}

} // ~Reflection namespace
} // ~Drx namespace
