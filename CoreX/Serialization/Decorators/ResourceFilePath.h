// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

namespace Serialization
{

struct ResourceFilePath
{
	enum { STRIP_EXTENSION = 1 << 0 };

	string* path;
	string  filter;
	string  startFolder;
	i32     flags;

	//! Filters are defined in the following format:
	//! "All Images (bmp, jpg, tga)|*.bmp;*.jpg;*.tga|Targa (tga)|*.tga".
	explicit ResourceFilePath(string& path, tukk filter = "All files|*.*", tukk startFolder = "", i32 flags = 0)
		: path(&path)
		, filter(filter)
		, startFolder(startFolder)
		, flags(flags)
	{
	}

	//! This function should stay virtual to ensure cross-dll calls are using right heap.
	virtual void SetPath(tukk path) { *this->path = path; }
};

bool Serialize(Serialization::IArchive& ar, Serialization::ResourceFilePath& value, tukk name, tukk label);

}

#include "ResourceFilePathImpl.h"
