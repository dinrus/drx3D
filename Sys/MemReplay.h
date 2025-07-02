// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MemReplay_h__
#define __MemReplay_h__

#define REPLAY_RECORD_FREECS        1
#define REPLAY_RECORD_USAGE_CHANGES 0
#define REPLAY_RECORD_THREADED      1
#define REPLAY_RECORD_CONTAINERS    0

#if CAPTURE_REPLAY_LOG || ENABLE_STATOSCOPE

	#include <drx3D/Network/DrxSocks.h>
	#include <drx3D/CoreX/Thread/IThreadUpr.h>

	#if !(DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID)
		#define REPLAY_SOCKET_WRITER
	#endif

class IReplayWriter
{
public:
	virtual ~IReplayWriter() {}

	virtual bool        Open() = 0;
	virtual void        Close() = 0;

	virtual tukk GetFilename() const = 0;
	virtual uint64      GetWrittenLength() const = 0;

	virtual i32         Write(ukk data, size_t sz, size_t n) = 0;
	virtual void        Flush() = 0;
};

class ReplayDiskWriter : public IReplayWriter
{
public:
	explicit ReplayDiskWriter(tukk pSuffix);

	bool        Open();
	void        Close();

	tukk GetFilename() const      { return m_filename; }
	uint64      GetWrittenLength() const { return m_written; }

	i32         Write(ukk data, size_t sz, size_t n);
	void        Flush();

private:
	char   m_filename[MAX_PATH];

	FILE*  m_fp;
	uint64 m_written;
};

class ReplaySocketWriter : public IReplayWriter
{
public:
	explicit ReplaySocketWriter(tukk address);

	bool        Open();
	void        Close();

	tukk GetFilename() const      { return "socket"; }
	uint64      GetWrittenLength() const { return m_written; }

	i32         Write(ukk data, size_t sz, size_t n);
	void        Flush();

private:
	char      m_address[128];
	u16    m_port;

	DRXSOCKET m_sock;
	uint64    m_written;
};

#endif

#if CAPTURE_REPLAY_LOG

	#include <drx3D/Sys/IDrxPak.h>
	#include <drx3D/CoreX/Memory/HeapAllocator.h>
	#include  <drx/Core/lib/z/zlib.h>

namespace MemReplayEventIds
{
enum Ids
{
	RE_Alloc,
	RE_Free,
	RE_Callstack,
	RE_FrameStart,
	RE_Label,
	RE_ModuleRef,
	RE_AllocVerbose,
	RE_FreeVerbose,
	RE_Info,
	RE_PushContext,
	RE_PopContext,
	RE_Alloc3,
	RE_Free3,
	RE_PushContext2,
	RE_ModuleShortRef,
	RE_AddressProfile,
	RE_PushContext3,
	RE_Free4,
	RE_AllocUsage,
	RE_Info2,
	RE_Screenshot,
	RE_SizerPush,
	RE_SizerPop,
	RE_SizerAddRange,
	RE_AddressProfile2,
	RE_Alloc64,
	RE_Free64,
	RE_BucketMark,
	RE_BucketMark2,
	RE_BucketUnMark,
	RE_BucketUnMark2,
	RE_PoolMark,
	RE_PoolUnMark,
	RE_TextureAllocContext,
	RE_PageFault,
	RE_AddAllocReference,
	RE_RemoveAllocReference,
	RE_PoolMark2,
	RE_TextureAllocContext2,
	RE_BucketCleanupEnabled,
	RE_Info3,
	RE_Alloc4,
	RE_Realloc,
	RE_RegisterContainer,
	RE_UnregisterContainer,
	RE_BindToContainer,
	RE_UnbindFromContainer,
	RE_RegisterFixedAddressRange,
	RE_SwapContainers,
	RE_MapPage,
	RE_UnMapPage,
	RE_ModuleUnRef,
	RE_Alloc5,
	RE_Free5,
	RE_Realloc2,
	RE_AllocUsage2,
	RE_Alloc6,
	RE_Free6,
	RE_Realloc3,
	RE_UnregisterAddressRange,
	RE_MapPage2,
};
}

