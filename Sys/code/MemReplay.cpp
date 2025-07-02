// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/CoreX/BitFiddling.h>

#include <stdio.h>
#include <algorithm>  // std::min()
#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Platform/platform.h>

#include <drx3D/Sys/MemoryUpr.h>

#include <drx3D/Sys/MemReplay.h>

#include <drx3D/Sys/System.h>
#include <drx3D/Network/DrxSocks.h>

#if DRX_PLATFORM_ORBIS
inline u32 DrxGetCurrentThreadId32()
{
	// A horrible hack that assumes that pthread_t has a first member that is a 32-bit ID.
	// This is totally unsafe and undocumented, but we need a 32-bit ID for MemReplay.
	return *reinterpret_cast<u32*>(DrxGetCurrentThreadId());
}
#else
inline u32 DrxGetCurrentThreadId32()
{
	return static_cast<u32>(DrxGetCurrentThreadId());
}
#endif

// FIXME DbgHelp broken on Durango currently
#if DRX_PLATFORM_WINDOWS //|| DRX_PLATFORM_DURANGO
	#include <drx3D/Sys/DebugCallStack.h>
	#include "Psapi.h"
	#include "DbgHelp.h"

namespace
{
struct CVDebugInfo
{
	DWORD cvSig;
	GUID  pdbSig;
	DWORD age;
	char  pdbName[256];
};
}
#endif

#if DRX_PLATFORM_DURANGO
	#include <processthreadsapi.h>
#endif

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	#include <sys/mman.h>
	#if !defined(_GNU_SOURCE)
		#define _GNU_SOURCE
	#endif
	#include <link.h>
#endif

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#include <sys/mman.h>
#endif

#if DRX_PLATFORM_ORBIS
	#include <net.h>
	#include <drx3D/Network/DrxSocks.h>
#endif

#if CAPTURE_REPLAY_LOG || ENABLE_STATOSCOPE

ReplayDiskWriter::ReplayDiskWriter(tukk pSuffix)
	: m_fp(NULL)
	, m_written(0)
{
	if (pSuffix == NULL || strcmp(pSuffix, "") == 0)
	{
		time_t curTime;
		time(&curTime);
		struct tm* lt = localtime(&curTime);

		strftime(m_filename, DRX_ARRAY_COUNT(m_filename), "memlog-%Y%m%d-%H%M%S.zmrl", lt);
	}
	else
	{
		drx_sprintf(m_filename, "memlog-%s.zmrl", pSuffix);
	}
}

bool ReplayDiskWriter::Open()
{
	#ifdef USE_FILE_HANDLE_CACHE
		#undef fopen
	#endif
	char fn[MAX_PATH] = "";
	#if DRX_PLATFORM_ORBIS
	drx_strcpy(fn, "/data/");
	#endif
	drx_strcat(fn, m_filename);
	m_fp = ::fopen(fn, "wb");
	return m_fp != NULL;

	#if defined(USE_FILE_HANDLE_CACHE)
		#define fopen WrappedFopen
	#endif
}

void ReplayDiskWriter::Close()
{
	#ifdef USE_FILE_HANDLE_CACHE
		#undef fclose
	#endif

	if (m_fp)
	{
		::fclose(m_fp);

		m_fp = NULL;
	}

	#if defined(USE_FILE_HANDLE_CACHE)
		#define fclose WrappedFclose
	#endif
}

i32 ReplayDiskWriter::Write(ukk data, size_t sz, size_t n)
{
	#ifdef USE_FILE_HANDLE_CACHE
		#undef fwrite
	#endif
	i32 res = ::fwrite(data, sz, n, m_fp);
	#if defined(USE_FILE_HANDLE_CACHE)
		#define fwrite WrappedFWrite
	#endif

	m_written += res * sz;

	return res;
}

void ReplayDiskWriter::Flush()
{
	#ifdef USE_FILE_HANDLE_CACHE
		#undef fflush
	#endif

	::fflush(m_fp);

	#if defined(USE_FILE_HANDLE_CACHE)
		#define fflush WrappedFflush
	#endif
}

ReplaySocketWriter::ReplaySocketWriter(tukk address)
	: m_port(29494)
	, m_sock(DRX_INVALID_SOCKET)
	, m_written(0)
{
	#if defined(REPLAY_SOCKET_WRITER)
	tukk portStart = strchr(address, ':');
	if (portStart)
		m_port = atoi(portStart + 1);
	else
		portStart = address + strlen(address);

	drx_strcpy(m_address, address, (size_t)(portStart - address));
	#else
	ZeroArray(m_address);
	#endif //#if defined(REPLAY_SOCKET_WRITER)
}

bool ReplaySocketWriter::Open()
{
	#if defined(REPLAY_SOCKET_WRITER)
	DRXINADDR_T addr32 = 0;

		#if DRX_PLATFORM_WINAPI
	WSADATA wsaData;
	PREFAST_SUPPRESS_WARNING(6031);       // we don't need wsaData
	WSAStartup(MAKEWORD(2, 2), &wsaData); // ensure DrxSock::socket has been initialized
		#endif

	addr32 = DrxSock::GetAddrForHostOrIP(m_address, 0);

	if (!addr32)
		return false;

	m_sock = DrxSock::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == DRX_INVALID_SOCKET)
		return false;

	DRXSOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = addr32;

	i32 connectErr = DRX_SOCKET_ERROR;
	for (u16 port = m_port; (connectErr == DRX_SOCKET_ERROR) && port < (29494 + 32); ++port)
	{
		addr.sin_port = htons(port);
		connectErr = DrxSock::connect(m_sock, (DRXSOCKADDR*) &addr, sizeof(addr));
	}

	if (connectErr == DRX_SOCKET_ERROR)
	{
		DrxSock::shutdown(m_sock, SD_BOTH);
		DrxSock::closesocket(m_sock);

		m_sock = DRX_INVALID_SOCKET;
		return false;
	}
	#endif //#if defined(REPLAY_SOCKET_WRITER)
	return true;
}

void ReplaySocketWriter::Close()
{
	#if defined(REPLAY_SOCKET_WRITER)
	if (m_sock != DRX_INVALID_SOCKET)
	{
		DrxSock::shutdown(m_sock, SD_BOTH);
		DrxSock::closesocket(m_sock);
		m_sock = DRX_INVALID_SOCKET;
	}

		#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_APPLE && !DRX_PLATFORM_ORBIS
	WSACleanup();
		#endif

	#endif //#if defined(REPLAY_SOCKET_WRITER)
}

i32 ReplaySocketWriter::Write(ukk data, size_t sz, size_t n)
{
	#if defined(REPLAY_SOCKET_WRITER)
	if (m_sock != DRX_INVALID_SOCKET)
	{
		tukk bdata = reinterpret_cast<tukk >(data);
		size_t toSend = sz * n;
		while (toSend > 0)
		{
			i32 sent = send(m_sock, bdata, toSend, 0);

			if (sent == DRX_SOCKET_ERROR)
			{
				__debugbreak();
				break;
			}

			toSend -= sent;
			bdata += sent;
		}

		m_written += sz * n;
	}

	#endif //#if defined(REPLAY_SOCKET_WRITER)

	return n;
}

void ReplaySocketWriter::Flush()
{
}

#endif

#if CAPTURE_REPLAY_LOG

typedef DrxLockT<DRXLOCK_RECURSIVE> MemFastMutex;
static MemFastMutex& GetLogMutex()
{
	static MemFastMutex logmutex;
	return logmutex;
}
static  threadID s_ignoreThreadId = (threadID) - 1;

static u32 g_memReplayFrameCount;
i32k k_maxCallStackDepth = 256;
extern  bool g_replayCleanedUp;

static  UINT_PTR s_replayLastGlobal = 0;
static  i32 s_replayStartingFree = 0;

inline void DrxMemStatIgnoreThread(threadID threadId)
{
	s_ignoreThreadId = threadId;
}

	#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO

uk ReplayAllocatorBase::ReserveAddressSpace(size_t sz)
{
	return VirtualAlloc(NULL, sz, MEM_RESERVE, PAGE_READWRITE);
}

