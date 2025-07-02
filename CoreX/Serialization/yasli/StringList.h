/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <string.h>
#include <drx3D/CoreX/Serialization/yasli/Assert.h>
#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include <drx3D/CoreX/Serialization/yasli/TypeID.h>

namespace yasli{

class Archive;
class StringListStatic : public StringListStaticBase{
public:
	static StringListStatic& emptyValue()
	{
		static StringListStatic e;
		return e;
	}

	enum { npos = -1 };
	i32 find(tukk value) const{
		i32 numItems = i32(size());
		for(i32 i = 0; i < numItems; ++i){
			if(strcmp((*this)[i], value) == 0)
				return i;
		}
		return npos;
	}
};

class StringListStaticValue{
public:
	StringListStaticValue()
	: stringList_(&StringListStatic::emptyValue())
	, index_(-1)
	{
        handle_ = this;
	}

	StringListStaticValue(const StringListStaticValue& original)
	: stringList_(original.stringList_)
	, index_(original.index_)
	{
	}
    StringListStaticValue(const StringListStatic& stringList, i32 value)
    : stringList_(&stringList)
    , index_(value)
    {
        handle_ = this;
    }
    StringListStaticValue(const StringListStatic& stringList, i32 value, ukk handle, const yasli::TypeID& type)
    : stringList_(&stringList)
    , index_(value)
    , handle_(handle)
    , type_(type)
    {
    }
    StringListStaticValue(const StringListStatic& stringList, tukk value, ukk handle, const yasli::TypeID& type)
    : stringList_(&stringList)
    , index_(stringList.find(value))
    , handle_(handle)
    , type_(type)
    {
    }
    StringListStaticValue& operator=(tukk value){
        index_ = stringList_->find(value);
		return *this;
    }
    StringListStaticValue& operator=(i32 value){
		YASLI_ASSERT(value >= 0 && StringListStatic::size_type(value) < stringList_->size());
        index_ = value;
		return *this;
    }
    StringListStaticValue& operator=(const StringListStaticValue& rhs){
        stringList_ = rhs.stringList_;
		index_ = rhs.index_;
        return *this;
    }
    tukk c_str() const{
        if(index_ >= 0 && StringListStatic::size_type(index_) < stringList_->size())
			return (*stringList_)[index_];
		else
			return "";
    }
    i32 index() const{ return index_; }
    ukk handle() const{ return handle_; }
    yasli::TypeID type() const { return type_; }
    const StringListStatic& stringList() const{ return *stringList_; }
    template<class Archive>
    void YASLI_SERIALIZE_METHOD(Archive& ar) {
        ar(index_, "index");
    }
protected:
    const StringListStatic* stringList_;
    i32 index_;
    ukk handle_;
    yasli::TypeID type_;
};

class StringList: public StringListBase{
public:
    StringList() {}
    
    StringList& operator=(const StringList& rhs)
	{
		// As StringList crosses dll boundaries it is important to copy strings
		// rather than reference count them to be sure that stored DrxString uses
		// proper allocator.
		resize(rhs.size());
		for (size_t i = 0; i < size_t(size()); ++i)
			(*this)[i] = rhs[i].c_str();
		return *this;
	}
    StringList(const StringListStatic& rhs){
        i32k size = i32(rhs.size());
        resize(size);
        for(i32 i = 0; i < i32(size); ++i)
            (*this)[i] = rhs[i];
    }
    enum { npos = -1 };
    i32 find(tukk value) const{
		i32k numItems = i32(size());
		for(i32 i = 0; i < numItems; ++i){
            if((*this)[i] == value)
                return i;
        }
        return npos;
    }
    
    StringList(const StringList& rhs){ *this  = rhs;   }
};

class StringListValue{
public:
	explicit StringListValue(const StringListStaticValue &value)
	{
		stringList_.resize(value.stringList().size());
		for (size_t i = 0; i < size_t(stringList_.size()); ++i)
			stringList_[i] = value.stringList()[i];
		index_ = value.index();
	}
	StringListValue(const StringListValue &value)
	{
		stringList_ = value.stringList_;
		index_ = value.index_;
	}
	StringListValue()
	: index_(StringList::npos)
	{
		handle_ = this;
	}
	StringListValue(const StringList& stringList, i32 value)
	: stringList_(stringList)
	, index_(value)
	{
		handle_ = this;
	}
	StringListValue(const StringList& stringList, i32 value, ukk handle, const yasli::TypeID& typeId)
	: stringList_(stringList)
	, index_(value)
	, handle_(handle)
	, type_(typeId)
	{
	}
	StringListValue(const StringList& stringList, tukk value)
	: stringList_(stringList)
	, index_(stringList.find(value))
	{
		handle_ = this;
		YASLI_ASSERT(index_ != StringList::npos);
	}
	StringListValue(const StringList& stringList, tukk value, ukk handle, const yasli::TypeID& typeId)
	: stringList_(stringList)
	, index_(stringList.find(value))
	, handle_(handle)
	, type_(typeId)
	{
		YASLI_ASSERT(index_ != StringList::npos);
	}
	StringListValue(const StringListStatic& stringList, tukk value)
	: stringList_(stringList)
	, index_(stringList.find(value))
	{
		handle_ = this;
		YASLI_ASSERT(index_ != StringList::npos);
	}
	StringListValue& operator=(tukk value){
		index_ = stringList_.find(value);
		return *this;
	}
	StringListValue& operator=(i32 value){
		YASLI_ASSERT(value >= 0 && size_t(value) < size_t(stringList_.size()));
		index_ = value;
		return *this;
	}
	tukk c_str() const{
		if(index_ >= 0 && size_t(index_) < size_t(stringList_.size()))
			return stringList_[index_].c_str();
		else
			return "";
	}
	i32 index() const{ return index_; }
	ukk handle() const { return handle_; }
	yasli::TypeID type() const { return type_; }
	const StringList& stringList() const{ return stringList_; }
	template<class Archive>
	void Serialize(Archive& ar) {
		ar(index_, "index");
		ar(stringList_, "stringList");
	}
protected:
	StringList stringList_;
	i32 index_;
	ukk handle_;
	yasli::TypeID type_;
};

class Archive;

void splitStringList(StringList* result, tukk str, char sep);
void joinStringList(string* result, const StringList& stringList, char sep);
void joinStringList(string* result, const StringListStatic& stringList, char sep);

bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, StringList& value, tukk name, tukk label);
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, StringListValue& value, tukk name, tukk label);
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, StringListStaticValue& value, tukk name, tukk label);

}

#if YASLI_INLINE_IMPLEMENTATION
#include "StringListImpl.h"
#endif
