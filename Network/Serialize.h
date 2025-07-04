// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  declaration of DinrusXNetwork ISerialize classes
   -------------------------------------------------------------------------
   История:
   - 26/07/2004   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __NETWORK_SERIALIZE_H__
#define __NETWORK_SERIALIZE_H__

#pragma once

#include <drx3D/Network/Config.h>

#if USE_ARITHSTREAM

	#include <drx3D/Network/CommStream.h>
	#include <drx3D/Sys/IConsole.h>

	#include <drx3D/Network/SimpleSerialize.h>
	#include <drx3D/Network/Network.h>
	#include <drx3D/Network/ICompressionPolicy.h>
	#include <drx3D/Network/CompressionUpr.h>
	#include <drx3D/Network/ArithModel.h>
	#include <drx3D/Network/DebugKit.h>

	#if ENABLE_SERIALIZATION_LOGGING
ILINE void LogSerialize(tukk type, tukk name, bool& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%d' (type 'bool', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, int8& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%d' (type 'int8', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, i16& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%d' (type 'i16', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, i32& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%d' (type 'i32', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, u8& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%d' (type 'u8', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, u16& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%d' (type 'u16', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, u32& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%d' (type 'u32', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, int64& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%" PRIi64 "' (type 'int64', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, uint64& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%" PRIu64 "' (type 'uint64', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, float& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%f' (type 'float', policy '%s', bit count %d)", type, name, value, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, Vec2& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%f,%f' (type 'Vec2', policy '%s', bit count %d)", type, name, value.x, value.y, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, Vec3& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%f,%f,%f' (type 'Vec3', policy '%s', bit count %d)", type, name, value.x, value.y, value.z, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, Quat& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%f,%f,%f,%f' (type 'Quat', policy '%s', bit count %d)", type, name, value.v.x, value.v.y, value.v.z, value.w, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, Ang3& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%f,%f,%f' (type 'Ang3', policy '%s', bit count %d)", type, name, value.x, value.y, value.z, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, CTimeValue& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%f' (type 'CTimeValue', policy '%s', bit count %d)", type, name, value.GetSeconds(), KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, SNetObjectID& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%s' (type 'SNetObjectID', policy '%s', bit count %d)", type, name, value.GetText(), KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, SSerializeString& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='%s' (type 'SSerializeString', policy '%s', bit count %d)", type, name, value.c_str(), KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, ScriptAnyValue& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
	NetLog("[%sSerialization]: '%s'='<variant>' (type 'ScriptAnyValue', policy '%s', bit count %d)", type, name, KeyToString(pPolicy->key).c_str(), bitcount);
}
ILINE void LogSerialize(tukk type, tukk name, XmlNodeRef& value, ICompressionPolicy* pPolicy, i32 bitcount)
{
		#if !defined(_DEBUG)
	NetLog("[%sSerialization]: '%s'='%s' (type 'ScriptAnyValue', policy '%s', bit count %d)", type, name, value->getXML(), KeyToString(pPolicy->key).c_str(), bitcount);
		#endif
}
	#endif // ENABLE_SERIALIZATION_LOGGING

	#if ENABLE_DEBUG_KIT
		#pragma warning(disable:4244)
	#endif

class CArithModel;

	#if DEEP_BANDWIDTH_ANALYSIS
extern bool g_DBAEnabled;
extern u32 g_DBASizePriorToUpdate;
extern DrxStringLocal g_DBAMainProfileBuffer;
extern DrxStringLocal g_DBASmallProfileBuffer;
extern DrxStringLocal g_DBALargeProfileBuffer;
	#endif

enum ESerializeChunkResult
{
	eSCR_Ok_Updated,
	eSCR_Ok,
	eSCR_Failed,
};

class CNetSerialize
{
public:
	CNetSerialize() : m_pArithModel(NULL), m_pCurState(0), m_pNewState(0), m_mementoAge(0), m_isOwner(false) {}

	void SetMementoStreams(
	  CByteInputStream* pCurState,
	  CByteOutputStream* pNewState,
	  u32 mementoAge,
	  u32 mementoTime,
	  bool isOwner)
	{
		NET_ASSERT(mementoAge < 20000000);
		m_pCurState = pCurState;
		m_pNewState = pNewState;
		m_mementoAge = mementoAge;
		m_isOwner = isOwner;
		m_mementoTime = mementoTime;
	}

	ILINE CArithModel* GetArithModel()
	{
		return m_pArithModel;
	}

	#if ENABLE_DEBUG_KIT
	static bool m_bEnableLogging;
	#endif

	virtual ESerializeChunkResult SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm) = 0;

protected:
	CByteInputStream*  GetCurrentState()        { return m_pCurState; }
	CByteOutputStream* GetNewState()            { return m_pNewState; }

	ILINE void         InvalidateCurrentState() { m_pCurState = NULL; }
	ILINE void         SetArithModel(CArithModel* pModel)
	{
		m_pArithModel = pModel;
	}

	ILINE u32 GetMementoAge() const
	{
		return m_mementoAge;
	}

	ILINE u32 GetMementoTime() const
	{
		return m_mementoTime;
	}

	ILINE bool IsOwner() const
	{
		return m_isOwner;
	}

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CNetSerialize");

		pSizer->Add(*this);
		m_pArithModel->GetMemoryStatistics(pSizer, true);
		m_pCurState->GetMemoryStatistics(pSizer);
		m_pNewState->GetMemoryStatistics(pSizer);
	}

private:
	CArithModel*       m_pArithModel;
	CByteInputStream*  m_pCurState;
	CByteOutputStream* m_pNewState;
	u32             m_mementoAge;
	u32             m_mementoTime;
	bool               m_isOwner;
};

class CNetOutputSerializeImpl :
	public CSimpleSerializeImpl<false, eST_Network>,
	public CNetSerialize
{
public:
	CNetOutputSerializeImpl(u8* pBuffer, size_t nSize, u8 nBonus = 0);
	CNetOutputSerializeImpl(IStreamAllocator* pAllocator, size_t initialSize, u8 bonus = 0);

	#if ENABLE_SERIALIZATION_LOGGING
	void SetNetChannel(CNetChannel* channel) { m_pChannel = channel; }
	#endif // ENABLE_SERIALIZATION_LOGGING

	template<class T_Value>
	ILINE void Value(tukk name, T_Value& value, u32 policy)
	{
		ValueImpl(name, value, CNetwork::Get()->GetCompressionUpr().GetCompressionPolicy(policy));
	}

	void ResetLogging();

	void SetArithModel(CArithModel* pModel)
	{
		CNetSerialize::SetArithModel(pModel);
	}

	bool BeginOptionalGroup(tukk name, bool condition)
	{
		this->Value(name, condition, 'bool');
		u8 prev = condition;
		if (GetCurrentState())
			prev = GetCurrentState()->GetTyped<u8>();
		if (prev != u8(condition))
			InvalidateCurrentState(); // trash mementos, as 'times, they are a changin''
		if (GetNewState())
			GetNewState()->PutTyped<u8>() = condition;
		return condition;
	}

	CCommOutputStream&            GetOutput() { return m_output; }

	virtual ESerializeChunkResult SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm);

	template<class T_Value>
	ILINE void ValueImpl(tukk name, T_Value& value, ICompressionPolicy* pPolicy)
	{
		if (!Ok())
			return;
		ConditionalPrelude(name);
		DEBUGKIT_SET_VALUE(name);
		DEBUGKIT_SET_KEY(pPolicy->key);
		DEBUGKIT_ADD_DATA_ENT(value);
		//DEBUGKIT_ADD_DATA_ENTITY(name, pPolicy->key, value);
	#if DEEP_BANDWIDTH_ANALYSIS
		float oldSize = m_output.GetBitSize();
	#endif

		pPolicy->SetTimeValue(GetMementoTime());
		if (!pPolicy->WriteValue(m_output, value, GetArithModel(), GetMementoAge(), IsOwner(), GetCurrentState(), GetNewState()))
		{
			NetWarning("Failed to compress %s", name);
			Failed();
			return;
		}
	#if ENABLE_SERIALIZATION_LOGGING
		if (!m_pChannel->IsLocal())
		{
			LogSerialize("Net", name, value, pPolicy, pPolicy->GetBitCount(value));
		}
	#endif // ENABLE_SERIALIZATION_LOGGING
	#if DEEP_BANDWIDTH_ANALYSIS
		IF_UNLIKELY(g_DBAEnabled)
		{
			float newSize = m_output.GetBitSize();
			g_DBASmallProfileBuffer.Format("{%s,%s : %0.2f(%d)}", name, KeyToString(pPolicy->key).c_str(), newSize - oldSize, pPolicy->GetBitCount(value));
			g_DBAMainProfileBuffer += g_DBASmallProfileBuffer;
		}
	#endif
		ConditionalPostlude(name);
	}

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CNetOutputSerializeImpl");

		pSizer->Add(*this);
		m_output.GetMemoryStatistics(pSizer);
	}

private:
	CCommOutputStream m_output;
	#if ENABLE_SERIALIZATION_LOGGING
	CNetChannel*      m_pChannel;
	#endif // ENABLE_SERIALIZATION_LOGGING

	ILINE void ConditionalPrelude(tukk name)
	{
		DEBUGKIT_ANNOTATION(string("begin: ") + name);
	#if STATS_COLLECTOR
		#if ENABLE_ACCURATE_BANDWIDTH_PROFILING
		m_sizeAtPrelude = m_output.GetApproximateSize();
		#else
		m_sizeAtPrelude = m_output.GetOutputSize();
		#endif
	#endif
	#if CHECK_ENCODING
		GetArithModel()->WriteString(m_output, name);
	#endif
	#if MINI_CHECK_ENCODING
		m_output.WriteBits(0, 1);
	#endif
	}

	ILINE void ConditionalPostlude(tukk name)
	{
	#if STATS_COLLECTOR
		#if ENABLE_ACCURATE_BANDWIDTH_PROFILING
		STATS.AddData(name, static_cast<float>(m_output.GetOutputSize() - m_sizeAtPrelude));
		#else
		STATS.AddData(name, static_cast<float>(m_output.GetApproximateSize() - m_sizeAtPrelude));
		#endif
	#endif
	#if CHECK_ENCODING
		GetArithModel()->WriteString(m_output, name);
	#endif
	#if MINI_CHECK_ENCODING
		m_output.WriteBits(0, 1);
	#endif
		DEBUGKIT_ANNOTATION(string("end: ") + name);
	}

	#if STATS_COLLECTOR
	u32 m_sizeAtPrelude;
	#endif
};

