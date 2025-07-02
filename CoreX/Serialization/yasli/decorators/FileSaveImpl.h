// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/CoreX/Serialization/yasli/decorators/FileSave.h>
#include <drx3D/CoreX/Serialization/yasli/STL.h>
#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include <drx3D/CoreX/Serialization/yasli/STLImpl.h>

namespace yasli {

YASLI_INLINE void FileSave::YASLI_SERIALIZE_METHOD(Archive& ar)
{
	ar(path, "path");
	ar(filter, "filter");
	ar(relativeToFolder, "folder");
}

YASLI_INLINE bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, FileSave& value, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serializer(value), name, label);
	else
		return ar(value.path, name, label);
}

}

