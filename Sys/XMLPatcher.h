// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   XMLPatcher.h
//  Created:     30/01/2012 by Rob Jessop.
//  Описание:
// -------------------------------------------------------------------------
//  История:	Split from GameDLL/DataPatcher so it could be applied to every
//						XML file on load
//
////////////////////////////////////////////////////////////////////////////

#ifndef __XML_PATCHER_HEADER__
#define __XML_PATCHER_HEADER__

#if DRX_PLATFORM_WINDOWS && !defined(_RELEASE)
	#define DATA_PATCH_DEBUG 1
#else
	#define DATA_PATCH_DEBUG 0
#endif

class CXMLPatcher
{
protected:
#if DATA_PATCH_DEBUG
	ICVar * m_pDumpFilesCVar;
#endif

	XmlNodeRef m_patchXML;
	tukk m_pFileBeingPatched;

	void       PatchFail(
	  tukk pInReason);
	XmlNodeRef ApplyPatchToNode(
	  const XmlNodeRef& inNode,
	  const XmlNodeRef& inPatch);

	XmlNodeRef DuplicateForPatching(
	  const XmlNodeRef& inOrig,
	  bool inShareChildren);

	bool CompareTags(
	  const XmlNodeRef& inA,
	  const XmlNodeRef& inB);
	XmlNodeRef GetMatchTag(
	  const XmlNodeRef& inNode);
	XmlNodeRef GetReplaceTag(
	  const XmlNodeRef& inNode,
	  bool* outShouldReplaceChildren);
	XmlNodeRef GetInsertTag(
	  const XmlNodeRef& inNode);
	XmlNodeRef GetDeleteTag(
	  const XmlNodeRef& inNode);
	XmlNodeRef FindPatchForFile(
	  tukk pInFileToPatch);

#if DATA_PATCH_DEBUG
	void DumpXMLNodes(
	  FILE* pInFile,
	  i32 inIndent,
	  const XmlNodeRef& inNode,
	  DrxFixedStringT<512>* ioTempString);
	void DumpFiles(
	  tukk pInXMLFileName,
	  const XmlNodeRef& inBefore,
	  const XmlNodeRef& inAfter);
	void DumpXMLFile(
	  tukk pInFilePath,
	  const XmlNodeRef& inNode);
#endif

public:
	CXMLPatcher(XmlNodeRef& patchXML);
	~CXMLPatcher();

	XmlNodeRef ApplyXMLDataPatch(
	  const XmlNodeRef& inNode,
	  tukk pInXMLFileName);
};

#endif //__XML_PATCHER_HEADER__