void ReplayAllocatorBase::UnreserveAddressSpace(uk base, uk committedEnd)
{
	VirtualFree(base, 0, MEM_RELEASE);
}

uk ReplayAllocatorBase::MapAddressSpace(uk addr, size_t sz)
{
	return VirtualAlloc(addr, sz, MEM_COMMIT, PAGE_READWRITE);
}

	#elif DRX_PLATFORM_ORBIS

uk ReplayAllocatorBase::ReserveAddressSpace(size_t sz)
{
	return VirtualAllocator::AllocateVirtualAddressSpace(sz);
}

void ReplayAllocatorBase::UnreserveAddressSpace(uk base, uk committedEnd)
{
	VirtualAllocator::FreeVirtualAddressSpace(base);
}

uk ReplayAllocatorBase::MapAddressSpace(uk addr, size_t sz)
{
	for (size_t i = 0; i < sz; i += PAGE_SIZE)
	{
		if (!VirtualAllocator::MapPage((tuk)addr + i))
		{
			for (size_t k = 0; k < i; k += PAGE_SIZE)
			{
				VirtualAllocator::UnmapPage((tuk)addr + k);
			}
			return NULL;
		}
	}
	return addr;
}

	#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
uk ReplayAllocatorBase::ReserveAddressSpace(size_t sz)
{
	return mmap(NULL, sz, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
}

void ReplayAllocatorBase::UnreserveAddressSpace(uk base, uk committedEnd)
{
	i32 res = munmap(base, reinterpret_cast<tuk>(committedEnd) - reinterpret_cast<tuk>(base));
	(void) res;
	assert(res == 0);
}

uk ReplayAllocatorBase::MapAddressSpace(uk addr, size_t sz)
{
	i32 res = mprotect(addr, sz, PROT_READ | PROT_WRITE);
	return addr;
}
	#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID

uk ReplayAllocatorBase::ReserveAddressSpace(size_t sz)
{
	return mmap(NULL, sz, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
}

void ReplayAllocatorBase::UnreserveAddressSpace(uk base, uk committedEnd)
{
	i32 res = munmap(base, reinterpret_cast<tuk>(committedEnd) - reinterpret_cast<tuk>(base));
}

uk ReplayAllocatorBase::MapAddressSpace(uk addr, size_t sz)
{
	i32 res = mprotect(addr, sz, PROT_READ | PROT_WRITE);
	return addr;
}

	#elif DRX_PLATFORM_ORBIS

uk ReplayAllocatorBase::ReserveAddressSpace(size_t sz)
{
	return VirtualAllocator::AllocateVirtualAddressSpace(sz);
}

void ReplayAllocatorBase::UnreserveAddressSpace(uk base, uk committedEnd)
{
	VirtualAllocator::FreeVirtualAddressSpace(base);
}

uk ReplayAllocatorBase::MapAddressSpace(uk addr, size_t sz)
{
	for (size_t i = 0; i < sz; i += PAGE_SIZE)
	{
		if (!VirtualAllocator::MapPage((tuk)addr + i))
		{
			for (size_t k = 0; k < i; k += PAGE_SIZE)
			{
				VirtualAllocator::UnmapPage((tuk)addr + k);
			}
			return NULL;
		}
	}
	return addr;
}

	#endif

namespace
{
//helper functions to become strict aliasing warning conform
ILINE uk * CastCallstack(u32* pCallstack)
{
	return alias_cast<uk *>(pCallstack);
}
ILINE uk * CastCallstack(uint64* pCallstack)
{
	return alias_cast<uk *>(pCallstack);
}
	#if !DRX_PLATFORM_WINDOWS && !DRX_PLATFORM_DURANGO && !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID
ILINE uk * CastCallstack(UINT_PTR* pCallstack)
{
	return alias_cast<uk *>(pCallstack);
}
	#endif
ILINE  long* LongPtr( i32* pI)
{
	return alias_cast< long*>(pI);
}

void RetrieveMemReplaySuffix(tuk pSuffixBuffer, tukk lwrCommandLine, i32 bufferLength)
{
	drx_strcpy(pSuffixBuffer, bufferLength, "");
	tukk pSuffix = strstr(lwrCommandLine, "-memreplaysuffix");
	if (pSuffix != NULL)
	{
		pSuffix += strlen("-memreplaysuffix");
		while (1)
		{
			if (*pSuffix == '\0' || *pSuffix == '+' || *pSuffix == '-')
			{
				pSuffix = NULL;
				break;
			}
			if (*pSuffix != ' ')
				break;
			++pSuffix;
		}
		if (pSuffix != NULL)
		{
			tukk pEnd = pSuffix;
			while (*pEnd != '\0' && *pEnd != ' ')
				++pEnd;
			drx_strcpy(pSuffixBuffer, bufferLength, pSuffix, (size_t)(pEnd - pSuffix));
		}
	}
}
}

ReplayAllocator::ReplayAllocator()
{
	m_heap = ReserveAddressSpace(MaxSize);
	if (!m_heap)
		__debugbreak();

	m_commitEnd = m_heap;
	m_allocEnd = m_heap;
}

ReplayAllocator::~ReplayAllocator()
{
	Free();
}

void ReplayAllocator::Reserve(size_t sz)
{
	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_lock);

	INT_PTR commitEnd = reinterpret_cast<INT_PTR>(m_commitEnd);
	INT_PTR newCommitEnd = std::min<INT_PTR>(commitEnd + sz, commitEnd + MaxSize);

	if ((newCommitEnd - commitEnd) > 0)
	{
		LPVOID res = MapAddressSpace(m_commitEnd, newCommitEnd - commitEnd);
		if (!res)
			__debugbreak();
	}

	m_commitEnd = reinterpret_cast<uk>(newCommitEnd);
}

uk ReplayAllocator::Allocate(size_t sz)
{
	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_lock);

	sz = (sz + 7) & ~7;

	u8* alloc = reinterpret_cast<u8*>(m_allocEnd);
	u8* newEnd = reinterpret_cast<u8*>(m_allocEnd) + sz;

	if (newEnd > reinterpret_cast<u8*>(m_commitEnd))
	{
		u8* alignedEnd = reinterpret_cast<u8*>((reinterpret_cast<INT_PTR>(newEnd) + (PageSize - 1)) & ~(PageSize - 1));

		if (alignedEnd > reinterpret_cast<u8*>(m_allocEnd) + MaxSize)
			__debugbreak();

		LPVOID res = MapAddressSpace(m_commitEnd, alignedEnd - reinterpret_cast<u8*>(m_commitEnd));
		if (!res)
			__debugbreak();

		m_commitEnd = alignedEnd;
	}

	m_allocEnd = newEnd;

	return alloc;
}

void ReplayAllocator::Free()
{
	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_lock);

	UnreserveAddressSpace(m_heap, m_commitEnd);
}

ReplayCompressor::ReplayCompressor(ReplayAllocator& allocator, IReplayWriter* writer)
	: m_allocator(&allocator)
	, m_writer(writer)
{
	m_zSize = 0;

	memset(&m_compressStream, 0, sizeof(m_compressStream));
	m_compressStream.zalloc = &ReplayCompressor::zAlloc;
	m_compressStream.zfree = &ReplayCompressor::zFree;
	m_compressStream.opaque = this;
	deflateInit(&m_compressStream, 2);//Z_DEFAULT_COMPRESSION);

	uint64 streamLen = 0;
	m_writer->Write(&streamLen, sizeof(streamLen), 1);

	m_compressTarget = (u8*) m_allocator->Allocate(CompressTargetSize);
}

ReplayCompressor::~ReplayCompressor()
{
	deflateEnd(&m_compressStream);
	m_compressTarget = NULL;
	m_writer = NULL;
}

void ReplayCompressor::Flush()
{
	m_writer->Flush();
}

