// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File: CTPEndpoint.cpp
//  Описание:
//
//	История:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef __CTPENDPOINT_H__
#define __CTPENDPOINT_H__

#pragma once

#include <queue>
#include <set>

#include <drx3D/Network/ArithStream.h>
#if NEW_BANDWIDTH_MANAGEMENT
	#include <drx3D/Network/BandwidthCalculator.h>
#else
	#include <drx3D/Network/PacketRateCalculator.h>
#endif // NEW_BANDWIDTH_MANAGEMENT
#include <drx3D/Network/MessageMapper.h>
#include <drx3D/Network/Serialize.h>
#include <drx3D/Network/FrameTypes.h>
#include <drx3D/Network/NetworkInspector.h>
#include <drx3D/Network/MessageQueue.h>
#include <drx3D/Network/ICTPEndpointListener.h>
#include <drx3D/Network/AutoFreeHandle.h>

#include <drx3D/Network/ArithAlphabet.h>
#include <drx3D/Network/ArithModel.h>

#include <drx3D/CoreX/Containers/DrxFixedArray.h>

#if ENCRYPTION_RIJNDAEL
	#include <drx3D/Network/rijndael.h>
#elif ENCRYPTION_STREAMCIPHER
	#include <drx3D/Network/StreamCipher.h>
#endif

class CNetInputSerialize;
class CNetChannel;
class CStatsCollector;

#if USE_ARITHSTREAM
typedef CArithLargeAlphabetOrder0<>                TMsgAlphabet;
typedef CArithAlphabetOrder1<CArithAlphabetOrder0> TAckAlphabet;

	#if ENABLE_DEBUG_PACKET_DATA_SIZE
// Not used by arith stream only needed to keep ENABLE_DEBUG_PACKET_DATA_SIZE working.
enum EMessageIDTable
{
	eMIDT_Global,
	eMIDT_NumTables
};
	#endif
#else
enum EMessageIDTable
{
	#if ENABLE_DEBUG_PACKET_DATA_SIZE
	eMIDT_Global,           // Only needed to provide a place to store the overall MessageID stats when ENABLE_DEBUG_PACKET_DATA_SIZE
	#endif
	eMIDT_Normal,
	eMIDT_UpdateObject,     // When doing an object update most messages are UpdateAspect* and EndUpdateObject so less bits can be used for these.
	eMIDT_NumTables
};
#endif

#if ENABLE_DEBUG_PACKET_DATA_SIZE

	#define INVALID_MESSAGEID      0xffffffff
	#define INVALID_MESSAGEIDTABLE eMIDT_NumTables

enum EDebugPacketDataSizeType
{
	eDPDST_Ack,
	eDPDST_MessageID,
	eDPDST_NetID,
	eDPDST_MessagesHeader,
	eDPDST_MessageBody,
	eDPDST_MessagesFooter,
	eDPDST_HadUrgentData,
	eDPDST_Padding,
	eDPDST_NUM
};

struct SDebugPacketDataSize
{
	SDebugPacketDataSize()
	{
		memset(this, 0, sizeof(SDebugPacketDataSize));
		messageID = INVALID_MESSAGEID;
	}

	struct SMessageIDData
	{
		SMessageIDData()
		{
			memset(data, 0, sizeof(data));
		}

		struct SMessageIDTableData
		{
			u32 count;
			u32 size;
			u32 messageIDSize;
			u32 numNetIDs;
			u32 netIDSize;
		};

		SMessageIDTableData data[eMIDT_NumTables];
		string              name;
	};

	SMessageIDData* pMessageIDData;

	u32          messageID;
	u32          messageDataStart;
	EMessageIDTable messageIDTable;

	u32          idSize;
	u32          seqBytesSize;
	u32          signingKeySize;
	u32          dataStart[eDPDST_NUM];
	u32          dataSize[eDPDST_NUM];
	u32          origMessageIDSize;
	u32          totalDataSize;
	u32          numMessageIDs;
	u32          totalNumMessages[eMIDT_NumTables];
	u32          numInvalidNetIDs;
	u32          numLowBitNetIDs;
	u32          numMediumBitNetIDs;
	u32          numHighBitNetIDs;
	u32          totalNumNetIDs;
	u32          origNetIDSize;
	bool            remote;
};

extern SDebugPacketDataSize g_debugPacketDataSize;

inline void debugPacketDataSizeSetMessageIDs(CMessageMapper* pMapper)
{
	if (g_debugPacketDataSize.remote)
	{
		if (!g_debugPacketDataSize.pMessageIDData)
		{
			u32 numIDs = pMapper->GetNumberOfMsgIds();

			g_debugPacketDataSize.numMessageIDs = numIDs;
			g_debugPacketDataSize.pMessageIDData = new SDebugPacketDataSize::SMessageIDData[numIDs];

			g_debugPacketDataSize.pMessageIDData[0].name = "CCTPEndpoint:EndOfStream";

			for (u32 i = 1; i < numIDs; i++)
			{
				const SNetMessageDef* pDef = pMapper->GetDispatchInfo(i, NULL);

				if (pDef)
				{
					g_debugPacketDataSize.pMessageIDData[i].name = pDef->description;
				}
			}
		}
	}
}

inline void debugPacketDataSizeStartPacket(bool local, CMessageMapper* pMapper)
{
	g_debugPacketDataSize.remote = !local;

	if (g_debugPacketDataSize.remote)
	{
		debugPacketDataSizeSetMessageIDs(pMapper);
		g_debugPacketDataSize.idSize += 8;
		g_debugPacketDataSize.seqBytesSize += 8;
	}
}

inline void debugPacketDataSizeEndPacket(u32 size)
{
	if (g_debugPacketDataSize.remote)
	{
		g_debugPacketDataSize.totalDataSize += size;
	}
}

