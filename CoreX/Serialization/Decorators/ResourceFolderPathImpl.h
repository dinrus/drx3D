// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>

namespace Serialization
{

inline bool Serialize(Serialization::IArchive& ar, Serialization::ResourceFolderPath& value, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(value), name, label);
	else
		return ar(*value.path, name, label);
}

}
