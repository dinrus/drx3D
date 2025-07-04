// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XMLCPB_AttrSetTableWriter.h>
#include <drx3D/Act/XMLCPB_Writer.h>

using namespace XMLCPB;

const AttrSetID CAttrSetTableWriter::SEARCHING_ID = 0Xfffffffe;

//////////////////////////////////////////////////////////////////////////

CAttrSetTableWriter::CAttrSetTableWriter(u32 maxNumSets, u32 bufferSize)
	: m_maxNumSets(maxNumSets)
	, m_buffer(bufferSize)
	, m_pWriter(NULL)
	, m_pCheckSet(NULL)
	, m_checkHash(0)
	, m_sortedIndexes(256, SetHasher(GetThis()), SetHasher(GetThis()))
{
}

void CAttrSetTableWriter::Init(CWriter* pWriter)
{
	m_pWriter = pWriter;
	m_buffer.Init(pWriter, false);
}

//////////////////////////////////////////////////////////////////////////
// AddSet could be used directly instead, with some changes. But then it would be very inefficient because it would need to copy every to-compare set first into the buffer.
AttrSetID CAttrSetTableWriter::GetSetID(const CAttrSet& set)
{
	m_pCheckSet = &set;  // the internal std::set comparison code will use this set when trying to find SEARCHING_ID
	m_checkHash = SetHasher::HashSet(set);
	HashTableType::const_iterator iter = m_sortedIndexes.find(SEARCHING_ID);
	m_pCheckSet = NULL;

	// found the set
	if (iter != m_sortedIndexes.end())
		return *iter;

	// didnt find
	AttrSetID ID = AddSet(set);
	return ID;
}

//////////////////////////////////////////////////////////////////////////

AttrSetID CAttrSetTableWriter::AddSet(const CAttrSet& set)
{
	assert(m_setAddrs.size() < m_maxNumSets);
	assert(set.m_numAttrs <= MAX_NUM_ATTRS);

	AttrSetID addedSetId = m_setAddrs.size();
	m_setAddrs.push_back(CBufferWriter::SAddr());
	CBufferWriter::SAddr& setAddr = m_setAddrs.back();
	m_buffer.AddDataNoSplit(setAddr, set.m_pHeaders, sizeof(set.m_pHeaders[0]) * set.m_numAttrs);  // no split = can not be internally splited in diferent internal buffers. Because we want to be able to read it in one go with the translated pointer
	m_numAttrsArray.push_back(set.m_numAttrs);

	m_sortedIndexes.insert(addedSetId);

	return addedSetId;
}

//////////////////////////////////////////////////////////////////////////

void CAttrSetTableWriter::WriteToFile()
{
	u32 numSets = GetNumSets();
	if (numSets == 0)
		return;

	FlatAddrVec flatAddrs;
	CalculateFlatAddrs(flatAddrs);

	u8* pNumTypes = &(m_numAttrsArray[0]);
	m_pWriter->WriteDataIntoFile(pNumTypes, sizeof(*pNumTypes) * numSets);

	FlatAddr16* pFlatAddrs = &(flatAddrs[0]);
	m_pWriter->WriteDataIntoFile(pFlatAddrs, sizeof(*pFlatAddrs) * numSets);

	m_buffer.WriteToFile();
}

//////////////////////////////////////////////////////////////////////////

void CAttrSetTableWriter::WriteToMemory(u8*& rpData, u32& outWriteLoc)
{
	u32 numSets = GetNumSets();
	if (numSets == 0)
		return;

	FlatAddrVec flatAddrs;
	CalculateFlatAddrs(flatAddrs);

	u8* pNumTypes = &(m_numAttrsArray[0]);
	m_pWriter->WriteDataIntoMemory(rpData, pNumTypes, sizeof(*pNumTypes) * numSets, outWriteLoc);

	FlatAddr16* pFlatAddrs = &(flatAddrs[0]);
	m_pWriter->WriteDataIntoMemory(rpData, pFlatAddrs, sizeof(*pFlatAddrs) * numSets, outWriteLoc);

	m_buffer.WriteToMemory(rpData, outWriteLoc);
}

//////////////////////////////////////////////////////////////////////////
// total size

u32 CAttrSetTableWriter::GetDataSize() const
{
	u32 size = GetNumSets() * sizeof(u8) + GetNumSets() * sizeof(FlatAddr16);
	size += GetSetsDataSize();

	return size;
}

//////////////////////////////////////////////////////////////////////////
// only the size of all the headers

u32 CAttrSetTableWriter::GetSetsDataSize() const
{
	u32 size = m_buffer.GetDataSize();
	return size;
}

//////////////////////////////////////////////////////////////////////////

void CAttrSetTableWriter::CalculateFlatAddrs(FlatAddrVec& outFlatAddrs)
{
	u32 numSets = GetNumSets();
	outFlatAddrs.resize(numSets);

	for (i32 i = 0; i < numSets; ++i)
	{
		FlatAddr16& flatAddr = outFlatAddrs[i];
		flatAddr = m_buffer.ConvertSegmentedAddrToFlatAddr(m_setAddrs[i]);
	}
}
