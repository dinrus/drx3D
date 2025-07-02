// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RANGEDUNSIGNEDINTPOLICY_H__
#define __RANGEDUNSIGNEDINTPOLICY_H__

#pragma once

#include <drx3D/Network/ICompressionPolicy.h>
#include <drx3D/Network/ArithModel.h>
#include <drx3D/Network/StationaryUnsignedInteger.h>

class CRangedUnsignedIntPolicy
{
public:
	void SetValues(u32 nMin, u32 nMax)
	{
		m_integer.SetValues(nMin, nMax);
	}

	bool Load(XmlNodeRef node, const string& filename)
	{
		return m_integer.Load(node, filename, "Range");
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
	bool ReadValue(CCommInputStream& in, u32& value, CArithModel* pModel, u32 age) const
	{
		value = m_integer.ReadValue(in);
		return true;
	}
	bool WriteValue(CCommOutputStream& out, u32 value, CArithModel* pModel, u32 age) const
	{
		m_integer.WriteValue(out, value);
		return true;
	}

	#define DECL_PROXY_UNSIGNED(T)                                                              \
	  bool ReadValue(CCommInputStream & in, T & value, CArithModel * pModel, u32 age) const  \
	  {                                                                                         \
	    u32 temp;                                                                            \
	    bool ok = ReadValue(in, temp, pModel, age);                                             \
	    if (ok)                                                                                 \
	    {                                                                                       \
	      value = (T)temp;                                                                      \
	      NET_ASSERT(value == temp);                                                            \
	    }                                                                                       \
	    return ok;                                                                              \
	  }                                                                                         \
	  bool WriteValue(CCommOutputStream & out, T value, CArithModel * pModel, u32 age) const \
	  {                                                                                         \
	    u32 temp = (u32)value;                                                            \
	    NET_ASSERT(temp == value);                                                              \
	    return WriteValue(out, temp, pModel, age);                                              \
	  }

	DECL_PROXY_UNSIGNED(u8)
	DECL_PROXY_UNSIGNED(u16)
	DECL_PROXY_UNSIGNED(uint64)
	DECL_PROXY_UNSIGNED(int8)
	DECL_PROXY_UNSIGNED(i16)
	DECL_PROXY_UNSIGNED(i32)
	DECL_PROXY_UNSIGNED(int64)

	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		NetWarning("RangedUnsignedIntPolicy: ReadValue not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		NetWarning("RangedUnsignedIntPolicy: WriteValue not implemented for generic types");
		return false;
	}
#else
	bool ReadValue(CNetInputSerializeImpl* in, u32& value, u32 age) const
	{
		value = m_integer.ReadValue(in);
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, u32 value, u32 age) const
	{
		m_integer.WriteValue(out, value);
		return true;
	}

	#define DECL_PROXY_UNSIGNED(T)                                              \
	  bool ReadValue(CNetInputSerializeImpl * in, T & value, u32 age) const  \
	  {                                                                         \
	    u32 temp;                                                            \
	    bool ok = ReadValue(in, temp, age);                                     \
	    if (ok)                                                                 \
	    {                                                                       \
	      value = (T)temp;                                                      \
	      NET_ASSERT(value == temp);                                            \
	    }                                                                       \
	    return ok;                                                              \
	  }                                                                         \
	  bool WriteValue(CNetOutputSerializeImpl * out, T value, u32 age) const \
	  {                                                                         \
	    u32 temp = (u32)value;                                            \
	    NET_ASSERT(temp == value);                                              \
	    return WriteValue(out, temp, age);                                      \
	  }

	DECL_PROXY_UNSIGNED(u8)
	DECL_PROXY_UNSIGNED(u16)
	DECL_PROXY_UNSIGNED(uint64)
	DECL_PROXY_UNSIGNED(int8)
	DECL_PROXY_UNSIGNED(i16)
	DECL_PROXY_UNSIGNED(i32)
	DECL_PROXY_UNSIGNED(int64)

	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, T& value, u32 age) const
	{
		NetWarning("RangedUnsignedIntPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, T value, u32 age) const
	{
		NetWarning("RangedUnsignedIntPolicy: not implemented for generic types");
		return false;
	}
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "");
		pSizer->Add(*this);
	}

#if NET_PROFILE_ENABLE
	i32 GetBitCount(u32 value)
	{
		return m_integer.GetBitCount();
	}

	#define DECL_COUNT_UNSIGNED(T)   \
	  i32 GetBitCount(T value)       \
	  {                              \
	    u32 temp = (u32)value; \
	    return GetBitCount(temp);    \
	  }

	DECL_COUNT_UNSIGNED(u8)
	DECL_COUNT_UNSIGNED(u16)
	DECL_COUNT_UNSIGNED(uint64)
	DECL_COUNT_UNSIGNED(int8)
	DECL_COUNT_UNSIGNED(i16)
	DECL_COUNT_UNSIGNED(i32)
	DECL_COUNT_UNSIGNED(int64)

	template<class T>
	i32 GetBitCount(T value)
	{
		return 0;
	}
#endif

private:
	CStationaryUnsignedInteger m_integer;
};

#endif