class CNetInputSerializeImpl :
	public CSimpleSerializeImpl<true, eST_Network>,
	public CNetSerialize
{
public:
	CNetInputSerializeImpl(u8k* pBuffer, size_t nSize, INetChannel* pChannel);

	void              Failed()   { CSimpleSerializeImpl<true, eST_Network>::Failed(); }

	CCommInputStream& GetInput() { return m_input; }

	template<class T_Value>
	void Value(tukk name, T_Value& value, u32 policy)
	{
		ValueImpl(name, value, CNetwork::Get()->GetCompressionUpr().GetCompressionPolicy(policy));
	}

	template<class T_Value>
	void Value(tukk szName, T_Value& value)
	{
		Value(szName, value, 0);
	}

	void SetArithModel(CArithModel* pModel)
	{
		CNetSerialize::SetArithModel(pModel);
	}

	bool BeginOptionalGroup(tukk name, bool condition)
	{
		Value(name, condition, 'bool');
		u8 prev = condition;
		if (GetCurrentState())
			prev = GetCurrentState()->GetTyped<u8>();
		if (prev != u8(condition))
			InvalidateCurrentState(); // trash mementos, as 'times, they are a changin''
		if (GetNewState())
			GetNewState()->PutTyped<u8>() = condition;
		return condition;
	}

	virtual ESerializeChunkResult SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm);

	template<class T_Value>
	ILINE void ValueImpl(tukk name, T_Value& value, ICompressionPolicy* pPolicy)
	{
		pPolicy->SetTimeValue(GetMementoTime());

		if (!Ok())
			return;
		ConditionalPrelude(name);
		if (!Ok())
			return;
		bool ok;
		if (m_bCommit)
			ok = pPolicy->ReadValue(m_input, value, GetArithModel(), GetMementoAge(), IsOwner(), GetCurrentState(), GetNewState());
		else
		{
			T_Value temp;
			ok = pPolicy->ReadValue(m_input, temp, GetArithModel(), GetMementoAge(), IsOwner(), GetCurrentState(), GetNewState());
		}
		if (!ok)
		{
			NetWarning("Failed to decompress %s", name);
			Failed();
			return;
		}
		ConditionalPostlude(name);
	}

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CNetInputSerializeImpl");

		pSizer->Add(*this);
		m_input.GetMemoryStatistics(pSizer);
	}

