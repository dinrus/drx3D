// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  implementation of DinrusXNetwork ISerialize classes
   -------------------------------------------------------------------------
   История:
   - 26/07/2004   10:34 : Created by Craig Tiller
*************************************************************************/
#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/Config.h>

#if USE_ARITHSTREAM

	#include  <drx3D/Network/ArithStream.h>
	#include  <drx3D/Network/Serialize.h>
	#include  <drx3D/Network/Network.h>
	#include  <drx3D/Network/SerializationChunk.h>
	#include  <drx3D/Network/CompressionUpr.h>
	#include  <drx3D/Network/ByteStream.h>

#else

	#include  <drx3D/Network/Serialize.h>
	#include  <drx3D/Network/NetChannel.h>
	#include  <drx3D/Network/Network.h>
	#include  <drx3D/Network/ContextView.h>

#endif

#if USE_ARITHSTREAM

	#if ENABLE_DEBUG_KIT
bool CNetSerialize::m_bEnableLogging;
	#endif

//
// NetOutputSerialize
//

CNetOutputSerializeImpl::CNetOutputSerializeImpl(u8* pBuffer, size_t nSize, u8 nBonus) :
	m_output(pBuffer, nSize, nBonus)
	#if ENABLE_SERIALIZATION_LOGGING
	, m_pChannel(NULL)
	#endif // ENABLE_SERIALIZATION_LOGGING
{
	#if ENABLE_DEBUG_KIT
	if (m_bEnableLogging)
		m_output.EnableLog();
	#endif
}

CNetOutputSerializeImpl::CNetOutputSerializeImpl(IStreamAllocator* pAllocator, size_t initialSize, u8 bonus) :
	m_output(pAllocator, initialSize, bonus)
	#if ENABLE_SERIALIZATION_LOGGING
	, m_pChannel(NULL)
	#endif // ENABLE_SERIALIZATION_LOGGING
{
	#if ENABLE_DEBUG_KIT
	if (m_bEnableLogging)
		m_output.EnableLog();
	#endif
}

ESerializeChunkResult CNetOutputSerializeImpl::SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm)
{
	CTimeValue temp;
	if (!phData || *phData == CMementoMemoryUpr::InvalidHdl)
	{
		return eSCR_Failed;
	}
	else
	{
		CByteInputStream in((u8k*) mmm.PinHdl(*phData), mmm.GetHdlSize(*phData));
		if (pTimeInfo)
		{
			*pTimeInfo = in.GetTyped<CTimeValue>();
			Value("_tstamp", *pTimeInfo, 'tPhy');
		}
		CNetwork::Get()->GetCompressionUpr().BufferToStream(chunk, profile, in, *this);
		return eSCR_Ok;
	}
}

void CNetOutputSerializeImpl::ResetLogging()
{
	#if ENABLE_DEBUG_KIT
	m_output.EnableLog(m_bEnableLogging);
	#endif
}

//
// NetInputSerialize
//

CNetInputSerializeImpl::CNetInputSerializeImpl(u8k* pBuffer, size_t nSize, INetChannel* pChannel) :
	m_input(pBuffer, nSize), m_pChannel(pChannel)
{
	#if ENABLE_DEBUG_KIT
	if (m_bEnableLogging)
		m_input.EnableLog();
	#endif
}

ESerializeChunkResult CNetInputSerializeImpl::SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm)
{
	if (!Ok())
		return eSCR_Failed;

	CMementoStreamAllocator alloc(&mmm);
	size_t sizeHint = 8;
	if (phData && *phData != CMementoMemoryUpr::InvalidHdl)
		sizeHint = mmm.GetHdlSize(*phData);
	CByteOutputStream stm(&alloc, sizeHint);
	if (pTimeInfo)
	{
		Value("_tstamp", *pTimeInfo, 'tPhy');
		stm.PutTyped<CTimeValue>() = *pTimeInfo;
	}
	CNetwork::Get()->GetCompressionUpr().StreamToBuffer(chunk, profile, *this, stm);
	if (!Ok())
	{
		mmm.FreeHdl(alloc.GetHdl());
		return eSCR_Failed;
	}
	bool take = false;
	if (phData && m_bCommit)
	{
		take = true;
		if (*phData != CMementoMemoryUpr::InvalidHdl)
		{
			if (mmm.GetHdlSize(*phData) == stm.GetSize())
				if (0 == memcmp(mmm.PinHdl(*phData), mmm.PinHdl(alloc.GetHdl()), stm.GetSize()))
					take = false;
			if (take)
			{
				mmm.FreeHdl(*phData);
				mmm.ResizeHdl(alloc.GetHdl(), stm.GetSize());
				*phData = alloc.GetHdl();
			}
			else
			{
				mmm.FreeHdl(alloc.GetHdl());
			}
		}
		else
		{
			mmm.ResizeHdl(alloc.GetHdl(), stm.GetSize());
			*phData = alloc.GetHdl();
		}
	}
	else
	{
		mmm.FreeHdl(alloc.GetHdl());
	}
	return take ? eSCR_Ok_Updated : eSCR_Ok;
}