namespace MemReplayPlatformIds
{
enum Ids
{
	PI_Unknown = 0,
	// PI_360,
	// PI_PS3,
	PI_PC = 3,
	PI_Durango,
	PI_Orbis
};
};

	#pragma pack(push)
	#pragma pack(1)

struct MemReplayLogHeader
{
	MemReplayLogHeader(u32 tag, u32 platform, u32 pointerWidth)
		: tag(tag)
		, platform(platform)
		, pointerWidth(pointerWidth)
	{
	}

	u32 tag;
	u32 platform;
	u32 pointerWidth;
} __PACKED;

struct MemReplayEventHeader
{
	MemReplayEventHeader(i32 id, size_t size, u8 sequenceCheck)
		: sequenceCheck(sequenceCheck)
		, eventId(static_cast<u8>(id))
		, eventLength(static_cast<u16>(size))
	{
	}

	u8  sequenceCheck;
	u8  eventId;
	u16 eventLength;
} __PACKED;

struct MemReplayFrameStartEvent
{
	static i32k EventId = MemReplayEventIds::RE_FrameStart;

	u32           frameId;

	MemReplayFrameStartEvent(u32 frameId)
		: frameId(frameId)
	{}
} __PACKED;

struct MemReplayLabelEvent
{
	static i32k EventId = MemReplayEventIds::RE_Label;

	char             label[1];

	MemReplayLabelEvent(tukk label)
	{
		// Assume there is room beyond this instance.
		strcpy(this->label, label); // we're intentionally writing beyond the end of this array, so don't use drx_strcpy()
	}
} __PACKED;

struct MemReplayPushContextEvent
{
	static i32k EventId = MemReplayEventIds::RE_PushContext3;

	u32           threadId;
	u32           contextType;
	u32           flags;

	// This field must be the last in the structure, and enough memory should be allocated
	// for the structure to hold the required name.
	char name[1];

	MemReplayPushContextEvent(u32 threadId, tukk name, EMemStatContextTypes::Type type, u32 flags)
	{
		// We're going to assume that there actually is enough space to store the name directly in the struct.

		this->threadId = threadId;
		this->contextType = static_cast<u32>(type);
		this->flags = flags;
		strcpy(this->name, name); // we're intentionally writing beyond the end of this array, so don't use drx_strcpy()
	}
} __PACKED;

struct MemReplayPopContextEvent
{
	static i32k EventId = MemReplayEventIds::RE_PopContext;

	u32           threadId;

	explicit MemReplayPopContextEvent(u32 threadId)
	{
		this->threadId = threadId;
	}
} __PACKED;

struct MemReplayModuleRefEvent
{
	static i32k EventId = MemReplayEventIds::RE_ModuleRef;

	char             name[256];
	char             path[256];
	char             sig[512];
	UINT_PTR         address;
	UINT_PTR         size;

	MemReplayModuleRefEvent(tukk name, tukk path, const UINT_PTR address, UINT_PTR size, tukk sig)
	{
		drx_strcpy(this->name, name);
		drx_strcpy(this->path, path);
		drx_strcpy(this->sig, sig);
		this->address = address;
		this->size = size;
	}
} __PACKED;

struct MemReplayModuleUnRefEvent
{
	static i32k EventId = MemReplayEventIds::RE_ModuleUnRef;

	UINT_PTR         address;

	MemReplayModuleUnRefEvent(UINT_PTR address)
		: address(address) {}
} __PACKED;

struct MemReplayModuleRefShortEvent
{
	static i32k EventId = MemReplayEventIds::RE_ModuleShortRef;

	char             name[256];

	MemReplayModuleRefShortEvent(tukk name)
	{
		drx_strcpy(this->name, name);
	}
} __PACKED;

struct MemReplayAllocEvent
{
	static i32k EventId = MemReplayEventIds::RE_Alloc6;

	u32           threadId;
	UINT_PTR         id;
	u32           alignment;
	u32           sizeRequested;
	u32           sizeConsumed;
	i32            sizeGlobal; //  Inferred from changes in global memory status

	u16           moduleId;
	u16           allocClass;
	u16           allocSubClass;
	u16           callstackLength;
	UINT_PTR         callstack[1]; // Must be last.

