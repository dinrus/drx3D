// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Reflection/IReflection.h>
#include <drx3D/CoreX/Extension/DrxGUID.h>

#include <unordered_map>
#include <vector>

namespace Drx {
namespace Reflection {

template<typename REGISTRY_INTERFACE_TYPE = ISystemTypeRegistry>
class CSystemTypeRegistry : public REGISTRY_INTERFACE_TYPE
{
public:
	CSystemTypeRegistry(tukk szLabel, const DrxTypeDesc& typeDesc, const DrxGUID& guid);

	virtual ~CSystemTypeRegistry() {}

	// ICustomTypeRegistry
	virtual bool                 UseType(const ITypeDesc& typeDesc) override;

	virtual const DrxGUID&       GetGuid() const override final     { return m_guid; }
	virtual tukk          GetLabel() const override final    { return m_label.c_str(); }
	virtual const DrxTypeDesc&   GetTypeDesc() const override final { return m_typeDesc; }
	virtual DrxTypeId            GetTypeId() const override final   { return m_typeDesc.GetTypeId(); }

	virtual TypeIndex::ValueType GetTypeCount() const override;
	virtual const ITypeDesc*     FindTypeByIndex(TypeIndex index) const override;
	virtual const ITypeDesc*     FindTypeByGuid(const DrxGUID& guid) const override;
	virtual const ITypeDesc*     FindTypeById(DrxTypeId typeId) const override;
	// ~ICustomTypeRegistry

protected:
	std::vector<TypeIndex>                              m_typesByIndex;
	std::unordered_map<DrxGUID, TypeIndex>              m_typeIndicesByGuid;
	std::unordered_map<DrxTypeId::ValueType, TypeIndex> m_typeIndicesByTypeId;

private:
	const string      m_label;
	const DrxGUID     m_guid;
	const DrxTypeDesc m_typeDesc;
};

template<typename REGISTRY_INTERFACE_TYPE>
CSystemTypeRegistry<REGISTRY_INTERFACE_TYPE>::CSystemTypeRegistry(tukk szLabel, const DrxTypeDesc& typeDesc, const DrxGUID& guid)
	: m_label(szLabel)
	, m_typeDesc(typeDesc)
	, m_guid(guid)
{
	GetReflectionRegistry().RegisterSystemRegistry(this);
}

} // ~Reflection namespace
} // ~Drx namespace