#else

	#if ENABLE_DEBUG_KIT
bool CNetSerialize::m_bEnableLogging;
	#endif

//
// NetOutputSerialize
//

CNetOutputSerializeImpl::CNetOutputSerializeImpl(u8* buffer, size_t size, u8 bonus)
{
	SetNetChannel(NULL);
	m_buffer = buffer;
	m_bufferSize = size;
	Reset(bonus);

	#if ENABLE_DEBUG_KIT
	//	if (m_bEnableLogging)
	//		m_output.EnableLog();
	#endif
}

ESerializeChunkResult CNetOutputSerializeImpl::SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm)
{
	CTimeValue temp;

	if (!phData || (*phData == CMementoMemoryUpr::InvalidHdl))
	{
		return eSCR_Failed;
	}
	else
	{
		CByteInputStream in((u8k*) mmm.PinHdl(*phData), mmm.GetHdlSize(*phData));

		if (pTimeInfo)
		{
			*pTimeInfo = in.GetTyped<CTimeValue>();
			Value("_tstamp", *pTimeInfo, 'tPhy');
		}

		CNetwork::Get()->GetCompressionUpr().BufferToStream(chunk, profile, in, *this);

		if (Ok())
		{
			return eSCR_Ok;
		}
		else
		{
			return eSCR_Failed;
		}
	}
}

void CNetOutputSerializeImpl::ResetLogging()
{
	#if ENABLE_DEBUG_KIT
	//	m_output.EnableLog( m_bEnableLogging );
	#endif
}

void CNetOutputSerializeImpl::Reset(u8 bonus)
{
	m_buffer[0] = bonus;
	m_bufferPos = 1;
	m_bufferPosBit = 0;
}

void CNetOutputSerializeImpl::WriteNetId(SNetObjectID id)
{
	debugPacketDataSizeStartNetIDData(id, GetBitSize());

	if (m_multiplayer)
	{
		CNetCVars& netCVars = CNetCVars::Get();

		if (id.id == SNetObjectID::InvalidId)
		{
			id.id = netCVars.net_invalidNetID;
		}

		if (id.id < netCVars.net_numNetIDLowBitIDs)
		{
			WriteBits(0, 1);
			WriteBits(id.id, netCVars.net_numNetIDLowBitBits);
		}
		else
		{
			if (id.id < netCVars.net_netIDHighBitStart)
			{
				WriteBits(2, 2);
				WriteBits(id.id - netCVars.net_numNetIDLowBitIDs, netCVars.net_numNetIDMediumBitBits);
			}
			else
			{
				WriteBits(3, 2);
				WriteBits(id.id - netCVars.net_netIDHighBitStart, netCVars.net_numNetIDHighBitBits);
			}
		}
	}
	else
	{
		WriteBits(id.id, 16);
	}

	debugPacketDataSizeEndData(eDPDST_NetID, GetBitSize());
}

void CNetOutputSerializeImpl::WriteTime(ETimeStream time, CTimeValue value)
{
	uint64 t = (uint64)value.GetMilliSecondsAsInt64();

	switch (time)
	{
	case eTS_Network:
		break;

	case eTS_NetworkPing:
		break;

	case eTS_NetworkPong:
		break;

	case eTS_PongElapsed:
		break;

	case eTS_Physics:
	case eTS_RemoteTime:
		t = t >> 5;
		break;
	}

	if (t < ((1 << 14) - 1))
	{
		WriteBits(0, 2);
		WriteBits((u32)t, 14);
	}
	else if (t < ((1 << 18) - 1))
	{
		WriteBits(1, 2);
		WriteBits((u32)t, 18);
	}
	else if (t < ((1 << 26) - 1))
	{
		WriteBits(2, 2);
		WriteBits((u32)t, 26);
	}
	else
	{
		WriteBits(3, 2);
		WriteBits((u32)(t >> 32) & 0x3f, 6);
		WriteBits((u32)t, 32);
	}
}

