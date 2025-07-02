// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __BOOLCOMPRESS_H__
#define __BOOLCOMPRESS_H__

#pragma once

#include <drx3D/Network/CommStream.h>
#include <drx3D/Network/ByteStream.h>

class CArithModel;

class CNetInputSerializeImpl;
class CNetOutputSerializeImpl;

class CBoolCompress
{
public:
	CBoolCompress();

#if USE_MEMENTO_PREDICTORS
	static i32k   AdaptBits = 5;
	static u8k StateRange = (1u << AdaptBits) - 1;
	static u8k StateMidpoint = (1u << (AdaptBits - 1)) - 1;

	void ReadMemento(CByteInputStream& stm) const;
	void WriteMemento(CByteOutputStream& stm) const;
	void NoMemento() const;
#endif

#if USE_ARITHSTREAM
	bool ReadValue(CCommInputStream& stm) const;
	void WriteValue(CCommOutputStream& stm, bool value) const;
#else
	bool ReadValue(CNetInputSerializeImpl* stm) const;
	void WriteValue(CNetOutputSerializeImpl* stm, bool value) const;
#endif
#if NET_PROFILE_ENABLE
	i32 GetBitCount();
#endif

private:
#if USE_MEMENTO_PREDICTORS
	static u8k LAST_VALUE_BIT = 0x80;

	mutable bool  m_lastValue;
	mutable u8 m_prob;
#endif
};

#endif
