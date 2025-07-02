// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XMLCPB_BufferReader.h>
#include <drx3D/Act/XMLCPB_Reader.h>

using namespace XMLCPB;

//////////////////////////////////////////////////////////////////////////

void SBufferReader::ReadFromFile(CReader& Reader, IPlatformOS::ISaveReaderPtr& pOSSaveReader, u32 readSize)
{
	assert(m_bufferSize == 0);
	assert(!m_pBuffer);

	m_pBuffer = (u8*)m_pHeap->Malloc(readSize, "");

	m_bufferSize = readSize;

	Reader.ReadDataFromFile(pOSSaveReader, const_cast<u8*>(GetPointer(0)), readSize);
}

//////////////////////////////////////////////////////////////////////////

void SBufferReader::ReadFromMemory(CReader& Reader, u8k* pData, u32 dataSize, u32 readSize, u32& outReadLoc)
{
	assert(m_bufferSize == 0);
	assert(!m_pBuffer);

	m_pBuffer = (u8*)m_pHeap->Malloc(readSize, "");

	m_bufferSize = readSize;

	Reader.ReadDataFromMemory(pData, dataSize, const_cast<u8*>(GetPointer(0)), readSize, outReadLoc);
}
