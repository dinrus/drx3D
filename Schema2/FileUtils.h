// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/TemplateUtils_Delegate.h>

namespace sxema2
{
	namespace FileUtils
	{
		enum class EFileEnumFlags
		{
			None                     = 0,
			IgnoreUnderscoredFolders = BIT(0),
			Recursive                = BIT(1),
		};

		DECLARE_ENUM_CLASS_FLAGS(EFileEnumFlags)

		typedef TemplateUtils::CDelegate<void (tukk , unsigned)> FileEnumCallback;

		void EnumFilesInFolder(tukk szFolderName, tukk szExtension, FileEnumCallback callback, EFileEnumFlags flags = EFileEnumFlags::Recursive);
	}
}