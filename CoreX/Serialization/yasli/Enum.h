/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 *
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>
#include <map>

#include <drx3D/CoreX/Serialization/yasli/StringList.h>
#include <drx3D/CoreX/Serialization/yasli/TypeID.h>


namespace yasli{

class Archive;

struct LessStrCmp : std::binary_function<tukk , tukk , bool>
{
	bool operator()(tukk l, tukk r) const{
		return strcmp(l, r) < 0;
	}
};

class EnumDescription{
public:
	EnumDescription(TypeID typeId)
		: type_(typeId)
	{}

	i32 value(tukk name) const;
	i32 valueByIndex(i32 index) const;
	i32 valueByLabel(tukk label) const;
	tukk name(i32 value) const;
	tukk nameByIndex(i32 index) const;
	tukk label(i32 value) const;
	tukk labelByIndex(i32 index) const;
	tukk indexByName(tukk name) const;
	i32 indexByValue(i32 value) const;

	bool serialize(Archive& ar, i32& value, tukk name, tukk label) const;
	bool serializeBitVector(Archive& ar, i32& value, tukk name, tukk label) const;

	void add(i32 value, tukk name, tukk label = ""); // TODO
	i32 count() const{ return (i32)values_.size(); }
	const StringListStatic& names() const{ return names_; }
	const StringListStatic& labels() const{ return labels_; }
	StringListStatic nameCombination(i32 bitVector) const;
	StringListStatic labelCombination(i32 bitVector) const;
	bool registered() const { return !names_.empty(); }
	TypeID type() const{ return type_; }
private:
	StringListStatic names_;
	StringListStatic labels_;

	typedef std::map<tukk , i32, LessStrCmp> NameToValue;
	NameToValue nameToValue_;
	typedef std::map<tukk , i32, LessStrCmp> LabelToValue;
	LabelToValue labelToValue_;
	typedef std::map<i32, i32> ValueToIndex;
	ValueToIndex valueToIndex_;
	typedef std::map<i32, tukk > ValueToName;
	ValueToName valueToName_;
	typedef std::map<i32, tukk > ValueToLabel;
	ValueToName valueToLabel_;
	std::vector<i32> values_;
	TypeID type_;
};

template<class Enum>
class EnumDescriptionImpl : public EnumDescription{
public:
	EnumDescriptionImpl(TypeID typeId)
		: EnumDescription(typeId)
	{}

	static EnumDescription& the(){
		static EnumDescriptionImpl description(TypeID::get<Enum>());
		return description;
	}
};

template<class Enum>
EnumDescription& getEnumDescription(){
	return EnumDescriptionImpl<Enum>::the();
}

inline bool serializeEnum(const EnumDescription& desc, Archive& ar, i32& value, tukk name, tukk label){
	return desc.serialize(ar, value, name, label);
}

}

#define YASLI_ENUM_BEGIN(Type, label)                                                \
	static bool registerEnum_##Type();                                               \
	static bool Type##_enum_registered = registerEnum_##Type();                      \
	static bool registerEnum_##Type(){                                               \
		yasli::EnumDescription& description = yasli::EnumDescriptionImpl<Type>::the();

#define YASLI_ENUM_BEGIN_NESTED(Class, Enum, label)                                  \
	static bool registerEnum_##Class##_##Enum();                                     \
	static bool Class##_##Enum##_enum_registered = registerEnum_##Class##_##Enum();  \
	static bool registerEnum_##Class##_##Enum(){                                     \
		yasli::EnumDescription& description = yasli::EnumDescriptionImpl<Class::Enum>::the();

#define YASLI_ENUM_BEGIN_NESTED2(Class, Class1, Enum, label)                                        \
	static bool registerEnum_##Class##Class1##_##Enum();                                            \
	static bool Class##Class1##_##Enum##_enum_registered = registerEnum_##Class##Class1##_##Enum(); \
	static bool registerEnum_##Class##Class1##_##Enum(){                                            \
	yasli::EnumDescription& description = yasli::EnumDescriptionImpl<Class::Class1::Enum>::the();


#define YASLI_ENUM_VALUE(value, label)                                              \
		description.add(i32(value), #value, label);

#define YASLI_ENUM(value, name, label)                                              \
		description.add(i32(value), name, label);

#define YASLI_ENUM_VALUE_NESTED(Class, value, label)                                       \
	description.add(i32(Class::value), #value, label);

#define YASLI_ENUM_VALUE_NESTED2(Class, Class1, value, label)                                       \
	description.add(i32(Class::Class1::value), #value, label);


#define YASLI_ENUM_END()													        \
		return true;                                                            \
	};

// vim:ts=4 sw=4:
//
#if YASLI_INLINE_IMPLEMENTATION
#include "EnumImpl.h"
#endif
