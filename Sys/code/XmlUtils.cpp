// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   XmlUtils.h
//  Created:     21/04/2006 by Timur.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/Plugins/md5/md5.h>
#include <drx3D/Sys/xml.h>
#include <drx3D/Sys/XmlUtils.h>
#include <drx3D/Sys/ReadWriteXMLSink.h>

#include <drx3D/Sys/SimpleStringPool.h>
#include <drx3D/Sys/SerializeXMLReader.h>
#include <drx3D/Sys/SerializeXMLWriter.h>

#include <drx3D/Sys/XMLBinaryWriter.h>
#include <drx3D/Sys/XMLBinaryReader.h>

#include <drx3D/Sys/XMLPatcher.h>

//////////////////////////////////////////////////////////////////////////
CXmlNode_PoolAlloc* g_pCXmlNode_PoolAlloc = 0;
#ifdef DRX_COLLECT_XML_NODE_STATS
SXmlNodeStats* g_pCXmlNode_Stats = 0;
#endif

extern bool g_bEnableBinaryXmlLoading;

//////////////////////////////////////////////////////////////////////////
CXmlUtils::CXmlUtils(ISystem* pSystem)
{
	m_pSystem = pSystem;
	m_pSystem->GetISystemEventDispatcher()->RegisterListener(this,"CXmlUtils");

	// create IReadWriteXMLSink object
	m_pReadWriteXMLSink = new CReadWriteXMLSink();
	g_pCXmlNode_PoolAlloc = new CXmlNode_PoolAlloc;
#ifdef DRX_COLLECT_XML_NODE_STATS
	g_pCXmlNode_Stats = new SXmlNodeStats();
#endif
	m_pStatsXmlNodePool = 0;
#ifndef _RELEASE
	m_statsThreadOwner = DrxGetCurrentThreadId();
#endif
	m_pXMLPatcher = NULL;
}

//////////////////////////////////////////////////////////////////////////
CXmlUtils::~CXmlUtils()
{
	m_pSystem->GetISystemEventDispatcher()->RemoveListener(this);
	delete g_pCXmlNode_PoolAlloc;
#ifdef DRX_COLLECT_XML_NODE_STATS
	delete g_pCXmlNode_Stats;
#endif
	SAFE_DELETE(m_pStatsXmlNodePool);
	SAFE_DELETE(m_pXMLPatcher);
}

