// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once
#ifndef XMLCPB_NODELIVEREADER_H
	#define XMLCPB_NODELIVEREADER_H

	#include "XMLCPB_AttrReader.h"
	#include <drx3D/Act/XMLCPB_Common.h>

namespace XMLCPB {

class CNodeLiveReader;
class CReader;

// from outside, CNodeLiveReaderRef is the class that is always used to access a node. CNodeLiveReader is never used directly from outside.
// The main reason for this is because the live nodes are reusable: It could be possible to keep a "valid" pointer to a livenode even after the original node has been discarded and another one has taken its place.
// That situation is avoided with the use of this wrapper reference and a refcounter. LiveNodes are reused only when refcounter is 0.
// This is slightly different than CNodeLiveWriterRef, read there for more info.

class CNodeLiveReaderRef
{
public:

	CNodeLiveReaderRef(CReader& Reader)
		: m_Reader(Reader)
		, m_nodeId(XMLCPB_INVALID_ID)
		, m_pNode_Debug(NULL)
	{}

	explicit CNodeLiveReaderRef(CReader& Reader, NodeLiveID nodeId);

	~CNodeLiveReaderRef()
	{
		FreeRef();
	}

	CNodeLiveReaderRef(const CNodeLiveReaderRef& other)
		: m_Reader(other.m_Reader)
		, m_nodeId(XMLCPB_INVALID_ID)
	{
		CopyFrom(other);
	}

	CNodeLiveReaderRef& operator=(const CNodeLiveReaderRef& other)
	{
		FreeRef();
		CopyFrom(other);
		return *this;
	}

	CNodeLiveReader* operator->() const { return GetNode(); }

	bool             IsValid() const    { return m_nodeId != XMLCPB_INVALID_ID; }

private:

	void             CopyFrom(const CNodeLiveReaderRef& other);
	void             FreeRef();
	CNodeLiveReader* GetNode() const;

private:
	CReader&               m_Reader;
	NodeLiveID             m_nodeId;
	const CNodeLiveReader* m_pNode_Debug;   // is only to help debugging. Dont use it for anything.
};

// When a node is needed, an available CNodeLiveReader object is constructed (reused actually) from the raw binary data that is in the main buffer.
// the live node keeps the data of that raw node until all references are gone. Then it become available again to be used to read other nodes in the raw data.

class CNodeLiveReader
{
	friend class CReader;
	friend class CNodeLiveReaderRef;

public:

	CNodeLiveReader(CReader& Reader) : m_Reader(Reader) { Reset(); }
	CNodeLiveReader(const CNodeLiveReader& other) : m_Reader(other.m_Reader) { *this = other; }
	~CNodeLiveReader();

	CNodeLiveReader& operator=(const CNodeLiveReader& other)
	{
		if (this != &other)
		{
			m_liveId = other.m_liveId;
			m_globalId = other.m_globalId;
			m_attrsSetId = other.m_attrsSetId;
			m_refCounter = other.m_refCounter;
			m_indexOfNextChild = other.m_indexOfNextChild;
			m_numChildren = other.m_numChildren;
			m_numAttrs = other.m_numAttrs;
			m_addrFirstAttr = other.m_addrFirstAttr;
			m_pTag = other.m_pTag;
			m_valid = other.m_valid;
			m_childBlocks.resize(other.m_childBlocks.size());
			m_childBlocks = other.m_childBlocks;
		}
		return *this;
	}

	CNodeLiveReaderRef GetChildNode(tukk pChildName);
	CNodeLiveReaderRef GetChildNode(u32 index);
	u32             GetNumChildren() const { return m_numChildren; }
	u32             GetNumAttrs() const    { return m_numAttrs; }
	tukk        GetTag() const         { return m_pTag; }
	u32             GetSizeWithoutChilds() const;
	template<class T>
	bool               ReadAttr(tukk pAttrName, T& data) const;
	bool               ReadAttr(tukk pAttrName, u8*& rdata, u32& outSize) const;
	template<class T>
	void               ReadAttr(uint index, T& data) const;
	void               ReadAttr(uint index, u8*& rdata, u32& outSize) const;
	bool               ReadAttrAsString(tukk pAttrName, string& str) const;

	bool               HaveAttr(tukk pAttrName) const;
	FlatAddr           GetAddrNextNode() const;

	// use ReadAttr functions whenever possible, instead of Obtain() ones.
	CAttrReader ObtainAttr(tukk pAttrName) const;
	CAttrReader ObtainAttr(u32 indAttr) const;

private:

	bool         IsValid() const { return m_valid; }
	void         Reset();
	void         ActivateFromCompact(NodeLiveID liveId, NodeGlobalID globalId);
	NodeLiveID   GetLiveId() const { return m_liveId; }
	bool         FindAttr(tukk pAttrName, CAttrReader& attr) const;
	void         GetAttr(u32 indAttr, CAttrReader& attr) const;

	tukk  GetTagNode(NodeGlobalID nodeId) const;
	NodeGlobalID GetChildGlobalId(u32 indChild) const;
	u16       GetHeaderAttr(u32 attrInd) const;

	#ifndef _RELEASE
	void Log() const;
	#endif

private:

	CReader&                  m_Reader;
	NodeLiveID                m_liveId;   // is unique for every live node, but it does not change, which means that the same live node will have always the same ID even when it stores the info of diferent raw nodes.
	NodeGlobalID              m_globalId; // unique for every node in the file.
	AttrSetID                 m_attrsSetId;
	u32                    m_refCounter;       // to know when it can be discarded and reused.
	u32                    m_indexOfNextChild; // to speed up the sequential reading of childs.
	u32                    m_numChildren;
	u32                    m_numAttrs;
	FlatAddr                  m_addrFirstAttr;  // all attrs are sequentially stored
	std::vector<NodeGlobalID> m_childBlocks;    // stores the globalId of the first child of each block

	tukk               m_pTag;
	bool                      m_valid;  // live nodes are reused. this is false when the node is invalid, which means is ready to be reused.
};

template<class T>
bool CNodeLiveReader::ReadAttr(tukk pAttrName, T& data) const
{
	CAttrReader attr(m_Reader);
	bool found = FindAttr(pAttrName, attr);

	if (found)
		attr.Get(data);

	return found;
}

template<class T>
void CNodeLiveReader::ReadAttr(u32 index, T& data) const
{
	assert(index < m_numAttrs);
	CAttrReader attr(m_Reader);
	FlatAddr nextAddr = m_addrFirstAttr;

	for (u32 a = 0; a < index; a++)
	{
		attr.InitFromCompact(nextAddr, GetHeaderAttr(a));
		nextAddr = attr.GetAddrNextAttr();
	}
	attr.Get(data);
}

} //end namespace

#endif