private:

	ILINE void ConditionalCheck(tukk name, tukk debugTime)
	{
	#if CHECK_ENCODING
		string temp;
		GetArithModel()->ReadString(m_input, temp);
		const bool bExpectedName = (temp == name);
		NET_ASSERT(bExpectedName);
		if (!bExpectedName)
		{
			NetWarning("Data mismatch: expected %s %s and got %s", debugTime, name, temp.c_str());
			Failed();
		}
	#endif
	#if MINI_CHECK_ENCODING
		const bool bReadBit = (m_input.ReadBits(1) != 0);
		NET_ASSERT(!bReadBit);
		if (bReadBit)
		{
			NetWarning("Data mismatch on %s %s", debugTime, name);
			Failed();
		}
	#endif
	}

	ILINE void ConditionalPrelude(tukk name)
	{
		DEBUGKIT_ANNOTATION(string("begin: ") + name);
		ConditionalCheck(name, "opening");
	}

	ILINE void ConditionalPostlude(tukk name)
	{
		ConditionalCheck(name, "closing");
		DEBUGKIT_ANNOTATION(string("end: ") + name);
	}

	INetChannel*     m_pChannel;
	CCommInputStream m_input;
};

// TODO: dirty: find better way
template<class T>
ILINE T* GetNetSerializeImplFromSerialize(TSerialize ser)
{
	ISerialize* pISerialize = GetImpl(ser);
	CSimpleSerialize<T>* pSimpleSerialize = static_cast<CSimpleSerialize<T>*>(pISerialize);
	return pSimpleSerialize->GetInnerImpl();
}

