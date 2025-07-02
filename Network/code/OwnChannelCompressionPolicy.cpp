// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  splits a compression policy into two pieces, one for the
               witness, and another for all other clients
   -------------------------------------------------------------------------
   История:
   - 02/11/2006   12:34 : Created by Craig Tiller
*************************************************************************/

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/OwnChannelCompressionPolicy.h>

#if USE_ARITHSTREAM
	#define SERIALIZATION_TYPE(T)                                                                                                                                                                    \
	  bool COwnChannelCompressionPolicy::ReadValue(CCommInputStream & in, T & value, CArithModel * pModel, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const  \
	  {                                                                                                                                                                                              \
	    return Get(own)->ReadValue(in, value, pModel, age, own, pCurState, pNewState);                                                                                                               \
	  }                                                                                                                                                                                              \
	  bool COwnChannelCompressionPolicy::WriteValue(CCommOutputStream & out, T value, CArithModel * pModel, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const \
	  {                                                                                                                                                                                              \
	    return Get(own)->WriteValue(out, value, pModel, age, own, pCurState, pNewState);                                                                                                             \
	  }
	#include <drx3D/Network/SerializationTypes.h>
	#undef SERIALIZATION_TYPE

bool COwnChannelCompressionPolicy::ReadValue(CCommInputStream& in, SSerializeString& value, CArithModel* pModel, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
{
	return Get(own)->ReadValue(in, value, pModel, age, own, pCurState, pNewState);
}

bool COwnChannelCompressionPolicy::WriteValue(CCommOutputStream& out, const SSerializeString& value, CArithModel* pModel, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
{
	return Get(own)->WriteValue(out, value, pModel, age, own, pCurState, pNewState);
}
#else
	#define SERIALIZATION_TYPE(T)                                                                                                                                                    \
	  bool COwnChannelCompressionPolicy::ReadValue(CNetInputSerializeImpl * in, T & value, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const  \
	  {                                                                                                                                                                              \
	    return Get(own)->ReadValue(in, value, age, own, pCurState, pNewState);                                                                                                       \
	  }                                                                                                                                                                              \
	  bool COwnChannelCompressionPolicy::WriteValue(CNetOutputSerializeImpl * out, T value, u32 age, bool own, CByteInputStream * pCurState, CByteOutputStream * pNewState) const \
	  {                                                                                                                                                                              \
	    return Get(own)->WriteValue(out, value, age, own, pCurState, pNewState);                                                                                                     \
	  }
	#include <drx3D/Network/SerializationTypes.h>
	#undef SERIALIZATION_TYPE

bool COwnChannelCompressionPolicy::ReadValue(CNetInputSerializeImpl* in, SSerializeString& value, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
{
	return Get(own)->ReadValue(in, value, age, own, pCurState, pNewState);
}

bool COwnChannelCompressionPolicy::WriteValue(CNetOutputSerializeImpl* out, const SSerializeString& value, u32 age, bool own, CByteInputStream* pCurState, CByteOutputStream* pNewState) const
{
	return Get(own)->WriteValue(out, value, age, own, pCurState, pNewState);
}
#endif

#if NET_PROFILE_ENABLE
	#define SERIALIZATION_TYPE(T)                            \
	  i32 COwnChannelCompressionPolicy::GetBitCount(T value) \
	  {                                                      \
	    i32k own = m_pOwn->GetBitCount(value);          \
	    i32k other = m_pOther->GetBitCount(value);      \
	    return own > other ? own : other;                    \
	  }
	#include <drx3D/Network/SerializationTypes.h>
	#undef SERIALIZATION_TYPE

i32 COwnChannelCompressionPolicy::GetBitCount(SSerializeString& value)
{
	assert(0);
	return 0;
}
#endif
