// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __WHIRLPOOL_H__
#define __WHIRLPOOL_H__

#pragma once

#include <drx3D/Network/ISerialize.h>

class CWhirlpoolHash
{
public:
	static i32k DIGESTBYTES = 64;

	CWhirlpoolHash();
	CWhirlpoolHash(u8k* input, size_t length);
	CWhirlpoolHash(const string& str);
	string      GetHumanReadable() const;
	void        SerializeWith(TSerialize ser);

	static bool Test();

	ILINE bool  operator==(const CWhirlpoolHash& rhs) const
	{
		return 0 == memcmp(m_hash, rhs.m_hash, DIGESTBYTES);
	}
	ILINE bool operator!=(const CWhirlpoolHash& rhs) const
	{
		return !this->operator==(rhs);
	}

	u8k* operator()() const { return m_hash; }

private:
	u8 m_hash[DIGESTBYTES];
};

#endif
