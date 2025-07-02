// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IValidatorArchive.h>

namespace sxema
{
class CValidatorArchive : public IValidatorArchive
{
private:

	typedef std::vector<string> Messages;

public:

	CValidatorArchive(const SValidatorArchiveParams& params);

	// Serialization::IArchive
	virtual bool operator()(bool& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(int8& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(u8& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(i32& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(u32& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(int64& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(uint64& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(float& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(Serialization::IString& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(const Serialization::SStruct& value, tukk szName = "", tukk szLabel = nullptr) override;
	virtual bool operator()(Serialization::IContainer& value, tukk szName = "", tukk szLabel = nullptr) override;
	// ~Serialization::IArchive

	// IValidatorArchive
	virtual void   Validate(const Validator& validator) const override;
	virtual u32 GetWarningCount() const override;
	virtual u32 GetErrorCount() const override;
	// ~IValidatorArchive

	using IValidatorArchive::operator();

protected:

	// Serialization::IArchive
	virtual void validatorMessage(bool bError, ukk handle, const Serialization::TypeID& type, tukk szMessage) override;
	// ~Serialization::IArchive

private:

	ValidatorArchiveFlags m_flags;
	Messages              m_warnings;
	Messages              m_errors;
};
} // sxema
