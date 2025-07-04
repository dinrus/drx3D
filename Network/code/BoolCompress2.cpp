// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/BoolCompress2.h>
#include  <drx3D/Network/Serialize.h>

#if USE_MEMENTO_PREDICTORS

struct CBoolCompress2::SSymLow
{
	u16 sym;
	u16 low;
};

void CBoolCompress2::ReadMemento(TMemento& memento, CByteInputStream& in) const
{
	u8 val = in.GetTyped<u8>();
	memento.lastValue = (val & LAST_VALUE_BIT) != 0;
	memento.prob = val & StateRange;
	NET_ASSERT(memento.prob);
}

void CBoolCompress2::WriteMemento(TMemento memento, CByteOutputStream& out) const
{
	out.PutTyped<u8>() = memento.prob | (memento.lastValue * LAST_VALUE_BIT);
}

void CBoolCompress2::InitMemento(TMemento& memento) const
{
	memento.lastValue = false;
	memento.prob = StateMidpoint;
}

void CBoolCompress2::UpdateMemento(TMemento& memento, bool newValue) const
{
	if (memento.lastValue == newValue)
		memento.prob += (memento.prob != StateRange);
	else
		memento.prob -= (memento.prob > 1);
	memento.lastValue = newValue;
}
#endif

#if USE_ARITHSTREAM
bool CBoolCompress2::ReadValue(TMemento memento, CCommInputStream& in, bool& value) const
{
	bool isLastValue = in.DecodeShift(AdaptBits) < memento.prob;
	SSymLow sl = GetSymLow(isLastValue, memento.prob);
	in.UpdateShift(AdaptBits, sl.low, sl.sym);
	*(u8*)& value = (memento.lastValue ^ 1) ^ *(u8*)&isLastValue;
	NetLogPacketDebug("CBoolCompress2::ReadValue %d (%f)", value, in.GetBitSize());
	return true;
}

bool CBoolCompress2::WriteValue(TMemento memento, CCommOutputStream& out, bool value) const
{
	SSymLow sl = GetSymLow(value == memento.lastValue, memento.prob);
	out.EncodeShift(AdaptBits, sl.low, sl.sym);
	return true;
}
#else
	#if USE_MEMENTO_PREDICTORS
bool CBoolCompress2::ReadValue(TMemento memento, CNetInputSerializeImpl* in, bool& value) const
{
	value = in->ReadBits(1) ? true : false;
	NetLogPacketDebug("CBoolCompress2::ReadValue %d (%f)", value, in->GetBitSize());
	return true;
}

bool CBoolCompress2::WriteValue(TMemento memento, CNetOutputSerializeImpl* out, bool value) const
{
	out->WriteBits(value ? 1 : 0, 1);
	return true;
}
	#else
bool CBoolCompress2::ReadValue(CNetInputSerializeImpl* in, bool& value) const
{
	value = in->ReadBits(1) ? true : false;
	NetLogPacketDebug("CBoolCompress2::ReadValue %d (%f)", value, in->GetBitSize());
	return true;
}

bool CBoolCompress2::WriteValue(CNetOutputSerializeImpl* out, bool value) const
{
	out->WriteBits(value ? 1 : 0, 1);
	return true;
}
	#endif
#endif

#if NET_PROFILE_ENABLE
i32 CBoolCompress2::GetBitCount()
{
	return BITCOUNT_BOOL;
}
#endif

#if USE_MEMENTO_PREDICTORS
ILINE CBoolCompress2::SSymLow CBoolCompress2::GetSymLow(bool isLastValue, u8 prob) const
{
	assert(prob > 0);
	assert(prob <= StateRange);
	SSymLow sl;
	sl.low = prob * !isLastValue;
	sl.sym = prob * !!isLastValue + (StateRange + 1 - prob) * !isLastValue;
	return sl;
}
#endif

#include <drx3D/Sys/DrxUnitTest.h>

#if defined(DRX_UNIT_TESTING) && USE_MEMENTO_PREDICTORS

DRX_UNIT_TEST_SUITE(NetCompression)
{

	CBoolCompress2::TMemento ChangeMemento(const CBoolCompress2& pol, bool value, CBoolCompress2::TMemento m)
	{
		pol.UpdateMemento(m, value);
		DRX_UNIT_TEST_ASSERT(m.lastValue == value);
		DRX_UNIT_TEST_ASSERT(m.prob > 0);
		DRX_UNIT_TEST_ASSERT(m.prob <= CBoolCompress2::StateRange);
		return m;
	}

	void NoOverflowHelper1(bool value)
	{
		CBoolCompress2::TMemento m;
		CBoolCompress2 pol;
		pol.InitMemento(m);
		pol.UpdateMemento(m, value);
		for (i32 i = 0; i < 50; i++)
		{
			CBoolCompress2::TMemento m1 = ChangeMemento(pol, value, m);
			DRX_UNIT_TEST_ASSERT(m1.prob >= m.prob);
		}
	}

	void NoOverflowHelper2()
	{
		CBoolCompress2::TMemento m;
		CBoolCompress2 pol;
		pol.InitMemento(m);
		for (i32 i = 0; i < 50; i++)
		{
			CBoolCompress2::TMemento m1 = ChangeMemento(pol, !m.lastValue, m);
			DRX_UNIT_TEST_ASSERT(m1.prob <= m.prob);
		}
	}

	DRX_UNIT_TEST(NoOverflow)
	{
		NoOverflowHelper1(true);
		NoOverflowHelper1(false);
		NoOverflowHelper2();
	}

	#if USE_ARITHSTREAM
	void EncDecTest(const CBoolCompress2::TMemento m, const bool value)
	{
		u8 buffer[64];
		CCommOutputStream out(buffer, sizeof(buffer));
		CBoolCompress2 c;
		c.WriteValue(m, out, value);
		size_t len = out.Flush();
		CCommInputStream in(buffer, len);
		bool x;
		c.ReadValue(m, in, x);
		DRX_UNIT_TEST_ASSERT(x == value);
		#if CRC8_ENCODING
		DRX_UNIT_TEST_ASSERT(out.GetCRC() == in.GetCRC());
		#endif
	}
	#else
	void EncDecTest(const CBoolCompress2::TMemento m, const bool value)
	{
		u8 buffer[64];
		CNetOutputSerializeImpl out(buffer, sizeof(buffer));
		CBoolCompress2 c;
		c.WriteValue(m, &out, value);
		size_t len = out.Flush();
		CNetInputSerializeImpl in(buffer, len);
		bool x;
		c.ReadValue(m, &in, x);
		DRX_UNIT_TEST_ASSERT(x == value);
		#if CRC8_ENCODING
		DRX_UNIT_TEST_ASSERT(out.GetCRC() == in.GetCRC());
		#endif
	}
	#endif

	DRX_UNIT_TEST(EncDec)
	{
		for (i32 i = 0; i < 2; i++)
		{
			for (i32 j = 0; j < 2; j++)
			{
				for (i32 k = 1; k <= CBoolCompress2::StateRange; k++)
				{
					CBoolCompress2::TMemento m;
					m.lastValue = (i != 0);
					m.prob = k;
					bool value = j != 0;
					EncDecTest(m, value);
				}
			}
		}
	}

}

#endif
