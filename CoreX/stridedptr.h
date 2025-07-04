// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <type_traits>
#include <drx3D/CoreX/DrxEndian.h>

template<class dtype>
class strided_pointer
{
public:
	strided_pointer()
		: data(0)
		, iStride(sizeof(dtype))
	{
	}

	strided_pointer(dtype* pdata, i32 stride = sizeof(dtype))
		: data(pdata)
		, iStride(stride)
	{
	}

	template<typename dtype1>
	strided_pointer(dtype1* pdata)
	{
		set(pdata, sizeof(dtype1));
	}

	template<typename dtype1>
	strided_pointer(const strided_pointer<dtype1>& src)
	{
		set(src.data, src.iStride);
	}

	template<typename dtype1>
	ILINE strided_pointer& operator=(const strided_pointer<dtype1>& src)
	{
		set(src.data, src.iStride);
		return *this;
	}

	ILINE dtype&                 operator[](i32 idx)       { return *(dtype*)((tuk)data + idx * iStride); }
	ILINE const dtype&           operator[](i32 idx) const { return *(const dtype*)((tukk)data + idx * iStride); }
	ILINE strided_pointer<dtype> operator+(i32 idx) const  { return strided_pointer<dtype>((dtype*)((tuk)data + idx * iStride), iStride); }
	ILINE strided_pointer<dtype> operator-(i32 idx) const  { return strided_pointer<dtype>((dtype*)((tuk)data - idx * iStride), iStride); }

	ILINE                        operator bool() const
	{
		return data != 0;
	}

private:
	template<typename dtype1>
	ILINE void set(dtype1* pdata, i32 stride)
	{
		static_assert(std::is_const<dtype>::value || !std::is_const<dtype1>::value, "Invalid const modifiers!");
		// note: we allow xint32 -> xint16 converting
		static_assert(
		  (std::is_same<typename std::remove_const<dtype1>::type, typename std::remove_const<dtype>::type>::value ||
		   ((std::is_same<typename std::remove_const<dtype1>::type, sint32>::value ||
		     std::is_same<typename std::remove_const<dtype1>::type, u32>::value ||
		     std::is_same<typename std::remove_const<dtype1>::type, sint16>::value ||
		     std::is_same<typename std::remove_const<dtype1>::type, u16>::value) &&
		    (std::is_same<typename std::remove_const<dtype>::type, sint16>::value ||
		     std::is_same<typename std::remove_const<dtype>::type, u16>::value))), "Invalid type!");
		data = (dtype*)pdata + (sizeof(dtype1) / sizeof(dtype) - 1) * (i32)eLittleEndian;
		iStride = stride;
	}

private:
	//! Prevents assignment of a structure member's address by mistake:
	//! stridedPtrObject = &myStruct.member;
	//! Use explicit constructor instead:
	//! stridedPtrObject = strided_pointer<baseType>(&myStruct.member, sizeof(myStruct));
	//! Keep it private and non-implemented.
	strided_pointer& operator=(dtype* pdata);

	//! Prevents using the address directly by mistake:
	//! memcpy(destination, stridedPtrObject, sizeof(baseType) * n);
	//! Use address of the first element instead:
	//! memcpy(destination, &stridedPtrObject[0], stridedPtrObject.iStride * n);
	//! Keep it private and non-implemented.
	operator uk () const;

	//! Prevents direct dereferencing to avoid confusing it with "normal" pointers:
	//! val = *stridedPtrObject;
	//! Use operator [] instead:
	//! val = stridedPtrObject[0];
	//! Keep it private and non-implemented.
	dtype& operator*() const;

public:
	dtype* data;
	i32  iStride;
};
