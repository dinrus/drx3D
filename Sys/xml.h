// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   xml.h
//  Created:     21/04/2006 by Timur.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __XML_HEADER__
#define __XML_HEADER__

#include <algorithm>
#include <drx3D/CoreX/Memory/PoolAllocator.h>

#include <drx3D/Sys/IXml.h>

#include <drx3D/CoreX/Platform/IPlatformOS.h>

// track some XML stats. only to find persistent XML nodes in the system
// slow, so disable by default
//#define DRX_COLLECT_XML_NODE_STATS
//#undef DRX_COLLECT_XML_NODE_STATS

struct IXmlStringPool
{
public:
	IXmlStringPool() { m_refCount = 0; }
	virtual ~IXmlStringPool() {};
	void AddRef() { m_refCount++; };
	void Release()
	{
		if (--m_refCount <= 0)
			delete this;
	};
	virtual tukk AddString(tukk str) = 0;
	virtual void        GetMemoryUsage(IDrxSizer* pSizer) const = 0;
private:
	i32                 m_refCount;
};

/************************************************************************/
/* XmlParser class, Parse xml and return root xml node if success.      */
/************************************************************************/
class XmlParser : public IXmlParser
{
public:
	explicit XmlParser(bool bReuseStrings);
	~XmlParser();

	void AddRef()
	{
		++m_nRefCount;
	}

	void Release()
	{
		if (--m_nRefCount <= 0)
			delete this;
	}

	virtual XmlNodeRef ParseFile(tukk filename, bool bCleanPools);

	virtual XmlNodeRef ParseBuffer(tukk buffer, i32 nBufLen, bool bCleanPools);

	tukk        getErrorString() const { return m_errorString; }

	void               GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	i32                 m_nRefCount;
	XmlString           m_errorString;
	class XmlParserImp* m_pImpl;
};

// Compare function for string comparasion, can be strcmp or stricmp
typedef i32 (__cdecl * XmlStrCmpFunc)(tukk str1, tukk str2);
extern XmlStrCmpFunc g_pXmlStrCmp;

//////////////////////////////////////////////////////////////////////////
// XmlAttribute class
//////////////////////////////////////////////////////////////////////////
struct XmlAttribute
{
	tukk key;
	tukk value;

	XmlAttribute()
		: key(nullptr)
		, value(nullptr)
	{}

	void GetMemoryUsage(IDrxSizer* pSizer) const    {}

	bool operator<(const XmlAttribute& attr) const  { return g_pXmlStrCmp(key, attr.key) < 0; }
	bool operator>(const XmlAttribute& attr) const  { return g_pXmlStrCmp(key, attr.key) > 0; }
	bool operator==(const XmlAttribute& attr) const { return g_pXmlStrCmp(key, attr.key) == 0; }
	bool operator!=(const XmlAttribute& attr) const { return g_pXmlStrCmp(key, attr.key) != 0; }
};

//! Xml node attributes class.

typedef std::vector<XmlAttribute>     XmlAttributes;
typedef XmlAttributes::iterator       XmlAttrIter;
typedef XmlAttributes::const_iterator XmlAttrConstIter;

/**
 ******************************************************************************
 * CXmlNode class
 * Never use CXmlNode directly instead use reference counted XmlNodeRef.
 ******************************************************************************
 */

class CXmlNode : public IXmlNode
{
public:
	//! Constructor.
	CXmlNode();
	CXmlNode(tukk tag, bool bReuseStrings);
	//! Destructor.
	~CXmlNode();

	// collect allocated memory  informations
	void GetMemoryUsage(IDrxSizer* pSizer) const;

	//////////////////////////////////////////////////////////////////////////
	// Custom new/delete with pool allocator.
	//////////////////////////////////////////////////////////////////////////
	//uk operator new( size_t nSize );
	//void operator delete( uk ptr );

	virtual void DeleteThis();

	//! Create new XML node.
	XmlNodeRef createNode(tukk tag);

	//! Get XML node tag.
	tukk getTag() const { return m_tag; };
	void        setTag(tukk tag);

	//! Return true if given tag equal to node tag.
	bool isTag(tukk tag) const;

	//! Get XML Node attributes.
	virtual i32  getNumAttributes() const { return m_pAttributes ? (i32)m_pAttributes->size() : 0; };
	//! Return attribute key and value by attribute index.
	virtual bool getAttributeByIndex(i32 index, tukk* key, tukk* value);

	//! Return attribute key and value by attribute index, string version.
	virtual bool getAttributeByIndex(i32 index, XmlString& key, XmlString& value);

	virtual void copyAttributes(XmlNodeRef fromNode);
	virtual void shareChildren(const XmlNodeRef& fromNode);

	//! Get XML Node attribute for specified key.
	tukk getAttr(tukk key) const;

	//! Get XML Node attribute for specified key.
	// Returns true if the attribute existes, alse otherwise.
	bool getAttr(tukk key, tukk* value) const;

	//! Check if attributes with specified key exist.
	bool haveAttr(tukk key) const;

