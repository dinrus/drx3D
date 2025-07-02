// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ICompressionPolicy.h>
#include  <drx3D/Network/ArithModel.h>
#include  <drx3D/Network/AdaptiveFloat.h>

class CAdaptiveFloatPolicy
{
public:
	bool Load(XmlNodeRef node, const string& filename)
	{
		return m_float.Load(node, filename, "Params");
	}
#if USE_MEMENTO_PREDICTORS
	bool ReadMemento(CByteInputStream& in) const
	{
		m_float.ReadMemento(in);
		return true;
	}

	bool WriteMemento(CByteOutputStream& out) const
	{
		m_float.WriteMemento(out);
		return true;
	}

	void NoMemento() const
	{
		m_float.NoMemento();
	}
#endif

#if USE_ARITHSTREAM
	bool ReadValue(CCommInputStream& in, float& value, CArithModel* pModel, u32 age) const
	{
		return m_float.ReadValue(in, value, age);
	}
	bool WriteValue(CCommOutputStream& out, float value, CArithModel* pModel, u32 age) const
	{
		m_float.WriteValue(out, value, age);
		return true;
	}

	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		NetWarning("AdaptiveFloatPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		NetWarning("AdaptiveFloatPolicy: not implemented for generic types");
		return false;
	}
#else
	bool ReadValue(CNetInputSerializeImpl* in, float& value, u32 age) const
	{
		return m_float.ReadValue(in, value, age);
	}
	bool WriteValue(CNetOutputSerializeImpl* out, float value, u32 age) const
	{
		m_float.WriteValue(out, value, age);
		return true;
	}

	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, T& value, u32 age) const
	{
		NetWarning("AdaptiveFloatPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, T value, u32 age) const
	{
		NetWarning("AdaptiveFloatPolicy: not implemented for generic types");
		return false;
	}
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CAdaptiveFloatPolicy");
		pSizer->Add(*this);
	}

#if NET_PROFILE_ENABLE
	i32 GetBitCount(float value)
	{
		return m_float.GetBitCount();
	}

	template<class T>
	i32 GetBitCount(T value)
	{
		return 0;
	}
#endif

private:
	CAdaptiveFloat m_float;
};

REGISTER_COMPRESSION_POLICY(CAdaptiveFloatPolicy, "AdaptiveFloat");
