// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/IProperties.h>
#include <drx3D/Schema2/DefaultInit.h>

namespace sxema2
{
	// Requirements of TYPE parameter are:
	//   - void Serialize(Serialization::IArchive&); // Create IsSerializable test to display helpful compile errors?
	template <typename TYPE> class CProperties : public IProperties
	{
	public:

		inline CProperties(const TYPE& value)
			: m_value(value)
		{}

		inline CProperties(const CProperties& rhs)
			: m_value(rhs.m_value)
		{}

		// IProperties

		virtual EnvTypeId GetEnvTypeId() const override
		{
			return sxema2::GetEnvTypeId<TYPE>();
		}

		virtual IPropertiesPtr Clone() const override
		{
			return std::make_shared<CProperties>(*this);
		}

		virtual GameSerialization::IContextPtr BindSerializationContext(Serialization::IArchive& archive) const override
		{
			return std::make_shared<GameSerialization::CContext<const TYPE> >(archive, m_value);
		}

		virtual void Serialize(Serialization::IArchive& archive) override
		{
			m_value.Serialize(archive);
		}

		virtual uk ToVoidPtr() override
		{
			return &m_value;
		}

		virtual ukk ToVoidPtr() const override
		{
			return &m_value;
		}
	
		// ~IProperties

	private:

		TYPE m_value;
	};

	namespace Properties
	{
		template <typename TYPE> inline CProperties<TYPE> Make()
		{
			return CProperties<TYPE>(DefaultInit<TYPE>());
		}

		template <typename TYPE> inline CProperties<TYPE> Make(const TYPE& value)
		{
			return CProperties<TYPE>(value);
		}

		template <typename TYPE> inline std::shared_ptr<CProperties<TYPE> > MakeShared()
		{
			return std::make_shared<CProperties<TYPE> >(DefaultInit<TYPE>());
		}

		template <typename TYPE> inline std::shared_ptr<CProperties<TYPE> > MakeShared(const TYPE& value)
		{
			return std::make_shared<CProperties<TYPE> >(value);
		}
	}
}
