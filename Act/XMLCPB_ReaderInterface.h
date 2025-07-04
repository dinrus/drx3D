// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once

#ifndef XMLCPB_READERINTERFACE_H
	#define XMLCPB_READERINTERFACE_H

	#include "XMLCPB_Reader.h"

namespace XMLCPB {

// this class is just to give a clear access to the intended interface functions and only them, without using a bunch of friend declarations
class CReaderInterface
{
public:
	explicit CReaderInterface(IGeneralMemoryHeap* pHeap)
		: m_reader(pHeap)
	{
	}

	CNodeLiveReaderRef GetRoot()                                          { return m_reader.GetRoot(); }
	CNodeLiveReaderRef CreateNodeRef()                                    { return m_reader.CreateNodeRef(); }
	bool               ReadBinaryFile(tukk pFileName)              { return m_reader.ReadBinaryFile(pFileName); }
	bool               ReadBinaryMemory(u8k* pData, u32 uSize) { return m_reader.ReadBinaryMemory(pData, uSize); }
	void               SaveTestFiles()                                    { m_reader.SaveTestFiles(); }

private:

	CReader m_reader;
};

}  // end namespace

#endif