	MemReplayAllocEvent(u32 threadId, u16 moduleId, u16 allocClass, u16 allocSubClass, UINT_PTR id, u32 alignment, u32 sizeReq, u32 sizeCon, i32 sizeGlobal)
		: threadId(threadId)
		, id(id)
		, alignment(alignment)
		, sizeRequested(sizeReq)
		, sizeConsumed(sizeCon)
		, sizeGlobal(sizeGlobal)
		, moduleId(moduleId)
		, allocClass(allocClass)
		, allocSubClass(allocSubClass)
		, callstackLength(0)
	{
	}
} __PACKED;

struct MemReplayFreeEvent
{
	static i32k EventId = MemReplayEventIds::RE_Free6;

	u32           threadId;
	UINT_PTR         id;
	i32            sizeGlobal; //  Inferred from changes in global memory status

	u16           moduleId;
	u16           allocClass;
	u16           allocSubClass;

	u16           callstackLength;
	UINT_PTR         callstack[1]; // Must be last.

	MemReplayFreeEvent(u32 threadId, u16 moduleId, u16 allocClass, u16 allocSubClass, UINT_PTR id, i32 sizeGlobal)
		: threadId(threadId)
		, id(id)
		, sizeGlobal(sizeGlobal)
		, moduleId(moduleId)
		, allocClass(allocClass)
		, allocSubClass(allocSubClass)
		, callstackLength(0)
	{
	}
} __PACKED;

struct MemReplayInfoEvent
{
	static i32k EventId = MemReplayEventIds::RE_Info3;

	u32           executableSize;
	u32           initialGlobalSize;
	u32           bucketsFree;

	MemReplayInfoEvent(u32 executableSize, u32 initialGlobalSize, u32 bucketsFree)
		: executableSize(executableSize)
		, initialGlobalSize(initialGlobalSize)
		, bucketsFree(bucketsFree)
	{
	}
} __PACKED;

struct MemReplayAddressProfileEvent
{
	static i32k EventId = MemReplayEventIds::RE_AddressProfile2;

	UINT_PTR         rsxStart;
	u32           rsxLength;

	MemReplayAddressProfileEvent(UINT_PTR rsxStart, u32 rsxLength)
		: rsxStart(rsxStart)
		, rsxLength(rsxLength)
	{
	}
} __PACKED;

struct MemReplayAllocUsageEvent
{
	static i32k EventId = MemReplayEventIds::RE_AllocUsage2;

	u32           allocClass;
	UINT_PTR         id;
	u32           used;

	MemReplayAllocUsageEvent(u16 allocClass, UINT_PTR id, u32 used)
		: allocClass(allocClass)
		, id(id)
		, used(used)
	{
	}
} __PACKED;

struct MemReplayScreenshotEvent
{
	static i32k EventId = MemReplayEventIds::RE_Screenshot;

	u8            bmp[1];

	MemReplayScreenshotEvent()
	{
	}
} __PACKED;

struct MemReplaySizerPushEvent
{
	static i32k EventId = MemReplayEventIds::RE_SizerPush;

	char             name[1];

	MemReplaySizerPushEvent(tukk name)
	{
		strcpy(this->name, name);
	}
} __PACKED;

struct MemReplaySizerPopEvent
{
	static i32k EventId = MemReplayEventIds::RE_SizerPop;
} __PACKED;

struct MemReplaySizerAddRangeEvent
{
	static i32k EventId = MemReplayEventIds::RE_SizerAddRange;

	UINT_PTR         address;
	u32           size;
	i32            count;

	MemReplaySizerAddRangeEvent(const UINT_PTR address, u32 size, i32 count)
		: address(address)
		, size(size)
		, count(count)
	{}

} __PACKED;

struct MemReplayBucketMarkEvent
{
	static i32k EventId = MemReplayEventIds::RE_BucketMark2;

	UINT_PTR         address;
	u32           length;
	i32            index;
	u32           alignment;

	MemReplayBucketMarkEvent(UINT_PTR address, u32 length, i32 index, u32 alignment)
		: address(address)
		, length(length)
		, index(index)
		, alignment(alignment)
	{}

} __PACKED;

struct MemReplayBucketUnMarkEvent
{
	static i32k EventId = MemReplayEventIds::RE_BucketUnMark2;