// TODO: extremely dirty... makes the above look clean: find better way
inline CNetSerialize* GetNetSerializeImpl(TSerialize ser)
{
	ISerialize* pISerialize = GetImpl(ser);
	if (pISerialize->IsReading())
		return static_cast<CNetSerialize*>(GetNetSerializeImplFromSerialize<CNetInputSerializeImpl>(ser));
	else
		return static_cast<CNetSerialize*>(GetNetSerializeImplFromSerialize<CNetOutputSerializeImpl>(ser));
}

#else

	#include <drx3D/Network/SimpleSerialize.h>
	#include <drx3D/Network/Network.h>
	#include <drx3D/Network/ICompressionPolicy.h>
	#include <drx3D/Network/ArithModel.h>
	#include <drx3D/Network/DebugKit.h>

enum ESerializeChunkResult
{
	eSCR_Ok_Updated,
	eSCR_Ok,
	eSCR_Failed,
};

class CNetSerialize
{
public:
	virtual ~CNetSerialize(){}
	CNetSerialize()
	{
		m_pCurState = NULL;
		m_pNewState = NULL;
		m_mementoAge = 0;
		m_isOwner = false;
	}

	void SetMementoStreams(CByteInputStream* pCurState, CByteOutputStream* pNewState, u32 mementoAge, u32 mementoTime32, bool isOwner)
	{
		NET_ASSERT(mementoAge < 20000000);
		m_pCurState = pCurState;
		m_pNewState = pNewState;
		m_mementoAge = mementoAge;
		m_mementoTime = mementoTime32;
		m_isOwner = isOwner;
	}

	virtual ESerializeChunkResult SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm) = 0;

	#if ENABLE_DEBUG_KIT
	static bool m_bEnableLogging;
	#endif

protected:
	CByteInputStream*  GetCurrentState()        { return m_pCurState; }
	CByteOutputStream* GetNewState()            { return m_pNewState; }

	ILINE void         InvalidateCurrentState() { m_pCurState = NULL; }
	ILINE u32       GetMementoAge() const    { return m_mementoAge; }
	ILINE bool         IsOwner() const          { return m_isOwner; }

	virtual void       GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CNetSerialize");

		pSizer->Add(*this);
		m_pCurState->GetMemoryStatistics(pSizer);
		m_pNewState->GetMemoryStatistics(pSizer);
	}

private:
	CByteInputStream*  m_pCurState;
	CByteOutputStream* m_pNewState;
	u32             m_mementoAge;
	u32             m_mementoTime;
	bool               m_isOwner;
};

