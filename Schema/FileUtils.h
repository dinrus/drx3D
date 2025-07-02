// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/EnumFlags.h>

namespace sxema
{
namespace FileUtils
{
enum class EFileEnumFlags
{
	None                     = 0,
	IgnoreUnderscoredFolders = BIT(0),
	Recursive                = BIT(1),
};

typedef CEnumFlags<EFileEnumFlags>              FileEnumFlags;

typedef std::function<void (tukk , unsigned)> FileEnumCallback;

void EnumFilesInFolder(tukk szFolderName, tukk szExtension, FileEnumCallback callback, const FileEnumFlags& flags = EFileEnumFlags::Recursive);
} // FileUtils
} // sxema