void ReplayCompressor::Write(u8k* data, size_t len)
{
	if (CompressTargetSize < len)
		__debugbreak();

	m_compressStream.next_in = const_cast<u8*>(data);
	m_compressStream.avail_in = len;
	m_compressStream.next_out = m_compressTarget;
	m_compressStream.avail_out = CompressTargetSize;
	m_compressStream.total_out = 0;

	do
	{
		i32 err = deflate(&m_compressStream, Z_SYNC_FLUSH);

		if ((err == Z_OK && m_compressStream.avail_out == 0) || (err == Z_BUF_ERROR && m_compressStream.avail_out == 0))
		{
			i32 total_out = m_compressStream.total_out;
			m_writer->Write(m_compressTarget, total_out * sizeof(u8), 1);

			m_compressStream.next_out = m_compressTarget;
			m_compressStream.avail_out = CompressTargetSize;
			m_compressStream.total_out = 0;

			continue;
		}
		else if (m_compressStream.avail_in == 0)
		{
			i32 total_out = m_compressStream.total_out;
			m_writer->Write(m_compressTarget, total_out * sizeof(u8), 1);
		}
		else
		{
			// Shrug?
		}

		break;
	}
	while (true);

	Flush();
}

voidpf ReplayCompressor::zAlloc(voidpf opaque, uInt items, uInt size)
{
	ReplayCompressor* str = reinterpret_cast<ReplayCompressor*>(opaque);
	return str->m_allocator->Allocate(items * size);
}

void ReplayCompressor::zFree(voidpf opaque, voidpf address)
{
	// Doesn't seem to be called, except when shutting down and then we'll just free it all anyway
}

	#if REPLAY_RECORD_THREADED

ReplayRecordThread::ReplayRecordThread(ReplayCompressor* compressor)
	: m_compressor(compressor)
{
	m_nextCommand = CMD_Idle;
	m_nextData = NULL;
	m_nextLength = 0;
}

void ReplayRecordThread::Flush()
{
	m_mtx.Lock();

	while (m_nextCommand != CMD_Idle)
	{
		m_mtx.Unlock();
		DrxSleep(1);
		m_mtx.Lock();
	}

	m_nextCommand = CMD_Flush;
	m_mtx.Unlock();
	m_cond.NotifySingle();
}

void ReplayRecordThread::Write(u8k* data, size_t len)
{
	m_mtx.Lock();

	while (m_nextCommand != CMD_Idle)
	{
		m_mtx.Unlock();
		DrxSleep(1);
		m_mtx.Lock();
	}

	m_nextData = data;
	m_nextLength = len;
	m_nextCommand = CMD_Write;
	m_mtx.Unlock();
	m_cond.NotifySingle();
}

void ReplayRecordThread::ThreadEntry()
{
	DrxMemStatIgnoreThread(DrxGetCurrentThreadId());

	while (true)
	{
		m_mtx.Lock();

		while (m_nextCommand == CMD_Idle)
			m_cond.Wait(m_mtx);

		switch (m_nextCommand)
		{
		case CMD_Stop:
			m_mtx.Unlock();
			m_nextCommand = CMD_Idle;
			return;

		case CMD_Write:
			{
				size_t length = m_nextLength;
				u8k* data = m_nextData;

				m_nextLength = 0;
				m_nextData = NULL;

				m_compressor->Write(data, length);
			}
			break;

		case CMD_Flush:
			m_compressor->Flush();
			break;
		}

		m_nextCommand = CMD_Idle;

		m_mtx.Unlock();
	}
}

void ReplayRecordThread::SignalStopWork()
{
	m_mtx.Lock();

	while (m_nextCommand != CMD_Idle)
	{
		m_mtx.Unlock();
		DrxSleep(1);
		m_mtx.Lock();
	}

	m_nextCommand = CMD_Stop;
	m_cond.NotifySingle();

	while (m_nextCommand != CMD_Idle)
	{
		m_mtx.Unlock();
		DrxSleep(1);
		m_mtx.Lock();
	}

	m_mtx.Unlock();
}

	#endif

ReplayLogStream::ReplayLogStream()
	: m_isOpen(0)
	, m_buffer(nullptr)
	, m_bufferSize(0)
	, m_bufferEnd(nullptr)
	, m_uncompressedLen(0)
	, m_sequenceCheck(0)
	, m_bufferA(nullptr)
	, m_compressor(nullptr)
	#if REPLAY_RECORD_THREADED
	, m_bufferB(nullptr)
	, m_recordThread(nullptr)
	#endif
	, m_writer(nullptr)
{
}

ReplayLogStream::~ReplayLogStream()
{
	if (IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());
		Close();
	}
}

bool ReplayLogStream::Open(tukk openString)
{
	using std::swap;

	m_allocator.Reserve(512 * 1024);

	while (isspace((u8) *openString))
		++openString;

	tukk sep = strchr(openString, ':');
	if (!sep)
		sep = openString + strlen(openString);

	char destination[32];
	tukk arg = "";

	if (sep != openString)
	{
		while (isspace((u8) sep[-1]))
			--sep;

		drx_strcpy(destination, openString, (size_t)(sep - openString));

		arg = sep;

		if (*arg)
		{
			++arg;
			while (isspace((u8) *arg))
				++arg;
		}
	}
	else
	{
		drx_strcpy(destination, "disk");
	}

	IReplayWriter* writer = NULL;

	if (strcmp(destination, "disk") == 0)
	{
		writer = new(m_allocator.Allocate(sizeof(ReplayDiskWriter)))ReplayDiskWriter(arg);
	}
	else if (strcmp(destination, "socket") == 0)
	{
		writer = new(m_allocator.Allocate(sizeof(ReplaySocketWriter)))ReplaySocketWriter(arg);
	}

	if (writer && writer->Open() == false)
	{
		fprintf(stdout, "Failed to open writer\n");
		writer->~IReplayWriter();
		writer = NULL;

		m_allocator.Free();
		return false;
	}

	swap(m_writer, writer);

	m_bufferSize = 64 * 1024;
	m_bufferA = (u8*) m_allocator.Allocate(m_bufferSize);

	#if REPLAY_RECORD_THREADED
	m_bufferB = (u8*) m_allocator.Allocate(m_bufferSize);
	#endif

	m_buffer = m_bufferA;
	m_compressor = new(m_allocator.Allocate(sizeof(ReplayCompressor)))ReplayCompressor(m_allocator, m_writer);

	{
		m_bufferEnd = &m_buffer[0];

		// Write stream header.
		u8 version = 3;
		u32 platform = MemReplayPlatformIds::PI_Unknown;

	#if DRX_PLATFORM_ORBIS
		platform = MemReplayPlatformIds::PI_Orbis;
	#elif DRX_PLATFORM_DURANGO
		platform = MemReplayPlatformIds::PI_Durango;
	#elif DRX_PLATFORM_WINDOWS || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
		platform = MemReplayPlatformIds::PI_PC;
	#endif

		MemReplayLogHeader hdr(('M' << 0) | ('R' << 8) | ('L' << 16) | (version << 24), platform, sizeof(uk ) * 8);
		memcpy(&m_buffer[0], &hdr, sizeof(hdr));
		m_bufferEnd += sizeof(hdr);

		DrxInterlockedIncrement(&m_isOpen);
	}

	return true;
}

void ReplayLogStream::Close()
{
	using std::swap;

	// Flag it as closed here, to prevent recursion into the memory logging as it is closing.
	DrxInterlockedDecrement(&m_isOpen);

	Flush();
	Flush();
	Flush();

	#if REPLAY_RECORD_THREADED
	if (m_recordThread)
	{
		m_recordThread->SignalStopWork();
		gEnv->pThreadUpr->JoinThread(m_recordThread, eJM_Join);
		m_recordThread->~ReplayRecordThread();
		m_recordThread = NULL;
	}
	#endif

	if (m_compressor)
	{
		m_compressor->~ReplayCompressor();
		m_compressor = NULL;
	}

	m_writer->Close();
	m_writer = NULL;

	m_buffer = NULL;
	m_bufferEnd = 0;
	m_bufferSize = 0;

	m_allocator.Free();
}