inline void debugPacketDataSizeStartData(EDebugPacketDataSizeType type, u32 pos)
{
	if (g_debugPacketDataSize.remote)
	{
		g_debugPacketDataSize.dataStart[type] = pos;
	}
}

inline void debugPacketDataSizeEndMessage(u32 pos)
{
	if (g_debugPacketDataSize.remote)
	{
		if (g_debugPacketDataSize.pMessageIDData && (g_debugPacketDataSize.messageID != INVALID_MESSAGEID))
		{
			g_debugPacketDataSize.pMessageIDData[g_debugPacketDataSize.messageID].data[eMIDT_Global].size += pos - g_debugPacketDataSize.messageDataStart;

			if (g_debugPacketDataSize.messageIDTable != INVALID_MESSAGEIDTABLE)
			{
				g_debugPacketDataSize.pMessageIDData[g_debugPacketDataSize.messageID].data[g_debugPacketDataSize.messageIDTable].size += pos - g_debugPacketDataSize.messageDataStart;
			}

			g_debugPacketDataSize.messageID = INVALID_MESSAGEID;
		}
	}
}

inline void debugPacketDataSizeStartMessageIDData(u32 id, u32 pos, EMessageIDTable table)
{
	if (g_debugPacketDataSize.remote)
	{
		debugPacketDataSizeEndMessage(pos);

		g_debugPacketDataSize.messageID = id;
		g_debugPacketDataSize.messageDataStart = pos;
		g_debugPacketDataSize.messageIDTable = table;

		debugPacketDataSizeStartData(eDPDST_MessageID, pos);

		if (g_debugPacketDataSize.pMessageIDData)
		{
			g_debugPacketDataSize.pMessageIDData[id].data[eMIDT_Global].count++;
			g_debugPacketDataSize.totalNumMessages[eMIDT_Global]++;

			if (g_debugPacketDataSize.messageIDTable != INVALID_MESSAGEIDTABLE)
			{
				g_debugPacketDataSize.pMessageIDData[id].data[table].count++;
				g_debugPacketDataSize.totalNumMessages[table]++;
			}
		}
	}
}

inline void debugPacketDataSizeStartNetIDData(SNetObjectID id, u32 pos)
{
	if (g_debugPacketDataSize.remote)
	{
		debugPacketDataSizeStartData(eDPDST_NetID, pos);

		if (!id)
		{
			g_debugPacketDataSize.numInvalidNetIDs++;
		}

		if (id.id < CNetCVars::Get().net_numNetIDLowBitIDs)
		{
			g_debugPacketDataSize.numLowBitNetIDs++;
		}
		else
		{
			if (id.id < CNetCVars::Get().net_netIDHighBitStart)
			{
				g_debugPacketDataSize.numMediumBitNetIDs++;
			}
			else
			{
				g_debugPacketDataSize.numHighBitNetIDs++;
			}
		}

		g_debugPacketDataSize.totalNumNetIDs++;
	}
}

inline void debugPacketDataSizeEndData(EDebugPacketDataSizeType type, u32 pos)
{
	if (g_debugPacketDataSize.remote)
	{
		u32 size = pos - g_debugPacketDataSize.dataStart[type];

		g_debugPacketDataSize.dataSize[type] += size;

		if (type == eDPDST_MessageID)
		{
			// Make sure Message ID size isn't included with the messages header size, the message Body size or the messages footer size as well.
			g_debugPacketDataSize.dataStart[eDPDST_MessagesHeader] += size;
			g_debugPacketDataSize.dataStart[eDPDST_MessageBody] += size;
			g_debugPacketDataSize.dataStart[eDPDST_MessagesFooter] += size;

			// Store what the original 8 bit message id size would have been.
			g_debugPacketDataSize.origMessageIDSize += 8;

			if (g_debugPacketDataSize.pMessageIDData && (g_debugPacketDataSize.messageID != INVALID_MESSAGEID))
			{
				g_debugPacketDataSize.pMessageIDData[g_debugPacketDataSize.messageID].data[eMIDT_Global].messageIDSize += size;

				if (g_debugPacketDataSize.messageIDTable != INVALID_MESSAGEIDTABLE)
				{
					g_debugPacketDataSize.pMessageIDData[g_debugPacketDataSize.messageID].data[g_debugPacketDataSize.messageIDTable].messageIDSize += size;
				}
			}
		}
		if (type == eDPDST_NetID)
		{
			// Make sure Net ID size isn't included with the message Body size as well.
			g_debugPacketDataSize.dataStart[eDPDST_MessageBody] += size;

			// Store what the original 16 bit net id size would have been.
			g_debugPacketDataSize.origNetIDSize += 16;

			if (g_debugPacketDataSize.pMessageIDData && (g_debugPacketDataSize.messageID != INVALID_MESSAGEID))
			{
				g_debugPacketDataSize.pMessageIDData[g_debugPacketDataSize.messageID].data[eMIDT_Global].numNetIDs++;
				g_debugPacketDataSize.pMessageIDData[g_debugPacketDataSize.messageID].data[eMIDT_Global].netIDSize += size;

				if (g_debugPacketDataSize.messageIDTable != INVALID_MESSAGEIDTABLE)
				{
					g_debugPacketDataSize.pMessageIDData[g_debugPacketDataSize.messageID].data[g_debugPacketDataSize.messageIDTable].numNetIDs++;
					g_debugPacketDataSize.pMessageIDData[g_debugPacketDataSize.messageID].data[g_debugPacketDataSize.messageIDTable].netIDSize += size;
				}
			}
		}
		if ((type == eDPDST_MessageBody) || (type == eDPDST_MessagesHeader) || (type == eDPDST_MessagesFooter))
		{
			debugPacketDataSizeEndMessage(pos);
		}
	}
}

