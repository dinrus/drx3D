// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Memory/STLPoolAllocator.h>

#include <drx3D/Schema/Component.h>
#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/EnvElementBase.h>
#include <drx3D/Schema/IEnvComponent.h>

#define SXEMA_MAKE_ENV_COMPONENT(component) sxema::EnvComponent::MakeShared<component>(SXEMA_SOURCE_FILE_INFO)

namespace sxema
{

template<typename COMPONENT> class CEnvComponent : public CEnvElementBase<IEnvComponent>
{
public:

	inline CEnvComponent(const SSourceFileInfo& sourceFileInfo)
		: CEnvElementBase(sourceFileInfo)
	{
		const CEntityComponentClassDesc& desc = sxema::GetTypeDesc<COMPONENT>();
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

	// IEnvComponent

	virtual const CEntityComponentClassDesc& GetDesc() const override
	{
		return GetTypeDesc<COMPONENT>();
	}

	virtual std::shared_ptr<IEntityComponent> CreateFromPool() const override
	{
		return std::allocate_shared<COMPONENT>(m_allocator);
	}

	virtual size_t GetSize() const override
	{
		return sizeof(COMPONENT);
	}

	virtual std::shared_ptr<IEntityComponent> CreateFromBuffer(uk pBuffer) const override
	{
		struct CustomDeleter
		{
			void operator()(COMPONENT* pComponent)
			{
				// Explicit call to the destructor
				pComponent->~COMPONENT();
			}
		};

		return std::shared_ptr<COMPONENT>(new(pBuffer) COMPONENT(), CustomDeleter());
	}

	// ~IEnvComponent

private:
	// Align for possible SIMD types
	stl::STLPoolAllocator<COMPONENT, stl::PSyncMultiThread, alignof(COMPONENT)> m_allocator;
};

namespace EnvComponent
{

template<typename TYPE> inline std::shared_ptr<CEnvComponent<TYPE>> MakeShared(const SSourceFileInfo& sourceFileInfo)
{
	static_assert(std::is_base_of<IEntityComponent, TYPE>::value, "Type must be derived from IEntityComponent!");
	SXEMA_VERIFY_TYPE_IS_REFLECTED(TYPE);

	return std::make_shared<CEnvComponent<TYPE>>(sourceFileInfo);
}

} // EnvComponent
} // sxema
