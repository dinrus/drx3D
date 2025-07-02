// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MESSAGEQUEUE_H__
#define __MESSAGEQUEUE_H__

#pragma once

#include <drx3D/Network/Config.h>
#include <drx3D/Network/INetwork.h>
#include <drx3D/Network/CommStream.h>
#include <queue>
#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/CoreX/Containers/VectorSet.h>

#if !NEW_BANDWIDTH_MANAGEMENT

class CNetChannel;
class CNetOutputSerializeImpl;
class CCTPEndpoint;

struct SSchedulingParams
{
	// what is the time now
	CTimeValue now;
	// what is the (approximate) time that we'll send again
	CTimeValue next;
	// how long (approximately!) does it take to send a packet from here to the remote pc
	float      transportLatency;
	// how many bytes should a packet be?
	u32     targetBytes;
	// what is this packets sequence number?
	u32     nSeq;

	// witness in the following code means the entity that is observing the world for this channel
	// i.e. the player for that channel :)

	#if FULL_ON_SCHEDULING
	bool haveWitnessPosition;
	bool haveWitnessFov;
	bool haveWitnessDirection;

	// do we have a witness position? if so, what is it?
	Vec3 witnessPosition;

	// do we have a witness direction? if so, what is it?
	Vec3 witnessDirection;

	// do we have a witness field of view? if so, what is it?
	float witnessFov;
	#endif

	CNetChannel* pChannel;
};

enum ESentMessageFlags
{
	// keep handle around for an ack (keep waiting on ordered stuff until that comes too)
	eSMF_HandleAck           = 0x0001,
	// deprecated... old style reliable packet handling
	eSMF_TraditionalReliable = 0x0002,
};

struct SSentMessage
{
	SSentMessage() : accWeight(0.0f){}
	INetSendablePtr pSendable;
	CTimeValue      inserted;
	u32          firstDepNodeId;
	u32          numDepNodes;
	u32          flags;
	float           accWeight;
};

enum EWriteMessageResult
{
	eWMR_Delay,
	eWMR_Fail_Continue,
	eWMR_Fail_Finish,
	eWMR_Ok_Continue,
	eWMR_Ok_Finish,
};

struct IMessageOutput
{
	virtual ~IMessageOutput(){}
	#if USE_ARITHSTREAM
	virtual CCommOutputStream*       GetStream() = 0;
	#else
	virtual CNetOutputSerializeImpl* GetStream() = 0;
	#endif
	virtual EWriteMessageResult      WriteMessage(SSentMessage& msg, SSendableHandle hdl) = 0;
	virtual EWriteMessageResult WriteHeader() = 0;
	virtual EWriteMessageResult WriteFooter() = 0;
	virtual tukk         GetName() = 0;
};

struct CDistanceScaler
{
public:
	CDistanceScaler() : m_a(0), m_b(0), m_c(0) {}

	bool  Load(XmlNodeRef n);
	float GetBump(float dist);

private:
	float            m_a, m_b, m_c;
	static i32k DEGREE = 2;
};

class CPulseScaler
{
public:
	CPulseScaler() : m_a(0), m_b(0) {}

	bool  Load(XmlNodeRef n);
	float GetBump(float t);

private:
	float m_a, m_b;
};

class CDirScaler
{
public:
	CDirScaler() : m_a(0), m_b(0), m_pow(0) {}

	bool  Load(XmlNodeRef n);
	float GetBump(float cosang);

private:
	float m_a, m_b, m_pow;
};

enum EMessageQueueFlags
{
	eMQF_IsAfterSpawning = 0x00000001,
};

enum EMessageQueueAddSendableResult
{
	eMQASR_Ok,
	eMQASR_Failed,
	eMQASR_Ok_BecomeAlerted
};

class CMessageQueue
{
public:
	class CConfig;

	CMessageQueue(CConfig* pConfig);
	~CMessageQueue();