bool ReplayLogStream::EnableAsynchMode()
{
	#if REPLAY_RECORD_THREADED
	// System to create thread is not running yet
	if (!gEnv || !gEnv->pSystem)
		return false;

	// Asynch mode already enabled
	if (m_recordThread)
		return true;

	// Write out pending data
	Flush();

	// Try create ReplayRecord thread
	m_recordThread = new(m_allocator.Allocate(sizeof(ReplayRecordThread)))ReplayRecordThread(m_compressor);
	if (!gEnv->pThreadUpr->SpawnThread(m_recordThread, "ReplayRecord"))
	{
		DRX_ASSERT_MESSAGE(false, "Error spawning \"ReplayRecord\" thread.");
		m_recordThread->~ReplayRecordThread();
		m_recordThread = NULL;
		return false;
	}

	return true;
	#else
	return false;
	#endif
}

void ReplayLogStream::Flush()
{
	if (m_writer != NULL)
	{
		m_uncompressedLen += m_bufferEnd - &m_buffer[0];

	#if REPLAY_RECORD_THREADED
		if (m_recordThread)
		{
			m_recordThread->Write(&m_buffer[0], m_bufferEnd - &m_buffer[0]);
			m_buffer = (m_buffer == m_bufferA) ? m_bufferB : m_bufferA;
		}
		else
		{
			DrxMemStatIgnoreThread(DrxGetCurrentThreadId());
			m_compressor->Write(&m_buffer[0], m_bufferEnd - &m_buffer[0]);
			DrxMemStatIgnoreThread((DWORD)-1);
		}
	#else
		DrxMemStatIgnoreThread(DrxGetCurrentThreadId());
		m_compressor->Write(&m_buffer[0], m_bufferEnd - &m_buffer[0]);
		DrxMemStatIgnoreThread((DWORD)-1);
	#endif
	}

	m_bufferEnd = &m_buffer[0];
}

void ReplayLogStream::FullFlush()
{
	#if REPLAY_RECORD_THREADED
	if (m_recordThread)
	{
		m_recordThread->Flush();
	}
	else
	{
		DrxMemStatIgnoreThread(DrxGetCurrentThreadId());
		m_compressor->Flush();
		DrxMemStatIgnoreThread((DWORD)-1);
	}
	#else
	DrxMemStatIgnoreThread(DrxGetCurrentThreadId());
	m_compressor->Flush();
	DrxMemStatIgnoreThread((DWORD)-1);
	#endif
}

size_t ReplayLogStream::GetSize() const
{
	return m_allocator.GetCommittedSize();
}

uint64 ReplayLogStream::GetWrittenLength() const
{
	return m_writer ? m_writer->GetWrittenLength() : 0ULL;
}

uint64 ReplayLogStream::GetUncompressedLength() const
{
	return m_uncompressedLen;
}

	#define SIZEOF_MEMBER(cls, mbr) (sizeof(reinterpret_cast<cls*>(0)->mbr))

static bool g_memReplayPaused = false;

//////////////////////////////////////////////////////////////////////////
// CMemReplay class implementation.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

static i32 GetCurrentSysAlloced()
{
	i32 trackingUsage = 0;//m_stream.GetSize();
	IMemoryUpr::SProcessMemInfo mi;
	CDrxMemoryUpr::GetInstance()->GetProcessMemInfo(mi);
	return (u32)(mi.PagefileUsage - trackingUsage);
}

	#if !DRX_PLATFORM_ORBIS
i32 CMemReplay::GetCurrentExecutableSize()
{
	return GetCurrentSysAlloced();
}
	#endif

//////////////////////////////////////////////////////////////////////////
static UINT_PTR GetCurrentSysFree()
{
	IMemoryUpr::SProcessMemInfo mi;
	CDrxMemoryUpr::GetInstance()->GetProcessMemInfo(mi);
	return (UINT_PTR)-(INT_PTR)mi.PagefileUsage;
}

void CReplayModules::RefreshModules(FModuleLoad onLoad, FModuleUnload onUnload, uk pUser)
{
	#if DRX_PLATFORM_WINDOWS
	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	u32 i;

	// Get a list of all the modules in this process.
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	if (NULL != hProcess)
	{
		if (SymInitialize(hProcess, NULL, TRUE))
		{
			if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
			{
				for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
				{
					TCHAR szModName[MAX_PATH];
					// Get the full path to the module's file.
					if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
					{
						char pdbPath[1024];
						drx_strcpy(pdbPath, szModName);
						tuk pPdbPathExt = strrchr(pdbPath, '.');
						if (pPdbPathExt)
							strcpy(pPdbPathExt, ".pdb");
						else
							drx_strcat(pdbPath, ".pdb");

						MODULEINFO modInfo;
						GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo));

						IMAGEHLP_MODULE64 ihModInfo;
						memset(&ihModInfo, 0, sizeof(ihModInfo));
						ihModInfo.SizeOfStruct = sizeof(ihModInfo);
						if (SymGetModuleInfo64(hProcess, (DWORD64)modInfo.lpBaseOfDll, &ihModInfo))
						{
							drx_strcpy(pdbPath, ihModInfo.CVData);

							if (ihModInfo.PdbSig70.Data1 == 0)
							{
								DWORD debugEntrySize;
								IMAGE_DEBUG_DIRECTORY* pDebugDirectory = (IMAGE_DEBUG_DIRECTORY*)ImageDirectoryEntryToDataEx(modInfo.lpBaseOfDll, TRUE, IMAGE_DIRECTORY_ENTRY_DEBUG, &debugEntrySize, NULL);
								if (pDebugDirectory)
								{
									size_t numDirectories = debugEntrySize / sizeof(IMAGE_DEBUG_DIRECTORY);
									for (size_t dirIdx = 0; dirIdx != numDirectories; ++dirIdx)
									{
										IMAGE_DEBUG_DIRECTORY& dir = pDebugDirectory[dirIdx];
										if (dir.Type == 2)
										{
											CVDebugInfo* pCVInfo = (CVDebugInfo*)((tuk)modInfo.lpBaseOfDll + dir.AddressOfRawData);
											ihModInfo.PdbSig70 = pCVInfo->pdbSig;
											ihModInfo.PdbAge = pCVInfo->age;
											drx_strcpy(pdbPath, pCVInfo->pdbName);
											break;
										}
									}
								}
							}

							ModuleLoadDesc mld;
							drx_sprintf(mld.sig, "%p, %08X, %08X, {%08X-%04X-%04X-%02X %02X-%02X %02X %02X %02X %02X %02X}, %d\n",
							            modInfo.lpBaseOfDll, modInfo.SizeOfImage, ihModInfo.TimeDateStamp,
							            ihModInfo.PdbSig70.Data1, ihModInfo.PdbSig70.Data2, ihModInfo.PdbSig70.Data3,
							            ihModInfo.PdbSig70.Data4[0], ihModInfo.PdbSig70.Data4[1],
							            ihModInfo.PdbSig70.Data4[2], ihModInfo.PdbSig70.Data4[3],
							            ihModInfo.PdbSig70.Data4[4], ihModInfo.PdbSig70.Data4[5],
							            ihModInfo.PdbSig70.Data4[6], ihModInfo.PdbSig70.Data4[7],
							            ihModInfo.PdbAge);

							drx_strcpy(mld.name, szModName);
							drx_strcpy(mld.path, pdbPath);
							mld.address = (UINT_PTR)modInfo.lpBaseOfDll;
							mld.size = modInfo.SizeOfImage;
							onLoad(pUser, mld);
						}
					}
				}
			}

			SymCleanup(hProcess);
		}
		CloseHandle(hProcess);
	}
	#elif DRX_PLATFORM_DURANGO && 0

	// Get a list of all the modules in this process.
	EnumModuleState ems;
	ems.hProcess = GetCurrentProcess();
	ems.pSelf = this;
	ems.onLoad = onLoad;
	ems.pOnLoadUser = pUser;
	if (NULL != ems.hProcess)
	{
		if (SymInitialize(ems.hProcess, NULL, TRUE))
		{
			EnumerateLoadedModulesEx(ems.hProcess, EnumModules_Durango, &ems);

			SymCleanup(ems.hProcess);
		}
	}
	#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	EnumModuleState ems;
	ems.hProcess = 0;
	ems.pSelf = this;
	ems.onLoad = onLoad;
	ems.pOnLoadUser = pUser;
	dl_iterate_phdr(EnumModules_Linux, &ems);
	#elif DRX_PLATFORM_ORBIS
	RefreshModules_Orbis(onLoad, pUser);
	#endif
}

	#if DRX_PLATFORM_DURANGO && 0