class CNetOutputSerializeImpl :
	public CSimpleSerializeImpl<false, eST_Network>,
	public CNetSerialize
{
public:
	CNetOutputSerializeImpl(u8* pBuffer, size_t nSize, u8 nBonus = 0);

	void         SetNetChannel(CNetChannel* channel);
	CNetChannel* GetNetChannel() { return m_channel; }

	template<class T_Value>
	ILINE void Value(tukk name, T_Value& value, u32 policy)
	{
		ValueImpl(name, value, CNetwork::Get()->GetCompressionUpr().GetCompressionPolicy(policy));
	}

	void Reset(u8 bonus = 0);
	void ResetLogging();

	void SetArithModel(CArithModel* pModel)
	{
	}

	bool BeginOptionalGroup(tukk name, bool condition)
	{
		this->Value(name, condition, 'bool');
		u8 prev = condition;

		if (GetCurrentState())
		{
			prev = GetCurrentState()->GetTyped<u8>();
		}

		if (prev != u8(condition))
		{
			InvalidateCurrentState(); // trash mementos, as 'times, they are a changin''
		}

		if (GetNewState())
		{
			GetNewState()->PutTyped<u8>() = condition;
		}

		return condition;
	}

	CNetOutputSerializeImpl&      GetOutput() { return *this; }

	virtual ESerializeChunkResult SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm);

	template<class T_Value>
	ILINE void ValueImpl(tukk name, T_Value& value, ICompressionPolicy* pPolicy)
	{
		if (Ok())
		{
			ConditionalPrelude(name);
			DEBUGKIT_SET_VALUE(name);
			DEBUGKIT_SET_KEY(pPolicy->key);
			DEBUGKIT_ADD_DATA_ENT(value);
			//DEBUGKIT_ADD_DATA_ENTITY(name, pPolicy->key, value);

			if (!pPolicy->WriteValue(this, value, GetMementoAge(), IsOwner(), GetCurrentState(), GetNewState()))
			{
				NetWarning("Failed to compress %s", name);
				Failed();
				return;
			}
	#if ENABLE_SERIALIZATION_LOGGING
			if (!m_channel->IsLocal())
			{
				LogSerialize("Net", name, value, pPolicy, pPolicy->GetBitCount(value));
			}
	#endif // ENABLE_SERIALIZATION_LOGGING
			ConditionalPostlude(name);
		}
	}

	void         WriteNetId(SNetObjectID id);
	void         WriteTime(ETimeStream time, CTimeValue value);
	void         WriteString(const SSerializeString* string);

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CNetOutputSerializeImpl");

		pSizer->Add(*this);
	}

	// returns the size of the stream
	size_t Flush()
	{
		if (m_bufferPosBit)
		{
			m_buffer[m_bufferPos] &= (0xff << (8 - m_bufferPosBit));
			m_bufferPosBit = 0;
			m_bufferPos++;
		}
		return GetOutputSize();
	}

	size_t GetApproximateSize() const
	{
		return GetOutputSize();
	}

	size_t GetOutputSize() const
	{
		return m_bufferPos;
	}

	size_t GetBitSize() const
	{
		return 8 * m_bufferPos + m_bufferPosBit;
	}

	void WriteBits(u32 value, i32 num);

	void PutZeros(i32 n);

	#if CRC8_ENCODING
	u8 GetCRC() const { return m_crc.Result(); }
	#endif

private:
	void       AddBits(u8 value, i32 num);

	ILINE void ConditionalPrelude(tukk name)
	{
		DEBUGKIT_ANNOTATION(string("begin: ") + name);

	#if STATS_COLLECTOR
		#if ENABLE_ACCURATE_BANDWIDTH_PROFILING
		m_sizeAtPrelude = GetApproximateSize();
		#else
		m_sizeAtPrelude = GetBitSize();
		#endif
	#endif

	#if CHECK_ENCODING
		//		GetArithModel()->WriteString( m_output, name );
	#endif

	#if MINI_CHECK_ENCODING
		WriteBits(0, 1);
	#endif
	}

	ILINE void ConditionalPostlude(tukk name)
	{
	#if STATS_COLLECTOR
		#if ENABLE_ACCURATE_BANDWIDTH_PROFILING
		STATS.AddData(name, GetBitSize() - m_sizeAtPrelude);
		#else
		STATS.AddData(name, GetApproximateSize() - m_sizeAtPrelude);
		#endif
	#endif

	#if CHECK_ENCODING
		//		GetArithModel()->WriteString(m_output, name);
	#endif

	#if MINI_CHECK_ENCODING
		WriteBits(0, 1);
	#endif

		DEBUGKIT_ANNOTATION(string("end: ") + name);
	}

	CNetChannel* m_channel;
	u8*       m_buffer;
	size_t       m_bufferSize;
	size_t       m_bufferPos;
	i32          m_bufferPosBit;
	bool         m_multiplayer;

	#if CRC8_ENCODING
	CCRC8 m_crc;
	#endif

	#if STATS_COLLECTOR
	u32 m_sizeAtPrelude;
	#endif
};

