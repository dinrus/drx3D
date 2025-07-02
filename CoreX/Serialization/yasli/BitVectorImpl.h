/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include <drx3D/CoreX/Serialization/yasli/BitVector.h>
#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include <drx3D/CoreX/Serialization/yasli/Enum.h>

namespace yasli{

struct BitVectorWrapper
{
	i32* valuePointer;
	i32 value;
	const EnumDescription* description;

	explicit BitVectorWrapper(i32* _value = 0, const EnumDescription* _description = 0)
    : valuePointer(_value)
    , description(_description)
    {
		if(valuePointer)
			value = *valuePointer;
    }
    BitVectorWrapper(const BitVectorWrapper& _rhs)
    : value(_rhs.value)
    , description(0)
	, valuePointer(0)
    {
    }

	~BitVectorWrapper()
	{
		if(valuePointer)
			*valuePointer = value;
	}
	BitVectorWrapper& operator=(const BitVectorWrapper& rhs){
		value = rhs.value;
		return *this;
	}


    void YASLI_SERIALIZE_METHOD(Archive& ar)
    {
		ar(value, "value", "Value");
    }
};

template<class Enum>
void BitVector<Enum>::YASLI_SERIALIZE_METHOD(Archive& ar)
{
    ar(value_, "value", "Value");
}

template<class Enum>
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, BitVector<Enum>& value, tukk name, tukk label)
{
    using namespace yasli;
    EnumDescription &desc = getEnumDescription<Enum>();
    if(ar.isEdit())
        return ar(BitVectorWrapper(&static_cast<i32&>(value), &desc), name, label);
    else
    {
        return desc.serializeBitVector(ar, static_cast<i32&>(value), name, label);
    }
}

}
