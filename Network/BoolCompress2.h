// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __BOOLCOMPRESS2_H__
#define __BOOLCOMPRESS2_H__

#pragma once

#include <drx3D/Network/ByteStream.h>
#include <drx3D/Network/CommStream.h>

class CNetInputSerializeImpl;
class CNetOutputSerializeImpl;

class CBoolCompress2
{
public:
	CBoolCompress2() {}
#if USE_MEMENTO_PREDICTORS
	struct TMemento
	{
		u8 prob;
		bool  lastValue;
	};

	static i32k   AdaptBits = 5;
	static u8k StateRange = (1u << AdaptBits) - 1;
	static u8k StateMidpoint = (1u << (AdaptBits - 1)) - 1;

	void ReadMemento(TMemento& memento, CByteInputStream& in) const;
	void WriteMemento(TMemento memento, CByteOutputStream& out) const;
	void InitMemento(TMemento& memento) const;

	void UpdateMemento(TMemento& memento, bool newValue) const;
#endif

#if USE_ARITHSTREAM
	bool ReadValue(TMemento memento, CCommInputStream& in, bool& value) const;
	bool WriteValue(TMemento memento, CCommOutputStream& out, bool value) const;
#else
	#if USE_MEMENTO_PREDICTORS
	bool ReadValue(TMemento memento, CNetInputSerializeImpl* in, bool& value) const;
	bool WriteValue(TMemento memento, CNetOutputSerializeImpl* out, bool value) const;
	#else
	bool ReadValue(CNetInputSerializeImpl* in, bool& value) const;
	bool WriteValue(CNetOutputSerializeImpl* out, bool value) const;
	#endif
#endif
#if NET_PROFILE_ENABLE
	i32 GetBitCount();
#endif

private:
#if USE_MEMENTO_PREDICTORS
	static u8k LAST_VALUE_BIT = 0x80;

	struct SSymLow;
	SSymLow GetSymLow(bool isLastValue, u8 prob) const;
#endif
};

#endif
