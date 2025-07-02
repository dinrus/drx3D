// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace sxema
{
template<typename TYPE, u32 SIZE> class CRingBuffer
{
public:

	inline CRingBuffer()
		: m_size(0)
		, m_pos(0)
	{
		std::fill(m_data, m_data + SIZE, TYPE());
	}

	inline void PushBack(const TYPE& value)
	{
		if (m_size < SIZE)
		{
			m_data[m_size++] = value;
		}
		else
		{
			m_data[m_pos] = value;
			m_pos = Wrap(m_pos + 1);
		}
	}

	inline TYPE& Front()
	{
		return (*this)[0];
	}

	inline const TYPE& Front() const
	{
		return (*this)[0];
	}

	inline TYPE& Back()
	{
		return (*this)[m_size - 1];
	}

	inline const TYPE& Back() const
	{
		return (*this)[m_size - 1];
	}

	inline TYPE& operator[](u32 idx)
	{
		DRX_ASSERT(idx < m_size);
		return m_data[Wrap(m_pos + idx)];
	}

	inline const TYPE& operator[](u32 idx) const
	{
		DRX_ASSERT(idx < m_size);
		return m_data[Wrap(m_pos + idx)];
	}

	inline u32 Size() const
	{
		return m_size;
	}

	inline void Clear()
	{
		std::fill(m_data, m_data + SIZE, TYPE());
		m_size = 0;
		m_pos = 0;
	}

private:

	inline u32 Wrap(u32 pos) const
	{
		return pos < m_size ? pos : pos - m_size;
	}

private:

	TYPE   m_data[SIZE];
	u32 m_size;
	u32 m_pos;
};
} // sxema
