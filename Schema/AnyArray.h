// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Separate into two classes: CAnyArray and CAnyArrayPtr!!!
// #SchematycTODO : Implement comparison operators?
// #SchematycTODO : Do we need to worry about data alignment?
// #SchematycTODO : Implement move semantics?

#pragma once

#include <drx3D/CoreX/Serialization/DynArray.h>
#include <drx3D/CoreX/Serialization/Forward.h>

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/IString.h>

namespace sxema
{

class CAnyArray
{
public:

	inline CAnyArray(const CCommonTypeDesc& typeDesc)
		: m_typeDesc(typeDesc)
		, m_capacity(0)
		, m_pBegin(nullptr)
		, m_pEnd(nullptr)
	{}

	inline CAnyArray(const CAnyArray& rhs)
		: m_typeDesc(rhs.m_typeDesc)
		, m_capacity(0)
		, m_pBegin(nullptr)
		, m_pEnd(nullptr)
	{
		Reserve(rhs.m_capacity);
		CopyConstruct(rhs.m_pBegin, rhs.m_pEnd, m_pBegin);

		m_pEnd = m_pBegin + (rhs.m_pEnd - rhs.m_pBegin);
	}

	inline ~CAnyArray()
	{
		if (m_pBegin)
		{
			Destruct(m_pBegin, m_pEnd);
			DrxModuleFree(m_pBegin);
		}
	}

	inline u32 GetSize() const
	{
		return static_cast<u32>(m_pEnd - m_pBegin) / m_typeDesc.GetSize();
	}

	inline u32 GetCapacity() const
	{
		return m_capacity;
	}

	inline void Reserve(u32 capacity)
	{
		if (capacity > m_capacity)
		{
			u32k minCapacity = 16;
			u32k growthFactor = 2;
			capacity = max(capacity, max(minCapacity, capacity * growthFactor));

			u8* pBegin = static_cast<u8*>(DrxModuleRealloc(m_pBegin, capacity));
			if (pBegin != m_pBegin)
			{
				Move(m_pBegin, m_pEnd, pBegin);

				m_pEnd = pBegin + (m_pEnd - m_pBegin);
				m_pBegin = pBegin;
			}

			m_capacity = capacity;
		}
	}

	inline void PushBack(const CAnyConstRef& value)
	{
		Reserve(GetSize() + 1);

		STypeOperators::CopyConstruct copyConstruct = m_typeDesc.GetOperators().copyConstruct;
		SXEMA_CORE_ASSERT(copyConstruct);

		(*copyConstruct)(m_pEnd, value.GetValue());

		m_pEnd += m_typeDesc.GetSize();
	}

	inline void PopBack()
	{
		m_pEnd -= m_typeDesc.GetSize();

		STypeOperators::Destruct destruct = m_typeDesc.GetOperators().destruct;
		SXEMA_CORE_ASSERT(destruct);

		(*destruct)(m_pEnd);
	}

	inline void RemoveByIdx(u32 idx)
	{
		// #SchematycTODO : Validate idx?

		if (idx == (GetSize() - 1))
		{
			PopBack();
		}
		else
		{
			// #SchematycTODO : Remove elements rather than destroying and re-constructing?

			STypeOperators::Destruct destruct = m_typeDesc.GetOperators().destruct;
			SXEMA_CORE_ASSERT(destruct);

			STypeOperators::CopyConstruct copyConstruct = m_typeDesc.GetOperators().copyConstruct;
			SXEMA_CORE_ASSERT(copyConstruct);

			u32k stride = m_typeDesc.GetSize();
			for (u8* pPos = m_pBegin + idx, * pNext = pPos + stride; pNext < m_pEnd; pPos = pNext, pNext += stride)
			{
				(*destruct)(pPos);
				(*copyConstruct)(pPos, pNext);
			}
			m_pEnd -= stride;
		}
	}

	inline void RemoveByValue(const CAnyConstRef& value)
	{
		STypeOperators::Equals equals = m_typeDesc.GetOperators().equals;
		SXEMA_CORE_ASSERT(equals);

		for (u32 idx = 0, size = GetSize(); idx < size; )
		{
			if ((*equals)((*this)[idx].GetValue(), value.GetValue()))
			{
				RemoveByIdx(idx);
				--size;
			}
			else
			{
				++idx;
			}
		}
	}

