// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ixml.h
//  Version:     v1.00
//  Created:     16/7/2002 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio.NET
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Color.h>

class IDrxSizer;
struct DrxGUID;

#if defined(_AFX) && !defined(RESOURCE_COMPILER)
#include <drx3D/CoreX/ToolsHelpers/GuidUtil.h>
#endif

class IXMLBinarySerializer;
struct IReadWriteXMLSink;
struct ISerialize;

/*
   This is wrapper around expat library to provide DOM type of access for xml.
   Do not use IXmlNode class directly instead always use XmlNodeRef wrapper that
   takes care of memory management issues.

   Usage Example:
   -------------------------------------------------------
   void testXml(bool bReuseStrings)
   {
   XmlParser xml(bReuseStrings);
   XmlNodeRef root = xml.ParseFile("test.xml", true);

   if (root)
   {
    for (i32 i = 0; i < root->getChildCount(); ++i)
    {
      XmlNodeRef child = root->getChild(i);
      if (child->isTag("world"))
      {
        if (child->getAttr("name") == "blah")
        {
          ....
        }
      }
    }
   }
   }
 */

//! Special string wrapper for xml nodes.
class XmlString : public string
{
public:
	XmlString() {};
	XmlString(tukk str) : string(str) {};
#ifdef  _AFX
	XmlString(const CString& str) : string((tukk)str) {};
#endif

	operator tukk () const { return c_str(); }

};

//! \cond INTERNAL
//! XML string data.
struct IXmlStringData
{
	// <interfuscator:shuffle>
	virtual ~IXmlStringData(){}
	virtual void        AddRef() = 0;
	virtual void        Release() = 0;
	virtual tukk GetString() = 0;
	virtual size_t      GetStringLength() = 0;
	// </interfuscator:shuffle>
};
//! \endcond

class IXmlNode;

//! XmlNodeRef, wrapper class implementing reference counting for IXmlNode.
class XmlNodeRef
{
private:
	IXmlNode* p;
public:
	XmlNodeRef() : p(NULL) {}
	XmlNodeRef(IXmlNode* p_);
	XmlNodeRef(const XmlNodeRef& p_);
	XmlNodeRef(XmlNodeRef&& other);

	~XmlNodeRef();
	
	bool     isValid() const   { return p != nullptr; }
	operator IXmlNode*() const { return p; }

	IXmlNode&   operator*() const      { return *p; }
	IXmlNode*   operator->(void) const { return p; }

	XmlNodeRef& operator=(IXmlNode* newp);
	XmlNodeRef& operator=(const XmlNodeRef& newp);

	XmlNodeRef& operator=(XmlNodeRef&& other);

#if !defined(RESOURCE_COMPILER)
	template<typename Sizer>
	void GetMemoryUsage(Sizer* pSizer) const
	{
		pSizer->AddObject(p);
	}
#endif
};

//! Never use IXmlNode directly - instead use reference counted XmlNodeRef.
class IXmlNode
{
protected:
	i32 m_nRefCount;

protected:
	// <interfuscator:shuffle>
	virtual void DeleteThis() = 0;
	virtual ~IXmlNode() {};
	// </interfuscator:shuffle>

public:

	// <interfuscator:shuffle>
	//! Creates new XML node.
	virtual XmlNodeRef createNode(tukk tag) = 0;

	// AddRef/Release need to be virtual to permit overloading from CXMLNodePool.

	//! Reference counting.
	virtual void AddRef() { m_nRefCount++; };

	//! When ref count reaches zero, the XML node dies.
	virtual void Release()           { if (--m_nRefCount <= 0) DeleteThis(); };
	virtual i32  GetRefCount() const { return m_nRefCount; };

	//! Get XML node tag.
	virtual tukk getTag() const = 0;

	//! Sets XML node tag.
	virtual void setTag(tukk tag) = 0;

	//! Return true if a given tag equal to node tag.
	virtual bool isTag(tukk tag) const = 0;

	//! Get XML Node attributes.
	virtual i32 getNumAttributes() const = 0;

	//! Return attribute key and value by attribute index.
	virtual bool getAttributeByIndex(i32 index, tukk* key, tukk* value) = 0;

