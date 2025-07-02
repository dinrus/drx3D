/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

//#include "stdafx.h"
#include "ConstStringList.h"
#include <algorithm>
#include <drx3D/CoreX/Serialization/yasli/STL.h>
#include <drx3D/CoreX/Serialization/yasli/Archive.h>

ConstStringList globalConstStringList;

tukk ConstStringList::findOrAdd(tukk string)
{
	// TODO: try sorted vector of tukk 
	Strings::iterator it = std::find(strings_.begin(), strings_.end(), string);
	if(it == strings_.end()){
		strings_.push_back(string);
		return strings_.back().c_str();
	}
	else{
		return it->c_str();
	}
}


ConstStringWrapper::ConstStringWrapper(ConstStringList* list, tukk & string)
: list_(list ? list : &globalConstStringList)
, string_(string)
{
	YASLI_ASSERT(string_);
}


bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, ConstStringWrapper& value, tukk name, tukk label)
{
	if(ar.isOutput()){
        YASLI_ASSERT(value.string_);
        string out = value.string_;
		return ar(out, name, label);
	}
	else{
		yasli::string in;
		bool result = ar(in, name, label);

        value.string_ = value.list_->findOrAdd(in.c_str());
		return result;
	}
}

