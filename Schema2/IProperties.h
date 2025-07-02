// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/EnvTypeId.h>
#include <drx3D/Schema2/SerializationUtils.h>

namespace sxema2
{
	struct IProperties;

	DECLARE_SHARED_POINTERS(IProperties)

	struct IProperties
	{
		virtual ~IProperties() {}

		virtual EnvTypeId GetEnvTypeId() const = 0;
		virtual IPropertiesPtr Clone() const = 0;
		virtual GameSerialization::IContextPtr BindSerializationContext(Serialization::IArchive& archive) const = 0; // #SchematycTODO : Can we query domain context instead?
		virtual void Serialize(Serialization::IArchive& archive) = 0;
		virtual uk ToVoidPtr() = 0;
		virtual ukk ToVoidPtr() const = 0;
		
		template <typename TYPE> inline TYPE* ToPtr()
		{
			if(GetEnvTypeId() == sxema2::GetEnvTypeId<TYPE>()) // #SchematycTODO : Perform type check in actual implementation?
			{
				return static_cast<TYPE*>(ToVoidPtr());
			}
			return nullptr;
		}

		template <typename TYPE> inline const TYPE* ToPtr() const
		{
			if(GetEnvTypeId() == sxema2::GetEnvTypeId<TYPE>()) // #SchematycTODO : Perform type check in actual implementation?
			{
				return static_cast<const TYPE*>(ToVoidPtr());
			}
			return nullptr;
		}
	};
}
