// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/PathExpansion.h>

#include <drx3D/CoreX/Math/Random.h>

// Expand patterns into paths. An example of a pattern before expansion:
// animations/facial/idle_{0,1,2,3}.fsq
// {} is used to specify options for parts of the string.
// output must point to buffer that is at least as large as the pattern.
void PathExpansion::SelectRandomPathExpansion(tukk pattern, tuk output)
{
	// Loop through all the characters.
	i32 output_pos = 0;
	for (i32 pos = 0; pattern[pos]; )
	{
		// Check whether this character is the start of an options list.
		if (pattern[pos] == '{')
		{
			struct Option
			{
				i32 start;
				i32 length;
			};
			static i32k MAX_OPTIONS = 100;
			Option options[MAX_OPTIONS];
			i32 option_count = 0;

			// Skip the '{'.
			++pos;

			// Create the first option.
			options[option_count].start = pos;
			options[option_count].length = 0;
			++option_count;

			// Loop until we find the matching }.
			while (pattern[pos] && pattern[pos] != '}')
			{
				if (pattern[pos] == ',')
				{
					// Skip the ','.
					++pos;

					// Add a new option to the list.
					if (option_count < MAX_OPTIONS)
					{
						options[option_count].start = pos;
						options[option_count].length = 0;
					}
					++option_count;
				}
				else
				{
					// Extend the length of the current option.
					if (option_count <= MAX_OPTIONS)
						++options[option_count - 1].length;

					++pos;
				}
			}

			// Skip the '}'.
			++pos;

			// Pick a random option from the list.
			i32 option_index = drx_random(0, std::min(MAX_OPTIONS, option_count) - 1);
			for (i32 i = 0; i < options[option_index].length; ++i)
				output[output_pos++] = pattern[options[option_index].start + i];
		}
		else
		{
			// The character is a normal one - simply copy it to the output.
			output[output_pos++] = pattern[pos++];
		}
	}

	// Write the null terminator.
	output[output_pos] = 0;
}

namespace PathExpansion
{
struct Option
{
	i32 start;
	i32 length;
};
struct Segment
{
	static i32k MAX_OPTIONS = 30;
	Option           options[MAX_OPTIONS];
	i32              optionCount;
};
}

void PathExpansion::EnumeratePathExpansions(tukk pattern, void (* enumCallback)(uk userData, tukk expansion), uk userData) PREFAST_SUPPRESS_WARNING(6262)
{
	// Allocate some temporary working buffers.
	i32 patternLength = strlen(pattern);
	std::vector<char> buffer(patternLength + 1);
	tuk expansion = &buffer[0];

	// Divide the pattern into a list of segments, which contain a list of option text (the case where there is only one
	// option is used to represent simple text copying).
	static i32k MAX_SEGMENTS = 100;
	Segment segments[MAX_SEGMENTS] = { Segment() };
	i32 segmentCount = 0;

	// Loop through all the characters.
	for (i32 pos = 0; pattern[pos]; )
	{
		// Check whether this character is the start of an options list.
		if (pattern[pos] == '{')
		{
			Segment dummySegment;
			dummySegment.optionCount = 0;
			Segment& segment = (segmentCount < MAX_SEGMENTS ? segments[segmentCount++] : dummySegment);
			segment.optionCount = 0;

			// Skip the '{'.
			++pos;

			// Create the first option.
			segment.options[segment.optionCount].start = pos;
			segment.options[segment.optionCount].length = 0;
			++segment.optionCount;

			// Loop until we find the matching }.
			while (pattern[pos] && pattern[pos] != '}')
			{
				if (pattern[pos] == ',')
				{
					// Skip the ','.
					++pos;

					// Add a new option to the list.
					if (segment.optionCount < Segment::MAX_OPTIONS)
					{
						segment.options[segment.optionCount].start = pos;
						segment.options[segment.optionCount].length = 0;
					}
					++segment.optionCount;
				}
				else
				{
					// Extend the length of the current option.
					if (segment.optionCount <= Segment::MAX_OPTIONS)
						++segment.options[segment.optionCount - 1].length;

					++pos;
				}
			}

			// Skip the '}'.
			++pos;
		}
		else
		{
			// Create a segment with only one option string.
			Segment dummySegment;
			dummySegment.optionCount = 0;
			Segment& segment = (segmentCount < MAX_SEGMENTS ? segments[segmentCount++] : dummySegment);
			Option& option = segment.options[0];
			segment.optionCount = 1;
			option.start = pos;
			option.length = 0;
			while (pattern[pos] && pattern[pos] != '{')
				++pos, ++option.length;
		}
	}

	// Calculate the number of combinations.
	i32 combinationCount = 1;
	for (i32 segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
	{
		PREFAST_SUPPRESS_WARNING(6001);
		combinationCount *= segments[segmentIndex].optionCount;
	}

	// Loop through each combination.
	for (i32 combinationIndex = 0; combinationIndex < combinationCount; ++combinationIndex)
	{
		i32 outputPos = 0;
		i32 combinationAccumulator = combinationIndex;

		// Loop through all the segments.
		for (i32 segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
		{
			// Select the appropriate option from the list.
			const Segment& segment = segments[segmentIndex];
			i32 optionIndex = combinationAccumulator % segment.optionCount;
			combinationAccumulator /= segment.optionCount;
			const Option& option = segment.options[optionIndex];
			for (i32 i = 0; i < option.length; ++i)
				expansion[outputPos++] = pattern[option.start + i];
		}

		// Write the null terminator.
		expansion[outputPos] = 0;

		// Call the callback.
		if (enumCallback)
			enumCallback(userData, expansion);
	}
}