inline void debugPacketDataSizeSigningKey(u32 size)
{
	if (g_debugPacketDataSize.remote)
	{
		g_debugPacketDataSize.signingKeySize += size;
	}
}
#else
	#define debugPacketDataSizeSetMessageIDs(pMapper)
	#define debugPacketDataSizeStartPacket(local, pMapper)
	#define debugPacketDataSizeEndPacket(size)
	#define debugPacketDataSizeStartData(type, pos)
	#define debugPacketDataSizeStartMessageIDData(id, pos, table)
	#define debugPacketDataSizeStartNetIDData(id, pos)
	#define debugPacketDataSizeEndData(type, pos)
	#define debugPacketDataSizeSigningKey(size)
#endif

class CCTPEndpoint
{
public:
	CCTPEndpoint(CMementoMemoryUprPtr pMMM);
	~CCTPEndpoint();

	static tukk GetCurrentProcessingMessageDescription();

	void               Reset();
	void               Init(CNetChannel* pParent);
	ILINE bool         AddSendable(INetSendable* p, i32 numAfterHandle, const SSendableHandle* afterHandle, SSendableHandle* handle)        { return CallAddSubstitute(p, numAfterHandle, afterHandle, handle, false); }
	ILINE bool         SubstituteSendable(INetSendable* p, i32 numAfterHandle, const SSendableHandle* afterHandle, SSendableHandle* handle) { return CallAddSubstitute(p, numAfterHandle, afterHandle, handle, true); }
	bool               RemoveSendable(SSendableHandle handle);
	INetSendablePtr    FindSendable(SSendableHandle handle);
	void               Update(CTimeValue nTime, bool isDisconnecting, bool bAllowUserSend, bool bForceSend, bool bFlushBuffers);
	CTimeValue GetNextUpdateTime(CTimeValue);
	// assumes we're already in the right memento region
	void       ProcessPacket(CTimeValue nTime, CAutoFreeHandle& hdl, bool allowQueueing, bool inSync);
	void       GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false);
	void       MarkNotUserSink(INetMessageSink* pSink);
	float      GetPing(bool smoothed) const                   { return m_PacketRateCalculator.GetPing(smoothed); }
	bool       IsSufferingHighLatency(CTimeValue nTime) const { return m_PacketRateCalculator.IsSufferingHighLatency(nTime); }
	CTimeValue GetRemoteTime() const                          { return m_PacketRateCalculator.GetRemoteTime(); }
	bool       IsTimeReady() const                            { return m_PacketRateCalculator.IsTimeReady(); }
	CTimeValue GetNextUpdateTime();
	u32     GetMostRecentAckedSeq() const                  { return m_nInputAck; }
	u32     GetMostRecentSentSeq() const                   { return m_nOutputSeq; }
	void       SetPerformanceMetrics(INetChannel::SPerformanceMetrics* pMetrics)
	{
		m_PacketRateCalculator.SetPerformanceMetrics(*pMetrics);
	}
#if NEW_BANDWIDTH_MANAGEMENT
	const INetChannel::SPerformanceMetrics* GetPerformanceMetrics(void) const
	{
		return m_PacketRateCalculator.GetPerformanceMetrics();
	}
#endif // NEW_BANDWIDTH_MANAGEMENT

	bool        IsLocal() const;
	tukk GetName(void) const;
	void        GetBandwidthStatistics(u32 channelIndex, SBandwidthStats* pStats);

	bool        LookupMessage(tukk name, SNetMessageDef const** ppDef, INetMessageSink** ppSink);
#if !NEW_BANDWIDTH_MANAGEMENT
	void        FragmentedPacket(CTimeValue nTime) { m_PacketRateCalculator.FragmentedPacket(nTime); }
#endif // !NEW_BANDWIDTH_MANAGEMENT
	void        BackOff();

	bool        GetBackoffTime(CTimeValue& tm, bool total);
	bool        IsBackingOff()
	{
		CTimeValue blah;
		return GetBackoffTime(blah, false);
	}

	void         SetEntityId(EntityId id)   { m_entityId = id; }

	u32 GetLostPackets()           { return m_vInputState[m_nInputSeq].GetLostPackets(); }
	u32 GetUnreliableLostPackets() { return GetLostPackets(); }

	void         PerformRegularCleanup();
	void         EmptyMessages();
	bool         InEmptyMode() const { return m_emptyMode; }

	void         UnblockMessages()
	{
		NET_ASSERT(m_nStateBlockers);
		m_nStateBlockers--;
	}

	void  ChangeSubscription(ICTPEndpointListener* pListener, u32 eventMask);

	void  SchedulerDebugDraw();
	void  ChannelStatsDraw();

	void  SetAfterSpawning(bool afterSpawning);

	float GetBandwidth(CPacketRateCalculator::eIncomingOutgoing direction)
	{
		return (float)m_PacketRateCalculator.GetBandwidthUsage(g_time, direction);
	}

	void AddPingSample(CTimeValue nTime, CTimeValue nPing, CTimeValue nRemoteTime)
	{
		m_PacketRateCalculator.AddPingSample(nTime, nPing, nRemoteTime);
	}

#if ENABLE_CORRUPT_PACKET_DUMP
	void DoPacketDump();
#endif

#if ENABLE_RMI_BENCHMARK || ENABLE_URGENT_RMIS
	CNetChannel* GetParentChannel() { return m_pParent; }
#endif

