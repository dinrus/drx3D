// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   xml.h
//  Created:     21/04/2006 by Timur.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __XML_NODE_HEADER__
#define __XML_NODE_HEADER__

#include <algorithm>
#include <drx3D/Sys/IXml.h>
#include <drx3D/Sys/XMLBinaryHeaders.h>

// Compare function for string comparison, can be strcmp or stricmp
typedef i32 (__cdecl * XmlStrCmpFunc)(tukk str1, tukk str2);
extern XmlStrCmpFunc g_pXmlStrCmp;

class CBinaryXmlNode;

//////////////////////////////////////////////////////////////////////////
class CBinaryXmlData
{
public:
	const XMLBinary::Node*      pNodes;
	const XMLBinary::Attribute* pAttributes;
	const XMLBinary::NodeIndex* pChildIndices;
	tukk                 pStringData;

	tukk                 pFileContents;
	size_t                      nFileSize;
	bool                        bOwnsFileContentsMemory;

	CBinaryXmlNode*             pBinaryNodes;

	i32                         nRefCount;

	CBinaryXmlData();
	~CBinaryXmlData();

	void GetMemoryUsage(IDrxSizer* pSizer) const;
};

// forward declaration
namespace XMLBinary
{
class XMLBinaryReader;
};

//////////////////////////////////////////////////////////////////////////
// CBinaryXmlNode class only used for fast read only binary XML import
//////////////////////////////////////////////////////////////////////////
class CBinaryXmlNode : public IXmlNode
{
public:

	// collect allocated memory  informations
	void GetMemoryUsage(IDrxSizer* pSizer) const;

	//////////////////////////////////////////////////////////////////////////
	// Custom new/delete with pool allocator.
	//////////////////////////////////////////////////////////////////////////
	//uk operator new( size_t nSize );
	//void operator delete( uk ptr );

	virtual void DeleteThis() {}

	//! Create new XML node.
	XmlNodeRef createNode(tukk tag);

	// Summary:
	//	 Reference counting.
	virtual void AddRef() { ++m_pData->nRefCount; };
	// Notes:
	//	 When ref count reach zero XML node dies.
	virtual void Release() { if (--m_pData->nRefCount <= 0) delete m_pData; };

	//! Get XML node tag.
	tukk getTag() const          { return _string(_node()->nTagStringOffset); };
	void        setTag(tukk tag) { assert(0); };

	//! Return true if given tag is equal to node tag.
	bool isTag(tukk tag) const;

	//! Get XML Node attributes.
	virtual i32  getNumAttributes() const { return (i32)_node()->nAttributeCount; };
	//! Return attribute key and value by attribute index.
	virtual bool getAttributeByIndex(i32 index, tukk* key, tukk* value);
	//! Return attribute key and value by attribute index, string version.
	virtual bool getAttributeByIndex(i32 index, XmlString& key, XmlString& value);

	virtual void shareChildren(const XmlNodeRef& fromNode) { assert(0); };
	virtual void copyAttributes(XmlNodeRef fromNode)       { assert(0); };

	//! Get XML Node attribute for specified key.
	tukk getAttr(tukk key) const;

	//! Get XML Node attribute for specified key.
	// Returns true if the attribute exists, false otherwise.
	bool getAttr(tukk key, tukk* value) const;

	//! Check if attributes with specified key exist.
	bool       haveAttr(tukk key) const;

	XmlNodeRef newChild(tukk tagName)                     { assert(0); return 0; };
	void       replaceChild(i32 inChild, const XmlNodeRef& node) { assert(0); };
	void       insertChild(i32 inChild, const XmlNodeRef& node)  { assert(0); };
	void       addChild(const XmlNodeRef& node)                  { assert(0); };
	void       removeChild(const XmlNodeRef& node)               { assert(0); };

	//! Remove all child nodes.
	void removeAllChilds() { assert(0); };

	//! Get number of child XML nodes.
	i32 getChildCount() const { return (i32)_node()->nChildCount; };

	//! Get XML Node child nodes.
	XmlNodeRef getChild(i32 i) const;

	//! Find node with specified tag.
	XmlNodeRef findChild(tukk tag) const;
	void       deleteChild(tukk tag) { assert(0); };
	void       deleteChildAt(i32 nIndex)    { assert(0); };

	//! Get parent XML node.
	XmlNodeRef getParent() const;

