// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание:
Macros for automatically building enumerations and matching tuk arrays
-------------------------------------------------------------------------
История:
- 6:11:2009: Created by Tim Furnish

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/AutoEnum.h>
#include <drx3D/Game/Utility/StringUtils.h>

#define DO_PARSE_BITFIELD_STRING_LOGS 0

TBitfield AutoEnum_GetBitfieldFromString(tukk  inString, tukk* inArray, i32 arraySize)
{
	u32 reply = 0;

	if (inString && inString[0] != '\0') // Avoid a load of work if the string's NULL or empty
	{
		tukk  startFrom = inString;

		assert (arraySize > 0);

		char skipThisString[32];
		size_t skipChars = drx_copyStringUntilFindChar(skipThisString, inArray[0], sizeof(skipThisString), '_');
		size_t foundAtIndex = 0;

#if DO_PARSE_BITFIELD_STRING_LOGS
		DrxLog("AutoEnum_GetBitfieldFromString: Parsing '%s' (skipping first %d chars '%s%s' of each string in array)", inString, skipChars, skipThisString, skipChars ? "_" : "");
#endif

		do
		{
			char gotToken[32];
			foundAtIndex = drx_copyStringUntilFindChar(gotToken, startFrom, sizeof(gotToken), '|');
			startFrom += foundAtIndex;

			bool done = false;
			for (i32 i = 0; i < arraySize; ++ i)
			{
				if (0 == stricmp(inArray[i] + skipChars, gotToken))
				{
					DRX_ASSERT_MESSAGE((reply & BIT(i)) == 0, string().Format("Bit '%s' already turned on! Does it feature more than once in string '%s'?", gotToken, inString));

#if DO_PARSE_BITFIELD_STRING_LOGS
					DrxLog("AutoEnum_GetBitfieldFromString: Token = '%s' = BIT(%d) = %d, remaining string = '%s'", gotToken, i, BIT(i), foundAtIndex ? startFrom : "");
#endif

					reply |= BIT(i);
					done = true;
					break;
				}
			}
			DRX_ASSERT_MESSAGE(done, string().Format("No flag called '%s' in list", gotToken));
		}
		while (foundAtIndex);
	}

	return reply;
}

bool AutoEnum_GetEnumValFromString(tukk inString, tukk* inArray, i32 arraySize, i32* outVal)
{
	bool  done = false;

	if (inString && (inString[0] != '\0'))  // avoid a load of work if the string's NULL or empty
	{
		DRX_ASSERT(arraySize > 0);

		char  skipThisString[32];
		size_t  skipChars = drx_copyStringUntilFindChar(skipThisString, inArray[0], sizeof(skipThisString), '_');

#if DO_PARSE_BITFIELD_STRING_LOGS
		DrxLog("AutoEnum_GetEnumValFromString: Parsing '%s' (skipping first %d chars '%s%s' of each string in array)", inString, skipChars, skipThisString, skipChars ? "_" : "");
#endif

			for (i32 i=0; i<arraySize; i++)
			{
#if DO_PARSE_BITFIELD_STRING_LOGS
				DrxLog("AutoEnum_GetEnumValFromString: Searching... Enum val %d = '%s'", i, (inArray[i] + skipChars));
#endif
				if (0 == stricmp((inArray[i] + skipChars), inString))
				{
#if DO_PARSE_BITFIELD_STRING_LOGS
					DrxLog("AutoEnum_GetEnumValFromString: Flag '%s' found in enum list as value %d", inString, i);
#endif
					if (outVal)
						(*outVal) = i;
					done = true;
					break;
				}
			}
			DRX_ASSERT_MESSAGE(done, string().Format("No flag called '%s' in enum list", inString));
	}

	return done;
}

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
string AutoEnum_GetStringFromBitfield(TBitfield bitfield, tukk* inArray, i32 arraySize)
{
	string output;
	TBitfield checkThis = 1;

	assert (arraySize > 0);

	char skipThisString[32];
	size_t skipChars = drx_copyStringUntilFindChar(skipThisString, inArray[0], sizeof(skipThisString), '_');

	for (i32 i = 0; i < arraySize; ++ i, checkThis <<= 1)
	{
		if (bitfield & checkThis)
		{
			if (! output.empty())
			{
				output.append("|");
			}
			output.append(inArray[i] + skipChars);
		}
	}

	return string().Format("%s%s", output.empty() ? "none" : output.c_str(), (bitfield >= checkThis) ? ", invalid bits found!" : "");
}
#endif
