// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Move functions to .inl file?
// #SchematycTODO : Do we need to worry about data alignment?

#pragma once

#include <type_traits>

#include <drx3D/CoreX/Math/Drx_Math.h>

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/Any.h>

namespace sxema
{

class CScratchpad
{
private:

	class CAnyValueImpl : public CAnyValue
	{
	public:

		inline CAnyValueImpl(const CCommonTypeDesc& typeDesc, ukk pValue)
			: CAnyValue(typeDesc, pValue)
		{}
	};

public:

	inline CScratchpad() {}

	inline CScratchpad(const CScratchpad& rhs)
	{
		Copy(rhs);
	}

	explicit inline CScratchpad(const SInPlaceStorageParams& storage)
		: m_capacity(storage.capacity)
		, m_pData(static_cast<u8*>(storage.pData))
		, m_bInPlace(true)
	{}

	inline CScratchpad(const SInPlaceStorageParams& storage, const CScratchpad& rhs)
		: m_capacity(storage.capacity)
		, m_pData(static_cast<u8*>(storage.pData))
		, m_bInPlace(true)
	{
		Copy(rhs);
	}

	inline ~CScratchpad()
	{
		Clear();
		if (m_pData && !m_bInPlace)
		{
			DrxModuleFree(m_pData);
		}
	}

	inline void Reserve(u32 capacity)
	{
		if (capacity > m_capacity)
		{
			static u32k minCapacity = 16;
			static u32k growthFactor = 2;
			m_capacity = max(max(capacity, m_capacity * growthFactor), minCapacity);

			u8* pData = static_cast<u8*>(DrxModuleMalloc(m_capacity));

			if (m_pData)
			{
				MoveData(pData, m_pData, m_size);
				if (!m_bInPlace)
				{
					DrxModuleFree(m_pData);
				}
			}

			m_pData = pData;
			m_bInPlace = false;
		}
	}

	inline u32 Add(const CAnyConstRef& value)
	{
		const CCommonTypeDesc& typeDesc = value.GetTypeDesc();

		u32k size = m_size + sizeof(CAnyValueImpl) + typeDesc.GetSize();
		Reserve(size);

		u32k pos = m_size;
		m_size = size;

		new(m_pData + pos)CAnyValueImpl(typeDesc, value.GetValue());

		return pos;
	}

	inline CAnyPtr Get(u32 pos)
	{
		return pos < m_size ? reinterpret_cast<CAnyValue*>(m_pData + pos) : CAnyPtr();
	}

	inline CAnyConstPtr Get(u32 pos) const
	{
		return pos < m_size ? reinterpret_cast<const CAnyValue*>(m_pData + pos) : CAnyConstPtr();
	}

	inline void ShrinkToFit()
	{
		if (m_pData && !m_bInPlace)
		{
			static u32k shrinkThreshold = 64;
			if ((m_capacity - m_size) >= shrinkThreshold)
			{
				u8* pData = static_cast<u8*>(DrxModuleMalloc(m_capacity));
				MoveData(pData, m_pData, m_size);

				DrxModuleFree(m_pData);

				m_capacity = m_size;
				m_pData = pData;
			}
		}
	}

	inline void Clear()
	{
		if (m_pData)
		{
			ClearData(m_pData, m_size);

			m_size = 0;
		}
	}

	inline void operator=(const CScratchpad& rhs)
	{
		Copy(rhs);
	}

	inline bool Compare(const CScratchpad &rhs) const
	{
		if (m_size != rhs.m_size)
			return false;
		return 0 == memcmp(m_pData,rhs.m_pData,m_size);
	}

protected:

	inline void Copy(const CScratchpad& rhs)
	{
		if (this != &rhs)
		{
			Clear();
			Reserve(rhs.m_size);
			CloneData(m_pData, rhs.m_pData, rhs.m_size);
			m_size = rhs.m_size;
		}
	}

private:

	inline void CloneData(u8* pDst, u8k* pSrc, u32 size) const
	{
		for (u32 pos = 0; pos < size; )
		{
			const CAnyValueImpl* pSrcValue = reinterpret_cast<const CAnyValueImpl*>(pSrc + pos);
			const CCommonTypeDesc& typeDesc = pSrcValue->GetTypeDesc();
			new(pDst + pos)CAnyValueImpl(typeDesc, pSrcValue->GetValue());
			pos += sizeof(CAnyValueImpl) + typeDesc.GetSize();
		}
	}

	inline void ClearData(u8* pData, u32 size) const
	{
		for (u32 pos = 0; pos < size; )
		{
			CAnyValueImpl* pValue = reinterpret_cast<CAnyValueImpl*>(pData + pos);
			const CCommonTypeDesc& typeDesc = pValue->GetTypeDesc();
			pos += sizeof(CAnyValueImpl) + typeDesc.GetSize();
			pValue->~CAnyValueImpl();
		}
	}

	inline void MoveData(u8* pDst, u8* pSrc, u32 size) const
	{
		CloneData(pDst, pSrc, size);
		ClearData(pSrc, size);
	}

private:

	u32 m_capacity = 0;
	u32 m_size = 0;
	u8* m_pData = nullptr;
	bool   m_bInPlace = false;
};

template<u32 CAPACITY> class CInPlaceScratchpad : public CScratchpad
{
public:

	inline CInPlaceScratchpad()
		: CScratchpad(SInPlaceStorageParams(CAPACITY, m_storage))
	{}

	inline CInPlaceScratchpad(const CScratchpad& rhs)
		: CScratchpad(SInPlaceStorageParams(CAPACITY, m_storage), rhs)
	{}

	inline CInPlaceScratchpad(const CInPlaceScratchpad& rhs)
		: CScratchpad(SInPlaceStorageParams(CAPACITY, m_storage), rhs)
	{}

	template <u32 RHS_CAPACITY> inline CInPlaceScratchpad(const CInPlaceScratchpad<RHS_CAPACITY>& rhs)
		: CScratchpad(SInPlaceStorageParams(CAPACITY, m_storage), rhs)
	{}

private:

	u8 m_storage[CAPACITY];
};

typedef CScratchpad             HeapScratchpad;
typedef CInPlaceScratchpad<256> StackScratchpad;

} // sxema
