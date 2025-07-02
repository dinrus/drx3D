// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once
#ifndef XMLCPB_STRING_TABLE_READER_H
	#define XMLCPB_STRING_TABLE_READER_H

	#include <drx3D/Act/XMLCPB_Common.h>
	#include <drx3D/Act/XMLCPB_BufferReader.h>
	#include <drx3D/CoreX/Platform/IPlatformOS.h>

namespace XMLCPB {

class CReader;

class CStringTableReader
{
public:
	explicit CStringTableReader(IGeneralMemoryHeap* pHeap);

	tukk GetString(StringID stringId) const;
	i32         GetNumStrings() const { return m_stringAddrs.size(); }

	void        ReadFromFile(CReader& Reader, IPlatformOS::ISaveReaderPtr pOSSaveReader, const SFileHeader::SStringTable& headerInfo);
	void        ReadFromMemory(CReader& Reader, u8k* pData, u32 dataSize, const SFileHeader::SStringTable& headerInfo, u32& outReadLoc);

	// for debug and testing
	#ifndef _RELEASE
	void WriteStringsIntoTextFile(tukk pFileName);
	#endif

private:
	typedef DynArray<FlatAddr, i32, NArray::SmallDynStorage<NAlloc::GeneralHeapAlloc>> FlatAddrVec;

private:
	SBufferReader m_buffer;
	FlatAddrVec   m_stringAddrs;
};

}  // end namespace

#endif
