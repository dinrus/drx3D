// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/STL.h>

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/IString.h>

namespace sxema
{

class CSharedString;

SXEMA_DECLARE_STRING_TYPE(CSharedString)

class CSharedString : public IString
{
	friend bool Serialize(Serialization::IArchive& archive, CSharedString& value, tukk szName, tukk szLabel);

private:

	typedef std::shared_ptr<string> StringPtr;

public:

	inline CSharedString() {}

	inline CSharedString(const CSharedString& rhs)
		: m_pString(rhs.m_pString)
	{}

	inline CSharedString(tukk szInput)
		: m_pString(std::make_shared<string>(szInput))
	{}

	inline CSharedString(const string& value)
		: m_pString(std::make_shared<string>(value))
	{}

	// IString

	virtual tukk c_str() const override
	{
		return m_pString ? m_pString->c_str() : "";
	}

	virtual u32 length() const override
	{
		return m_pString ? m_pString->length() : 0;
	}

	virtual bool empty() const override
	{
		return m_pString ? m_pString->empty() : true;
	}

	virtual void clear() override
	{
		m_pString.reset();
	}

	virtual void assign(tukk szInput) override
	{
		if (!m_pString || !m_pString.unique())
		{
			m_pString = std::make_shared<string>(szInput);
		}
		else
		{
			m_pString->assign(szInput);
		}
	}

	virtual void assign(tukk szInput, u32 offset, u32 length) override
	{
		if (!m_pString || !m_pString.unique())
		{
			m_pString = std::make_shared<string>(szInput, offset, length);
		}
		else
		{
			m_pString->assign(szInput, offset, length);
		}
	}

	virtual void append(tukk szInput) override
	{
		if (!m_pString)
		{
			m_pString = std::make_shared<string>();
		}
		else if (!m_pString.unique())
		{
			m_pString = std::make_shared<string>(*m_pString);
		}

		m_pString->append(szInput);
	}

	virtual void append(tukk szInput, u32 offset, u32 length) override
	{
		if (!m_pString)
		{
			m_pString = std::make_shared<string>();
		}
		else if (!m_pString.unique())
		{
			m_pString = std::make_shared<string>(*m_pString);
		}

		m_pString->append(szInput, offset, length);
	}

	virtual void insert(u32 offset, tukk szInput) override
	{
		if (!m_pString)
		{
			m_pString = std::make_shared<string>();
		}
		else if (!m_pString.unique())
		{
			m_pString = std::make_shared<string>(*m_pString);
		}

		m_pString->insert(offset, szInput);
	}

	virtual void insert(u32 offset, tukk szInput, u32 length) override
	{
		if (!m_pString)
		{
			m_pString = std::make_shared<string>();
		}
		else if (!m_pString.unique())
		{
			m_pString = std::make_shared<string>(*m_pString);
		}

		m_pString->insert(offset, szInput, length);
	}

	virtual void Format(tukk szFormat, ...) override
	{
		if (!m_pString || !m_pString.unique())
		{
			m_pString = std::make_shared<string>();
		}

		va_list va_args;
		va_start(va_args, szFormat);

		m_pString->FormatV(szFormat, va_args);

		va_end(va_args);
	}

	virtual void TrimLeft(tukk szChars) override
	{
		if (m_pString)
		{
			if (!m_pString.unique())
			{
				m_pString = std::make_shared<string>();
			}

			m_pString->TrimLeft(szChars);
		}
	}

	virtual void TrimRight(tukk szChars) override
	{
		if (m_pString)
		{
			if (!m_pString.unique())
			{
				m_pString = std::make_shared<string>();
			}

			m_pString->TrimRight(szChars);
		}
	}

	// ~IString

	inline void ToString(IString& output) const
	{
		output.assign(c_str());
	}

	inline void operator=(const CSharedString& rhs)
	{
		m_pString = rhs.m_pString;
	}

	inline void operator=(tukk szInput)
	{
		if (!m_pString || !m_pString.unique())
		{
			m_pString = std::make_shared<string>(szInput);
		}
		else
		{
			m_pString->assign(szInput);
		}
	}

	inline void operator=(const string& rhs)
	{
		if (!m_pString || !m_pString.unique())
		{
			m_pString = std::make_shared<string>(rhs);
		}
		else
		{
			m_pString->assign(rhs);
		}
	}

	inline bool operator==(const CSharedString& rhs) const
	{
		if (m_pString && rhs.m_pString)
		{
			return (m_pString == rhs.m_pString) || (*m_pString == *rhs.m_pString);
		}

		return m_pString == nullptr && rhs.m_pString == nullptr;
	}

	inline bool operator!=(const CSharedString& rhs) const
	{
		if (m_pString && rhs.m_pString)
		{
			return (m_pString != rhs.m_pString) && (*m_pString != *rhs.m_pString);
		}

		return m_pString || rhs.m_pString;
	}

	static inline void ReflectType(CTypeDesc<CSharedString>& desc)
	{
		desc.SetGUID("02b79308-c51c-4841-99ec-c887577217a7"_drx_guid);
		desc.SetLabel("String");
		desc.SetDescription("String");
		desc.SetFlags(ETypeFlags::Switchable);
		desc.SetToStringOperator<& CSharedString::ToString>();
		desc.SetStringGetCharsOperator<& CSharedString::c_str>();
	}

private:

	StringPtr m_pString;
};

inline bool Serialize(Serialization::IArchive& archive, CSharedString& value, tukk szName, tukk szLabel)
{
	bool bResult = false;
	if (archive.isInput())
	{
		string temp;
		bResult = archive(temp, szName, szLabel);
		value = temp;
	}
	else if (archive.isOutput())
	{
		if (value.m_pString)
		{
			bResult = archive(*value.m_pString, szName, szLabel);
		}
		else
		{
			string temp;
			bResult = archive(temp, szName, szLabel);
		}
	}
	return bResult;
}
} // sxema
