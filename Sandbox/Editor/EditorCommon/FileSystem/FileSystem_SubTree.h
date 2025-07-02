// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileSystem_File.h"
#include "FileSystem_Directory.h"

namespace FileSystem
{

struct SSubTree;
typedef QVector<SSubTree> SubTreeVector;

/// \brief sub tree of a directory
///
/// used for hierarchical filter results
struct SSubTree
{
	DirectoryPtr  directory;
	FileVector    files;
	SubTreeVector subDirectories;
	bool          wasConsidered;

public:
	inline SSubTree() : wasConsidered(false) {}
};

} //endns FileSystem

