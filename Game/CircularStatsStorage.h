// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/******************************************************************************
** CircularStatsStorage.h
** 14/04/10
******************************************************************************/

#ifndef __CIRCULARSTATSSTORAGE_H__
#define __CIRCULARSTATSSTORAGE_H__

#include <drx3D/Game/RecordingBuffer.h>
#include <drx3D/CoreX/Game/IGameStatistics.h>
#include <drx3D/Sys/TimeValue.h>
#include <drx3D/Game/Utility/DoubleLinkedList.h>
#include <drx3D/Game/ITelemetryCollector.h>

#if defined(_DEBUG)
#define DEBUG_CIRCULAR_STATS		1
#else
#define DEBUG_CIRCULAR_STATS		0
#endif

#if DEBUG_CIRCULAR_STATS
#include <drx3D/Game/GameMechanismBase.h>
#endif

enum
{
	eRBPT_TimelineEntry	= eRBPT_Custom,
	eRBPT_StatAnyValue,
	eRBPT_Free
};

class CCircularBufferStatsStorage : public IStatsStorageFactory
#if DEBUG_CIRCULAR_STATS
,protected CGameMechanismBase
#endif
{
	protected:
		CRecordingBuffer										m_circularBuffer;
		i32																	m_totalBytesAlloced;
		i32																	m_totalBytesRequested;
		i32																	m_peakAlloc;
		i32																	m_numDiscards;
		i32																	m_refCount;
		bool																m_serializeLocked;

		static void													DiscardCallback(SRecording_Packet *ps, float recordedTime, uk inUserData);

		virtual															~CCircularBufferStatsStorage();

		static CCircularBufferStatsStorage	*s_storage;			// it wouldn't be too difficult to extend the system to support multiple circular buffers, it would just require all allocations of
																												// timeline entries to specify which buffer they want to allocate from. for now, have a global buffer for all stats

	public:
#if DEBUG_CIRCULAR_STATS
		i32																	m_numLegacyEvents;
		i32																	m_numCircularEvents;
		virtual void												Update(float inDt)		{ DebugUpdate(); }
		void																DebugUpdate();
#endif

		static CCircularBufferStatsStorage	*GetDefaultStorage();
		static CCircularBufferStatsStorage	*GetDefaultStorageNoCheck();

																				CCircularBufferStatsStorage(size_t inBufferSize);

		void																AddRef();
		void																Release();

		virtual IStatsContainer							*CreateContainer();


		bool																ContainsPtr(ukk inPtr);
		bool																IsUsingCircularBuffer()			{ return m_circularBuffer.capacity()>0; }

		void																*Alloc(i32 inSize, u8 inType);
		static void													Free(uk inPtr);

		bool																IsDataTruncated()				{ return m_numDiscards>0; }
		i32																	GetTotalSessionMemoryRequests()	{ return m_totalBytesRequested; }
		i32																	GetBufferCapacity()				{ return m_circularBuffer.capacity(); }
		void																ResetUsageCounters();

		void																LockForSerialization();
		void																UnlockFromSerialization();
		bool																IsLockedForSerialization();
};

// an implementation of the IGameStatistics IXMLSerializable interface which can be stored in a circular buffer
// data payload in in subclass
// as entry can be purged at any time, it is not advisable to hold references to these objects
class CCircularBufferTimelineEntry : public CXMLSerializableBase
{
	protected:
		CTimeValue													m_time;

	public:
		CDoubleLinkedElement								m_timelineLink;

																				CCircularBufferTimelineEntry();
																				CCircularBufferTimelineEntry(
																					const CCircularBufferTimelineEntry		&inCopyMe);
		virtual															~CCircularBufferTimelineEntry();

		CCircularBufferTimelineEntry				&operator=(const CCircularBufferTimelineEntry &inCopyMe)
																				{
																					m_time=inCopyMe.m_time;
																					return *this;
																				}

																				// called from the storage discard callback when the memory is being freed and the object HAS to go
																				// will assert if references are still held
		void																ForceRelease();

		const CTimeValue										&GetTime() const					{ return m_time; }
		void																SetTime(const CTimeValue &inTime)	{ m_time=inTime; }

		static CCircularBufferTimelineEntry *EntryFromListElement(const CDoubleLinkedElement *inElement);

		// allocations will come from the circular pool and be deleted if purged due to lack of space
		// there is potential to allocate from different circular storage pools as required, but in order to ease transition there is a default one set and multiple ones aren't currently supported
		void																*operator new(size_t inSize) throw()												{ return CCircularBufferStatsStorage::GetDefaultStorage()->Alloc(inSize,eRBPT_TimelineEntry); }
		void																*operator new(size_t inSize, CCircularBufferStatsStorage *inStorage) throw()		{ return inStorage->Alloc(inSize,eRBPT_TimelineEntry); }
		void																operator delete(uk inPtr)												{ CCircularBufferStatsStorage::Free(inPtr); }
};

// simple storage for a stat any value
struct SCircularStatAnyValue
{
	SStatAnyValue							val;
	CTimeValue								time;
	CDoubleLinkedElement					link;

											SCircularStatAnyValue()
											{
											}

											SCircularStatAnyValue(const SCircularStatAnyValue &inCopyMe)
											{
												*this=inCopyMe;
											}

	SCircularStatAnyValue					&operator=(const SCircularStatAnyValue &inCopyMe)
											{
												val=inCopyMe.val;
												time=inCopyMe.time;
												return *this;
											}

