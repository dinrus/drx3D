// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/EnvElementBase.h>
#include <drx3D/Schema/IEnvDataType.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/TypeName.h>

#define SXEMA_MAKE_ENV_DATA_TYPE(type) sxema::EnvDataType::MakeShared<type>(SXEMA_SOURCE_FILE_INFO)

namespace sxema
{

template<typename TYPE> class CEnvDataType : public CEnvElementBase<IEnvDataType>
{
public:

	inline CEnvDataType(const SSourceFileInfo& sourceFileInfo)
		: CEnvElementBase(sourceFileInfo)
	{
		const CCommonTypeDesc& typeDesc = sxema::GetTypeDesc<TYPE>();
		CEnvElementBase::SetGUID(typeDesc.GetGUID());
		CEnvElementBase::SetName(typeDesc.GetLabel());
		CEnvElementBase::SetDescription(typeDesc.GetDescription());
	}

	// IEnvElement

	virtual bool IsValidScope(IEnvElement& scope) const override
	{
		const EEnvElementType elementType = scope.GetType();
		switch (elementType)
		{
		case EEnvElementType::Root:
		case EEnvElementType::Module:
		case EEnvElementType::Class:
		case EEnvElementType::Component:
		case EEnvElementType::Action:
			{
				return true;
			}
		default:
			{
				return false;
			}
		}
	}

	// ~IEnvElement

	// IEnvType

	virtual const CCommonTypeDesc& GetDesc() const override
	{
		return sxema::GetTypeDesc<TYPE>();
	}

	// IEnvType
};

namespace EnvDataType
{

template<typename TYPE> inline std::shared_ptr<CEnvDataType<TYPE>> MakeShared(const SSourceFileInfo& sourceFileInfo)
{
	static_assert(std::is_default_constructible<TYPE>::value, "Type must be default constructible!");
	SXEMA_VERIFY_TYPE_IS_REFLECTED(TYPE);

	return std::make_shared<CEnvDataType<TYPE>>(sourceFileInfo);
}

} // EnvDataType
} // sxema