BOOL CReplayModules::EnumModules_Durango(PCSTR moduleName, DWORD64 moduleBase, ULONG moduleSize, PVOID userContext)
{
	EnumModuleState& ems = *static_cast<EnumModuleState*>(userContext);

	char pdbPath[1024];
	drx_strcpy(pdbPath, moduleName);
	tuk pPdbPathExt = strrchr(pdbPath, '.');
	if (pPdbPathExt)
		strcpy(pPdbPathExt, ".pdb");
	else
		drx_strcat(pdbPath, ".pdb");

	IMAGEHLP_MODULE64 ihModInfo;
	memset(&ihModInfo, 0, sizeof(ihModInfo));
	ihModInfo.SizeOfStruct = sizeof(ihModInfo);
	if (SymGetModuleInfo64(ems.hProcess, moduleBase, &ihModInfo))
	{
		drx_strcpy(pdbPath, ihModInfo.CVData);

		if (ihModInfo.PdbSig70.Data1 == 0)
		{
			DWORD debugEntrySize;
			IMAGE_DEBUG_DIRECTORY* pDebugDirectory = (IMAGE_DEBUG_DIRECTORY*)ImageDirectoryEntryToDataEx((PVOID)moduleBase, TRUE, IMAGE_DIRECTORY_ENTRY_DEBUG, &debugEntrySize, NULL);
			if (pDebugDirectory)
			{
				size_t numDirectories = debugEntrySize / sizeof(IMAGE_DEBUG_DIRECTORY);
				for (size_t dirIdx = 0; dirIdx != numDirectories; ++dirIdx)
				{
					IMAGE_DEBUG_DIRECTORY& dir = pDebugDirectory[dirIdx];
					if (dir.Type == 2)
					{
						CVDebugInfo* pCVInfo = (CVDebugInfo*)((tuk)moduleBase + dir.AddressOfRawData);
						ihModInfo.PdbSig70 = pCVInfo->pdbSig;
						ihModInfo.PdbAge = pCVInfo->age;
						drx_strcpy(pdbPath, pCVInfo->pdbName);
						break;
					}
				}
			}
		}

		ModuleLoadDesc mld;
		drx_sprintf(mld.sig, "%p, %08X, %08X, {%08X-%04X-%04X-%02X %02X-%02X %02X %02X %02X %02X %02X}, %d\n",
		            moduleBase, moduleSize, ihModInfo.TimeDateStamp,
		            ihModInfo.PdbSig70.Data1, ihModInfo.PdbSig70.Data2, ihModInfo.PdbSig70.Data3,
		            ihModInfo.PdbSig70.Data4[0], ihModInfo.PdbSig70.Data4[1],
		            ihModInfo.PdbSig70.Data4[2], ihModInfo.PdbSig70.Data4[3],
		            ihModInfo.PdbSig70.Data4[4], ihModInfo.PdbSig70.Data4[5],
		            ihModInfo.PdbSig70.Data4[6], ihModInfo.PdbSig70.Data4[7],
		            ihModInfo.PdbAge);

		drx_strcpy(mld.name, moduleName);
		drx_strcpy(mld.path, pdbPath);
		mld.address = moduleBase;
		mld.size = moduleSize;
		ems.onLoad(ems.pOnLoadUser, mld);
	}

	return TRUE;
}

	#endif

	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
i32 CReplayModules::EnumModules_Linux(struct dl_phdr_info* info, size_t size, uk userContext)
{
	EnumModuleState& ems = *reinterpret_cast<EnumModuleState*>(userContext);

	ModuleLoadDesc mld;
	mld.address = info->dlpi_addr;
	drx_strcpy(mld.name, info->dlpi_name);
	drx_strcpy(mld.path, info->dlpi_name);

	Module& module = ems.pSelf->m_knownModules[ems.pSelf->m_numKnownModules];
	ems.pSelf->m_numKnownModules++;
	module.baseAddress = info->dlpi_addr;
	module.timestamp = 0;

	uint64 memSize = 0;
	for (i32 j = 0; j < info->dlpi_phnum; j++)
	{
		memSize += info->dlpi_phdr[j].p_memsz;
	}
	mld.size = (UINT_PTR)memSize;
	module.size = (UINT_PTR)memSize;

	ems.onLoad(ems.pOnLoadUser, mld);

	return 0;
}
	#endif

// Ensure IMemReplay is not destroyed during atExit() procedure which has no defined destruction order.
// In some cases it was destroyed while still being used.
DRX_ALIGN(8) char g_memReplay[sizeof(CMemReplay)];
CMemReplay* CMemReplay::GetInstance()
{
	static bool bIsInit = false;
	if (!bIsInit)
	{
		new(&g_memReplay)CMemReplay();
		bIsInit = true;
	}

	return alias_cast<CMemReplay*>(g_memReplay);
}

CMemReplay::CMemReplay()
	: m_allocReference(0)
	, m_scopeDepth(0)
	, m_scopeClass((EMemReplayAllocClass::Class)0)
	, m_scopeSubClass(0)
	, m_scopeModuleId(0)
{
}