void CNetOutputSerializeImpl::WriteString(const SSerializeString* string)
{
	size_t length = string->length();
	tukk s = string->c_str();

	for (size_t i = 0; i < length + 1; i++)
	{
		WriteBits((u8)s[i], 8);
	}
}

void CNetOutputSerializeImpl::AddBits(u8 value, i32 num)
{
	if (m_bufferPos < m_bufferSize)
	{
		u8 write;
		u8 mask;
		i32 shift = (8 - num) - m_bufferPosBit;
		u8 curByte = m_buffer[m_bufferPos];

		if (shift > 0)
		{
			write = value << shift;
			mask = ((1 << num) - 1) << shift;
		}
		else
		{
			shift = -shift;
			write = value >> shift;
			mask = ((1 << num) - 1) >> shift;
		}

		curByte &= ~mask;
		curByte |= write;
		m_buffer[m_bufferPos] = curByte;

		if (m_bufferPosBit + num > 7)
		{
			num -= (8 - m_bufferPosBit);

			m_bufferPos++;
			m_bufferPosBit = 0;

			if (num > 0)
			{
				if (m_bufferPos < m_bufferSize)
				{
					curByte = m_buffer[m_bufferPos];
					shift = (8 - num);
					write = value << shift;
					mask = ((1 << num) - 1) << shift;

					curByte &= ~mask;
					curByte |= write;
					m_buffer[m_bufferPos] = curByte;

					m_bufferPosBit += num;
				}
				else
				{
					Failed();
				}
			}
		}
		else
		{
			m_bufferPosBit += num;
		}
	}
	else
	{
		Failed();
	}
}

void CNetOutputSerializeImpl::WriteBits(u32 value, i32 num)
{
	i32 numBytes = ((num & 7) == 0) ? num >> 3 : (num >> 3) + 1;

	switch (numBytes)
	{
	case 4:
		AddBits(value >> 24, num - 24);
	case 3:
		AddBits((value >> 16) & 0xff, num >= 24 ? 8 : num - 16);
	case 2:
		AddBits((value >> 8) & 0xff, num >= 16 ? 8 : num - 8);
	case 1:
		AddBits(value & 0xff, num >= 8 ? 8 : num);
	}
}

void CNetOutputSerializeImpl::PutZeros(i32 n)
{
	for (i32 i = 0; i < n; i++)
	{
		m_buffer[m_bufferPos++] = 0;
	}
}

void CNetOutputSerializeImpl::SetNetChannel(CNetChannel* channel)
{
	CNetContext* pNetContext = channel ? channel->GetContextView()->Context() : NULL;

	m_channel = channel;
	m_multiplayer = pNetContext ? pNetContext->IsMultiplayer() : false;
}

//
// NetInputSerialize
//

CNetInputSerializeImpl::CNetInputSerializeImpl(u8k* buffer, size_t size, CNetChannel* channel)
{
	#if ENABLE_DEBUG_KIT
	//	if (m_bEnableLogging)
	//		m_input.EnableLog();
	#endif

	SetNetChannel(channel); // inits channel and m_mutliplayer
	m_buffer = buffer;
	m_bufferSize = size;
	m_bufferPos = 1;
	m_bufferPosBit = 0;
}

ESerializeChunkResult CNetInputSerializeImpl::SerializeChunk(ChunkID chunk, u8 profile, TMemHdl* phData, CTimeValue* pTimeInfo, CMementoMemoryUpr& mmm)
{
	if (Ok())
	{
		CMementoStreamAllocator alloc(&mmm);
		size_t sizeHint = 8;

		if (phData && (*phData != CMementoMemoryUpr::InvalidHdl))
		{
			sizeHint = mmm.GetHdlSize(*phData);
		}

		CByteOutputStream stm(&alloc, sizeHint);

		if (pTimeInfo)
		{
			Value("_tstamp", *pTimeInfo, 'tPhy');
			stm.PutTyped<CTimeValue>() = *pTimeInfo;
		}

		CNetwork::Get()->GetCompressionUpr().StreamToBuffer(chunk, profile, *this, stm);

		bool take = false;

		if (Ok())
		{
			if (phData && m_bCommit)
			{
				take = true;

				if (*phData != CMementoMemoryUpr::InvalidHdl)
				{
					if (mmm.GetHdlSize(*phData) == stm.GetSize())
					{
						if (0 == memcmp(mmm.PinHdl(*phData), mmm.PinHdl(alloc.GetHdl()), stm.GetSize()))
						{
							take = false;
						}
					}

					if (take)
					{
						mmm.FreeHdl(*phData);
						mmm.ResizeHdl(alloc.GetHdl(), stm.GetSize());
						*phData = alloc.GetHdl();
					}
					else
					{
						mmm.FreeHdl(alloc.GetHdl());
					}
				}
				else
				{
					mmm.ResizeHdl(alloc.GetHdl(), stm.GetSize());
					*phData = alloc.GetHdl();
				}
			}
			else
			{
				mmm.FreeHdl(alloc.GetHdl());
			}
		}
		else
		{
			mmm.FreeHdl(alloc.GetHdl());
		}

		return take ? eSCR_Ok_Updated : eSCR_Ok;
	}

	return eSCR_Failed;
}

