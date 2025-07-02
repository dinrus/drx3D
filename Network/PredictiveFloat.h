// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Network/Quantizer.h>
#include <drx3D/Network/CommStream.h>

#include <drx3D/Network/PredictiveBase.h>

class CNetInputSerializeImpl;
class CNetOutputSerializeImpl;
class CPredictiveFloatTracker;

class CPredictiveFloat :
	public CPredictiveBase<i32>
{
public:
	bool Load(XmlNodeRef node, const string& filename, const string& child);

#if USE_ARITHSTREAM
	bool ReadValue(CCommInputStream& stm, float& value, u32 mementoAge, u32 timeFraction32 = 0) const;
	void WriteValue(CCommOutputStream& stm, float value, u32 mementoAge, u32 timeFraction32 = 0) const;
#else
	bool ReadValue(CNetInputSerializeImpl* stm, float& value, u32 mementoAge, u32 timeFraction32 = 0) const;
	void WriteValue(CNetOutputSerializeImpl* stm, float value, u32 mementoAge, u32 timeFraction32 = 0) const;
#endif

#if NET_PROFILE_ENABLE
	i32 GetBitCount();
#endif

private:
	CQuantizer m_quantizer;
};