	inline void Clear()
	{
		Destruct(m_pBegin, m_pEnd);

		m_pEnd = m_pBegin;
	}

	inline void ToString(IString& output) const
	{
		output.assign("{ ");

		STypeOperators::ToString toString = m_typeDesc.GetOperators().toString;
		if (toString)
		{
			for (u32 idx = 0, size = GetSize(); idx < size; ++idx)
			{
				CStackString temp;
				(*toString)(temp, (*this)[idx].GetValue());
				output.append(temp.c_str());
				output.append(", ");
			}
			output.TrimRight(", ");
		}

		output.append(" }");
	}

	inline CAnyRef operator[](u32 idx)
	{
		return CAnyRef(m_typeDesc, m_pBegin + (idx * m_typeDesc.GetSize()));
	}

	inline CAnyConstRef operator[](u32 idx) const
	{
		return CAnyRef(m_typeDesc, m_pBegin + (idx * m_typeDesc.GetSize()));
	}

	//inline CAnyArray& operator = (const CAnyArray& rhs);
	//inline bool operator == (const CAnyArray& rhs) const
	//inline bool operator != (const CAnyArray& rhs) const

	static inline void ReflectType(CTypeDesc<CAnyArray>& desc)
	{
		desc.SetGUID("f6af4221-8344-49e9-9ef8-5f7e8144aa57"_drx_guid);
		desc.SetToStringOperator<&CAnyArray::ToString>();
	}

private:

	void CopyConstruct(u8k* pSrc, u8k* pEnd, u8* pDst) const
	{
		STypeOperators::CopyConstruct copyConstruct = m_typeDesc.GetOperators().copyConstruct;
		SXEMA_CORE_ASSERT(copyConstruct);

		for (; pSrc < pEnd; ++pSrc, ++pDst)
		{
			(*copyConstruct)(pDst, pSrc);
		}
	}

	void Move(u8* pSrc, u8* pEnd, u8* pDst) const
	{
		STypeOperators::CopyConstruct copyConstruct = m_typeDesc.GetOperators().copyConstruct;
		SXEMA_CORE_ASSERT(copyConstruct);

		STypeOperators::Destruct destruct = m_typeDesc.GetOperators().destruct;
		SXEMA_CORE_ASSERT(destruct);

		for (; pSrc < pEnd; ++pSrc, ++pDst)
		{
			(*copyConstruct)(pDst, pSrc);
			(*destruct)(pSrc);
		}
	}

	void Destruct(u8* pPos, u8* pEnd) const
	{
		STypeOperators::Destruct destruct = m_typeDesc.GetOperators().destruct;
		SXEMA_CORE_ASSERT(destruct);

		for (; pPos < pEnd; ++pPos)
		{
			(*destruct)(pPos);
		}
	}

private:

	const CCommonTypeDesc& m_typeDesc;   // #SchematycTODO : Rather than storing type info could we just store size, copyConstruct and destruct?
	u32                 m_capacity;
	u8*                 m_pBegin;
	u8*                 m_pEnd;   // #SchematycTODO : Store size explicitly as u32?
};

class CAnyArrayPtr // #SchematycTODO : Create more generic pointer class for storing all types of value on scratchpad? Or just use CAnyPtr?
{
public:

	inline CAnyArrayPtr(CAnyArray* pArray = nullptr)
		: m_pArray(pArray)
	{}

	inline CAnyArrayPtr(const CAnyArrayPtr& rhs)
		: m_pArray(rhs.m_pArray)
	{}

	explicit inline operator bool() const
	{
		return m_pArray != nullptr;
	}

	inline CAnyArray* operator->()
	{
		return m_pArray;
	}

	inline const CAnyArray* operator->() const
	{
		return m_pArray;
	}

	inline CAnyArray& operator*()
	{
		SXEMA_CORE_ASSERT(m_pArray);
		return *m_pArray;
	}

	inline const CAnyArray& operator*() const
	{
		SXEMA_CORE_ASSERT(m_pArray);
		return *m_pArray;
	}

	CAnyArrayPtr& operator=(const CAnyArrayPtr& rhs)
	{
		m_pArray = rhs.m_pArray;
		return *this;
	}

	static inline void ReflectType(CTypeDesc<CAnyArrayPtr>& desc)
	{
		desc.SetGUID("9500b20f-4264-4a09-a7ec-6c8136113369"_drx_guid);
	}

private:

	CAnyArray* m_pArray;
};

}   // sxema
