// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __STATIONARYUNSIGNEDINTEGER_H__
#define __STATIONARYUNSIGNEDINTEGER_H__

#pragma once

#include <drx3D/Network/CommStream.h>
#include <drx3D/Network/ByteStream.h>

class CNetOutputSerializeImpl;
class CNetInputSerializeImpl;

class CStationaryUnsignedInteger
{
public:
	CStationaryUnsignedInteger(u32 nMin, u32 nMax);
	CStationaryUnsignedInteger();

	void SetValues(u32 nMin, u32 nMax)
	{
		m_nMin = nMin;
		m_nMax = nMax;
		PreComputeBits();
		NET_ASSERT(m_nMax > m_nMin);
	}

	void PreComputeBits()
	{
		m_numBits = IntegerLog2_RoundUp(u32(m_nMax - m_nMin) + 1);
	}

	bool Load(XmlNodeRef node, const string& filename, const string& child = "Params");
#if USE_MEMENTO_PREDICTORS
	bool WriteMemento(CByteOutputStream& stm) const;
	bool ReadMemento(CByteInputStream& stm) const;
	void NoMemento() const;
#endif

#if USE_ARITHSTREAM
	void   WriteValue(CCommOutputStream& stm, u32 value) const;
	u32 ReadValue(CCommInputStream& stm) const;
#else
	void   WriteValue(CNetOutputSerializeImpl* stm, u32 value) const;
	u32 ReadValue(CNetInputSerializeImpl* stm) const;
#endif
#if NET_PROFILE_ENABLE
	i32 GetBitCount();
#endif

private:
	u32 Quantize(u32 x) const;
	u32 Dequantize(u32 x) const;

	// static data
	uint64 m_nMin;
	uint64 m_nMax;
	u32 m_numBits;

#if USE_MEMENTO_PREDICTORS
	// memento data
	mutable u32 m_oldValue;
	mutable u32 m_probabilitySame;
	mutable bool   m_haveMemento;
#endif
};

#endif
