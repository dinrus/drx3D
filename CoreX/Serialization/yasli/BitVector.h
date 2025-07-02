/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

namespace yasli{

class Archive;
template<class Enum>
class BitVector
{
public:
    BitVector(i32 value = 0) : value_(value) {}

	operator i32&() { return value_; }
    operator i32() const { return value_; }

    BitVector& operator|= (Enum value) { value_ |= value; return *this; }
    BitVector& operator|= (i32 value) { value_ |= value; return *this; }
    BitVector& operator&= (i32 value) { value_ &= value; return *this; }

    void YASLI_SERIALIZE_METHOD(Archive& ar);
private:
    i32 value_;
};

template<class Enum>
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, BitVector<Enum>& value, tukk name, tukk label);

}

#include "BitVectorImpl.h"
