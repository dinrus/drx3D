/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <cstddef>

namespace yasli{

class MemoryReader{
public:

    YASLI_INLINE MemoryReader();
	YASLI_INLINE explicit MemoryReader(tukk fileName);
	YASLI_INLINE MemoryReader(ukk memory, size_t size, bool ownAndFree = false);
	YASLI_INLINE ~MemoryReader();

	YASLI_INLINE void setPosition(tukk position);
    tukk position(){ return position_; }

    template<class T>
    void read(T& value){
        read(reinterpret_cast<uk>(&value), sizoef(value));
    }
	YASLI_INLINE void read(uk data, size_t size);
	YASLI_INLINE bool checkedSkip(size_t size);
	YASLI_INLINE bool checkedRead(uk data, size_t size);
    template<class T>
    bool checkedRead(T& t){
        return checkedRead((uk )&t, sizeof(t));
    }

    tukk buffer() const{ return memory_; }
    size_t size() const{ return size_; }

    tukk begin() const{ return memory_; }
    tukk end() const{ return memory_ + size_; }
private:
    size_t size_;
    tukk position_;
    tukk memory_;
    bool ownedMemory_;
};

}
// vim:ts=4 sw=4:


#if YASLI_INLINE_IMPLEMENTATION
#include "MemoryReaderImpl.h"
#endif