CMemReplay::~CMemReplay()
{
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::DumpStats()
{
	CMemReplay::DumpSymbols();
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::DumpSymbols()
{
	if (m_stream.IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());
		if (m_stream.IsOpen())
		{
			RecordModules();

			m_stream.FullFlush();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::StartOnCommandLine(tukk cmdLine)
{
	tukk mrCmd = 0;

	DrxStackStringT<char, 2048> lwrCmdLine = cmdLine;
	lwrCmdLine.MakeLower();

	if ((mrCmd = strstr(lwrCmdLine.c_str(), "-memreplay")) != 0)
	{
		bool bPaused = strstr(lwrCmdLine.c_str(), "-memreplaypaused") != 0;

		//Retrieve file suffix if present
		i32k bufferLength = 64;
		char openCmd[bufferLength] = "disk:";

		// Try and detect a new style open string, and fall back to the old suffix format if it fails.
		if (
		  (strncmp(mrCmd, "-memreplay disk", 15) == 0) ||
		  (strncmp(mrCmd, "-memreplay socket", 17) == 0))
		{
			tukk arg = mrCmd + strlen("-memreplay ");
			tukk argEnd = arg;
			while (*argEnd && !isspace((u8) *argEnd))
				++argEnd;
			drx_strcpy(openCmd, arg, (size_t)(argEnd - arg));
		}
		else
		{
			RetrieveMemReplaySuffix(openCmd + 5, lwrCmdLine.c_str(), bufferLength - 5);
		}

		Start(bPaused, openCmd);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::Start(bool bPaused, tukk openString)
{
	if (!openString)
	{
		openString = "disk";
	}

	DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());
	g_memReplayPaused = bPaused;

	if (!m_stream.IsOpen())
	{
		if (m_stream.Open(openString))
		{
			s_replayLastGlobal = GetCurrentSysFree();
			i32 initialGlobal = GetCurrentSysAlloced();
			i32 exeSize = GetCurrentExecutableSize();

	#if DRX_PLATFORM_ORBIS
			// Also record used space in the system allocator
			exeSize += DrxSystemCrtGetUsedSpace();
	#endif

			m_stream.WriteEvent(MemReplayAddressProfileEvent(0xffffffff, 0));
			m_stream.WriteEvent(MemReplayInfoEvent(exeSize, initialGlobal, s_replayStartingFree));
			m_stream.WriteEvent(MemReplayFrameStartEvent(g_memReplayFrameCount++));

	#if DRX_PLATFORM_DURANGO
			m_stream.WriteEvent(MemReplayModuleRefShortEvent("minidump"));
	#endif

	#if DRX_PLATFORM_ORBIS
			m_stream.WriteEvent(MemReplayModuleRefShortEvent("orbis"));
	#endif

			u32 id = 0;
			//#define MEMREPLAY_DEFINE_CONTEXT(name) RecordContextDefinition(id ++, #name);
			//#include "DrxMemReplayContexts.h"
			//#undef MEMREPLAY_DEFINE_CONTEXT
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::Stop()
{
	DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());
	if (m_stream.IsOpen())
	{
		RecordModules();

		m_stream.Close();
	}
}

//////////////////////////////////////////////////////////////////////////
bool CMemReplay::EnterScope(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId)
{
	if (m_stream.IsOpen())
	{
		m_scope.Lock();

		++m_scopeDepth;

		if (m_scopeDepth == 1)
		{
			m_scopeClass = cls;
			m_scopeSubClass = subCls;
			m_scopeModuleId = moduleId;
		}

		return true;
	}

	return false;
}

void CMemReplay::ExitScope_Alloc(UINT_PTR id, UINT_PTR sz, UINT_PTR alignment)
{
	--m_scopeDepth;

	if (m_scopeDepth == 0)
	{
		if (id && m_stream.IsOpen() && DrxGetCurrentThreadId() != s_ignoreThreadId)
		{
			DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

			if (m_stream.IsOpen())
			{
				UINT_PTR global = GetCurrentSysFree();
				INT_PTR changeGlobal = (INT_PTR)(s_replayLastGlobal - global);
				s_replayLastGlobal = global;

				RecordAlloc(m_scopeClass, m_scopeSubClass, m_scopeModuleId, id, alignment, sz, sz, changeGlobal);
			}
		}
	}

	m_scope.Unlock();
}

void CMemReplay::ExitScope_Realloc(UINT_PTR originalId, UINT_PTR newId, UINT_PTR sz, UINT_PTR alignment)
{
	if (!originalId)
	{
		ExitScope_Alloc(newId, sz, alignment);
		return;
	}

	if (!newId)
	{
		ExitScope_Free(originalId);
		return;
	}

	--m_scopeDepth;

	if (m_scopeDepth == 0)
	{
		if (m_stream.IsOpen() && DrxGetCurrentThreadId() != s_ignoreThreadId)
		{
			DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

			if (m_stream.IsOpen())
			{
				UINT_PTR global = GetCurrentSysFree();
				INT_PTR changeGlobal = (INT_PTR)(s_replayLastGlobal - global);
				s_replayLastGlobal = global;

				RecordRealloc(m_scopeClass, m_scopeSubClass, m_scopeModuleId, originalId, newId, alignment, sz, sz, changeGlobal);
			}
		}
	}

	m_scope.Unlock();
}

void CMemReplay::ExitScope_Free(UINT_PTR id)
{
	--m_scopeDepth;

	if (m_scopeDepth == 0)
	{
		if (id && m_stream.IsOpen() && DrxGetCurrentThreadId() != s_ignoreThreadId)
		{
			DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

			if (m_stream.IsOpen())
			{
				UINT_PTR global = GetCurrentSysFree();
				INT_PTR changeGlobal = (INT_PTR)(s_replayLastGlobal - global);
				s_replayLastGlobal = global;

				PREFAST_SUPPRESS_WARNING(6326)
				RecordFree(m_scopeClass, m_scopeSubClass, m_scopeModuleId, id, changeGlobal, REPLAY_RECORD_FREECS != 0);
			}
		}
	}

	m_scope.Unlock();
}

void CMemReplay::ExitScope()
{
	--m_scopeDepth;
	m_scope.Unlock();
}

bool CMemReplay::EnterLockScope()
{
	if (m_stream.IsOpen())
	{
		m_scope.Lock();
		return true;
	}

	return false;
}

void CMemReplay::LeaveLockScope()
{
	m_scope.Unlock();
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::AddLabel(tukk label)
{
	if (m_stream.IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			new(m_stream.AllocateRawEvent<MemReplayLabelEvent>(strlen(label)))MemReplayLabelEvent(label);
		}
	}
}

void CMemReplay::AddLabelFmt(tukk labelFmt, ...)
{
	if (m_stream.IsOpen())
	{
		va_list args;
		va_start(args, labelFmt);

		char msg[256];
		drx_vsprintf(msg, labelFmt, args);

		va_end(args);

		AddLabel(msg);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::AddFrameStart()
{
	if (m_stream.IsOpen())
	{
		static bool bSymbolsDumped = false;
		if (!bSymbolsDumped)
		{
			DumpSymbols();
			bSymbolsDumped = true;
		}

		if (!g_replayCleanedUp)
		{
			g_replayCleanedUp = true;
		}

		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			m_stream.WriteEvent(MemReplayFrameStartEvent(g_memReplayFrameCount++));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::GetInfo(DrxReplayInfo& infoOut)
{
	memset(&infoOut, 0, sizeof(DrxReplayInfo));

	if (m_stream.IsOpen())
	{
		infoOut.uncompressedLength = m_stream.GetUncompressedLength();
		infoOut.writtenLength = m_stream.GetWrittenLength();
		infoOut.trackingSize = m_stream.GetSize();
		infoOut.filename = m_stream.GetFilename();
	}
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::AllocUsage(EMemReplayAllocClass::Class allocClass, UINT_PTR id, UINT_PTR used)
{
	#if REPLAY_RECORD_USAGE_CHANGES
	if (!ptr)
		return;

	if (m_stream.IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());
		if (m_stream.IsOpen())
			m_stream.WriteEvent(MemReplayAllocUsageEvent((u16)allocClass, id, used));
	}
	#endif
}

void CMemReplay::AddAllocReference(uk ptr, uk ref)
{
	if (ptr && m_stream.IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			MemReplayAddAllocReferenceEvent* ev = m_stream.BeginAllocateRawEvent<MemReplayAddAllocReferenceEvent>(
			  k_maxCallStackDepth * SIZEOF_MEMBER(MemReplayAddAllocReferenceEvent, callstack[0])
			  - SIZEOF_MEMBER(MemReplayAddAllocReferenceEvent, callstack));

			new(ev) MemReplayAddAllocReferenceEvent(reinterpret_cast<UINT_PTR>(ptr), reinterpret_cast<UINT_PTR>(ref));

			u32 callstackLength = k_maxCallStackDepth;
			CSystem::debug_GetCallStackRaw(CastCallstack(ev->callstack), callstackLength);

			ev->callstackLength = callstackLength;

			m_stream.EndAllocateRawEvent<MemReplayAddAllocReferenceEvent>(ev->callstackLength * sizeof(ev->callstack[0]) - sizeof(ev->callstack));
		}
	}
}

void CMemReplay::RemoveAllocReference(uk ref)
{
	if (ref && m_stream.IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
			m_stream.WriteEvent(MemReplayRemoveAllocReferenceEvent(reinterpret_cast<UINT_PTR>(ref)));
	}
}

void CMemReplay::AddContext(i32 type, u32 flags, tukk str)
{
	const threadID threadId = DrxGetCurrentThreadId();
	if (m_stream.IsOpen() && threadId != s_ignoreThreadId && !g_memReplayPaused)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			new(m_stream.AllocateRawEvent<MemReplayPushContextEvent>(strlen(str)))
			MemReplayPushContextEvent(DrxGetCurrentThreadId32(), str, (EMemStatContextTypes::Type) type, flags);
		}
	}
}

void CMemReplay::AddContextV(i32 type, u32 flags, tukk format, va_list args)
{
	const threadID threadId = DrxGetCurrentThreadId();
	if (m_stream.IsOpen() && threadId != s_ignoreThreadId && !g_memReplayPaused)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			MemReplayPushContextEvent* ev = m_stream.BeginAllocateRawEvent<MemReplayPushContextEvent>(511);
			new(ev) MemReplayPushContextEvent(DrxGetCurrentThreadId32(), "", (EMemStatContextTypes::Type) type, flags);

			drx_vsprintf(ev->name, 512, format, args);

			m_stream.EndAllocateRawEvent<MemReplayPushContextEvent>(strlen(ev->name));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMemReplay::RemoveContext()
{
	const threadID threadId = DrxGetCurrentThreadId();

	if (m_stream.IsOpen() && threadId != s_ignoreThreadId && !g_memReplayPaused)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			m_stream.WriteEvent(MemReplayPopContextEvent(DrxGetCurrentThreadId32()));
		}
	}
}

void CMemReplay::Flush()
{
	DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

	if (m_stream.IsOpen())
		m_stream.Flush();
}

bool CMemReplay::EnableAsynchMode()
{
	DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

	if (m_stream.IsOpen())
		return m_stream.EnableAsynchMode();

	return false;
}

void CMemReplay::MapPage(uk base, size_t size)
{
	if (m_stream.IsOpen() && base && size)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			MemReplayMapPageEvent* ev = m_stream.BeginAllocateRawEvent<MemReplayMapPageEvent>(
			  k_maxCallStackDepth * sizeof(uk ) - SIZEOF_MEMBER(MemReplayMapPageEvent, callstack));
			new(ev) MemReplayMapPageEvent(reinterpret_cast<UINT_PTR>(base), size);

			u32 callstackLength = k_maxCallStackDepth;
			CSystem::debug_GetCallStackRaw(CastCallstack(ev->callstack), callstackLength);
			ev->callstackLength = callstackLength;

			m_stream.EndAllocateRawEvent<MemReplayMapPageEvent>(sizeof(ev->callstack[0]) * ev->callstackLength - sizeof(ev->callstack));
		}
	}
}

void CMemReplay::UnMapPage(uk base, size_t size)
{
	if (m_stream.IsOpen() && base && size)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			m_stream.WriteEvent(MemReplayUnMapPageEvent(reinterpret_cast<UINT_PTR>(base), size));
		}
	}
}

void CMemReplay::RegisterFixedAddressRange(uk base, size_t size, tukk name)
{
	if (m_stream.IsOpen() && base && size && name)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			new(m_stream.AllocateRawEvent<MemReplayRegisterFixedAddressRangeEvent>(strlen(name)))
			MemReplayRegisterFixedAddressRangeEvent(reinterpret_cast<UINT_PTR>(base), size, name);
		}
	}
}

void CMemReplay::MarkBucket(i32 bucket, size_t alignment, uk base, size_t length)
{
	if (m_stream.IsOpen() && base && length)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
			m_stream.WriteEvent(MemReplayBucketMarkEvent(reinterpret_cast<UINT_PTR>(base), length, bucket, alignment));
	}
}