	//! Copy attributes to this node from a given node.
	virtual void copyAttributes(XmlNodeRef fromNode) = 0;

	//! Get XML Node attribute for specified key.
	virtual tukk getAttr(tukk key) const = 0;

	//! Gets XML Node attribute for specified key.
	//! \return true if the attribute exists, false otherwise.
	virtual bool getAttr(tukk key, tukk* value) const = 0;

	//! Checks if attributes with specified key exist.
	virtual bool haveAttr(tukk key) const = 0;

	//! Add new child node.
	virtual void addChild(const XmlNodeRef& node) = 0;

	//! Create new xml node and add it to childs list.
	virtual XmlNodeRef newChild(tukk tagName) = 0;

	//! Remove child node.
	virtual void removeChild(const XmlNodeRef& node) = 0;

	//! Insert child node.
	virtual void insertChild(i32 nIndex, const XmlNodeRef& node) = 0;

	//! Replaces a specified child with the passed one.
	//! Not supported by all node implementations.
	virtual void replaceChild(i32 nIndex, const XmlNodeRef& fromNode) = 0;

	//! Remove all child nodes.
	virtual void removeAllChilds() = 0;

	//! Get number of child XML nodes.
	//! \par Example
	//! \include DinrusXSys/Examples/XmlParsing.cpp
	virtual i32 getChildCount() const = 0;

	//! Get XML Node child nodes.
	//! \par Example
	//! \include DinrusXSys/Examples/XmlParsing.cpp
	virtual XmlNodeRef getChild(i32 i) const = 0;

	//! Find node with specified tag.
	virtual XmlNodeRef findChild(tukk tag) const = 0;

	//! Get parent XML node.
	virtual XmlNodeRef getParent() const = 0;

	//! Set parent XML node.
	virtual void setParent(const XmlNodeRef& inRef) = 0;

	//! Return content of this node.
	virtual tukk getContent() const = 0;

	//! Set content of this node.
	virtual void setContent(tukk str) = 0;

	//! Deep clone of this and all child xml nodes.
	virtual XmlNodeRef clone() = 0;

	//! Return line number for XML tag.
	virtual i32 getLine() const = 0;

	//! Set line number in xml.
	virtual void setLine(i32 line) = 0;

	//! Return XML of this node and sub nodes.
	//! \note IXmlStringData pointer must be release when string is not needed anymore.
	//! \see IXmlStringData
	virtual IXmlStringData* getXMLData(i32 nReserveMem = 0) const = 0;

	//! Returns XML of this node and sub nodes.
	virtual XmlString getXML(i32 level = 0) const = 0;
	//! Saves the XML node to disk
	//! \par Example
	//! \include DinrusXSys/Examples/XmlWriting.cpp
	virtual bool      saveToFile(tukk fileName) = 0;

	//! Set new XML Node attribute (or override attribute with same key).
	//! @{
	virtual void setAttr(tukk key, tukk value) = 0;
	virtual void setAttr(tukk key, i32 value) = 0;
	virtual void setAttr(tukk key, u32 value) = 0;
	virtual void setAttr(tukk key, int64 value) = 0;
	virtual void setAttr(tukk key, uint64 value, bool useHexFormat = true) = 0;
	virtual void setAttr(tukk key, float value) = 0;
	virtual void setAttr(tukk key, double value) = 0;
	virtual void setAttr(tukk key, const Vec2& value) = 0;
	virtual void setAttr(tukk key, const Vec2d& value) = 0;
	virtual void setAttr(tukk key, const Ang3& value) = 0;
	virtual void setAttr(tukk key, const Vec3& value) = 0;
	virtual void setAttr(tukk key, const Vec4& value) = 0;
	virtual void setAttr(tukk key, const Vec3d& value) = 0;
	virtual void setAttr(tukk key, const Quat& value) = 0;
#if (DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) || DRX_PLATFORM_APPLE
	//! Compatability functions, on Linux and Mac i32 is the default int64_t.
	ILINE void setAttr(tukk key, u64 value, bool useHexFormat = true)
	{
		setAttr(key, (uint64)value, useHexFormat);
	}

	ILINE void setAttr(tukk key, i32 value)
	{
		setAttr(key, (int64)value);
	}
#endif
	//! @}.