private:
	bool CallAddSubstitute(INetSendable*, i32 numAfterHandle, const SSendableHandle* afterHandle, SSendableHandle* handle, bool subs);

	//
	// types
	//

	// these are the messages that the endpoint can deal with directly
	// eIM_LastInternalMessage must appear last so that the dispatch code knows
	// when to pass a message to a higher level protocol handler
	enum EInternalMessages
	{
		// end of stream (needed so we know when to stop decoding)
		eIM_EndOfStream = 0,
		// must stay at the end
		eIM_LastInternalMessage
	};

	// this class represents data that we don't want to keep around
	// in all 128 sequence states (2*64) if at all possible
	struct SBigEndpointState
	{
#if ENCRYPTION_RIJNDAEL
		SBigEndpointState(size_t acks, CMessageMapper& msgMapper, Rijndael::Direction dir, u8k* key, u8* initVec);
#elif ENCRYPTION_STREAMCIPHER
		SBigEndpointState(size_t acks, CMessageMapper& msgMapper, u8k* key, i32 keyLen);
#else
		SBigEndpointState(size_t acks, CMessageMapper& msgMapper);
#endif
		SBigEndpointState(const SBigEndpointState& cp)
#if USE_ARITHSTREAM || ALLOW_ENCRYPTION
			:
#endif
#if USE_ARITHSTREAM
		m_AckAlphabet(cp.m_AckAlphabet),
		m_MsgAlphabet(cp.m_MsgAlphabet),
		m_ArithModel(cp.m_ArithModel)
#endif
#if USE_ARITHSTREAM && ALLOW_ENCRYPTION
		,
#endif
#if ALLOW_ENCRYPTION
		m_crypt(cp.m_crypt)
#endif
		{
#if !USE_ARITHSTREAM
			m_currentTable = cp.m_currentTable;
			memcpy(&m_normalData, &cp.m_normalData, sizeof(m_normalData));
			memcpy(&m_updateObjectData, &cp.m_updateObjectData, sizeof(m_updateObjectData));
			memcpy(m_messageIDTableChangeData, cp.m_messageIDTableChangeData, sizeof(m_messageIDTableChangeData));
#endif
			++g_objcnt.bigEndpointState;
		}
		~SBigEndpointState()
		{
			--g_objcnt.bigEndpointState;
		}
		SBigEndpointState& operator=(const SBigEndpointState& other)
		{
#if USE_ARITHSTREAM
			m_AckAlphabet = other.m_AckAlphabet;
			m_MsgAlphabet = other.m_MsgAlphabet;
			m_ArithModel = other.m_ArithModel;
#endif
#if ALLOW_ENCRYPTION
			m_crypt = other.m_crypt;
#endif
#if !USE_ARITHSTREAM
			m_currentTable = other.m_currentTable;
			memcpy(&m_normalData, &other.m_normalData, sizeof(m_normalData));
			memcpy(&m_updateObjectData, &other.m_updateObjectData, sizeof(m_updateObjectData));
			memcpy(m_messageIDTableChangeData, other.m_messageIDTableChangeData, sizeof(m_messageIDTableChangeData));
#endif
			return *this;
		}
		void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
		{
			SIZER_COMPONENT_NAME(pSizer, "CCTPEndpoint::SBigEndpointState");
			if (countingThis)
				pSizer->Add(*this);
#if USE_ARITHSTREAM
			m_ArithModel.GetMemoryStatistics(pSizer);
			m_AckAlphabet.GetMemoryStatistics(pSizer);
			m_MsgAlphabet.GetMemoryStatistics(pSizer);
#endif
		}

		void Encrypt(u8* pBuf, size_t len)
		{
#if ENCRYPTION_RIJNDAEL
			// cppcheck-suppress allocaCalled
			PREFAST_SUPPRESS_WARNING(6255) u8 * buf = (u8*)alloca(len);
			NET_ASSERT(0 == (len & 15));
			m_crypt.blockEncrypt(pBuf, len * 8, buf);
			memcpy(pBuf, buf, len);
#elif ENCRYPTION_STREAMCIPHER
			m_crypt.Encrypt(pBuf, len, pBuf);
#endif
		}
		void Decrypt(u8* pBuf, size_t len)
		{
#if ENCRYPTION_RIJNDAEL
			// cppcheck-suppress allocaCalled
			PREFAST_SUPPRESS_WARNING(6255) u8 * buf = (u8*)alloca(len);
			NET_ASSERT(0 == (len & 15));
			m_crypt.blockDecrypt(pBuf, len * 8, buf);
			memcpy(pBuf, buf, len);
#elif ENCRYPTION_STREAMCIPHER
			m_crypt.Decrypt(pBuf, len, pBuf);
#endif
		}

		size_t GetSize()
		{
#if USE_ARITHSTREAM
			return m_AckAlphabet.GetSize() + /*m_MsgAlphabet.GetSize() +*/ m_ArithModel.GetSize();
#else
			return 0;
#endif
		}
#if USE_ARITHSTREAM
		TAckAlphabet m_AckAlphabet;
		TMsgAlphabet m_MsgAlphabet;
		CArithModel  m_ArithModel;

		ILINE void   WriteMsgIDData(CCommOutputStream& stm, u32 id);
		ILINE u32 ReadMsgIDData(CCommInputStream& stm);
#endif
#if !USE_ARITHSTREAM
		// eMIDT_Normal data
		// 0 - BeginUpdateObject
		// 1 + idBitSize - for all other IDs
	#define NORMAL_NUM_LOW_BIT_MESSAGE_IDS     1
	#define NORMAL_NUM_LOW_BIT_MESSAGE_ID_BITS 1

		struct SNormalData
		{
			u32 idBitSize;
			u32 lowBitIDs[NORMAL_NUM_LOW_BIT_MESSAGE_IDS];
		};

		SNormalData m_normalData;

		// eMIDT_UpdateObject data
		// 0 - EndUpdateObject
		// 10 - UpdateAspect31
		// 11 + NUM_UPDATEOBJECT_HIGH_BIT_MESSAGE_ID_BITS - UpdateAspect**
		// UpdateAspect31 High Bit Version + idBitSize - for all other IDs
	#define UPDATEOBJECT_NUM_LOW_BIT_MESSAGE_IDS        1
	#define UPDATEOBJECT_NUM_LOW_BIT_MESSAGE_ID_BITS    1
	#define UPDATEOBJECT_NUM_MEDIUM_BIT_MESSAGE_IDS     1
	#define UPDATEOBJECT_NUM_MEDIUM_BIT_MESSAGE_ID_BITS 1
	#define UPDATEOBJECT_NUM_HIGH_BIT_MESSAGE_IDS       (NUM_ASPECTS - 1)
	#if NUM_ASPECTS == 8
		#define UPDATEOBJECT_NUM_HIGH_BIT_MESSAGE_ID_BITS 3
	#elif NUM_ASPECTS == 16
		#define UPDATEOBJECT_NUM_HIGH_BIT_MESSAGE_ID_BITS 4
	#elif NUM_ASPECTS == 32
		#define UPDATEOBJECT_NUM_HIGH_BIT_MESSAGE_ID_BITS 5
	#else
		#error Unkown NUM_ASPECTS
	#endif

		struct SUpdateObjectData
		{
			u32 lowBitIDs[UPDATEOBJECT_NUM_LOW_BIT_MESSAGE_IDS];
			u32 mediumBitIDs[UPDATEOBJECT_NUM_MEDIUM_BIT_MESSAGE_IDS];
			u32 highBitIDs[UPDATEOBJECT_NUM_HIGH_BIT_MESSAGE_IDS];
		};

		SUpdateObjectData m_updateObjectData;

		// MessageID Table change data
	#define NUM_MESSAGEID_TABLE_CHANGE_DATAS 4
		// 0 - EndUpdateObject switch to table eMIDT_Normal
		// 1 - BeginUpdateObject switch to table eMIDT_UpdateObject
		// 2 - BeginBindObject switch to table eMIDT_UpdateObject
		// 3 - BeginBindStaticObject switch to table eMIDT_UpdateObject
		struct  SMessageIDTableChangeData
		{
			u32          id;
			EMessageIDTable table;
		};

		SMessageIDTableChangeData m_messageIDTableChangeData[NUM_MESSAGEID_TABLE_CHANGE_DATAS];
		EMessageIDTable           m_currentTable;

		ILINE void   WriteMsgIDData(CNetOutputSerializeImpl& stm, u32 id, i32 numBits);
		ILINE u32 ReadMsgIDData(CNetInputSerializeImpl& stm, i32 numBits);
		ILINE void   SwitchTable(u32 id);
#endif
	private:
#if ENCRYPTION_RIJNDAEL
		Rijndael m_crypt;
#elif ENCRYPTION_STREAMCIPHER
		CStreamCipher m_crypt;
#endif
	};

	class CBigEndpointStateUpr
	{
	public:
		CBigEndpointStateUpr()
		{
			m_nAlloced = 0;
			m_minFree = 0;
			m_operationsSinceCleanup = 0;
		}

		~CBigEndpointStateUpr()
		{
			FlushAll();
		}

		void FlushAll()
		{
			NET_ASSERT(m_nAlloced == 0);

			while (m_vBuffer.size())
			{
				delete m_vBuffer.back();
				m_vBuffer.pop_back();
			}
		}

#if ENCRYPTION_RIJNDAEL
		SBigEndpointState* Create(size_t a, CMessageMapper& m, Rijndael::Direction dir, u8k* key, u8* initVec)
		{
			m_nAlloced++;
			return new SBigEndpointState(a, m, dir, key, initVec);
		}
#elif ENCRYPTION_STREAMCIPHER
		SBigEndpointState* Create(size_t a, CMessageMapper& m, u8k* key, i32 keyLen)
		{
			m_nAlloced++;
			return new SBigEndpointState(a, m, key, keyLen);
		}
#else
		SBigEndpointState* Create(size_t a, CMessageMapper& m)
		{
			m_nAlloced++;
			return new SBigEndpointState(a, m);
		}
#endif

		SBigEndpointState* Clone(SBigEndpointState* pState)
		{
			DRX_PROFILE_REGION(PROFILE_NETWORK, "SBigEndpointsState:Clone");
			m_nAlloced++;

			m_operationsSinceCleanup++;

			SBigEndpointState* pCloned;
			if (m_vBuffer.size())
			{
				pCloned = m_vBuffer.back();
				m_vBuffer.pop_back();
				*pCloned = *pState;
				m_minFree = std::min(m_minFree, u32(m_vBuffer.size()));
			}
			else
			{
				pCloned = new SBigEndpointState(*pState);
			}
			return pCloned;
		}

		void Free(SBigEndpointState* pState)
		{
			NET_ASSERT(m_nAlloced != 0);
			m_nAlloced--;
			if (!m_vBuffer.empty())
			{
#if USE_ARITHSTREAM
				pState->m_AckAlphabet = m_vBuffer[0]->m_AckAlphabet;
				pState->m_MsgAlphabet = m_vBuffer[0]->m_MsgAlphabet;
				pState->m_ArithModel.SimplifyMemoryUsing(m_vBuffer[0]->m_ArithModel);
#endif
			}
			m_vBuffer.push_back(pState);
		}

		size_t SpareSize()
		{
			size_t n = 0;
			for (size_t i = 0; i < m_vBuffer.size(); i++)
				n += m_vBuffer[i]->GetSize();
			return n;
		}

		void PerformRegularCleanup()
		{
			if (m_operationsSinceCleanup < 1024)
				return;

			NET_ASSERT(m_minFree <= m_vBuffer.size());
			for (u32 i = 0; i < m_minFree; i++)
			{
				delete m_vBuffer.back();
				m_vBuffer.pop_back();
			}
			m_minFree = m_vBuffer.size();
			m_operationsSinceCleanup = 0;
		}

		void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
		{
			SIZER_COMPONENT_NAME(pSizer, "CCTPEndpoint::CBigEndpointStateUpr");
			if (countingThis)
				pSizer->Add(*this);
			for (size_t i = 0; i < m_vBuffer.size(); i++)
				m_vBuffer[i]->GetMemoryStatistics(pSizer, true);
		}

	private:
		std::vector<SBigEndpointState*, stl::STLGlobalAllocator<SBigEndpointState*>> m_vBuffer;
		u32 m_nAlloced;
		u32 m_minFree;
		i32    m_operationsSinceCleanup;
	};

	// this tracks the state we were in at each sent packet
	// (so we know what state the other end "thinks" that we're in)
	// also contains read/write helpers for different things that we compress
	// adaptively (we need to know what the other end thinks is the connection
	// state, so it makes sense to make these operations members of this class)
	class CState
	{
	public:
#define ACK_TYPE_NACK                  0
#define ACK_TYPE_ACK                   1
#define ACK_TYPE_END_RETURN_NEEDED     2
#define ACK_TYPE_END_RETURN_NOT_NEEDED 3
#define ACK_TYPE_NUM_TYPES             4
#define ACK_TYPE_NUM_BITS              2

		// must call Reset() before using!!
		CState() : m_pBigState(NULL) {}

		// reset back to initial state (a new connection is starting)
		void Reset(CBigEndpointStateUpr* pBigMgr);
		// start a new state based on an existing one
		void Clone(CBigEndpointStateUpr* pBigMgr, const CState&);

#if USE_ARITHSTREAM
		// write an ack or a nack
		void WriteAck(CCommOutputStream& stm, bool bAck, CStatsCollector* pStats);
		// finish writing ack block
		void WriteEndAcks(CCommOutputStream& stm, bool returnAckNeeded, CStatsCollector* pStats);
		// read an ack or a nack (returns false if at the end of the ack block)
		bool ReadAck(CCommInputStream& stm, bool& bAck, u32& nSeq, bool& recvAckNeeded);

		// read a message id
		u32 ReadMsgId(CCommInputStream& stm);
		// write a message id
		bool   WriteMsgId(CCommOutputStream& stm, u32 id, CStatsCollector* pStats, tukk name);
#else
		// write an ack or a nack
		void WriteAck(CNetOutputSerializeImpl& stm, bool bAck, CStatsCollector* pStats);
		// finish writing ack block
		void WriteEndAcks(CNetOutputSerializeImpl& stm, bool returnAckNeeded, CStatsCollector* pStats);
		// read an ack or a nack (returns false if at the end of the ack block)
		bool ReadAck(CNetInputSerializeImpl& stm, bool& bAck, u32& nSeq, bool& recvAckNeeded);

		// read a message id
		u32 ReadMsgId(CNetInputSerializeImpl& stm);
		// write a message id
		bool   WriteMsgId(CNetOutputSerializeImpl& stm, u32 id, CStatsCollector* pStats, tukk name);
#endif

		ILINE void Encrypt(u8* buf, size_t len)
		{
			m_pBigState->Encrypt(buf, len);
		}
		ILINE void Decrypt(u8* buf, size_t len)
		{
			m_pBigState->Decrypt(buf, len);
		}

		// number of acked packets seen in this state-chain
		u32 GetNumberOfAckedPackets() const { return m_nAckedPackets; }
		// number of lost packets seen in this state-chain
		u32 GetLostPackets() const          { return m_nAckedPackets - m_nAcks; }

#if USE_ARITHSTREAM
		CArithModel* GetArithModel() { return &m_pBigState->m_ArithModel; }
#endif

		void FreeState(CBigEndpointStateUpr* pBigMgr)
		{
			if (m_pBigState) pBigMgr->Free(m_pBigState);
			m_pBigState = NULL;
		}
#if ENCRYPTION_RIJNDAEL
		void CreateState(CBigEndpointStateUpr* pBigMgr, CMessageMapper& msgMapper, Rijndael::Direction dir, u8k* key, u8* initVec)
		{
			NET_ASSERT(!m_pBigState);
			m_pBigState = pBigMgr->Create(ACK_TYPE_NUM_TYPES, msgMapper, dir, key, initVec);
		}
#elif ENCRYPTION_STREAMCIPHER
		void CreateState(CBigEndpointStateUpr* pBigMgr, CMessageMapper& msgMapper, u8k* key, i32 keyLen)
		{
			NET_ASSERT(!m_pBigState);
			m_pBigState = pBigMgr->Create(ACK_TYPE_NUM_TYPES, msgMapper, key, keyLen);
		}
#else
		void CreateState(CBigEndpointStateUpr* pBigMgr, CMessageMapper& msgMapper)
		{
			NET_ASSERT(!m_pBigState);
			m_pBigState = pBigMgr->Create(ACK_TYPE_NUM_TYPES, msgMapper);
		}
#endif
		void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
		{
			SIZER_COMPONENT_NAME(pSizer, "CCTPEndpoint::CState");
			if (countingThis)
				pSizer->Add(*this);
			if (m_pBigState)
				m_pBigState->GetMemoryStatistics(pSizer, true);
		}
		size_t GetSize() { return sizeof(*this) + (m_pBigState ? m_pBigState->GetSize() : 0); }

#if DEBUG_ENDPOINT_LOGIC
		void Dump(FILE* f) const;
#endif

	private:
		// helper to update ack count trackers
		u32 AddAck(bool bAck);

		// total packets acknowledged
		u32             m_nAcks;
		// total packets acknowledged or not acknowledged
		u32             m_nAckedPackets;
		// total packets through this state train
		u32             m_nPackets;
		// number of acks in this state so far
		u32             m_nAcksThisPacket;

		SBigEndpointState* m_pBigState;
	};
	// specialization of CState for outgoing messages
	class COutputState : public CState
	{
	public:
		COutputState() : m_nStateBlockers(0), m_headSentMessage(InvalidSentElem), m_bAvailable(false) {}

		void Reset(CBigEndpointStateUpr* pBigMgr);
		void Clone(CBigEndpointStateUpr* pBigMgr, const COutputState&);
		void SentMessage(CCTPEndpoint& ep, SSendableHandle msghdl, i32 nStateBlockers)
		{
			ep.SentElem(m_headSentMessage, msghdl);
			m_nStateBlockers += nStateBlockers;
		}
		// these are debug helpers - to prevent over-writing a still in use state
		void Available()   { NET_ASSERT(m_bAvailable == false); m_bAvailable = true; }
		void Unavailable() { NET_ASSERT(m_bAvailable == true); m_bAvailable = false; }
		bool IsAvailable() { return m_bAvailable; }
		void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
		{
			SIZER_COMPONENT_NAME(pSizer, "CCTPEndpoint::COutputState");
			if (countingThis)
				pSizer->Add(*this);
			CState::GetMemoryStatistics(pSizer);
		}

		// the messages which were sent for this state (for implementing the various
		// reliability mechanisms)
		u32 m_headSentMessage;
		i32    m_nStateBlockers;

	private:
		bool m_bAvailable;
	};
	class CInputState : public CState
	{
	public:
		void   Clone(CBigEndpointStateUpr* pBigMgr, const CInputState&, u32 nSeq);
		u32 LastValid() const { return m_nValidSeq; }
		void   Reset(CBigEndpointStateUpr* pBigMgr);

	private:
		u32 m_nValidSeq;
	};
	class CMessageSender;

	struct SAckData
	{
		SAckData(bool received, bool hadData, CTimeValue whatsTheTimeAgain) : bReceived(received), bHadData(hadData), bHadUrgentData(0), when(whatsTheTimeAgain) {}
		bool       bReceived;
		bool       bHadData;
		bool       bHadUrgentData;
		CTimeValue when;
	};

	typedef std::deque<SAckData> TAckDeque;

	// this struct holds a packet that has been received but not processed yet
	// (in the case of out of order received packets, we try to delay their
	//  processing momentarily)
	struct TQueuedIncomingPacket
	{
		TMemHdl    hdl;
		CTimeValue when;
	};
	typedef std::map<u32, TQueuedIncomingPacket> TIncomingPacketMap;

	//
	// constants
	//

	// the length of a whole window of packets
	static u32k WHOLE_SEQ = NUM_SEQ_SLOTS;
	// the length of half a window of packets
	static u32k HALF_SEQ = WHOLE_SEQ / 2;

	class CSequenceNumberParser;
	class CParsePacketContext;

	//
	// variables
	//

	// we use this to actually send data
	CNetChannel* m_pParent;

	// the sequence number of the last packet sent
	// (incremented in SendPacket)
	u32 m_nOutputSeq;
	// the sequence number of the last packet we received
	// from our paired endpoint
	u32 m_nInputSeq;
	// the sequence number of the last ack we received
	u32 m_nInputAck;
	// the last basis sequence we got
	u32 m_nLastBasisSeq;

	// this helps support RMI - by allowing upper layers to specify
	// an entity id to pass around
	EntityId m_entityId;

	// the sequence number of the last packet sent with reliable data
	u32 m_nReliableSeq;
	// should we wait for an ack/nack before sending reliable data again
	// (we are ping-pong (1-bit sliding window) for reliable ordered messages,
	//  we need to wait for an ack before sending the next packet with
	//  reliable ordered messages in it)
	bool m_bReliableWait;

	// message mapper for outgoing packets
	CMessageMapper m_OutputMapper;
	// message mapper for incoming packets
	CMessageMapper m_InputMapper;
	// all of our message sinks
	struct SSinkInfo
	{
		SSinkInfo(INetMessageSink* p) : bDinrusXNetwork(false), pSink(p), lastUsed(0) {}
		// drxnetwork supplied message sinks get some special treatment - they
		// are assumed to know about when it's appropriate to post a message
		// and when not
		bool             bDinrusXNetwork;
		// the actual sink
		INetMessageSink* pSink;
		// when was this sink last used?
		u32           lastUsed;
	};

	std::vector<SSinkInfo> m_MessageSinks;

	// these two variables track which acks and nacks we need to send
	// (bool true == ack, false == nack, dqAcks first ack sequence number
	//  is nFrontAck)
	TAckDeque m_dqAcks;
	u32    m_nFrontAck;

	// these two structures track queued packets that are waiting to be
	// processed
	TIncomingPacketMap          m_incomingPackets;
	// only a member to avoid reallocating each frame - see Update()
	std::priority_queue<u32> m_timedOutPackets;

	// the state that the other end of the connection thinks that we're in
	COutputState                              m_vOutputState[WHOLE_SEQ];
	// the state we think the other end of the connection is in
	CInputState                               m_vInputState[WHOLE_SEQ];

	CPacketRateCalculator                     m_PacketRateCalculator;

	CBigEndpointStateUpr                  m_bigStateMgrInput;
	CBigEndpointStateUpr                  m_bigStateMgrOutput;
	CNetOutputSerializeImpl                   m_outputStreamImpl;
	bool                                      m_bWritingPacketNeedsInSyncProcessing;
	CSimpleSerialize<CNetOutputSerializeImpl> m_outputStream;

	bool                                      m_emptyMode;
	//i32 m_nBlockingMessages;