class CNetInputSerializeImpl :
	public CSimpleSerializeImpl<true, eST_Network>,
	public CNetSerialize
{
public:
	CNetInputSerializeImpl(u8k* buffer, size_t size, CNetChannel* channel = NULL);

	void                    Failed()   { CSimpleSerializeImpl<true, eST_Network>::Failed(); }
	CNetInputSerializeImpl& GetInput() { return *this; }

	void                    SetNetChannel(CNetChannel* channel);
	CNetChannel*            GetNetChannel() { return m_channel; }

	template<class T_Value>
	void Value(tukk name, T_Value& value, u32 policy)
	{
		ValueImpl(name, value, CNetwork::Get()->GetCompressionUpr().GetCompressionPolicy(policy));
	}

	template<class T_Value>
	void Value(tukk szName, T_Value& value)
	{
		Value(szName, value, 0);
	}

	bool BeginOptionalGroup(tukk name, bool condition)
	{
		Value(name, condition, 'bool');
		u8 prev = condition;

		if (GetCurrentState())
		{
			prev = GetCurrentState()->GetTyped<u8>();
		}

		if (prev != u8(condition))
		{
			InvalidateCurrentState(); // trash mementos, as 'times, they are a changin''
		}

		if (GetNewState())
		{
			GetNewState()->PutTyped<u8>() = condition;
		}

		NetLogPacketDebug("BeginOptionalGroup %d", condition);

		return condition;
	}

	virtual ESerializeChunkResult SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm);

	template<class T_Value>
	ILINE void ValueImpl(tukk name, T_Value& value, ICompressionPolicy* pPolicy)
	{
		if (Ok())
		{
			ConditionalPrelude(name);

			if (Ok())
			{
				bool ok;

				if (m_bCommit)
				{
					ok = pPolicy->ReadValue(this, value, GetMementoAge(), IsOwner(), GetCurrentState(), GetNewState());
				}
				else
				{
					T_Value temp;
					ok = pPolicy->ReadValue(this, temp, GetMementoAge(), IsOwner(), GetCurrentState(), GetNewState());
				}

				if (ok)
				{
					ConditionalPostlude(name);
				}
				else
				{
					NetWarning("Failed to decompress %s", name);
					Failed();
					return;
				}
			}
		}
	}

	SNetObjectID ReadNetId();
	CTimeValue   ReadTime(ETimeStream time);
	void         ReadString(SSerializeString* string);

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CNetInputSerializeImpl");

		pSizer->Add(*this);
	}

	u32 ReadBits(i32 nBits);

	#if CRC8_ENCODING
	u8 GetCRC() const { return m_crc.Result(); }
	#endif

	#if ENABLE_DEBUG_KIT || ENABLE_CORRUPT_PACKET_DUMP
	inline float GetBitSize() const
	{
		return float(8 * m_bufferPos + m_bufferPosBit);
	}
	#endif

