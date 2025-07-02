// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __UTILS_H__
#define __UTILS_H__

#pragma once

#include <drx3D/Network/Config.h>

bool   StringToKey(tukk s, u32& key);
string KeyToString(u32 key);
void   KeyToString(u32 key, tuk buffer);

template<class T>
bool equiv(const T& a, const T& b)
{
	std::less<T> f;
	if (f(a, b))
		return false;
	else if (f(b, a))
		return false;
	else
		return true;
}

class CCRC8
{
public:
	CCRC8() : m_crc(~0) {}

	ILINE void Add(u8 x)
	{
		m_crc = m_table[m_crc ^ x];
	}

	ILINE void Add32(u32 x)
	{
		Add(x >> 24);
		Add(x >> 16);
		Add(x >> 8);
		Add(x);
	}

	ILINE void Add64(uint64 x)
	{
		Add32((u32)(x >> 32));
		Add32((u32)x);
	}

	u8 Result() const
	{
		return m_crc;
	}

private:
	static u8k m_table[256];
	u8              m_crc;
};

#if ENABLE_DEBUG_KIT
#include <drx3D/CoreX/Math/MTPseudoRandom.h>

class CAutoCorruptAndRestore
{
public:
	CAutoCorruptAndRestore(u8k* pBuffer, size_t nLength, bool corrupt) : m_buf(NULL), m_bit(-1)
	{
		if (corrupt)
		{
			m_buf = const_cast<u8*>(pBuffer);
			m_bit = CMTRand_int32().GenerateUint32() % (nLength * 8);
			m_buf[m_bit / 8] ^= (1 << m_bit % 8);
		}
	}

	~CAutoCorruptAndRestore()
	{
		if (m_buf)
			m_buf[m_bit / 8] ^= (1 << m_bit % 8);
	}

private:
	u8* m_buf;
	size_t m_bit;
};
#endif

#endif
