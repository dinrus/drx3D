// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/IArchive.h>

#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/EnumFlags.h>
#include <drx3D/Schema/Validator.h>

namespace sxema
{
enum class EValidatorArchiveFlags
{
	None                 = 0,
	ForwardWarningsToLog = BIT(1),
	ForwardErrorsToLog   = BIT(2)
};

typedef CEnumFlags<EValidatorArchiveFlags> ValidatorArchiveFlags;

struct SValidatorArchiveParams
{
	inline SValidatorArchiveParams(const ValidatorArchiveFlags& _flags = EValidatorArchiveFlags::None)
		: flags(_flags)
	{}

	ValidatorArchiveFlags flags;
};

typedef std::function<void (tukk)> ValidatorMessageEnumerator; // #SchematycTODO : Replace with Validator?

struct IValidatorArchive : public Serialization::IArchive
{
	inline IValidatorArchive(i32 caps)
		: Serialization::IArchive(caps)
	{}

	virtual ~IValidatorArchive() {}

	virtual void   Validate(const Validator& validator) const = 0;
	virtual u32 GetWarningCount() const = 0;
	virtual u32 GetErrorCount() const = 0;

	using Serialization::IArchive::operator();
};

DECLARE_SHARED_POINTERS(IValidatorArchive)
} // sxema
