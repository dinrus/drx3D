// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Memory/STLPoolAllocator.h>

#include <drx3D/Schema/Action.h>
#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/EnvElementBase.h>
#include <drx3D/Schema/IEnvAction.h>
#include <drx3D/Schema/ActionDesc.h>

#define SXEMA_MAKE_ENV_ACTION(type) sxema::EnvAction::MakeShared<type>(SXEMA_SOURCE_FILE_INFO)

namespace sxema
{

template<typename TYPE> class CEnvAction : public CEnvElementBase<IEnvAction> // #SchematycTODO : Does this type need to be template or can we simply store a const reference to the type description?
{
public:

	inline CEnvAction(const SSourceFileInfo& sourceFileInfo)
		: CEnvElementBase(sourceFileInfo)
	{
		const CCommonTypeDesc& desc = sxema::GetTypeDesc<TYPE>();
		CEnvElementBase::SetGUID(desc.GetGUID());
		CEnvElementBase::SetName(desc.GetLabel());
		CEnvElementBase::SetDescription(desc.GetDescription());
	}

	// IEnvElement

	virtual bool IsValidScope(IEnvElement& scope) const override
	{
		switch (scope.GetType())
		{
		case EEnvElementType::Module:
		case EEnvElementType::Class:
		case EEnvElementType::Component:
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

	// IEnvAction

	virtual const CActionDesc& GetDesc() const override
	{
		return sxema::GetTypeDesc<TYPE>();
	}

	virtual CActionPtr CreateFromPool() const override
	{
		return std::allocate_shared<TYPE>(m_allocator);
	}

	// ~IEnvAction

private:

	stl::STLPoolAllocator<TYPE> m_allocator;
};

namespace EnvAction
{

template<typename TYPE> inline std::shared_ptr<CEnvAction<TYPE>> MakeShared(const SSourceFileInfo& sourceFileInfo)
{
	static_assert(std::is_base_of<CAction, TYPE>::value, "Type must be derived from sxema::CAction!");
	SXEMA_VERIFY_TYPE_IS_REFLECTED(TYPE);

	return std::make_shared<CEnvAction<TYPE>>(sourceFileInfo);
}

} // EnvAction
} // sxema