	UINT_PTR         address;
	i32            index;

	MemReplayBucketUnMarkEvent(UINT_PTR address, i32 index)
		: address(address)
		, index(index)
	{}
} __PACKED;

struct MemReplayAddAllocReferenceEvent
{
	static i32k EventId = MemReplayEventIds::RE_AddAllocReference;

	UINT_PTR         address;
	UINT_PTR         referenceId;
	u32           callstackLength;
	UINT_PTR         callstack[1];

	MemReplayAddAllocReferenceEvent(UINT_PTR address, UINT_PTR referenceId)
		: address(address)
		, referenceId(referenceId)
		, callstackLength(0)
	{
	}
} __PACKED;

struct MemReplayRemoveAllocReferenceEvent
{
	static i32k EventId = MemReplayEventIds::RE_RemoveAllocReference;

	UINT_PTR         referenceId;

	MemReplayRemoveAllocReferenceEvent(UINT_PTR referenceId)
		: referenceId(referenceId)
	{
	}
} __PACKED;

struct MemReplayPoolMarkEvent
{
	static i32k EventId = MemReplayEventIds::RE_PoolMark2;

	UINT_PTR         address;
	u32           length;
	i32            index;
	u32           alignment;
	char             name[1];

	MemReplayPoolMarkEvent(UINT_PTR address, u32 length, i32 index, u32 alignment, tukk name)
		: address(address)
		, length(length)
		, index(index)
		, alignment(alignment)
	{
		strcpy(this->name, name);
	}

} __PACKED;

struct MemReplayPoolUnMarkEvent
{
	static i32k EventId = MemReplayEventIds::RE_PoolUnMark;

	UINT_PTR         address;
	i32            index;

	MemReplayPoolUnMarkEvent(UINT_PTR address, i32 index)
		: address(address)
		, index(index)
	{}
} __PACKED;

struct MemReplayTextureAllocContextEvent
{
	static i32k EventId = MemReplayEventIds::RE_TextureAllocContext2;

	UINT_PTR         address;
	u32           mip;
	u32           width;
	u32           height;
	u32           flags;
	char             name[1];

	MemReplayTextureAllocContextEvent(UINT_PTR ptr, u32 mip, u32 width, u32 height, u32 flags, tukk name)
		: address(ptr)
		, mip(mip)
		, width(width)
		, height(height)
		, flags(flags)
	{
		strcpy(this->name, name);
	}
} __PACKED;

struct MemReplayBucketCleanupEnabledEvent
{
	static i32k EventId = MemReplayEventIds::RE_BucketCleanupEnabled;

	UINT_PTR         allocatorBaseAddress;
	u32           cleanupsEnabled;

	MemReplayBucketCleanupEnabledEvent(UINT_PTR allocatorBaseAddress, bool cleanupsEnabled)
		: allocatorBaseAddress(allocatorBaseAddress)
		, cleanupsEnabled(cleanupsEnabled ? 1 : 0)
	{
	}
} __PACKED;

struct MemReplayReallocEvent
{
	static i32k EventId = MemReplayEventIds::RE_Realloc3;

	u32           threadId;
	UINT_PTR         oldId;
	UINT_PTR         newId;
	u32           alignment;
	u32           newSizeRequested;
	u32           newSizeConsumed;
	i32            sizeGlobal; //  Inferred from changes in global memory status

	u16           moduleId;
	u16           allocClass;
	u16           allocSubClass;

	u16           callstackLength;
	UINT_PTR         callstack[1]; // Must be last.

	MemReplayReallocEvent(u32 threadId, u16 moduleId, u16 allocClass, u16 allocSubClass, UINT_PTR oldId, UINT_PTR newId, u32 alignment, u32 newSizeReq, u32 newSizeCon, i32 sizeGlobal)
		: threadId(threadId)
		, oldId(oldId)
		, newId(newId)
		, alignment(alignment)
		, newSizeRequested(newSizeReq)
		, newSizeConsumed(newSizeCon)
		, sizeGlobal(sizeGlobal)
		, moduleId(moduleId)
		, allocClass(allocClass)
		, allocSubClass(allocSubClass)
		, callstackLength(0)
	{
	}
} __PACKED;

