// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once

#ifndef XMLCPB_WRITERINTERFACE_H
	#define XMLCPB_WRITERINTERFACE_H

	#include "XMLCPB_Writer.h"

namespace XMLCPB {

// this class is just to give a clear access to the intended interface functions and only them, without using a bunch of friend declarations
// CWriter should not be used directly from outside
class CWriterInterface
{
public:

	void               Init(tukk pRootName, tukk pFileName)  { m_writer.Init(pRootName, pFileName); }
	CNodeLiveWriterRef GetRoot()                                           { return m_writer.GetRoot(); }
	void               Done()                                              { m_writer.Done(); } // should be called when everything is added and finished.
	bool               FinishWritingFile()                                 { return m_writer.FinishWritingFile(); }
	bool               WriteAllIntoMemory(u8*& rpData, u32& outSize) { return m_writer.WriteAllIntoMemory(rpData, outSize); }

private:

	CWriter m_writer;
};

}  // end namespace

#endif