	static CConfig*                LoadConfig(tukk name);
	static void                    DestroyConfig(CConfig*);
	void                           SetParent(CCTPEndpoint* pParent) { m_pParent = pParent; }

	EMessageQueueAddSendableResult AddSendable(INetSendable* pMsg, i32 nAfterHandle, const SSendableHandle* afterHandle, SSendableHandle* handle, bool subs);
	bool                           RemoveSendable(SSendableHandle handle);
	INetSendablePtr                FindSendable(SSendableHandle handle);
	bool                           AreMessagesToWrite(const SSchedulingParams& params);
	bool                           BuildPacket(IMessageOutput* pOut, const SSchedulingParams& params);
	void                           FinishFrame(const SSchedulingParams* pParams);
	bool                           IsEmpty() const;
	bool                           IsBlockingStateChange() const;
	void                           AckMessages(SSendableHandle* pMsgs, size_t nMsgs, u32 nSeq, bool ack, bool clear);
	void                           Empty(bool includeRoots = false);

	u32                         GetFlags() const       { return m_flags; }
	void                           SetFlags(u32 flags) { m_flags = flags; }

	void                           GetBandwidthStatistics(u32 channelIndex, SBandwidthStats* pStats);

	void                           DrawLabel(float x, float y, float* clr, tukk msg, ...);
	void                           DebugDraw(const SSchedulingParams& params);

	void                           GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false);

	bool                           IsIdle() const { return m_nAlertedMessages == 0; }

