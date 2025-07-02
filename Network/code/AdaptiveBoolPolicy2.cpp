// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ICompressionPolicy2.h>
#include  <drx3D/Network/ArithModel.h>
#include  <drx3D/Network/BoolCompress2.h>

class CAdaptiveBoolPolicy2
{
public:
#if USE_MEMENTO_PREDICTORS
	typedef CBoolCompress2::TMemento TMemento;
#endif
	bool Load(XmlNodeRef node, const string& filename)
	{
		return true;
	}

#if USE_MEMENTO_PREDICTORS
	bool ReadMemento(TMemento& memento, CByteInputStream& in) const
	{
		m_bool.ReadMemento(memento, in);
		return true;
	}

	bool WriteMemento(TMemento memento, CByteOutputStream& out) const
	{
		m_bool.WriteMemento(memento, out);
		return true;
	}

	void InitMemento(TMemento& memento) const
	{
		m_bool.InitMemento(memento);
	}

	bool UpdateMemento(TMemento& memento, bool value) const
	{
		m_bool.UpdateMemento(memento, value);
		return true;
	}
#endif

#if USE_ARITHSTREAM
	bool ReadValue(TMemento memento, CCommInputStream& in, CArithModel* pModel, u32 age, bool& value) const
	{
		return m_bool.ReadValue(memento, in, value);
	}
	bool WriteValue(TMemento memento, CCommOutputStream& out, CArithModel* pModel, u32 age, bool value) const
	{
		return m_bool.WriteValue(memento, out, value);
	}
#else
	#if USE_MEMENTO_PREDICTORS
	bool ReadValue(TMemento memento, CNetInputSerializeImpl* in, u32 age, bool& value) const
	{
		return m_bool.ReadValue(memento, in, value);
	}
	bool WriteValue(TMemento memento, CNetOutputSerializeImpl* out, u32 age, bool value) const
	{
		return m_bool.WriteValue(memento, out, value);
	}
	#else
	bool ReadValue(CNetInputSerializeImpl* in, u32 age, bool& value) const
	{
		return m_bool.ReadValue(in, value);
	}
	bool WriteValue(CNetOutputSerializeImpl* out, u32 age, bool value) const
	{
		return m_bool.WriteValue(out, value);
	}
	#endif
#endif

#if USE_MEMENTO_PREDICTORS
	template<class T>
	bool UpdateMemento(TMemento& memento, T value) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
#endif

#if USE_ARITHSTREAM
	template<class T>
	bool ReadValue(TMemento memento, CCommInputStream& in, CArithModel* pModel, u32 age, T& value) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(TMemento memento, CCommOutputStream& out, CArithModel* pModel, u32 age, T value) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
#else
	#if USE_MEMENTO_PREDICTORS
	template<class T>
	bool ReadValue(TMemento memento, CNetInputSerializeImpl* in, u32 age, T& value) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(TMemento memento, CNetOutputSerializeImpl* out, u32 age, T value) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
	#else
	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, u32 age, T& value) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, u32 age, T value) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
	#endif
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CAdaptiveBoolPolicy");
		pSizer->Add(*this);
	}

#if NET_PROFILE_ENABLE
	i32 GetBitCount(bool value)
	{
		return m_bool.GetBitCount();
	}

	template<class T>
	i32 GetBitCount(T value)
	{
		return 0;
	}
#endif

private:
	CBoolCompress2 m_bool;
};

REGISTER_COMPRESSION_POLICY2(CAdaptiveBoolPolicy2, "AdaptiveBool2");
