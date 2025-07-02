// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/StationaryUnsignedInteger.h>
#include <limits>
#include  <drx3D/Network/Serialize.h>

CStationaryUnsignedInteger::CStationaryUnsignedInteger(u32 nMin, u32 nMax)
	: m_nMin(nMin)
	, m_nMax(nMax)
#if USE_MEMENTO_PREDICTORS
	, m_haveMemento(false)
#endif
{
	PreComputeBits();
	NET_ASSERT(m_nMax > m_nMin);
}
CStationaryUnsignedInteger::CStationaryUnsignedInteger()
	: m_nMin(0)
	, m_nMax(1000)
#if USE_MEMENTO_PREDICTORS
	, m_haveMemento(false)
#endif
{
	PreComputeBits();
}

bool CStationaryUnsignedInteger::Load(XmlNodeRef node, const string& filename, const string& child)
{
	bool ok = true;
	if (XmlNodeRef params = node->findChild(child))
	{
		int64 mn, mx;
		ok &= params->getAttr("min", mn);
		ok &= params->getAttr("max", mx);
		ok &= mx > mn;
		if (mx > std::numeric_limits<u32>::max() || mn < std::numeric_limits<u32>::min())
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
		NetWarning("CStationaryUnsignedInteger couldn't find parameters named %s at %s:%d", child.c_str(), filename.c_str(), node->getLine());
		ok = false;
	}
	return ok;
}

#if USE_MEMENTO_PREDICTORS
bool CStationaryUnsignedInteger::WriteMemento(CByteOutputStream& stm) const
{
	stm.PutTyped<u32>() = m_oldValue;
	stm.PutTyped<u8>() = m_probabilitySame;
	m_haveMemento = true;
	return true;
}

bool CStationaryUnsignedInteger::ReadMemento(CByteInputStream& stm) const
{
	m_oldValue = stm.GetTyped<u32>();
	m_probabilitySame = stm.GetTyped<u8>();
	m_haveMemento = true;
	return true;
}

void CStationaryUnsignedInteger::NoMemento() const
{
	m_oldValue = Quantize(u32((m_nMin + m_nMax) / 2));
	m_probabilitySame = 8;
	m_haveMemento = false;
}
#endif

u32 CStationaryUnsignedInteger::Quantize(u32 x) const
{
	int64 y = (int64)x;
	if (y < (int64)m_nMin)
		return 0;
	else if (y >= (int64)m_nMax)
		return u32(m_nMax - m_nMin);
	else
		return (u32)(y - m_nMin);
}

u32 CStationaryUnsignedInteger::Dequantize(u32 x) const
{
	return u32(x + m_nMin);
}

#if USE_ARITHSTREAM
void CStationaryUnsignedInteger::WriteValue(CCommOutputStream& stm, u32 value) const
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

u32 CStationaryUnsignedInteger::ReadValue(CCommInputStream& stm) const
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
	u32 ret = Dequantize(quantized);
	NetLogPacketDebug("CStationaryUnsignedInteger::ReadValue %d Min %" PRId64 " Max %" PRId64 " NumBits %d (%f)", ret, m_nMin, m_nMax, m_numBits, stm.GetBitSize());
	return ret;
}
#else
void CStationaryUnsignedInteger::WriteValue(CNetOutputSerializeImpl* stm, u32 value) const
{
	u32 quantized = Quantize(value);

	stm->WriteBits(quantized, m_numBits);
}

u32 CStationaryUnsignedInteger::ReadValue(CNetInputSerializeImpl* stm) const
{
	u32 quantized = stm->ReadBits(m_numBits);
	u32 ret = Dequantize(quantized);
	NetLogPacketDebug("CStationaryUnsignedInteger::ReadValue %d Min %" PRId64 " Max %" PRId64 " NumBits %d (%f)", ret, m_nMin, m_nMax, m_numBits, stm->GetBitSize());
	return ret;
}
#endif

#if NET_PROFILE_ENABLE
i32 CStationaryUnsignedInteger::GetBitCount()
{
	return m_numBits;
}
#endif