	//! Delete attribute.
	virtual void delAttr(tukk key) = 0;

	//! Remove all node attributes.
	virtual void removeAllAttributes() = 0;

	//! Get attribute value of node.
	//! @{
	virtual bool getAttr(tukk key, i32& value) const = 0;
	virtual bool getAttr(tukk key, u32& value) const = 0;
	virtual bool getAttr(tukk key, int64& value) const = 0;
	virtual bool getAttr(tukk key, uint64& value, bool useHexFormat = true) const = 0;
	virtual bool getAttr(tukk key, float& value) const = 0;
	virtual bool getAttr(tukk key, double& value) const = 0;
	virtual bool getAttr(tukk key, Vec2& value) const = 0;
	virtual bool getAttr(tukk key, Vec2d& value) const = 0;
	virtual bool getAttr(tukk key, Ang3& value) const = 0;
	virtual bool getAttr(tukk key, Vec3& value) const = 0;
	virtual bool getAttr(tukk key, Vec4& value) const = 0;
	virtual bool getAttr(tukk key, Vec3d& value) const = 0;
	virtual bool getAttr(tukk key, Quat& value) const = 0;
	virtual bool getAttr(tukk key, bool& value) const = 0;
	virtual bool getAttr(tukk key, XmlString& value) const = 0;
	virtual bool getAttr(tukk key, ColorB& value) const = 0;

#if (DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) || DRX_PLATFORM_APPLE
	//! Compatability functions, on Linux and Mac i32 is the default int64_t.
	ILINE bool getAttr(tukk key, u64& value, bool useHexFormat = true) const
	{
		return getAttr(key, (uint64&)value, useHexFormat);
	}

	ILINE bool getAttr(tukk key, i32& value) const
	{
		return getAttr(key, (int64&)value);
	}
#endif

	// </interfuscator:shuffle>

#if !defined(RESOURCE_COMPILER)
	// <interfuscator:shuffle>
	//! Collect all allocated memory
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	//! Copy children to this node from a given node.
	//! Children are reference copied (shallow copy) and the children's parent is NOT set to this
	//! node, but left with its original parent (which is still the parent).
	virtual void shareChildren(const XmlNodeRef& fromNode) = 0;

	//! Remove child node at known position.
	virtual void deleteChildAt(i32 nIndex) = 0;

	//! Return XML of this node and sub nodes into tmpBuffer without XML checks (much faster).
	virtual XmlString getXMLUnsafe(i32 level, tuk tmpBuffer, u32 sizeOfTmpBuffer) const { return getXML(level); }

	//! Save in small memory chunks.
	virtual bool saveToFile(tukk fileName, size_t chunkSizeBytes, FILE* file = NULL) = 0;
	// </interfuscator:shuffle>
#endif
	//! @}.

	//! Inline Helpers.
	//! @{
#if !(DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) && !DRX_PLATFORM_APPLE
	bool getAttr(tukk key, long& value) const           { i32 v; if (getAttr(key, v)) { value = v; return true; } else return false; }
	bool getAttr(tukk key, u64& value) const  { u32 v; if (getAttr(key, v)) { value = v; return true; } else return false; }
	void setAttr(tukk key, u64 value)         { setAttr(key, (u32)value); };
	void setAttr(tukk key, long value)                  { setAttr(key, (i32)value); };
#endif
	bool getAttr(tukk key, unsigned short& value) const { u32 v; if (getAttr(key, v)) { value = v; return true; } else return false; }
	bool getAttr(tukk key, u8& value) const  { u32 v; if (getAttr(key, v)) { value = v; return true; } else return false; }
	bool getAttr(tukk key, short& value) const          { i32 v; if (getAttr(key, v)) { value = v; return true; } else return false; }
	bool getAttr(tukk key, char& value) const           { i32 v; if (getAttr(key, v)) { value = v; return true; } else return false; }
	//! @}.

#ifndef RESOURCE_COMPILER
#ifdef _AFX
	//! Gets CString attribute.
	bool getAttr(tukk key, CString& value) const
	{
		if (!haveAttr(key))
			return false;
		value = getAttr(key);
		return true;
	}