//////////////////////////////////////////////////////////////////////////
IXmlParser* CXmlUtils::CreateXmlParser()
{
	const bool bReuseStrings = false; //TODO: do we ever want to reuse strings here?
	return new XmlParser(bReuseStrings);
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CXmlUtils::LoadXmlFromFile(tukk sFilename, bool bReuseStrings, bool bEnablePatching)
{
	XmlParser parser(bReuseStrings);
	XmlNodeRef node = parser.ParseFile(sFilename, true);

	// XmlParser is supposed to log warnings and errors (if any),
	// so we don't need to call parser.getErrorString(),
	// DrxLog() etc here.

	if (node && bEnablePatching && m_pXMLPatcher)
	{
		node = m_pXMLPatcher->ApplyXMLDataPatch(node, sFilename);
	}

	return node;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CXmlUtils::LoadXmlFromBuffer(tukk buffer, size_t size, bool bReuseStrings)
{
	XmlParser parser(bReuseStrings);
	XmlNodeRef node = parser.ParseBuffer(buffer, size, true);
	return node;
}

//////////////////////////////////////////////////////////////////////////
tukk CXmlUtils::HashXml(XmlNodeRef node)
{
	static char signature[16 * 2 + 1];
	static char temp[16];
	static tukk hex = "0123456789abcdef";
	XmlString str = node->getXML();

	SMD5Context context;
	MD5Init(&context);
	MD5Update(&context, (u8*)str.data(), str.length());
	MD5Final((u8*)temp, &context);

	for (i32 i = 0; i < 16; i++)
	{
		signature[2 * i + 0] = hex[((u8)temp[i]) >> 4];
		signature[2 * i + 1] = hex[((u8)temp[i]) & 0xf];
	}
	signature[16 * 2] = 0;
	return signature;
}

//////////////////////////////////////////////////////////////////////////
IReadWriteXMLSink* CXmlUtils::GetIReadWriteXMLSink()
{
	return m_pReadWriteXMLSink;
}

//////////////////////////////////////////////////////////////////////////
class CXmlSerializer : public IXmlSerializer
{
public:
	CXmlSerializer()
		: m_nRefCount(0)
		, m_pReaderImpl(NULL)
		, m_pReaderSer(NULL)
		, m_pWriterSer(NULL)
		, m_pWriterImpl(NULL)
	{
	}
	~CXmlSerializer()
	{
		ClearAll();
	}
	void ClearAll()
	{
		SAFE_DELETE(m_pReaderSer);
		SAFE_DELETE(m_pReaderImpl);
		SAFE_DELETE(m_pWriterSer);
		SAFE_DELETE(m_pWriterImpl);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void        AddRef()  { ++m_nRefCount; }
	virtual void        Release() { if (--m_nRefCount <= 0) delete this; }

	virtual ISerialize* GetWriter(XmlNodeRef& node)
	{
		ClearAll();
		m_pWriterImpl = new CSerializeXMLWriterImpl(node);
		m_pWriterSer = new CSimpleSerializeWithDefaults<CSerializeXMLWriterImpl>(*m_pWriterImpl);
		return m_pWriterSer;
	}
	virtual ISerialize* GetReader(XmlNodeRef& node)
	{
		ClearAll();
		m_pReaderImpl = new CSerializeXMLReaderImpl(node);
		m_pReaderSer = new CSimpleSerializeWithDefaults<CSerializeXMLReaderImpl>(*m_pReaderImpl);
		return m_pReaderSer;
	}

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->Add(*this);
		pSizer->AddObject(m_pReaderImpl);
		pSizer->AddObject(m_pWriterImpl);
	}
	//////////////////////////////////////////////////////////////////////////
private:
	i32                      m_nRefCount;
	CSerializeXMLReaderImpl* m_pReaderImpl;
	CSimpleSerializeWithDefaults<CSerializeXMLReaderImpl>* m_pReaderSer;

	CSerializeXMLWriterImpl*                               m_pWriterImpl;
	CSimpleSerializeWithDefaults<CSerializeXMLWriterImpl>* m_pWriterSer;
};

//////////////////////////////////////////////////////////////////////////
IXmlSerializer* CXmlUtils::CreateXmlSerializer()
{
	return new CXmlSerializer;
}

//////////////////////////////////////////////////////////////////////////
void CXmlUtils::GetMemoryUsage(IDrxSizer* pSizer)
{
	{
		SIZER_COMPONENT_NAME(pSizer, "Nodes");
		g_pCXmlNode_PoolAlloc->GetMemoryUsage(pSizer);
	}

#ifdef DRX_COLLECT_XML_NODE_STATS
	// yes, slow
	std::vector<const CXmlNode*> rootNodes;
	{
		TXmlNodeSet::const_iterator iter = g_pCXmlNode_Stats->nodeSet.begin();
		TXmlNodeSet::const_iterator iterEnd = g_pCXmlNode_Stats->nodeSet.end();
		while (iter != iterEnd)
		{
			const CXmlNode* pNode = *iter;
			if (pNode->getParent() == 0)
			{
				rootNodes.push_back(pNode);
			}
			++iter;
		}
	}

	// use the following to log to console
	#if 0
	DrxLogAlways("NumXMLRootNodes=%d NumXMLNodes=%d TotalAllocs=%d TotalFrees=%d",
	             rootNodes.size(), g_pCXmlNode_Stats->nodeSet.size(),
	             g_pCXmlNode_Stats->nAllocs, g_pCXmlNode_Stats->nFrees);
	#endif

	// use the following to debug the nodes in the system
	#if 0
	{
		std::vector<const CXmlNode*>::const_iterator iter = rootNodes.begin();
		std::vector<const CXmlNode*>::const_iterator iterEnd = rootNodes.end();
		while (iter != iterEnd)
		{
			const CXmlNode* pNode = *iter;
			DrxLogAlways("Node 0x%p Tag='%s'", pNode, pNode->getTag());
			++iter;
		}
	}
	#endif

	// only for debugging, add it as pseudo numbers to the DrxSizer.
	// shift it by 10, so we get the actual number
	{
		SIZER_COMPONENT_NAME(pSizer, "#NumTotalNodes");
		pSizer->Add("#NumTotalNodes", g_pCXmlNode_Stats->nodeSet.size() << 10);
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "#NumRootNodes");
		pSizer->Add("#NumRootNodes", rootNodes.size() << 10);
	}
#endif

}

//////////////////////////////////////////////////////////////////////////
void CXmlUtils::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
	case ESYSTEM_EVENT_LEVEL_LOAD_END:
		g_pCXmlNode_PoolAlloc->FreeMemoryIfEmpty();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
class CXmlBinaryDataWriterFile : public XMLBinary::IDataWriter
{
public:
	CXmlBinaryDataWriterFile(tukk file) { m_file = gEnv->pDrxPak->FOpen(file, "wb"); }
	~CXmlBinaryDataWriterFile() { if (m_file) gEnv->pDrxPak->FClose(m_file); };
	virtual bool IsOk()                                { return m_file != 0; };
	virtual void Write(ukk pData, size_t size) { if (m_file) gEnv->pDrxPak->FWrite(pData, size, 1, m_file); }
private:
	FILE* m_file;
};

//////////////////////////////////////////////////////////////////////////
bool CXmlUtils::SaveBinaryXmlFile(tukk filename, XmlNodeRef root)
{
	CXmlBinaryDataWriterFile fileSink(filename);
	if (!fileSink.IsOk())
		return false;
	XMLBinary::CXMLBinaryWriter writer;
	string error;
	return writer.WriteNode(&fileSink, root, false, 0, error);
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CXmlUtils::LoadBinaryXmlFile(tukk filename, bool bEnablePatching)
{
	XMLBinary::XMLBinaryReader reader;
	XMLBinary::XMLBinaryReader::EResult result;
	XmlNodeRef root = reader.LoadFromFile(filename, result);

	if (result == XMLBinary::XMLBinaryReader::eResult_Success && bEnablePatching == true && m_pXMLPatcher != NULL)
	{
		root = m_pXMLPatcher->ApplyXMLDataPatch(root, filename);
	}

	return root;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlUtils::EnableBinaryXmlLoading(bool bEnable)
{
	bool bPrev = g_bEnableBinaryXmlLoading;
	g_bEnableBinaryXmlLoading = bEnable;
	return bPrev;
}

//////////////////////////////////////////////////////////////////////////
class CXmlTableReader : public IXmlTableReader
{
public:
	CXmlTableReader();
	virtual ~CXmlTableReader();

	virtual void Release();

	virtual bool Begin(XmlNodeRef rootNode);
	virtual i32  GetEstimatedRowCount();
	virtual bool ReadRow(i32& rowIndex);
	virtual bool ReadCell(i32& columnIndex, tukk & pContent, size_t& contentSize);

private:
	bool       m_bExcel;

	XmlNodeRef m_tableNode;

	XmlNodeRef m_rowNode;

	i32        m_rowNodeIndex;
	i32        m_row;

	i32        m_columnNodeIndex; // used if m_bExcel == true
	i32        m_column;

	size_t     m_rowTextSize; // used if m_bExcel == false
	size_t     m_rowTextPos;  // used if m_bExcel == false
};

//////////////////////////////////////////////////////////////////////////
CXmlTableReader::CXmlTableReader()
	: m_bExcel(false)
	, m_rowNodeIndex(0)
	, m_row(0)
	, m_columnNodeIndex(0)
	, m_column(0)
	, m_rowTextSize(0)
	, m_rowTextPos(0)
{
}

//////////////////////////////////////////////////////////////////////////
CXmlTableReader::~CXmlTableReader()
{
}

//////////////////////////////////////////////////////////////////////////
void CXmlTableReader::Release()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlTableReader::Begin(XmlNodeRef rootNode)
{
	m_tableNode = 0;

	if (!rootNode)
	{
		return false;
	}

	XmlNodeRef worksheetNode = rootNode->findChild("Worksheet");
	if (worksheetNode)
	{
		m_bExcel = true;
		m_tableNode = worksheetNode->findChild("Table");
	}
	else
	{
		m_bExcel = false;
		m_tableNode = rootNode->findChild("Table");
	}

	m_rowNode = 0;
	m_rowNodeIndex = -1;
	m_row = -1;

	return (m_tableNode != 0);
}

//////////////////////////////////////////////////////////////////////////
i32 CXmlTableReader::GetEstimatedRowCount()
{
	if (!m_tableNode)
	{
		return -1;
	}
	return m_tableNode->getChildCount();
}

//////////////////////////////////////////////////////////////////////////
bool CXmlTableReader::ReadRow(i32& rowIndex)
{
	if (!m_tableNode)
	{
		return false;
	}

	m_columnNodeIndex = -1;
	m_column = -1;

	i32k rowNodeCount = m_tableNode->getChildCount();

	if (m_bExcel)
	{
		for (;; )
		{
			if (++m_rowNodeIndex >= rowNodeCount)
			{
				m_rowNodeIndex = rowNodeCount;
				return false;
			}

			m_rowNode = m_tableNode->getChild(m_rowNodeIndex);
			if (!m_rowNode)
			{
				m_rowNodeIndex = rowNodeCount;
				return false;
			}

			if (!m_rowNode->isTag("Row"))
			{
				m_rowNode = 0;
				continue;
			}

			++m_row;

			i32 index = 0;
			if (m_rowNode->getAttr("ss:Index", index))
			{
				--index;  // one-based -> zero-based
				if (index < m_row)
				{
					m_rowNodeIndex = rowNodeCount;
					m_rowNode = 0;
					return false;
				}
				m_row = index;
			}

			rowIndex = m_row;
			return true;
		}
	}

	{
		m_rowTextSize = 0;
		m_rowTextPos = 0;

		if (++m_rowNodeIndex >= rowNodeCount)
		{
			m_rowNodeIndex = rowNodeCount;
			return false;
		}

		m_rowNode = m_tableNode->getChild(m_rowNodeIndex);
		if (!m_rowNode)
		{
			m_rowNodeIndex = rowNodeCount;
			return false;
		}

		tukk const pContent = m_rowNode->getContent();
		if (pContent)
		{
			m_rowTextSize = strlen(pContent);
		}

		m_row = m_rowNodeIndex;
		rowIndex = m_rowNodeIndex;
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CXmlTableReader::ReadCell(i32& columnIndex, tukk & pContent, size_t& contentSize)
{
	pContent = 0;
	contentSize = 0;

	if (!m_tableNode)
	{
		return false;
	}

	if (!m_rowNode)
	{
		return false;
	}

	if (m_bExcel)
	{
		i32k columnNodeCount = m_rowNode->getChildCount();

		for (;; )
		{
			if (++m_columnNodeIndex >= columnNodeCount)
			{
				m_columnNodeIndex = columnNodeCount;
				return false;
			}

			XmlNodeRef columnNode = m_rowNode->getChild(m_columnNodeIndex);
			if (!columnNode)
			{
				m_columnNodeIndex = columnNodeCount;
				return false;
			}

			if (!columnNode->isTag("Cell"))
			{
				continue;
			}

			++m_column;

			i32 index = 0;
			if (columnNode->getAttr("ss:Index", index))
			{
				--index;  // one-based -> zero-based
				if (index < m_column)
				{
					m_columnNodeIndex = columnNodeCount;
					return false;
				}
				m_column = index;
			}
			columnIndex = m_column;

			XmlNodeRef dataNode = columnNode->findChild("Data");
			if (dataNode)
			{
				pContent = dataNode->getContent();
				if (pContent)
				{
					contentSize = strlen(pContent);
				}
			}
			return true;
		}
	}

	{
		if (m_rowTextPos >= m_rowTextSize)
		{
			return false;
		}

		tukk const pRowContent = m_rowNode->getContent();
		if (!pRowContent)
		{
			m_rowTextPos = m_rowTextSize;
			return false;
		}
		// cppcheck-suppress autoVariables
		pContent = &pRowContent[m_rowTextPos];

		columnIndex = ++m_column;

		for (;; )
		{
			char c = pRowContent[m_rowTextPos++];

			if ((c == '\n') || (c == '\0'))
			{
				return true;
			}

			if (c == '\r')
			{
				// ignore all '\r' chars
				for (;; )
				{
					c = pRowContent[m_rowTextPos++];
					if ((c == '\n') || (c == '\0'))
					{
						return true;
					}
					if (c != '\r')
					{
						// broken data. '\r' expected to be followed by '\n' or '\0'.
						contentSize = 0;
						m_rowTextPos = m_rowTextSize;
						return false;
					}
				}
			}

			++contentSize;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
IXmlTableReader* CXmlUtils::CreateXmlTableReader()
{
	return new CXmlTableReader;
}

//////////////////////////////////////////////////////////////////////////
// Init xml stats nodes pool
void CXmlUtils::InitStatsXmlNodePool(u32 nPoolSize)
{
	CHECK_STATS_THREAD_OWNERSHIP();
	if (0 == m_pStatsXmlNodePool)
	{
		// create special xml node pools for game statistics

		const bool bReuseStrings = true;  // TODO parameterise?
		m_pStatsXmlNodePool = new CXmlNodePool(nPoolSize, bReuseStrings);
		assert(m_pStatsXmlNodePool);
	}
	else
	{
		DrxLog("[CXmlNodePool]: Xml stats nodes pool already initialized");
	}
}

//////////////////////////////////////////////////////////////////////////
// Creates new xml node for statistics.
XmlNodeRef CXmlUtils::CreateStatsXmlNode(tukk sNodeName)
{
	CHECK_STATS_THREAD_OWNERSHIP();
	if (0 == m_pStatsXmlNodePool)
	{
		DrxLog("[CXmlNodePool]: Xml stats nodes pool isn't initialized. Perform default initialization.");
		InitStatsXmlNodePool();
	}
	return m_pStatsXmlNodePool->GetXmlNode(sNodeName);
}

void CXmlUtils::SetStatsOwnerThread(threadID threadId)
{
#ifndef _RELEASE
	m_statsThreadOwner = threadId;
#endif
}

void CXmlUtils::FlushStatsXmlNodePool()
{
	CHECK_STATS_THREAD_OWNERSHIP();
	if (m_pStatsXmlNodePool)
	{
		if (m_pStatsXmlNodePool->empty())
		{
			SAFE_DELETE(m_pStatsXmlNodePool);
		}
	}
}

void CXmlUtils::SetXMLPatcher(XmlNodeRef* pPatcher)
{
	SAFE_DELETE(m_pXMLPatcher);

	if (pPatcher != NULL)
	{
		m_pXMLPatcher = new CXMLPatcher(*pPatcher);
	}
}
