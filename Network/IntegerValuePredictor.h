// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   CIntegerValuePredictor
   Описание:  Predicts the range in which the next integer in a continuous
   queue could be positioned based on curve gradient analysis.
   This class is meant to be used as a Memento with the Float Serialization
   Policy and thus is simplyfied and shrinked.
   It is limited to 16 Bit differences and is vulnerable for error values.
   It can't work on frame-differences larger than 16 Bit.
   -------------------------------------------------------------------------
   История:
   - 14/11/2005   18:45 : Created by Jan Müller
*************************************************************************/

#ifndef INTEGER_VALUE_PREDICTOR
#define INTEGER_VALUE_PREDICTOR

#pragma once
#include <drx3D/Network/Config.h>

#if USE_MEMENTO_PREDICTORS

	#include <drx3D/Network/ByteStream.h>

class CIntegerValuePredictor
{
public:
	enum EPredictorMode
	{
		ePredictorMode_Time = 0,
		ePredictorMode_Age,
		ePredictorMode_Num
	};

public:
	CIntegerValuePredictor(i32k startValue = 0);

	void SetMode(EPredictorMode mode);

	/************************************************************************/
	/* Updates the predictor with the real value. (after prediction)        */
	/************************************************************************/
	void Update(i32 value, i32k prediction, i32 mementoAge, u32 time = 0);
	void UpdateTime(i32 value, i32k prediction, i32 mementoAge, u32 time = 0);
	void UpdateAge(i32 value, i32k prediction, i32 mementoAge, u32 time = 0);

	/************************************************************************/
	/* Predicts a range in which the next value in a continuous integer
	   queue could probably be positioned.
	   @param	delta is the return of the update() after the last prediction */
	/************************************************************************/
	i32        Predict(u32& left, u32& right, u32 avgHeight, i32 minRange, i32 maxQuantizedValue, i32 maxDifference = 0, i32 mementoAge = 1, u32 time = 0);
	i32        PredictTime(u32& left, u32& right, u32 avgHeight, i32 minRange, i32 maxQuantizedValue, i32 maxDifference = 0, i32 mementoAge = 1, u32 time = 0);
	i32        PredictAge(u32& left, u32& right, u32 avgHeight, i32 minRange, i32 maxQuantizedValue, i32 maxDifference = 0, i32 mementoAge = 1, u32 time = 0);

	ILINE u32 GetOff() const
	{
		return (u32)m_off;
	}

	ILINE i32 GetDelta() const
	{
		return (i32)m_delta;
	}

	ILINE i32 GetLastValue() const
	{
		return m_lastValue;
	}

	bool IsTimePredictor() const
	{
		return m_predictorMode == ePredictorMode_Time;
	}

	//don't call this after first prediction ...!
	ILINE void Clear(i32 startValue)
	{
		m_lastValue = startValue;
		m_delta = 0;
		m_off = 0;
		m_belowMinimum = 0;
		m_lastTime32 = 0;
	}

	//this function writes the memento to the stream
	void Serialize(class CByteOutputStream& stm);

	//this function reads a memento from the stream
	void Deserialize(class CByteInputStream& stm);

private:
	//this are the necessary member variables for this predictor/memento

	//this is the time of previous prediction
	u32 m_lastTime32;
	//this is the delta - the predicted "gradient" for the next value
	i32 m_delta;
	//this is the saved differences between predicted and real values
	u16 m_off;
	//this is the last value
	i32  m_lastValue;
	//this counter keeps track of very small offs to reduce the minimum range
	u8 m_belowMinimum;

	EPredictorMode m_predictorMode;

	uint64 m_totalError;
	u32 m_totalNum;
	static u32k s_deltaPrecision = 5000;
};

#endif

#endif
