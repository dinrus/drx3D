// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileSystem/FileSystem_SubTreeMonitor.h"

namespace FileSystem
{
namespace Internal
{

struct SSnapshotFileUpdate
{
	FilePtr from;
	FilePtr to;
};

struct SSnapshotDirectoryUpdate
{
	DirectoryPtr                      from;
	DirectoryPtr                      to;
	QVector<SSnapshotDirectoryUpdate> directoryUpdates;
	QVector<SSnapshotFileUpdate>      fileUpdates;
};

struct SSnapshotUpdate
{
	SnapshotPtr              from;
	SnapshotPtr              to;
	SSnapshotDirectoryUpdate root;
};

} //endns Internal
} //endns FileSystem

