// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ICompressionPolicy.h>
#include  <drx3D/Network/ArithModel.h>
#include  <drx3D/Network/Serialize.h>

class CDefaultPolicy
{
public:
	bool Load(XmlNodeRef node, const string& filename)
	{
		return true;
	}

#if USE_MEMENTO_PREDICTORS
	bool ReadMemento(CByteInputStream& in) const
	{
		return true;
	}

	bool WriteMemento(CByteOutputStream& out) const
	{
		return true;
	}

	void NoMemento() const
	{
	}
#endif

#if USE_ARITHSTREAM
	bool ReadValue(CCommInputStream& in, bool& value, CArithModel* pModel, u32 age) const
	{
		value = in.ReadBits(1) != 0;
		return true;
	}
	bool WriteValue(CCommOutputStream& out, bool value, CArithModel* pModel, u32 age) const
	{
		out.WriteBits(value ? 1 : 0, 1);
		return true;
	}

	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		if (!ReadBytes(in, &value, sizeof(value)))
			return false;
		SwapEndian(value);
		return true;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		SwapEndian(value);
		return WriteBytes(out, &value, sizeof(value));
	}

	bool ReadValue(CCommInputStream& in, CTimeValue& value, CArithModel* pModel, u32 age) const
	{
		value = pModel->ReadTime(in, eTS_Network);
		return true;
	}
	bool WriteValue(CCommOutputStream& out, CTimeValue value, CArithModel* pModel, u32 age) const
	{
		pModel->WriteTime(out, eTS_Network, value);
		return true;
	}

	bool ReadValue(CCommInputStream& in, ScriptAnyValue& value, CArithModel* pModel, u32 age) const
	{
		NET_ASSERT(!"script values not supported");
		NetWarning("Network serialization of script types is not supported");
		return false;
	}
	bool WriteValue(CCommOutputStream& out, const ScriptAnyValue& value, CArithModel* pModel, u32 age) const
	{
		NET_ASSERT(!"script values not supported");
		NetWarning("Network serialization of script types is not supported");
		return false;
	}

	bool ReadValue(CCommInputStream& in, XmlNodeRef& value, CArithModel* pModel, u32 age) const
	{
		NET_ASSERT(!"XmlNodeRef not supported");
		NetWarning("Network serialization of script types is not supported");
		return false;
	}
	bool WriteValue(CCommOutputStream& out, const XmlNodeRef& value, CArithModel* pModel, u32 age) const
	{
		NET_ASSERT(!"XmlNodeRef not supported");
		NetWarning("Network serialization of script types is not supported");
		return false;
	}

	bool ReadValue(CCommInputStream& in, SSerializeString& value, CArithModel* pModel, u32 age) const
	{
		string s;
		bool bRes = pModel->ReadString(in, s);
		if (bRes)
			value = s.c_str();
		return bRes;
	}
	bool WriteValue(CCommOutputStream& out, const SSerializeString& value, CArithModel* pModel, u32 age) const
	{
		pModel->WriteString(out, string(value.c_str()));
		return true;
	}
#else
	bool ReadValue(CNetInputSerializeImpl* in, bool& value, u32 age) const
	{
		value = in->ReadBits(1) != 0;
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, bool value, u32 age) const
	{
		out->WriteBits(value ? 1 : 0, 1);
		return true;
	}

	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, T& value, u32 age) const
	{
		if (!ReadBytes(in, &value, sizeof(value)))
			return false;
		SwapEndian(value);
		return true;
	}
	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, T value, u32 age) const
	{
		SwapEndian(value);
		return WriteBytes(out, &value, sizeof(value));
	}

	bool ReadValue(CNetInputSerializeImpl* in, CTimeValue& value, u32 age) const
	{
		value = in->ReadTime(eTS_Network);
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, CTimeValue value, u32 age) const
	{
		out->WriteTime(eTS_Network, value);
		return true;
	}

	bool ReadValue(CNetInputSerializeImpl* in, ScriptAnyValue& value, u32 age) const
	{
		NET_ASSERT(!"script values not supported");
		NetWarning("Network serialization of script types is not supported");
		return false;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, const ScriptAnyValue& value, u32 age) const
	{
		NET_ASSERT(!"script values not supported");
		NetWarning("Network serialization of script types is not supported");
		return false;
	}

	bool ReadValue(CNetInputSerializeImpl* in, XmlNodeRef& value, u32 age) const
	{
		NET_ASSERT(!"XmlNodeRef not supported");
		NetWarning("Network serialization of script types is not supported");
		return false;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, const XmlNodeRef& value, u32 age) const
	{
		NET_ASSERT(!"XmlNodeRef not supported");
		NetWarning("Network serialization of script types is not supported");
		return false;
	}

	bool ReadValue(CNetInputSerializeImpl* in, SSerializeString& value, u32 age) const
	{
		in->ReadString(&value);
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, const SSerializeString& value, u32 age) const
	{
		out->WriteString(&value);
		return true;
	}
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CDefaultPolicy");
		pSizer->Add(*this);
	}
#if NET_PROFILE_ENABLE
	i32 GetBitCount(bool value)
	{
		return BITCOUNT_BOOL;
	}

	template<class T>
	i32 GetBitCount(T value)
	{
		return sizeof(value) * 8;
	}

	i32 GetBitCount(CTimeValue value)
	{
		return BITCOUNT_TIME;
	}

	i32 GetBitCount(ScriptAnyValue value)
	{
		return 0;
	}

	i32 GetBitCount(SSerializeString& value)
	{
		return BITCOUNT_STRINGID;
	}
#endif
private:
#if USE_ARITHSTREAM
	bool ReadBytes(CCommInputStream& in, uk pValue, size_t nBytes) const
	{
		u8* pAry = (u8*) pValue;
		for (size_t i = 0; i < nBytes; i++)
			pAry[i] = in.ReadBits(8);
		return true;
	}
	bool WriteBytes(CCommOutputStream& out, ukk pValue, size_t nBytes) const
	{
		u8k* pAry = (u8k*) pValue;
		for (size_t i = 0; i < nBytes; i++)
			out.WriteBits(pAry[i], 8);
		return true;
	}
#else
	bool ReadBytes(CNetInputSerializeImpl* in, uk pValue, size_t nBytes) const
	{
		u8* pAry = (u8*) pValue;

		for (size_t i = 0; i < nBytes; i++)
		{
			pAry[i] = in->ReadBits(8);
		}

		return true;
	}
	bool WriteBytes(CNetOutputSerializeImpl* out, ukk pValue, size_t nBytes) const
	{
		u8k* pAry = (u8k*) pValue;

		for (size_t i = 0; i < nBytes; i++)
		{
			out->WriteBits(pAry[i], 8);
		}

		return true;
	}
#endif
};

REGISTER_COMPRESSION_POLICY(CDefaultPolicy, "Default");

#ifndef _LIB
	#include <drx3D/CoreX/Common_TypeInfo.h>
#endif

#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/Network/ISerialize_info.h>
