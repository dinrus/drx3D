// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once
#ifndef XMLCPB_ATTRSET_TABLE_WRITER_H
	#define XMLCPB_ATTRSET_TABLE_WRITER_H

	#include <drx3D/Act/XMLCPB_Common.h>
	#include <drx3D/Act/XMLCPB_BufferWriter.h>
	#include <drx3D/CoreX/Platform/IPlatformOS.h>
	#include <drx3D/CoreX/StlUtils.h>

// an "AttrSet" defines the datatype + tagId of all the attributes of a node.  each datatype+tagId entry is called a "header".
// those sets are stored in a common table because many nodes share similar lists of attrs.

namespace XMLCPB {

class CWriter;
class CAttrSet;

class CAttrSet
{
public:

	bool operator<(const CAttrSet& other) const
	{
		if (m_numAttrs < other.m_numAttrs)
			return true;
		if (m_numAttrs > other.m_numAttrs)
			return false;

		for (u32 i = 0; i < m_numAttrs; ++i)
		{
			if (m_pHeaders[i] == other.m_pHeaders[i])
				continue;
			if (m_pHeaders[i] < other.m_pHeaders[i])
				return true;
			else
				return false;
		}
		return false;
	}

	bool operator==(const CAttrSet& other) const
	{
		if (m_numAttrs != other.m_numAttrs)
			return false;

		for (u32 i = 0; i < m_numAttrs; ++i)
		{
			if (m_pHeaders[i] != other.m_pHeaders[i])
				return false;
		}
		return true;
	}

	u16* m_pHeaders;
	u32  m_numAttrs;
};

//////////////////////////////////////////////////////////////////////////

class CAttrSetTableWriter
{
public:

	CAttrSetTableWriter(u32 maxNumSets, u32 bufferSize);
	void      Init(CWriter* pWriter);

	AttrSetID GetSetID(const CAttrSet& set);
	i32       GetNumSets() const { return m_setAddrs.size(); }
	void      WriteToFile();
	void      WriteToMemory(u8*& rpData, u32& outWriteLoc);
	u32    GetDataSize() const;
	u32    GetSetsDataSize() const;

private:

	CAttrSetTableWriter&       operator=(const CAttrSetTableWriter& other); // just to prevent assignments and copy constructors
	CAttrSetTableWriter(const CAttrSetTableWriter&);                        //
	const CAttrSetTableWriter& GetThis() const { return *this; }

	static const AttrSetID SEARCHING_ID;

	struct SetHasher
	{
		const CAttrSetTableWriter& m_owner;

		SetHasher(const CAttrSetTableWriter& owner) : m_owner(owner) {}

		ILINE static size_t HashSet(const CAttrSet& set)
		{
			size_t hash = 5381;
			for (u32 i = 0; i < set.m_numAttrs; ++i)
			{
				hash = ((hash << 5) + hash) + set.m_pHeaders[i];   /* hash * 33 + c */
			}
			return hash;
		}

		size_t operator()(const AttrSetID& ID) const
		{
			if (ID == CAttrSetTableWriter::SEARCHING_ID)
				return m_owner.m_checkHash;

			CAttrSet set;
			assert(ID < m_owner.m_numAttrsArray.size());
			set.m_numAttrs = m_owner.m_numAttrsArray[ID];
			set.m_pHeaders = (u16*)(m_owner.m_buffer.GetPointerFromAddr(m_owner.m_setAddrs[ID]));

			return HashSet(set);
		}

		bool operator()(const AttrSetID& s0Id, const AttrSetID& s1Id) const
		{
			CAttrSet set0;
			CAttrSet set1;
			m_owner.BuildSetForLessThanCheck(set0, s0Id);
			m_owner.BuildSetForLessThanCheck(set1, s1Id);
			return set0 == set1;
		}
	};

private:
	ILINE void BuildSetForLessThanCheck(CAttrSet& set, const AttrSetID ID) const
	{
		assert(ID != XMLCPB_INVALID_ID);

		if (ID == SEARCHING_ID)
		{
			set = *m_pCheckSet;
		}
		else
		{
			assert(ID < m_numAttrsArray.size());
			set.m_numAttrs = m_numAttrsArray[ID];
			set.m_pHeaders = (u16*)(m_buffer.GetPointerFromAddr(m_setAddrs[ID]));
		}
	}

	AttrSetID AddSet(const CAttrSet& set);

	typedef std::vector<FlatAddr16> FlatAddrVec;
	void CalculateFlatAddrs(FlatAddrVec& outFlatAddrs);

private:

	std::vector<u8>                m_numAttrsArray; // stores the number of attrs that each sets has. indexed by id.
	CBufferWriter                     m_buffer;        // all headers are sequentially stored here
	std::vector<CBufferWriter::SAddr> m_setAddrs;      // pointers into m_buffer
	typedef std::unordered_set<AttrSetID, SetHasher, SetHasher, AttrStringAllocator<AttrSetID>> HashTableType;
	HashTableType                     m_sortedIndexes;
	i32                               m_maxNumSets;   // used for checks. we need to control it because in the final data we use a limited amount of bits to store the ID.
	CWriter*                          m_pWriter;
	const CAttrSet*                   m_pCheckSet;    // only used when comparing sets inside the GetSetID function. it should be NULL otherwise
	size_t                            m_checkHash;    // ditto
};

}  // end namespace

#endif