struct MemReplayRegisterContainerEvent
{
	static i32k EventId = MemReplayEventIds::RE_RegisterContainer;

	UINT_PTR         key;
	u32           type;

	u32           callstackLength;
	UINT_PTR         callstack[1]; // Must be last

	MemReplayRegisterContainerEvent(UINT_PTR key, u32 type)
		: key(key)
		, type(type)
		, callstackLength(0)
	{
	}

} __PACKED;

struct MemReplayUnregisterContainerEvent
{
	static i32k EventId = MemReplayEventIds::RE_UnregisterContainer;

	UINT_PTR         key;

	explicit MemReplayUnregisterContainerEvent(UINT_PTR key)
		: key(key)
	{
	}

} __PACKED;

struct MemReplayBindToContainerEvent
{
	static i32k EventId = MemReplayEventIds::RE_BindToContainer;

	UINT_PTR         key;
	UINT_PTR         ptr;

	MemReplayBindToContainerEvent(UINT_PTR key, UINT_PTR ptr)
		: key(key)
		, ptr(ptr)
	{
	}

} __PACKED;

struct MemReplayUnbindFromContainerEvent
{
	static i32k EventId = MemReplayEventIds::RE_UnbindFromContainer;

	UINT_PTR         key;
	UINT_PTR         ptr;

	MemReplayUnbindFromContainerEvent(UINT_PTR key, UINT_PTR ptr)
		: key(key)
		, ptr(ptr)
	{
	}

} __PACKED;

struct MemReplayRegisterFixedAddressRangeEvent
{
	static i32k EventId = MemReplayEventIds::RE_RegisterFixedAddressRange;

	UINT_PTR         address;
	u32           length;

	char             name[1];

	MemReplayRegisterFixedAddressRangeEvent(UINT_PTR address, u32 length, tukk name)
		: address(address)
		, length(length)
	{
		strcpy(this->name, name);
	}
} __PACKED;

struct MemReplaySwapContainersEvent
{
	static i32k EventId = MemReplayEventIds::RE_SwapContainers;

	UINT_PTR         keyA;
	UINT_PTR         keyB;

	MemReplaySwapContainersEvent(UINT_PTR keyA, UINT_PTR keyB)
		: keyA(keyA)
		, keyB(keyB)
	{
	}
} __PACKED;

struct MemReplayMapPageEvent
{
	static i32k EventId = MemReplayEventIds::RE_MapPage2;

	UINT_PTR         address;
	u32           length;
	u32           callstackLength;
	UINT_PTR         callstack[1];

	MemReplayMapPageEvent(UINT_PTR address, u32 length)
		: address(address)
		, length(length)
		, callstackLength(0)
	{
	}
} __PACKED;

struct MemReplayUnMapPageEvent
{
	static i32k EventId = MemReplayEventIds::RE_UnMapPage;

	UINT_PTR         address;
	u32           length;

	MemReplayUnMapPageEvent(UINT_PTR address, u32 length)
		: address(address)
		, length(length)
	{
	}
} __PACKED;

	#pragma pack(pop)

	#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE

class ReplayAllocatorBase
{
public:
	enum
	{
		PageSize = 4096
	};

public:
	uk ReserveAddressSpace(size_t sz);
	void  UnreserveAddressSpace(uk base, uk committedEnd);

	uk MapAddressSpace(uk addr, size_t sz);
};

	#elif DRX_PLATFORM_ORBIS

class ReplayAllocatorBase
{
public:
	enum
	{
		PageSize = 65536
	};

public:
	uk ReserveAddressSpace(size_t sz);
	void  UnreserveAddressSpace(uk base, uk committedEnd);

	uk MapAddressSpace(uk addr, size_t sz);
};

	#endif

class ReplayAllocator : private ReplayAllocatorBase
{
public:
	ReplayAllocator();
	~ReplayAllocator();

	void   Reserve(size_t sz);

	uk  Allocate(size_t sz);
	void   Free();

	size_t GetCommittedSize() const { return static_cast<size_t>(reinterpret_cast<INT_PTR>(m_commitEnd) - reinterpret_cast<INT_PTR>(m_heap)); }

private:
	ReplayAllocator(const ReplayAllocator&);
	ReplayAllocator& operator=(const ReplayAllocator&);

private:
	enum
	{
		MaxSize = 6 * 1024 * 1024
	};

private:
	DrxCriticalSectionNonRecursive m_lock;

