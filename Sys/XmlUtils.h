// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   XmlUtils.h
//  Created:     21/04/2006 by Timur.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __XmlUtils_h__
#define __XmlUtils_h__
#pragma once

#include <drx3D/Sys/ISystem.h>

#ifdef _RELEASE
	#define CHECK_STATS_THREAD_OWNERSHIP()
#else
	#define CHECK_STATS_THREAD_OWNERSHIP() if (m_statsThreadOwner != DrxGetCurrentThreadId()) __debugbreak();
#endif

class CXmlNodePool;
class CXMLPatcher;

//////////////////////////////////////////////////////////////////////////
// Implements IXmlUtils interface.
//////////////////////////////////////////////////////////////////////////
class CXmlUtils : public IXmlUtils, public ISystemEventListener
{
public:
	CXmlUtils(ISystem* pSystem);
	virtual ~CXmlUtils();

	//////////////////////////////////////////////////////////////////////////
	// IXmlUtils
	//////////////////////////////////////////////////////////////////////////

	virtual IXmlParser* CreateXmlParser();

	// Load xml from file, returns 0 if load failed.
	virtual XmlNodeRef LoadXmlFromFile(tukk sFilename, bool bReuseStrings = false, bool bEnablePatching = true);
	// Load xml from memory buffer, returns 0 if load failed.
	virtual XmlNodeRef LoadXmlFromBuffer(tukk buffer, size_t size, bool bReuseStrings = false);

	// create an MD5 hash of an XML file
	virtual tukk HashXml(XmlNodeRef node);

	// Get an object that can read a xml into a IReadXMLSink
	// and write a xml from a IWriteXMLSource
	virtual IReadWriteXMLSink* GetIReadWriteXMLSink();

	virtual IXmlSerializer*    CreateXmlSerializer();

	virtual bool               SaveBinaryXmlFile(tukk sFilename, XmlNodeRef root);
	virtual XmlNodeRef         LoadBinaryXmlFile(tukk sFilename, bool bEnablePatching = true);

	virtual bool               EnableBinaryXmlLoading(bool bEnable);

	// Create XML Table reader.
	virtual IXmlTableReader* CreateXmlTableReader();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// ISystemEventListener
	//////////////////////////////////////////////////////////////////////////
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	virtual void GetMemoryUsage(IDrxSizer* pSizer);

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Init xml stats nodes pool
	virtual void InitStatsXmlNodePool(u32 nPoolSize = 1024*1024);

	// Create new xml node for statistics
	virtual XmlNodeRef CreateStatsXmlNode(tukk sNodeName = "");

	// Set owner thread
	virtual void SetStatsOwnerThread(threadID threadId);

	// Free memory if stats xml node pool is empty
	virtual void FlushStatsXmlNodePool();

	// Set the XML Patcher. This is an XML object that modifies named XML files as they are loaded
	// EXCEPT for xml files loaded from a buffer, for which names aren't passed in
	virtual void SetXMLPatcher(XmlNodeRef* pPatcher);

private:
	ISystem*           m_pSystem;
	IReadWriteXMLSink* m_pReadWriteXMLSink;
	CXmlNodePool*      m_pStatsXmlNodePool;
	CXMLPatcher*       m_pXMLPatcher;           //If set, applies data patches to any XML file that is loaded by this class
#ifndef _RELEASE
	threadID           m_statsThreadOwner;
#endif
};

#endif //__XmlUtils_h__
