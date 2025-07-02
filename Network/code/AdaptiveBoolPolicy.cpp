// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ICompressionPolicy.h>
#include  <drx3D/Network/ArithModel.h>
#include  <drx3D/Network/BoolCompress.h>
#include  <drx3D/Network/Serialize.h>

class CAdaptiveBoolPolicy
{
public:
	bool Load(XmlNodeRef node, const string& filename)
	{
		return true;
	}
#if USE_MEMENTO_PREDICTORS
	bool ReadMemento(CByteInputStream& in) const
	{
		m_bool.ReadMemento(in);
		return true;
	}

	bool WriteMemento(CByteOutputStream& out) const
	{
		m_bool.WriteMemento(out);
		return true;
	}

	void NoMemento() const
	{
		m_bool.NoMemento();
	}
#endif

#if USE_ARITHSTREAM
	bool ReadValue(CCommInputStream& in, bool& value, CArithModel* pModel, u32 age) const
	{
		value = m_bool.ReadValue(in);
		return true;
	}
	bool WriteValue(CCommOutputStream& out, bool value, CArithModel* pModel, u32 age) const
	{
		m_bool.WriteValue(out, value);
		return true;
	}
	bool ReadValue(CCommInputStream& in, SNetObjectID& value, CArithModel* pModel, u32 age) const
	{
		value = pModel->ReadNetId(in);
		return true;
	}
	bool WriteValue(CCommOutputStream& out, SNetObjectID value, CArithModel* pModel, u32 age) const
	{
		pModel->WriteNetId(out, value);
		return true;
	}
	bool ReadValue(CCommInputStream& in, XmlNodeRef& value, CArithModel* pModel, u32 age) const
	{
		assert(false);
		return false;
	}
	bool WriteValue(CCommOutputStream& out, XmlNodeRef value, CArithModel* pModel, u32 age) const
	{
		assert(false);
		return false;
	}

	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
#else
	bool ReadValue(CNetInputSerializeImpl* in, bool& value, u32 age) const
	{
		value = m_bool.ReadValue(in);
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, bool value, u32 age) const
	{
		m_bool.WriteValue(out, value);
		return true;
	}
	bool ReadValue(CNetInputSerializeImpl* in, SNetObjectID& value, u32 age) const
	{
		value = in->ReadNetId();
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, SNetObjectID value, u32 age) const
	{
		out->WriteNetId(value);
		return true;
	}

	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, T& value, u32 age) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, T value, u32 age) const
	{
		NetWarning("BoolPolicy: not implemented for generic types");
		return false;
	}
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

	i32 GetBitCount(SNetObjectID value)
	{
		return BITCOUNT_NETID;
	}

	template<class T>
	i32 GetBitCount(T value)
	{
		return 0;
	}
#endif

private:
	CBoolCompress m_bool;
};

REGISTER_COMPRESSION_POLICY(CAdaptiveBoolPolicy, "AdaptiveBool");
