// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   CIntegerValuePredictor
   Описание:  Predicts the range in which the next integer in a continuous
   row could be positioned.
   This class is meant to be used as a Memento with the Float Serialization
   Policy and thus is simplyfied and shrinked.
   -------------------------------------------------------------------------
   История:
   - 14/11/2005   18:45 : Created by Jan Müller
*************************************************************************/

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/IntegerValuePredictor.h>
#include  <drx3D/Network/ArithStream.h>

#if USE_MEMENTO_PREDICTORS

CIntegerValuePredictor::CIntegerValuePredictor(i32k startValue)
{
	m_lastValue = startValue;
	m_delta = 0;
	m_off = 0;
	m_belowMinimum = 0;

	m_totalError = 0;
	m_totalNum = 0;
	m_lastTime32 = 0;
	m_predictorMode = ePredictorMode_Age;
}

void CIntegerValuePredictor::SetMode(EPredictorMode mode)
{
	m_predictorMode = mode;
}

void CIntegerValuePredictor::Update(i32 value, i32k prediction, i32 mementoAge, u32 time)
{
	i32 delta = abs(prediction - value);

	if (m_predictorMode == ePredictorMode_Time)
		UpdateTime(value, prediction, mementoAge, time);
	else
		UpdateAge(value, prediction, mementoAge, time);
}

void CIntegerValuePredictor::UpdateTime(i32 value, i32k prediction, i32 mementoAge, u32 timeFraction32)
{
	//can be zero if we have no memento's so far
	if(mementoAge == 0)
		mementoAge = 1;

	i32 timeDelta = timeFraction32 - m_lastTime32;

	m_lastTime32 = timeFraction32;

	//did we hit or miss?
	bool lastHit = false;
	if(prediction - m_off <= value && prediction + m_off >= value)
		lastHit = true; 

	//compute the mis-prediction and update offset with it
	i32 tempOff = abs(value - prediction);
	if(tempOff + m_off/2 < 65536)
		m_off = (u16)(tempOff + m_off/2);
	else
		m_off = 65535;

	//increase range additionally when last prediction did not hit
	if(!lastHit && u32(m_off) * 2 < 65536)
		m_off *= 2;


	if (timeDelta)
	{
		i32 delta = 0;
		i32 diff = (value - m_lastValue) * s_deltaPrecision;

		delta = diff / timeDelta;
		if (abs(delta) > 60 * s_deltaPrecision && abs(m_delta) < 10 * s_deltaPrecision)
			m_delta = 0;
		else if (diff == 0)
			m_delta = 0;
		else
			m_delta = delta + (delta - m_delta) / 3;
	}
	else
		m_delta /= 2;

	m_lastValue = value;
}

void CIntegerValuePredictor::UpdateAge(i32 value, i32k prediction, i32 mementoAge, u32 timeFraction32)
{
	//can be zero if we have no memento's so far
	if (mementoAge == 0)
		mementoAge = 1;

	m_lastTime32 = timeFraction32;

	//did we hit or miss?
	bool lastHit = false;
	if (prediction - m_off <= value && prediction + m_off >= value)
		lastHit = true;

	//compute the mis-prediction and update offset with it
	i32 tempOff = abs(value - prediction);
	if (tempOff + m_off / 2 < 65536)
		m_off = (u16)(tempOff + m_off / 2);
	else
		m_off = 65535;

	//increase range additionally when last prediction did not hit
	if (!lastHit && u32(m_off) * 2 < 65536)
		m_off *= 2;

	i32 delta = 0;
	i32 diff = value - m_lastValue;
	if (diff != 0)
	{
		if (mementoAge < 4 && (diff < 0 && m_delta > 0 || diff > 0 && m_delta < 0))    //little tweak: if there was a change in direction, interpolate less
			delta = ((diff / mementoAge) + 3 * m_delta) / i32(4);
		else
			delta = ((diff / mementoAge) + 7 * m_delta) / i32(8);
		//check for overflow
		if (delta > 32767)
			delta = 32767;
		else if (delta < -32768)
			delta = -32768;
	}

	m_delta = delta;

	m_lastValue = value;
}

i32 CIntegerValuePredictor::Predict(u32& left, u32& right, u32 avgHeight, i32 minRange,
                                      i32 maxQuantizedValue, i32 maxDifference, i32 mementoAge, u32 timeFraction32)
{
	if (m_predictorMode == ePredictorMode_Time)
		return PredictTime(left, right, avgHeight, minRange, maxQuantizedValue, maxDifference, mementoAge, timeFraction32);
	else
		return PredictAge(left, right, avgHeight, minRange, maxQuantizedValue, maxDifference, mementoAge, timeFraction32);
}