	void									*operator new(size_t inSize, CCircularBufferStatsStorage *inStorage) throw()		{ return inStorage->Alloc(inSize,eRBPT_StatAnyValue); }
	void									operator delete(uk inPtr, CCircularBufferStatsStorage *inStorage)		{ CCircularBufferStatsStorage::Free(inPtr); }
	void									operator delete(uk inPtr)												{ CCircularBufferStatsStorage::Free(inPtr); }

	static SCircularStatAnyValue			*EntryFromListElement(const CDoubleLinkedElement *inElement);
};

// a timeline of events. the type specifies the type of allocation and all events on the time line must be of the same type
class CCircularBufferTimeline
{
#ifndef _RELEASE
	friend class CCircularBufferStatsContainer;
#endif

	protected:
		u8								m_type;						// eRBPT_* enum

	public:
		CDoubleLinkedList					m_list;

											CCircularBufferTimeline() :
												m_type(eRBPT_Invalid)
											{
											}

		void								SetType(
												u8		inType)
											{
												DRX_ASSERT_MESSAGE(m_type==eRBPT_Invalid || m_type==inType,"Cannot intermix different types of stats on the same event time line");
												m_type=inType;
											}
		u8								GetType() const
											{
												return m_type;
											}
};


// implementation of a stats container that uses the circular buffer to manage its allocations
class CCircularBufferStatsContainer : public IStatsContainer
{
	protected:
		CCircularBufferStatsStorage			*m_storage;
		CCircularBufferTimeline					*m_timelines;
		SStatAnyValue										*m_states;
		i32															m_refCount;
		size_t													m_numTimelines;
		size_t													m_numStates;

		CCircularBufferTimeline					*GetMutableTimeline(size_t inTimelineId);
		SStatAnyValue										*GetMutableState(size_t inStateId);

		inline const char								*GetEventName(size_t inEventId);

																		~CCircularBufferStatsContainer();

	public:
																		CCircularBufferStatsContainer(CCircularBufferStatsStorage *inStorage);

		virtual void										Init(size_t numEvents, size_t numStates);
		virtual void										AddRef();
		virtual void										Release();

		virtual void										AddEvent(size_t eventID, const CTimeValue& time, const SStatAnyValue& val);
		virtual void										AddState(size_t stateID, const SStatAnyValue& val);

		virtual size_t									GetEventTrackLength(size_t eventID) const;
		virtual void										GetEventInfo(size_t eventID, size_t idx, CTimeValue& outTime, SStatAnyValue& outParam) const;
		virtual void										GetStateInfo(size_t stateID, SStatAnyValue& outValue) const;

		virtual void										Clear();
		virtual bool										IsEmpty() const;

		virtual void										GetMemoryStatistics(IDrxSizer *pSizer);

		const CCircularBufferTimeline		*GetTimeline(size_t inTimelineId) const;
		const SStatAnyValue							*GetState(size_t inStateId) const;
		bool														HasAnyTimelineEvents() const;

#ifndef _RELEASE
		void														Validate() const;
#endif
};

class CCircularXMLSerializer : public IStatsSerializer, public ITelemetryProducer
{
	protected:
		enum ESerializeAction
		{
			k_openTag,
			k_closeTag
		};
		struct SSerializeEntry
		{
			ESerializeAction														action;
			DrxFixedStringT<64>													tagName;
			IStatsContainerPtr													pContainer;
		};
		typedef std::vector<SSerializeEntry>					TEntries;
		enum EState
		{
			k_notStartedProducing,
			k_producing
		};

		struct SWriteState
		{
			char			*pBuffer;
			i32				bufferSize;
			i32				dataWritten;
			bool			full;
		};

		TEntries																			m_entries;
		_smart_ptr<CCircularBufferStatsStorage>				m_pStorage;
		EState																				m_state;

		i32																						m_containerIterator;
		CDoubleLinkedList::const_iterator							m_eventIterator;
		i32																						m_timelineIterator;
		i32																						m_stateIterator;
		i32																						m_indentLevel;
		char																					m_indentStr[32];				// max indentation

		void																					SerializeContainerTag(
																										SWriteState											*pIOState,
																										CCircularBufferStatsContainer		*pInCont,
																										IGameStatistics									*pInStats);
		void																					SerializeTimeline(
																										SWriteState											*pIOState,
																										CCircularBufferStatsContainer		*pInCont,
																										IGameStatistics									*pInStats);
		void																					SerializeTimelines(
																										SWriteState											*pIOState,
																										CCircularBufferStatsContainer		*pInCont,
																										IGameStatistics									*pInStats);
		void																					SerializeStates(
																										SWriteState											*pIOState,
																										CCircularBufferStatsContainer		*pInCont,
																										IGameStatistics									*pInStats);

		void																					IncreaseIndentation(
																										i32										inDelta);
		bool																					Output(
																										SWriteState						*pIOState,
																										const char						*pInDataToWrite,
																										i32										inDataLenToWrite,
																										bool									inDoIndent);

	public:
																									CCircularXMLSerializer(
																										CCircularBufferStatsStorage		*pInStorage);
		virtual																				~CCircularXMLSerializer();

		virtual void																	VisitNode(
																										const SNodeLocator&		locator,
																										tukk 						serializeName,
																										IStatsContainer&			container,
																										EStatNodeState				state);
		virtual void																	LeaveNode(
																										const SNodeLocator&		locator,
																										tukk 						serializeName,
																										IStatsContainer&			container,
																										EStatNodeState				state);
		virtual EResult																ProduceTelemetry(
																										char									*pOutBuffer,
																										i32										inMinRequired,
																										i32										inBufferSize,
																										i32										*pOutWritten);
};

#endif // __CIRCULARSTATSSTORAGE_H__