	LPVOID                         m_heap;
	LPVOID                         m_commitEnd;
	LPVOID                         m_allocEnd;
};

class ReplayCompressor
{
public:
	ReplayCompressor(ReplayAllocator& allocator, IReplayWriter* writer);
	~ReplayCompressor();

	void Flush();
	void Write(u8k* data, size_t len);

private:
	enum
	{
		CompressTargetSize = 64 * 1024
	};

private:
	static voidpf zAlloc(voidpf opaque, uInt items, uInt size);
	static void   zFree(voidpf opaque, voidpf address);

private:
	ReplayCompressor(const ReplayCompressor&);
	ReplayCompressor& operator=(const ReplayCompressor&);

private:
	ReplayAllocator* m_allocator;
	IReplayWriter*   m_writer;

	u8*           m_compressTarget;
	z_stream         m_compressStream;
	i32              m_zSize;
};

	#if REPLAY_RECORD_THREADED

class ReplayRecordThread : public IThread
{
public:
	ReplayRecordThread(ReplayCompressor* compressor);

	void Flush();
	void Write(u8k* data, size_t len);

public:
	// Start accepting work on thread
	virtual void ThreadEntry();

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork();

private:
	enum Command
	{
		CMD_Idle = 0,
		CMD_Write,
		CMD_Flush,
		CMD_Stop
	};

private:
	DrxMutex              m_mtx;
	DrxConditionVariable  m_cond;

	ReplayCompressor*     m_compressor;
	 Command      m_nextCommand;
	u8k*  m_nextData;
	long          m_nextLength;
};
	#endif

class ReplayLogStream
{
public:
	ReplayLogStream();
	~ReplayLogStream();

	bool              Open(tukk openString);
	void              Close();

	bool              IsOpen() const { return m_isOpen != 0; }
	tukk const GetFilename()  { assert(m_writer); return m_writer->GetFilename(); }

	bool              EnableAsynchMode();

	template<typename T>
	ILINE T* BeginAllocateRawEvent(size_t evAdditionalReserveLength)
	{
		return reinterpret_cast<T*>(BeginAllocateRaw(sizeof(T) + evAdditionalReserveLength));
	}

	template<typename T>
	ILINE void EndAllocateRawEvent(size_t evAdditionalLength)
	{
		EndAllocateRaw(T::EventId, sizeof(T) + evAdditionalLength);
	}

	template<typename T>
	ILINE T* AllocateRawEvent(size_t evAdditionalLength)
	{
		return reinterpret_cast<T*>(AllocateRaw(T::EventId, sizeof(T) + evAdditionalLength));
	}

	template<typename T>
	ILINE T* AllocateRawEvent()
	{
		return AllocateRaw(T::EventId, sizeof(T));
	}

	void WriteRawEvent(u8 id, ukk event, uint eventSize)
	{
		uk ev = AllocateRaw(id, eventSize);
		memcpy(ev, event, eventSize);
	}

	template<typename T>
	ILINE void WriteEvent(const T* ev, size_t evAdditionalLength)
	{
		WriteRawEvent(T::EventId, ev, sizeof(T) + evAdditionalLength);
	}

	template<typename T>
	ILINE void WriteEvent(const T& ev)
	{
		WriteRawEvent(T::EventId, &ev, sizeof(T));
	}

	void   Flush();
	void   FullFlush();

	size_t GetSize() const;
	uint64 GetWrittenLength() const;
	uint64 GetUncompressedLength() const;

private:
	ReplayLogStream(const ReplayLogStream&);
	ReplayLogStream& operator=(const ReplayLogStream&);

private:
	uk BeginAllocateRaw(size_t evReserveLength)
	{
		u32 size = sizeof(MemReplayEventHeader) + evReserveLength;
		size = (size + 3) & ~3;
		evReserveLength = size - sizeof(MemReplayEventHeader);

		if (((&m_buffer[m_bufferSize]) - m_bufferEnd) < (i32)size)
			Flush();

		return reinterpret_cast<MemReplayEventHeader*>(m_bufferEnd) + 1;
	}