void CMemReplay::UnMarkBucket(i32 bucket, uk base)
{
	if (m_stream.IsOpen() && base)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
			m_stream.WriteEvent(MemReplayBucketUnMarkEvent(reinterpret_cast<UINT_PTR>(base), bucket));
	}
}

void CMemReplay::BucketEnableCleanups(uk allocatorBase, bool enabled)
{
	if (m_stream.IsOpen() && allocatorBase)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
			m_stream.WriteEvent(MemReplayBucketCleanupEnabledEvent(reinterpret_cast<UINT_PTR>(allocatorBase), enabled));
	}
}

void CMemReplay::MarkPool(i32 pool, size_t alignment, uk base, size_t length, tukk name)
{
	if (m_stream.IsOpen() && base && length)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			new(m_stream.AllocateRawEvent<MemReplayPoolMarkEvent>(strlen(name)))
			MemReplayPoolMarkEvent(reinterpret_cast<UINT_PTR>(base), length, pool, alignment, name);
		}
	}
}

void CMemReplay::UnMarkPool(i32 pool, uk base)
{
	if (m_stream.IsOpen() && base)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
			m_stream.WriteEvent(MemReplayPoolUnMarkEvent(reinterpret_cast<UINT_PTR>(base), pool));
	}
}

void CMemReplay::AddTexturePoolContext(uk ptr, i32 mip, i32 width, i32 height, tukk name, u32 flags)
{
	if (m_stream.IsOpen() && ptr)
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			new(m_stream.AllocateRawEvent<MemReplayTextureAllocContextEvent>(strlen(name)))
			MemReplayTextureAllocContextEvent(reinterpret_cast<UINT_PTR>(ptr), mip, width, height, flags, name);
		}
	}
}

void CMemReplay::AddSizerTree(tukk name)
{
	MemReplayDrxSizer sizer(m_stream, name);
	static_cast<CSystem*>(gEnv->pSystem)->CollectMemStats(&sizer);
}

void CMemReplay::AddScreenshot()
{
	#if 0
	if (m_stream.IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			PREFAST_SUPPRESS_WARNING(6255)
			MemReplayScreenshotEvent * ev = new(alloca(sizeof(MemReplayScreenshotEvent) + 65536))MemReplayScreenshotEvent();
			gEnv->pRenderer->ScreenShotBuf(8, ev->bmp);
			m_stream.WriteEvent(ev, ev->bmp[0] * ev->bmp[1] * 3 + 3);
		}
	}
	#endif
}

void CMemReplay::RegisterContainer(ukk key, i32 type)
{
	#if REPLAY_RECORD_CONTAINERS
	if (key)
	{
		if (m_stream.IsOpen())
		{
			DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

			if (m_stream.IsOpen())
			{
				MemReplayRegisterContainerEvent* ev = m_stream.BeginAllocateRawEvent<MemReplayRegisterContainerEvent>(
				  k_maxCallStackDepth * SIZEOF_MEMBER(MemReplayRegisterContainerEvent, callstack[0]) - SIZEOF_MEMBER(MemReplayRegisterContainerEvent, callstack));

				new(ev) MemReplayRegisterContainerEvent(reinterpret_cast<UINT_PTR>(key), type);

				u32 length = k_maxCallStackDepth;
				CSystem::debug_GetCallStackRaw(CastCallstack(ev->callstack), length);

				ev->callstackLength = length;

				m_stream.EndAllocateRawEvent<MemReplayRegisterContainerEvent>(length * sizeof(ev->callstack[0]) - sizeof(ev->callstack));
			}
		}
	}
	#endif
}

void CMemReplay::UnregisterContainer(ukk key)
{
	#if REPLAY_RECORD_CONTAINERS
	if (key)
	{
		if (m_stream.IsOpen())
		{
			DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

			if (m_stream.IsOpen())
			{
				m_stream.WriteEvent(MemReplayUnregisterContainerEvent(reinterpret_cast<UINT_PTR>(key)));
			}
		}
	}
	#endif
}

void CMemReplay::BindToContainer(ukk key, ukk alloc)
{
	#if REPLAY_RECORD_CONTAINERS
	if (key && alloc)
	{
		if (m_stream.IsOpen())
		{
			DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

			if (m_stream.IsOpen())
			{
				m_stream.WriteEvent(MemReplayBindToContainerEvent(reinterpret_cast<UINT_PTR>(key), reinterpret_cast<UINT_PTR>(alloc)));
			}
		}
	}
	#endif
}

void CMemReplay::UnbindFromContainer(ukk key, ukk alloc)
{
	#if REPLAY_RECORD_CONTAINERS
	if (key && alloc)
	{
		if (m_stream.IsOpen())
		{
			DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

			if (m_stream.IsOpen())
			{
				m_stream.WriteEvent(MemReplayUnbindFromContainerEvent(reinterpret_cast<UINT_PTR>(key), reinterpret_cast<UINT_PTR>(alloc)));
			}
		}
	}
	#endif
}

void CMemReplay::SwapContainers(ukk keyA, ukk keyB)
{
	#if REPLAY_RECORD_CONTAINERS
	if (keyA && keyB && (keyA != keyB) && m_stream.IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream.IsOpen())
		{
			m_stream.WriteEvent(MemReplaySwapContainersEvent(reinterpret_cast<UINT_PTR>(keyA), reinterpret_cast<UINT_PTR>(keyB)));
		}
	}
	#endif
}

