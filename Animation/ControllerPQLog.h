// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Eng3D/CGFContent.h>

class CControllerPQLog : public IController
{
public:

	/**
	 * Default constructor.
	 */
	CControllerPQLog();

	/**
	 * Assigns a data payload to this controller object.
	 */
	void SetControllerData(const DynArray<PQLogS>& arrKeys, const DynArray<i32>& arrTimes)
	{
		m_arrKeys = arrKeys;
		m_arrTimes = arrTimes;
	}

	//////////////////////////////////////////////////////////
	// IController implementation
	virtual JointState GetOPS(f32 key, Quat& quat, Vec3& pos, Diag33& scl) const override;
	virtual JointState GetOP(f32 key, Quat& quat, Vec3& pos) const override;
	virtual JointState GetO(f32 key, Quat& quat) const override;
	virtual JointState GetP(f32 key, Vec3& pos) const override;
	virtual JointState GetS(f32 key, Diag33& scl) const override;
	virtual i32      GetO_numKey() const override;
	virtual i32      GetP_numKey() const override;
	virtual size_t     GetRotationKeysNum() const override;
	virtual size_t     GetPositionKeysNum() const override;
	virtual size_t     GetScaleKeysNum() const override;
	virtual size_t     SizeOfController() const override;
	virtual size_t     ApproximateSizeOfThis() const override;
	//////////////////////////////////////////////////////////

private:

	const QuatTNS& DecodeKey(f32 keyTime) const;

public:

	// TODO: make these members private
	DynArray<PQLogS> m_arrKeys;
	DynArray<i32>    m_arrTimes;

private:

	mutable float   m_lastTime;
	mutable QuatTNS m_lastValue;
};

TYPEDEF_AUTOPTR(CControllerPQLog);

inline const QuatTNS& CControllerPQLog::DecodeKey(f32 keyTime) const
{
#ifdef DEFINE_PROFILER_FUNCTION
	DEFINE_PROFILER_FUNCTION();
#endif

	if (keyTime == m_lastTime)
	{
		return m_lastValue;
	}
	m_lastTime = keyTime;

	// Use local variables to spare checking the this pointer everytime
	const PQLogS* const __restrict arrKeys = &m_arrKeys[0];
	i32k* const __restrict arrTimes = &m_arrTimes[0];
	i32k nNumKeysPQ = m_arrKeys.size();

	assert(m_arrKeys.size() > 0);

	const f32 keytimeStart = (f32)arrTimes[0];
	if (keyTime < keytimeStart)
	{
		const PQLogS& value = arrKeys[0];
		m_lastValue = QuatTNS(!Quat::exp(value.rotationLog), value.position, value.scale);
		return m_lastValue;
	}

	const f32 keytimeEnd = (f32)arrTimes[nNumKeysPQ - 1];
	if (keyTime >= keytimeEnd)
	{
		const PQLogS& value = arrKeys[nNumKeysPQ - 1];
		m_lastValue = QuatTNS(!Quat::exp(value.rotationLog), value.position, value.scale);
		return m_lastValue;
	}

	assert(m_arrKeys.size() > 1);

	i32 nPos = nNumKeysPQ >> 1;
	i32 nStep = nNumKeysPQ >> 2;

	// use binary search
	while (nStep)
	{
		i32k time = arrTimes[nPos];
		if (keyTime < time)
		{
			nPos = nPos - nStep;
		}
		else if (keyTime > time)
		{
			nPos = nPos + nStep;
		}
		else
		{
			break;
		}
		nStep = nStep >> 1;
	}

	// fine-tuning needed since time is not linear
	while (keyTime > arrTimes[nPos])
	{
		nPos++;
	}

	while (keyTime < arrTimes[nPos - 1])
	{
		nPos--;
	}

	assert(nPos > 0 && nPos < (i32)nNumKeysPQ);

	PQLogS pKeys[2] = { arrKeys[nPos - 1], arrKeys[nPos] };

	i32k k0 = arrTimes[nPos];
	i32k k1 = arrTimes[nPos - 1];

	if (k0 == k1)
	{
		m_lastValue = QuatTNS(!Quat::exp(pKeys[1].rotationLog), pKeys[1].position, pKeys[1].scale);
		return m_lastValue;
	}

	AdjustLogRotations(pKeys[0].rotationLog, pKeys[1].rotationLog);

	PQLogS value;
	const f32 blendFactor = (f32(keyTime - k1)) / (k0 - k1);
	value.Blend(pKeys[0], pKeys[1], blendFactor);

	m_lastValue = QuatTNS(!Quat::exp(value.rotationLog), value.position, value.scale);
	return m_lastValue;
}
