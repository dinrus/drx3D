// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ADAPTIVEFLOAT_H__
#define __ADAPTIVEFLOAT_H__

#pragma once

#include <drx3D/Network/Quantizer.h>
#include <drx3D/Network/IntegerValuePredictor.h>
#include <drx3D/Network/CommStream.h>

class CNetInputSerializeImpl;
class CNetOutputSerializeImpl;

class CAdaptiveFloat
{
public:
	CAdaptiveFloat();

	void Init(tukk szName, u32 channel, tukk szUseDir, tukk szAccDir) {}
	bool Manage(CCompressionUpr* pUpr, tukk szPolicy, i32 channel) { return false; }

	bool Load(XmlNodeRef node, const string& filename, const string& child);
#if USE_MEMENTO_PREDICTORS
	void ReadMemento(CByteInputStream& stm) const;
	void WriteMemento(CByteOutputStream& stm) const;
	void NoMemento() const;
#endif
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
	CQuantizer                     m_quantizer;
#if USE_MEMENTO_PREDICTORS
	u32                         m_nHeight;
	u32                         m_nQuantizedMinDifference;
	u32                         m_nQuantizedMaxDifference;
	u32                         m_nQuantizedStartValue;
	u8                          m_nInRangePercentage;

	mutable CIntegerValuePredictor m_predictor;
	mutable bool                   m_haveMemento;
#endif
};

#endif
