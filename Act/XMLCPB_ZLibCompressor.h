// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once
#ifndef XMLCPB_ZLIBCOMPRESSOR_H
	#define XMLCPB_ZLIBCOMPRESSOR_H

	#include <drx3D/CoreX/Platform/IPlatformOS.h>
	#include <drx3D/Act/XMLCPB_Common.h>

namespace XMLCPB {

bool InitializeCompressorThread();
void ShutdownCompressorThread();

//////////////////////////////////////////////////////////////////////////

struct SZLibBlock
{
	SZLibBlock(class CZLibCompressor* pCompressor);
	~SZLibBlock();

	CZLibCompressor* m_pCompressor;
	u8*           m_pZLibBuffer;                         // data that is going to be compressed is stored here
	u32           m_ZLibBufferSizeUsed;                  // how much of m_pZLibBuffer is currently used
};

class CZLibCompressor
{
public:
	CZLibCompressor(tukk pFileName);
	~CZLibCompressor();

	void         WriteDataIntoFile(uk pSrc, u32 numBytes);
	void         AddDataToZLibBuffer(u8*& pSrc, u32& numBytes);
	void         CompressAndWriteZLibBufferIntoFile(bool bFlush);
	void         FlushZLibBuffer();
	SFileHeader& GetFileHeader() { return m_fileHeader; }

public:

	SFileHeader  m_fileHeader;                              // actually a footer
	class CFile* m_pFile;
	SZLibBlock*  m_currentZlibBlock;
	bool         m_bUseZLibCompression;
	bool         m_errorWritingIntoFile;
};

}  // end namespace

#endif // XMLCPB_ZLIBCOMPRESSOR_H