	//! Gets string attribute.
	bool getAttr(tukk key, string& value) const
	{
		if (!haveAttr(key))
			return false;
		value = getAttr(key);
		return true;
	}

	//! Sets GUID attribute.
	void setAttr(tukk key, REFGUID value)
	{
		tukk str = GuidUtil::ToString(value);
		setAttr(key, str);
	};

	//! Gets GUID from attribute.
	bool getAttr(tukk key, GUID& value) const
	{
		if (!haveAttr(key))
			return false;
		tukk guidStr = getAttr(key);
		value = GuidUtil::FromString(guidStr);
		if (value.Data1 == 0)
		{
			memset(&value, 0, sizeof(value));
			// If bad GUID, use old guid system.
			value.Data1 = atoi(guidStr);
		}
		return true;
	}
#endif //_AFX
	// Those are defined as abstract here because DrxGUID header has access to gEnv, which breaks compilation
	virtual void setAttr(tukk key, const DrxGUID& value) = 0;

	virtual bool getAttr(tukk key, DrxGUID& value) const = 0;
#endif //RESOURCE_COMPILER

	// Lets be friendly to him.
	friend class XmlNodeRef;
};

/*
   //! Inline Implementation of XmlNodeRef
   inline XmlNodeRef::XmlNodeRef(tukk tag, IXmlNode *node)
   {
   if (node)
   {
    p = node->createNode(tag);
   }
   else
   {
    p = new XmlNode(tag);
   }
   p->AddRef();
   }
 */

//////////////////////////////////////////////////////////////////////////
inline XmlNodeRef::XmlNodeRef(IXmlNode* p_) : p(p_)
{
	if (p) p->AddRef();
}

inline XmlNodeRef::XmlNodeRef(const XmlNodeRef& p_) : p(p_.p)
{
	if (p) p->AddRef();
}

// Move constructor
inline XmlNodeRef::XmlNodeRef(XmlNodeRef&& other)
{
	if (this != &other)
	{
		p = other.p;
		other.p = nullptr;
	}
}

inline XmlNodeRef::~XmlNodeRef()
{
	if (p) p->Release();
}

inline XmlNodeRef& XmlNodeRef::operator=(XmlNodeRef&& other)
{
	if (this != &other)
	{
		if (p) p->Release();
		p = other.p;
		other.p = nullptr;
	}
	return *this;
}

inline XmlNodeRef& XmlNodeRef::operator=(IXmlNode* newp)
{
	if (newp) newp->AddRef();
	if (p) p->Release();
	p = newp;
	return *this;
}

