// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************/
/* Player Vis Table
/************************************************************************/

#ifndef PLAYER_VIS_TABLE_H
#define PLAYER_VIS_TABLE_H

#if !defined(_RELEASE)
	#define ALLOW_VISTABLE_DEBUGGING 1
#else
	#define ALLOW_VISTABLE_DEBUGGING 0
#endif

typedef i16 VisEntryIndex;
typedef i16 TLinetestIndex;
typedef VisEntryIndex VisEntryCount;

static i32k		kMaxVisTableLinetestsPerFrame	= 5;
static const float	kVisTableDefaultZAxisOffset		= 1.4f;

enum
{
	eVQP_None									=	0,
	eVQP_UseCenterAsReference	= BIT(0),
	eVQP_IgnoreSmoke					= BIT(1),
	eVQP_IgnoreGlass					= BIT(2)
};

typedef u8 eVisibilityQueryParams;

enum
{
	eVF_None									= (0),
	eVF_Visible								= BIT(0),
	eVF_VisibleThroughGlass		= BIT(1),
	eVF_Remove								= BIT(2),
	eVF_Requested							= BIT(3),
	eVF_Pending								= BIT(4),
	eVF_CheckAgainstCenter		= BIT(5)
};

typedef u16 eVisibilityFlags;

typedef struct SVisTableEntry
{
	SVisTableEntry() : heightOffset(0.0f)
	{
		Reset();
	}

	ILINE void Reset()
	{
		entityId							= 0;
		flags									= eVF_None;
		framesSinceLastCheck	= 255;
		lastRequestedLatency	= 0;
		heightOffset					= kVisTableDefaultZAxisOffset;
	}

	EntityId									entityId;
	float										heightOffset;
	eVisibilityFlags							flags;
	u8										framesSinceLastCheck;
	u8										lastRequestedLatency;			

} SVisTableEntry;


typedef struct SDeferredLinetestReceiver
{
	SDeferredLinetestReceiver()
		: visTableIndex(-1)
		, visBufferIndex(-1)
		, queuedRayID(0)
	{
		
	}

	~SDeferredLinetestReceiver()
	{
		CancelPendingRay();
	}

	//async raycasts results callback
	void OnDataReceived(const QueuedRayID& rayID, const RayCastResult& result);

	void CancelPendingRay();

	ILINE void SetFree()			{ visBufferIndex = -1; }
	ILINE bool IsFree()				{ return visBufferIndex == -1; }	

	void SetInvalid();
	ILINE bool IsValid()			{ return visTableIndex != -1; }

	QueuedRayID						queuedRayID;
	VisEntryIndex					visTableIndex;
	int8							visBufferIndex;

} SVisTableProcessing;


typedef struct SVisTablePriority
{
	SVisTableEntry *	visInfo;
	i16							priority;
	VisEntryIndex			visIndex;
} SVisTablePriority;


//This is used to allow the vis table entries to be double buffered
typedef struct SDeferredLinetestBuffer
{
	SDeferredLinetestBuffer()
	{
		m_numLinetestsCurrentlyProcessing = 0;
	}

	SDeferredLinetestReceiver m_deferredLinetestReceivers[kMaxVisTableLinetestsPerFrame];
	VisEntryCount							m_numLinetestsCurrentlyProcessing;
} SDeferredLinetestBuffer;

#if ALLOW_VISTABLE_DEBUGGING

class CPlayerVisTableDebugDraw
{
	struct SDebugInfo
	{

		SDebugInfo()
			: m_targetId(0)
			, m_localTargetPos(ZERO)
			, m_lastUpdatedTime(0.0f)
			, m_visible(false)
		{

		}

		EntityId	m_targetId;
		Vec3		m_localTargetPos;
		float		m_lastUpdatedTime;
		bool		m_visible;
	};

	typedef std::vector<SDebugInfo>	TDebugTargets;

public:
	CPlayerVisTableDebugDraw();

	void UpdateDebugTarget(const EntityId entityId, const Vec3& worldRefPoint, bool visible);
	void Update();

private:

	SDebugInfo* GetDebugInfoForEntity(const EntityId targetId);

	TDebugTargets m_debugTargets;
};

#endif

class CPlayerVisTable
{
public:

