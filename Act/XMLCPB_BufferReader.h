// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once

#ifndef XMLCPB_BUFFERREADER_H
	#define XMLCPB_BUFFERREADER_H

	#include <drx3D/Act/XMLCPB_Common.h>
	#include <drx3D/CoreX/Platform/IPlatformOS.h>

namespace XMLCPB {

class CReader;

struct SBufferReader
{
	explicit SBufferReader(IGeneralMemoryHeap* pHeap)
		: m_pHeap(pHeap)
		, m_pBuffer(NULL)
		, m_bufferSize(0)
	{
	}

	~SBufferReader()
	{
		if (m_pBuffer)
			m_pHeap->Free(m_pBuffer);
	}

	inline u8k* GetPointer(FlatAddr addr) const
	{
		assert(addr < m_bufferSize);
		return &(m_pBuffer[addr]);
	}

	inline void CopyTo(u8* pDst, FlatAddr srcAddr, u32 bytesToCopy) const
	{
		assert(srcAddr < m_bufferSize);
		assert(srcAddr + bytesToCopy <= m_bufferSize);

		memcpy(pDst, GetPointer(srcAddr), bytesToCopy);
	}

	void ReadFromFile(CReader& Reader, IPlatformOS::ISaveReaderPtr& pOSSaveReader, u32 readSize);
	void ReadFromMemory(CReader& Reader, u8k* pData, u32 dataSize, u32 readSize, u32& outReadLoc);

private:
	SBufferReader(const SBufferReader&);
	SBufferReader& operator=(const SBufferReader&);

private:
	_smart_ptr<IGeneralMemoryHeap> m_pHeap;
	u8*                         m_pBuffer;
	size_t                         m_bufferSize;
};

}  // end namespace

#endif
