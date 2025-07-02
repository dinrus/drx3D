// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XMLCPB_StringTableReader.h>
#include <drx3D/Act/XMLCPB_Reader.h>

using namespace XMLCPB;

//////////////////////////////////////////////////////////////////////////
CStringTableReader::CStringTableReader(IGeneralMemoryHeap* pHeap)
	: m_buffer(pHeap)
	, m_stringAddrs(NAlloc::GeneralHeapAlloc(pHeap))
{
}

//////////////////////////////////////////////////////////////////////////
tukk CStringTableReader::GetString(StringID stringId) const
{
	assert(stringId < m_stringAddrs.size());
	tuk pString = (tuk)(m_buffer.GetPointer(m_stringAddrs[stringId]));
	return pString;
}

//////////////////////////////////////////////////////////////////////////

void CStringTableReader::ReadFromFile(CReader& Reader, IPlatformOS::ISaveReaderPtr pOSSaveReader, const SFileHeader::SStringTable& headerInfo)
{
	assert(m_stringAddrs.empty());

	m_stringAddrs.resize(headerInfo.m_numStrings);
	FlatAddr* pstringAddrsTable = &(m_stringAddrs[0]);
	Reader.ReadDataFromFile(pOSSaveReader, pstringAddrsTable, sizeof(FlatAddr) * headerInfo.m_numStrings);
	for (u32 i = 0; i < m_stringAddrs.size(); ++i)
		SwapIntegerValue(m_stringAddrs[i]);

	m_buffer.ReadFromFile(Reader, pOSSaveReader, headerInfo.m_sizeStringData);
}

//////////////////////////////////////////////////////////////////////////

void CStringTableReader::ReadFromMemory(CReader& Reader, u8k* pData, u32 dataSize, const SFileHeader::SStringTable& headerInfo, u32& outReadLoc)
{
	assert(m_stringAddrs.empty());

	m_stringAddrs.resize(headerInfo.m_numStrings);
	FlatAddr* pstringAddrsTable = &(m_stringAddrs[0]);
	Reader.ReadDataFromMemory(pData, dataSize, pstringAddrsTable, sizeof(FlatAddr) * headerInfo.m_numStrings, outReadLoc);

	m_buffer.ReadFromMemory(Reader, pData, dataSize, headerInfo.m_sizeStringData, outReadLoc);
}

//////////////////////////////////////////////////////////////////////////
// debug / statistics purposes

#ifndef _RELEASE
void CStringTableReader::WriteStringsIntoTextFile(tukk pFileName)
{
	FILE* pFile = gEnv->pDrxPak->FOpen(pFileName, "wb");
	for (i32 i = 0; i < m_stringAddrs.size(); ++i)
	{
		string logString;
		logString.Format("%s\r\n", GetString(i));
		gEnv->pDrxPak->FWrite(logString.c_str(), logString.size(), pFile);
	}
	gEnv->pDrxPak->FClose(pFile);
}
#endif