#if LOG_TFRC
	FILE* m_log_tfrc;
#endif

#if DEBUG_ENDPOINT_LOGIC
	// these functions are useful for debugging the low-level details
	// of the protocol implemented by this class
	FILE* m_log_output;
	FILE* m_log_input;
	void DumpOutputState(const CTimeValue& time);
	void DumpInputState(const CTimeValue& time);
#endif

#if DETECT_DUPLICATE_ACKS
	std::set<u32> m_ackedPackets;
#endif

#if ENABLE_CORRUPT_PACKET_DUMP
	struct              SCorruptPacketDumpData
	{
		SCorruptPacketDumpData()
		{
			hdl = TMemInvalidHdl;
			inSync = false;
			processingPacket = false;
			doingDump = false;
		}

		TMemHdl hdl;
		bool    inSync;
		bool    processingPacket;
		bool    doingDump;
	} m_corruptPacketDumpData;

	class CAutoSetCorruptPacketDumpData
	{
	public:
		CAutoSetCorruptPacketDumpData(SCorruptPacketDumpData& data, TMemHdl hdl, bool inSync) :
			m_data(data)
		{
			if (!m_data.processingPacket)
			{
				m_data.hdl = hdl;
				m_data.inSync = inSync;
				m_data.doingDump = false;
				m_data.processingPacket = true;
			}
		}

		~CAutoSetCorruptPacketDumpData()
		{
			m_data.doingDump = false;
			m_data.processingPacket = false;
		}

		SCorruptPacketDumpData& m_data;
	};