void CMemReplay::RecordModuleLoad(uk pSelf, const CReplayModules::ModuleLoadDesc& mld)
{
	CMemReplay* pMR = reinterpret_cast<CMemReplay*>(pSelf);

	tukk baseName = strrchr(mld.name, '\\');
	if (!baseName) baseName = mld.name;
	size_t baseNameLen = strlen(baseName);

	pMR->m_stream.WriteEvent(MemReplayModuleRefEvent(mld.name, mld.path, mld.address, mld.size, mld.sig));

	u32k threadId = DrxGetCurrentThreadId32();

	MemReplayPushContextEvent* pEv = new(pMR->m_stream.BeginAllocateRawEvent<MemReplayPushContextEvent>(baseNameLen))MemReplayPushContextEvent(threadId, baseName, EMemStatContextTypes::MSC_Other, 0);
	pMR->m_stream.EndAllocateRawEvent<MemReplayPushContextEvent>(baseNameLen);

	pMR->RecordAlloc(
	  EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc, eDrxModule,
	  (UINT_PTR)mld.address,
	  4096,
	  mld.size,
	  mld.size,
	  0);

	pMR->m_stream.WriteEvent(MemReplayPopContextEvent(threadId));
}

void CMemReplay::RecordModuleUnload(uk pSelf, const CReplayModules::ModuleUnloadDesc& mld)
{
	CMemReplay* pMR = reinterpret_cast<CMemReplay*>(pSelf);

	PREFAST_SUPPRESS_WARNING(6326)
	pMR->RecordFree(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc, eDrxModule, mld.address, 0, REPLAY_RECORD_FREECS != 0);
	pMR->m_stream.WriteEvent(MemReplayModuleUnRefEvent(mld.address));
}

void CMemReplay::RecordAlloc(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId, UINT_PTR p, UINT_PTR alignment, UINT_PTR sizeRequested, UINT_PTR sizeConsumed, INT_PTR sizeGlobal)
{
	MemReplayAllocEvent* ev = m_stream.BeginAllocateRawEvent<MemReplayAllocEvent>(
	  k_maxCallStackDepth * sizeof(uk ) - SIZEOF_MEMBER(MemReplayAllocEvent, callstack));

	new(ev) MemReplayAllocEvent(
	  DrxGetCurrentThreadId32(),
	  static_cast<u16>(moduleId),
	  static_cast<u16>(cls),
	  static_cast<u16>(subCls),
	  p,
	  static_cast<u32>(alignment),
	  static_cast<u32>(sizeRequested),
	  static_cast<u32>(sizeConsumed),
	  static_cast<i32>(sizeGlobal));

	u32 callstackLength = k_maxCallStackDepth;
	CSystem::debug_GetCallStackRaw(CastCallstack(ev->callstack), callstackLength);

	ev->callstackLength = callstackLength;

	m_stream.EndAllocateRawEvent<MemReplayAllocEvent>(sizeof(ev->callstack[0]) * ev->callstackLength - sizeof(ev->callstack));
}

void CMemReplay::RecordRealloc(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId, UINT_PTR originalId, UINT_PTR newId, UINT_PTR alignment, UINT_PTR sizeRequested, UINT_PTR sizeConsumed, INT_PTR sizeGlobal)
{
	MemReplayReallocEvent* ev = new(m_stream.BeginAllocateRawEvent<MemReplayReallocEvent>(
	                                  k_maxCallStackDepth * SIZEOF_MEMBER(MemReplayReallocEvent, callstack[0]) - SIZEOF_MEMBER(MemReplayReallocEvent, callstack)))
	                            MemReplayReallocEvent(
	  DrxGetCurrentThreadId32(),
	  static_cast<u16>(moduleId),
	  static_cast<u16>(cls),
	  static_cast<u16>(subCls),
	  originalId,
	  newId,
	  static_cast<u32>(alignment),
	  static_cast<u32>(sizeRequested),
	  static_cast<u32>(sizeConsumed),
	  static_cast<i32>(sizeGlobal));

	u32 callstackLength = k_maxCallStackDepth;
	CSystem::debug_GetCallStackRaw(CastCallstack(ev->callstack), callstackLength);
	ev->callstackLength = callstackLength;

	m_stream.EndAllocateRawEvent<MemReplayReallocEvent>(ev->callstackLength * sizeof(ev->callstack[0]) - sizeof(ev->callstack));
}

void CMemReplay::RecordFree(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId, UINT_PTR id, INT_PTR sizeGlobal, bool captureCallstack)
{
	MemReplayFreeEvent* ev =
	  new(m_stream.BeginAllocateRawEvent<MemReplayFreeEvent>(SIZEOF_MEMBER(MemReplayFreeEvent, callstack[0]) * k_maxCallStackDepth - SIZEOF_MEMBER(MemReplayFreeEvent, callstack)))
	  MemReplayFreeEvent(
	    DrxGetCurrentThreadId32(),
	    static_cast<u16>(moduleId),
	    static_cast<u16>(cls),
	    static_cast<u16>(subCls),
	    id,
	    static_cast<i32>(sizeGlobal));

	if (captureCallstack)
	{
		u32 callstackLength = k_maxCallStackDepth;
		CSystem::debug_GetCallStackRaw(CastCallstack(ev->callstack), callstackLength);
		ev->callstackLength = callstackLength;
	}

	m_stream.EndAllocateRawEvent<MemReplayFreeEvent>(ev->callstackLength * sizeof(ev->callstack[0]) - sizeof(ev->callstack));
}

void CMemReplay::RecordModules()
{
	m_modules.RefreshModules(RecordModuleLoad, RecordModuleUnload, this);
}

MemReplayDrxSizer::MemReplayDrxSizer(ReplayLogStream& stream, tukk name)
	: m_stream(&stream)
	, m_totalSize(0)
	, m_totalCount(0)
{
	Push(name);
}

MemReplayDrxSizer::~MemReplayDrxSizer()
{
	Pop();

	{
		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, 0);
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream->IsOpen())
		{
			// Clear the added objects set within the g_replayProcessed lock,
			// as memReplay won't have seen the allocations made by it.
			m_addedObjects.clear();
		}
	}
}

size_t MemReplayDrxSizer::GetTotalSize()
{
	return m_totalSize;
}

size_t MemReplayDrxSizer::GetObjectCount()
{
	return m_totalCount;
}

void MemReplayDrxSizer::Reset()
{
	m_totalSize = 0;
	m_totalCount = 0;
}

bool MemReplayDrxSizer::AddObject(ukk pIdentifier, size_t nSizeBytes, i32 nCount)
{
	bool ret = false;

	if (m_stream->IsOpen())
	{
		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, 0);
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream->IsOpen())
		{
			std::set<ukk>::iterator it = m_addedObjects.find(pIdentifier);
			if (it == m_addedObjects.end())
			{
				m_totalSize += nSizeBytes;
				m_totalCount += nCount;
				m_addedObjects.insert(pIdentifier);
				m_stream->WriteEvent(MemReplaySizerAddRangeEvent((UINT_PTR)pIdentifier, nSizeBytes, nCount));
				ret = true;
			}
		}
	}

	return ret;
}

static NullResCollector s_nullCollectorMemreplay;

IResourceCollector* MemReplayDrxSizer::GetResourceCollector()
{
	return &s_nullCollectorMemreplay;
}

void MemReplayDrxSizer::SetResourceCollector(IResourceCollector*)
{
}

void MemReplayDrxSizer::Push(tukk szComponentName)
{
	if (m_stream->IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream->IsOpen())
		{
			new(m_stream->AllocateRawEvent<MemReplaySizerPushEvent>(strlen(szComponentName)))MemReplaySizerPushEvent(szComponentName);
		}
	}
}

void MemReplayDrxSizer::PushSubcomponent(tukk szSubcomponentName)
{
	Push(szSubcomponentName);
}

void MemReplayDrxSizer::Pop()
{
	if (m_stream->IsOpen())
	{
		DrxAutoLock<DrxCriticalSection> lock(GetLogMutex());

		if (m_stream->IsOpen())
		{
			m_stream->WriteEvent(MemReplaySizerPopEvent());
		}
	}
}

#else // CAPTURE_REPLAY_LOG

#endif //CAPTURE_REPLAY_LOG
