// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once 
#include <stdlib.h> // for malloc and free

#include <drx3D/CoreX/Serialization/yasli/Config.h>

namespace yasli
{

// Black box is used to store opaque data blobs in a format internal to
// specific Archive. For example it can be used to store sections of the JSON
// or binary archive.
//
// This is useful for the Editor to store portions of files with unfamiliar
// structure.
//
// We store deallocation function here so we can safely pass the blob
// across DLLs with different memory allocators.
struct BlackBox
{
	tukk format;
	uk data;
	size_t size;
	i32 xmlVersion;
	typedef uk (*Allocator)(size_t, ukk );
	Allocator allocator;

	BlackBox()
	: format("")
	, data(0)
	, size(0)
	, allocator(0)
	, xmlVersion(0)
	{
	}

	BlackBox(const BlackBox& rhs)
	: format("")
	, data(0)
	, size(0)
	, allocator(0)
	, xmlVersion(rhs.xmlVersion)
	{
		*this = rhs;
	}

	void set(tukk format, ukk data, size_t size, Allocator allocator = &defaultAllocator)
	{
		YASLI_ASSERT(data != this->data);
		if(data != this->data)
		{
			release();
			this->format = format;
			if (data && size) 
			{
				this->data = allocator(size, data);
				this->size = size;
				this->allocator = allocator;
			}
		}
	}

	template<typename Type>
	void set(tukk format, const Type& data)
	{
		this->set(format, &data, sizeof(Type), &templateAllocator<Type>);
	}

	void release()
	{
		if (data && allocator) {
			allocator(0, data);
		}
		format = 0;
		data = 0;
		size = 0;
		allocator = 0;
		xmlVersion = 0;
	}

	BlackBox& operator=(const BlackBox& rhs)
	{
		set(rhs.format, rhs.data, rhs.size, rhs.allocator);
		xmlVersion = rhs.xmlVersion;
		return *this;
	}

	~BlackBox()
	{
		release();
	}

	static uk defaultAllocator(size_t size, ukk ptr)
	{
		if (size) {
			return memcpy(malloc(size), ptr, size);
		}
		else {
			free(const_cast<uk>(ptr));
			return 0;
		}
	}

	template<typename Type>
	static uk templateAllocator(size_t size, ukk ptr)
	{
		if (size) {
			return new Type(*static_cast<const Type*>(ptr));
		}
		else {
			delete static_cast<const Type*>(ptr);
			return 0;
		}
	}
};


}