	struct SVisibilityParams
	{
		SVisibilityParams(EntityId _targetEntityId)
			: targetEntityId(_targetEntityId)
			, heightOffset(kVisTableDefaultZAxisOffset)
			, queryParams(0)
		{

		}

		EntityId	targetEntityId;			//Target entity which is evaluated if visible or not
		float		heightOffset;
		eVisibilityQueryParams queryParams;
	};

	CPlayerVisTable();
	~CPlayerVisTable();

	bool CanLocalPlayerSee(const SVisibilityParams& target);
	bool CanLocalPlayerSee(const SVisibilityParams& target, u8 acceptableFrameLatency);
	inline SVisTableEntry& GetNthVisTableEntry(i32 n) { return m_visTableEntries[n]; }

	// Set an entity id that all vis tests should ignore when determining if a target is obscured
	// NOTE: This is DEFERRED. Ignore entity wont be respected until vistable entries next updated.
	void AddGlobalIgnoreEntity(const EntityId entId, tukk pCallerName = NULL); 

	void RemoveGlobalIgnoreEntity(const EntityId entId);
	void ClearGlobalIgnoreEntities();

	void Update(float dt);

	void Reset();
	SDeferredLinetestBuffer& GetDeferredLinetestBuffer(i32 n) { return m_linetestBuffers[n]; }

#if ALLOW_VISTABLE_DEBUGGING
	ILINE CPlayerVisTableDebugDraw& GetDebugDraw() { return m_debugDraw; };
#endif

private:
	void	DoVisibilityCheck(const Vec3& localPlayerPosn, SVisTableEntry& visInfo, VisEntryIndex visIndex);
	void	GetLocalPlayerPosn(Vec3& localPlayerPosn);

	VisEntryIndex	GetEntityIndexFromID(EntityId entityId);
	void	ClearRemovedEntities();
	i32		AddVisTableEntriesToPriorityList();
	void	UpdatePendingDeferredLinetest(const VisEntryIndex source, const VisEntryIndex dest);
	void	RemovePendingDeferredLinetest(const VisEntryIndex index);
	
	SDeferredLinetestReceiver* GetDeferredLinetestReceiverFromVisTableIndex(const VisEntryIndex index, const TLinetestIndex bufferIndex);
	SDeferredLinetestReceiver* GetAvailableDeferredLinetestReceiver(SDeferredLinetestBuffer& visBuffer);

	inline i32 GetCurrentLinetestBufferTargetIndex()			{ return m_currentBufferTarget; }

	void RemoveNthEntity(const VisEntryIndex n);
	
	static i32k		kMinUnusedFramesBeforeEntryRemoved = 20;
	static i32k		kMaxVisTableEntries							= 128;
	static i32k		kDefaultAcceptableLatency				= 10;
	static i32k		kNumVisTableBuffers							=	2;
	static u8k		kMaxNumIgnoreEntities				= 8; 

	SDeferredLinetestBuffer			m_linetestBuffers[kNumVisTableBuffers];

	SVisTableEntry			m_visTableEntries[kMaxVisTableEntries];

	SVisTablePriority 	m_visTablePriorities[kMaxVisTableLinetestsPerFrame];

	struct SIgnoreEntity
	{
		SIgnoreEntity()
		{
			Clear(); 
		}

		void Clear()
		{
			id		 = 0; 
			refCount = 0; 

#if ALLOW_VISTABLE_DEBUGGING
			requesteeName = "UNKNOWN";
#endif 
		}

		EntityId id; 
		u32 refCount;

#if ALLOW_VISTABLE_DEBUGGING
		// Since ref counted.. this will only store name of *last* requestee but ok for now. 
		DrxFixedStringT<64> requesteeName; 
#endif // #ifndef _RELEASE

	};

	SIgnoreEntity 				m_globalIgnoreEntities[kMaxNumIgnoreEntities]; 

	VisEntryCount				m_numUsedVisTableEntries;
	VisEntryCount				m_numLinetestsThisFrame;

	u8								m_currentNumIgnoreEntities;
	u8								m_currentBufferTarget;
	u8								m_currentBufferProcessing;


#if ALLOW_VISTABLE_DEBUGGING
	void UpdateIgnoreEntityDebug(); 

	i32 m_numQueriesThisFrame;
	CPlayerVisTableDebugDraw m_debugDraw;
#endif
};

#endif