private:
	u8      GetBits(i32 num);

	void       SetInvalid() { m_bCommit = false; }

	ILINE void ConditionalPrelude(tukk name)
	{
		DEBUGKIT_ANNOTATION(string("begin: ") + name);

	#if CHECK_ENCODING
		/*		string temp;

		    GetArithModel()->ReadString( m_input, temp );

		    if (temp != name)
		    {
		      NetWarning( "Data mismatch: expected %s and got %s", name, temp.c_str() );
		      m_pChannel->Disconnect( eDC_ProtocolError, name );
		      SetInvalid();
		    }*/
	#endif

	#if MINI_CHECK_ENCODING
		if (ReadBits(1))
		{
			NetWarning("Data mismatch on opening %s", name);
			//m_pChannel->Disconnect( eDC_ProtocolError, name );
			//SetInvalid();
			Failed();
		}
	#endif
	}

	ILINE void ConditionalPostlude(tukk name)
	{
	#if CHECK_ENCODING
		/*		string temp;

		    GetArithModel()->ReadString( m_input, temp );

		    if (temp != name)
		    {
		      NetWarning( "Data mismatch: expected closing %s and got %s", name, temp.c_str() );
		      m_pChannel->Disconnect( eDC_ProtocolError, name );
		      SetInvalid();
		    }*/
	#endif

	#if MINI_CHECK_ENCODING
		if (ReadBits(1))
		{
			NetWarning("Data mismatch on closing %s", name);
			//m_pChannel->Disconnect( eDC_ProtocolError, name );
			//SetInvalid();
			Failed();
		}
	#endif

		DEBUGKIT_ANNOTATION(string("end: ") + name);
	}

	CNetChannel* m_channel;
	u8k* m_buffer;
	size_t       m_bufferSize;
	size_t       m_bufferPos;
	i32          m_bufferPosBit;
	bool         m_multiplayer;

	#if CRC8_ENCODING
	CCRC8 m_crc;
	#endif
};

// TODO: dirty: find better way
template<class T>
ILINE T* GetNetSerializeImplFromSerialize(TSerialize ser)
{
	ISerialize* pISerialize = GetImpl(ser);
	CSimpleSerialize<T>* pSimpleSerialize = static_cast<CSimpleSerialize<T>*>(pISerialize);
	return pSimpleSerialize->GetInnerImpl();
}

// TODO: extremely dirty... makes the above look clean: find better way
ILINE CNetSerialize* GetNetSerializeImpl(TSerialize ser)
{
	ISerialize* pISerialize = GetImpl(ser);
	if (pISerialize->IsReading())
		return static_cast<CNetSerialize*>(GetNetSerializeImplFromSerialize<CNetInputSerializeImpl>(ser));
	else
		return static_cast<CNetSerialize*>(GetNetSerializeImplFromSerialize<CNetOutputSerializeImpl>(ser));
}

#endif

#if NET_PROFILE_ENABLE

class CNetSizingSerializeImpl :
	public CSimpleSerializeImpl<false, eST_Network>,
	public CNetSerialize
{
public:
	#if ENABLE_SERIALIZATION_LOGGING
	CNetSizingSerializeImpl(INetChannel* pChannel)
		: m_pChannel(pChannel)
	#else
	CNetSizingSerializeImpl()
	#endif // ENABLE_SERIALIZATION_LOGGING
	{
		Reset();
	};

	template<class T_Value>
	ILINE void Value(tukk name, T_Value& value, u32 policy)
	{
		ValueImpl(name, value, CNetwork::Get()->GetCompressionUpr().GetCompressionPolicy(policy));
	}

	void Reset()
	{
		m_nBitCount = 0;
	}

	bool BeginOptionalGroup(tukk name, bool condition)
	{
		return condition;
	}

	virtual ESerializeChunkResult SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm)
	{
		return eSCR_Ok;
	}

	template<class T_Value>
	ILINE void ValueImpl(tukk name, T_Value& value, ICompressionPolicy* pPolicy)
	{
	#if ENABLE_SERIALIZATION_LOGGING
		size_t bitCountBefore = m_nBitCount;
	#endif // ENABLE_SERIALIZATION_LOGGING
		m_nBitCount += pPolicy->GetBitCount(value);
	#if ENABLE_SERIALIZATION_LOGGING
		if (!m_pChannel->IsLocal())
		{
			LogSerialize("Net", name, value, pPolicy, m_nBitCount - bitCountBefore);
		}
	#endif // ENABLE_SERIALIZATION_LOGGING
	}

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CNetSizingSerializeImpl");
		pSizer->Add(*this);
	}

	size_t GetBitSize() const
	{
		return m_nBitCount;
	}

private:
	#if ENABLE_SERIALIZATION_LOGGING
	INetChannel * m_pChannel;
	#endif // ENABLE_SERIALIZATION_LOGGING
	size_t m_nBitCount;
};

#endif //-- NET_PROFILE_ENABLE

#endif
