#include <drx3D/Act/SerializeBits.h>

/*
   =========================================================================================================
   Implementation
   =========================================================================================================
 */

/* This is a naive implementation */
CBitArray::CBitArray(TSerialize* ser)
{
	m_ser = ser;
	if (ser->IsReading())
	{
		ResetForRead();
	}
	else
	{
		ResetForWrite();
	}
}

void CBitArray::ResetForWrite()
{
	m_bytePos = -1;
	m_bitPos = 7;
	m_multiplier = 1;
	m_numberBytes = 0;
	m_isReading = false;
}

void CBitArray::ResetForRead()
{
	m_bitPos = 7;
	m_bytePos = -1;
	m_isReading = true;
}

void CBitArray::PushBit(i32 bit)
{
#if !defined(_RELEASE)
	if (m_bytePos >= maxBytes)
	{
		DrxFatalError("CBitArray ran out of room, maxBytes: %d, will need to be increased, or break up serialisation into separate CBitArray", maxBytes);
	}
#endif
	m_bitPos++;
	if (m_bitPos == 8)
	{
		m_multiplier = 1;
		m_bitPos = 0;
		m_bytePos++;
		assert(m_bytePos < maxBytes);
		PREFAST_ASSUME(m_bytePos < maxBytes);
		m_data[m_bytePos] = 0;
		m_numberBytes++;
	}
	assert((u32)m_bytePos < (u32)maxBytes);
	PREFAST_ASSUME((u32)m_bytePos < (u32)maxBytes);
	m_data[m_bytePos] |= m_multiplier * (bit & 1);  // Use multiplier because variable bit shift on consoles is really slow
	m_multiplier = m_multiplier << 1;
}

i32 CBitArray::NumberOfBitsPushed()
{
	return m_bytePos * 8 + m_bitPos + 1;
}

i32 CBitArray::PopBit()  /* from the front */
{
	m_bitPos++;
	if (m_bitPos == 8)
	{
		m_bitPos = 0;
		m_bytePos++;
		DRX_ASSERT(m_ser->IsReading());
		m_ser->Value("bitarray", m_readByte); // read a byte
	}
	u8 ret = m_readByte & 1;
	m_readByte = m_readByte >> 1;
	return ret;
}

void CBitArray::ReadBits(u8* out, i32 numBits)
{
	u8 byte;
	while (numBits >= 8)
	{
		byte = PopBit();
		byte |= PopBit() << 1;
		byte |= PopBit() << 2;
		byte |= PopBit() << 3;
		byte |= PopBit() << 4;
		byte |= PopBit() << 5;
		byte |= PopBit() << 6;
		byte |= PopBit() << 7;
		*out = byte;
		out++;
		numBits = numBits - 8;
	}
	switch (numBits)
	{
	case 0:
		break;
	case 1:
		*out = PopBit();
		break;
	case 2:
		byte = PopBit();
		byte |= PopBit() << 1;
		*out = byte;
		break;
	case 3:
		byte = PopBit();
		byte |= PopBit() << 1;
		byte |= PopBit() << 2;
		*out = byte;
		break;
	case 4:
		byte = PopBit();
		byte |= PopBit() << 1;
		byte |= PopBit() << 2;
		byte |= PopBit() << 3;
		*out = byte;
		break;
	case 5:
		byte = PopBit();
		byte |= PopBit() << 1;
		byte |= PopBit() << 2;
		byte |= PopBit() << 3;
		byte |= PopBit() << 4;
		*out = byte;
		break;
	case 6:
		byte = PopBit();
		byte |= PopBit() << 1;
		byte |= PopBit() << 2;
		byte |= PopBit() << 3;
		byte |= PopBit() << 4;
		byte |= PopBit() << 5;
		*out = byte;
		break;
	case 7:
		byte = PopBit();
		byte |= PopBit() << 1;
		byte |= PopBit() << 2;
		byte |= PopBit() << 3;
		byte |= PopBit() << 4;
		byte |= PopBit() << 5;
		byte |= PopBit() << 6;
		*out = byte;
		break;
	}
}

void CBitArray::WriteBits(u8k* in, i32 numBits)
{
	u8 v;
	while (numBits >= 8)
	{
		v = *in;
		PushBit((v >> 0) & 1);
		PushBit((v >> 1) & 1);
		PushBit((v >> 2) & 1);
		PushBit((v >> 3) & 1);
		PushBit((v >> 4) & 1);
		PushBit((v >> 5) & 1);
		PushBit((v >> 6) & 1);
		PushBit((v >> 7) & 1);
		numBits = numBits - 8;
		in++;
	}
	v = *in;
	switch (numBits)
	{
	case 0:
		break;
	case 1:
		PushBit((v >> 0) & 1);
		break;
	case 2:
		PushBit((v >> 0) & 1);
		PushBit((v >> 1) & 1);
		break;
	case 3:
		PushBit((v >> 0) & 1);
		PushBit((v >> 1) & 1);
		PushBit((v >> 2) & 1);
		break;
	case 4:
		PushBit((v >> 0) & 1);
		PushBit((v >> 1) & 1);
		PushBit((v >> 2) & 1);
		PushBit((v >> 3) & 1);
		break;
	case 5:
		PushBit((v >> 0) & 1);
		PushBit((v >> 1) & 1);
		PushBit((v >> 2) & 1);
		PushBit((v >> 3) & 1);
		PushBit((v >> 4) & 1);
		break;
	case 6:
		PushBit((v >> 0) & 1);
		PushBit((v >> 1) & 1);
		PushBit((v >> 2) & 1);
		PushBit((v >> 3) & 1);
		PushBit((v >> 4) & 1);
		PushBit((v >> 5) & 1);
		break;
	case 7:
		PushBit((v >> 0) & 1);
		PushBit((v >> 1) & 1);
		PushBit((v >> 2) & 1);
		PushBit((v >> 3) & 1);
		PushBit((v >> 4) & 1);
		PushBit((v >> 5) & 1);
		PushBit((v >> 6) & 1);
		break;
	}
}


void CBitArray::WriteToSerializer()
{
	DRX_ASSERT(IsReading() == 0);

	for (i32 i = 0; i < m_numberBytes; i++)
	{
		m_ser->Value("bitarray", m_data[i]);
	}
}