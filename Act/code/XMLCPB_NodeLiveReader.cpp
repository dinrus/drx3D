// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XMLCPB_NodeLiveReader.h>
#include <drx3D/Act/XMLCPB_Reader.h>

using namespace XMLCPB;

//////////////////////////////////////////////////////////////////////////

void CNodeLiveReaderRef::FreeRef()
{
	if (m_nodeId != XMLCPB_INVALID_ID && m_nodeId != XMLCPB_ROOTNODE_ID)
	{
		CNodeLiveReader* pNode = m_Reader.GetNodeLive(m_nodeId);

		assert(pNode->IsValid());

		if (pNode->IsValid())
		{
			assert(pNode->m_refCounter > 0);
			pNode->m_refCounter--;
			if (pNode->m_refCounter == 0)
				m_Reader.FreeNodeLive(m_nodeId);
		}
	}
	m_nodeId = XMLCPB_INVALID_ID;
	m_pNode_Debug = NULL;
}

/////////////////
void CNodeLiveReaderRef::CopyFrom(const CNodeLiveReaderRef& other)
{
	assert(m_nodeId == XMLCPB_INVALID_ID);
	m_nodeId = other.m_nodeId;
	m_pNode_Debug = NULL;
	if (m_nodeId != XMLCPB_INVALID_ID)
	{
		CNodeLiveReader* pNode = m_Reader.GetNodeLive(m_nodeId);
		m_pNode_Debug = pNode;
		assert(pNode->IsValid());
		pNode->m_refCounter++;
	}
}

///////////////
CNodeLiveReader* CNodeLiveReaderRef::GetNode() const
{
	if (m_nodeId == XMLCPB_INVALID_ID)
		return NULL;

	CNodeLiveReader* pNode = m_Reader.GetNodeLive(m_nodeId);

	assert(pNode->IsValid());

	if (!pNode->IsValid())
		return NULL;

	return pNode;
}

/////////////////
CNodeLiveReaderRef::CNodeLiveReaderRef(CReader& Reader, NodeLiveID nodeId)
	: m_Reader(Reader)
	, m_nodeId(nodeId)
{
	CNodeLiveReader* pNode = GetNode();
	m_pNode_Debug = pNode;
	if (pNode)
		pNode->m_refCounter++;
}

//////////////////////////////////////////////////////////////////////////

CNodeLiveReader::~CNodeLiveReader()
{
}

//////////////////////////////////////////////////////////////////////////

CNodeLiveReaderRef CNodeLiveReader::GetChildNode(tukk pChildName)
{
	bool found = false;
	NodeGlobalID childId = XMLCPB_INVALID_ID;

	if (m_numChildren > 0)
	{
		// first try with the next child. if is not the one, then look from start
		childId = GetChildGlobalId(m_indexOfNextChild);
		if (m_indexOfNextChild < m_numChildren && strcmp(pChildName, GetTagNode(childId)) == 0)
		{
			found = true;
		}

		// if not found, then just look on all from the start
		if (!found)
		{
			i32 childNumber = 0;

			u32 childrenLeftToCheck = m_numChildren;
			u32 numChildBlocks = m_childBlocks.size();
			for (uint block = 0; block < numChildBlocks && !found; ++block)
			{
				uint childrenInBlock = min(u32(CHILDREN_PER_BLOCK), childrenLeftToCheck);
				childrenLeftToCheck -= childrenInBlock;
				childId = m_childBlocks[block];

				for (i32 i = 0; i < childrenInBlock; ++i)
				{
					if (strcmp(pChildName, GetTagNode(childId)) == 0)
					{
						found = true;
						m_indexOfNextChild = childNumber;
						break;
					}
					++childNumber;
					++childId;
				}
			}
		}
	}

	CNodeLiveReaderRef nodeRef(m_Reader);

	if (found)
	{
		const CNodeLiveReader& node = m_Reader.ActivateLiveNodeFromCompact(childId);
		nodeRef = CNodeLiveReaderRef(m_Reader, node.GetLiveId());
		m_indexOfNextChild = (m_indexOfNextChild + 1) % m_numChildren;
	}

	return nodeRef;
}

//////////////////////////////////////////////////////////////////////////

