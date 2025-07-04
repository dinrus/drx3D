// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>

#include  <drx3D/Network/PredictiveFloat.h>

#include  <drx3D/Network/ArithPrimitives.h>
#include  <drx3D/Network/Serialize.h>

bool CPredictiveFloat::Load(XmlNodeRef node, const string& filename, const string& child)
{
	bool ret = m_quantizer.Load(node, filename, child);

#if USE_MEMENTO_PREDICTORS
	if (XmlNodeRef params = node->findChild(child))
	{
		float quantizationScale = m_quantizer.Dequantize(2) - m_quantizer.Dequantize(1);

		CPredictiveBase::Load(params, quantizationScale);
	}
#endif

	return ret;
}


#if USE_ARITHSTREAM
void CPredictiveFloat::WriteValue(CCommOutputStream& stm, float value, u32 mementoAge, u32 timeFraction32) const
{
	//quantize value

	u32 quantized = m_quantizer.Quantize(value);

	i32 predicted = Predict(timeFraction32);

	predicted = clamp_tpl(predicted, 0, (i32)m_quantizer.GetMaxQuantizedValue());

	i32 error = quantized - predicted;

	m_mementoSequenceId += mementoAge;

	if (m_bHaveMemento)
	{
		Count(error);
	}

	u32 symSize = 0xffffffff;
	u32 symTotal = 0xffffffff;

#if ENABLE_ACCURATE_BANDWIDTH_PROFILING	
	float s1 = stm.GetBitSize();
#endif

	if (m_bHaveMemento)
	{
		if (!m_errorDistributionWrite.WriteValue(error, &stm, m_lastTimeZero != 0))
			stm.WriteBits(quantized, m_quantizer.GetNumBits());

		m_errorDistributionWrite.GetDebug(symSize, symTotal);
	}
	else
		stm.WriteBits(quantized, m_quantizer.GetNumBits());

#if ENABLE_ACCURATE_BANDWIDTH_PROFILING
	if (m_bHaveMemento)
	{
		float s2 = stm.GetBitSize();
		Track(quantized, predicted, mementoAge, timeFraction32, s2 - s1, symSize, symTotal);
	}
#endif

	Update(quantized, predicted, mementoAge, timeFraction32);
}

bool CPredictiveFloat::ReadValue(CCommInputStream& stm, float& value, u32 mementoAge, u32 timeFraction32) const
{
	i32 prediction = Predict(timeFraction32);
	prediction = clamp_tpl(prediction, 0, (i32)m_quantizer.GetMaxQuantizedValue());

	u32 quantized;

	if (m_bHaveMemento)
	{
		i32 error = 0;

		if (m_errorDistributionRead.ReadValue(error, &stm, m_lastTimeZero != 0))
			quantized = prediction + error;
		else
			quantized = stm.ReadBits(m_quantizer.GetNumBits());
	}
	else
		quantized = stm.ReadBits(m_quantizer.GetNumBits());

	Update(quantized, prediction, mementoAge, timeFraction32);

	value = m_quantizer.Dequantize(quantized);

	NetLogPacketDebug("CPredictiveFloat::ReadValue %f Min %f Max %f NumBits %d (%f)", value, m_quantizer.GetMinValue(), m_quantizer.GetMaxValue(), m_quantizer.GetNumBits(), stm.GetBitSize());
	return true;
}
#else
void CPredictiveFloat::WriteValue(CNetOutputSerializeImpl* stm, float value, u32 mementoAge, u32 timeFraction32) const
{
	u32 quantized = m_quantizer.Quantize(value);

	stm->WriteBits(quantized, m_quantizer.GetNumBits());
}

bool CPredictiveFloat::ReadValue(CNetInputSerializeImpl* stm, float& value, u32 mementoAge, u32 timeFraction32) const
{
	u32 quantized = stm->ReadBits(m_quantizer.GetNumBits());

	value = m_quantizer.Dequantize(quantized);
	NetLogPacketDebug("m_accurateSize::ReadValue %f Min %f Max %f NumBits %d (%f)", value, m_quantizer.GetMinValue(), m_quantizer.GetMaxValue(), m_quantizer.GetNumBits(), stm->GetBitSize());

	return true;
}
#endif
#if NET_PROFILE_ENABLE
i32 CPredictiveFloat::GetBitCount()
{
	return m_quantizer.GetNumBits();
}
#endif
