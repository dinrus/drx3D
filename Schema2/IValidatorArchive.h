// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/CoreX/Serialization/Forward.h>

namespace sxema2
{
	enum class EValidatorArchiveFlags
	{
		None                 = 0,
		ForwardWarningsToLog = BIT(1),
		ForwardErrorsToLog   = BIT(2)
	};

	DECLARE_ENUM_CLASS_FLAGS(EValidatorArchiveFlags)

	struct SValidatorArchiveParams
	{
		inline SValidatorArchiveParams(EValidatorArchiveFlags _flags = EValidatorArchiveFlags::None)
			: flags(_flags)
		{}

		EValidatorArchiveFlags flags;
	};

	struct IValidatorArchive : public Serialization::IArchive
	{
		inline IValidatorArchive(i32 caps)
			: Serialization::IArchive(caps)
		{}

		virtual ~IValidatorArchive() {}

		virtual u32 GetWarningCount() const = 0;
		virtual u32 GetErrorCount() const = 0;

		using Serialization::IArchive::operator ();
	};

	DECLARE_SHARED_POINTERS(IValidatorArchive)
}