#endif

	//
	// functions
	//

	// this struct encodes the parameters that SendPacketIfAppropriate has
	// determined for SendPacket
	struct SSendPacketParams
	{
		SSendPacketParams()
			: nSize(0)
#if NEW_BANDWIDTH_MANAGEMENT
			, nSpareBytes(0)
#endif // NEW_BANDWIDTH_MANAGEMENT
			, bForce(false),
			bAllowUserSend(true)
		{}

		TPacketSize nSize;
#if NEW_BANDWIDTH_MANAGEMENT
		TPacketSize nSpareBytes;
#endif // NEW_BANDWIDTH_MANAGEMENT
		bool        bForce;
		bool        bAllowUserSend;
	};
	// send a packet with some configuration parameters
	u32 SendPacket(CTimeValue nTime, const SSendPacketParams&);

	// helper to perform actions necessary when receiving an ack
	bool AckPacket(CTimeValue nTime, u32 nSeq, bool bOk);
	void BroadcastMessage(const SCTPEndpointEvent& evt);
	void UpdatePendingQueue(CTimeValue nTime, bool isDisconnecting);
	void SendPacketsIfNecessary(CTimeValue nTime, bool isDisconnecting, bool bAllowUserSend, bool bForce, bool bFlush);

	bool IsIdle();

	void ProcessPacket_OneMessage(CParsePacketContext&);
	void ProcessPacket_EndOfStream(CParsePacketContext&);
	void ProcessPacket_NormalMessage(u32 msg, CParsePacketContext&);

