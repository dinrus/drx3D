/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <list>
#include <drx3D/CoreX/Serialization/yasli/Config.h>

class ConstStringWrapper;

namespace yasli { class Archive; }

bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, ConstStringWrapper &wrapper, tukk name, tukk label);

class ConstStringList{
public:
	tukk findOrAdd(tukk string);
protected:
	typedef std::list<yasli::string> Strings;
	Strings strings_;
};

class ConstStringWrapper{
public:
	ConstStringWrapper(ConstStringList* list, tukk & string);
protected:
	ConstStringList* list_;
	tukk & string_;
	friend bool ::YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, ConstStringWrapper &wrapper, tukk name, tukk label);
};

extern ConstStringList globalConstStringList;