	//! Creates new xml node and add it to childs list.
	XmlNodeRef newChild(tukk tagName);

	//! Adds new child node.
	void addChild(const XmlNodeRef& node);
	//! Remove child node.
	void removeChild(const XmlNodeRef& node);

	void insertChild(i32 nIndex, const XmlNodeRef& node);
	void replaceChild(i32 nIndex, const XmlNodeRef& node);

	//! Remove all child nodes.
	void removeAllChilds();

	//! Get number of child XML nodes.
	i32 getChildCount() const { return m_pChilds ? (i32)m_pChilds->size() : 0; };

	//! Get XML Node child nodes.
	XmlNodeRef getChild(i32 i) const;

	//! Find node with specified tag.
	XmlNodeRef findChild(tukk tag) const;
	void       deleteChild(tukk tag);
	void       deleteChildAt(i32 nIndex);

	//! Get parent XML node.
	XmlNodeRef getParent() const { return m_parent; }
	void       setParent(const XmlNodeRef& inRef);

	//! Returns content of this node.
	tukk getContent() const { return m_content; };
	void        setContent(tukk str);

	XmlNodeRef  clone();

	//! Returns line number for XML tag.
	i32  getLine() const   { return m_line; };
	//! Set line number in xml.
	void setLine(i32 line) { m_line = line; };

	//! Returns XML of this node and sub nodes.
	virtual IXmlStringData* getXMLData(i32 nReserveMem = 0) const;
	XmlString               getXML(i32 level = 0) const;
	XmlString               getXMLUnsafe(i32 level, tuk tmpBuffer, u32 sizeOfTmpBuffer) const;
	bool                    saveToFile(tukk fileName);                                           // saves in one huge chunk
	bool                    saveToFile(tukk fileName, size_t chunkSizeBytes, FILE* file = NULL); // save in small memory chunks

	//! Set new XML Node attribute (or override attribute with same key).
	void setAttr(tukk key, tukk value);
	void setAttr(tukk key, i32 value);
	void setAttr(tukk key, u32 value);
	void setAttr(tukk key, int64 value);
	void setAttr(tukk key, uint64 value, bool useHexFormat = true);
	void setAttr(tukk key, float value);
	void setAttr(tukk key, double value);
	void setAttr(tukk key, const Vec2& value);
	void setAttr(tukk key, const Vec2d& value);
	void setAttr(tukk key, const Ang3& value);
	void setAttr(tukk key, const Vec3& value);
	void setAttr(tukk key, const Vec4& value);
	void setAttr(tukk key, const Vec3d& value);
	void setAttr(tukk key, const Quat& value);
	void setAttr(tukk key, const DrxGUID& value);

	//! Delete attrbute.
	void delAttr(tukk key);
	//! Remove all node attributes.
	void removeAllAttributes();

	//! Get attribute value of node.
	bool getAttr(tukk key, i32& value) const;
	bool getAttr(tukk key, u32& value) const;
	bool getAttr(tukk key, int64& value) const;
	bool getAttr(tukk key, uint64& value, bool useHexFormat = true /*ignored*/) const;
	bool getAttr(tukk key, float& value) const;
	bool getAttr(tukk key, double& value) const;
	bool getAttr(tukk key, bool& value) const;

	bool getAttr(tukk key, XmlString& value) const { tukk v(NULL); bool boHasAttribute(getAttr(key, &v)); value = v; return boHasAttribute; }

	bool getAttr(tukk key, Vec2& value) const;
	bool getAttr(tukk key, Vec2d& value) const;
	bool getAttr(tukk key, Ang3& value) const;
	bool getAttr(tukk key, Vec3& value) const;
	bool getAttr(tukk key, Vec4& value) const;
	bool getAttr(tukk key, Vec3d& value) const;
	bool getAttr(tukk key, Quat& value) const;
	bool getAttr(tukk key, ColorB& value) const;
	bool getAttr(tukk key, DrxGUID& value) const;

protected:

private:
	CXmlNode(const CXmlNode&);
	CXmlNode& operator=(const CXmlNode&);

private:
	void             ReleaseChild(IXmlNode* pChild);
	void             removeAllChildsImpl();