CNodeLiveReaderRef CNodeLiveReader::GetChildNode(u32 index)
{
	assert(index < m_numChildren);

	// create the reference object
	const CNodeLiveReader& child = m_Reader.ActivateLiveNodeFromCompact(GetChildGlobalId(index));
	CNodeLiveReaderRef childRef(m_Reader, child.GetLiveId());

	// now prepares	to point to the next child
	m_indexOfNextChild = (index + 1) % m_numChildren;

	return childRef;
}

//////////////////////////////////////////////////////////////////////////

NodeGlobalID CNodeLiveReader::GetChildGlobalId(u32 indChild) const
{
	u32 block = indChild / CHILDREN_PER_BLOCK;
	u32 indInBlock = indChild % CHILDREN_PER_BLOCK;
	NodeGlobalID childId = m_childBlocks[block] + indInBlock;
	return childId;
}

//////////////////////////////////////////////////////////////////////////

u32 CNodeLiveReader::GetSizeWithoutChilds() const
{
	FlatAddr addr0 = m_Reader.GetAddrNode(m_globalId);
	FlatAddr addr1 = m_globalId + 1 == m_Reader.GetNumNodes() ? m_Reader.GetNodesDataSize() : m_Reader.GetAddrNode(m_globalId + 1);

	u32 size = addr1 - addr0;

	if (addr1 == XMLCPB_INVALID_FLATADDR)
		size = m_Reader.GetNodesDataSize() - addr0;

	return size;
}

//////////////////////////////////////////////////////////////////////////
// constructs the NodeLive from the compacted (raw) binary data
// CNodeLive::Compact() has the format description for the data

void CNodeLiveReader::ActivateFromCompact(NodeLiveID liveId, NodeGlobalID globalId)
{
	assert(!IsValid());

	Reset();

	m_liveId = liveId;
	m_globalId = globalId;
	FlatAddr nextAddr = m_Reader.GetAddrNode(m_globalId);
	m_valid = true;

	// --- read header----
	u16 header;
	nextAddr = m_Reader.ReadFromBuffer(nextAddr, header);

	StringID stringId = header & MASK_TAGID;
	m_pTag = m_Reader.GetTagsTable().GetString(stringId);

	bool childrenAreRightBefore = IsFlagActive(header, FLN_CHILDREN_ARE_RIGHT_BEFORE);
	bool hasAttrs = IsFlagActive(header, FLN_HASATTRS);

	// --------- children ------------
	m_numChildren = (header & CHILDREN_HEADER_MASK) >> CHILDREN_HEADER_BIT_POSITION;
	bool childrenAndDistStoredInFirstByte = false;
	if (m_numChildren == CHILDREN_HEADER_CANTFIT) // if cant fit in the header, it meants that there are additional byte(s) to store the number
	{
		u8 val;
		nextAddr = m_Reader.ReadFromBuffer(nextAddr, val);
		if (IsFlagActive(val, FL_CHILDREN_NUMBER_IS_16BITS))
		{
			u8 val2;
			nextAddr = m_Reader.ReadFromBuffer(nextAddr, val2);
			m_numChildren = (val2 | (val << 8)) & MAX_NUMBER_OF_CHILDREN_IN_16BITS; // build the number of children
		}
		else
		{
			if (IsFlagActive(val, FL_CHILDREN_NUMBER_AND_BLOCKDIST_IN_ONE_BYTE)) // number of children and dist to them is stored in the single byte
			{
				m_numChildren = (val & CHILDANDDISTINONEBYTE_MAX_AMOUNT_CHILDREN) + CHILDREN_HEADER_MAXNUM;          // +CHILDREN_HEADER_MAXNUM because that is subtracted when stored
				u32 distChild = ((val >> CHILDANDDISTINONEBYTE_CHILD_BITS) & CHILDANDDISTINONEBYTE_MAX_DIST) + 1; // +1 because is subtracted when stored
				m_childBlocks.resize(1);
				m_childBlocks[0] = m_globalId - (distChild + m_numChildren);
				childrenAndDistStoredInFirstByte = true;
			}
			else
			{
				m_numChildren = val;
			}
		}
	}
	u32 numChildBlocks = m_numChildren / CHILDREN_PER_BLOCK + ((m_numChildren % CHILDREN_PER_BLOCK) > 0 ? 1 : 0);
	m_childBlocks.resize(numChildBlocks);

	// ---------- attrSetId ------------------
	if (hasAttrs)
	{
		u8 val;
		nextAddr = m_Reader.ReadFromBuffer(nextAddr, val);
		u32 highBits = (header & HIGHPART_ATTRID_MASK) >> (HIGHPART_ATTRID_BIT_POSITION - 8);
		m_attrsSetId = highBits | val;

		if (m_attrsSetId == ATRID_USING_16_BITS)
		{
			u16 val16;
			nextAddr = m_Reader.ReadFromBuffer(nextAddr, val16);
			m_attrsSetId = val16;
		}

		m_numAttrs = m_Reader.GetDataTypeSetsTable().GetNumAttrs(m_attrsSetId);
	}

	// --------- children blocks ----------
	if (!childrenAndDistStoredInFirstByte && !childrenAreRightBefore && numChildBlocks > 0)
	{
		if (numChildBlocks == 1)
		{
			u8 val;
			nextAddr = m_Reader.ReadFromBuffer(nextAddr, val);
			if (!IsFlagActive(val, CHILDBLOCKS_USING_MORE_THAN_8BITS))
			{
				m_childBlocks[0] = m_globalId - (val + m_numChildren);
			}
			else if (!IsFlagActive(val, CHILDBLOCKS_USING_24BITS))
			{
				u8 val2;
				nextAddr = m_Reader.ReadFromBuffer(nextAddr, val2);
				u32 dist = ((val & CHILDBLOCKS_MASK_TO_REMOVE_FLAGS) << 8) | val2;
				m_childBlocks[0] = m_globalId - (dist + m_numChildren);
			}
			else // as 24 bits
			{
				val = val & CHILDBLOCKS_MASK_TO_REMOVE_FLAGS;
				u16 low;
				nextAddr = m_Reader.ReadFromBuffer(nextAddr, low);
				u32 dist = low | (val << 16);
				m_childBlocks[0] = m_globalId - (dist + m_numChildren);
			}
		}
		else // when there are more than 1 block, they are always stored as 24 bits
		{
			for (u32 i = 0; i < numChildBlocks; ++i)
			{
				u8 high;
				nextAddr = m_Reader.ReadFromBuffer(nextAddr, high);
				high &= CHILDBLOCKS_MASK_TO_REMOVE_FLAGS;
				u16 low;
				nextAddr = m_Reader.ReadFromBuffer(nextAddr, low);
				u32 dist = low | (high << 16);
				u32 numChildsInBlock = min(u32(CHILDREN_PER_BLOCK), m_numChildren - (i * CHILDREN_PER_BLOCK));
				m_childBlocks[i] = m_globalId - (dist + numChildsInBlock);
			}
		}
	}

	if (childrenAreRightBefore)
	{
		m_childBlocks[0] = m_globalId - m_numChildren;
	}

	m_addrFirstAttr = nextAddr;
}

