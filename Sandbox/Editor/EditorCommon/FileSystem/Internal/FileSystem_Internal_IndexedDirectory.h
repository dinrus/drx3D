// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileSystem/FileSystem_Directory.h"

namespace FileSystem
{
namespace Internal
{

/// internal extended directory structure
struct SIndexedDirectory : SDirectory
{
	// indices
	QMultiHash<QString, DirectoryPtr> tokenDirectories;
	QMultiHash<QString, FilePtr>      tokenFiles;
	QMultiHash<QString, FilePtr>      extensionFiles;
};

} //endns Internal
} //endns FileSystem

