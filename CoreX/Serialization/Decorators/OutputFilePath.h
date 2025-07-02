// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include <drx3D/CoreX/Serialization/Forward.h>

namespace Serialization
{

struct OutputFilePath
{
	string* path;
	//! If we don't use dynamic filters we could replace following two with tukk .
	string  filter;
	string  startFolder;

	//! Filters are defined in the following format:
	//! "All Images (bmp, jpg, tga)|*.bmp;*.jpg;*.tga|Targa (tga)|*.tga".
	explicit OutputFilePath(string& path, tukk filter = "All files|*.*", tukk startFolder = "")
		: path(&path)
		, filter(filter)
		, startFolder(startFolder)
	{
	}

	//! This function should stay virtual to ensure cross-dll calls are using right heap.
	virtual void SetPath(tukk path) { *this->path = path; }
};

bool Serialize(Serialization::IArchive& ar, Serialization::OutputFilePath& value, tukk name, tukk label);

}

#include "OutputFilePathImpl.h"
