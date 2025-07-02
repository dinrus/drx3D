// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once
#ifndef XMLCPB_WRITER_H
	#define XMLCPB_WRITER_H

	#include "XMLCPB_StringTableWriter.h"
	#include "XMLCPB_AttrSetTableWriter.h"
	#include "XMLCPB_NodeLiveWriter.h"

namespace XMLCPB {

// CWriter should not be used directly from outside. use CWriterInterface instead.

class CWriterBase
{
public:
	~CWriterBase();
};

class CWriter : public CWriterBase
{
public:

	CWriter();
	~CWriter();

	// used from outside, thru the CWriterInterface
	void               Init(tukk pRootName, tukk pFileName);
	CNodeLiveWriterRef GetRoot();
	void               Done();          // should be called when everything is added and finished.
	bool               FinishWritingFile();
	bool               WriteAllIntoMemory(u8*& rpData, u32& outSize);        // used to write all into a memory buffer (as opposite to a file ).

	// used internally, from other XMLCPB modules
	void                 WriteDataIntoFile(uk pSrc, u32 numBytes);
	void                 WriteDataIntoMemory(u8*& rpData, uk pSrc, u32 numBytes, u32& outWriteLoc);
	CStringTableWriter&  GetAttrNamesTable()        { return m_tableAttrNames; }
	CStringTableWriter&  GetStrDataTable()          { return m_tableStrData; }
	CStringTableWriter&  GetStrDataConstantsTable() { return m_tableStrDataConstants; }
	CStringTableWriter&  GetTagsTable()             { return m_tableTags; }
	CAttrSetTableWriter& GetAttrSetTable()          { return m_tableAttrSets; }
	CBufferWriter&       GetMainBuffer()            { return m_mainBuffer; }
	CNodeLiveWriter*     GetNodeLive(NodeLiveID nodeId);
	void                 FreeNodeLive(NodeLiveID nodeId);
	CNodeLiveWriter*     CreateAndAddLiveNode(tukk pName);
	void                 NotifyRootNodeStartCompact();
	bool                 IsSavingIntoFile() const    { return m_isSavingIntoFile; }
	void                 NotifyNodeCompacted()       { m_counterCompactedNodes++; }
	NodeGlobalID         GetLastUsedGlobalId() const { return m_counterCompactedNodes - 1; }  // globalId is just the order of the node in the big buffer
	void                 NotifyInternalError()       { m_hasInternalError = true; }
	bool                 HasInternalError() const    { return m_hasInternalError; }

	#ifdef XMLCPB_COLLECT_STATS
	void LogStatistics();
	#endif

private:

	void CreateFileHeader(SFileHeader& fileHeader);

private:

	std::vector<_smart_ptr<CNodeLiveWriter>> m_liveNodes;  // nodes that are not done yet. TODO: this maybe could be better as an array of objects instead pointers. could get rid of the reference target too that way
	CBufferWriter                            m_mainBuffer; // where the bulk of binary data is being stored as the nodes get done. it is actually an array of smaller buffers.
	CStringTableWriter                       m_tableTags;
	CStringTableWriter                       m_tableAttrNames;
	CStringTableWriter                       m_tableStrData;
	CStringTableWriter                       m_tableStrDataConstants; // this actually should not be here. it is used only inside CAttrWriter, for optimizations. Is not there because then it would need to be an static member. also, this table is not saved into the file.
	CAttrSetTableWriter                      m_tableAttrSets;
	CBufferWriter::SAddr                     m_rootAddr; // this only has a meaning after the root has been compacted, which only happens at the end of the process
	class CZLibCompressor*                   m_compressor;
	i32              m_firstFreeLiveNode;              // to speed a bit the search for free spaces in the livenodes table.
	u32           m_safecheckIDCounter;             // safecheckID is used in the nodes to avoid using the wrong node.
	u32           m_counterCompactedNodes;
	bool             m_isSavingIntoFile;
	bool             m_hasInternalError;               // usually means that one of the hard coded limits have been surpassed. (more than X attrs, more than X different tags, etc)
	static i32k MAX_NUM_LIVE_NODES = 256;
	static i32k MAIN_BUFFER_SIZE_INCREMENT = 64 * 1024;
};

}  // end namespace

#endif
