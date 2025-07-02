// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/STL.h>

#include <drx3D/Schema2/IString.h>

namespace sxema2
{
class CStackString : public IString, public stack_string
{
public:

	inline CStackString() {}

	inline CStackString(tukk szValue)
		: stack_string(szValue)
	{}

	inline CStackString(const stack_string& rhs)
		: stack_string(rhs)
	{}

	inline CStackString(const CStackString& rhs)
		: stack_string(rhs)
	{}

	// IStringWrapper

	virtual tukk c_str() const override
	{
		return stack_string::c_str();
	}

	virtual u32 length() const override
	{
		return stack_string::length();
	}

	virtual bool empty() const override
	{
		return stack_string::empty();
	}

	virtual void clear() override
	{
		stack_string::clear();
	}

	virtual void assign(tukk szInput) override
	{
		stack_string::assign(szInput);
	}

	virtual void assign(tukk szInput, u32 offset, u32 length) override
	{
		stack_string::assign(szInput, offset, length);
	}

	virtual void append(tukk szInput) override
	{
		stack_string::append(szInput ? szInput : "");
	}

	virtual void append(tukk szInput, u32 offset, u32 length) override
	{
		stack_string::append(szInput ? szInput : "", offset, length);
	}

	virtual void insert(u32 offset, tukk szInput) override
	{
		stack_string::insert(offset, szInput);
	}

	virtual void insert(u32 offset, tukk szInput, u32 length) override
	{
		stack_string::insert(offset, szInput, length);
	}

	virtual void Format(tukk szFormat, ...) override
	{
		va_list va_args;
		va_start(va_args, szFormat);

		stack_string::FormatV(szFormat, va_args);

		va_end(va_args);
	}

	virtual void TrimLeft(tukk szChars) override
	{
		stack_string::TrimLeft(szChars);
	}

	virtual void TrimRight(tukk szChars) override
	{
		stack_string::TrimRight(szChars);
	}

	// ~IStringWrapper

	inline CStackString& operator=(tukk szValue)
	{
		stack_string::operator=(szValue);
		return *this;
	}
};

namespace SerializationUtils
{
class CStackStringWrapper : public Serialization::IString
{
public:

	inline CStackStringWrapper(CStackString& value)
		: m_value(value)
	{}

	// Serialization::IString

	virtual void set(tukk szValue) override
	{
		m_value = szValue;
	}

	virtual tukk get() const override
	{
		return m_value.c_str();
	}

	virtual ukk handle() const override
	{
		return &m_value;
	}

	virtual Serialization::TypeID type() const override
	{
		return Serialization::TypeID::get<CStackString>();
	}

	// Serialization::IString

private:

	CStackString& m_value;
};
}

inline bool Serialize(Serialization::IArchive& archive, CStackString& value, tukk szName, tukk szLabel)
{
	SerializationUtils::CStackStringWrapper wrapper(value);
	return archive(static_cast<Serialization::IString&>(wrapper), szName, szLabel);
}
} // sxema
