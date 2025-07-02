// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Network/PredictiveBase.h>
#include <drx3D/Network/TimePolicy.h>

class CTimePolicyWithDistribution
	: public CTimePolicy
{
public:
	void SetTimeValue(u32 timeFraction32);
	bool Manage(CCompressionUpr* pUpr);

	void Init(CCompressionUpr* pUpr);
	bool Load(XmlNodeRef node, const string& filename);

#if USE_ARITHSTREAM
	bool ReadValue(CCommInputStream& in, CTimeValue& value, CArithModel* pModel, u32 age) const;
	bool WriteValue(CCommOutputStream& out, CTimeValue value, CArithModel* pModel, u32 age) const;

	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		NetWarning("CTimePolicyWithDistribution: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		NetWarning("CTimePolicyWithDistribution: not implemented for generic types");
		return false;
	}
#endif

private:
	string m_name;

#if USE_ARITHSTREAM
	CPredictiveBase<int64> m_dist[2];
#endif
};
