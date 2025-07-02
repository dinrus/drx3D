// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Reflection/IReflection.h>
#include <drx3D/Reflection/IFunctionDesc.h>
#include <drx3D/Reflection/ITypeDesc.h>
#include <drx3D/CoreX/Extension/DrxGUID.h>
#include <drx3D/CoreX/String/DrxString.h>

#include <drx3D/Reflection/DescExtension.h>

namespace Drx {
namespace Reflection {

class CFunctionParameterDesc : public IFunctionParameterDesc
{
public:
	CFunctionParameterDesc(const SFunctionParameterDesc& paramDesc)
		: m_flags(paramDesc.flags)
		, m_typeId(paramDesc.typeId)
		, m_index(paramDesc.index)
	{}

	// IFunctionParameterDesc
	virtual DrxTypeId          GetTypeId() const override                  { return m_typeId; }
	virtual u32             GetIndex() const override                   { return m_index; }

	virtual FunctionParamFlags GetFlags() const override                   { return m_flags; }
	virtual void               SetFlags(FunctionParamFlags flags) override { m_flags = flags; }

	virtual bool               IsConst() const override                    { return m_flags.Check(EFunctionParamFlags::IsConst); }
	virtual bool               IsReference() const override                { return m_flags.Check(EFunctionParamFlags::IsReference); }
	// ~IFunctionParameterDesc

private:
	FunctionParamFlags m_flags;
	DrxTypeId          m_typeId;
	u32             m_index;
};

class CReflectedFunctionDesc : public CExtensibleDesc<IFunctionDesc>
{
public:
	CReflectedFunctionDesc(const ITypeDesc* pReturnType, const CFunction& func, const ParamArray& params, tukk szLabel, const DrxGUID& guid);
	CReflectedFunctionDesc(const ITypeDesc* pObjectType, DrxTypeId returnTypeId, const CFunction& func, const ParamArray& params, tukk szLabel, const DrxGUID& guid);

	// IFunctionDesc
	virtual const DrxGUID&                GetGuid() const override                           { return m_guid; }

	virtual tukk                   GetLabel() const override                          { return m_label.c_str(); }
	virtual void                          SetLabel(tukk szLabel)     override         { m_label = szLabel; }

	virtual tukk                   GetDescription() const override                    { return m_description.c_str(); }
	virtual void                          SetDescription(tukk szDescription) override { m_description = szDescription; }

	virtual bool                          IsMemberFunction() const override                  { return (m_pObjectType != nullptr); }

	virtual const CFunction&              GetFunction() const override                       { return m_function; }

	virtual const ITypeDesc*              GetObjectTypeDesc() const override;
	virtual DrxTypeId                     GetReturnTypeId() const override      { return m_returnTypeId; }

	virtual size_t                        GetParamCount() const override        { return m_params.size(); }
	virtual const IFunctionParameterDesc* GetParam(size_t index) const override { return index < m_params.size() ? &m_params[index] : nullptr; }
	// ~IFunctionDesc

private:
	DrxGUID                             m_guid;
	CFunction                           m_function;

	string                              m_label;
	string                              m_description;

	const ITypeDesc*                    m_pObjectType;
	DrxTypeId                           m_returnTypeId;
	std::vector<CFunctionParameterDesc> m_params;

	bool                                m_isConstFunction;
};

inline CReflectedFunctionDesc::CReflectedFunctionDesc(const ITypeDesc* pReturnType, const CFunction& func, const ParamArray& params, tukk szLabel, const DrxGUID& guid)
	: m_function(func)
	, m_label(szLabel)
	, m_isConstFunction(false)
	, m_guid(guid)
{
	for (const SFunctionParameterDesc& desc : params)
	{
		m_params.emplace_back(desc);
	}
}

inline CReflectedFunctionDesc::CReflectedFunctionDesc(const ITypeDesc* pObjectType, DrxTypeId returnTypeId, const CFunction& func, const ParamArray& params, tukk szLabel, const DrxGUID& guid)
	: m_pObjectType(pObjectType)
	, m_returnTypeId(returnTypeId)
	, m_function(func)
	, m_label(szLabel)
	, m_isConstFunction(false)
	, m_guid(guid)
{
	for (const SFunctionParameterDesc& desc : params)
	{
		m_params.emplace_back(desc);
	}
}

} // ~Drx namespace
} // ~Reflection namespace
