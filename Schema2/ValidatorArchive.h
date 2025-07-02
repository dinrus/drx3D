// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IValidatorArchive.h>

namespace sxema2
{
	class CValidatorArchive : public IValidatorArchive
	{
	public:

		CValidatorArchive(const SValidatorArchiveParams& params);

		// IValidatorArchive

		virtual bool operator () (bool& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (int8& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (u8& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (i32& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (u32& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (int64& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (uint64& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (float& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (Serialization::IString& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (const Serialization::SStruct& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (Serialization::IContainer& value, tukk szName = "", tukk szLabel = nullptr) override;

		using IValidatorArchive::operator ();

		virtual u32 GetWarningCount() const override;
		virtual u32 GetErrorCount() const override;

		// ~IValidatorArchive

	protected:

		// Serialization::IArchive
		virtual void validatorMessage(bool bError, ukk handle, const Serialization::TypeID& type, tukk szMessage) override;
		// ~Serialization::IArchive

	private:

		EValidatorArchiveFlags m_flags;
		u32                 m_warningCount;
		u32                 m_errorCount;
	};
}
