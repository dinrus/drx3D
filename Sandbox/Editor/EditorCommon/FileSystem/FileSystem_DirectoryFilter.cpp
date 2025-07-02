// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "FileSystem_DirectoryFilter.h"
#include "FileSystem_EnginePath.h"

namespace FileSystem
{

SDirectoryFilter SDirectoryFilter::ForDirectory(const QString& directory)
{
	SDirectoryFilter filter;
	DRX_ASSERT(!directory.isEmpty());
	filter.directories.push_back(directory);
	return filter;
}

SDirectoryFilter SDirectoryFilter::ForDirectoryTree(const QString& directory)
{
	SDirectoryFilter filter = ForDirectory(directory);
	filter.recursiveSubdirectories = true;
	return filter;
}

void SDirectoryFilter::MakeInputValid()
{
	for (auto& directoryPath : directories)
	{
		directoryPath = SEnginePath::ConvertUserToKeyPath(directoryPath);
	}
	for (auto& token : directoryTokens)
	{
		DRX_ASSERT(token == token.toLower());
	}
	// empty means scan in engine root path
	if (directories.empty())
	{
		directories << QString();
	}
}

} //endns FileSystem