i32 CIntegerValuePredictor::PredictAge(u32& left, u32& right, u32 avgHeight, i32 minRange,
  i32 maxQuantizedValue, i32 maxDifference, i32 mementoAge, u32 timeFraction32)
{
	//can be zero if we have no memento's so far
	if (mementoAge == 0)
		mementoAge = 1;

	i32 prediction = 0;
	//compute the prediction range
	u32 range = (u32)m_off;

	if (range)
	{
		//add something for old mementos
		if (range < uint64(minRange) * 3 / 2)
		{
			for (i32 j = 1; j < mementoAge && j < 5; j++)
			{
				u32 oldRange = range;
				range = (u32)CLAMP(uint64(range) * uint64(6) / uint64(5), uint64(0), uint64(~u32(0)));
				NET_ASSERT(range >= oldRange);
			}
		}

		//don't use less than minRange (but when there is no movement)
		if ((i32)range < minRange)
		{
			if ((i32)range < minRange / 2)
				m_belowMinimum++;
			else
				m_belowMinimum = 0;

			if (m_belowMinimum > 7)
			{
				range = (u32)(minRange / 2);
				m_belowMinimum = 7; //clamp it for serialization
			}
			else
				range = minRange;
		}
		else if (maxDifference && (i32)range < maxDifference)
			m_belowMinimum = 0;
		else if (maxDifference)
			range = maxDifference;

		//check for range*height explosion -> too high ranges kill the stream
		NET_ASSERT(uint64(range) * uint64(avgHeight) < (uint64(2147483647) - avgHeight - maxQuantizedValue));
	}
	else
		m_belowMinimum = 0;

	//predict the next value - regarding the memento age - and clamp it to quantized range
	if (m_lastValue + (m_delta * mementoAge) < maxQuantizedValue)
		prediction = m_lastValue + (m_delta * mementoAge);
	else
		prediction = maxQuantizedValue;
	if (prediction < 0)
		prediction = 0;

	//now the bottom and top borders of the range are set and clamped
	if (int64(prediction) - range > 0)
		left = (u32)(prediction - range);
	else
		left = 0;
	if (int64(prediction) + range < maxQuantizedValue)
		right = (u32)(prediction + range);
	else
		right = maxQuantizedValue;

	//debug
	NET_ASSERT(right >= left);

	return prediction;
}

i32 CIntegerValuePredictor::PredictTime(u32 &left, u32 &right, u32 avgHeight, i32 minRange,
																			i32 maxQuantizedValue, i32 maxDifference, i32 mementoAge, u32 timeFraction32)
{
	//can be zero if we have no memento's so far
	if(mementoAge == 0)
		mementoAge = 1;

	i32 timeDelta = timeFraction32 - m_lastTime32;

	i32	prediction = 0;
	//compute the prediction range
	u32 range = (u32)m_off;

	if(range)
	{
		//add something for old mementos
		if(range < uint64(minRange) * 3 / 2)
		{
			for(i32 j = 1; j < mementoAge && j < 5; j++)
			{
				u32 oldRange = range;
				range = (u32)CLAMP( uint64(range) * uint64(6) / uint64(5), uint64(0), uint64(~u32(0)) );
				NET_ASSERT(range >= oldRange);
			}
		}

		//don't use less than minRange (but when there is no movement)
		if((i32)range < minRange)
		{
			if((i32)range < minRange/2)
				m_belowMinimum++;
			else
				m_belowMinimum = 0;

			if(m_belowMinimum > 7)
			{
				range = (u32)(minRange / 2);
				m_belowMinimum = 7;	//clamp it for serialization
			}
			else
				range = minRange;
		}
		else if(maxDifference && (i32)range < maxDifference)
			m_belowMinimum = 0;
		else if(maxDifference)
			range = maxDifference;

		//check for range*height explosion -> too high ranges kill the stream
		NET_ASSERT(uint64(range) * uint64(avgHeight) < (uint64(2147483647) - avgHeight - maxQuantizedValue));
	}
	else
		m_belowMinimum = 0;

	//predict the next value - regarding the memento age - and clamp it to quantized range
	if(m_lastValue + (m_delta * timeDelta / s_deltaPrecision) < maxQuantizedValue)
		prediction = m_lastValue + (m_delta * timeDelta / s_deltaPrecision);
	else
		prediction = maxQuantizedValue;
	if(prediction < 0)
		prediction = 0;

	//now the bottom and top borders of the range are set and clamped
	if(int64(prediction) - range > 0)
		left = (u32)(prediction - range);
	else
		left = 0;
	if(int64(prediction) + range < maxQuantizedValue)
		right = (u32)(prediction + range);
	else
		right = maxQuantizedValue;

	//debug
	NET_ASSERT (right >= left);

	return prediction;
}

namespace
{
struct SAlignedData
{
	i16  delta;
	u32 lastTime;
	u16 off;
	i32  lastValue;
};
}

void CIntegerValuePredictor::Serialize(class CByteOutputStream& stm)
{
	//	NetLog("CIVP: %d %d %d %d", m_delta, m_off, m_lastValue, m_belowMinimum);

	SAlignedData ad = { (i16)m_delta, m_lastTime32, m_off, m_lastValue };
	stm.PutTyped<SAlignedData>() = ad;

	NET_ASSERT(this->m_lastValue >= 0);

	stm.PutTyped<u8>() = m_belowMinimum;
}

void CIntegerValuePredictor::Deserialize(class CByteInputStream& stm)
{
	const SAlignedData& ad = stm.GetTyped<SAlignedData>();
	this->m_delta = ad.delta;
	this->m_lastTime32 = ad.lastTime;
	this->m_off = ad.off;
	this->m_lastValue = ad.lastValue;

	this->m_belowMinimum = stm.GetTyped<u8>();
}

#endif
