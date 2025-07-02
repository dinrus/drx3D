/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <unordered_map>

#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include <drx3D/CoreX/Serialization/yasli/Serializer.h>
#include <drx3D/CoreX/Serialization/yasli/KeyValue.h>

namespace yasli{ class Archive; }

namespace std{ // not nice, but needed for argument-dependent lookup to work

template<class T, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::vector<T, Alloc>& container, tukk name, tukk label);

template<class T, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::list<T, Alloc>& container, tukk name, tukk label);

template<class K, class V, class C, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::map<K, V, C, Alloc>& container, tukk name, tukk label);

#if !YASLI_NO_MAP_AS_DICTIONARY
template<class V>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::pair<yasli::string, V>& pair, tukk name, tukk label);
#endif

template<class K, class V>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::pair<K, V>& pair, tukk name, tukk label);

#if YASLI_CXX11
template<class K, class V, class C, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::unordered_map<K, V, C, Alloc>& container, tukk name, tukk label);
template<class T>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::shared_ptr<T>& pair, tukk name, tukk label);
template<class T, class D>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::unique_ptr<T, D>& pair, tukk name, tukk label);
#endif

}

YASLI_STRING_NAMESPACE_BEGIN // std by default
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, yasli::string& value, tukk name, tukk label);
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, yasli::wstring& value, tukk name, tukk label);
YASLI_STRING_NAMESPACE_END


#include "STLImpl.h"