SNetObjectID CNetInputSerializeImpl::ReadNetId()
{
	SNetObjectID id;

	if (m_multiplayer)
	{
		CNetCVars& netCVars = CNetCVars::Get();
		u32 type = ReadBits(1);

		if (type == 0)
		{
			id.id = ReadBits(netCVars.net_numNetIDLowBitBits);
		}
		else
		{
			type = ReadBits(1);

			if (type == 0)
			{
				id.id = ReadBits(netCVars.net_numNetIDMediumBitBits) + netCVars.net_numNetIDLowBitIDs;
			}
			else
			{
				id.id = ReadBits(netCVars.net_numNetIDHighBitBits) + netCVars.net_netIDHighBitStart;
			}
		}

		if (id.id == netCVars.net_invalidNetID)
		{
			id.id = SNetObjectID::InvalidId;
		}
	}
	else
	{
		id.id = ReadBits(16);
	}

	m_channel->GetContextView()->ContextState()->Resaltify(id);

	return id;
}

CTimeValue CNetInputSerializeImpl::ReadTime(ETimeStream time)
{
	CTimeValue value;
	uint64 t = 0;
	u32 r = ReadBits(2);

	switch (r)
	{
	case 0:
		t = ReadBits(14);
		break;

	case 1:
		t = ReadBits(18);
		break;

	case 2:
		t = ReadBits(26);
		break;

	case 3:
		t = ReadBits(6);
		t = (t << 32) | ReadBits(32);
		break;
	}

	switch (time)
	{
	case eTS_Network:
		break;

	case eTS_NetworkPing:
		break;

	case eTS_NetworkPong:
		break;

	case eTS_PongElapsed:
		break;

	case eTS_Physics:
	case eTS_RemoteTime:
		t = t << 5;
		break;
	}

	value.SetMilliSeconds((int64)t);

	return value;
}

void CNetInputSerializeImpl::ReadString(SSerializeString* string)
{
	char s[1024];

	for (i32 i = 0; i < 1024; i++)
	{
		s[i] = (char)ReadBits(8);

		if (s[i] == 0)
		{
			break;
		}
	}

	*string = s;
}

u8 CNetInputSerializeImpl::GetBits(i32 num)
{
	u8 value = 0;

	if (m_bufferPos < m_bufferSize)
	{
		i32 shift = (8 - num) - m_bufferPosBit;

		value = (shift < 0)
		        ? (m_buffer[m_bufferPos] << (-shift)) & ((1 << num) - 1)
		        : (m_buffer[m_bufferPos] >> shift) & ((1 << num) - 1);

		if (m_bufferPosBit + num > 7)
		{
			num -= (8 - m_bufferPosBit);

			m_bufferPos++;
			m_bufferPosBit = 0;

			if (num > 0)
			{
				if (m_bufferPos < m_bufferSize)
				{
					value |= (m_buffer[m_bufferPos] >> (8 - num));

					m_bufferPosBit += num;
				}
				else
				{
					Failed();
				}
			}
		}
		else
		{
			m_bufferPosBit += num;
		}
	}
	else
	{
		Failed();
	}

	return value;
}

u32 CNetInputSerializeImpl::ReadBits(i32 num)
{
	i32 numBytes = ((num & 7) == 0) ? num >> 3 : (num >> 3) + 1;
	u32 value = 0;

	switch (numBytes)
	{
	case 4:
		value |= GetBits(num - 24) << 24;
	case 3:
		value |= GetBits(num >= 24 ? 8 : num - 16) << 16;
	case 2:
		value |= GetBits(num >= 16 ? 8 : num - 8) << 8;
	case 1:
		value |= GetBits(num >= 8 ? 8 : num);
	}

	return value;
}

void CNetInputSerializeImpl::SetNetChannel(CNetChannel* channel)
{
	CNetContext* pNetContext = channel ? channel->GetContextView()->Context() : NULL;

	m_channel = channel;
	m_multiplayer = pNetContext ? pNetContext->IsMultiplayer() : false;
}

#endif
