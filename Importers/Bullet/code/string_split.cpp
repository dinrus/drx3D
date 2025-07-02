#include <assert.h>
//#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../string_split.h"

///todo: remove stl dependency

namespace bullet_utils
{
void split(AlignedObjectArray<STxt> &pieces, const STxt &vector_str, const STxt &separator)
{
	char **strArray = str_split(vector_str.c_str(), separator.c_str());
	i32 numSubStr = str_array_len(strArray);
	for (i32 i = 0; i < numSubStr; i++)
		pieces.push_back(STxt(strArray[i]));
	str_array_free(strArray);
}

};  // namespace bullet_utils

/* Append an item to a dynamically allocated array of strings. On failure,
 return NULL, in which case the original array is intact. The item
 string is dynamically copied. If the array is NULL, allocate a new
 array. Otherwise, extend the array. Make sure the array is always
 NULL-terminated. Input string might not be '\0'-terminated. */
char **str_array_append(char **array, size_t nitems, tukk item,
						size_t itemlen)
{
	/* Make a dynamic copy of the item. */
	char *copy;
	if (item == NULL)
		copy = NULL;
	else
	{
		copy = (char *)malloc(itemlen + 1);
		if (copy == NULL)
			return NULL;
		memcpy(copy, item, itemlen);
		copy[itemlen] = '\0';
	}

	/* Extend array with one element. Except extend it by two elements,
	 in case it did not yet exist. This might mean it is a teeny bit
	 too big, but we don't care. */
	array = (char **)realloc(array, (nitems + 2) * sizeof(array[0]));
	if (array == NULL)
	{
		free(copy);
		return NULL;
	}

	/* Add copy of item to array, and return it. */
	array[nitems] = copy;
	array[nitems + 1] = NULL;
	return array;
}

/* Free a dynamic array of dynamic strings. */
void str_array_free(char **array)
{
	if (array == NULL)
		return;
	for (size_t i = 0; array[i] != NULL; ++i)
		free(array[i]);
	free(array);
}

/* Split a string into substrings. Return dynamic array of dynamically
 allocated substrings, or NULL if there was an error. Caller is
 expected to free the memory, for example with str_array_free. */
char **str_split(tukk input, tukk sep)
{
	size_t nitems = 0;
	char **array = NULL;
	tukk start = input;
	tukk next = strstr(start, sep);
	size_t seplen = strlen(sep);
	tukk item;
	size_t itemlen;

	for (;;)
	{
		next = strstr(start, sep);
		if (next == NULL)
		{
			/* Add the remaining string (or empty string, if input ends with
			 separator. */
			char **newstr = str_array_append(array, nitems, start, strlen(start));
			if (newstr == NULL)
			{
				str_array_free(array);
				return NULL;
			}
			array = newstr;
			++nitems;
			break;
		}
		else if (next == input)
		{
			/* Input starts with separator. */
			item = "";
			itemlen = 0;
		}
		else
		{
			item = start;
			itemlen = next - item;
		}
		char **newstr = str_array_append(array, nitems, item, itemlen);
		if (newstr == NULL)
		{
			str_array_free(array);
			return NULL;
		}
		array = newstr;
		++nitems;
		start = next + seplen;
	}

	if (nitems == 0)
	{
		/* Input does not contain separator at all. */
		assert(array == NULL);
		array = str_array_append(array, nitems, input, strlen(input));
	}

	return array;
}

/* Return length of a NULL-delimited array of strings. */
size_t str_array_len(char **array)
{
	size_t len;

	for (len = 0; array[len] != NULL; ++len)
		continue;
	return len;
}

#ifdef UNIT_TEST_STRING

#define MAX_OUTPUT 20

i32 main(void)
{
	struct
	{
		tukk input;
		tukk sep;
		char *output[MAX_OUTPUT];
	} tab[] = {
		/* Input is empty string. Output should be a list with an empty
		 string. */
		{
			"",
			"and",
			{
				"",
				NULL,
			},
		},
		/* Input is exactly the separator. Output should be two empty
		 strings. */
		{
			"and",
			"and",
			{
				"",
				"",
				NULL,
			},
		},
		/* Input is non-empty, but does not have separator. Output should
		 be the same string. */
		{
			"foo",
			"and",
			{
				"foo",
				NULL,
			},
		},
		/* Input is non-empty, and does have separator. */
		{
			"foo bar 1 and foo bar 2",
			" and ",
			{
				"foo bar 1",
				"foo bar 2",
				NULL,
			},
		},
	};
	i32k tab_len = sizeof(tab) / sizeof(tab[0]);
	bool errors;

	errors = false;

	for (i32 i = 0; i < tab_len; ++i)
	{
		printf("test %d\n", i);

		char **output = str_split(tab[i].input, tab[i].sep);
		if (output == NULL)
		{
			fprintf(stderr, "output is NULL\n");
			errors = true;
			break;
		}
		size_t num_output = str_array_len(output);
		printf("num_output %lu\n", (u64)num_output);

		size_t num_correct = str_array_len(tab[i].output);
		if (num_output != num_correct)
		{
			fprintf(stderr, "wrong number of outputs (%lu, not %lu)\n",
					(u64)num_output, (u64)num_correct);
			errors = true;
		}
		else
		{
			for (size_t j = 0; j < num_output; ++j)
			{
				if (strcmp(tab[i].output[j], output[j]) != 0)
				{
					fprintf(stderr, "output[%lu] is '%s' not '%s'\n",
							(u64)j, output[j], tab[i].output[j]);
					errors = true;
					break;
				}
			}
		}

		str_array_free(output);
		printf("\n");
	}

	if (errors)
		return EXIT_FAILURE;
	return 0;
}

#endif  //
