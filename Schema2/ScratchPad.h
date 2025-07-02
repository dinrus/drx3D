// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Separate POD and non-POD data?
// #SchematycTODO : Does this class really need to live in public interface?

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/BasicTypes.h>
#include <drx3D/Schema2/IAny.h>

namespace sxema2
{
	class CScratchPad
	{
	public:

		inline CScratchPad()
			: m_pData(nullptr)
			, m_size(0)
			, m_capacity(0)
		{}

		inline CScratchPad(const CScratchPad& rhs)
			: m_size(0)
			, m_capacity(0)
			, m_pData(nullptr)
		{
			Reserve(rhs.m_size);
			Clone(m_pData, rhs.m_pData, rhs.m_size);
			m_size = rhs.m_size;
		}

		inline ~CScratchPad()
		{
			Release(m_pData, m_size);
			if(m_pData)
			{
				free(m_pData);
			}
		}

		inline void Reserve(u32 capacity)
		{
			if(capacity > m_capacity)
			{
				static u32k s_minCapacity  = 2;
				static u32k s_growthFactor = 2;
				m_capacity = std::max(std::max(capacity, m_capacity * s_growthFactor), s_minCapacity);
				uk pData = malloc(m_capacity);
				Move(pData, m_pData, m_size);
				if(m_pData)
				{
					free(m_pData);
				}
				m_pData = pData;
			}
		}

		inline u32 PushBack(const IAny& value)
		{
			u32k pos = m_size;
			u32k size = pos + value.GetSize();
			if(size > m_capacity)
			{
				Reserve(size);
			}
			value.Clone(static_cast<u8*>(m_pData) + pos);
			m_size = size;
			return pos;
		}

		inline void ShrinkToFit()
		{
			// #SchematycTODO : Check difference between size and capacity against a threshold.
			uk pData = malloc(m_size);
			Move(pData, m_pData, m_size);
			free(m_pData);
			m_capacity = m_size;
			m_pData    = pData;
		}

		inline void Clear()
		{
			Release(m_pData, m_size);
			if(m_pData)
			{
				free(m_pData);
			}
			m_size     = 0;
			m_capacity = 0;
			m_pData    = nullptr;
		}

		inline CScratchPad& operator = (const CScratchPad& rhs)
		{
			Clear();
			Reserve(rhs.m_size);
			Clone(m_pData, rhs.m_pData, rhs.m_size);
			m_size = rhs.m_size;
			return *this;
		}

		inline IAny* operator [] (u32 pos)
		{
			// #SchematycTODO : Verify position?
			return reinterpret_cast<IAny*>(static_cast<u8*>(m_pData) + pos);
		}

		inline const IAny* operator [] (u32 pos) const
		{
			// #SchematycTODO : Verify position?
			return reinterpret_cast<const IAny*>(static_cast<u8*>(m_pData) + pos);
		}

	private:

		inline void Clone(uk pDst, ukk pSrc, u32 size) const
		{
			for(u32 pos = 0; pos < size; )
			{
				const IAny* pSrcValue = reinterpret_cast<const IAny*>(static_cast<u8k*>(pSrc) + pos);
				pSrcValue->Clone(static_cast<u8*>(pDst) + pos);
				pos += pSrcValue->GetSize();
			}
		}

		inline void Release(uk pData, u32 size) const
		{
			for(u32 pos = 0; pos < size; )
			{
				IAny* pValue = reinterpret_cast<IAny*>(static_cast<u8*>(pData) + pos);
				pos += pValue->GetSize();
				pValue->~IAny();
			}
		}

		inline void Move(uk pDst, uk pSrc, u32 size) const
		{
			Clone(pDst, pSrc, size);
			Release(pSrc, size);
		}

	private:

		u32 m_size;
		u32 m_capacity;
		uk  m_pData;
	};
}
