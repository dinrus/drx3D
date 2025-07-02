// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

namespace Serialization
{

struct ResourceFolderPath
{
	string* path;
	string  startFolder;

	explicit ResourceFolderPath(string& path, tukk startFolder = "")
		: path(&path)
		, startFolder(startFolder)
	{
	}

	//! The function should stay virtual to ensure cross-dll calls are using right heap.
	virtual void SetPath(tukk path) { *this->path = path; }
};

bool Serialize(Serialization::IArchive& ar, Serialization::ResourceFolderPath& value, tukk name, tukk label);

}

#include "ResourceFolderPathImpl.h"
