// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/EnvElementBase.h>
#include <drx3D/Schema/IEnvSignal.h>
#include <drx3D/Schema/TypeDesc.h>

#define SXEMA_MAKE_ENV_SIGNAL(type) sxema::EnvSignal::MakeShared<type>(SXEMA_SOURCE_FILE_INFO)

namespace sxema
{

class CEnvSignal : public CEnvElementBase<IEnvSignal>
{
public:

	inline CEnvSignal(const CClassDesc& desc, const SSourceFileInfo& sourceFileInfo)
		: CEnvElementBase(sourceFileInfo)
		, m_desc(desc)
	{
		CEnvElementBase::SetGUID(desc.GetGUID());
		CEnvElementBase::SetName(desc.GetLabel());
		CEnvElementBase::SetDescription(desc.GetDescription());
	}

	// IEnvElement

	virtual bool IsValidScope(IEnvElement& scope) const override
	{
		switch (scope.GetType())
		{
		case EEnvElementType::Root:
		case EEnvElementType::Module:
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

	// IEnvSignal

	virtual const CClassDesc& GetDesc() const override
	{
		return m_desc;
	}

	// ~IEnvSignal

private:

	const CClassDesc& m_desc;
};

namespace EnvSignal
{

template<typename TYPE> inline std::shared_ptr<CEnvSignal> MakeShared(const SSourceFileInfo& sourceFileInfo)
{
	static_assert(std::is_default_constructible<TYPE>::value, "Type must be default constructible!");
	static_assert(std::is_class<TYPE>::value, "Type must be struct/class!");
	SXEMA_VERIFY_TYPE_IS_REFLECTED(TYPE);

	return std::make_shared<CEnvSignal>(GetTypeDesc<TYPE>(), sourceFileInfo);
}

} // EnvSignal
} // sxema
