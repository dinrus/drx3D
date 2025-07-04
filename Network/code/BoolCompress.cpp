// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/BoolCompress.h>
#include  <drx3D/Network/Serialize.h>

CBoolCompress::CBoolCompress()
#if USE_MEMENTO_PREDICTORS
	: m_lastValue(false)
#endif
{
}

#if USE_MEMENTO_PREDICTORS
void CBoolCompress::ReadMemento(CByteInputStream& stm) const
{
	u8 val = stm.GetTyped<u8>();
	m_lastValue = (val & LAST_VALUE_BIT) != 0;
	m_prob = val & StateRange;
	NET_ASSERT(m_prob);
}

void CBoolCompress::WriteMemento(CByteOutputStream& stm) const
{
	stm.PutTyped<u8>() = m_prob | (m_lastValue * LAST_VALUE_BIT);
}

void CBoolCompress::NoMemento() const
{
	m_lastValue = false;
	m_prob = StateMidpoint;
}
#endif

#if USE_ARITHSTREAM
bool CBoolCompress::ReadValue(CCommInputStream& stm) const
{
	u8 curProb = m_prob;
	bool lastValue = m_lastValue;
	NET_ASSERT((*(u8*)&lastValue) < 2);
	bool isLastValue = stm.DecodeShift(AdaptBits) < curProb;
	u16 sym, low;
	bool value;
	if (isLastValue)
	{
		sym = curProb;
		low = 0;
		m_prob = curProb + (curProb != StateRange);
		value = lastValue;
	}
	else
	{
		sym = StateRange + 1 - curProb;
		low = curProb;
		m_prob = curProb - (curProb > 1);
		value = !lastValue;
	}
	stm.UpdateShift(AdaptBits, low, sym);
	m_lastValue = value;
	NetLogPacketDebug("CBoolCompress::ReadValue %d (%f)", value, stm.GetBitSize());
	return value;
}

void CBoolCompress::WriteValue(CCommOutputStream& stm, bool value) const
{
	u8 curProb = m_prob;
	bool lastValue = m_lastValue;
	NET_ASSERT((*(u8*)&lastValue) < 2);
	u16 sym, low;
	if (value == lastValue)
	{
		sym = curProb;
		low = 0;
		m_prob = curProb + (curProb != StateRange);
	}
	else
	{
		sym = StateRange + 1 - curProb;
		low = curProb;
		m_prob = curProb - (curProb > 1);
	}
	stm.EncodeShift(AdaptBits, low, sym);
	m_lastValue = value;
}
#else
bool CBoolCompress::ReadValue(CNetInputSerializeImpl* stm) const
{
	bool ret = stm->ReadBits(1) ? true : false;
	NetLogPacketDebug("CBoolCompress::ReadValue %d (%f)", ret, stm->GetBitSize());
	return ret;
}

void CBoolCompress::WriteValue(CNetOutputSerializeImpl* stm, bool value) const
{
	stm->WriteBits(value ? 1 : 0, 1);
}
#endif

#if NET_PROFILE_ENABLE
i32 CBoolCompress::GetBitCount()
{
	return BITCOUNT_BOOL;
}
#endif
