// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/StationaryInteger.h>
#include <limits>
#include  <drx3D/Network/Serialize.h>

CStationaryInteger::CStationaryInteger(i32 nMin, i32 nMax)
	: m_nMin(nMin)
	, m_nMax(nMax)
#if USE_MEMENTO_PREDICTORS
	, m_haveMemento(false)
#endif
{
	PreComputeBits();
	NET_ASSERT(m_nMax > m_nMin);
}

CStationaryInteger::CStationaryInteger()
	: m_nMin(-1000)
	, m_nMax(1000)
#if USE_MEMENTO_PREDICTORS
	, m_haveMemento(false)
#endif
{
	PreComputeBits();
}

bool CStationaryInteger::Load(XmlNodeRef node, const string& filename, const string& child)
{
	bool ok = true;
	if (XmlNodeRef params = node->findChild(child))
	{
		int64 mn, mx;
		ok &= params->getAttr("min", mn);
		ok &= params->getAttr("max", mx);
		ok &= mx > mn;
		if ((mx > std::numeric_limits<i32>::max() || mn < std::numeric_limits<i32>::min()) &&
		    (mx > std::numeric_limits<u32>::max() || mn < std::numeric_limits<u32>::min()))
			ok = false;
		if (ok)
		{
			m_nMin = mn;
			m_nMax = mx;
			PreComputeBits();
		}
	}
	else
	{
		NetWarning("StationaryInteger couldn't find parameters named %s at %s:%d", child.c_str(), filename.c_str(), node->getLine());
		ok = false;
	}
	return ok;
}

#if USE_MEMENTO_PREDICTORS
bool CStationaryInteger::WriteMemento(CByteOutputStream& stm) const
{
	stm.PutTyped<u32>() = m_oldValue;
	stm.PutTyped<u8>() = m_probabilitySame;
	m_haveMemento = true;
	return true;
}

bool CStationaryInteger::ReadMemento(CByteInputStream& stm) const
{
	m_oldValue = stm.GetTyped<u32>();
	m_probabilitySame = stm.GetTyped<u8>();
	m_haveMemento = true;
	return true;
}

void CStationaryInteger::NoMemento() const
{
	m_oldValue = Quantize(i32((m_nMin + m_nMax) / 2));
	m_probabilitySame = 8;
	m_haveMemento = false;
}
#endif

u32 CStationaryInteger::Quantize(i32 x) const
{
	int64 y = x;
	if (y < m_nMin)
		return 0;
	else if (y >= m_nMax)
		return u32(m_nMax - m_nMin);
	else
		return (u32)(y - m_nMin);
}

i32 CStationaryInteger::Dequantize(u32 x) const
{
	return i32(x + m_nMin);
}

#if USE_ARITHSTREAM
void CStationaryInteger::WriteValue(CCommOutputStream& stm, i32 value) const
{
	u32 quantized = Quantize(value);

	if (m_haveMemento)
	{
		if (quantized == m_oldValue)
		{
			stm.EncodeShift(4, 0, m_probabilitySame);
			m_probabilitySame = m_probabilitySame + (m_probabilitySame < 15);
		}
		else
		{
			stm.EncodeShift(4, m_probabilitySame, 16 - m_probabilitySame);
			stm.WriteInt(quantized, u32(m_nMax - m_nMin));
			m_probabilitySame = 1;
		}
	}
	else
	{
		stm.WriteInt(quantized, u32(m_nMax - m_nMin));
		m_probabilitySame = 1;
	}

	m_oldValue = quantized;
}

i32 CStationaryInteger::ReadValue(CCommInputStream& stm) const
{
	u32 quantized;

	if (m_haveMemento)
	{
		u16 probSame = stm.DecodeShift(4);
		if (probSame < m_probabilitySame)
		{
			stm.UpdateShift(4, 0, m_probabilitySame);
			quantized = m_oldValue;
			m_probabilitySame = m_probabilitySame + (m_probabilitySame < 15);
		}
		else
		{
			stm.UpdateShift(4, m_probabilitySame, 16 - m_probabilitySame);
			quantized = stm.ReadInt(u32(m_nMax - m_nMin));
			m_probabilitySame = 1;
		}
	}
	else
	{
		quantized = stm.ReadInt(u32(m_nMax - m_nMin));
		m_probabilitySame = 1;
	}

	m_oldValue = quantized;
	i32 ret = Dequantize(quantized);
	NetLogPacketDebug("CStationaryInteger::ReadValue %d Min %" PRIi64 " Max %" PRIi64 " NumBits %d (%f)", ret, m_nMin, m_nMax, m_numBits, stm.GetBitSize());
	return ret;
}
#else
void CStationaryInteger::WriteValue(CNetOutputSerializeImpl* stm, i32 value) const
{
	u32 quantized = Quantize(value);

	stm->WriteBits(quantized, m_numBits);
}

i32 CStationaryInteger::ReadValue(CNetInputSerializeImpl* stm) const
{
	u32 quantized = stm->ReadBits(m_numBits);
	i32 ret = Dequantize(quantized);
	NetLogPacketDebug("CStationaryInteger::ReadValue %d Min %" PRIi64 " Max %" PRIi64 " NumBits %d (%f)", ret, m_nMin, m_nMax, m_numBits, stm->GetBitSize());
	return ret;
}
#endif

#if NET_PROFILE_ENABLE
i32 CStationaryInteger::GetBitCount()
{
	return m_numBits;
}
#endif
