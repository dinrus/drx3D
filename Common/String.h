#pragma once

#include <string>
#include <vector>
#include <optional>
#include <sstream>

#include <drx3D/Export.h>
#include <drx3D/Common/ConstExpr.h>


namespace drx3d {
/**
 * @brief Helper class for C++ strings.
 */
class DRX3D_EXPORT String {
public:
	String() = delete;

	/**
	 * Converts a CTF16 (wide) string to a UTF8 string.
	 * @param string The view of the string to convert.
	 * @return The converted string.
	 */
	static STxt ConvertUtf8(const std::wstring_view &string);

	/**
	 * Converts a CTF16 (wide) char to a UTF8 char.
	 * @param c The char to convert.
	 * @return The converted char.
	 */
	static char ConvertUtf8(wchar_t c);

	/**
	 * Converts a CTF8 string to a UTF16 (wide) string.
	 * @param string The view of the string to convert.
	 * @return The converted string.
	 */
	static std::wstring ConvertUtf16(const STxtview &string);

	/**
	 * Converts a CTF8 char to a UTF16 (wide) char.
	 * @param c The char to convert.
	 * @return The converted char.
	 */
	static wchar_t ConvertUtf16(char c);

	/**
	 * Splits a string by a separator.
	 * @param str The string.
	 * @param sep The separator.
	 * @return The split string vector.
	 */
	static std::vector<STxt> Split(const STxt &str, char sep);

	/**
	 * Gets if a string starts with a token.
	 * @param str The string.
	 * @param token The token.
	 * @return If a string starts with the token.
	 */
	static bool StartsWith(STxtview str, STxtview token);

	/**
	 * Gets if a string contains a token.
	 * @param str The string.
	 * @param token The token.
	 * @return If a string contains the token.
	 */
	static bool Contains(STxtview str, STxtview token) noexcept;

	/**
	 * Gets if a character is a whitespace.
	 * @param c The character.
	 * @return If a character is a whitespace.
	 */
	static bool IsWhitespace(char c) noexcept;

	/**
	 * Gets if a string is a number.
	 * @param str The string.
	 * @return If a string is a number.
	 */
	static bool IsNumber(STxtview str) noexcept;

	/**
	 * Gets the first char index in the string.
	 * @param str The string.
	 * @param c The char to look for.
	 * @return The char index.
	 */
	static int32_t FindCharPos(STxtview str, char c) noexcept;

	/**
	 * Trims the left and right side of a string of whitespace.
	 * @param str The string.
	 * @param whitespace The whitespace type.
	 * @return The trimmed string.
	 */
	static STxtview Trim(STxtview str, STxtview whitespace = " \t\n\r");

	/**
	 * Removes all tokens from a string.
	 * @param str The string.
	 * @param token The token.
	 * @return The string with the tokens removed.
	 */
	static STxt RemoveAll(STxt str, char token);

	/**
	 * Removes the last token from a string.
	 * @param str The string.
	 * @param token The token.
	 * @return The string with the last token removed.
	 */
	static STxt RemoveLast(STxt str, char token);

	/**
	 * Replaces all tokens from a string.
	 * @param str The string.
	 * @param token The token.
	 * @param to The string to replace the tokens with.
	 * @return The string with the tokens replaced.
	 */
	static STxt ReplaceAll(STxt str, STxtview token, STxtview to);

	/**
	 * Replaces the first token from a string.
	 * @param str The string.
	 * @param token The token.
	 * @param to The string to replace the tokens with.
	 * @return The string with the tokens replaced.
	 */
	static STxt ReplaceFirst(STxt str, STxtview token, STxtview to);

	/**
	 * Fixes all tokens return line tokens from a string.
	 * @param str The string.
	 * @return The string with return lines fixed.
	 */
	static STxt FixEscapedChars(STxt str);

	/**
	 * Unfixes all tokens return line tokens from a string.
	 * @param str The string.
	 * @return The string with return lines unfixed.
	 */
	static STxt UnfixEscapedChars(STxt str);

	/**
	 * Lower cases a string.
	 * @param str The string.
	 * @return The lowercased string.
	 */
	static STxt Lowercase(STxt str);

	/**
	 * Uppercases a string.
	 * @param str The string.
	 * @return The uppercased string.
	 */
	static STxt Uppercase(STxt str);

	/**
	 * Converts a type to a string.
	 * @tparam T The type to convert from.
	 * @param val The value to convert.
	 * @return The value as a string.
	 */
	template<typename T>
	static STxt To(T val) {
		if constexpr (std::is_same_v<STxt, T> || std::is_same_v<tukk , T>) {
			return val;
		} else if constexpr (std::is_enum_v<T>) {
			typedef typename std::underlying_type<T>::type safe_type;
			return std::to_string(static_cast<safe_type>(val));
		} else if constexpr (std::is_same_v<bool, T>) {
			return val ? "true" : "false";
		} else if constexpr (std::is_same_v<std::nullptr_t, T>) {
			return "null";
		} else if constexpr (is_optional_v<T>) {
			if (!val.has_value())
				return "null";
			return To(*val);
		} else if constexpr (std::is_same_v<char, T>) {
			return STxt(1, val);
		} else {
			return std::to_string(val);
		}
	}

	/**
	 * Converts a string to a type.
	 * @tparam T The type to convert to.
	 * @param str The string to convert.
	 * @return The string as a value.
	 */
	template<typename T>
	static T From(const STxt &str) {
		if constexpr (std::is_same_v<STxt, T>) {
			return str;
		} else if constexpr (std::is_enum_v<T>) {
			typedef typename std::underlying_type<T>::type safe_type;
			return static_cast<T>(From<safe_type>(str));
		} else if constexpr (std::is_same_v<bool, T>) {
			return str == "true" || From<std::optional<int32_t>>(str) == 1;
		} else if constexpr (is_optional_v<T>) {
			typedef typename T::value_type base_type;
			base_type temp;
			std::istringstream iss(str);

			if ((iss >> temp).fail())
				return std::nullopt;
			return temp;
		} else {
			long double temp;
			std::istringstream iss(str);
			iss >> temp;
			return static_cast<T>(temp);
		}
	}

	// fnv1a 32 and 64 bit hash functions
	// key is the data to hash, len is the size of the data (or how much of it to hash against)
	// code license: public domain or equivalent
	// post: https://notes.underscorediscovery.com/constexpr-fnv1a/

	constexpr static uint32_t fnv1a_32(STxtview str, const uint32_t value = 0x811c9dc5) noexcept {
		return str.size() == 0 ? value : fnv1a_32(STxtview(str.data() + 1, str.size() - 1),
			(value ^ uint32_t(str[0])) * 0x1000193);
	}

	constexpr static uint64_t fnv1a_64(STxtview str, const uint64_t value = 0xcbf29ce484222325) noexcept {
		return str.size() == 0 ? value : fnv1a_64(STxtview(str.data() + 1, str.size() - 1),
			(value ^ uint64_t(str[0])) * 0x100000001b3);
	}
};

constexpr uint32_t operator"" _hash(tukk s, std::size_t count) {
	return String::fnv1a_32(STxtview(s, count));
}
}
