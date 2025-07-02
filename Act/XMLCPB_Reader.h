// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once
#ifndef XMLCPB_READER_H
	#define XMLCPB_READER_H

	#include <drx3D/Act/XMLCPB_StringTableReader.h>
	#include <drx3D/Act/XMLCPB_AttrSetTableReader.h>
	#include <drx3D/Act/XMLCPB_NodeLiveReader.h>
	#include <drx3D/Act/XMLCPB_BufferReader.h>
	#include <drx3D/CoreX/Platform/IPlatformOS.h>

namespace XMLCPB {

// CReader should not be used directly from outside. use CReaderInterface instead.

class CReader
{
public:
	static _smart_ptr<IGeneralMemoryHeap> CreateHeap();

public:

	explicit CReader(IGeneralMemoryHeap* pHeap);
	~CReader();

	CNodeLiveReaderRef         GetRoot();
	CNodeLiveReaderRef         CreateNodeRef();
	bool                       ReadBinaryFile(tukk pFileName);
	bool                       ReadBinaryMemory(u8k* pData, u32 uSize);
	void                       SaveTestFiles();
	u32                     GetTotalDataSize() const     { return m_totalSize; }
	u32                     GetNodesDataSize() const     { return m_nodesDataSize; }
	u32                     GetNumNodes() const          { return m_numNodes; }

	const CStringTableReader&  GetAttrNamesTable() const    { return m_tableAttrNames; }
	const CStringTableReader&  GetStrDataTable() const      { return m_tableStrData; }
	const CStringTableReader&  GetTagsTable() const         { return m_tableTags; }
	const CAttrSetTableReader& GetDataTypeSetsTable() const { return m_tableAttrSets; }
	const CNodeLiveReader&     ActivateLiveNodeFromCompact(NodeGlobalID nodeId);
	CNodeLiveReader*           GetNodeLive(NodeLiveID nodeId);
	void                       FreeNodeLive(NodeLiveID nodeId);
	template<class T>
	FlatAddr                   ReadFromBuffer(FlatAddr addr, T& data);
	template<class T>
	FlatAddr                   ReadFromBufferEndianAware(FlatAddr addr, T& data);
	FlatAddr                   ReadFromBuffer(FlatAddr addr, u8*& rdata, u32 len);
	void                       ReadDataFromFile(IPlatformOS::ISaveReaderPtr& pOSSaveReader, uk pDst, u32 numBytes);
	void                       ReadDataFromMemory(u8k* pData, u32 dataSize, uk pDst, u32 numBytes, u32& outReadLoc);
	FlatAddr                   GetAddrNode(NodeGlobalID id) const;
	u8k*               GetPointerFromFlatAddr(FlatAddr addr) const { return m_buffer.GetPointer(addr); }

private:

	void CheckErrorFlag(IPlatformOS::EFileOperationCode code);
	void ReadDataFromFileInternal(IPlatformOS::ISaveReaderPtr& pOSSaveReader, uk pSrc, u32 numBytes);
	void ReadDataFromZLibBuffer(IPlatformOS::ISaveReaderPtr& pOSSaveReader, u8*& pDst, u32& numBytesToRead);
	void CreateNodeAddressTables();

	#ifdef XMLCPB_CHECK_FILE_INTEGRITY
	bool CheckFileCorruption(IPlatformOS::ISaveReaderPtr& pOSSaveReader, const SFileHeader& fileHeader, u32 totalSize);
	#endif

private:
	typedef DynArray<CNodeLiveReader, i32, NArray::SmallDynStorage<NAlloc::GeneralHeapAlloc>> LiveNodesVec;
	typedef DynArray<FlatAddr, i32, NArray::SmallDynStorage<NAlloc::GeneralHeapAlloc>>        FlatAddrVec;

	_smart_ptr<IGeneralMemoryHeap> m_pHeap;
	LiveNodesVec                   m_liveNodes;
	SBufferReader                  m_buffer;
	u32                         m_firstFreeLiveNode;
	CStringTableReader             m_tableTags;
	CStringTableReader             m_tableAttrNames;
	CStringTableReader             m_tableStrData;
	CAttrSetTableReader            m_tableAttrSets;
	u32                         m_maxNumActiveNodes;
	u32                         m_numActiveNodes;
	u32                         m_totalSize;
	u32                         m_nodesDataSize;
	u8*                         m_pZLibBuffer;                // zlib output uncompressed data buffer
	u8*                         m_pZLibCompressedBuffer;      // zlib input compressed data buffer
	u32                         m_ZLibBufferSizeWithData;     // how much of m_pZLibBuffer is filled with actual data
	u32                         m_ZLibBufferSizeAlreadyRead;  // how much of m_pZLibBuffer is already been read
	u32                         m_numNodes;
	bool                           m_errorReading;
	FlatAddrVec                    m_nodesAddrTable;     // stores the address of each node. Index is the NodeGlobalId (which is the order in the big buffer)

	static i32                     MAX_NUM_LIVE_NODES;
};

template<class T>
FlatAddr CReader::ReadFromBuffer(FlatAddr addr, T& data)
{
	u8* pDst = (u8*)(&data);

	m_buffer.CopyTo(pDst, addr, sizeof(T));

	return addr + sizeof(T);
}

template<class T>
FlatAddr CReader::ReadFromBufferEndianAware(FlatAddr addr, T& data)
{
	u8* pDst = (u8*)(&data);

	m_buffer.CopyTo(pDst, addr, sizeof(T));

	SwapIntegerValue(data);

	return addr + sizeof(T);
}

//////////////////////////////////////////////////////////////////////////

inline FlatAddr CReader::GetAddrNode(NodeGlobalID id) const
{
	assert(id < m_numNodes);

	FlatAddr addr = m_nodesAddrTable[id];
	return addr;
}

}  // end namespace

#endif