	//! Returns content of this node.
	tukk getContent() const          { return _string(_node()->nContentStringOffset); };
	void        setContent(tukk str) { assert(0); };

	//! Typically creates a clone of the XML node, but since we are disallowing writing anyway we simply return the current node.
	XmlNodeRef  clone()                     { return this; };

	//! Returns line number for XML tag.
	i32  getLine() const   { return 0; };
	//! Set line number in xml.
	void setLine(i32 line) { assert(0); };

	//! Returns XML of this node and sub nodes.
	virtual IXmlStringData* getXMLData(i32 nReserveMem = 0) const                                      { assert(0); return 0; };
	XmlString               getXML(i32 level = 0) const                                                { assert(0); return ""; };
	bool                    saveToFile(tukk fileName)                                           { assert(0); return false; }; // saves in one huge chunk
	bool                    saveToFile(tukk fileName, size_t chunkSizeBytes, FILE* file = NULL) { assert(0); return false; }; // save in small memory chunks

	//! Set new XML Node attribute (or override attribute with same key).
	void setAttr(tukk key, tukk value)                                    { assert(0); };
	void setAttr(tukk key, i32 value)                                            { assert(0); };
	void setAttr(tukk key, u32 value)                                   { assert(0); };
	void setAttr(tukk key, int64 value)                                          { assert(0); };
	void setAttr(tukk key, uint64 value, bool useHexFormat = true /* ignored */) { assert(0); };
	void setAttr(tukk key, float value)                                          { assert(0); };
	void setAttr(tukk key, f64 value)                                            { assert(0); };
	void setAttr(tukk key, const Vec2& value)                                    { assert(0); };
	void setAttr(tukk key, const Vec2d& value)                                   { assert(0); };
	void setAttr(tukk key, const Ang3& value)                                    { assert(0); };
	void setAttr(tukk key, const Vec3& value)                                    { assert(0); };
	void setAttr(tukk key, const Vec4& value)                                    { assert(0); };
	void setAttr(tukk key, const Vec3d& value)                                   { assert(0); };
	void setAttr(tukk key, const Quat& value)                                    { assert(0); };
	void delAttr(tukk key)                                                       { assert(0); };
	void setAttr(tukk key, const DrxGUID& value);
	void removeAllAttributes()                                                          { assert(0); };

	//! Get attribute value of node.
	bool getAttr(tukk key, i32& value) const;
	bool getAttr(tukk key, u32& value) const;
	bool getAttr(tukk key, int64& value) const;
	bool getAttr(tukk key, uint64& value, bool useHexFormat = true /* ignored */) const;
	bool getAttr(tukk key, float& value) const;
	bool getAttr(tukk key, f64& value) const;
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
	//	bool getAttr( tukk key,CString &value ) const { XmlString v; if (getAttr(key,v)) { value = (tukk)v; return true; } else return false; }

private:
	//////////////////////////////////////////////////////////////////////////
	// INTERNAL METHODS
	//////////////////////////////////////////////////////////////////////////
	tukk GetValue(tukk key) const
	{
		const XMLBinary::Attribute* const pAttributes = m_pData->pAttributes;
		tukk const pStringData = m_pData->pStringData;

		i32k nFirst = _node()->nFirstAttributeIndex;
		i32k nLast = nFirst + _node()->nAttributeCount;
		for (i32 i = nFirst; i < nLast; i++)
		{
			tukk const attrKey = pStringData + pAttributes[i].nKeyStringOffset;
			if (g_pXmlStrCmp(key, attrKey) == 0)
			{
				tukk attrValue = pStringData + pAttributes[i].nValueStringOffset;
				return attrValue;
			}
		}
		return 0;
	}

	// Return current node in binary data.
	const XMLBinary::Node* _node() const
	{
		// cppcheck-suppress thisSubtraction
		return &m_pData->pNodes[this - m_pData->pBinaryNodes];
	}

	tukk _string(i32 nIndex) const
	{
		return m_pData->pStringData + nIndex;
	}

protected:
	virtual void setParent(const XmlNodeRef& inRef) { assert(0); }

	//////////////////////////////////////////////////////////////////////////
private:
	CBinaryXmlData* m_pData;

	friend class XMLBinary::XMLBinaryReader;
};

#endif // __XML_NODE_HEADER__
