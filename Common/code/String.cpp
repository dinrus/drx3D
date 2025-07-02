#include <drx3D/Common/String.h>

#include <codecvt>
#include <locale>
#include <algorithm>

namespace drx3d {
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> UTF8_TO_UTF16_CONVERTER;

STxt String::ConvertUtf8(const std::wstring_view &string) {
	return UTF8_TO_UTF16_CONVERTER.to_bytes(string.data(), string.data() + string.length());
}

char String::ConvertUtf8(wchar_t c) {
	return UTF8_TO_UTF16_CONVERTER.to_bytes(c)[0];
}

std::wstring String::ConvertUtf16(const STxtview &string) {
	return UTF8_TO_UTF16_CONVERTER.from_bytes(string.data(), string.data() + string.length());
}

wchar_t String::ConvertUtf16(char c) {
	return UTF8_TO_UTF16_CONVERTER.from_bytes(c)[0];
}

std::vector<STxt> String::Split(const STxt &str, char sep) {
	std::vector<STxt> tokens;
	STxt token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, sep))
		tokens.emplace_back(token);
	return tokens;
}

bool String::StartsWith(STxtview str, STxtview token) {
	if (str.length() < token.length())
		return false;
	return str.compare(0, token.length(), token) == 0;
}

bool String::Contains(STxtview str, STxtview token) noexcept {
	return str.find(token) != STxt::npos;
}

bool String::IsWhitespace(char c) noexcept {
	return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

bool String::IsNumber(STxtview str) noexcept {
	return std::all_of(str.cbegin(), str.cend(), [](auto c) {
		return (c >= '0' && c <= '9') || c == '.' || c == '-';
	});
}

int32_t String::FindCharPos(STxtview str, char c) noexcept {
	auto res = str.find(c);
	return res == STxt::npos ? -1 : static_cast<int32_t>(res);
}

STxtview String::Trim(STxtview str, STxtview whitespace) {
	auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == STxt::npos)
		return "";

	auto strEnd = str.find_last_not_of(whitespace);
	auto strRange = strEnd - strBegin + 1;
	return str.substr(strBegin, strRange);
}

STxt String::RemoveAll(STxt str, char token) {
	str.erase(std::remove(str.begin(), str.end(), token), str.end());
	return str;
}

STxt String::RemoveLast(STxt str, char token) {
	for (auto it = str.end(); it != str.begin(); --it) {
		if (*it == token) {
			str.erase(it);
			return str;
		}
	}

	return str;
}

STxt String::ReplaceAll(STxt str, STxtview token, STxtview to) {
	auto pos = str.find(token);
	while (pos != STxt::npos) {
		str.replace(pos, token.size(), to);
		pos = str.find(token, pos + token.size());
	}

	return str;
}

STxt String::ReplaceFirst(STxt str, STxtview token, STxtview to) {
	const auto startPos = str.find(token);
	if (startPos == STxt::npos)
		return str;

	str.replace(startPos, token.length(), to);
	return str;
}

STxt String::FixEscapedChars(STxt str) {
	static const std::vector<std::pair<char, STxtview>> replaces = {{'\\', "\\\\"}, {'\n', "\\n"}, {'\r', "\\r"}, {'\t', "\\t"}, {'\"', "\\\""}};

	for (const auto &[from, to] : replaces) {
		auto pos = str.find(from);
		while (pos != STxt::npos) {
			str.replace(pos, 1, to);
			pos = str.find(from, pos + 2);
		}
	}

	return str;
}

STxt String::UnfixEscapedChars(STxt str) {
	static const std::vector<std::pair<STxtview, char>> replaces = {{"\\n", '\n'}, {"\\r", '\r'}, {"\\t", '\t'}, {"\\\"", '\"'}, {"\\\\", '\\'}};

	for (const auto &[from, to] : replaces) {
		auto pos = str.find(from);
		while (pos != STxt::npos) {
			if (pos != 0 && str[pos - 1] == '\\')
				str.erase(str.begin() + --pos);
			else
				str.replace(pos, from.size(), 1, to);
			pos = str.find(from, pos + 1);
		}
	}

	return str;
}

STxt String::Lowercase(STxt str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

STxt String::Uppercase(STxt str) {
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}
}
