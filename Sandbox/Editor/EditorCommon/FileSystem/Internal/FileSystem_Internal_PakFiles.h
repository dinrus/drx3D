// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileSystem_Internal_Data.h"

namespace FileSystem
{
namespace Internal
{

class CPakFiles
{
public:
	SArchiveContentInternal GetContents(const QString& keyEnginePath) const;

#ifdef EDITOR_COMMON_EXPORTS
private:
	void GetContentsInternal(const STxt& archiveEnginePath, const QString& fullLocalPath, SArchiveContentInternal& result, QStringList& directoryList) const;
#endif
};

} //endns Internal
} //endns FileSystem

