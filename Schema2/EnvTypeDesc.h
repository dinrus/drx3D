// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Rename file!

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Any.h>
#include <drx3D/Schema2/IEnvTypeDesc.h>
#include <drx3D/Schema2/TypeInfo.h>

namespace sxema2
{
	template <typename TYPE> inline EEnvTypeCategory GetEnvTypeCategory()
	{
		if(std::is_fundamental<TYPE>::value)
		{
			return EEnvTypeCategory::Fundamental;
		}
		else if(std::is_enum<TYPE>::value)
		{
			return EEnvTypeCategory::Enumeration;
		}
		else if(std::is_class<TYPE>::value)
		{
			return EEnvTypeCategory::Class;
		}
		else
		{
			SXEMA2_SYSTEM_FATAL_ERROR("Unable to discern type category!");
		}
	}

	template <typename TYPE> class CEnvTypeDesc: public IEnvTypeDesc
	{
	public:

		inline CEnvTypeDesc(const SGUID& guid, tukk szName, tukk szDescription, const TYPE& defaultValue, EEnvTypeFlags flags)
			: m_guid(guid)
			, m_name(szName)
			, m_description(szDescription)
			, m_defaultValue(defaultValue)
			, m_flags(flags)
		{}

		// ITypeDesc

		virtual EnvTypeId GetEnvTypeId() const override // #SchematycTODO : Remove, we can access type id from type info!!!
		{
			return sxema2::GetEnvTypeId<TYPE>();
		}

		virtual SGUID GetGUID() const override
		{
			return m_guid;
		}

		virtual tukk GetName() const override
		{
			return m_name.c_str();
		}

		virtual tukk GetDescription() const override
		{
			return m_description.c_str();
		}

		virtual CTypeInfo GetTypeInfo() const override
		{
			return sxema2::GetTypeInfo<TYPE>();
		}

		virtual EEnvTypeCategory GetCategory() const override
		{
			return GetEnvTypeCategory<TYPE>();
		}

		virtual EEnvTypeFlags GetFlags() const override
		{
			return m_flags;
		}

		virtual IAnyPtr Create() const override
		{
			return MakeAnyShared(m_defaultValue);
		}

		// ITypeDesc

	private:

		string        m_name;
		SGUID         m_guid;
		string        m_description;
		TYPE          m_defaultValue;
		EEnvTypeFlags m_flags;
	};

	template <typename TYPE> inline IEnvTypeDescPtr MakeEnvTypeDescShared(const SGUID& guid, tukk szName, tukk szDescription, const TYPE& defaultValue, EEnvTypeFlags flags)
	{
		return IEnvTypeDescPtr(std::make_shared<CEnvTypeDesc<TYPE> >(guid, szName, szDescription, defaultValue, flags));
	}
}