private:
	struct SSentEnt
	{
		SSentEnt(CTimeValue w, float b) : when(w), bandwidth(b) {}
		SSentEnt() : when(0.0f), bandwidth(0) {}
		CTimeValue when;
		float      bandwidth;
	};
	typedef MiniQueue<SSentEnt, 31> TSentEntQueue;

	struct SAccountingGroupPulse
	{
		SAccountingGroupPulse() : name(0) {}
		u32       name;
		CPulseScaler scaler;
		bool operator<(const SAccountingGroupPulse& pulse) const
		{
			return name < pulse.name;
		}
	};

	struct SAccountingGroupPolicy
	{
		SAccountingGroupPolicy()
			: priority(0.f)
			, maxLatency(0.f)
			, discardLatency(0.f)
			, maxBandwidth(0.f)
			, drawn(false)
		{
		}
		// scheduling policy
		float                 priority;
		float                 maxLatency;
		float                 discardLatency;
		float                 maxBandwidth;
		bool                  drawn;
		// distance/direction scaling
		CDistanceScaler       distanceScaler;
		CDirScaler            dirScaler;
		// pulse scaling
		u32                numPulses;
		SAccountingGroupPulse pulses[MAXIMUM_PULSES_PER_STATE];
	};

	struct SAccountingGroup
	{
		u32                 id;
		SAccountingGroupPolicy policy;

		// accounting history
		TSentEntQueue sends;
		float         totBandwidthUsed;
		float         bandwidthThisFrame;

		SAccountingGroup()
		{
			++g_objcnt.mqAccountingGroup;
			totBandwidthUsed = 0.0f;
			bandwidthThisFrame = 0.0f;
		}

		~SAccountingGroup()
		{
			--g_objcnt.mqAccountingGroup;
		}

		float GetBandwidthUsed(CTimeValue now) const
		{
			if (sends.Empty())
				return 0;
			if (sends.Front().when == now)
				return totBandwidthUsed;
			return totBandwidthUsed / (now - sends.Front().when).GetSeconds();
		}

		bool BandwidthExceeded(CTimeValue now, bool randomEarlyDetection) const
		{
			float max = policy.maxBandwidth;
			if (max > 0)
			{
				// cap per frame
				if (randomEarlyDetection && bandwidthThisFrame > 0.15f * max)
					return true;

				float cur = GetBandwidthUsed(now);
				float por = cur / max;
				if (por > 1.0f)
					return true;
				if (!randomEarlyDetection)
					return false;
				const float det = 0.2f;
				if (por < (1.0f - det))
					return false;
				float cutoff = (por - (1.0f - det)) / det;
				return drx_random(0.0f, 1.0f) < cutoff;
			}
			else
				return false;
		}

		void PopSend()
		{
			totBandwidthUsed -= sends.Front().bandwidth;
			sends.Pop();
		}
	};

	enum ELatencyClass
	{
		eLC_Discardable,
		eLC_Expired,
		eLC_NearlyExpired,
		eLC_DontCare,
		eLC_DontBother,
		// we can prove early that it's impossible to send this message
		eLC_CantSend,
		// must be last
		eLC_NUM_CLASSES,
	};

	enum ELiveness
	{
		eL_Fresh,
		eL_Alive,
		eL_Sent,
		eL_Rotten,
		eL_Discarded,
	};

	struct SMsgEntOrderingInfo
	{
		SMsgEntOrderingInfo() : latencyClass(eLC_DontBother), bandwidthExceeded(false), schedulingOrder(0.0f) {}
		ELatencyClass latencyClass;
		bool          bandwidthExceeded;
		float         schedulingOrder;

		i32           LatencyBucket() const;
	};

	enum EMsgSlotState
	{
		eMSS_Active = 0,
		eMSS_JustQueued,
		eMSS_Waiting,
		eMSS_Limbo,
		eMSS_Dead,
		eMSS_NUM_LIVE_SLOT_TYPES,
		// special 'root node' slot for the different state lists
		eMSS_Root,
		// special 'not allocated' slot
		eMSS_Free,
	};

	struct SMsgSlot
	{
		SMsgSlot()
		{
			Reset();
		}

		void Reset()
		{
			msg.pSendable = INetSendablePtr();
			sortOrderingSlot = u32(-1);
			depth = 0;
			childCount = 0;
			childrenPatched = 0;
			liveness = eL_Rotten;
			pAG = 0;
		}

		SSentMessage        msg;
		SMsgEntOrderingInfo ordering;
		u32              sortOrderingSlot;
		i32                 depth;
		i32                 childCount;
		i32                 childrenPatched;
		ELiveness           liveness;
		SAccountingGroup*   pAG;

	#ifdef _DEBUG
		string lastName;
	#endif

		u32 nextLatencySort;
		u32 nextOrderingSort;
	};

	struct SSlotState
	{
		SSlotState() : state(eMSS_Free), next(0), prev(0)
		{
			++g_objcnt.mqSlotState;
		}
		~SSlotState()
		{
			--g_objcnt.mqSlotState;
		}
		SSlotState(const SSlotState& rhs) : state(rhs.state), next(rhs.next), prev(rhs.prev), nextTop(rhs.nextTop), prevTop(rhs.prevTop)
		{
			++g_objcnt.mqSlotState;
		}
		SSlotState& operator=(const SSlotState& rhs)
		{
			this->~SSlotState();
			new(this)SSlotState(rhs);
			return *this;
		}
		EMsgSlotState state;
		u32        next;
		u32        prev;

		u32        nextTop;
		u32        prevTop;

		u32        nextObj;
	};

	static u32k InvalidDepNodeId = ~u32(0);
	struct SDepNode
	{
		SDepNode() : next(InvalidDepNodeId)
		{
			++g_objcnt.mqDepNode;
		}
		~SDepNode()
		{
			--g_objcnt.mqDepNode;
		}
		SDepNode(const SDepNode& rhs)
		{
			hdl = rhs.hdl;
			next = rhs.next;
			++g_objcnt.mqDepNode;
		}
		SDepNode& operator=(const SDepNode& rhs)
		{
			hdl = rhs.hdl;
			next = rhs.next;
			return *this;
		}
		SSendableHandle hdl;
		u32          next;
	};

	static i32k LOG2_SLOTS_PER_BIN = 7;
	static i32k SLOTS_PER_BIN = (1 << LOG2_SLOTS_PER_BIN);

	struct SFreeBin
	{
		u32 elems[SLOTS_PER_BIN];

		SFreeBin()
		{
			++g_objcnt.rsbins;
		}
		~SFreeBin()
		{
			--g_objcnt.rsbins;
		}
		SFreeBin(const SFreeBin& fb)
		{
			memcpy(elems, fb.elems, sizeof(elems));
			++g_objcnt.rsbins;
		}
		SFreeBin& operator=(const SFreeBin& fb)
		{
			memcpy(elems, fb.elems, sizeof(elems));
			return *this;
		}
	};

	std::vector<SMsgSlot>   m_slots;
	std::vector<u32>     m_salt;
	std::vector<SSlotState> m_slotState;
	std::vector<u32>     m_objectHeads;
	std::vector<SDepNode>   m_depNodes;
	std::vector<u32>     m_freeSlotNumElems;
	std::vector<SFreeBin>   m_freeSlots;
	std::vector<u32>     m_freeDepLinks;

	std::vector<u32>     m_liveList;
	std::vector<SMsgSlot*>  m_recurseCache;

	SSendableHandle         m_rootSlots[eMSS_NUM_LIVE_SLOT_TYPES];
	CCTPEndpoint*           m_pParent;

	SMsgSlot*             AllocSlot(SSendableHandle& id, EMsgSlotState initState);
	SMsgSlot*             GetSlot(SSendableHandle id);
	SMsgSlot*             GetSlotSafe(SSendableHandle id);
	ELatencyClass         GetLatencyClassForGroup(float expectedTimeNow, float expectedTimeNext, SAccountingGroup* pGrp);

	void                  PrepareMessageList(const SSchedulingParams& params);
	void                  BeginAccountingFrame();
	void                  PrepareLiveList();
	void                  CalculatePerFrameData(const SSchedulingParams& params);
	void                  PatchObjectGroupings();
	void                  PatchOrderedPriorities();
	void                  WriteMessages(IMessageOutput* pOut, const SSchedulingParams& params);
	void                  RegularCleanup(const SSchedulingParams& params);
	void                  SetConfig(CConfig* pConfig, i32 version);
	void                  AddToQueue(SSendableHandle hdl);
	void                  FlushObjectHeads();

	ILINE SSendableHandle HandleFromPointer(const SMsgSlot* pSlot)
	{
		u32 idx = (u32)(pSlot - &*m_slots.begin());
		SSendableHandle ret;
		ret.id = idx;
		ret.salt = m_salt[idx];
		return ret;
	}

	SAccountingGroup* GetAccountingGroup(u32 grpId);

	class CCompareMsgEnts;
	class CCompareMsgDepth;

	SSendableHandle              m_stateChangeHandle;
	SSendableHandle              m_lastOrderedHandle;

	u32                       m_reliableSeq;

	i32                          m_nBlockingMessages;
	i32                          m_nAlertedMessages;
	i32                          m_nMessages;
	bool                         m_emptyMode;
	u32                       m_flags;
	i32                          m_version;
	bool                         m_inWrite;

	std::vector<SSendableHandle> m_pendingRemovals;

	static i32k             MAX_ACCOUNTING_GROUPS = 64;
	i32                          m_nAccountingGroups;
	SAccountingGroup             m_accountingGroups[MAX_ACCOUNTING_GROUPS];

	class CDependencyCalculator;
	class CActiveElemIterator;
	class CIncrementalSorter;
	class CCompareAccountingGroupsById;

	void                VerifyBlocking() const;
	void                ValidateHandles() const;

	ILINE EMsgSlotState GetState(u32 slot)
	{
		return m_slotState[slot].state;
	}
	ILINE void  SetState(u32 slot, EMsgSlotState state);

	tukk DebugMessageForHandle(SSendableHandle hdl);

	void        AddDependencyToSlot(SSendableHandle hdl, SMsgSlot* pSlot);
	void        FreeDependencies(SMsgSlot* pSlot);
};

#endif // !NEW_BANDWIDTH_MANAGEMENT

#endif