	void EndAllocateRaw(u8 id, size_t evLength)
	{
		u32 size = sizeof(MemReplayEventHeader) + evLength;
		size = (size + 3) & ~3;
		evLength = size - sizeof(MemReplayEventHeader);

		MemReplayEventHeader* header = reinterpret_cast<MemReplayEventHeader*>(m_bufferEnd);
		new(header) MemReplayEventHeader(id, evLength, m_sequenceCheck);

		++m_sequenceCheck;
		m_bufferEnd += size;
	}

	uk AllocateRaw(u8 id, uint eventSize)
	{
		uk result = BeginAllocateRaw(eventSize);
		EndAllocateRaw(id, eventSize);
		return result;
	}

private:
	ReplayAllocator   m_allocator;

	 i32      m_isOpen;

	u8*            m_buffer;
	size_t            m_bufferSize;
	u8*            m_bufferEnd;

	uint64            m_uncompressedLen;
	u8             m_sequenceCheck;

	u8*            m_bufferA;
	ReplayCompressor* m_compressor;

	#if REPLAY_RECORD_THREADED
	u8*              m_bufferB;
	ReplayRecordThread* m_recordThread;
	#endif

	IReplayWriter* m_writer;
};

class MemReplayDrxSizer : public IDrxSizer
{
public:
	MemReplayDrxSizer(ReplayLogStream& stream, tukk name);
	~MemReplayDrxSizer();

	virtual void                Release() { delete this; }

	virtual size_t              GetTotalSize();
	virtual size_t              GetObjectCount();
	virtual void                Reset();
	virtual void                End() {}

	virtual bool                AddObject(ukk pIdentifier, size_t nSizeBytes, i32 nCount = 1);

	virtual IResourceCollector* GetResourceCollector();
	virtual void                SetResourceCollector(IResourceCollector*);

	virtual void                Push(tukk szComponentName);
	virtual void                PushSubcomponent(tukk szSubcomponentName);
	virtual void                Pop();

private:
	ReplayLogStream*      m_stream;

	size_t                m_totalSize;
	size_t                m_totalCount;
	std::set<ukk> m_addedObjects;
};

extern i32 GetPageBucketAlloc_wasted_in_allocation();
extern i32 GetPageBucketAlloc_get_free();

class CReplayModules
{
public:
	struct ModuleLoadDesc
	{
		UINT_PTR address;
		UINT_PTR size;
		char     name[256];
		char     path[256];
		char     sig[512];
	};

	struct ModuleUnloadDesc
	{
		UINT_PTR address;
	};

	typedef void (* FModuleLoad)(uk , const ModuleLoadDesc& desc);
	typedef void (* FModuleUnload)(uk , const ModuleUnloadDesc& desc);

public:
	CReplayModules()
		: m_numKnownModules(0)
	{
	}

	void RefreshModules(FModuleLoad onLoad, FModuleUnload onUnload, uk pUser);

private:
	struct Module
	{
		UINT_PTR baseAddress;
		UINT_PTR size;
		UINT     timestamp;

		Module(){}
		Module(UINT_PTR ba, UINT_PTR s, UINT t)
			: baseAddress(ba), size(s), timestamp(t) {}

		friend bool operator<(Module const& a, Module const& b)
		{
			if (a.baseAddress != b.baseAddress) return a.baseAddress < b.baseAddress;
			if (a.size != b.size) return a.size < b.size;
			return a.timestamp < b.timestamp;
		}
	};

	#if DRX_PLATFORM_DURANGO || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	struct EnumModuleState
	{
		CReplayModules* pSelf;
		HANDLE          hProcess;
		FModuleLoad     onLoad;
		uk           pOnLoadUser;
	};
	#endif

private:
	#if DRX_PLATFORM_DURANGO
	static BOOL EnumModules_Durango(PCSTR moduleName, DWORD64 moduleBase, ULONG moduleSize, PVOID userContext);
	#endif

	#if DRX_PLATFORM_ORBIS
	void RefreshModules_Orbis(FModuleLoad onLoad, uk pUser);
	#endif

	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	static i32 EnumModules_Linux(struct dl_phdr_info* info, size_t size, uk userContext);
	#endif

private:
	Module m_knownModules[128];
	size_t m_numKnownModules;
};

