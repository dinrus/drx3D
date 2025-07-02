// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Type/Type.h>

namespace Drx {
namespace Reflection {

class CDescExtension
{
public:
	CDescExtension(tukk szLabel, DrxTypeId typeId)
		: m_szLabel(szLabel)
		, m_typeId(typeId)
	{}

	CDescExtension(CDescExtension&& desc)
		: m_szLabel(desc.m_szLabel)
		, m_typeId(desc.m_typeId)
	{}

	DrxTypeId   GetTypeId() const { return m_typeId; }
	tukk GetLabel() const  { return m_szLabel; }

private:
	tukk     m_szLabel;
	const DrxTypeId m_typeId;
};

struct IExtensibleDesc
{
	virtual ~IExtensibleDesc() {}

	virtual bool                  AddExtension(const CDescExtension* pExtension) const = 0;
	virtual const CDescExtension* FindExtensionByTypeId(DrxTypeId typeId) const = 0;

	template<typename EXTENSION_TYPE>
	const EXTENSION_TYPE* FindExtension() const
	{
		static_assert(std::is_base_of<CDescExtension, typename Type::PureType<EXTENSION_TYPE>::Type>::value, "Type doesn't derive from 'Drx::Reflection::CDescExtension'.");
		return static_cast<const EXTENSION_TYPE*>(FindExtensionByTypeId(TypeIdOf<EXTENSION_TYPE>()));
	}
};

} // ~Reflection namespace
} // ~Drx namespace