#if MESSAGE_BACKTRACE_LENGTH > 0
	class CPreviouslyReceivedMessages
	{
	public:
		void Put(const SNetMessageDef* pDef)
		{
			if (q.Full())
				q.Pop();
			q.Push(pDef);
		}

		void Dump(tukk type, bool trailingDashDashDash)
		{
			NetLog("---------------------------------------------------------------------------------");
			NetLog("%d previously %s messages:", q.Size(), type);
			for (MQ::SIterator iter = q.Begin(); iter != q.End(); ++iter)
				NetLog("   %s", (*iter)->description);
			if (trailingDashDashDash)
				NetLog("---------------------------------------------------------------------------------");
		}

	private:
		typedef MiniQueue<const SNetMessageDef*, MESSAGE_BACKTRACE_LENGTH> MQ;
		MQ q;
	};
#else
	class CPreviouslyReceivedMessages
	{
	public:
		ILINE void Put(const SNetMessageDef* pDef)                   {}
		ILINE void Dump(tukk type, bool trailingDashDashDash) {}
	};
#endif
	CPreviouslyReceivedMessages m_previouslyReceivedMessages;
	CPreviouslyReceivedMessages m_previouslySentMessages;

	// simple counters to aid debugging
	i32        m_nReceived;
	i32        m_nSent;
	i32        m_nDropped;
	i32        m_nReordered;
	i32        m_nQueued;
	i32        m_nRepeated;

	i32        m_nPQTimeout;
	i32        m_nPQReady;

	CTimeValue m_backoffTimer;
	bool       m_receivedPacketSinceBackoff;

	class CMessageOutput;

	CMessageQueue m_queue;
	i32           m_nStateBlockers;
	i32           m_nUrgentAcks;

	struct SListener
	{
		ICTPEndpointListener* pListener;
		u32                eventMask;
	};
	std::vector<SListener> m_listeners;

	u8                  m_assemblyBuffer[MAX_TRANSMISSION_PACKET_SIZE];
	u32                 m_assemblySize;

	static u32k    InvalidSentElem = ~u32(0);
	struct SSentElem
	{
		SSendableHandle hdl;
		u32          next;
		SSentElem() : next(InvalidSentElem) {}
	};
	std::vector<SSentElem> m_sentElems;
	u32                 m_freeSentElem;
	void SentElem(u32& head, const SSendableHandle& hdl);
	void AckMessages(u32& head, u32 nSeq, bool ack, bool clear);
	void QueueForLater(CTimeValue nTime, CAutoFreeHandle& hdl, u32 nCurrentSeq);

	CMementoMemoryUprPtr m_pMMM;
	bool                     m_recvAckNeeded;
	bool                     m_sentAckNeeded;
};

#endif
