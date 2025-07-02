// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __COMPRESSINGSTREAM_H__
#define __COMPRESSINGSTREAM_H__

#pragma once

#include <drx3D/Network/Config.h>
#if INCLUDE_DEMO_RECORDING

	#include <drx3D/Network/ArithAlphabet.h>
	#include <drx3D/Network/SimpleOutputStream.h>
	#include <drx3D/Network/SimpleInputStream.h>

	#if USE_ARITHSTREAM
typedef CArithAlphabetOrder0 TArithAlphabetCompressingStream;
	#endif

class CCompressingOutputStream : public CSimpleOutputStream
{
public:
	static const size_t DEFAULT_BUFFER_SIZE = 64 * 1024;

	CCompressingOutputStream(size_t bufsz = DEFAULT_BUFFER_SIZE);
	~CCompressingOutputStream();

	bool Init(tukk filename);

	void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CCompressingOutputStream");

		pSizer->Add(*this);
		pSizer->Add(m_tempBuffer, m_bufsz);
	}

private:
	FILE*  m_file;
	tuk  m_tempBuffer;
	size_t m_bufsz;

	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
	tuk*                          m_tempBW;
	TArithAlphabetCompressingStream m_alphabet;
	#endif

	virtual void Flush(const SStreamRecord* pRecords, size_t numRecords);

	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_WRITTEN
	i32   m_nLog;
	FILE* m_file_txt;
	#endif
	#if COMPRESSING_STREAM_SANITY_CHECK_EVERYTHING
	tuk m_throwaway;
	#endif
};

class CCompressingInputStream : public CSimpleInputStream
{
public:
	CCompressingInputStream(size_t bufsz = CCompressingOutputStream::DEFAULT_BUFFER_SIZE);
	~CCompressingInputStream();

	bool Init(tukk filename);

	void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CCompressingInputStream");

		pSizer->Add(*this);
		pSizer->Add(m_tempBuffer, m_nCapacity);
	}

private:
	virtual void Underflow(SStreamRecord* pStream, size_t& count);

	FILE*                           m_file;
	size_t                          m_nCapacity;

	tuk                           m_tempBuffer;
	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
	tuk*                          m_tempBW;
	TArithAlphabetCompressingStream m_alphabet;
	#endif

	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_READ
	i32   m_nBlock;
	FILE* m_file_txt;
	#endif
};

#endif

#endif
