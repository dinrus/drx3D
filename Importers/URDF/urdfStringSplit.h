
#ifndef STRING_SPLIT_H
#define STRING_SPLIT_H

#ifdef __cplusplus

#include <cstring>
#include <string>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

void urdfStringSplit(AlignedObjectArray<STxt>& pieces, const STxt& vector_str, const AlignedObjectArray<STxt>& separators);

void urdfIsAnyOf(tukk seps, AlignedObjectArray<STxt>& strArray);
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	///The string split C code is by Lars Wirzenius
	///See http://stackoverflow.com/questions/2531605/how-to-split-a-string-with-a-delimiter-larger-than-one-single-char

	/* Split a string into substrings. Return dynamic array of dynamically
 allocated substrings, or NULL if there was an error. Caller is
 expected to free the memory, for example with str_array_free. */
	tuk* urdfStrSplit(tukk input, tukk sep);

	/* Free a dynamic array of dynamic strings. */
	void urdfStrArrayFree(tuk* array);

	/* Return length of a NULL-delimited array of strings. */
	size_t urdfStrArrayLen(tuk* array);

#ifdef __cplusplus
}
#endif

#endif  //STRING_SPLIT_H