//////////////////////////////////////////////////////////////////////////

tukk CNodeLiveReader::GetTagNode(NodeGlobalID nodeId) const
{
	FlatAddr addr = m_Reader.GetAddrNode(nodeId);

	u16 header;
	m_Reader.ReadFromBuffer(addr, header);

	StringID stringId = header & MASK_TAGID;
	tukk pTag = m_Reader.GetTagsTable().GetString(stringId);
	return pTag;
}

//////////////////////////////////////////////////////////////////////////

void CNodeLiveReader::Reset()
{
	m_liveId = XMLCPB_INVALID_ID;
	m_globalId = XMLCPB_INVALID_ID;
	m_refCounter = 0;
	m_valid = false;
	m_pTag = NULL;
	m_numChildren = 0;
	m_numAttrs = 0;
	m_addrFirstAttr = XMLCPB_INVALID_FLATADDR;
	m_indexOfNextChild = 0;
	m_childBlocks.clear();
	m_attrsSetId = XMLCPB_INVALID_ID;
}

//////////////////////////////////////////////////////////////////////////

FlatAddr CNodeLiveReader::GetAddrNextNode() const
{
	FlatAddr nextAddr = m_addrFirstAttr;
	for (i32 a = 0; a < m_numAttrs; a++)
	{
		CAttrReader attr(m_Reader);
		attr.InitFromCompact(nextAddr, m_Reader.GetDataTypeSetsTable().GetHeaderAttr(m_attrsSetId, a));
		nextAddr = attr.GetAddrNextAttr();
	}

	return nextAddr;
}

//////////////////////////////////////////////////////////////////////////

bool CNodeLiveReader::ReadAttr(tukk pAttrName, u8*& rdata, u32& outSize) const
{
	CAttrReader attr(m_Reader);
	bool found = FindAttr(pAttrName, attr);

	if (found)
		attr.Get(rdata, outSize);

	return found;
}

