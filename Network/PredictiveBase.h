// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Network/ErrorDistributionEncoding.h>
#include <drx3D/Network/PredictiveFloatTracker.h>

template<typename T>
class CPredictiveBase
{
public:
#if USE_MEMENTO_PREDICTORS
	CPredictiveBase()
		: m_bHaveMemento(false)
	{
		m_countingEnabled = false;
		m_deltaPrecision = 5000;
		m_pTracker = nullptr;
		m_countBits = 14;
		m_symbolBits = 10;
		m_bitBucketSize = 2;
		m_continuityThreshold = 0;
	}

	~CPredictiveBase()
	{
		delete m_pTracker;
		m_pTracker = 0;
	}
#endif

	void Init(tukk name, u32 channel, tukk szUseDir, tukk szAccDir)
	{
#if USE_MEMENTO_PREDICTORS
		m_errorDistributionRead.Init(name, channel, m_symbolBits, m_bitBucketSize, szUseDir, eDistributionType_Read);
		m_errorDistributionWrite.Init(name, channel, m_symbolBits, m_bitBucketSize, szUseDir, eDistributionType_Write);

		m_errorDistributionCount.Init(name, channel, m_countBits, m_bitBucketSize, szAccDir, eDistributionType_Count);
		m_errorDistributionCount.Clear();
#endif
	}

	void Count(T error) const
	{
#if USE_MEMENTO_PREDICTORS
		if (m_countingEnabled)
		{
			DrxAutoLock<DrxMutex> lock(m_mutex);
			if (m_lastTimeZero != 0)
				m_errorDistributionCount.NotifyAfterZero((i32)error);

			if (m_lastTimeZero == 0 || error != 0)
				m_errorDistributionCount.CountError((i32)error);
		}
#endif
	}

	void ReadMemento(CByteInputStream& stm) const
	{
#if USE_MEMENTO_PREDICTORS
		m_lastValue = stm.GetTyped<T>();
		m_delta = stm.GetTyped<T>();
		m_lastTime32 = stm.GetTyped<u32>();
		m_lastTimeZero = stm.GetTyped<u8>();

		if (m_pTracker)
		{
			m_mementoId = stm.GetTyped<u32>();
			m_mementoSequenceId = stm.GetTyped<u32>();
		}

		m_bHaveMemento = true;
#endif
	}

	void WriteMemento(CByteOutputStream& stm) const
	{
#if USE_MEMENTO_PREDICTORS
		stm.PutTyped<T>() = m_lastValue;
		stm.PutTyped<T>() = m_delta;
		stm.PutTyped<u32>() = m_lastTime32;
		stm.PutTyped<u8>() = m_lastTimeZero;

		if (m_pTracker)
		{
			stm.PutTyped<u32>() = m_mementoId;
			stm.PutTyped<u32>() = m_mementoSequenceId;
		}

		m_bHaveMemento = true;
#endif
	}

	void NoMemento() const
	{
#if USE_MEMENTO_PREDICTORS
		m_lastValue = 0;
		m_delta = 0;
		m_lastTime32 = 0;
		m_lastTimeZero = 0;

		static i32 nextMementoId = 1;
		m_mementoId = nextMementoId++;
		m_mementoSequenceId = 0;

		m_bHaveMemento = false;
#endif
	}

	void Load(XmlNodeRef params, float quantizationScale)
	{
#if USE_MEMENTO_PREDICTORS
		u32 countBits = 14;
		u32 symbolBits = 10;
		u32 bitBucketSize = 2;
		u32 deltaPrecision = 5000;
		u32 continuityThreshold = 0;
		u32 orientationMode = 0;

		i32 trackValues = 0;

		if (gEnv->IsDedicated())
		{
			string dir = params->getAttr("valueTrackDirectory");
			if (!dir.empty())
				m_pTracker = new CPredictiveFloatTracker(dir, 0, quantizationScale);
		}

		if (params->getAttr("continuityThreshold", continuityThreshold))
			m_continuityThreshold = continuityThreshold;

		if (params->getAttr("deltaPrecision", deltaPrecision))
			m_deltaPrecision = deltaPrecision;

		if (params->getAttr("countBits", countBits))
			m_countBits = countBits;

		if (params->getAttr("symbolBits", symbolBits))
			m_symbolBits = symbolBits;

		if (params->getAttr("bitBucketSize", bitBucketSize))
			m_bitBucketSize = bitBucketSize;
#endif
	}

