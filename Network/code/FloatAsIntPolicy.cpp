// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ICompressionPolicy.h>
#include  <drx3D/Network/ArithModel.h>
#include  <drx3D/Network/Quantizer.h>
#include  <drx3D/Network/StationaryInteger.h>

class CFloatAsIntPolicy
{
public:
	bool Load(XmlNodeRef node, const string& filename)
	{
		bool ok = m_quantizer.Load(node, filename);
		if (ok && m_quantizer.GetNumBits() > 31)
			ok = false;
		if (ok)
			m_integer.SetValues(0, m_quantizer.GetMaxQuantizedValue());
		return ok;
	}

#if USE_MEMENTO_PREDICTORS
	bool ReadMemento(CByteInputStream& in) const
	{
		m_integer.ReadMemento(in);
		return true;
	}

	bool WriteMemento(CByteOutputStream& out) const
	{
		m_integer.WriteMemento(out);
		return true;
	}

	void NoMemento() const
	{
		m_integer.NoMemento();
	}
#endif

#if USE_ARITHSTREAM
	bool ReadValue(CCommInputStream& in, float& value, CArithModel* pModel, u32 age) const
	{
		u32 q = m_integer.ReadValue(in);
		value = m_quantizer.Dequantize(q);
		NetLogPacketDebug("CFloatAsIntPolicy::ReadValue Previously Read (CStationaryInteger) %f Min %f Max %f NumBits %d  (%f)", value, m_quantizer.GetMinValue(), m_quantizer.GetMaxValue(), m_quantizer.GetNumBits(), in.GetBitSize());
		return true;
	}
	bool WriteValue(CCommOutputStream& out, float value, CArithModel* pModel, u32 age) const
	{
		u32 q = m_quantizer.Quantize(value);
		m_integer.WriteValue(out, q);
		return true;
	}

	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		NetWarning("FloatAsIntPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		NetWarning("FloatAsIntPolicy: not implemented for generic types");
		return false;
	}
#else
	bool ReadValue(CNetInputSerializeImpl* in, float& value, u32 age) const
	{
		u32 q = m_integer.ReadValue(in);
		value = m_quantizer.Dequantize(q);
		NetLogPacketDebug("CFloatAsIntPolicy::ReadValue Previously Read (CStationaryInteger) %f Min %f Max %f NumBits %d  (%f)", value, m_quantizer.GetMinValue(), m_quantizer.GetMaxValue(), m_quantizer.GetNumBits(), in->GetBitSize());
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, float value, u32 age) const
	{
		u32 q = m_quantizer.Quantize(value);
		m_integer.WriteValue(out, q);
		return true;
	}

	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, T& value, u32 age) const
	{
		NetWarning("FloatAsIntPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, T value, u32 age) const
	{
		NetWarning("FloatAsIntPolicy: not implemented for generic types");
		return false;
	}
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CFloatAsIntPolicy");
		pSizer->Add(*this);
	}
#if NET_PROFILE_ENABLE
	i32 GetBitCount(float value)
	{
		return m_integer.GetBitCount();
	}

	template<class T>
	i32 GetBitCount(T value)
	{
		return 0;
	}
#endif
private:
	CQuantizer         m_quantizer;
	CStationaryInteger m_integer;
};

REGISTER_COMPRESSION_POLICY(CFloatAsIntPolicy, "FloatAsInt");