inline XmlNodeRef& XmlNodeRef::operator=(const XmlNodeRef& newp)
{
	if (newp.p) newp.p->AddRef();
	if (p) p->Release();
	p = newp.p;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
struct IXmlSerializer
{
	// <interfuscator:shuffle>
	virtual ~IXmlSerializer(){}
	virtual void        AddRef() = 0;
	virtual void        Release() = 0;

	virtual ISerialize* GetWriter(XmlNodeRef& node) = 0;
	virtual ISerialize* GetReader(XmlNodeRef& node) = 0;
	// </interfuscator:shuffle>
#if !defined(RESOURCE_COMPILER)
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;
#endif
};

#if !defined(RESOURCE_COMPILER)
//////////////////////////////////////////////////////////////////////////
//! XML Parser interface.
//! \cond INTERNAL
struct IXmlParser
{
	// <interfuscator:shuffle>
	virtual ~IXmlParser(){}
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	//! Parse xml file.
	virtual XmlNodeRef ParseFile(tukk filename, bool bCleanPools) = 0;

	//! Parse xml from memory buffer.
	virtual XmlNodeRef ParseBuffer(tukk buffer, i32 nBufLen, bool bCleanPools) = 0;

	virtual void       GetMemoryUsage(IDrxSizer* pSizer) const = 0;
	// </interfuscator:shuffle>
};

//////////////////////////////////////////////////////////////////////////
//! XML Table Reader interface.
//! Can be used to read tables exported from Excel in .xml format.
//! Supports reading DRXENGINE's version of those Excel .xml tables (produced by RC).
//! Usage:
//! \code
//! p->Begin(rootNode);
//! while (p->ReadRow(...))
//! {
//!     while (p->ReadCell(...))
//!     {
//!         ...
//!     }
//! }
//! \endcode
struct IXmlTableReader
{
	// <interfuscator:shuffle>
	virtual ~IXmlTableReader(){}

	virtual void Release() = 0;

	//! Return false if XML tree is not in supported table format.
	virtual bool Begin(XmlNodeRef rootNode) = 0;

	//! Return estimated number of rows (estimated number of ReadRow() calls returning true).
	//! Returned number is equal *or greater* than real number, because it's impossible to
	//! know real number in advance in case of Excel XML.
	virtual i32 GetEstimatedRowCount() = 0;

	//! Prepare next row for reading by ReadCell().
	//! \note Empty rows are skipped sometimes, so use returned rowIndex if you need to know absolute row index.
	//! \return true and sets rowIndex if the row was prepared successfully, false if no rows left.
	virtual bool ReadRow(i32& rowIndex) = 0;

	//! Read next cell in the current row.
	//! \note Empty cells are skipped sometimes, so use returned cellIndex if you need to know absolute cell index (i.e. column).
	//! \return true and sets columnIndex, pContent, contenSize if the cell was read successfully, false if no cells left in the row.
	virtual bool ReadCell(i32& columnIndex, tukk & pContent, size_t& contentSize) = 0;
	// </interfuscator:shuffle>
};
//! \endcond
#endif

//////////////////////////////////////////////////////////////////////////
//! IXmlUtils structure.
struct IXmlUtils
{
	// <interfuscator:shuffle>
	virtual ~IXmlUtils(){}

	//! Load xml file, returns 0 if load failed.
	virtual XmlNodeRef LoadXmlFromFile(tukk sFilename, bool bReuseStrings = false, bool bEnablePatching = true) = 0;

	//! Load xml from memory buffer, returns 0 if load failed.
	virtual XmlNodeRef LoadXmlFromBuffer(tukk buffer, size_t size, bool bReuseStrings = false) = 0;

	//! Create an MD5 hash of an XML file.
	virtual tukk HashXml(XmlNodeRef node) = 0;

	//! Get an object that can read a xml into a IReadXMLSink and writes a xml from a IWriteXMLSource
	virtual IReadWriteXMLSink* GetIReadWriteXMLSink() = 0;

	//! Create XML Writer for ISerialize interface.
	virtual IXmlSerializer* CreateXmlSerializer() = 0;
	// </interfuscator:shuffle>

#if !defined(RESOURCE_COMPILER)
	// <interfuscator:shuffle>
	//! Create XML Parser.
	//! \note IXmlParser does not normally support recursive XML loading, all nodes loaded by this parser are invalidated on loading new file.
	//! This is a specialized interface for fast loading of many XMLs,
	//! After use it must be released with call to Release method.
	virtual IXmlParser* CreateXmlParser() = 0;

	//! Create XML to file in the binary form.
	virtual bool SaveBinaryXmlFile(tukk sFilename, XmlNodeRef root) = 0;

	//! Read XML data from file in the binary form.
	virtual XmlNodeRef LoadBinaryXmlFile(tukk sFilename, bool bEnablePatching = true) = 0;

	//! Enable or disables checking for binary xml files.
	//! \return Previous status.
	virtual bool EnableBinaryXmlLoading(bool bEnable) = 0;

	//! After use it must be released with call to Release method.
	virtual IXmlTableReader* CreateXmlTableReader() = 0;

	//! Init xml stats nodes pool.
	virtual void InitStatsXmlNodePool(u32 nPoolSize) = 0;

	//! Creates new xml node for statistics.
	virtual XmlNodeRef CreateStatsXmlNode(tukk sNodeName) = 0;

	//! Set owner thread.
	virtual void SetStatsOwnerThread(threadID threadId) = 0;

	//! Free memory held on to by xml pool if empty.
	virtual void FlushStatsXmlNodePool() = 0;

	//! Sets the patch which is used to transform loaded XML files.
	//! The patch itself is encoded into XML.
	//! Set to NULL to clear an existing transform and disable further patching.
	virtual void SetXMLPatcher(XmlNodeRef* pPatcher) = 0;
	// </interfuscator:shuffle>
#endif
};
