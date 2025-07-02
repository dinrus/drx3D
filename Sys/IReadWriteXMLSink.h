// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/CoreX/DrxVariant.h>
#include <drx3D/Sys/IXml.h>

struct IReadXMLSink;
struct IWriteXMLSource;

struct IReadWriteXMLSink
{
	// <interfuscator:shuffle>
	virtual ~IReadWriteXMLSink(){}
	virtual bool       ReadXML(tukk definitionFile, tukk dataFile, IReadXMLSink* pSink) = 0;
	virtual bool       ReadXML(tukk definitionFile, XmlNodeRef node, IReadXMLSink* pSink) = 0;
	virtual bool       ReadXML(XmlNodeRef definition, tukk dataFile, IReadXMLSink* pSink) = 0;
	virtual bool       ReadXML(XmlNodeRef definition, XmlNodeRef node, IReadXMLSink* pSink) = 0;

	virtual XmlNodeRef CreateXMLFromSource(tukk definitionFile, IWriteXMLSource* pSource) = 0;
	virtual bool       WriteXML(tukk definitionFile, tukk dataFile, IWriteXMLSource* pSource) = 0;
	// </interfuscator:shuffle>
};

struct SReadWriteXMLCommon
{
	typedef DrxVariant<Vec3, i32, float, tukk , bool> TValue;
};

TYPEDEF_AUTOPTR(IReadXMLSink);
typedef IReadXMLSink_AutoPtr IReadXMLSinkPtr;

//! This interface allows customization of the data read routines.
struct IReadXMLSink : public SReadWriteXMLCommon
{
	// <interfuscator:shuffle>
	virtual ~IReadXMLSink(){}

	//! Reference counting.
	virtual void            AddRef() = 0;
	virtual void            Release() = 0;

	virtual IReadXMLSinkPtr BeginTable(tukk name, const XmlNodeRef& definition) = 0;
	virtual IReadXMLSinkPtr BeginTableAt(i32 elem, const XmlNodeRef& definition) = 0;
	virtual bool            SetValue(tukk name, const TValue& value, const XmlNodeRef& definition) = 0;
	virtual bool            EndTableAt(i32 elem) = 0;
	virtual bool            EndTable(tukk name) = 0;

	virtual IReadXMLSinkPtr BeginArray(tukk name, const XmlNodeRef& definition) = 0;
	virtual bool            SetAt(i32 elem, const TValue& value, const XmlNodeRef& definition) = 0;
	virtual bool            EndArray(tukk name) = 0;

	virtual bool            Complete() = 0;

	virtual bool            IsCreationMode() = 0;
	virtual XmlNodeRef      GetCreationNode() = 0;
	virtual void            SetCreationNode(XmlNodeRef definition) = 0;
	// </interfuscator:shuffle>
};

TYPEDEF_AUTOPTR(IWriteXMLSource);
typedef IWriteXMLSource_AutoPtr IWriteXMLSourcePtr;

//! This interface allows customization of the data write routines.
struct IWriteXMLSource : public SReadWriteXMLCommon
{
	// <interfuscator:shuffle>
	virtual ~IWriteXMLSource(){}

	//! Reference counting.
	virtual void               AddRef() = 0;
	virtual void               Release() = 0;

	virtual IWriteXMLSourcePtr BeginTable(tukk name) = 0;
	virtual IWriteXMLSourcePtr BeginTableAt(i32 elem) = 0;
	virtual bool               HaveValue(tukk name) = 0;
	virtual bool               GetValue(tukk name, TValue& value, const XmlNodeRef& definition) = 0;
	virtual bool               EndTableAt(i32 elem) = 0;
	virtual bool               EndTable(tukk name) = 0;

	virtual IWriteXMLSourcePtr BeginArray(tukk name, size_t* numElems, const XmlNodeRef& definition) = 0;
	virtual bool               HaveElemAt(i32 elem) = 0;
	virtual bool               GetAt(i32 elem, TValue& value, const XmlNodeRef& definition) = 0;
	virtual bool               EndArray(tukk name) = 0;

	virtual bool               Complete() = 0;
	// </interfuscator:shuffle>
};

//! \endcond