//////////////////////////////////////////////////////////////////////////

void CNodeLiveReader::ReadAttr(u32 index, u8*& rdata, u32& outSize) const
{
	CAttrReader attr(m_Reader);
	GetAttr(index, attr);
	attr.Get(rdata, outSize);
}

//////////////////////////////////////////////////////////////////////////

bool CNodeLiveReader::HaveAttr(tukk pAttrName) const
{
	CAttrReader attr(m_Reader);
	bool have = FindAttr(pAttrName, attr);
	return have;
}

//////////////////////////////////////////////////////////////////////////

bool CNodeLiveReader::FindAttr(tukk pAttrName, CAttrReader& attr) const
{
	FlatAddr nextAddr = m_addrFirstAttr;

	bool found = false;
	for (i32 a = 0; a < m_numAttrs; a++)
	{
		attr.InitFromCompact(nextAddr, m_Reader.GetDataTypeSetsTable().GetHeaderAttr(m_attrsSetId, a));
		if (strcmp(pAttrName, attr.GetName()) == 0)
		{
			found = true;
			break;
		}
		nextAddr = attr.GetAddrNextAttr();
	}

	return found;
}

//////////////////////////////////////////////////////////////////////////

void CNodeLiveReader::GetAttr(uint index, CAttrReader& attr) const
{
	assert(index < m_numAttrs);

	FlatAddr nextAddr = m_addrFirstAttr;

	for (i32 a = 0; a <= index; a++)
	{
		attr.InitFromCompact(nextAddr, m_Reader.GetDataTypeSetsTable().GetHeaderAttr(m_attrsSetId, a));
		nextAddr = attr.GetAddrNextAttr();
	}
}

//////////////////////////////////////////////////////////////////////////

CAttrReader CNodeLiveReader::ObtainAttr(tukk pAttrName) const
{
	CAttrReader attr(m_Reader);

	bool found = FindAttr(pAttrName, attr);
	assert(found);

	return attr;
}

//////////////////////////////////////////////////////////////////////////

CAttrReader CNodeLiveReader::ObtainAttr(uint index) const
{
	CAttrReader attr(m_Reader);
	GetAttr(index, attr);

	return attr;
}

//////////////////////////////////////////////////////////////////////////

u16 CNodeLiveReader::GetHeaderAttr(u32 attrInd) const
{
	return m_Reader.GetDataTypeSetsTable().GetHeaderAttr(m_attrsSetId, attrInd);
}

//////////////////////////////////////////////////////////////////////////
// reads any attr value, converts to string and store it into an string
// is separated from the normal ReadAttr functions in purpose: This function should be used only for error handling or other special situations

bool CNodeLiveReader::ReadAttrAsString(tukk pAttrName, string& str) const
{
	CAttrReader attr(m_Reader);

	bool found = FindAttr(pAttrName, attr);

	if (found)
		attr.GetValueAsString(str);

	return found;
}

//////////////////////////////////////////////////////////////////////////
// for debugging use only
#ifndef _RELEASE
void CNodeLiveReader::Log() const
{
	// read tag id again, is not stored
	FlatAddr myAddr = m_Reader.GetAddrNode(m_globalId);
	u16 header;
	m_Reader.ReadFromBuffer(myAddr, header);
	StringID tagId = header & MASK_TAGID;

	string str;
	str.Format("--- globalId: %u   tag: (%u) %s   size: %u  children: %u  numAttrs: %u ", m_globalId, tagId, m_pTag, GetAddrNextNode() - myAddr, m_numChildren, m_numAttrs);

	if (m_numAttrs > 0)
	{
		str += "ATTRS: ";

		CAttrReader attr(m_Reader);
		FlatAddr nextAddr = m_addrFirstAttr;

		for (u32 a = 0; a < m_numAttrs; a++)
		{
			attr.InitFromCompact(nextAddr, GetHeaderAttr(a));
			nextAddr = attr.GetAddrNextAttr();
			string strAttr;
			string strAttrVal;
			attr.GetValueAsString(strAttrVal);
			strAttr.Format("<(%u) '%s'='%s'> ", attr.GetNameId(), attr.GetName(), strAttrVal.c_str());
			str += strAttr;
		}
	}
	DrxLog("%s", str.c_str());
}
#endif
