// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Serialization
{

inline bool Serialize(Serialization::IArchive& ar, Serialization::ResourceFilePath& value, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(value), name, label);
	else
		return ar(*value.path, name, label);
}

}
