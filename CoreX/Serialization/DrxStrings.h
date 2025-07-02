// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/DrxFixedString.h>
#include <drx3D/CoreX/Serialization/Forward.h>

// Note : if you are looking for the DrxStringT serialization, it is handled in Serialization/STL.h

template<size_t N>
bool Serialize(Serialization::IArchive& ar, DrxFixedStringT<N>& value, tukk name, tukk label);

template<size_t N>
bool Serialize(Serialization::IArchive& ar, DrxStackStringT<char, N>& value, tukk name, tukk label);

template<size_t N>
bool Serialize(Serialization::IArchive& ar, DrxStackStringT<wchar_t, N>& value, tukk name, tukk label);

#include <drx3D/CoreX/Serialization/DrxStringsImpl.h>
