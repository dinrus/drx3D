// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ICOMPRESSIONPOLICY_H__
#define __ICOMPRESSIONPOLICY_H__

#pragma once

#include <drx3D/Network/CompressionUpr.h>
#include <drx3D/Network/ByteStream.h>
#include <drx3D/Network/CommStream.h>

class CArithModel;

struct ICompressionPolicy : public CMultiThreadRefCount
{
	u32k key;

	ICompressionPolicy(u32 k) : key(k)
	{
		++g_objcnt.compressionPolicy;
	}

	virtual ~ICompressionPolicy()
	{
		--g_objcnt.compressionPolicy;
	}

	virtual void Init(CCompressionUpr* pUpr) {}
	virtual bool Load(XmlNodeRef node, const string& filename) = 0;

	virtual void SetTimeValue(u32 timeFraction32){}

	// when policy needs to process some data from time to time
	// if it return false, this method will never be called again
	// this method is called from different thread than replication and should be thread safe
	virtual bool Manage(CCompressionUpr* pUpr) { return false; }


#if USE_ARITHSTREAM
	#define SERIALIZATION_TYPE(T)                                                                                                                                                  \
	  virtual bool ReadValue(CCommInputStream & in, T & value, CArithModel * pModel, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const = 0; \
	  virtual bool WriteValue(CCommOutputStream & out, T value, CArithModel * pModel, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const = 0;
	#include <drx3D/Network/SerializationTypes.h>
	#undef SERIALIZATION_TYPE
	virtual bool ReadValue(CCommInputStream& in, SSerializeString& value, CArithModel* pModel, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const = 0;
	virtual bool WriteValue(CCommOutputStream& out, const SSerializeString& value, CArithModel* pModel, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const = 0;
#else
	#define SERIALIZATION_TYPE(T)                                                                                                                                  \
	  virtual bool ReadValue(CNetInputSerializeImpl * in, T & value, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const = 0; \
	  virtual bool WriteValue(CNetOutputSerializeImpl * out, T value, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const = 0;
	#include <drx3D/Network/SerializationTypes.h>
	#undef SERIALIZATION_TYPE
	virtual bool ReadValue(CNetInputSerializeImpl* in, SSerializeString& value, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const = 0;
	virtual bool WriteValue(CNetOutputSerializeImpl* out, const SSerializeString& value, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const = 0;
#endif

	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const = 0;

#if NET_PROFILE_ENABLE
	#define SERIALIZATION_TYPE(T) \
	  virtual i32 GetBitCount(T value) = 0;
	#include <drx3D/Network/SerializationTypes.h>
	#undef SERIALIZATION_TYPE
	virtual i32 GetBitCount(SSerializeString& value) = 0;
#endif
};

typedef ICompressionPolicy* ICompressionPolicyPtr;

template<class T>
class CCompressionPolicy : public ICompressionPolicy
{
public:
	CCompressionPolicy(u32 key) : ICompressionPolicy(key), m_impl() {}
	CCompressionPolicy(u32 key, const T& impl) : ICompressionPolicy(key), m_impl(impl) {}

	virtual bool Load(XmlNodeRef node, const string& filename)
	{
		return m_impl.Load(node, filename);
	}

#if USE_ARITHSTREAM
	#define SERIALIZATION_TYPE(T)                                                                                                                                              \
	  virtual bool ReadValue(CCommInputStream & in, T & value, CArithModel * pModel, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const  \
	  {                                                                                                                                                                        \
	    Setup(pCurState);                                                                                                                                                      \
	    if (!m_impl.ReadValue(in, value, pModel, age))                                                                                                                         \
	      return false;                                                                                                                                                        \
	    Complete(pNewState);                                                                                                                                                   \
	    return true;                                                                                                                                                           \
	  }                                                                                                                                                                        \
	  virtual bool WriteValue(CCommOutputStream & out, T value, CArithModel * pModel, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const \
	  {                                                                                                                                                                        \
	    Setup(pCurState);                                                                                                                                                      \
	    if (!m_impl.WriteValue(out, value, pModel, age))                                                                                                                       \
	      return false;                                                                                                                                                        \
	    Complete(pNewState);                                                                                                                                                   \
	    return true;                                                                                                                                                           \
	  }
	#include <drx3D/Network/SerializationTypes.h>
	#undef SERIALIZATION_TYPE

	virtual bool ReadValue(CCommInputStream& in, SSerializeString& value, CArithModel* pModel, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
	{
		Setup(pCurState);
		if (!m_impl.ReadValue(in, value, pModel, age))
			return false;
		Complete(pNewState);
		return true;
	}
	virtual bool WriteValue(CCommOutputStream& out, const SSerializeString& value, CArithModel* pModel, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
	{
		Setup(pCurState);
		if (!m_impl.WriteValue(out, value, pModel, age))
			return false;
		Complete(pNewState);
		return true;
	}
#else
	// !USE_ARITHSTREAM

	#if USE_MEMENTO_PREDICTORS

		#define SERIALIZATION_TYPE(T)                                                                                                                              \
		  virtual bool ReadValue(CNetInputSerializeImpl * in, T & value, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const  \
		  {                                                                                                                                                        \
		    Setup(pCurState);                                                                                                                                      \
		    if (m_impl.ReadValue(in, value, age))                                                                                                                  \
		    {                                                                                                                                                      \
		      Complete(pNewState);                                                                                                                                 \
		      return true;                                                                                                                                         \
		    }                                                                                                                                                      \
		    return false;                                                                                                                                          \
		  }                                                                                                                                                        \
		  virtual bool WriteValue(CNetOutputSerializeImpl * out, T value, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const \
		  {                                                                                                                                                        \
		    Setup(pCurState);                                                                                                                                      \
		    if (m_impl.WriteValue(out, value, age))                                                                                                                \
		    {                                                                                                                                                      \
		      Complete(pNewState);                                                                                                                                 \
		      return true;                                                                                                                                         \
		    }                                                                                                                                                      \
		    return false;                                                                                                                                          \
		  }
		#include <drx3D/Network/SerializationTypes.h>
		#undef SERIALIZATION_TYPE

	virtual bool ReadValue(CNetInputSerializeImpl* in, SSerializeString& value, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
	{
		Setup(pCurState);
		if (m_impl.ReadValue(in, value, age))
		{
			Complete(pNewState);
			return true;
		}
		return false;
	}
	virtual bool WriteValue(CNetOutputSerializeImpl* out, const SSerializeString& value, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
	{
		Setup(pCurState);
		if (m_impl.WriteValue(out, value, age))
		{
			Complete(pNewState);
			return true;
		}
		return false;
	}
	#else

		#define SERIALIZATION_TYPE(T)                                                                                                                              \
		  virtual bool ReadValue(CNetInputSerializeImpl * in, T & value, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const  \
		  {                                                                                                                                                        \
		    return m_impl.ReadValue(in, value, age);                                                                                                               \
		  }                                                                                                                                                        \
		  virtual bool WriteValue(CNetOutputSerializeImpl * out, T value, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const \
		  {                                                                                                                                                        \
		    return m_impl.WriteValue(out, value, age);                                                                                                             \
		  }
		#include <drx3D/Network/SerializationTypes.h>
		#undef SERIALIZATION_TYPE

	virtual bool ReadValue(CNetInputSerializeImpl* in, SSerializeString& value, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
	{
		return m_impl.ReadValue(in, value, age);
	}
	virtual bool WriteValue(CNetOutputSerializeImpl* out, const SSerializeString& value, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
	{
		return m_impl.WriteValue(out, value, age);
	}

	#endif

#endif

#if NET_PROFILE_ENABLE
	#define SERIALIZATION_TYPE(T)         \
	  virtual i32 GetBitCount(T value)    \
	  {                                   \
	    return m_impl.GetBitCount(value); \
	  }
	#include <drx3D/Network/SerializationTypes.h>
	#undef SERIALIZATION_TYPE

	virtual i32 GetBitCount(SSerializeString& value)
	{
		return m_impl.GetBitCount(value);
	}
#endif

	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		m_impl.GetMemoryStatistics(pSizer);
	}

private:

#if USE_MEMENTO_PREDICTORS
	void Setup(CByteInputStream* pCurState) const
	{
		if (pCurState)
		{
	#if VERIFY_MEMENTO_BUFFERS
			u32 tag = pCurState->GetTyped<u32>();
			NET_ASSERT(tag == 0x12345678);
	#endif
			m_impl.ReadMemento(*pCurState);
	#if VERIFY_MEMENTO_BUFFERS
			tag = pCurState->GetTyped<u32>();
			NET_ASSERT(tag == 0x87654321);
	#endif
		}
		else
			m_impl.NoMemento();
	}
	void Complete(CByteOutputStream* pNewState) const
	{
		if (pNewState)
		{
	#if VERIFY_MEMENTO_BUFFERS
			pNewState->PutTyped<u32>() = 0x12345678;
	#endif
			m_impl.WriteMemento(*pNewState);
	#if VERIFY_MEMENTO_BUFFERS
			pNewState->PutTyped<u32>() = 0x87654321;
	#endif
		}
	}
#endif

protected:
	T m_impl;
};

struct CompressionPolicyFactoryBase {};

template<class T>
struct CompressionPolicyFactory : public CompressionPolicyFactoryBase
{
	CompressionPolicyFactory(string name)
	{
		CCompressionRegistry::Get()->RegisterPolicy(name, Create);
	}

	static ICompressionPolicyPtr Create(u32 key)
	{
		return new CCompressionPolicy<T>(key);
	}
};

#define REGISTER_COMPRESSION_POLICY(cls, name)                  \
  extern void RegisterCompressionPolicy_ ## cls()               \
  {                                                             \
    static CompressionPolicyFactory<cls> cls ## _Factory(name); \
  }                                                             \

#endif
