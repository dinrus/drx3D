// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/CompressionPolicyTime.h>
#include  <drx3D/Network/ArithModel.h>
#include  <drx3D/Network/AdaptiveFloat.h>
#include  <drx3D/Network/PredictiveFloat.h>

template<typename FloatType>
class CAdaptiveVec3PolicyT
{
public:
	CAdaptiveVec3PolicyT()
	{
		m_timeFraction32 = 0;
	}

	bool Load(XmlNodeRef node, const string& filename)
	{
		m_name = node->getAttr("name");
		bool ret =  
		  m_floats[0].Load(node, filename, "XParams") &&
		  m_floats[1].Load(node, filename, "YParams") &&
		  m_floats[2].Load(node, filename, "ZParams");

		return ret;
	}

#if USE_MEMENTO_PREDICTORS
	bool ReadMemento(CByteInputStream& in) const
	{
		for (i32 i = 0; i < 3; i++)
			m_floats[i].ReadMemento(in);
		return true;
	}

	bool WriteMemento(CByteOutputStream& out) const
	{
		for (i32 i = 0; i < 3; i++)
			m_floats[i].WriteMemento(out);
		return true;
	}

	void NoMemento() const
	{
		for (i32 i = 0; i < 3; i++)
			m_floats[i].NoMemento();
	}

	bool Manage(CCompressionUpr* pUpr)
	{
		bool ret = false;
		for (unsigned k = 0; k < 3; k++)
			ret |= m_floats[k].Manage(pUpr, m_name.c_str(), k);

		return ret;
	}

	void Init(CCompressionUpr* pUpr)
	{
		for (i32 k = 0; k < 3; k++)
			m_floats[k].Init(m_name.c_str(), k, pUpr->GetUseDirectory().c_str(), pUpr->GetAccDirectory().c_str());
	}
#else
	bool Manage(CCompressionUpr* pUpr) { return false; }
	void Init(CCompressionUpr* pUpr) {}
#endif

#if USE_ARITHSTREAM
	bool ReadValue(CCommInputStream& in, Vec3& value, CArithModel* pModel, u32 age) const
	{
		for (i32 i = 0; i < 3; i++)
			if (!m_floats[i].ReadValue(in, value[i], age, m_timeFraction32))
				return false;
		NetLogPacketDebug("CAdaptiveVec3Policy::ReadValue Previously Read (CAdaptiveFloat, CAdaptiveFloat, CAdaptiveFloat) (%f, %f, %f) (%f)", value.x, value.y, value.z, in.GetBitSize());
		return true;
	}
	bool WriteValue(CCommOutputStream& out, Vec3 value, CArithModel* pModel, u32 age) const
	{
		for (i32 i = 0; i < 3; i++)
			m_floats[i].WriteValue(out, value[i], age, m_timeFraction32);
		return true;
	}

	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		NetWarning("AdaptiveVec3Policy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		NetWarning("AdaptiveVec3Policy: not implemented for generic types");
		return false;
	}

#else
	bool ReadValue(CNetInputSerializeImpl* in, Vec3& value, u32 age) const
	{
		for (i32 i = 0; i < 3; i++)
			if (!m_floats[i].ReadValue(in, value[i], age))
				return false;
		NetLogPacketDebug("CAdaptiveVec3Policy::ReadValue Previously Read (CAdaptiveFloat, CAdaptiveFloat, CAdaptiveFloat) (%f, %f, %f) (%f)", value.x, value.y, value.z, in->GetBitSize());
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, Vec3 value, u32 age) const
	{
		for (i32 i = 0; i < 3; i++)
			m_floats[i].WriteValue(out, value[i], age);
		return true;
	}

	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, T& value, u32 age) const
	{
		NetWarning("AdaptiveVec3Policy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, T value, u32 age) const
	{
		NetWarning("AdaptiveVec3Policy: not implemented for generic types");
		return false;
	}
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CAdaptiveVec3Policy");
		pSizer->Add(*this);
	}

#if NET_PROFILE_ENABLE
	i32 GetBitCount(Vec3 value)
	{
		return m_floats[0].GetBitCount() + m_floats[1].GetBitCount() + m_floats[2].GetBitCount();
	}

	template<class T>
	i32 GetBitCount(T value)
	{
		return 0;
	}
#endif
	void SetTimeValue(u32 timeFraction32)
	{
		m_timeFraction32 = timeFraction32;
	}
	
private:
	FloatType m_floats[3];
	mutable u32 m_timeFraction32;
	string m_name;
};

typedef CAdaptiveVec3PolicyT<CAdaptiveFloat> TAdaptiveVec3Policy;
typedef CAdaptiveVec3PolicyT<CPredictiveFloat> TPredictiveVec3Policy;

REGISTER_COMPRESSION_POLICY_TIME(TAdaptiveVec3Policy, "AdaptiveVec3");
REGISTER_COMPRESSION_POLICY_TIME(TPredictiveVec3Policy, "PredictiveVec3");
