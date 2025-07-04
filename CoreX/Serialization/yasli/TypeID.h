/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#if !YALSI_NO_RTTI
#include <typeinfo>
#endif
#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include <drx3D/CoreX/Serialization/yasli/Assert.h>
#include <string.h>


namespace yasli{

#if !YASLI_NO_RTTI
#ifndef _MSC_VER
using std::type_info;
#endif
#endif

class Archive;
struct TypeInfo;

class TypeID{
	friend class TypesFactory;
public:
	TypeID()
	: typeInfo_(0)
	, module_(0)
	{}

	TypeID(const TypeID& original)
	: typeInfo_(original.typeInfo_)
	, module_(original.module_)
#if !YASLI_NO_RTTI
	, name_(original.name_)
#endif
	{
	}

#if !YASLI_NO_RTTI
	explicit TypeID(const type_info& typeInfo)
	: typeInfo_(&typeInfo)
	{
	}
#endif

	operator bool() const{
		return *this != TypeID();
	}

	template<class T>
	static TypeID get();
	std::size_t sizeOf() const;
	tukk name() const;

	bool operator==(const TypeID& rhs) const;
	bool operator!=(const TypeID& rhs) const;
	bool operator<(const TypeID& rhs) const;

#if !YASLI_NO_RTTI
	typedef const type_info TypeInfo;
#endif
	const TypeInfo* typeInfo() const{ return typeInfo_; }
private:
	TypeInfo* typeInfo_;
#if YASLI_NO_RTTI 
	friend class bTypeInfo;
#else
	string name_;
#endif
	uk module_;
	friend class TypeDescription;
	friend struct TypeInfo;
};

#if YASLI_NO_RTTI
struct TypeInfo
{
	TypeID id;
	size_t size;
	char name[256];

	// We are trying to minimize type names here. Stripping namespaces,
	// whitespaces and S/C/E/I prefixes. Why namespaces? Type names are usually
	// used in two contexts: for unique name within factory context, where
	// collision is unlikely, or for filtering in PropertyTree where concise
	// name is much more useful.
	static void cleanTypeName(tuk& d, tukk dend, tukk & s, tukk send)
	{
		if(strncmp(s, "class ", 6) == 0)
			s += 6;
		else if(strncmp(s, "struct ", 7) == 0)
			s += 7;

		while(*s == ' ' && s != send)
			++s;

		// strip C/S/I/E prefixes
		if ((*s == 'C' || *s == 'S' || *s == 'I' || *s == 'E') && s[1] >= 'A' && s[1] <= 'Z')
			++s;

		if (s >= send)
			return;

		tuk startd = d;
		while (d != dend && s != send) {
			while(*s == ' ' && s != send)
				++s;
			if (s == send)
				break;
			if (*s == ':' && s[1] == ':') {
				// strip namespaces
				s += 2;
				d = startd;

				if ((*s == 'C' || *s == 'S' || *s == 'I' || *s == 'E') && s[1] >= 'A' && s[1] <= 'Z')
					++s;
			}
			if (s >= send)
				break;

			const bool nest = *s == '<';
			const bool list = *s == ',';
			if (nest || list) {
				*d = *s;
				++d;
				++s;
				cleanTypeName(d, dend, s, send);
				if(list)
					continue; // don't copy trailing '>' character unless type name is nested
			}
			else if (*s == '>') {
				return;
			}
			*d = *s;
			++s;
			++d;
		}
	}

