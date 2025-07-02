// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// Created by: Michael Smith
//---------------------------------------------------------------------------
#ifndef __XMLBINARYHEADERS_H__
#define __XMLBINARYHEADERS_H__

namespace XMLBinary
{
class IDataWriter
{
public:
	virtual ~IDataWriter() {}
	virtual void Write(ukk pData, size_t size) = 0;
};

class IFilter
{
public:
	enum EType
	{
		eType_ElementName,
		eType_AttributeName
	};
	virtual ~IFilter() {}
	virtual bool IsAccepted(EType type, tukk pName) const = 0;
};

//////////////////////////////////////////////////////////////////////////
typedef u32 NodeIndex;              //!< Only u32 or u16 are supported.

template<i32 size> struct Pad
{
	char pad[size];
};
template<> struct Pad<0> {};

struct Node
{
	u32    nTagStringOffset;        //!< Offset in CBinaryXmlData::pStringData.
	u32    nContentStringOffset;    //!< Offset in CBinaryXmlData::pStringData.
	u16    nAttributeCount;
	u16    nChildCount;
	NodeIndex nParentIndex;
	NodeIndex nFirstAttributeIndex;
	NodeIndex nFirstChildIndex;
	Pad<sizeof(u32) - sizeof(NodeIndex)> reserved_for_alignment;
};

struct Attribute
{
	u32 nKeyStringOffset;           //!< Offset in CBinaryXmlData::pStringData.
	u32 nValueStringOffset;         //!< Offset in CBinaryXmlData::pStringData.
};

struct BinaryFileHeader
{
	char   szSignature[8];
	u32 nXMLSize;
	u32 nNodeTablePosition;
	u32 nNodeCount;
	u32 nAttributeTablePosition;
	u32 nAttributeCount;
	u32 nChildTablePosition;
	u32 nChildCount;
	u32 nStringDataPosition;
	u32 nStringDataSize;
};
}

#endif //__XMLBINARYHEADERS_H__
