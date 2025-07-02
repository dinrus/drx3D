// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Platform/platform.h>

#if defined(EDITOR_COMMON_EXPORTS)
#define PROPERTY_TREE_API __declspec(dllexport)
#elif defined(EDITOR_COMMON_IMPORTS)
#define PROPERTY_TREE_API __declspec(dllimport)
#else
#define PROPERTY_TREE_API 
#endif

#if defined(DRX_PLATFORM_ORBIS) || defined(DRX_PLATFORM_ANDROID)
#define YASLI_NO_FCVT 1
#else
#define YASLI_NO_FCVT 0
#endif

#define YASLI_SERIALIZE_METHOD Serialize
#define YASLI_SERIALIZE_OVERRIDE Serialize

#define YASLI_STRINGS_DEFINED
#ifdef RESOURCE_COMPILER
namespace yasli {
typedef DrxStringLocalT<char> string;
typedef DrxStringLocalT<wchar_t> wstring;
}
#else
namespace yasli {
typedef ::string string;
typedef ::wstring wstring;
}
#endif
namespace Serialization
{
typedef yasli::string string;
typedef yasli::wstring wstring;
}

#define YASLI_STRING_NAMESPACE_BEGIN 
#define YASLI_STRING_NAMESPACE_END 

#define YASLI_ASSERT_DEFINED
#define YASLI_ASSERT(x) DRX_ASSERT(x)
#define YASLI_ASSERT_STR(x,str) DRX_ASSERT_MESSAGE(x,str)
#define YASLI_ESCAPE(x, action) if(!(x)) { YASLI_ASSERT(0 && #x); action; };
#define YASLI_CHECK(x) (x)

#define YASLI_INTS_DEFINED 1
namespace yasli
{
	typedef int8 i8;
	typedef i16 i16;
	typedef i32 i32;
	typedef int64 i64;
	typedef u8 u8;
	typedef u16 u16;
	typedef u32 u32;
	typedef uint64 u64;
}

#define YASLI_INLINE_IMPLEMENTATION 1
#define YASLI_INLINE inline
#define YASLI_NO_MAP_AS_DICTIONARY 1

#ifndef DRX_PLATFORM_DESKTOP
#define YASLI_NO_EDITING 1
#endif

// Override string list types so that we can safely pass string lists over dll boundaries.


namespace yasli
{
	class Archive;
}


#include <drx3D/CoreX/Containers/DrxArray.h>

template<class T, class I, class S>
bool Serialize(yasli::Archive& ar, DynArray<T, I, S>& container, tukk name, tukk label);

namespace yasli
{
	typedef ::DynArray<tukk > StringListStaticBase;
	typedef ::DynArray<string> StringListBase;
}

#define YASLI_STRING_LIST_BASE_DEFINED
#define YASLI_INCLUDE_PROPERTY_TREE_CONFIG_LOCAL 1