	void Update(T value, T prediction, i32 mementoAge, u32 timeFraction32) const
	{
#if USE_MEMENTO_PREDICTORS
		if (value == prediction)
			m_lastTimeZero = 1;
		else
			m_lastTimeZero = 0;

		i32 timeDelta = timeFraction32 - m_lastTime32;

		if (m_lastTime32 == 0)
			timeDelta = 100;

		m_lastTime32 = timeFraction32;

		if (timeDelta)
		{
			T delta = 0;
			T diff = (value - m_lastValue);

			if (m_continuityThreshold != 0 && abs(diff) > (T)(m_continuityThreshold * mementoAge))
				m_delta = 0;
			else
			{
				diff *= m_deltaPrecision;

				delta = diff / timeDelta;
				if (abs(delta) > 60 * m_deltaPrecision && abs(m_delta) < 10 * m_deltaPrecision) //optimisation for value switching instead continuus change
					m_delta = 0;
				else if (diff == 0) //if value stopped, it will probably not move in next step also
					m_delta = 0;
				else //take in to account delta of delta to exploit higher order continuity of variables
					m_delta = delta + (delta - m_delta) / 3;
			}
		}
		else
			m_delta /= 2;

		m_lastValue = value;
#endif
	}

	T Predict(u32 timeFraction32) const
	{
#if USE_MEMENTO_PREDICTORS
		//can be zero if we have no memento's so far
		i32 timeDelta = (i32)(timeFraction32 - m_lastTime32);

		T frac2 = ((m_delta * timeDelta) % m_deltaPrecision) * 2;

		T prediction = m_lastValue + (m_delta * timeDelta / m_deltaPrecision);

/*		if (frac2 < -m_deltaPrecision)
			prediction--;
		else if (frac2 > m_deltaPrecision)
			prediction++;*/

		return prediction;
#else
		return 0;
#endif
	}

	const CErrorDistribution* GetReadDistribution() const
	{
#if USE_MEMENTO_PREDICTORS
		return &m_errorDistributionRead;
#else
		static CErrorDistribution null;
		return &null;
#endif
	}

	const CErrorDistribution* GetWriteDistribution() const
	{
#if USE_MEMENTO_PREDICTORS
		return &m_errorDistributionWrite;
#else
		static CErrorDistribution null;
		return &null;
#endif
	}

	bool Manage(CCompressionUpr* pUpr, tukk szPolicy, i32 channel)
	{
#if USE_MEMENTO_PREDICTORS
		m_countingEnabled = true;

		if (!m_errorDistributionCount.IsEmpty())
		{
			CErrorDistribution acc;
			m_errorDistributionCount.AccumulateInit(acc, pUpr->GetAccDirectory().c_str());

			{
				DrxAutoLock<DrxMutex> lock(m_mutex);
				m_errorDistributionCount.Accumulate(acc);
			}

			acc.SaveJson(pUpr->GetAccDirectory().c_str());
		}

		if (m_pTracker)
			m_pTracker->Manage(szPolicy, channel);

		return true;
#else
		return false;
#endif
	}

	void Track(i32 quantized, i32 predicted, u32 mementoAge, u32 timeFraction32, float size, u32 symSize, u32 totalSize) const
	{
#if USE_MEMENTO_PREDICTORS
		if (m_pTracker)
			m_pTracker->Track(m_mementoId, m_mementoSequenceId, (i32)m_lastValue, (i32)m_delta, m_lastTime32, quantized, predicted, mementoAge, timeFraction32, size, symSize, totalSize);
#endif
	}

#if USE_MEMENTO_PREDICTORS
protected:
	CErrorDistribution m_errorDistributionRead;
	CErrorDistribution m_errorDistributionWrite;

	mutable DrxMutex m_mutex;
	mutable CErrorDistribution m_errorDistributionCount;

	mutable CPredictiveFloatTracker* m_pTracker;

	mutable bool m_bHaveMemento;

	//this two variables are used in tracking, they are not written to memento when tracking is disabled
	mutable u32 m_mementoId;
	mutable u32 m_mementoSequenceId;

	u32 m_continuityThreshold;
	bool m_countingEnabled;

	//this is the time of previous prediction
	mutable u32 m_lastTime32;
	//this is the delta - the predicted "gradient" for the next value
	mutable T m_delta;
	//this is the last value
	mutable T m_lastValue;
	//this indicates that last prediction had error = 0
	mutable u8 m_lastTimeZero;

	u32 m_bitBucketSize;
	u32 m_symbolBits;
	u32 m_countBits;

	T m_deltaPrecision;
#endif
};

