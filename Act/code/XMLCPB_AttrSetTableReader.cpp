// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XMLCPB_AttrSetTableReader.h>
#include <drx3D/Act/XMLCPB_Reader.h>

using namespace XMLCPB;

CAttrSetTableReader::CAttrSetTableReader(IGeneralMemoryHeap* pHeap)
	: m_setAddrs(NAlloc::GeneralHeapAlloc(pHeap))
	, m_numAttrs(NAlloc::GeneralHeapAlloc(pHeap))
	, m_buffer(pHeap)
{
}

//////////////////////////////////////////////////////////////////////////
// returns the num of attributes that a given AttrSet has

u32 CAttrSetTableReader::GetNumAttrs(AttrSetID setId) const
{
	assert(setId < m_numAttrs.size());
	u32 num = m_numAttrs[setId];
	return num;
}

//////////////////////////////////////////////////////////////////////////
// returns the header of an attr (16bits value that combines datatype+tagId)

u16 CAttrSetTableReader::GetHeaderAttr(AttrSetID setId, u32 indAttr) const
{
	assert(setId < m_setAddrs.size());
	assert(indAttr < m_numAttrs[setId]);

	FlatAddr flatAddr = m_setAddrs[setId];
	u16* pHeader = (u16*)(m_buffer.GetPointer(flatAddr));
	pHeader += indAttr;
	return *pHeader;
}

//////////////////////////////////////////////////////////////////////////

void CAttrSetTableReader::ReadFromFile(CReader& Reader, IPlatformOS::ISaveReaderPtr& pOSSaveReader, const SFileHeader& headerInfo)
{
	assert(m_numAttrs.empty() && m_setAddrs.empty());

	m_numAttrs.resize(headerInfo.m_numAttrSets);
	m_setAddrs.resize(headerInfo.m_numAttrSets);

	u8* pNumAttrsTable = &(m_numAttrs[0]);
	Reader.ReadDataFromFile(pOSSaveReader, pNumAttrsTable, sizeof(*pNumAttrsTable) * headerInfo.m_numAttrSets);

	FlatAddr16* pSetAddrsTable = &(m_setAddrs[0]);
	Reader.ReadDataFromFile(pOSSaveReader, pSetAddrsTable, sizeof(*pSetAddrsTable) * headerInfo.m_numAttrSets);

	m_buffer.ReadFromFile(Reader, pOSSaveReader, headerInfo.m_sizeAttrSets);
}

//////////////////////////////////////////////////////////////////////////

void CAttrSetTableReader::ReadFromMemory(CReader& Reader, u8k* pData, u32 dataSize, const SFileHeader& headerInfo, u32& outReadLoc)
{
	assert(m_numAttrs.empty() && m_setAddrs.empty());

	m_numAttrs.resize(headerInfo.m_numAttrSets);
	m_setAddrs.resize(headerInfo.m_numAttrSets);

	u8* pNumAttrsTable = &(m_numAttrs[0]);
	Reader.ReadDataFromMemory(pData, dataSize, pNumAttrsTable, sizeof(*pNumAttrsTable) * headerInfo.m_numAttrSets, outReadLoc);

	FlatAddr16* pSetAddrsTable = &(m_setAddrs[0]);
	Reader.ReadDataFromMemory(pData, dataSize, pSetAddrsTable, sizeof(*pSetAddrsTable) * headerInfo.m_numAttrSets, outReadLoc);

	m_buffer.ReadFromMemory(Reader, pData, dataSize, headerInfo.m_sizeAttrSets, outReadLoc);
}
