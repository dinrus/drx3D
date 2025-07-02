/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <string>
#include <string.h>

namespace yasli{

struct Token{
	Token(tukk _str = 0)
	: start(_str)
	, end(_str ? _str + strlen(_str) : 0)
	{
	}

	Token(tukk _str, size_t _len) : start(_str) , end(_str + _len) {}
	Token(tukk _start, tukk _end) : start(_start) , end(_end) {}

	void set(tukk _start, tukk _end) { start = _start; end = _end; }
	std::size_t length() const{ return end - start; }

	bool operator==(const Token& rhs) const{
		if(length() != rhs.length())
			return false;
		return memcmp(start, rhs.start, length()) == 0;
	}
	bool operator==(const STxt& rhs) const{
		if(length() != rhs.size())
			return false;
		return memcmp(start, rhs.c_str(), length()) == 0;
	}

	bool operator==(tukk text) const{
		if(strncmp(text, start, length()) == 0)
			return text[length()] == '\0';
		return false;
	}
	bool operator!=(tukk text) const{
		if(strncmp(text, start, length()) == 0)
			return text[length()] != '\0';
		return true;
	}
	bool operator==(char c) const{
		return length() == 1 && *start == c;
	}
	bool operator!=(char c) const{
		return length() != 1 || *start != c;
	}

	operator bool() const{ return start != end; }
	STxt str() const{ return STxt(start, end); }

	tukk start;
	tukk end;
};


}