	void             AddToXmlString(XmlString& xml, i32 level, FILE* pFile = 0, IPlatformOS::ISaveWriterPtr pSaveWriter = IPlatformOS::ISaveWriterPtr(), size_t chunkSizeBytes = 0) const;
	tuk            AddToXmlStringUnsafe(tuk xml, i32 level, tuk endPtr, FILE* pFile = 0, IPlatformOS::ISaveWriterPtr pSaveWriter = IPlatformOS::ISaveWriterPtr(), size_t chunkSizeBytes = 0) const;
	XmlString        MakeValidXmlString(const XmlString& xml) const;
	bool             IsValidXmlString(tukk str) const;
	XmlAttrConstIter GetAttrConstIterator(tukk key) const
	{
		assert(m_pAttributes);

		XmlAttribute tempAttr;
		tempAttr.key = key;

		XmlAttributes::const_iterator it = std::find(m_pAttributes->begin(), m_pAttributes->end(), tempAttr);
		return it;

		/*
		   XmlAttributes::const_iterator it = std::lower_bound( m_attributes.begin(),m_attributes.end(),tempAttr );
		   if (it != m_attributes.end() && _stricmp(it->key,key) == 0)
		   return it;
		   return m_attributes.end();
		 */
	}
	XmlAttrIter GetAttrIterator(tukk key)
	{
		assert(m_pAttributes);

		XmlAttribute tempAttr;
		tempAttr.key = key;

		XmlAttributes::iterator it = std::find(m_pAttributes->begin(), m_pAttributes->end(), tempAttr);
		return it;

		//		XmlAttributes::iterator it = std::lower_bound( m_attributes.begin(),m_attributes.end(),tempAttr );
		//if (it != m_attributes.end() && _stricmp(it->key,key) == 0)
		//return it;
		//return m_attributes.end();
	}
	tukk GetValue(tukk key) const
	{
		if (m_pAttributes)
		{
			XmlAttrConstIter it = GetAttrConstIterator(key);
			if (it != m_pAttributes->end())
				return it->value;
		}
		return 0;
	}

protected:
	// String pool used by this node.
	IXmlStringPool* m_pStringPool;

	//! Tag of XML node.
	tukk m_tag;

private:

	//! Content of XML node.
	tukk m_content;
	//! Parent XML node.
	IXmlNode*   m_parent;

	//typedef DynArray<CXmlNode*,XmlDynArrayAlloc>	XmlNodes;
	typedef std::vector<IXmlNode*> XmlNodes;
	//XmlNodes m_childs;
	XmlNodes* m_pChilds;

	//! Xml node attributes.
	//XmlAttributes m_attributes;
	XmlAttributes* m_pAttributes;

	//! Line in XML file where this node firstly appeared (useful for debugging).
	i32 m_line;

	friend class XmlParserImp;
};

typedef stl::PoolAllocatorNoMT<sizeof(CXmlNode)> CXmlNode_PoolAlloc;
extern CXmlNode_PoolAlloc* g_pCXmlNode_PoolAlloc;

#ifdef DRX_COLLECT_XML_NODE_STATS
typedef std::set<CXmlNode*> TXmlNodeSet; // yes, slow, but really only for one-shot debugging
struct SXmlNodeStats
{
	SXmlNodeStats() : nAllocs(0), nFrees(0) {}
	TXmlNodeSet nodeSet;
	u32      nAllocs;
	u32      nFrees;
};
extern SXmlNodeStats* g_pCXmlNode_Stats;
#endif

/*
   //////////////////////////////////////////////////////////////////////////
   inline uk CXmlNode::operator new( size_t nSize )
   {
   uk ptr = g_pCXmlNode_PoolAlloc->Allocate();
   if (ptr)
   {
    memset( ptr,0,nSize ); // Clear objects memory.
   #ifdef DRX_COLLECT_XML_NODE_STATS
    g_pCXmlNode_Stats->nodeSet.insert(reinterpret_cast<CXmlNode*> (ptr));
   ++g_pCXmlNode_Stats->nAllocs;
   #endif
   }
   return ptr;
   }

   //////////////////////////////////////////////////////////////////////////
   inline void CXmlNode::operator delete( uk ptr )
   {
   if (ptr)
   {
    g_pCXmlNode_PoolAlloc->Deallocate(ptr);
   #ifdef DRX_COLLECT_XML_NODE_STATS
    g_pCXmlNode_Stats->nodeSet.erase(reinterpret_cast<CXmlNode*> (ptr));
   ++g_pCXmlNode_Stats->nFrees;
   #endif
   }
   }
 */

//////////////////////////////////////////////////////////////////////////
//
// Reusable XmlNode for XmlNode pool with shared xml string pool
//
//////////////////////////////////////////////////////////////////////////
class CXmlNodePool;

class CXmlNodeReuse : public CXmlNode
{
public:
	CXmlNodeReuse(tukk tag, CXmlNodePool* pPool);
	virtual void Release();

protected:
	CXmlNodePool* m_pPool;
};

//////////////////////////////////////////////////////////////////////////
//
// Pool of reusable XML nodes with shared string pool
//
//////////////////////////////////////////////////////////////////////////
class CXmlNodePool
{
public:
	CXmlNodePool(u32 nBlockSize, bool bReuseStrings);
	virtual ~CXmlNodePool();

	XmlNodeRef GetXmlNode(tukk sNodeName);
	bool       empty() const { return (m_nAllocated == 0); }

protected:
	virtual void    OnRelease(i32 iRefCount, uk pThis);
	IXmlStringPool* GetStringPool() { return m_pStringPool; }

private:
	friend class CXmlNodeReuse;

	IXmlStringPool*            m_pStringPool;
	u32               m_nAllocated;
	std::stack<CXmlNodeReuse*> m_pNodePool;
};

#endif // __XML_HEADER__
