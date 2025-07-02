// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   XMLBinaryWriter.h
//  Created:     21/04/2006 by Michael Smith.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//     8/1/2008 - Modified by Timur
////////////////////////////////////////////////////////////////////////////

#ifndef __XMLBINARYWRITER_H__
#define __XMLBINARYWRITER_H__

#include <drx3D/Sys/IXml.h>
#include <drx3D/Sys/XMLBinaryHeaders.h>
#include <vector>
#include <map>

class IXMLDataSink;

namespace XMLBinary
{
class CXMLBinaryWriter
{
public:
	CXMLBinaryWriter();
	bool WriteNode(IDataWriter* pFile, XmlNodeRef node, bool bNeedSwapEndian, XMLBinary::IFilter* pFilter, string& error);

private:
	bool CompileTables(XmlNodeRef node, XMLBinary::IFilter* pFilter, string& error);

	bool CompileTablesForNode(XmlNodeRef node, i32 nParentIndex, XMLBinary::IFilter* pFilter, string& error);
	bool CompileChildTable(XmlNodeRef node, XMLBinary::IFilter* pFilter, string& error);
	i32  AddString(const XmlString& sString);

private:
	// tables.
	typedef std::map<IXmlNode*, i32> NodesMap;
	typedef std::map<string, uint>   StringMap;

	std::vector<Node>      m_nodes;
	NodesMap               m_nodesMap;
	std::vector<Attribute> m_attributes;
	std::vector<NodeIndex> m_childs;
	std::vector<string>    m_strings;
	StringMap              m_stringMap;

	uint                   m_nStringDataSize;
};
}

#endif //__XMLBINARYWRITER_H__
