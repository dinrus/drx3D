// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "UnicodeIterator.h"

#include <algorithm> // std::min()
#include <cstdio>    // vsnprintf()
#include <cstdarg>   // va_list

namespace DrxStringUtils
{
//! Convert a single ASCII character to lower case, this is compatible with the standard "C" locale (ie, only A-Z).
inline char toLowerAscii(char c)
{
	return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

//! Convert a single ASCII character to upper case, this is compatible with the standard "C" locale (ie, only A-Z).
inline char toUpperAscii(char c)
{
	return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
}
}

//! \cond INTERNAL
//! drx_sXXX()/drx_vsXXX() and DrxStringUtils_Internal::strXXX()/vsprintfXXX():
//! The functions copy characters from src to dst one by one until any of
//! the following conditions is met:
//! 1) the end of the destination buffer (minus one character) is reached.
//! 2) the end of the source buffer is reached.
//! 3) zero character is found in the source buffer.
//! When any of 1), 2), 3) happens, the functions write the terminating zero
//! character to the destination buffer and return.
//! The functions guarantee writing the terminating zero character to the
//! destination buffer (if the buffer can fit at least one character).
//! The functions return false when a null pointer is passed or when
//! clamping happened (i.e. when the end of the destination buffer is
//! reached but the source has some characters left).
namespace DrxStringUtils_Internal
{

template<class TChar>
inline bool strcpy_with_clamp(TChar* const dst, size_t const dst_size_in_bytes, const TChar* const src, size_t const src_size_in_bytes)
{
	static_assert(sizeof(TChar) == sizeof(char) || sizeof(TChar) == sizeof(wchar_t), "Invalid type size!");

	if (!dst || dst_size_in_bytes < sizeof(TChar))
	{
		return false;
	}

	if (!src || src_size_in_bytes < sizeof(TChar))
	{
		dst[0] = 0;
		return src != 0;  // we return true for non-null src without characters
	}

	const size_t src_n = src_size_in_bytes / sizeof(TChar);
	const size_t n = (std::min)(dst_size_in_bytes / sizeof(TChar) - 1, src_n);

	for (size_t i = 0; i < n; ++i)
	{
		dst[i] = src[i];
		if (!src[i])
		{
			return true;
		}
	}

	dst[n] = 0;
	return n >= src_n || src[n] == 0;
}

template<class TChar>
inline bool strcat_with_clamp(TChar* const dst, size_t const dst_size_in_bytes, const TChar* const src, size_t const src_size_in_bytes)
{
	static_assert(sizeof(TChar) == sizeof(char) || sizeof(TChar) == sizeof(wchar_t), "Invalid type size!");

	if (!dst || dst_size_in_bytes < sizeof(TChar))
	{
		return false;
	}

	const size_t dst_n = dst_size_in_bytes / sizeof(TChar) - 1;

	size_t dst_len = 0;
	while (dst_len < dst_n && dst[dst_len])
	{
		++dst_len;
	}

	if (!src || src_size_in_bytes < sizeof(TChar))
	{
		dst[dst_len] = 0;
		return src != 0;  // we return true for non-null src without characters
	}

	const size_t src_n = src_size_in_bytes / sizeof(TChar);
	const size_t n = (std::min)(dst_n - dst_len, src_n);
	TChar* dst_ptr = &dst[dst_len];

	for (size_t i = 0; i < n; ++i)
	{
		*dst_ptr++ = src[i];
		if (!src[i])
		{
			return true;
		}
	}

	*dst_ptr = 0;
	return n >= src_n || src[n] == 0;
}

inline bool vsprintf_with_clamp(tuk const dst, size_t const dst_size_in_bytes, tukk const format, va_list args)
{
	if (!dst || dst_size_in_bytes < 1)
	{
		return false;
	}

	if (!format)
	{
		dst[0] = 0;
		return false;
	}

	PREFAST_SUPPRESS_WARNING(4996); // 'function': was declared deprecated
	i32k n = vsnprintf(dst, dst_size_in_bytes, format, args);

#if DRX_COMPILER_MSVC && DRX_COMPILER_VERSION < 1900
	if (n < 0 || n == dst_size_in_bytes)
	{
		// Truncated or has no space for the terminating zero character
		dst[dst_size_in_bytes - 1] = 0;
		return false;
	}
	assert(n < (i32)dst_size_in_bytes);
	return true;
#else
	if (n >= (i32)dst_size_in_bytes)
	{
		// Truncated.
		// Note: vsnprintf() always writes the terminating zero.
		return false;
	}
	if (n < 0)
	{
		// An error occurred
		dst[0] = 0;
		return false;
	}
	return true;
#endif
}

//! Returns number of characters (char) needed to store result of the
//! formatted output. Trailing zero is not counted. In case of an error
//! the function returns a negative value.
inline i32 compute_length_formatted_va(tukk const format, va_list args)
{
	if (!format || !format[0])
	{
		return 0;
	}

	PREFAST_SUPPRESS_WARNING(4996); // 'function': was declared deprecated
	return vsnprintf(nullptr, 0, format, args);
}

//! Returns number of characters (char) needed to store result of the
//! formatted output. Trailing zero is not counted. In case of an error
//! the function returns a negative value.
inline i32 compute_length_formatted(tukk const format, ...)
{
	va_list args;
	va_start(args, format);
	i32k n = compute_length_formatted_va(format, args);
	va_end(args);
	return n;
}

//! Compares characters case-sensitively. Locale agnostic.
template<class CharType>
struct SCharComparatorCaseSensitive
{
	static bool IsEqual(CharType a, CharType b)
	{
		return a == b;
	}
};

//! Compares characters case-insensitively, uses the standard "C" locale.
template<class CharType>
struct SCharComparatorCaseInsensitive
{
	static bool IsEqual(CharType a, CharType b)
	{
		if (a < 0x80 && b < 0x80)
		{
			a = (CharType)DrxStringUtils::toLowerAscii(char(a));
			b = (CharType)DrxStringUtils::toLowerAscii(char(b));
		}
		return a == b;
	}
};

//! Template for wildcard matching, UCS code-point aware.
//! Can be used for ASCII and Unicode (UTF-8/UTF-16/UTF-32), but not for ANSI.
//! ? will match exactly one code-point.
//! * will match zero or more code-points.
//! Note: function moved here from DrxCommonTools.
template<template<class> class CharComparator, class CharType>
static inline bool MatchesWildcards_Tpl(const CharType* pStr, const CharType* pWild)
{
	const CharType* savedStr = 0;
	const CharType* savedWild = 0;

	while ((*pStr) && (*pWild != '*'))
	{
		if (!CharComparator<CharType>::IsEqual(*pWild, *pStr) && (*pWild != '?'))
		{
			return false;
		}

		// We need special handling of '?' for Unicode
		if (*pWild == '?' && *pStr > 127)
		{
			Unicode::CIterator<const CharType*> utf(pStr, pStr + 4);
			if (utf.IsAtValidCodepoint())
			{
				pStr = (++utf).GetPosition();
				--pStr;
			}
		}

		++pWild;
		++pStr;
	}

	while (*pStr)
	{
		if (*pWild == '*')
		{
			if (!*++pWild)
			{
				return true;
			}
			savedWild = pWild;
			savedStr = pStr + 1;
		}
		else if (CharComparator<CharType>::IsEqual(*pWild, *pStr) || (*pWild == '?'))
		{
			// We need special handling of '?' for Unicode
			if (*pWild == '?' && *pStr > 127)
			{
				Unicode::CIterator<const CharType*> utf(pStr, pStr + 4);
				if (utf.IsAtValidCodepoint())
				{
					pStr = (++utf).GetPosition();
					--pStr;
				}
			}

			++pWild;
			++pStr;
		}
		else
		{
			pWild = savedWild;
			pStr = savedStr++;
		}
	}

	while (*pWild == '*')
	{
		++pWild;
	}

	return *pWild == 0;
}

} // namespace DrxStringUtils_Internal
//! \endcond

//////////////////////////////////////////////////////////////////////////
// drx_strcpy(), drx_strcpy_wchar()

inline bool drx_strcpy(_Out_writes_z_(dst_size_in_bytes) tuk const dst, size_t const dst_size_in_bytes, tukk const src)
{
	return DrxStringUtils_Internal::strcpy_with_clamp<char>(dst, dst_size_in_bytes, src, (size_t)-1);
}

inline bool drx_strcpy(_Out_writes_z_(dst_size_in_bytes) tuk const dst, size_t const dst_size_in_bytes, tukk const src, size_t const src_size_in_bytes)
{
	return DrxStringUtils_Internal::strcpy_with_clamp<char>(dst, dst_size_in_bytes, src, src_size_in_bytes);
}

template<size_t SIZE_IN_CHARS>
inline bool drx_strcpy(_Out_writes_z_(SIZE_IN_CHARS) char (&dst)[SIZE_IN_CHARS], tukk const src)
{
	return DrxStringUtils_Internal::strcpy_with_clamp<char>(dst, SIZE_IN_CHARS, src, (size_t)-1);
}

template<size_t SIZE_IN_CHARS>
inline bool drx_strcpy(_Out_writes_z_(SIZE_IN_CHARS) char (&dst)[SIZE_IN_CHARS], tukk const src, size_t const src_size_in_bytes)
{
	return DrxStringUtils_Internal::strcpy_with_clamp<char>(dst, SIZE_IN_CHARS, src, src_size_in_bytes);
}

inline bool drx_strcpy_wchar(_Out_writes_z_(dst_size_in_bytes) wchar_t* const dst, size_t const dst_size_in_bytes, const wchar_t* const src)
{
	return DrxStringUtils_Internal::strcpy_with_clamp<wchar_t>(dst, dst_size_in_bytes, src, (size_t)-1);
}

inline bool drx_strcpy_wchar(_Out_writes_z_(dst_size_in_bytes) wchar_t* const dst, size_t const dst_size_in_bytes, const wchar_t* const src, size_t const src_size_in_bytes)
{
	return DrxStringUtils_Internal::strcpy_with_clamp<wchar_t>(dst, dst_size_in_bytes, src, src_size_in_bytes);
}

template<size_t SIZE_IN_WCHARS>
inline bool drx_strcpy_wchar(_Out_writes_z_(SIZE_IN_CHARS*2) wchar_t (&dst)[SIZE_IN_WCHARS], const wchar_t* const src)
{
	return DrxStringUtils_Internal::strcpy_with_clamp<wchar_t>(dst, SIZE_IN_WCHARS * sizeof(wchar_t), src, (size_t)-1);
}

template<size_t SIZE_IN_WCHARS>
inline bool drx_strcpy_wchar(_Out_writes_z_(SIZE_IN_CHARS*2) wchar_t (&dst)[SIZE_IN_WCHARS], const wchar_t* const src, size_t const src_size_in_bytes)
{
	return DrxStringUtils_Internal::strcpy_with_clamp<wchar_t>(dst, SIZE_IN_WCHARS * sizeof(wchar_t), src, src_size_in_bytes);
}

//////////////////////////////////////////////////////////////////////////
// drx_strcat(), drx_strcat_wchar()

inline bool drx_strcat( _Inout_updates_z_(dst_size_in_bytes) tuk const dst, size_t const dst_size_in_bytes, tukk const src)
{
	return DrxStringUtils_Internal::strcat_with_clamp<char>(dst, dst_size_in_bytes, src, (size_t)-1);
}

template<size_t SIZE_IN_CHARS>
inline bool drx_strcat( _Inout_updates_z_(SIZE_IN_CHARS) char (&dst)[SIZE_IN_CHARS], tukk const src)
{
	return DrxStringUtils_Internal::strcat_with_clamp<char>(dst, SIZE_IN_CHARS, src, (size_t)-1);
}

inline bool drx_strcat( _Inout_updates_z_(dst_size_in_bytes) tuk const dst, size_t const dst_size_in_bytes, tukk const src, size_t const src_size_in_bytes)
{
	return DrxStringUtils_Internal::strcat_with_clamp<char>(dst, dst_size_in_bytes, src, src_size_in_bytes);
}

template<size_t SIZE_IN_CHARS>
inline bool drx_strcat( _Inout_updates_z_(SIZE_IN_CHARS) char (&dst)[SIZE_IN_CHARS], tukk const src, size_t const src_size_in_bytes)
{
	return DrxStringUtils_Internal::strcat_with_clamp<char>(dst, SIZE_IN_CHARS, src, src_size_in_bytes);
}

inline bool drx_strcat_wchar( _Inout_updates_z_(dst_size_in_bytes) wchar_t* const dst, size_t const dst_size_in_bytes, const wchar_t* const src)
{
	return DrxStringUtils_Internal::strcat_with_clamp<wchar_t>(dst, dst_size_in_bytes, src, (size_t)-1);
}

template<size_t SIZE_IN_WCHARS>
inline bool drx_strcat_wchar( _Inout_updates_z_(SIZE_IN_CHARS*2) wchar_t (&dst)[SIZE_IN_WCHARS], const wchar_t* const src)
{
	return DrxStringUtils_Internal::strcat_with_clamp<wchar_t>(dst, SIZE_IN_WCHARS * sizeof(wchar_t), src, (size_t)-1);
}

inline bool drx_strcat_wchar( _Inout_updates_z_(dst_size_in_bytes) wchar_t* const dst, size_t const dst_size_in_bytes, const wchar_t* const src, size_t const src_size_in_bytes)
{
	return DrxStringUtils_Internal::strcat_with_clamp<wchar_t>(dst, dst_size_in_bytes, src, src_size_in_bytes);
}

template<size_t SIZE_IN_WCHARS>
inline bool drx_strcat_wchar( _Inout_updates_z_(SIZE_IN_CHARS*2) wchar_t (&dst)[SIZE_IN_WCHARS], const wchar_t* const src, size_t const src_size_in_bytes)
{
	return DrxStringUtils_Internal::strcat_with_clamp<wchar_t>(dst, SIZE_IN_WCHARS * sizeof(wchar_t), src, src_size_in_bytes);
}

//////////////////////////////////////////////////////////////////////////
// drx_vsprintf(), drx_sprintf()

inline bool drx_vsprintf(_Out_writes_z_(dst_size_in_bytes) tuk const dst, size_t const dst_size_in_bytes, tukk const format, va_list args)
{
	return DrxStringUtils_Internal::vsprintf_with_clamp(dst, dst_size_in_bytes, format, args);
}

template<size_t SIZE_IN_CHARS>
inline bool drx_vsprintf(_Out_writes_z_(SIZE_IN_CHARS) char (&dst)[SIZE_IN_CHARS], tukk const format, va_list args)
{
	return DrxStringUtils_Internal::vsprintf_with_clamp(dst, SIZE_IN_CHARS, format, args);
}

inline bool drx_sprintf(_Out_writes_z_(dst_size_in_bytes) tuk const dst, size_t const dst_size_in_bytes, tukk const format, ...)
{
	va_list args;
	va_start(args, format);
	const bool b = DrxStringUtils_Internal::vsprintf_with_clamp(dst, dst_size_in_bytes, format, args);
	va_end(args);
	return b;
}

template<size_t SIZE_IN_CHARS>
inline bool drx_sprintf(_Out_writes_z_(SIZE_IN_CHARS) char (&dst)[SIZE_IN_CHARS], tukk const format, ...)
{
	va_list args;
	va_start(args, format);
	const bool b = DrxStringUtils_Internal::vsprintf_with_clamp(dst, SIZE_IN_CHARS, format, args);
	va_end(args);
	return b;
}

//////////////////////////////////////////////////////////////////////////
// drx_strcmp, drx_strncmp, drx_strcmpi, drx_strnicmp

inline i32 drx_strcmp(tukk string1, tukk string2)
{
	return strcmp(string1, string2);
}

inline i32 drx_stricmp(tukk string1, tukk string2)
{
#if DRX_PLATFORM_WINDOWS
	//_stricmp is deprecated according to MSDN
	return _stricmp(string1, string2);
#else
	return stricmp(string1, string2);
#endif
}

inline i32 drx_strncmp(tukk string1, tukk string2, size_t count)
{
	return strncmp(string1, string2, count);
}

template<size_t STRING2_CHAR_COUNT> inline i32 drx_strncmp(tukk string1, const char(&string2)[STRING2_CHAR_COUNT])
{
	return strncmp(string1, string2, STRING2_CHAR_COUNT - 1);
}

inline i32 drx_strnicmp(tukk string1, tukk string2, size_t count)
{
#if DRX_PLATFORM_WINDOWS
	//strnicmp is deprecated according to MSDN
	return _strnicmp(string1, string2, count);
#else
	return strnicmp(string1, string2, count);
#endif
}
