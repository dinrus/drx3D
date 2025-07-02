// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once

#ifndef XMLCPB_UTILS_H
	#define XMLCPB_UTILS_H

	#include <drx3D/Act/XMLCPB_NodeLiveReader.h>
	#include <drx3D/CoreX/Platform/IPlatformOS.h>

namespace XMLCPB {

	#ifdef XMLCPB_DEBUGUTILS

class CDebugUtils : public IPlatformOS::IPlatformListener
{
public:

	static void Create()
	{
		if (!s_pThis)
		{
			s_pThis = new CDebugUtils();
		}
	}

	static void Destroy()
	{
		SAFE_DELETE(s_pThis);
	}

	static XmlNodeRef BinaryFileToXml(tukk pBinaryFileName);
	static void       DumpToXmlFile(CNodeLiveReaderRef BRoot, tukk pXmlFileName);
	static void       DumpToLog(CNodeLiveReaderRef BRoot);
	static void       SetLastFileNameSaved(tukk pFileName);

	// IPlatformOS::IPlatformListener
	virtual void OnPlatformEvent(const IPlatformOS::SPlatformEvent& event);
	// ~IPlatformOS::IPlatformListener

private:

	CDebugUtils();
	virtual ~CDebugUtils();
	static void RecursiveCopyAttrAndChildsIntoXmlNode(XmlNodeRef xmlNode, const CNodeLiveReaderRef& BNode);
	static void GenerateXMLFromLastSaveCmd(IConsoleCmdArgs* args);
	static void GenerateXmlFileWithSizeInformation(tukk pBinaryFileName, tukk pXmlFileName);

private:
	string              m_lastFileNameSaved;
	static CDebugUtils* s_pThis;
};

	#else //XMLCPB_DEBUGUTILS
class CDebugUtils
{
public:
	static void Create()  {}
	static void Destroy() {};
};
	#endif
} // end namespace

#endif
