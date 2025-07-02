#ifndef BOOST_REPLACEMENT_LEXICAL_CAST_H
#define BOOST_REPLACEMENT_LEXICAL_CAST_H

#include <stdlib.h>

template <typename T>
T urdfLexicalCast(tukk txt)
{
	double result = atof(txt);
	return result;
};

#endif
