// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Serialization
{

inline bool Serialize(Serialization::IArchive& ar, Serialization::ToggleButton& button, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(button), name, label);
	else
		return ar(*button.value, name, label);
}

inline bool Serialize(Serialization::IArchive& ar, Serialization::RadioButton& button, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(button), name, label);
	else
		return false;
}

}

