// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IString.h>

namespace sxema
{
namespace StringUtils
{
// Is character numeric?
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool CharIsNumeric(char x)
{
	return ((x >= '0') && (x <= '9'));
}

// Is character alphanumeric?
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool CharIsAlphanumeric(char x)
{
	return ((x >= 'a') && (x <= 'z')) || ((x >= 'A') && (x <= 'Z')) || ((x >= '0') && (x <= '9'));
}

// Check to see if string contains non-alphanumeric characters.
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool ContainsNonAlphaNumericChars(tukk szInput, tukk szExceptions = nullptr)
{
	if (szInput)
	{
		for (tukk szPos = szInput; *szPos != '\0'; ++szPos)
		{
			if (!CharIsAlphanumeric(*szPos) && (!szExceptions || !strchr(szExceptions, *szPos)))
			{
				return true;
			}
		}
	}
	return false;
}

// Check to see if string contains at least on alphanumeric character.
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool ContainsAlphaNumericChars(tukk szInput)
{
	if (szInput)
	{
		for (tukk szPos = szInput; *szPos != '\0'; ++szPos)
		{
			if (CharIsAlphanumeric(*szPos))
			{
				return true;
			}
		}
	}
	return false;
}

// Convert hexadecimal character to u8.
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr u8 HexCharToUnisgnedChar(char x)
{
	return x >= '0' && x <= '9' ? x - '0' :
	       x >= 'a' && x <= 'f' ? x - 'a' + 10 :
	       x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
}

// Convert hexadecimal character to unsigned short.
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr unsigned short HexCharToUnisgnedShort(char x)
{
	return x >= '0' && x <= '9' ? x - '0' :
	       x >= 'a' && x <= 'f' ? x - 'a' + 10 :
	       x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
}

// Convert hexadecimal character to u64.
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr u64 HexCharToUnisgnedLong(char x)
{
	return x >= '0' && x <= '9' ? x - '0' :
	       x >= 'a' && x <= 'f' ? x - 'a' + 10 :
	       x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
}

// Convert hexadecimal string to u8.
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr u8 HexStringToUnsignedChar(tukk szInput)
{
	return (HexCharToUnisgnedChar(szInput[0]) << 4) +
	       HexCharToUnisgnedChar(szInput[1]);
}

// Convert hexadecimal string to unsigned short.
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr unsigned short HexStringToUnsignedShort(tukk szInput)
{
	return (HexCharToUnisgnedShort(szInput[0]) << 12) +
	       (HexCharToUnisgnedShort(szInput[1]) << 8) +
	       (HexCharToUnisgnedShort(szInput[2]) << 4) +
	       HexCharToUnisgnedShort(szInput[3]);
}

// Convert hexadecimal string to u64.
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr u64 HexStringToUnsignedLong(tukk szInput)
{
	return (HexCharToUnisgnedLong(szInput[0]) << 28) +
	       (HexCharToUnisgnedLong(szInput[1]) << 24) +
	       (HexCharToUnisgnedLong(szInput[2]) << 20) +
	       (HexCharToUnisgnedLong(szInput[3]) << 16) +
	       (HexCharToUnisgnedLong(szInput[4]) << 12) +
	       (HexCharToUnisgnedLong(szInput[5]) << 8) +
	       (HexCharToUnisgnedLong(szInput[6]) << 4) +
	       HexCharToUnisgnedLong(szInput[7]);
}

// Filter string.
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Filter(tukk szInput, tukk szFilter)
{
	// Convert inputs to lower case.

	string input = szInput;
	input.MakeLower();

	string filter = szFilter;
	filter.MakeLower();

	// Tokenize filter and ensure input contains all words in filter.

	string::size_type pos = 0;
	string::size_type filterEnd = -1;
	do
	{
		const string::size_type filterStart = filterEnd + 1;
		filterEnd = filter.find(' ', filterStart);
		pos = input.find(filter.substr(filterStart, filterEnd - filterStart), pos);
		if (pos == string::npos)
		{
			return false;
		}
		pos += filterEnd - filterStart;
	}
	while (filterEnd != string::npos);
	return true;
}

// Convert string to 'snake' case (all lower case with underscore word separators).
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void ToSnakeCase(IString& value)
{
	string snakeCase;
	bool bPrevLowercase = false;
	for (tukk szSrc = value.c_str(); szSrc[0] != '\0'; ++szSrc)
	{
		if (szSrc[0] == ' ')
		{
			snakeCase.append("_");
		}
		else if ((szSrc[0] >= 'A') && (szSrc[0] <= 'Z'))
		{
			if (bPrevLowercase)
			{
				snakeCase.append("_");
			}
			snakeCase.append(1, szSrc[0] + ('a' - 'A'));
		}
		else
		{
			snakeCase.append(1, szSrc[0]);
		}
		bPrevLowercase = (szSrc[0] >= 'a') && (szSrc[0] <= 'z');
	}
	value.assign(snakeCase.c_str());
}

inline bool IsValidElementName(tukk szName, tukk & szErrorMessage)
{
	if (!szName || szName[0] == '\0')
	{
		szErrorMessage = "Name cannot be empty.";
		return false;
	}

	if (CharIsNumeric(szName[0]) || szName[0] == ' ' || szName[0] == '_')
	{
		szErrorMessage = "Name cannot start with a number, underscore or whitespace.";
		return false;
	}

	u32 numAlphaNumericChars = 0;
	tukk szPos = szName;
	for (; *szPos != '\0'; ++szPos)
	{
		if (CharIsAlphanumeric(*szPos))
		{
			++numAlphaNumericChars;
			continue;
		}

		if (*szPos != ' ' && *szPos != '_')
		{
			szErrorMessage = "Name can only contain alphanumeric characters, underscores and whitespaces.";
			return false;
		}
	}

	if (numAlphaNumericChars == 0)
	{
		szErrorMessage = "Name must at least contain one alphanumeric character.";
		return false;
	}

	if (szPos[-1] == ' ')
	{
		szErrorMessage = "Name cannot end with a whitespace.";
		return false;
	}

	return true;
}
} // StringUtils
} // sxema
