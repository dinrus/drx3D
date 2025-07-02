// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/IArchive.h>

#include <drx3D/Schema/IString.h>

namespace sxema
{
namespace SerializationUtils
{
namespace Private
{
class CStringOArchive : public Serialization::IArchive
{
public:

	inline CStringOArchive(IString& output)
		: Serialization::IArchive(Serialization::IArchive::OUTPUT)
		, m_output(output)
	{}

	// Serialization::IArchive

	virtual bool operator()(bool& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		m_output.append(value ? "1" : "0");
		m_output.append(", ");
		return true;
	}

	virtual bool operator()(int8& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		{
			char temp[8] = "";
			itoa(value, temp, 10);
			m_output.append(temp);
		}
		m_output.append(", ");
		return true;
	}

	virtual bool operator()(u8& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		{
			char temp[8] = "";
			ltoa(value, temp, 10);
			m_output.append(temp);
		}
		m_output.append(", ");
		return true;
	}

	virtual bool operator()(i32& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		{
			char temp[16] = "";
			itoa(value, temp, 10);
			m_output.append(temp);
		}
		m_output.append(", ");
		return true;
	}

	virtual bool operator()(u32& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		{
			char temp[16] = "";
			ltoa(value, temp, 10);
			m_output.append(temp);
		}
		m_output.append(", ");
		return true;
	}

	virtual bool operator()(int64& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		{
			char temp[32] = "";
			drx_sprintf(temp, "%llu", static_cast<zu64>(value)); // #SchematycTODO : Make sure %llu works on Windows platforms!!!
			m_output.append(temp);
		}
		m_output.append(", ");
		return true;
	}

	virtual bool operator()(uint64& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		{
			char temp[32] = "";
			drx_sprintf(temp, "%lld", static_cast<zu64>(value)); // #SchematycTODO : Make sure %lld works on Windows platforms!!!
			m_output.append(temp);
		}
		m_output.append(", ");
		return true;
	}

	virtual bool operator()(float& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		{
			char temp[64] = "";
			drx_sprintf(temp, "%.8f", value);
			m_output.append(temp);
		}
		m_output.append(", ");
		return true;
	}

	virtual bool operator()(Serialization::IString& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		m_output.append(value.get());
		m_output.append(", ");
		return true;
	}

	virtual bool operator()(const Serialization::SStruct& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		PrintHeader(szName, szLabel);
		m_output.append("{ ");
		value(*this);
		m_output.TrimRight(", ");
		m_output.append(" }, ");
		return true;
	}

	virtual bool operator()(Serialization::IContainer& value, tukk szName = "", tukk szLabel = nullptr) override
	{
		if (value.size())
		{
			PrintHeader(szName, szLabel);
			m_output.append("[ ");
			do
			{
				value(*this, "", nullptr);
			}
			while (value.next());
			m_output.TrimRight(", ");
			m_output.append(" ], ");
		}
		return true;
	}

	using Serialization::IArchive::operator();

	// ~Serialization::IArchive

private:

	inline void PrintHeader(tukk szName, tukk szLabel)
	{
		if (szLabel && szLabel[0])
		{
			m_output.append(szLabel);
			m_output.append(" = ");
		}
		else if (szName[0])
		{
			m_output.append(szName);
			m_output.append(" = ");
		}
	}

	IString& m_output;
};

template<typename TYPE, bool TYPE_IS_ENUM = std::is_enum<TYPE>::value> struct SToStringHelper;

template<typename TYPE> struct SToStringHelper<TYPE, true>
{
	inline bool operator()(IString& output, const TYPE& input) const
	{
		const Serialization::EnumDescription& enumDescription = Serialization::getEnumDescription<TYPE>();
		output.assign(enumDescription.labelByIndex(enumDescription.indexByValue(static_cast<i32>(input))));
		return true;
	}
};

template<typename TYPE> struct SToStringHelper<TYPE, false>
{
	inline bool operator()(IString& output, const TYPE& input) const
	{
		CStringOArchive archive(output);
		const bool bResult = archive(input);
		output.TrimRight(", ");
		return bResult;
	}
};
}   // Private

template<typename TYPE> bool ToString(IString& output, const TYPE& input)
{
	return Private::SToStringHelper<TYPE>()(output, input);
}
} // SerializationUtils
} // sxema
