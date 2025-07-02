// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     04/05/2015 by Filipe amim
//  Описание:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

namespace pfx2
{

ILINE void CopyToVideoMemory(uk pDst, uk pSrc, u32 size)
{
	//	assert(((uintptr_t)pDst & 127) == ((uintptr_t)pSrc & 127));
	memcpy(pDst, pSrc, size);
}

template<class T, uint bufferSize, uint chunckSize>
ILINE CWriteCombinedBuffer<T, bufferSize, chunckSize>::CWriteCombinedBuffer(FixedDynArray<T>& destMem)
	: m_pDestMem(&destMem)
	, m_pDestMemBegin((byte*)destMem.begin())
	, m_flushedBytes(0)
	, m_writtenDestBytes(0)
{
	uint alignOffset = alias_cast<uint>(m_pDestMemBegin - (byte*)m_srcBuffer) & (DRX_PFX2_PARTICLES_ALIGNMENT - 1);
	m_pSrcMemBegin = m_srcBuffer + alignOffset;
	i32 capacity = (sizeof(m_srcBuffer) - alignOffset) / sizeof(T);
	capacity = min(capacity, destMem.capacity());
	m_srcArray.set(ArrayT((T*)m_pSrcMemBegin, capacity));
}

template<class T, uint bufferSize, uint chunckSize>
ILINE CWriteCombinedBuffer<T, bufferSize, chunckSize>::~CWriteCombinedBuffer()
{
	// Write remaining data.
	FlushData(UnflushedBytes());

	// Set final count of elems written to dest buffer.
	//	DRX_PFX2_ASSERT(m_writtenDestBytes % sizeof(T) == 0);
	m_pDestMem->resize(m_writtenDestBytes / sizeof(T), eNoInit);
}

template<class T, uint bufferSize, uint chunckSize>
ILINE FixedDynArray<T>& CWriteCombinedBuffer<T, bufferSize, chunckSize >::Array()
{
	return m_srcArray;
}

template<class T, uint bufferSize, uint chunckSize>
ILINE T* CWriteCombinedBuffer<T, bufferSize, chunckSize >::CheckAvailable(i32 elems)
{
	// Transfer data chunks no longer needed
	uint unflushedBytes = UnflushedBytes();
	if (unflushedBytes >= chunckSize)
		FlushData(unflushedBytes & ~(chunckSize - 1));

	// If buffer is too full for new elements, clear out flushed data.
	if (m_srcArray.available() < elems)
	{
		WrapBuffer();
		if (m_srcArray.available() < elems)
			return 0;
	}
	return m_srcArray.end();
}

template<class T, uint bufferSize, uint chunckSize>
ILINE uint CWriteCombinedBuffer<T, bufferSize, chunckSize >::UnflushedBytes() const
{
	return check_cast<uint>((byte*)m_srcArray.end() - m_pSrcMemBegin - m_flushedBytes);
}

template<class T, uint bufferSize, uint chunckSize>
ILINE void CWriteCombinedBuffer<T, bufferSize, chunckSize >::FlushData(uint flushBytes)
{
	if (flushBytes)
	{
		//		DRX_PFX2_ASSERT(m_writtenDestBytes + flushBytes <= m_pDestMem->capacity() * sizeof(T));
		CopyToVideoMemory(
		  m_pDestMemBegin + m_writtenDestBytes,
		  m_pSrcMemBegin + m_flushedBytes,
		  flushBytes);
		m_writtenDestBytes += flushBytes;
		m_flushedBytes += flushBytes;
	}
}

template<class T, uint bufferSize, uint chunckSize>
ILINE void CWriteCombinedBuffer<T, bufferSize, chunckSize >::WrapBuffer()
{
	// Copy unflushed bytes to the buffer beginning
	uint unflushedBytes = UnflushedBytes();
	memcpy(m_pSrcMemBegin, m_pSrcMemBegin + m_flushedBytes, unflushedBytes);
	uint availableBytes = check_cast<uint>(m_srcBuffer + sizeof(m_srcBuffer) - m_pSrcMemBegin - unflushedBytes);
	availableBytes = min(availableBytes, m_pDestMem->capacity() * (uint)sizeof(T) - m_writtenDestBytes - unflushedBytes);
	m_srcArray.set(ArrayT((T*)(m_pSrcMemBegin + unflushedBytes), i32(availableBytes / sizeof(T))));
	m_flushedBytes = 0;
}

}
