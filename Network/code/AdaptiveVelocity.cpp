// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/AdaptiveVelocity.h>
#include  <drx3D/Network/ArithPrimitives.h>
#include  <drx3D/Network/Serialize.h>

bool CAdaptiveVelocity::Load(XmlNodeRef node, const string& filename, const string& child)
{
	bool ok = true;
	if (m_quantizer.Load(node, filename, child, eFQM_TruncateLeft))
	{
#if USE_MEMENTO_PREDICTORS
		i32 height = 1024000;
		node->getAttr("height", height);
		if (height < 0)
			ok = false;

		if (ok)
		{
			m_nHeight = height;
		}
#endif
	}
	else
	{
		ok = false;
	}
	return ok;
}

#if USE_MEMENTO_PREDICTORS
void CAdaptiveVelocity::ReadMemento(CByteInputStream& stm) const
{
	m_boolCompress.ReadMemento(stm);
	m_nLastQuantized = stm.GetTyped<u32>();
}

void CAdaptiveVelocity::WriteMemento(CByteOutputStream& stm) const
{
	m_boolCompress.WriteMemento(stm);
	stm.PutTyped<u32>() = m_nLastQuantized;
}

void CAdaptiveVelocity::NoMemento() const
{
	m_boolCompress.NoMemento();
	m_nLastQuantized = 0;
}
#endif

#if USE_ARITHSTREAM
bool CAdaptiveVelocity::ReadValue(CCommInputStream& stm, float& value) const
{
	u32 left, right;
	u32 quantized = left = right = m_nLastQuantized;

	bool moved = m_boolCompress.ReadValue(stm);

	if (moved)
	{
		i32 searchArea = m_quantizer.GetMaxQuantizedValue() / i32(50);
		if (searchArea == 0)
			searchArea = 1;
		left = (u32)((int64(m_nLastQuantized) - searchArea > 0) ? m_nLastQuantized - searchArea : 0);
		right = (u32)((uint64(m_nLastQuantized) + searchArea < m_quantizer.GetMaxQuantizedValue()) ? m_nLastQuantized + searchArea : m_quantizer.GetMaxQuantizedValue());

		if (!SquarePulseProbabilityReadImproved(quantized, stm, left, right, 1024, m_quantizer.GetMaxQuantizedValue(), 95, m_quantizer.GetNumBits()))
			return false;
		m_nLastQuantized = quantized;
	}

	value = m_quantizer.Dequantize(quantized);
	NetLogPacketDebug("CAdaptiveVelocity::ReadValue Previously Read (CBoolCompress) %f Min %f Max %f NumBits %d (%f)", value, m_quantizer.GetMinValue(), m_quantizer.GetMaxValue(), m_quantizer.GetNumBits(), stm.GetBitSize());
	return true;
}

void CAdaptiveVelocity::WriteValue(CCommOutputStream& stm, float value) const
{
	//quantize value
	u32 quantized = m_quantizer.Quantize(value);
	u32 left, right;
	left = right = m_nLastQuantized;

	bool moved = (quantized != m_nLastQuantized) ? true : false;
	m_boolCompress.WriteValue(stm, moved);
	if (moved)
	{
		i32 searchArea = m_quantizer.GetMaxQuantizedValue() / i32(50);
		if (searchArea == 0)
			searchArea = 1;
		left = (u32)((int64(m_nLastQuantized) - searchArea > 0) ? m_nLastQuantized - searchArea : 0);
		right = (u32)((uint64(m_nLastQuantized) + searchArea < m_quantizer.GetMaxQuantizedValue()) ? m_nLastQuantized + searchArea : m_quantizer.GetMaxQuantizedValue());
		SquarePulseProbabilityWriteImproved(stm, quantized, left, right, 1024, m_quantizer.GetMaxQuantizedValue(), 95, m_quantizer.GetNumBits());

		m_nLastQuantized = quantized;
	}
}
#else
bool CAdaptiveVelocity::ReadValue(CNetInputSerializeImpl* stm, float& value) const
{
	u32 quantized = stm->ReadBits(m_quantizer.GetNumBits());

	value = m_quantizer.Dequantize(quantized);
	NetLogPacketDebug("CAdaptiveVelocity::ReadValue %f Min %f Max %f NumBits %d (%f)", value, m_quantizer.GetMinValue(), m_quantizer.GetMaxValue(), m_quantizer.GetNumBits(), stm->GetBitSize());

	return true;
}

void CAdaptiveVelocity::WriteValue(CNetOutputSerializeImpl* stm, float value) const
{
	u32 quantized = m_quantizer.Quantize(value);

	stm->WriteBits(quantized, m_quantizer.GetNumBits());
}
#endif
#if NET_PROFILE_ENABLE
i32 CAdaptiveVelocity::GetBitCount()
{
	return
	#if USE_MEMENTO_PREDICTORS
	  m_boolCompress.GetBitCount() +
	#endif
	  m_quantizer.GetNumBits();
}
#endif