	template<size_t nameLen>
	static void extractTypeName(char (&name)[nameLen], tukk funcName)
	{
#ifdef __clang__
        tukk s;
        tukk send;
        tukk lastSquareBracket = strrchr(funcName, ']');
        tukk lastRoundBracket = strrchr(funcName, ')');
        if (lastSquareBracket && lastRoundBracket && lastSquareBracket > lastRoundBracket)
        {
            // static yasli::TypeID yasli::TypeID::get() [T = ActualTypeName]
            s = strstr(funcName, "[T = ");
            if (s)
                s += 5;
            send = strrchr(funcName, ']');
        }
        else
        {
            // old versions of Xcode
            // static yasli::funcNameHelper(ActualTypeName *)
            s = strstr(funcName, "(");
            if (s)
                s += 1;
            send = strstr(funcName, "*)");
            if (send > s && *(send-1) == ' ')
                --send;
        }
#elif __GNUC__ >= 4 && __GNUC_MINOR__ >= 4
		// static yasli::TypeID yasli::TypeID::get() [with T = ActualTypeName]
		tukk s = strstr(funcName, "[with T = ");
		if (s)
			s += 10;
		tukk send = strrchr(funcName, ']');
#else
		// static yasli::TypeID yasli::TypeID::get<ActualTypeName>()
		tukk s = strchr(funcName, '<');
		tukk send = strrchr(funcName, '>');
		YASLI_ASSERT(s != 0  && send != 0);
		if (s != send)
			++s;
#endif
		YASLI_ASSERT(s != 0  && send != 0);

		tuk d = name;
		tukk dend = name + sizeof(name) - 1;
		cleanTypeName(d, dend, s, send);
		*d = '\0';

		// This assertion is not critical, but may result in collision as
		// stripped name wil be used, e.g. for lookup in factory.
		YASLI_ASSERT(s == send && "Type name does not fit into the buffer");
	}

	TypeInfo(size_t size, tukk templatedFunctionName)
	: size(size)
	{
		extractTypeName(name, templatedFunctionName);
		id.typeInfo_ = this;
		static i32 moduleSpecificSymbol;
		id.module_ = &moduleSpecificSymbol;
	}

	bool operator==(const TypeInfo& rhs) const{
		return size == rhs.size && strcmp(name, rhs.name) == 0;
	}

	bool operator<(const TypeInfo& rhs) const{
		if (size == rhs.size)
			return strcmp(name, rhs.name) < 0;
		else
			return size < rhs.size;
	}
};
#endif

template<class T>
TypeID TypeID::get()
{
#if YASLI_NO_RTTI
# ifdef _MSC_VER
	static TypeInfo typeInfo(sizeof(T), __FUNCSIG__);
# else
	static TypeInfo typeInfo(sizeof(T), __PRETTY_FUNCTION__);
# endif
	return typeInfo.id;
#else
	static TypeID result(typeid(T));
	return result;
#endif
}

inline bool TypeID::operator==(const TypeID& rhs) const{
#if YASLI_NO_RTTI
	if (typeInfo_ == rhs.typeInfo_)
		return true;
	else if (!typeInfo_ || !rhs.typeInfo_)
		return false;
	else if (module_ == rhs.module_)
		return false;
	else
		return *typeInfo_ == *rhs.typeInfo_;
#else
	if(typeInfo_ && rhs.typeInfo_)
		return typeInfo_ == rhs.typeInfo_;
	else
	{
		tukk name1 = name();
		tukk name2 = rhs.name();
		return strcmp(name1, name2) == 0;
	}
#endif
}

inline bool TypeID::operator!=(const TypeID& rhs) const{
	return !operator==(rhs);
}

inline bool TypeID::operator<(const TypeID& rhs) const{
#if YASLI_NO_RTTI
	if (!typeInfo_)
		return rhs.typeInfo_ != 0;
	else if (!rhs.typeInfo_)
		return false;
	else
		return *typeInfo_ < *rhs.typeInfo_;
#else
	if(typeInfo_ && rhs.typeInfo_)
		return typeInfo_->before(*rhs.typeInfo_) > 0;
	else if(!typeInfo_)
		return rhs.typeInfo_!= 0;
	else if(!rhs.typeInfo_)
		return false;
	return false;
#endif
}

template<class T>
T* createDerivedClass(TypeID typeID);

}