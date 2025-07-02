// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once
#ifndef XMLCPB_ATTRSET_TABLE_READER_H
	#define XMLCPB_ATTRSET_TABLE_READER_H

	#include <drx3D/Act/XMLCPB_Common.h>
	#include <drx3D/Act/XMLCPB_BufferReader.h>
	#include <drx3D/CoreX/Platform/IPlatformOS.h>

// an "AttrSet" defines the datatype + tagId of all the attributes of a node.  each datatype+tagId entry is called a "header".
// those sets are stored in a common table because many nodes use the same type of attrs

namespace XMLCPB {

class CReader;

class CAttrSetTableReader
{
public:
	explicit CAttrSetTableReader(IGeneralMemoryHeap* pHeap);

	u32 GetNumAttrs(AttrSetID setId) const;
	u16 GetHeaderAttr(AttrSetID setId, u32 indAttr) const;

	u32 GetNumSets() const { return m_setAddrs.size(); }

	void   ReadFromFile(CReader& Reader, IPlatformOS::ISaveReaderPtr& pOSSaveReader, const SFileHeader& headerInfo);
	void   ReadFromMemory(CReader& Reader, u8k* pData, u32 dataSize, const SFileHeader& headerInfo, u32& outReadLoc);

private:
	typedef DynArray<FlatAddr16, i32, NArray::SmallDynStorage<NAlloc::GeneralHeapAlloc>> SetAddrVec;
	typedef DynArray<u8, i32, NArray::SmallDynStorage<NAlloc::GeneralHeapAlloc>>      NumAttrsVec;

	SetAddrVec    m_setAddrs;
	NumAttrsVec   m_numAttrs;
	SBufferReader m_buffer;
};

}  // end namespace

#endif