//////////////////////////////////////////////////////////////////////////
class CMemReplay : public IMemReplay
{
public:
	static CMemReplay* GetInstance();

public:
	CMemReplay();
	~CMemReplay();

	//////////////////////////////////////////////////////////////////////////
	// IMemReplay interface implementation
	//////////////////////////////////////////////////////////////////////////
	void DumpStats();
	void DumpSymbols();

	void StartOnCommandLine(tukk cmdLine);
	void Start(bool bPaused, tukk openString);
	void Stop();
	void Flush();
	bool EnableAsynchMode();

	void GetInfo(DrxReplayInfo& infoOut);

	// Call to begin a new allocation scope.
	bool EnterScope(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId);

	// Records an event against the currently active scope and exits it.
	void ExitScope_Alloc(UINT_PTR id, UINT_PTR sz, UINT_PTR alignment = 0);
	void ExitScope_Realloc(UINT_PTR originalId, UINT_PTR newId, UINT_PTR sz, UINT_PTR alignment = 0);
	void ExitScope_Free(UINT_PTR id);
	void ExitScope();

	bool EnterLockScope();
	void LeaveLockScope();

	void AllocUsage(EMemReplayAllocClass::Class allocClass, UINT_PTR id, UINT_PTR used);

	void AddAllocReference(uk ptr, uk ref);
	void RemoveAllocReference(uk ref);

	void AddLabel(tukk label);
	void AddLabelFmt(tukk labelFmt, ...);
	void AddFrameStart();
	void AddScreenshot();

	void AddContext(i32 type, u32 flags, tukk str);
	void AddContextV(i32 type, u32 flags, tukk format, va_list args);
	void RemoveContext();

	void OwnMemory(uk p, size_t size);

	void MapPage(uk base, size_t size);
	void UnMapPage(uk base, size_t size);

	void RegisterFixedAddressRange(uk base, size_t size, tukk name);
	void MarkBucket(i32 bucket, size_t alignment, uk base, size_t length);
	void UnMarkBucket(i32 bucket, uk base);
	void BucketEnableCleanups(uk allocatorBase, bool enabled);

	void MarkPool(i32 pool, size_t alignment, uk base, size_t length, tukk name);
	void UnMarkPool(i32 pool, uk base);
	void AddTexturePoolContext(uk ptr, i32 mip, i32 width, i32 height, tukk name, u32 flags);

	void AddSizerTree(tukk name);

	void RegisterContainer(ukk key, i32 type);
	void UnregisterContainer(ukk key);
	void BindToContainer(ukk key, ukk alloc);
	void UnbindFromContainer(ukk key, ukk alloc);
	void SwapContainers(ukk keyA, ukk keyB);
	//////////////////////////////////////////////////////////////////////////

private:
	static void RecordModuleLoad(uk pSelf, const CReplayModules::ModuleLoadDesc& mld);
	static void RecordModuleUnload(uk pSelf, const CReplayModules::ModuleUnloadDesc& mld);

private:
	CMemReplay(const CMemReplay&);
	CMemReplay& operator=(const CMemReplay&);

private:
	void RecordAlloc(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId, UINT_PTR p, UINT_PTR alignment, UINT_PTR sizeRequested, UINT_PTR sizeConsumed, INT_PTR sizeGlobal);
	void RecordRealloc(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId, UINT_PTR op, UINT_PTR p, UINT_PTR alignment, UINT_PTR sizeRequested, UINT_PTR sizeConsumed, INT_PTR sizeGlobal);
	void RecordFree(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId, UINT_PTR p, INT_PTR sizeGlobal, bool captureCallstack);
	void RecordModules();

	i32  GetCurrentExecutableSize();

private:
	 u32             m_allocReference;
	ReplayLogStream             m_stream;
	CReplayModules              m_modules;

	DrxCriticalSection          m_scope;

	i32                         m_scopeDepth;
	EMemReplayAllocClass::Class m_scopeClass;
	u16                      m_scopeSubClass;
	i32                         m_scopeModuleId;
};
//////////////////////////////////////////////////////////////////////////

#endif

#endif
