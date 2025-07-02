///The string split C code is by Lars Wirzenius
///See http://stackoverflow.com/questions/2531605/how-to-split-a-string-with-a-delimiter-larger-than-one-single-char

#ifndef STRING_SPLIT_H
#define STRING_SPLIT_H

#include <cstring>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#include <string>

namespace bullet_utils
{
void split(AlignedObjectArray<STxt>& pieces, const STxt& vector_str, const STxt& separator);
};

///The string split C code is by Lars Wirzenius
///See http://stackoverflow.com/questions/2531605/how-to-split-a-string-with-a-delimiter-larger-than-one-single-char

/* Split a string into substrings. Return dynamic array of dynamically
 allocated substrings, or NULL if there was an error. Caller is
 expected to free the memory, for example with str_array_free. */
tuk* str_split(tukk input, tukk sep);

/* Free a dynamic array of dynamic strings. */
void str_array_free(tuk* array);

/* Return length of a NULL-delimited array of strings. */
size_t str_array_len(tuk* array);

#endif  //STRING_SPLIT_H
