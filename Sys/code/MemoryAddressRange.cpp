// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/MemoryAddressRange.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#include <sys/mman.h>
#endif

CMemoryAddressRange::CMemoryAddressRange(tuk pBaseAddress, size_t nPageSize, size_t nPageCount, tukk sName)
	: m_pBaseAddress(pBaseAddress)
	, m_nPageSize(nPageSize)
	, m_nPageCount(nPageCount)
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	, m_allocatedSpace(0)
#endif
{
	DrxGetIMemReplay()->RegisterFixedAddressRange(pBaseAddress, nPageSize * nPageCount, sName);
}

void CMemoryAddressRange::Release()
{
	delete this;
}

tuk CMemoryAddressRange::GetBaseAddress() const
{
	return m_pBaseAddress;
}

size_t CMemoryAddressRange::GetPageCount() const
{
	return m_nPageCount;
}

size_t CMemoryAddressRange::GetPageSize() const
{
	return m_nPageSize;
}

#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO

uk CMemoryAddressRange::ReserveSpace(size_t capacity)
{
	return VirtualAlloc(NULL, capacity, MEM_RESERVE, PAGE_READWRITE);
}

size_t CMemoryAddressRange::GetSystemPageSize()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwPageSize;
}

CMemoryAddressRange::CMemoryAddressRange(size_t capacity, tukk name)
{
	m_nPageSize = GetSystemPageSize();

	size_t algnCap = Align(capacity, m_nPageSize);
	m_pBaseAddress = (tuk)ReserveSpace(algnCap);
	m_nPageCount = algnCap / m_nPageSize;

	if (m_pBaseAddress && name && name[0])
	{
		DrxGetIMemReplay()->RegisterFixedAddressRange(m_pBaseAddress, m_nPageSize * m_nPageCount, name);
	}
}

CMemoryAddressRange::~CMemoryAddressRange()
{
	VirtualFree(m_pBaseAddress, 0, MEM_RELEASE);
}

uk CMemoryAddressRange::MapPage(size_t pageIdx)
{
	uk pRet = VirtualAlloc(m_pBaseAddress + pageIdx * m_nPageSize, m_nPageSize, MEM_COMMIT, PAGE_READWRITE);

	#if CAPTURE_REPLAY_LOG
	if (pRet)
		DrxGetIMemReplay()->MapPage(pRet, m_nPageSize);
	#endif

	return pRet;
}

void CMemoryAddressRange::UnmapPage(size_t pageIdx)
{
	tuk pBase = m_pBaseAddress + pageIdx * m_nPageSize;

	DrxGetIMemReplay()->UnMapPage(pBase, m_nPageSize);

	// Disable warning about only decommitting pages, and not releasing them
	#pragma warning( push )
	#pragma warning( disable : 6250 )
	VirtualFree(pBase, m_nPageSize, MEM_DECOMMIT);
	#pragma warning( pop )
}

#elif DRX_PLATFORM_ORBIS

uk CMemoryAddressRange::ReserveSpace(size_t capacity)
{
	return VirtualAllocator::AllocateVirtualAddressSpace(capacity);
}

size_t CMemoryAddressRange::GetSystemPageSize()
{
	return PAGE_SIZE;
}

CMemoryAddressRange::CMemoryAddressRange(size_t capacity, tukk name)
{
	m_nPageSize = PAGE_SIZE;

	size_t algnCap = Align(capacity, std::max<size_t>(1024 * 1024, m_nPageSize));
	m_pBaseAddress = (tuk)ReserveSpace(algnCap);
	m_nPageCount = algnCap / m_nPageSize;

	if (m_pBaseAddress && name && name[0])
	{
		DrxGetIMemReplay()->RegisterFixedAddressRange(m_pBaseAddress, m_nPageSize * m_nPageCount, name);
	}
}

CMemoryAddressRange::~CMemoryAddressRange()
{
	if (m_pBaseAddress)
	{
		VirtualAllocator::FreeVirtualAddressSpace(m_pBaseAddress);
	}
}

uk CMemoryAddressRange::MapPage(size_t pageIdx)
{
	tuk pBase = m_pBaseAddress + pageIdx * m_nPageSize;
	VirtualAllocator::MapPage(pBase);

	#if CAPTURE_REPLAY_LOG
	if (pBase)
		DrxGetIMemReplay()->MapPage(pBase, m_nPageSize);
	#endif

	return pBase;
}

void CMemoryAddressRange::UnmapPage(size_t pageIdx)
{
	tuk pBase = m_pBaseAddress + pageIdx * m_nPageSize;
	DrxGetIMemReplay()->UnMapPage(pBase, m_nPageSize);

	VirtualAllocator::UnmapPage(pBase);
}

#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE

uk CMemoryAddressRange::ReserveSpace(size_t capacity)
{
	return mmap(0, capacity, PROT_NONE, MAP_ANON | MAP_NORESERVE | MAP_PRIVATE, -1, 0);
}

size_t CMemoryAddressRange::GetSystemPageSize()
{
	return sysconf(_SC_PAGESIZE);
}

CMemoryAddressRange::CMemoryAddressRange(size_t capacity, tukk name)
{
	m_nPageSize = GetSystemPageSize();

	m_allocatedSpace = Align(capacity, m_nPageSize);
	m_pBaseAddress = (tuk)ReserveSpace(m_allocatedSpace);
	assert(m_pBaseAddress != MAP_FAILED);
	m_nPageCount = m_allocatedSpace / m_nPageSize;

	if (m_pBaseAddress && name && name[0])
	{
		DrxGetIMemReplay()->RegisterFixedAddressRange(m_pBaseAddress, m_nPageSize * m_nPageCount, name);
	}
}

CMemoryAddressRange::~CMemoryAddressRange()
{
	i32 ret = munmap(m_pBaseAddress, m_allocatedSpace);
	(void) ret;
	assert(ret == 0);
}

uk CMemoryAddressRange::MapPage(size_t pageIdx)
{
	// There is no equivalent to this function with mmap, this
	// happens automatically in the OS. We just return the
	// correct address.
	uk pRet = NULL;
	if (0 == mprotect(m_pBaseAddress + (pageIdx * m_nPageSize), m_nPageSize, PROT_READ | PROT_WRITE))
	{
		pRet = m_pBaseAddress + (pageIdx * m_nPageSize);
	}
	#if CAPTURE_REPLAY_LOG
	if (pRet)
		DrxGetIMemReplay()->MapPage(pRet, m_nPageSize);
	#endif

	return pRet;
}

void CMemoryAddressRange::UnmapPage(size_t pageIdx)
{
	tuk pBase = m_pBaseAddress + pageIdx * m_nPageSize;
	i32 ret = mprotect(pBase, m_nPageSize, PROT_NONE);
	(void) ret;
	assert(ret == 0);
	DrxGetIMemReplay()->UnMapPage(pBase, m_nPageSize);
}

#endif
